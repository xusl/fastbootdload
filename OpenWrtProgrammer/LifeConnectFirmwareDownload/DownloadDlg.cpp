
// DownloadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Download.h"
#include "DownloadDlg.h"
#include "NicListDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "log.h"
#include <setupapi.h>
#include <dbt.h>
#include "telnet.h"
#include "tftp.h"
#include "VersionInfo.h"

using namespace std;

#define TPST
//#define LOGIN
#undef DESKTOP_TEST

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum E_information {
    HOST_NIC,
    PTS_STATUS,
    DEV_IP_ADDR,
    DEV_FW_VERSION,
    DEV_OS_VERSION
};

enum E_fields { FD_PEER, FD_FILE, FD_START, FD_PROGRESS, FD_BYTES, FD_TOTAL, FD_TIMEOUT };

static struct S_TftpGui *pTftpGuiFirst=NULL;

static void gmt_time_string(char *buf, size_t buf_len)
 {
  struct tm newtime;
   __int64 ltime;
   errno_t err;

   _time64( &ltime );
      // Obtain coordinated universal time:
   err = _gmtime64_s( &newtime, &ltime );
   if (err)
   {
      LOGE("Invalid Argument to _gmtime64_s.");
   }
      // Convert to an ASCII representation
   err = asctime_s(buf, buf_len, &newtime);
   if (err)
   {
      LOGE("Invalid Argument to asctime_s.");
   }

  //  time_t curtime = time(NULL);
  //strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime_s(&curtime));
}

/*
 * Wow64 means Windows-On-Windows64.
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms684139%28v=vs.85%29.aspx
 * WOW64 is the x86 emulator that allows 32-bit Windows-based applications to
 * run seamlessly on 64-bit Windows. WOW64 is provided with the operating
 * system and does not have to be explicitly enabled.
 *
 * So, build this tools under X86 (or win32), do not use X64 build.
 *
 */
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()

private:
  CStatic m_AboutVersion;
public:
    virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX,  IDC_ABOUT_VERSION, m_AboutVersion);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	char path_buffer[MAX_PATH] = {0};

	GetModuleFileName(NULL, path_buffer, MAX_PATH);

    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    CVersionInfo vi;
    //AfxGetApp()->m_pszExeName/*m_pszAppName*/
    vi.GetVersionInfo(path_buffer, "040904B0", "ProductVersion");
    CString version = "Version ";
    version += vi.m_strVersionInfo;
    m_AboutVersion.SetWindowText(version);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


// CDownloadDlg dialog

CDownloadDlg::CDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDownloadDlg::IDD, pParent),
	mWSAInitialized(FALSE),
	m_LogText(""),
#ifdef _WIN64
	m_DialgoTitle("LifeConnect TPST (x64)")
#else
	m_DialgoTitle("LifeConnect TPST (win32)")
#endif
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pCoordinator = new DeviceCoordinator;
    m_SyncSemaphore = CreateSemaphore(NULL, 1, 1, "ThreadSyncDevie");
	server_state=false;
	Progress_range=350;
	is_downloading=false;
	b_download=false;
    m_Config.ReadConfigIni(CONFIG_FILE);
    mNic.Configuration(&m_Config);
}

CDownloadDlg::~CDownloadDlg() {
    if (mWSAInitialized) {
        WSACleanup();
        mWSAInitialized = FALSE;
    }

    delete m_pCoordinator;
    CloseHandle(m_SyncSemaphore);

    StopLogging();
}

void CDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESS1, m_progMac2);
    	//m_MessageControl=(CEdit*)GetDlgItem(IDC_Error_Message);
	DDX_Control(pDX, IDC_Error_Message, m_MessageControl);
    DDX_Control(pDX, IDC_FIRMWARE_IMAGE, m_RomPathStaticText);
    DDX_Control(pDX, IDC_DEVICE_IPADDRESS, m_DeviceIpAddress);
    DDX_Control(pDX, IDC_DEVICE_OS_VERSION, m_DeviceOSVersion);
    DDX_Control(pDX, IDC_DEVICE_FIRMWARE_VERSION, m_DeviceFWVersion);
    DDX_Control(pDX, IDC_FILE_STATS, m_PSTStatus);
    DDX_Control(pDX, IDC_DISABLE_CHECK, m_VersionCheckButton);
    DDX_Control(pDX, IDC_NIC_INFORMATION, m_NicInformation);
    DDX_Control(pDX, IDCANCEL, m_ExitButton);
    DDX_Control(pDX, IDC_LV_TFTP, m_TransferFileList);
    DDX_Control(pDX, IDC_PACKAGE_FIRMWARE_VERSION, m_RomVersion);
  	DDX_Control(pDX, IDC_CHANGE_NIC, m_ChangeNic);
//	DDX_Text(pDX, IDC_CHANGE_NIC, m_ChangeNic);
}

BEGIN_MESSAGE_MAP(CDownloadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_DEVICECHANGE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_Browse, &CDownloadDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(ID_Start, &CDownloadDlg::OnBnClickedStart)
    ON_MESSAGE(UI_MESSAGE_TFTPINFO, &CDownloadDlg::OnMessageTftpInfo)
    ON_BN_CLICKED(IDC_DISABLE_CHECK, &CDownloadDlg::OnBnClickedDisableCheck)
    ON_BN_CLICKED(IDC_CHANGE_NIC, &CDownloadDlg::OnBnClickedChangeNic)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDCANCEL, &CDownloadDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDownloadDlg message handlers

BOOL CDownloadDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    WSADATA wsaData;
    WORD  wVersionRequested = MAKEWORD(2, 0);
    if ( WSAStartup(wVersionRequested, &wsaData ) != 0 ) {
        AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
    } else {
        if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
            HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)) {
            LOGE("WSAStartup error, version not match");
            WSACleanup( );
        } else {
            mWSAInitialized = TRUE;
        }
    }

    HICON icon = AfxGetApp()->LoadIcon(IDI_LOCK);
    CButton* versionCheck = (CButton *)GetDlgItem(IDC_DISABLE_CHECK);
    versionCheck->SetIcon(icon);

    m_TransferFileList.SetExtendedStyle(//LVS_EX_GRIDLINES |
                                    LVS_EX_FULLROWSELECT |
                                    //LVS_EX_COLUMNSNAPPOINTS |
                                    LVS_EX_BORDERSELECT);

    m_TransferFileList.InsertColumn(FD_PEER,     _T("Peer"), LVCFMT_CENTER, 130);
    m_TransferFileList.InsertColumn(FD_FILE,     _T("File"), LVCFMT_CENTER, 250);
    m_TransferFileList.InsertColumn(FD_START,    _T("Start"), LVCFMT_CENTER, 80);
    m_TransferFileList.InsertColumn(FD_PROGRESS, _T("Progress"), LVCFMT_CENTER, 100);
    m_TransferFileList.InsertColumn(FD_BYTES,    _T("Bytes"), LVCFMT_CENTER, 100);
    m_TransferFileList.InsertColumn(FD_TOTAL,    _T("Total"), LVCFMT_CENTER, 100);
    m_TransferFileList.InsertColumn(FD_TIMEOUT,  _T("Timeout"), LVCFMT_CENTER, 80);

/*
  CFont * font = emphasize->GetFont();
  if (font != NULL) {
    CFont boldFont;
    LOGFONT lf;
    font->GetObject(sizeof(LOGFONT), & lf);
    lf.lfWeight = FW_BOLD;
    boldFont.CreateFontIndirect(&lf);
    emphasize->SetFont(&boldFont);
    boldFont.Detach();
  }
*/
    CFont* ptf = m_PSTStatus.GetFont();
    LOGFONT lf;
    ptf->GetLogFont(&lf);
    lf.lfHeight = 20;
    lf.lfWeight = FW_BOLD;
    CFont font;
    font.CreateFontIndirect(&lf);
    m_PSTStatus.SetFont(&font);

    m_progMac2.SetRange(0,Progress_range);
    m_progMac2.SetBarColor(RGB(255,255,0));
    m_progMac2.SetTextColor(RGB(0,0,0));
    m_progMac2.SetStep(10);

    //GetPrivateProfileString(_T("MISC"), _T("SSIDPrefix"), _T(""), s_SSID_Prefix, 20, config);
    //s_SSID_Prefix[strlen(s_SSID_Prefix)] = 0;

    //if(!PathFileExists(WS_LABEL_DIR)) {
    //    CreateDirectory(WS_LABEL_DIR, NULL);
    //}
    GetDlgItem(ID_Start)->EnableWindow(TRUE);

    SetWindowText(m_DialgoTitle.GetString());
    m_RomPathStaticText.SetWindowText(m_Config.GetPackageDir());
    CString RomVersion;
    m_Config.GetFirmwareReleaseVersion(RomVersion);
    m_RomVersion.SetWindowText(RomVersion);
    m_pCoordinator->SetDownloadFirmware(m_Config.GetFirmwareFiles());

    mNic.EnumNetCards();
    SetInformation(HOST_NIC, "");
    if (mNic.GetNicNum() <= 1)
        m_ChangeNic.ShowWindow(SW_HIDE);

//#define NIC_DEBUG
#ifdef NIC_DEBUG
    NetCardStruct nic = mNic.GetDefaultNic();
    CString nicInfo;
    nicInfo.Format("%s %s ", nic.mConnectionName.c_str(), nic.mNicDesc.c_str());
    m_NicInformation.SetWindowText(nicInfo.GetString());
    mNic.SetIP("192.168.1.10", "192.168.1.1", "255.255.255.0");
    //mNic.SetIP("192.168.1.121", "192.168.1.1", "255.255.255.0") ;
    //mNic.SetIP("192.168.237.138", "192.168.237.1", "255.255.255.0") ;
    //mNic.EnableDhcp();
    //::MessageBox(this->m_hWnd, "Successfully!", "Set Ip Result", MB_OK | MB_ICONINFORMATION);
    //::MessageBox(this->m_hWnd, "Failed", "Set Ip Result", MB_OK | MB_ICONERROR);
#endif

    return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CDownloadDlg::CleanDevice(const char *const ipAddr, BOOL killSniff, BOOL updateUI) {
    CString msg;
    CDevLabel *dev = NULL;

    m_pCoordinator->GetDevice(ipAddr, &dev, true);

#ifdef MULTI_DEVICE_FEATURE
    if(dev == NULL) {
        m_pCoordinator->GetDevice(ipAddr, &dev, false);
    }
#endif

    if (dev == NULL || (updateUI == FALSE && FALSE == dev->CheckRemovable())) {
        return FALSE;
    }

/*
    msg.Format("Remove device %s ", ipAddr);
    if (dev->GetStatus() != DEVICE_FINISH)
        msg += "for work timeout.";
    else
        msg += "for update finish.";
*/

#ifdef MULTI_DEVICE_FEATURE
    list<DWORD> ids;
    dev->GetTransferIDs(ids);
    list<DWORD>::iterator it;
    for (it = ids.begin(); it != ids.end(); ++it) {
        LVFINDINFO  LvInfo;
        int itemPos;

        // search peer field (key)
        LvInfo.flags = LVFI_PARAM;
        LvInfo.lParam = *it;
        itemPos = m_TransferFileList.FindItem (& LvInfo);
        if (itemPos != -1)
            m_TransferFileList.DeleteItem(itemPos);
    }
#else
    m_TransferFileList.DeleteAllItems();
#endif

    b_download = false;

    MessageBeep(MB_ICONWARNING);
    m_pCoordinator->RemoveDevice(dev);

    if (m_Config.GetRunMode() ==RUNMODE_ONCE)
        StopWork(killSniff);
    else
        StopTftpd32Services ();
    //SetPtsText(msg);
    mNic.FlushArp();
    return TRUE;
}

DWORD WINAPI CDownloadDlg::CaptureLog(LPVOID lpPARAM)  {
}
	
/*
* Two important note:
* 1. Life Connect will detect internet connection, so it alway changes ip from 192.168.1.1 to 192.168.237.1
*   when it connect to a host pc, not a router.
* 2. reboot sometimes may not take affect, so tools start tftp server, but none fetch file request.
*/
DWORD WINAPI CDownloadDlg::NetworkSniffer(LPVOID lpPARAM) {
    CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
    BOOL result = FALSE;
    ConfigIni* conf;
    string ipAddress;
    NicManager *nm;
    CString msg;
    int    mode;

    conf = pThis->GetConfig();
    mode = conf->GetRunMode();

    nm = pThis->GetNicManager();
    nm->UpdateIP();

    //int from = conf.GetHostIPStart();
    //int to = conf.GetHostIPEnd();
    //const char * const segment = conf.GetNetworkSegment();
    //msg.Format(_T("Search device by IP from %s.%d to %s.%d"), segment, from, segment,  to);
    //pThis->UpdateMessage(msg);

#ifdef TPST
    ipAddress.clear();
#else
    nm->SetIP("192.168.1.10", "192.168.1.1", "255.255.255.0");
    ipAddress = "192.168.1.1";
#endif

    pThis->SetInformation(HOST_NIC, "");
    NetCardStruct nic =nm->GetDefaultNic();
    DeviceCoordinator * dc = pThis->GetDeviceCoodinator();

    LOGD("Search LifeConnect through %s (%s) at mode %s", nic.mNicDesc.c_str(),
                        nic.mConnectionName.c_str(),
                        mode == RUNMODE_CYCLE ? APP_RM_CYCLE : APP_RM_ONCE);

    for(;;) {
#ifdef TPST
        if ( (dc->IsEmpty()) && (pThis->b_download == FALSE || ipAddress.size() == 0)) {
//            LOGD("b_download %d, ipAddress %s", pThis->b_download, ipAddress.c_str());
/*
            if(!nic.IsInvalid() || !nic.mEnableDHCP) {
            nm->EnableDhcp();
            } else {
             ipAddress = nic.mGateway;
            result = pThis->SniffNetwork(ipAddress.c_str());
            }
*/
            if (nm->GetConnectedState() == FALSE) {
                if (mode == RUNMODE_ONCE) {
                    pThis->StopWork(FALSE);
                    pThis->SetPtsText("No network connection, please check whether cable inserted.");
                    break;
                } else {
                    pThis->SetPtsText("No network connection, please check whether cable inserted.");
                    Sleep(TIMER_ELAPSE);
                    continue;
                }
            }

            if ((!nic.IsInvalid()) &&
                ((nic.mGateway == "192.168.1.1") || (nic.mGateway == "192.168.237.1"))) {
                result = pThis->SniffNetwork("Policy default", nic.mGateway.c_str());
                if (result)
                    ipAddress = nic.mGateway;
            }

            if (result == FALSE && nic.mGateway != "192.168.1.1") {
                pThis->UpdateMessage("Change IP 192.168.1.10, Gateway 192.168.1.1");
                result = nm->SetIP("192.168.1.10", "192.168.1.1", "255.255.255.0");
                if (result)
                    result = pThis->SniffNetwork("Policy 1", "192.168.1.1");
                if (result)
                    ipAddress = "192.168.1.1";
            }
            if ( result == FALSE && nic.mGateway != "192.168.237.1") {
                pThis->UpdateMessage("Change IP 192.168.237.10, Gateway 192.168.237.1");
                result = nm->SetIP("192.168.237.10", "192.168.237.1", "255.255.255.0");
                if (result)
                    result = pThis->SniffNetwork("Policy 2", "192.168.237.1");
                if (result)
                    ipAddress = "192.168.237.1";
            }
            if (result) {
                LOGD("Start update device %s", ipAddress.c_str());
                CDevLabel *dev = NULL;
                int code = pThis->TelnetPST(&dev);
                if (NO_ERROR == code ) {
                    code = pThis->TFTPDownload(dev, TRUE);
                    if (NO_ERROR != code && dev != NULL) {
                      if (pThis->SniffNetwork("Policy again", ipAddress.c_str())) {
                          dev->SetStatus(DEVICE_ARRIVE);
                           code = pThis->TelnetPST(NULL);
                           if (NO_ERROR == code ) {
                               code = pThis->TFTPDownload(dev, FALSE);
                           }
                      }
                    } else {
                        LOGE("return error %d, dev 0x%x", code, dev);
                    }
                } else if (ERROR_OPEN_FAILED == code) {
                    if (dc->GetSuperMode()) {
                        pThis->SetPtsText("Can not connects to device, start download server.");
                        pThis->b_download = true;
                        code = pThis->TFTPDownload(dev, FALSE);
                    } else {
                        pThis->SetPtsText( "Can not connects to device");
                    }
                }
                if (code == NO_ERROR) {
                    nic =nm->GetDefaultNic();
                } else if (mode == RUNMODE_ONCE ) {
                    pThis->CleanDevice(ipAddress.c_str(), FALSE, TRUE);
                    break;
                }
            } else if (mode == RUNMODE_ONCE) {
                pThis->StopWork(FALSE);
                pThis->SetPtsText( "None device found, stop working.");
                break;
            } else {
                Sleep(TIMER_ELAPSE);
            }
        } else {
            if (!nic.IsInvalid()) {
                ipAddress = nic.mGateway;
            }
            LOGD("Watchdog bite %d", ipAddress.c_str());
            result = pThis->CleanDevice(ipAddress.c_str(), FALSE, FALSE);
            if (result && (mode == RUNMODE_ONCE)){
                break;
            } else{
                Sleep(TIMER_ELAPSE);
            }
        }
#else
        result = pThis->SniffNetwork("HDT", ipAddress.c_str());
        if (result) {
            pThis->TelnetPST();
        }
#endif
    }
    return 0;
}

BOOL CDownloadDlg::SniffNetwork(const char * const tag, const char * const pcIpAddr) {
    CString msg;
    //const char * const segment = m_Config.GetNetworkSegment();
    //msg.Format(_T("Search device by IP from %s.%d to %s.%d"), segment, 1, segment, from, to);
    //UpdateMessage(msg);

    string mac;
    ASSERT(pcIpAddr != NULL && tag != NULL);

    msg.Format(_T("%s: sniff device %s"), tag, pcIpAddr);
    UpdateMessage(msg);
    SetInformation(HOST_NIC, "");

//unfortunately, uboot do not response PING message.
/*
    if(Ping(pcIpAddr) == FALSE) {
        LOGD("ping %s failed. ", pcIpAddr);
        CleanDevice(pcIpAddr);
        return FALSE;
    }
*/

   int waitCount = 0;

   for (; waitCount < 5; waitCount++) {
        if (0 != mNic.ResolveIpMac(pcIpAddr, mac) &&
           !mNic.CheckIpInArpTable(pcIpAddr, mac) ) {
           LOGE("can not found device, wait 2000");
           Sleep(2000);
        } else {
          break;
        }
   }

   if (waitCount >= 5) {
        msg.Format("can not find device (%s ).", pcIpAddr);
        SetPtsText( msg);
        return FALSE;
    }

#if 0
    if (0 != mNic.ResolveIpMac(pcIpAddr, mac) &&
       !mNic.CheckIpInArpTable(pcIpAddr, mac)) {
        msg.Format("can not find device (%s ).", pcIpAddr);
        SetPtsText( msg);
        return FALSE;
    }
#endif

    if (!m_pCoordinator->AddDevice(CDevLabel(mac, string(pcIpAddr)) , NULL)) {
        msg.Format("%s have alread been add into device manager", mac.c_str());
        SetPtsText( msg);
    } else {
        msg.Format(_T("Found device mac :%s, ip: %s"), mac.c_str(), pcIpAddr);
        UpdateMessage(msg);
    }
    //Schedule();
    //SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    //WaitForSingleObject(m_SyncSemaphore, TIMER_ELAPSE);//INFINITE);
    return TRUE;
}

void CDownloadDlg::OnTimer(UINT_PTR nIDEvent) {
   // KillTimer(nIDEvent);
   // Schedule();
}

#ifdef MULTI_DEVICE_FEATURE
DWORD CDownloadDlg::Schedule() {
    if (is_downloading == FALSE) {
        LOGE("PST is now stopped");
        return -1;
    }
    if (b_download) {
        LOGE("PST is now updating a device");
        return -2;
    }
    if (!m_pCoordinator->IsEmpty()) {
     //   ReleaseSemaphore(m_SyncSemaphore, 1, NULL);
        m_WorkThreadHandle = CreateThread(NULL,
            0,
            WorkThread,
            this,
            0,
            &m_WorkThreadID);
        // WaitForSingleObject(m_WorkThreadHandle, INFINITE);
    }
    return 0;
}
#endif

void CDownloadDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDownloadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDownloadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

afx_msg HBRUSH CDownloadDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    if(nCtlColor == CTLCOLOR_STATIC){
      if(pWnd->GetDlgCtrlID()== IDC_FILE_STATS) {
           pDC->SetTextColor(RGB(255,0,0)); //文字颜色
           //pDC->SetBkColor(RGB(251, 247, 200));
           pDC->SetBkMode(TRANSPARENT);//透明
           //return (HBRUSH)::GetStockObject(NULL_BRUSH);
           //return (HBRUSH) m_Brush.GetSafeHandle();
       }
      }

    return brush;
}
void CDownloadDlg::OnBnClickedButtonBrowse() {
#if 0
    CFileDialog dlgFile(TRUE);
    CString fileName;
    CString selectedPath;
    const int c_cMaxFiles = 100;
    const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
    char *p = fileName.GetBuffer(c_cbBuffSize);
    OPENFILENAME& ofn = dlgFile.GetOFN( );
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFilter = _T("Image Files (*.sam;*.uImage;*.jffs2)|*.sam;*.uImage;*.jffs2|")
        _T("All Files (*.*)|*.*||");
    ofn.lpstrFile = p;
    ofn.nMaxFile = c_cbBuffSize;

    if (dlgFile.DoModal())
    {
        selectedPath = dlgFile.GetPathName();
    }

    fileName.ReleaseBuffer();

    /*
       char* pBufEnd = p + FILE_LIST_BUFFER_SIZE - 2;
       char* start = p;
       while( ( p < pBufEnd ) && ( *p ) )
       p++;
       if( p > start )
       {
       _tprintf(_T("Path to folder where files were selected:  %s\r\n\r\n"), start );
       p++;

       int fileCount = 1;
       while( ( p < pBufEnd ) && ( *p ) )
       {
       start = p;
       while( ( p < pBufEnd ) && ( *p ) )
       p++;
       if( p > start )
       _tprintf(_T("%2d. %s\r\n"), fileCount, start );
       p++;
       fileCount++;
       }
       }
       */
    if(!PathFileExists(selectedPath)) {
        ::MessageBox(NULL, selectedPath ,_T("Check Fireware file"),MB_OK);
        return;
    }

    //::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_FIRMWARE_IMAGE,selectedPath);
    m_RomPathStaticText.SetWindowText(selectedPath);
    m_Config.SetPackageDir(selectedPath.GetString());
#else
    char szPath[MAX_PATH] = { 0 };
    //ZeroMemory(szPath, sizeof(szPath));

    BROWSEINFO bi;
    bi.hwndOwner = m_hWnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szPath;
    bi.lpszTitle = "Please select path place firmware";
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;
    bi.iImage = 0;
    LPITEMIDLIST lp = SHBrowseForFolder(&bi);

    if(lp && SHGetPathFromIDList(lp, szPath))
    {
        if (m_Config.ReadFirmwareFiles(szPath, TRUE)) {
            m_RomPathStaticText.SetWindowText(szPath);
            m_Config.SetPackageDir(szPath, TRUE);
            m_pCoordinator->SetDownloadFirmware(m_Config.GetFirmwareFiles());
            CString RomVersion;
            m_Config.GetFirmwareReleaseVersion(RomVersion);
            m_RomVersion.SetWindowText(RomVersion);
        } else {
        CString msg;
        msg.Format("folder %s does not contain any firmware.", szPath);
        AfxMessageBox(msg);
        }
    } else {
        AfxMessageBox("The folder is not exist");
    }
#endif
}

VOID CDownloadDlg::StartWork() {
    LOGI("Start PST");
    SetUIStatus(FALSE);
    ClearMessage();
    m_DeviceIpAddress.SetWindowText("");
    m_DeviceOSVersion.SetWindowText("");
    m_DeviceFWVersion.SetWindowText("");
    m_PSTStatus.SetWindowText("");
    is_downloading=TRUE;
    b_download = false;
    //SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    //m_pCoordinator->AddDevice(CDevLabel(string("FC-4D-D4-D2-BA-84"), string("192.168.1.10")) , NULL);
    //Schedule();
    //Server_Listen_Thread=CreateThread(NULL,0,Thread_Server_Listen,this,0,&Server_Listen_Thread_ID);
    m_NetworkSnifferThreadHandle = CreateThread(NULL,
                                        0,
                                        NetworkSniffer,
                                        this,
                                        0,
                                        &m_NetworkSnifferThreadID);
}

VOID CDownloadDlg::StopWork(BOOL killSniff) {
   if (killSniff) {
       TerminateThread(m_NetworkSnifferThreadHandle, 1);
   }

   StopTftpd32Services ();
   SetUIStatus(TRUE);
   is_downloading = false;
   m_pCoordinator->Reset();
   m_TransferFileList.DeleteAllItems();
}

VOID CDownloadDlg::SetUIStatus(BOOL enable) {
    GetDlgItem(ID_Start)->SetWindowText(enable ? "Start" : "Stop");
    GetDlgItem(IDC_BUTTON_Browse)->EnableWindow(enable);
    m_RomPathStaticText.EnableWindow(enable);
    m_ExitButton.EnableWindow(enable);
    m_ChangeNic.EnableWindow(enable);
}

void CDownloadDlg::OnBnClickedStart() {
    if(is_downloading) {
        if (b_download == false && mNic.IsChangingIp() == FALSE) {
            StopWork(TRUE);
            LOGW("Stop PST");
        } else {
         ::MessageBox(NULL,
            _T("Tool is working now."),
            _T("Error"),
            MB_OK);
        }
        return;
    }

    if(m_Config.IsPackageChecked() == FALSE) {
         ::MessageBox(NULL,
            _T("Please select correct package or check Config.ini!"),
            _T("No firmware available"),
            MB_OK);
         return;
    }

#ifdef _WIN64
	//if (!IsWow64()) {
	//	AfxMessageBox("This is x64 Build, please use win32 build");
    //    return;
	//}
#else// _WIN32
	if (IsWow64() && m_Config.GetNicToggle() == NIC_SETUPDI_TOGGLE) {
		AfxMessageBox("This is win32 Build, please use x64 build");
         return;
	}
#endif
    StartWork();
}

#define DEVICE_DOWNLOAD_IP   "192.168.1.1"
#define DEFAULT_SERVER_IP    "192.168.1.10"
#define COMMAND_UPDATE "send_data 254 0 0 7 0 1 0\n"
//#define CMD_REBOOT      "send_data 254 0 0 5 0 0 0\n"
#define CMD_REBOOT      "reboot\n"
#define CMD_FW_VERSION  "jrd_system_get_firmware_version \n"
//#define CMD_OS_VERSION  "cat /proc/version \n"
#define CMD_OS_VERSION  "uname -r\n"
#define SET_DOWNLOAD_DEVIP       "fw_setenv ipaddr " //DEVICE_DOWNLOAD_IP
#define SET_DOWNLOAD_SRVIP "fw_setenv serverip " //DEFAULT_SERVER_IP

int CDownloadDlg::TelnetPST(CDevLabel **ppDev) {
    CString msg;
    string osVersion;
    string fwVersion;
    string customId;
    string versionCode;
    string buildId;
	string cmd;
    string result;
    int i = 0;
    NetCardStruct nic = mNic.GetDefaultNic();
    DeviceCoordinator * dc = GetDeviceCoodinator();

    CDevLabel *dev = dc->GetValidDevice();
    if (dev == NULL) {
        SetPtsText( "There is none device needs to update.");
        if (m_Config.GetRunMode() ==RUNMODE_ONCE)
            StopWork(FALSE);
        return ERROR_INVALID_HANDLE;
    }
    if (ppDev != NULL) {
        *ppDev = dev;
    }

    SOCKET sock = CreateSocket(dev->GetIpAddr().c_str());
    if ( sock == INVALID_SOCKET) {
        return ERROR_OPEN_FAILED;
    }

    dev->SetStatus(DEVICE_COMMAND);
    SetInformation(DEV_IP_ADDR, dev->GetIpAddr().c_str());

    b_download = true;
    TelnetClient tn(sock, m_Config.GetTelnetTimeoutMs(), m_Config.GetVerbose());

    LOGE("start telnet negotiate");
    tn.send_command(NULL, result);
    //UpdateMessage(result.c_str());
    tn.send_command(NULL, result);
    //UpdateMessage(result.c_str());

#ifdef DESKTOP_TEST
    m_PSTStatus.SetWindowText("Do user login");

    LOGE("User Login, enter user ");
    //if (strstr(buf, "login") != NULL)
    {
        dev->TickWatchDog();
        tn.send_command(m_Config.GetLoginUser(), result, false);
        UpdateMessage(result.c_str());

        LOGE("Send password ");
        dev->TickWatchDog();
        tn.send_command(m_Config.GetLoginPassword(), result, false);
        UpdateMessage(result.c_str());
    }
#endif

    //dev->TickWatchDog();
    //tn.send_command(CMD_OS_VERSION, osVersion);
    //SetInformation(DEV_OS_VERSION, osVersion.c_str());

    dev->TickWatchDog();
    SetPtsText( "Get device firmware version");
    tn.send_command(CMD_FW_VERSION, fwVersion);

    if (fwVersion.length() == 0) {
        msg =  "Cannot get firmware version";
        goto TELPSTERR;
    }

    SetInformation(DEV_FW_VERSION, fwVersion.c_str());

    m_Config.ParseExternalVersion(fwVersion, customId, versionCode, buildId);

    if (dc->GetSuperMode() == FALSE) {
        if ( customId != m_Config.GetFirmwareCustomId()) {
            msg = "Customer ID of device and package are not matched.";
            goto TELPSTERR;
        }
        if (buildId >= m_Config.GetFirmwareBuildId()) {
            msg = "Device firmware is the lastest version.";
            goto TELPSTERR;
        }
    } else {
        if ( customId != m_Config.GetFirmwareCustomId()) {
            int result = ::MessageBox(NULL,
                                  _T("Customer ID of device and package are not matched. Press 'Ok' to continue, press 'Cancel' to abort?"),
                                  _T("Confirm"),
                                  MB_OKCANCEL | MB_ICONQUESTION | MB_TASKMODAL);
            if (result == IDOK) {
                LOGE("Confirm to update device when customer ID of device and package are not matched.");
            } else {
                msg = "Customer ID of device and package are not matched.";
                goto TELPSTERR;
            }
        }
        if (buildId >= m_Config.GetFirmwareBuildId()) {
            int result = ::MessageBox(NULL,
                                  _T("Device firmware is the lastest version. Press 'Ok' to continue, press 'Cancel' to abort?"),
                                  _T("Confirm"),
                                  MB_OKCANCEL | MB_ICONQUESTION | MB_TASKMODAL);
            if (result == IDOK) {
                LOGE("Confirm to update when Device firmware is the lastest version.");
            } else {
                msg = "Device firmware is the lastest version.";
                goto TELPSTERR;
            }
        }
    }

    for (i = 0; i < 3; i++) {
        if (dc->RequestDownloadPermission(dev)) {
            break;
        }

        if (i == 3) {
            msg = "Request download failed. Other device is block!";
            goto TELPSTERR;
        }
        Sleep(3000);
    }

	msg.Format( "Set download environment, ip %s, server %s",
		dev->GetIpAddr().c_str(), nic.mIPAddress.c_str());
    SetPtsText(msg);
	cmd = SET_DOWNLOAD_DEVIP;
	cmd += dev->GetIpAddr(); //nic.mGateway
    tn.send_command(cmd.c_str(), result, false);	
	cmd = SET_DOWNLOAD_SRVIP;
	cmd += nic.mIPAddress;
    tn.send_command(cmd.c_str(), result, false);
	
    SetPtsText( "Enter download mode");
    dev->TickWatchDog();
    tn.send_command(COMMAND_UPDATE, result, false);

    SetPtsText( "Let device enters download mode");
    dev->TickWatchDog();
    tn.send_command(CMD_REBOOT, result, false);

/*
after send reboot command
C:\WinDDK\7600.16385.1>arp -a

Interface: 169.254.74.103 --- 0x70002
  Internet Address      Physical Address      Type
  192.168.237.1         0e-e0-f0-19-18-92     dynamic

C:\WinDDK\7600.16385.1>ipconfig

Windows IP Configuration


Ethernet adapter 本地连接:

        Connection-specific DNS Suffix  . :
        Autoconfiguration IP Address. . . : 169.254.74.103
        Subnet Mask . . . . . . . . . . . : 255.255.0.0
        Default Gateway . . . . . . . . . :

*/
    LOGE("wait for device reboot");
    Sleep(500);
    closesocket(sock);
    return NO_ERROR;

TELPSTERR:
    SetPtsText(msg.GetString());
    closesocket(sock);
    b_download = false;
    MessageBeep(MB_ICONERROR);
    return ERROR_GEN_FAILURE;
}

int CDownloadDlg::TFTPDownload(CDevLabel *pDev, BOOL again) {
    CString msg;
//	string hostIp;
	string gatewayIp;
	string deviceIp = DEVICE_DOWNLOAD_IP;
    string mac;
    int  waitCount = 0;
    NetCardStruct nic = mNic.GetDefaultNic();
	BOOL reset = FALSE;
	
	if (pDev != NULL && pDev->GetIpAddr().length() > 0) {		
    	deviceIp = pDev->GetIpAddr();	
		gatewayIp = deviceIp;//nic.mGateway
		if (nic.mEnableDHCP) {		
	        msg.Format("Set Host IP %s, Gateway %s", nic.mIPAddress.c_str(), gatewayIp.c_str());
	        SetPtsText( msg);
	        mNic.SetIP(nic.mIPAddress.c_str(), gatewayIp.c_str(), nic.mSubnetMask.c_str());
		}
	} else if (nic.mIPAddress != DEFAULT_SERVER_IP) {
        msg.Format("set Host IP %s, Gateway %s", DEFAULT_SERVER_IP, DEVICE_DOWNLOAD_IP);
        SetPtsText( msg);
        mNic.SetIP(DEFAULT_SERVER_IP, DEVICE_DOWNLOAD_IP, "255.255.255.0");
		reset = TRUE;
    }

	msg.Format("Find target device with IP Address %s", deviceIp.c_str());
	SetPtsText( msg);
    SetInformation(HOST_NIC, "");

#if 1
#define POLL_TARGET_MAX 30
    for (; waitCount < POLL_TARGET_MAX; waitCount++) {
        if (0 != mNic.ResolveIpMac(deviceIp.c_str(), mac) &&
           !mNic.CheckIpInArpTable(deviceIp.c_str(), mac)) {
           //LOGE("can not found device, wait 2s");
           msg.Format("Wait for target device response, %d time(s)", waitCount + 1);
	       SetPtsText( msg);
           Sleep(3000);
        } else {
          break;
        }
    }
   if (waitCount >= POLL_TARGET_MAX) {
        if (again) {
            SetPtsText( "Target device do not response, try again.");
			if (reset)
              mNic.SetIP(nic.mIPAddress.c_str(), nic.mGateway.c_str(), nic.mSubnetMask.c_str());
        } else {
            SetPtsText( "Lost device, abort updating.");
        }
        return -1;
   }
#endif

    mNic.UpdateIP();
    SetInformation(HOST_NIC, "");
    SetPtsText( "start download server");
    StartTftpd32Services(GetSafeHwnd(), m_pCoordinator);
    return NO_ERROR;
}


DWORD WINAPI CDownloadDlg::WorkThread(LPVOID lpPARAM) {
    CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
#ifdef TPST
    pThis->TelnetPST(NULL);
    return 0;
#else
    CString cmd;

    CString msg;
    //if (pThis->BuildUpdateCommand(pThis->mRomPath, cmd)) {
    DeviceCoordinator * dc = pThis->GetDeviceCoodinator();
    CDevLabel *dev = dc->GetValidDevice();
    if (dev == NULL) {
        LOGE("There is none device in the network.");
        return 2;
    }

    SOCKET sock = pThis->CreateSocket(dev->GetIpAddr().c_str());
    if ( sock == INVALID_SOCKET) {
        return ERROR_INVALID_HANDLE;
    }

    pThis->SetInformation(DEV_IP_ADDR, dev->GetIpAddr().c_str());
    pThis->b_download = true;

    TelnetClient tn(sock);
    string result;

 //   tn.receive_telnet_data(buf, BUFSIZE);
  //  pThis->UpdateMessage(buf);
 #endif

#if defined(MMI)
    char buf[BUFSIZE];
    unsigned char red[100] = "send_data 254 0 2 3 0 1 0\n";
    unsigned char blue[100] = "send_data 254 0 2 3 0 2 0\n";
    char *ledon = "send_data 254 0 2 3 0 3 0\n";
    char *keytest = "send_data 254 0 2 4 1 6 0\n";
    //char *factmood = "send_data 254 0 0 6 0 1 0\n";
    //char *offled = "send_data 254 0 2 3 0 0 0\n";
    char *wifitest = "send_data 254 0 3 0 1 0 0\n";
    char *wifitest1 = "wpa_cli -i wlan0 scan_result \n";
    char *readtrace = "send_data 254 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \n";
    char *writetrace = "send_data 254 0 0 0 0 1 2 3 4 5 6 7 8 9 1 2 3 4 5 6 1 \n";

    //tn.send_telnet_data(COMMAN_UPDATE, strlen(COMMAN_UPDATE));//send command 'ls /\r\n'
    //tn.receive_telnet_data(buf, BUFSIZE);
    //tn.send_telnet_data(COMMAN_REBOOT, strlen(COMMAN_REBOOT));//send command 'ls /\r\n'
    //tn.receive_telnet_data(buf, BUFSIZE);
    tn.receive_telnet_data(buf, BUFSIZE);
    tn.send_telnet_data(ledon, strlen(ledon));
    tn.receive_telnet_data(buf, BUFSIZE);
    pThis->UpdateMessage(buf);

    tn.send_telnet_data(keytest, strlen(keytest));
    pThis->UpdateMessage("Please PRESS key in 6 seconds");
    tn.receive_telnet_data(buf, BUFSIZE);
    pThis->UpdateMessage(buf);

    tn.send_telnet_data(wifitest, strlen(wifitest));
    tn.receive_telnet_data(buf, BUFSIZE);

    pThis->UpdateMessage(buf);
    for (int i = 0; i <= 6; i++) {
        tn.receive_telnet_data(buf, BUFSIZE);

        if (strlen(buf) > 0) {
            pThis->UpdateMessage(buf);
            break;
        }
    }
    pThis->UpdateMessage("Finish");

    closesocket(sock);
    dc->RemoveDevice(dev);

#elif defined(HDT)//HDT
#define COMMAN_GETTRACE "send_data 254 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
#define COMMAN_GETMAC "send_data 254 0 3 1 1 0 0 0 0 0 0\n"
#define COMMAN_UPDATE "send_data 254 0 0 7 0 1 0\n"
#define COMMAN_REBOOT "send_data 254 0 0 5 0 0 0\n"
    int i_ret,i,j,k;
    char s_trace[17]={0};
    char s_PCBNo[16]={0};
    char s_OldWIFINo[13]={0},s_WIFIWrite[7]={0},s_NewWIFINo[13] = {0};
    unsigned char s_WIFIREAD[7]={0};
    char s_FmtStr[5] ;
    int i_TempInt;
    char s_InetCmd[SUBBUFSIZE]={0};
    char s_SN[16]={0};
    char s_memo[301]={0};
    char buf[BUFSIZE];
    i_ret = tn.send_telnet_data(COMMAN_GETTRACE, strlen(COMMAN_GETTRACE));
    Sleep(1000);
    i_ret = tn.receive_telnet_cmd(buf, BUFSIZE);
    if(i_ret)
    {
        msg.Format(_T("Read traceability fail!"));
        goto ERROR;
    }
    else
    {
        j = 0;
        for(i =67; j < 16; j++)
        {
            s_trace[j] = atoi(buf+i);
            while(buf[i] != 0x20 && buf[i] != '#')
            {
                i++;
            }
            if(buf[i] == '#')
            {
                break;
            }
            i++;
        }
        strncpy(s_PCBNo,s_trace,15);

    }
    i_ret = tn.send_telnet_data(COMMAN_GETMAC, strlen(COMMAN_GETMAC));
    Sleep(1000);
    i_ret = tn.receive_telnet_cmd(buf, BUFSIZE);
    if(i_ret)
    {
        msg.Format(_T("Read WIFI MAC fail!"));
        goto ERROR;
    }
    else
    {
        j = 0;
        for(i =0x2f; j < 6; j++)
        {
            s_WIFIREAD[j] = atoi(buf+i);
            while(buf[i] != 0x20 && buf[i] != '#')
            {
                i++;
            }
            if(buf[i] == '#')
            {
                break;
            }
            i++;
        }
        //strncpy(s_PCBNo,s_trace,15);
    }
    for(i=0;i<6;i++)
    {
        sprintf(s_FmtStr, "%02X", s_WIFIREAD[i]);
        s_OldWIFINo[i*2]=s_FmtStr[0];
        toupper(s_OldWIFINo[i*2]);
        s_OldWIFINo[i*2+1]=s_FmtStr[1];
        toupper(s_OldWIFINo[i*2+1]);
    }
    s_OldWIFINo[6*2]=0;

    {

        strcpy(s_InetCmd,"send_data 254 0 3 1 0");
        for(i=0;i<6;i++)
        {
            if(!isxdigit(s_NewWIFINo[2*i]))
            {
                msg.Format(_T("Get WIFI MAC is invalid!"));
                goto ERROR;
            }
            s_FmtStr[0]=s_NewWIFINo[2*i];
            if(!isxdigit(s_NewWIFINo[2*i+1]))
            {
                msg.Format(_T("Get WIFI MAC is invalid!"));
                goto ERROR;
            }
            s_FmtStr[1]=s_NewWIFINo[2*i+1];
            s_FmtStr[2] = 0;
            sscanf(s_FmtStr, "%x", &i_TempInt);
            sprintf(s_FmtStr,"%d",i_TempInt);
            strcat(s_InetCmd," ");
            strcat(s_InetCmd,s_FmtStr);
        }
        strcat(s_InetCmd,"\n");
        i_ret = tn.send_telnet_data(s_InetCmd, strlen(s_InetCmd));//send command 'ls /\r\n'
        i_ret = tn.receive_telnet_cmd(buf, BUFSIZE);
    }

    GaliSNfromWIFI(s_SN,s_NewWIFINo);

    //i_ret = tn.send_telnet_data(COMMAN_UPDATE, strlen(COMMAN_UPDATE));//send command 'ls /\r\n'
    //i_ret = tn.receive_telnet_data(buf, BUFSIZE);
    //i_ret = tn.send_telnet_data(COMMAN_REBOOT, strlen(COMMAN_REBOOT));//send command 'ls /\r\n'
    //i_ret = tn.receive_telnet_data(buf, BUFSIZE);

    closesocket(sock);
    dc->Reset();
    //dc->RemoveDevice(dev);
    pThis->m_progMac2.SetPos(pThis->Progress_range);
    pThis->m_progMac2.SetBarColor(RGB(0,255,0));
    pThis->m_progMac2.SetWindowText(_T("Download successfully!"));
    pThis->m_progMac2.Invalidate(FALSE);
    //pThis->GetDlgItem(ID_Start)->EnableWindow(true);
    pThis->b_download = true;
#elif defined(TEST)
    char buf[BUFSIZE];
    tn.send_telnet_data("zen\n", strlen("zen\n")); //send user name
    tn.receive_telnet_data(buf, BUFSIZE, false);
    tn.send_telnet_data("zen\n", strlen("zen\n")); //send password
    tn.receive_telnet_data(buf, BUFSIZE);
    pThis->UpdateMessage(buf);
    tn.send_telnet_data("ls /\n", strlen("ls /\n"));//send command 'ls /\r\n'
    tn.receive_telnet_data(buf, BUFSIZE);
    tn.send_telnet_data("echo hello world\n", strlen("echo hello world\n"));//send command 'ls /\r\n'
    tn.receive_telnet_data(buf, BUFSIZE);
    closesocket(sock);
    dc->RemoveDevice(dev);
#endif

    return 0;
}

#if 0
void CDownloadDlg::HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent) {
    UpdateMessage(msg);
    //        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
    if (ppContent != NULL && *ppContent != NULL) {
        free((void *)*ppContent);
        *ppContent = NULL;
    }
    if (INVALID_SOCKET != sockConn)
        closesocket(sockConn);
    if (INVALID_SOCKET != sockSrv)
        closesocket(sockSrv);
}

char const* CDownloadDlg::BuildHttpServerResponse(const char *path, size_t  *contentLength) {
    char* content = NULL;
    __int64  nLen = 0;

    if (path == NULL || contentLength == NULL) {
        UpdateMessage(_T("Bad parameter of BuildHttpServerResponse"));
        return content;
    }
    *contentLength = 0;

    FILE *pF = fopen(path, "rb" );
    if(pF==NULL) {
        //perror(file_name);
        UpdateMessage(_T("Open file failed!"));
        return content;
    }
    _fseeki64(pF, 0, SEEK_END);
    nLen = _ftelli64(pF);

    char temp[64]={0};
    char date[64]={0};
    sprintf(temp, "%I64d", nLen);
    gmt_time_string(date, sizeof(date));
    string header= "HTTP/1.1 200 OK\r\n";
    header.append("Content-Length: ").append(temp).append("\r\n");
    header.append("Date: ").append(date).append("\r\n");
    header.append("Content-Type: application/x-sam\r\n");
    header.append("Accept-Ranges: bytes\r\n" );
    header.append("\r\n");
    size_t length=header.size() + (size_t)nLen + 1;
    rewind(pF);
    content= (char*) malloc(sizeof(char)*length);
    if(!content) {
        UpdateMessage(_T("Failed to allocate memory!"));
        return content;
    }
    memset(content, 0, length);
    memcpy(content, header.c_str(), header.size());
    int len = header.size();

    fread(content + len, 1, (size_t)nLen, pF);
    content[length]='\0';
    fclose(pF);
    //printf("Header length is %d\n", header.size());
    //printf("Header is \n%s\n", header.c_str());
    *contentLength = length;
    return content;
}

BOOL CDownloadDlg::BuildUpdateCommand(CString file, CString &cmd) {
    string Firmware_name=file.GetString();
    int tmp=Firmware_name.find_last_of("\\");
    Firmware_name=Firmware_name.substr(tmp+1,Firmware_name.length());
    const char * filename = basename(file.GetString());

    if(file =="" ) {
        ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
        return FALSE;
    }

    if(mHostIPAddr.empty()) {
        UpdateMessage(_T("Get IP failed!"));
        return FALSE;
    }

    string download_comand="update update -u \"http://"+mHostIPAddr+"/"+Firmware_name+
                            "/\" -f --no-cert-check --no-device-check\r\n\r\n";

    cmd.Format("update update -u \"http://%s/%s/\" -f --no-cert-check --no-device-check\r\n\r\n",
                                mHostIPAddr.c_str(), Firmware_name);

    return TRUE;
}

void CDownloadDlg::OnSend_Comand(SOCKET sockClient, const char * cmd) {
    char s_SN[16]      = {0};
    char s_SSID[51]    = {0};
    char MAC_label[20] = {0};
    char recvBuf[101] = {'\0'};
    string Send_result;
    int i=0;
    int bytes = 0;

    if (sockClient == INVALID_SOCKET) {
        return;
    }

    //recv(sockClient, recvBuf, 100, 0);
    //bool is_set_time_out=SetTimeOut(sockClient, 50000, true);
    bytes = send(sockClient, cmd, strlen(cmd)+1, 1);
    LOGE("Send %d bytes", bytes);

    do {
        memset(recvBuf, '\0', 101);
        bytes = recv(sockClient, recvBuf, 100, 0);
        LOGE("Receive %d bytes", bytes);
        if(strstr(recvBuf,"root@Qualcomm")!=NULL) {
            if(strstr(recvBuf,"Writing data")!=NULL) {
                UpdateMessage(recvBuf);
            }
        }

        i++;
        Send_result+=recvBuf;
        LOGE("%s", recvBuf);
        m_progMac2.SetPos(i);
        //m_progMac2.Invalidate(FALSE);
        if((Send_result.find("[processCommand] Processing send"))!=-1) {
            UpdateMessage(_T("send software successfully!"));
            /*m_progMac2.SetPos(Progress_range);
              m_progMac2.Invalidate(FALSE);
              m_progMac2.SetBarColor(RGB(0,255,0));
              DWORD dwEndTime = ::GetTickCount();
              DWORD dwSpaceTime = (dwEndTime - dwBeginTime)/1000;
              sprintf(s_SSID,"%s_%s_AJ",s_SSID_Prefix,s_WIFI+6);
              strupr(s_WIFI);
              GaliSNfromWIFI(s_SN,s_WIFI);
              sprintf(MAC_label, "%c%c%s%c%c%s%c%c%s%c%c%s%c%c%s%c%c",
              s_WIFI[0], s_WIFI[1], ":", s_WIFI[2], s_WIFI[3], ":",
              s_WIFI[4], s_WIFI[5], ":", s_WIFI[6], s_WIFI[7], ":",
              s_WIFI[8], s_WIFI[9], ":", s_WIFI[10], s_WIFI[11]);
            //DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,Original_Wifi_name,-1,NULL,0,NULL,FALSE);
            //WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)Original_Wifi_name, -1,(LPSTR)s_SSID, dwNum, NULL, FALSE);
            GenSAV(s_SN,s_CommercialRef,s_PCBNo,s_PTS_new,"","","",0,"HDT",dwSpaceTime,0,MAC_label,s_SSID,"","","");
            GetDlgItem(ID_Start)->EnableWindow(true);
            is_downloading=false;
            ::MessageBoxA(NULL,"Download finished, please remove the device for next!","MBO1",MB_SYSTEMMODAL);*/
            m_progMac2.SetWindowText(_T("Send software finished!, Please wait for the device update and then restart "));
            m_progMac2.Invalidate(FALSE);
            Sleep(30000);
            //TODO:: comment by xusl
            //			get_Wifi_connect();
            //is_downloading=false;
            break;
        }

        if((Send_result.find("[ERROR]"))!=-1) {
            UpdateMessage(_T("send software failed!"));
            m_progMac2.SetBarColor(RGB(255,50,50));
            m_progMac2.Invalidate(FALSE);
            GetDlgItem(ID_Start)->EnableWindow(true);
            is_downloading=false;
            ::MessageBox(NULL,_T("Download failed!"),_T("Download"),MB_OK);
            break;
        }
    } while(bytes > 0);
}

DWORD WINAPI CDownloadDlg::Thread_Server_Listen(LPVOID lpPARAM) {
	CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
	pThis->server_listen();
	return 1;
}
void CDownloadDlg::server_listen(u_short port)
{
    SOCKET sockConn = INVALID_SOCKET;
    SOCKET sockSrv  = INVALID_SOCKET;
    CString msg;
    size_t length = 0;
    __int64 iResult;

    if (mWSAInitialized == FALSE) {
        UpdateMessage(_T("Server Startup failed!"));
        return ;
    }

   server_state=false;
    char const * content = BuildHttpServerResponse(m_Config.GetPackageDir(), &length);

    if(content == NULL) {
        UpdateMessage(_T("Open file failed!"));
        return ;
    }

    sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockSrv == INVALID_SOCKET) {
        HandleServerException(_T("Create server socket failed!"), sockConn, sockSrv, &content);
        return ;
    }
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    iResult = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if (iResult == SOCKET_ERROR) {
        msg.Format(_T("Bind socket port %d failed!"), port);
        HandleServerException(msg, sockConn, sockSrv, &content);
        return ;
    }
    msg.Format(_T("bind socket success on port :%d"), port);
    UpdateMessage(msg);
    if (listen(sockSrv, SOMAXCONN) == SOCKET_ERROR) {
        HandleServerException(_T("server listen failed!"), sockConn, sockSrv, &content);
        return ;
    }

    UpdateMessage(_T("Listening on socket..."));
    SOCKADDR_IN  addrClient;
    while(true) {
        unsigned long on = 1;
        char recvBuf[101]={0};

        int sin_size = sizeof(struct sockaddr_in);
        sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &sin_size);
        UpdateMessage(_T("Send softwre ..., please wait..."));
        ioctlsocket(sockConn, FIONBIO, &on);

        int bytes = 0;
        do {
            memset(recvBuf, 0, 101);
            bytes = recv(sockConn, recvBuf, 100, 0);
        } while(bytes > 0);

        iResult = send(sockConn, content , length , 0);

        if (iResult == SOCKET_ERROR) {
            HandleServerException(_T("Send software failed!"), sockConn, sockSrv, &content);
            break;
        }

        do {
            memset(recvBuf, 0, 101);
            bytes = recv(sockConn, recvBuf, 100, 0);
        }while(bytes > 0);
        UpdateMessage(_T("Send softwre successfully!"));

        closesocket(sockConn);
        sockConn = INVALID_SOCKET;
    }
    HandleServerException(_T("Server exist"), sockConn, sockSrv, &content);
}
#endif

SOCKET CDownloadDlg::CreateSocket(const char *ip_addr,  u_short port) {
    SOCKET sockClient = INVALID_SOCKET;
    SOCKADDR_IN addrSrv;
    __int64 iResult;
    if (mWSAInitialized == FALSE) {
        UpdateMessage(_T("Server Startup failed!"));
        return sockClient;
    }

    addrSrv.sin_addr.S_un.S_addr = inet_addr(ip_addr);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //int timeout=50000;
    //iResult=setsockopt(sockClient,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(int));
    //if(SOCKET_ERROR==iResult)
    //return ;

    iResult = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(iResult==SOCKET_ERROR) {
        closesocket(sockClient);
        LOGE("Connect device %s failed!", ip_addr);
        sockClient = INVALID_SOCKET;
    }
    return sockClient;
}

// Create a pTftpGui structure and fill it with the msg
int CDownloadDlg::TFTPNew (const struct S_TftpTrfNew *pTrf)
{
    struct S_TftpGui *pTftpGui;
    pTftpGui = (struct S_TftpGui *)calloc (1, sizeof *pTftpGui);
    pTftpGui->dwTransferId = pTrf->dwTransferId;
    pTftpGui->stat = pTrf->stat;
    pTftpGui->filename = _strdup(pTrf->szFile);
    pTftpGui->opcode = pTrf->opcode;
    pTftpGui->stg_addr = pTrf->from_addr;

    pTftpGui->next = pTftpGuiFirst;
    // places the leaf at the head of the structure
    pTftpGuiFirst = pTftpGui;

    TFTPReporting (pTftpGuiFirst);

    struct subStats stat;
    stat.stat =pTrf->stat;
    stat.dwTransferId = pTrf->dwTransferId;
    time_t dNow;
    time(&dNow);
    TFTPStat(&stat, dNow);

    CString msg;
    msg.Format("Start download file %s", pTftpGui->filename);
    SetPtsText(msg);

    return 0;
} // GuiNewTrf

// terminates a transfer
int CDownloadDlg::TFTPEnd (struct S_TftpTrfEnd *pTrf)
{
    struct S_TftpGui *pTftpGui, *pTftpPrev;
    struct subStats stat;
    time_t dNow;

    stat.stat =pTrf->stat;
    stat.dwTransferId = pTrf->dwTransferId;
    time(&dNow);
    TFTPStat(&stat, dNow);

    // search mathing internal structure and get previous member
    for (pTftpPrev=NULL, pTftpGui=pTftpGuiFirst;
         pTftpGui != NULL && pTftpGui->dwTransferId != pTrf->dwTransferId;
         pTftpGui = pTftpGui->next)
        pTftpPrev = pTftpGui;

    // in the service, the GUI may have missed the begining of the transfer
    if (pTftpGui==NULL)
        return 0;

    CDevLabel *dev= m_pCoordinator->EndFirmwareTransfer(pTftpGui->stg_addr,
                                            pTftpGui->filename,
                                            pTftpGui->dwTransferId);
     BOOL result = FALSE;
     if (dev != NULL)
         result = dev->IsDownloadFinish();

    // detach leaf
    if (pTftpPrev != NULL)
        pTftpPrev->next = pTftpGui->next ;
    else
        pTftpGuiFirst = pTftpGui->next ;

    // now we can play with the leaf : it belongs no more to the linked list
    // update stat
    pTftpGui->stat = pTrf->stat;
    TFTPReporting (pTftpGuiFirst);

    // free allocation
    free (pTftpGui->filename);
    free (pTftpGui);
    LOGD ("GUI: transfer destroyed");

    // recall TFTPReporting : it will notice the process
    TFTPReporting (pTftpGuiFirst);

    if (result && b_download) {
        MessageBeep(MB_ICONINFORMATION);
        SetPtsText("Device updated.");
        if (RUNMODE_ONCE == m_Config.GetRunMode())
            CleanDevice(mNic.GetDefaultNic().mGateway.c_str(), TRUE, TRUE);
        b_download = false;
    }
    return 0;
} // TFTPEnd

int CDownloadDlg::TFTPStat (struct subStats *pTrf, time_t dNow)
{
    struct S_TftpGui *pTftpGui;
    // search mathing internal structure
    for ( pTftpGui = pTftpGuiFirst;
         pTftpGui != NULL && pTftpGui->dwTransferId != pTrf->dwTransferId;
         pTftpGui = pTftpGui->next);

    if (pTftpGui == NULL)
        return -1;

    ASSERT (pTftpGui != NULL);

    pTftpGui->stat = pTrf->stat;
    time (& pTftpGui->stat.dLastUpdate) ;

    // do not update gauge window if last update has been done in the current second
    // NB: another feature is to avoid division by 0
    if (pTftpGui->stat.dLastUpdate == dNow)
        return -1;

    ((CProgressCtrl *)GetDlgItem(IDC_TRF_PROGRESS))->SetPos(pTftpGui->stat.dwTotalBytes/(pTftpGui->stat.dwTransferSize/100));

    // Update stat text
    char szTitle [_MAX_PATH+sizeof " from 255.255.255.255 "];
    wsprintf (szTitle, "%d Bytes %s \t %d Bytes/sec",
              pTftpGui->stat.dwTotalBytes,
              (pTftpGui->opcode == TFTP_RRQ) ? "sent" : "rcvd",
              pTftpGui->stat.dwTotalBytes / (dNow-pTftpGui->stat.StartTime) );

    //   SetWindowText (GetDlgItem (hMainWnd, IDC_FILE_STATS), szTitle);
    //GetDlgItem(IDC_FILE_STATS)->SetWindowText(szTitle);
    //GetDlgItem(IDC_FILE_SIZE)->SetWindowText(pTftpGui->filename);
    return 0;
} // TFTPStat

LRESULT CDownloadDlg::OnMessageTftpInfo(WPARAM wParam, LPARAM lParam) {
    switch(wParam) {
    case C_TFTP_TRF_STAT:
        {
            struct S_TftpTrfStat *trf_stat = (struct S_TftpTrfStat *)lParam;
            for (int Ark=0; Ark < trf_stat->nbTrf; Ark++)
                TFTPStat(& trf_stat->t[Ark], trf_stat->dNow);
            TFTPReporting( pTftpGuiFirst);
        }
        break;
    case C_TFTP_TRF_END:
        {
            TFTPEnd((struct S_TftpTrfEnd *)lParam);
        }
        break;
    case C_TFTP_TRF_NEW:
        TFTPNew((struct S_TftpTrfNew *)lParam);
        break;
    case C_TFTP_TRF_ERROR: {
        struct S_TftpError *gui_msg = (struct S_TftpError*) lParam;
        if (gui_msg->errorCode == ENOTFOUND) {
            CString msg = gui_msg->detail;
            msg += " is not exist, please check your update package.";
            MessageBeep(MB_ICONERROR);
            UpdateMessage(msg);
            //::MessageBox(NULL, msg, _T("File no exist"), MB_OK | MB_ICONHAND);
        } else if (gui_msg->errorCode == EACCESS) {
            MessageBeep(MB_ICONERROR);
            UpdateMessage(gui_msg->detail);
            //::MessageBox(NULL, gui_msg->detail, _T("Permission Error"), MB_OK | MB_ICONHAND);
        }
        }
        break;
    default:
        break;
    }
    //    Invalidate();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
// Reporting into LV_LOG list View
//
////////////////////////////////////////////////////////////////////////////////////////
static int TFTPItemAdd (CListCtrl* listView, const struct S_TftpGui *pTftpGui, int Pos)
{
    LVITEM      LvItem;
    char        szTxt [512] = {0}, szAddr[MAXLEN_IPv6]={0}, szServ[NI_MAXSERV] ={0};
    int         itemPos;
    struct tm   ltime;
    char		cDel;

    //LvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    LvItem.mask = LVIF_PARAM | LVIF_STATE;
    LvItem.state = 0;
    LvItem.stateMask = 0;
    LvItem.iItem = Pos;      // num閞o de l'item
    LvItem.lParam = (LPARAM) pTftpGui->dwTransferId;    // for Right-Click actions
    LvItem.iSubItem = 0;     // index dans la ligne
    // LvItem.pszText = "";
    //itemPos = ListView_InsertItem (hListV, & LvItem);
    itemPos = listView->InsertItem(&LvItem);

    getnameinfo ( (LPSOCKADDR) & pTftpGui->stg_addr, sizeof (pTftpGui->stg_addr),
                 szAddr, sizeof szAddr,
                 szServ, sizeof szServ,
                 NI_NUMERICHOST | AI_NUMERICSERV );

    wsprintf (szTxt, "%s:%s", szAddr, szServ);
    LOGD ("CREATING item <%s>", szTxt);
    //ListView_SetItemText (hListV, itemPos, FD_PEER, szTxt);
    listView->SetItemText (itemPos, FD_PEER, szTxt);
#ifdef _MSC_VER
    localtime_s (&ltime, & pTftpGui->stat.StartTime);
#else
    memcpy (& ltime, localtime (& pTftpGui->stat.StartTime), sizeof ltime);
#endif

    wsprintf (szTxt, "%02d:%02d:%02d", ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
    listView->SetItemText (itemPos, FD_START, szTxt);
    cDel = pTftpGui->opcode == TFTP_RRQ ? '<' : '>';
    wsprintf (szTxt, "%c%s%c", cDel,pTftpGui->filename, cDel );

    listView->SetItemText (itemPos, FD_FILE, szTxt );
    return   itemPos;
}

static int TFTPItemUpdate (HWND hListV, const struct S_TftpGui *pTftpGui, int itemPos)
{
    char szTxt [512];

    lstrcpy (szTxt, "N/A");
    switch (pTftpGui->stat.ret_code)
    {
    case TFTP_TRF_RUNNING :
        if (pTftpGui->stat.dwTransferSize > 100)
            wsprintf (szTxt, "%d%%", pTftpGui->stat.dwTotalBytes/(pTftpGui->stat.dwTransferSize/100));
        break;
    case TFTP_TRF_SUCCESS :
        lstrcpy (szTxt, "100%");
        break;
    case TFTP_TRF_STOPPED :
        lstrcpy (szTxt, "STPD");
        break;
    case TFTP_TRF_ERROR :
        lstrcpy (szTxt, "ERR");
        break;
    }
    ListView_SetItemText (hListV, itemPos, FD_PROGRESS, szTxt);
    wsprintf (szTxt, "%d", pTftpGui->stat.dwTotalBytes);
    ListView_SetItemText (hListV, itemPos, FD_BYTES, szTxt);
    wsprintf (szTxt, "%d", pTftpGui->stat.dwTransferSize);
    ListView_SetItemText (hListV, itemPos, FD_TOTAL,
                          pTftpGui->stat.dwTransferSize==0 ? "unknown" : szTxt);
    wsprintf (szTxt, "%d", pTftpGui->stat.dwTotalTimeOut);
    ListView_SetItemText (hListV, itemPos, FD_TIMEOUT, szTxt);
    ListView_SetItemState(hListV, itemPos, LVIS_SELECTED,LVIS_SELECTED);
    return TRUE;
} // TFTPItemUpdate


static int TFTPManageTerminatedTransfers (HWND hListV, int itemPos)
{
    char szTxt [512] = {0};
//    LVITEM      LvItem;
    int  tNow  = (int) time(NULL);

    ListView_GetItemText (hListV, itemPos, FD_PROGRESS, szTxt, sizeof szTxt -1 );

    if (strcmp(szTxt, "100%") != 0)
    {
#if 0
        // Put in param the times before deletion
        LvItem.iSubItem =FD_PEER;
        LvItem.mask = LVIF_PARAM;
        LvItem.iItem = itemPos;
        if (ListView_GetItem (hListV, & LvItem) )
        {
            LvItem.lParam = sSettings.nGuiRemanence + tNow;
            ListView_SetItem (hListV, & LvItem) ;
        }
#endif

        ListView_SetItemText (hListV, itemPos, FD_PROGRESS, "100%");
        ListView_GetItemText (hListV, itemPos, FD_TOTAL, szTxt, sizeof szTxt -1 );
        ListView_SetItemText (hListV, itemPos, FD_BYTES, szTxt);
    } // transfer not already marked as termnated
    else
    {
#if 0
        LvItem.iSubItem =FD_PEER;
        LvItem.mask = LVIF_PARAM;
        LvItem.iItem = itemPos;
        if (ListView_GetItem (hListV, & LvItem) && LvItem.lParam < tNow)
        {
           ListView_DeleteItem (hListV, itemPos);
        }
#endif
    }
    return TRUE;
} // TFTPManageTerminatedTransfers

///////////////////////////////////////////////
// populate listview
// called each new transfer or each end of transfer
int CDownloadDlg::TFTPReporting (const struct S_TftpGui *pTftpGuiFirst)
{
    LVFINDINFO  LvInfo;
    int itemPos;
    const struct S_TftpGui *pTftpGui;
    int    Ark;         // date of entry
    short  tPos [512];

    HWND hListV= GetDlgItem (IDC_LV_TFTP)->GetSafeHwnd();

    // ListView_DeleteAllItems (hListV);
    memset (tPos, 0, sizeof tPos);

    for (Ark=0, pTftpGui = pTftpGuiFirst; pTftpGui != NULL; pTftpGui = pTftpGui->next, Ark++)
    {
        // search peer field (key)
        LvInfo.flags = LVFI_PARAM;
        LvInfo.lParam = pTftpGui->dwTransferId;
        itemPos = ListView_FindItem (hListV, -1, & LvInfo);
        BOOL add = (itemPos==-1);

        // item has not been found --> should be inserted
        if (add)
        {
            itemPos = TFTPItemAdd (&m_TransferFileList, pTftpGui, Ark);
            m_TransferFileList.SetSelectionMark(itemPos);
        } // create transfers
        m_pCoordinator->RefreshFirmwareTransfer(pTftpGui->stg_addr,
                pTftpGui->filename, add);
        // actualize fields
        TFTPItemUpdate (hListV, pTftpGui, itemPos);
        // flag : ths item has been processed
        if (itemPos < SizeOfTab(tPos)) tPos [itemPos] = 1;	// flag item
    }

    // manage item that are not on the stat record --> they are terminated
    for (Ark = 0; Ark < ListView_GetItemCount(hListV); Ark++) {
        if (Ark < SizeOfTab(tPos) && tPos[Ark] == 0) {
            TFTPManageTerminatedTransfers (hListV, Ark);
            }
        }
    return Ark;
}

void CDownloadDlg::ClearMessage(void) {
    m_LogText.Empty();
    UpdateMessage(m_LogText);
}

void CDownloadDlg::UpdateMessage(CString errormsg){
    CString msg;
    int nLen;
    SYSTEMTIME time;
    GetLocalTime(&time);
    CString timestamp;
    timestamp.Format("%02d:%02d:%02d ", time.wHour, time.wMinute,time.wSecond);
    if (errormsg.GetLength() == 0) {
        m_MessageControl.Clear();
        return;
    }
    m_LogText += timestamp;
    m_LogText += errormsg;
//    m_LogText = errormsg;
    if (m_LogText.GetLength() != 0) {
        m_LogText += "\r\n";
    }

    SLOGD(errormsg.GetString());
    //::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message, msg);
    nLen = m_MessageControl.GetWindowTextLength();
    m_MessageControl.SetWindowText(m_LogText);
    m_MessageControl.SetSel(nLen, nLen);//(0, 0);
    m_MessageControl.LineScroll(m_MessageControl.GetLineCount());
    m_MessageControl.SetFocus();
}

VOID CDownloadDlg::SetPtsText(LPCTSTR lpszString) {
    ASSERT(lpszString != NULL);
    m_PSTStatus.SetWindowText(lpszString);
    UpdateMessage(lpszString);
}
VOID CDownloadDlg::SetInformation(int type, LPCTSTR lpszString) {
    ASSERT(lpszString != NULL);
    switch(type) {
    case DEV_FW_VERSION:
        m_DeviceFWVersion.SetWindowText(lpszString);
        LOGD("device firmware is %s ", lpszString);
        break;
    case DEV_OS_VERSION:
        m_DeviceOSVersion.SetWindowText(lpszString);
        break;
    case DEV_IP_ADDR:
        m_DeviceIpAddress.SetWindowText(lpszString);
        break;
    case PTS_STATUS:
        SetPtsText(lpszString);
        break;
    case HOST_NIC: {
            NetCardStruct nic = mNic.GetDefaultNic();
            CString nicInfo;
            nicInfo.Format("%s, %s ", nic.mConnectionName.c_str(), nic.mIPAddress.c_str());
            m_NicInformation.SetWindowText(nicInfo.GetString());
            UpdateMessage(nicInfo);
        }
        break;

    default:
        break;
    }
}

void CDownloadDlg::OnBnClickedDisableCheck()
{
    if (is_downloading) {
        ::MessageBox(NULL,
                     _T("Please stop download first."),
                     _T("Information"),
                     MB_OK | MB_ICONINFORMATION);
        return;
    }

    if (m_pCoordinator->GetSuperMode()) {
        int result = ::MessageBox(NULL,
                                  _T("Enbale version check?"),
                                  _T("Information"),
                                  MB_OKCANCEL | MB_ICONQUESTION | MB_TASKMODAL);
        if (result == IDOK) {
            SetIcon(m_hIcon, TRUE);
            SetIcon(m_hIcon, FALSE);
            m_pCoordinator->SetSuperMode(FALSE);
            m_VersionCheckButton.SetIcon(AfxGetApp()->LoadIcon(IDI_LOCK));
            SetWindowText(m_DialgoTitle.GetString());
        }
        return;
    }

    PasswordEnterDlg m_PasswordEnterDlg;
    //    m_PasswordEnterDlg.ShowWindow(SW_SHOW);
    INT_PTR nResponse = m_PasswordEnterDlg.DoModal();
    if (nResponse == IDOK) {
        m_pCoordinator->SetSuperMode(TRUE);
        CString title = m_DialgoTitle + ", Super Mode (Version Check DISABLE)";
        //GetWindowText(title);
        SetWindowText(title.GetString());
        m_VersionCheckButton.SetIcon(AfxGetApp()->LoadIcon(IDI_UNLOCK));

        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SUPERMODE));
        //        SendMessage(WM_SETICON, ICON_BIG, (LPARAM)hicon);
        SetIcon(hIcon, TRUE);
        SetIcon(hIcon, FALSE);
    } else if (nResponse == IDCANCEL) {
    }
}


void CDownloadDlg::OnBnClickedChangeNic()
{
    NicListDlg dlg;
    dlg.SetNicManager(mNic);
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK) {
      SetInformation(HOST_NIC, "");
    } else if (nResponse == IDCANCEL) {
    }
}


void CDownloadDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default

    mNic.RestoreDefaultNic();
    CDialogEx::OnClose();
}


void CDownloadDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    mNic.RestoreDefaultNic();
    CDialogEx::OnCancel();
}
