
// DownloadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Download.h"
#include "DownloadDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "log.h"
#include <setupapi.h>
#include <dbt.h>
#include "telnet.h"
#include "tftp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum EVersion {DEV_FW_VERSION, DEV_OS_VERSION};
enum E_fields { FD_PEER, FD_FILE, FD_START, FD_PROGRESS, FD_BYTES, FD_TOTAL, FD_TIMEOUT };

// a transfer terminated but still displayed
#define ZOMBIE_STATUS ' '

struct S_TftpGui {
    // identifier
    DWORD                     dwTransferId;
    int                       opcode;
    char                      *filename;
    SOCKADDR_STORAGE          stg_addr;
    struct S_Trf_Statistics   stat;
    struct S_TftpGui          *next;
};

static struct S_TftpGui *pTftpGuiFirst=NULL;

int Gui_TftpReporting (HWND hListV, const struct S_TftpGui *pTftpGuiFirst);

static void gmt_time_string(char *buf, size_t buf_len, time_t *t)
 {
  strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime(t));
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
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDownloadDlg dialog

CDownloadDlg::CDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDownloadDlg::IDD, pParent),
	mWSAInitialized(FALSE),
	mHostIPAddr(""),
	mHostGWAddr(""),
	m_LogText("")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pCoordinator = new DeviceCoordinator;
    m_SyncSemaphore = CreateSemaphore(NULL, 1, 1, "ThreadSyncDevie");
	server_state=false;
	Progress_range=350;
	is_downloading=false;
	downloading_successfull=false;
	b_download=false;
    m_bSuperMode = FALSE;
    m_Config.ReadConfigIni(CONFIG_FILE);
    StartLogging("lifeconnect-flash.log", "all", "all");
}

CDownloadDlg::~CDownloadDlg() {
    if (mWSAInitialized) {
        WSACleanup();
        mWSAInitialized = FALSE;
    }

    StopLogging();
    m_pCoordinator->Reset();
    delete m_pCoordinator;
    CloseHandle(m_SyncSemaphore);
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
	//DDX_Text(pDX, IDC_EDIT_LINUX_VER_MAIN, m_LinuxVer);
}

BEGIN_MESSAGE_MAP(CDownloadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BUTTON_Browse, &CDownloadDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(ID_Start, &CDownloadDlg::OnBnClickedStart)
    ON_MESSAGE(UI_MESSAGE_TFTPINFO, &CDownloadDlg::OnMessageTftpInfo)
    ON_BN_CLICKED(IDC_DISABLE_CHECK, &CDownloadDlg::OnBnClickedDisableCheck)
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

    CListCtrl* listView = (CListCtrl*)GetDlgItem (IDC_LV_TFTP);

    listView->InsertColumn(FD_PEER,     _T("Peer"), LVCFMT_LEFT, 130);
    listView->InsertColumn(FD_FILE,     _T("File"), LVCFMT_LEFT, 180);
    listView->InsertColumn(FD_START,    _T("Start"), LVCFMT_LEFT, 80);
    listView->InsertColumn(FD_PROGRESS, _T("Progress"), LVCFMT_LEFT, 60);
    listView->InsertColumn(FD_BYTES,    _T("Bytes"), LVCFMT_LEFT, 100);
    listView->InsertColumn(FD_TOTAL,    _T("Total"), LVCFMT_LEFT, 100);
    listView->InsertColumn(FD_TIMEOUT,  _T("Timeout"), LVCFMT_LEFT, 80);


    m_progMac2.SetRange(0,Progress_range);
    m_progMac2.SetBarColor(RGB(255,255,0));
    m_progMac2.SetTextColor(RGB(0,0,0));
    m_progMac2.SetStep(10);

    //GetPrivateProfileString(_T("MISC"), _T("SSIDPrefix"), _T(""), s_SSID_Prefix, 20, config);
    //s_SSID_Prefix[strlen(s_SSID_Prefix)] = 0;

    if(!PathFileExists(WS_LABEL_DIR)) {
        CreateDirectory(WS_LABEL_DIR, NULL);
    }
    GetDlgItem(ID_Start)->EnableWindow(TRUE);
    m_RomPathStaticText.SetWindowText(m_Config.GetPackageDir());
    return TRUE;  // return TRUE  unless you set the focus to a control
}

DWORD WINAPI CDownloadDlg::NetworkSniffer(LPVOID lpPARAM) {
    CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
    pThis->UpdateMessage(_T("Search Life Connect..., please wait...\r\n"));
    pThis->GetHostIpAddr();

    for(;;) {
        pThis->SniffNetwork();
    }
    return 0;
}

void CDownloadDlg::SniffNetwork() {
    CString msg;
    for (int i = 1; i < 2; i++) {
        CString ip_addr;
        const char* pcIpAddr;
        string mac;
        ip_addr.Format("%s.%d", m_Config.GetNetworkSegment(), i);
        pcIpAddr = ip_addr.GetString();
        if (pcIpAddr == mHostIPAddr || pcIpAddr == mHostGWAddr)
            continue;
        if(Ping(pcIpAddr)) {
            if (0 == ResolveIpMac(pcIpAddr, mac)) {
                m_DeviceIpAddress.SetWindowText(pcIpAddr);
                m_pCoordinator->AddDevice(CDevLabel(mac, string(pcIpAddr)) , NULL);
                //				if(mac != mac_old)
                {
                    //mac_old = mac;
                    b_download = false;
                    m_progMac2.SetPos(0);
                    m_progMac2.SetBarColor(RGB(255,255,0));
                    m_progMac2.SetWindowText(_T(" "));
                    m_progMac2.Invalidate(FALSE);
                    LOGD("ping %s succefully, mac :%s", pcIpAddr, mac.c_str());
                    msg.Format(_T("ping %s succefully, mac :%s"), pcIpAddr, mac.c_str());
                    UpdateMessage(msg);
                }
                break;
            }
        } else {
            ResolveIpMac(pcIpAddr, mac);
            LOGD("ping %s failed. mac :%s", pcIpAddr, mac.c_str());
        }
    }
    SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    WaitForSingleObject(m_SyncSemaphore, 10000);//INFINITE);
}

DWORD CDownloadDlg::Schedule() {
    if (!m_pCoordinator->IsEmpty()) {
        Send_Comand_Thread= CreateThread(NULL,0,
                                         Thread_Send_Comand,this,0,&Send_Comand_Thread_ID);
        // WaitForSingleObject(Send_Comand_Thread, INFINITE);
    }
    ReleaseSemaphore(m_SyncSemaphore, 1, NULL);

    //SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    return 0;
}

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

void CDownloadDlg::OnBnClickedButtonBrowse() {
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
}

void CDownloadDlg::OnBnClickedStart() {
    if(is_downloading) {
        LOGW("Tool is working now.");
        if (b_download == false) {
            TerminateThread(m_NetworkSnifferThreadHandle, 1);
            GetDlgItem(ID_Start)->SetWindowText("Start");
            is_downloading = false;
        }
        return;
    }
    m_progMac2.SetPos(0);
    m_progMac2.SetBarColor(RGB(255,255,0));
    m_progMac2.SetWindowText(_T(" "));
    m_progMac2.Invalidate(FALSE);
    //if(mRomPath=="") {
    //     ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
    //     return;
    //}

    GetDlgItem(ID_Start)->SetWindowText("Stop");
    GetDlgItem(IDC_BUTTON_Browse)->EnableWindow(false);
    m_RomPathStaticText.EnableWindow(false);

    ClearMessage();

    is_downloading=TRUE;
    downloading_successfull=false;
    b_download = false;

    //SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    //m_pCoordinator->AddDevice(CDevLabel(string("FC-4D-D4-D2-BA-84"), string("192.168.1.10")) , NULL);
    //Schedule();
    //Server_Listen_Thread=CreateThread(NULL,0,Thread_Server_Listen,this,0,&Server_Listen_Thread_ID);

    m_NetworkSnifferThreadHandle = CreateThread(NULL,0,NetworkSniffer,this,0,&m_NetworkSnifferThreadID);
    _beginthread ( StartTftpd32Services, 0, (void *)GetSafeHwnd());
}

void CDownloadDlg::OnTimer(UINT_PTR nIDEvent) {
    KillTimer(nIDEvent);
    Schedule();
}

DWORD WINAPI CDownloadDlg::Thread_Send_Comand(LPVOID lpPARAM) {
    CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
    CString cmd;
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
    CString msg;
    //if (pThis->BuildUpdateCommand(pThis->mRomPath, cmd)) {
    DeviceCoordinator * dc = pThis->GetDeviceCoodinator();
    if (dc == NULL) {
        LOGE("Can not get available DeviceCoordinator");
        return 1;
    }
    CDevLabel *dev = dc->GetValidDevice();
    if (dev == NULL) {
        LOGE("There is none device in the network.");
        return 2;
    }

    SOCKET sock = pThis->CreateSocket(dev->GetIpAddr().c_str());
    if ( sock != INVALID_SOCKET) {
        telnet tn(sock);
        char buf[BUFSIZE];
        Sleep(5000);
        tn.receive_telnet_data(buf, BUFSIZE);

#ifdef TEST
        tn.send_telnet_data("zen\n", strlen("zen\n")); //send user name
        tn.receive_telnet_data(buf, BUFSIZE);
        tn.send_telnet_data("zen\n", strlen("zen\n")); //send password
        tn.receive_telnet_data(buf, BUFSIZE);

        tn.send_telnet_data("ls /\n", strlen("ls /\n"));//send command 'ls /\r\n'
        tn.receive_telnet_data(buf, BUFSIZE);
        tn.send_telnet_data("echo hello world\n", strlen("echo hello world\n"));//send command 'ls /\r\n'
        tn.receive_telnet_data(buf, BUFSIZE);
        closesocket(sock);
        dc->RemoveDevice(dev);
#elif defined(MMI)
#define COMMAN_UPDATE "send_data 254 0 0 7 0 1 0\n"
#define COMMAN_REBOOT "reboot send_data 254 0 0 5 0 0 0\n"
        unsigned char red[100] = "send_data 254 0 2 3 0 1 0\n";
        unsigned char blue[100] = "send_data 254 0 2 3 0 2 0\n";
        char *ledon = "send_data 254 0 2 3 0 3 0\n";
        char *offled = "send_data 254 0 2 3 0 0 0\n";
        char *keytest = "send_data 254 0 2 4 1 6 0\n";
        char *factmood = "send_data 254 0 0 6 0 1 0\n";
        char *wifitest = "send_data 254 0 3 0 1 0 0\n";
        char *wifitest1 = "wpa_cli -i wlan0 scan_result";
        char *readtrace = "send_data 254 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
        char *writetrace = "send_data 254 0 0 0 0 1 2 3 4 5 6 7 8 9 1 2 3 4 5 6 1";

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
#else //HDT
#define COMMAN_GETTRACE "send_data 254 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
#define COMMAN_GETMAC "send_data 254 0 3 1 1 0 0 0 0 0 0\n"
#define COMMAN_UPDATE "send_data 254 0 0 7 0 1 0\n"
#define COMMAN_REBOOT "send_data 254 0 0 5 0 0 0\n"
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
#endif
    }
    //}
    return 0;
ERROR:
    closesocket(sock);
    //dc->RemoveDevice(dev);
    dc->Reset();
    pThis->m_progMac2.SetPos(pThis->Progress_range);
    pThis->m_progMac2.SetBarColor(RGB(255,50,50));
    pThis->m_progMac2.SetWindowText(msg);
    pThis->m_progMac2.Invalidate(FALSE);
    //pThis->GetDlgItem(ID_Start)->EnableWindow(true);
    pThis->b_download = true;
    return 2;
}

void CDownloadDlg::GetHostIpAddr() {
    GetIp getIp(m_Config.GetNetworkSegment());

    mHostIPAddr.clear();
    mHostGWAddr.clear();
    for(int i = 0; i< 3; i++) {
        if(getIp.GetHostIP(mHostIPAddr, mHostGWAddr)) {
            CString msg;
            msg.Format(_T("Get Ip successfully! IP %s, Gateway %s."),
                mHostIPAddr.c_str(), mHostGWAddr.c_str());
            UpdateMessage(msg);
            return;
        }
        Sleep(2000);
    }
    UpdateMessage(_T("Get IP failed!"));
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
    time_t curtime = time(NULL);
    gmt_time_string(date, sizeof(date), &curtime);
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
            downloading_successfull=true;
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
            downloading_successfull=false;
            ::MessageBoxA(NULL,"Download finished, please remove the device for next!","MBO1",MB_SYSTEMMODAL);*/
            m_progMac2.SetWindowText(_T("Send software finished!, Please wait for the device update and then restart "));
            m_progMac2.Invalidate(FALSE);
            Sleep(30000);
            //TODO:: comment by xusl
            //			get_Wifi_connect();
            //is_downloading=false;
            //downloading_successfull=true;
            break;
        }

        if((Send_result.find("[ERROR]"))!=-1) {
            UpdateMessage(_T("send software failed!"));
            m_progMac2.SetBarColor(RGB(255,50,50));
            m_progMac2.Invalidate(FALSE);
            GetDlgItem(ID_Start)->EnableWindow(true);
            downloading_successfull=false;
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
    if (mWSAInitialized == FALSE) {
        UpdateMessage(_T("Server Startup failed!"));
        return sockClient;
    }

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(ip_addr);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    __int64 iResult;
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
int CDownloadDlg::GuiTFTPNew (const struct S_TftpTrfNew *pTrf)
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

    Gui_TftpReporting (pTftpGuiFirst);

    struct subStats stat;
    stat.stat =pTrf->stat;
    stat.dwTransferId = pTrf->dwTransferId;
    time_t dNow;
    time(&dNow);
    GuiTFTPStat(&stat, dNow);

    return 0;
} // GuiNewTrf

// terminates a transfer
int CDownloadDlg::GuiTFTPEnd (struct S_TftpTrfEnd *pTrf)
{
    struct S_TftpGui *pTftpGui, *pTftpPrev;
    struct subStats stat;
    time_t dNow;

    stat.stat =pTrf->stat;
    stat.dwTransferId = pTrf->dwTransferId;
    time(&dNow);
    GuiTFTPStat(&stat, dNow);

    // search mathing internal structure and get previous member
    for (pTftpPrev=NULL, pTftpGui=pTftpGuiFirst;
         pTftpGui != NULL && pTftpGui->dwTransferId != pTrf->dwTransferId;
         pTftpGui = pTftpGui->next)
        pTftpPrev = pTftpGui;

    // in the service, the GUI may have missed the begining of the transfer
    if (pTftpGui==NULL)
        return 0;

    // detach leaf
    if (pTftpPrev != NULL)
        pTftpPrev->next = pTftpGui->next ;
    else
        pTftpGuiFirst = pTftpGui->next ;

    // now we can play with the leaf : it belongs no more to the linked list
    // update stat
    pTftpGui->stat = pTrf->stat;
    Gui_TftpReporting (pTftpGuiFirst);

    // free allocation
    free (pTftpGui->filename);
    free (pTftpGui);
    LOGD ("GUI: transfer destroyed\n");

    // recall TftpReporting : it will notice the process
    Gui_TftpReporting (pTftpGuiFirst);
    return 0;
} // GuiTFTPEnd


int CDownloadDlg::GuiTFTPStat (struct subStats *pTrf, time_t dNow)
{
    struct S_TftpGui *pTftpGui;
    // search mathing internal structure
    for ( pTftpGui = pTftpGuiFirst;
         pTftpGui != NULL && pTftpGui->dwTransferId != pTrf->dwTransferId;
         pTftpGui = pTftpGui->next);

    if (pTftpGui == NULL)
        return -1;

    assert (pTftpGui != NULL);

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
    GetDlgItem(IDC_FILE_STATS)->SetWindowText(szTitle);
    GetDlgItem(IDC_FILE_SIZE)->SetWindowText(pTftpGui->filename);
    return 0;
} // GuiTFTPStat

LRESULT CDownloadDlg::OnMessageTftpInfo(WPARAM wParam, LPARAM lParam) {
    switch(wParam) {
    case C_TFTP_TRF_STAT:
        {
            struct S_TftpTrfStat *trf_stat = (struct S_TftpTrfStat *)lParam;
            for (int Ark=0; Ark < trf_stat->nbTrf; Ark++)
                GuiTFTPStat (& trf_stat->t[Ark], trf_stat->dNow);
            Gui_TftpReporting ( pTftpGuiFirst);
        }
        //GetSafeHwnd(), m_hWnd
        //            UpdateMessage(_T("TFTP start"));
        break;
    case C_TFTP_TRF_END:
        {
            GuiTFTPEnd ((struct S_TftpTrfEnd *)lParam);
        }
        //        UpdateMessage(_T("TFTP end"));
        break;
    case C_TFTP_TRF_NEW:
        GuiTFTPNew ((struct S_TftpTrfNew *)lParam);
        //            UpdateMessage(_T("TFTP new"));
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
static int AddNewTftpItem (CListCtrl* listView, const struct S_TftpGui *pTftpGui, int Pos)
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
    LvItem.iItem = Pos;      // num�ro de l'item
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
    LOGE ("CREATING item <%s>\n", szTxt);
    //ListView_SetItemText (hListV, itemPos, FD_PEER, szTxt);
    listView->SetItemText (itemPos, FD_PEER, szTxt);
#ifdef _MSC_VER
    localtime_s (&ltime, & pTftpGui->stat.StartTime);
#else
    memcpy (& ltime, localtime (& pTftpGui->stat.StartTime), sizeof ltime);
#endif

    wsprintf (szTxt, "%02d:%02d:%02d", ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
    //ListView_SetItemText (hListV, itemPos, FD_START, szTxt);
    listView->SetItemText (itemPos, FD_START, szTxt);
    cDel = pTftpGui->opcode == TFTP_RRQ ? '<' : '>';
    wsprintf (szTxt, "%c%s%c", cDel,pTftpGui->filename, cDel );

    //ListView_SetItemText (hListV, itemPos, FD_FILE, szTxt );
    listView->SetItemText (itemPos, FD_FILE, szTxt );
    return   itemPos;
}

static int UpdateTftpItem (HWND hListV, const struct S_TftpGui *pTftpGui, int itemPos)
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
    return TRUE;
} // UpdateTftpItem


static int ManageTerminatedTransfers (HWND hListV, int itemPos)
{
    char szTxt [512];
    LVITEM      LvItem;
    int  tNow  = (int) time(NULL);

    szTxt[sizeof szTxt - 1]=0;
    ListView_GetItemText (hListV, itemPos, FD_FILE, szTxt, sizeof szTxt -1 );
    // The '.' is added for terminated transfer
    if (szTxt [0] != ZOMBIE_STATUS)
    {
        // update target name
        szTxt[0] = ZOMBIE_STATUS;
        ListView_SetItemText (hListV, itemPos, FD_FILE, szTxt);
        // Put in param the times before deletion
        LvItem.iSubItem =FD_PEER;
        LvItem.mask = LVIF_PARAM;
        LvItem.iItem = itemPos;
        if (ListView_GetItem (hListV, & LvItem) )
        {
            LvItem.lParam = sSettings.nGuiRemanence + tNow;
            ListView_SetItem (hListV, & LvItem) ;
        }

        ListView_SetItemText (hListV, itemPos, FD_PROGRESS, "100%");
        ListView_GetItemText (hListV, itemPos, FD_TOTAL, szTxt, sizeof szTxt -1 );
        ListView_SetItemText (hListV, itemPos, FD_BYTES, szTxt);
    } // transfer not already marked as termnated
    else
    {
        LvItem.iSubItem =FD_PEER;
        LvItem.mask = LVIF_PARAM;
        LvItem.iItem = itemPos;
        if (ListView_GetItem (hListV, & LvItem) && LvItem.lParam < tNow)
        {
            ListView_DeleteItem (hListV, itemPos);
        }
    }
    return TRUE;
} // ManageTerminatedTransfers

///////////////////////////////////////////////
// populate listview
// called each new transfer or each end of transfer
int CDownloadDlg::Gui_TftpReporting (const struct S_TftpGui *pTftpGuiFirst)
{
    LVFINDINFO  LvInfo;
    int itemPos;
    const struct S_TftpGui *pTftpGui;
    // date of entry
    int    Ark;
    short  tPos [512];

    HWND hListV= GetDlgItem (IDC_LV_TFTP)->GetSafeHwnd();
    CListCtrl* listView = (CListCtrl*)GetDlgItem (IDC_LV_TFTP);

    // ListView_DeleteAllItems (hListV);
    memset (tPos, 0, sizeof tPos);

    for (Ark=0, pTftpGui = pTftpGuiFirst ; pTftpGui != NULL ; pTftpGui = pTftpGui->next, Ark++)
    {
        // search peer field (key)
        LvInfo.flags = LVFI_PARAM;
        LvInfo.lParam = pTftpGui->dwTransferId;
        itemPos = ListView_FindItem (hListV, -1, & LvInfo);

        // item has not been found --> should be inserted
        if (itemPos==-1)
        {
            itemPos = AddNewTftpItem (listView, pTftpGui, Ark);
        } // create transfers
        // actualize fields
        UpdateTftpItem (hListV, pTftpGui, itemPos);
        // flag : ths item has been processed
        if (itemPos < SizeOfTab(tPos)) tPos [itemPos] = 1 ;	// flag item
    }

    // manage item that are not on the stat record --> they are terminated
    for (Ark=ListView_GetItemCount (hListV) - 1 ; Ark>=0 ;  Ark-- )
        if (Ark<SizeOfTab(tPos) &&  tPos[Ark]==0)
            ManageTerminatedTransfers (hListV, Ark) ;
    // ListView_DeleteItem (hListV, Ark);
    return Ark;
} // Reporting

void CDownloadDlg::ClearMessage(void) {
    m_LogText.Empty();// = "";
    UpdateMessage(m_LogText);
    //GetDlgItem(ID_Start)->EnableWindow(false);
}

void CDownloadDlg::UpdateMessage(CString errormsg){
    CString msg;
    if (errormsg.GetLength() == 0) {
        m_MessageControl.Clear();
        return;
    } else if (m_LogText.GetLength() == 0) {
        m_LogText = errormsg;
        msg = m_LogText;
    } else {
        m_LogText += "\r\n";
        m_LogText += errormsg;
        msg = m_LogText;
    }
    LOGE("%s", errormsg.GetString());
    //::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message, msg);
    m_MessageControl.SetWindowText(msg);
    m_MessageControl.SetSel(0, 0);
    m_MessageControl.SetFocus();
}

VOID CDownloadDlg::SetDeviceInformation(int type, LPCTSTR lpszString) {
    switch(type) {
    case DEV_FW_VERSION:
        m_DeviceFWVersion.SetWindowText(lpszString);
        break;
    case DEV_OS_VERSION:
        m_DeviceOSVersion.SetWindowText(lpszString);
        break;
    default:
        break;
    }
}

void CDownloadDlg::OnBnClickedDisableCheck()
{
    if (m_bSuperMode || is_downloading)
        return;

    PasswordEnterDlg m_PasswordEnterDlg;
    //    m_PasswordEnterDlg.ShowWindow(SW_SHOW);
    INT_PTR nResponse = m_PasswordEnterDlg.DoModal();
    if (nResponse == IDOK) {
        CString title;
        GetWindowText(title);
        title = "Super Mode:Version Check DISABLE, " + title;
        SetWindowText(title.GetString());

        HICON icon = AfxGetApp()->LoadIcon(IDI_UNLOCK);
        CButton* versionCheck = (CButton *)GetDlgItem(IDC_DISABLE_CHECK);
        versionCheck->SetIcon(icon);

        m_hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SUPERMODE));
        //        SendMessage(WM_SETICON, ICON_BIG, (LPARAM)hicon);
        SetIcon(m_hIcon, TRUE);			// Set big icon
        SetIcon(m_hIcon, FALSE);		// Set small icon
        m_bSuperMode = TRUE;
    } else if (nResponse == IDCANCEL) {
    }
}
