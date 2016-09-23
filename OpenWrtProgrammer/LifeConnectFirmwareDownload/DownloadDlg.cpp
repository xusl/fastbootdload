
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
#include "settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum E_fields { FD_PEER, FD_FILE, FD_START, FD_PROGRESS, FD_BYTES, FD_TOTAL, FD_TIMEOUT };

// a transfer terminated but still displayed
#define ZOMBIE_STATUS ' '

struct S_TftpGui
{
    // identifier
    DWORD   dwTransferId;
    // items to be displayed
    char   *filename;
    SOCKADDR_STORAGE stg_addr;
    int    opcode;
    // stats
    struct S_Trf_Statistics stat;
    // GUI resources
    //HWND    hGaugeWnd;
    // next
    struct S_TftpGui *next;
};

static struct S_TftpGui *pTftpGuiFirst=NULL;
const struct S_TftpGui *Gui_GetFirstGuiItem (void) { return pTftpGuiFirst; }

int Gui_TftpReporting (HWND hListV, const struct S_TftpGui *pTftpGuiFirst);


char	s_NAPUAP[10][7];
char 	s_Order[21];
int iGetIMEIFromDatabase;
char s_CommercialRef[21];
char s_PTS[4];
bool b_checkTrace;
string mac_old;

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
	mWSAInitialized(FALSE),mHostIPAddr("")
{
    char path_buffer[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char filename[MAX_PATH] = {0};
	SYSTEMTIME time;

//	GetCurrentDirectory(MAX_PATH, currdir);
	GetModuleFileName(NULL, path_buffer, MAX_PATH);
	_splitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, 0, 0, 0, 0);
	mModulePath.Format("%s%s", drive, dir);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pCoordinator = new DeviceCoordinator;
    m_SyncSemaphore = CreateSemaphore(NULL, 1, 1, "ThreadSyncDevie");
	server_state=false;
	error_message="";
	Progress_range=350;
	is_downloading=false;
	downloading_successfull=false;
	b_download=false;
    mRomPath = "cus531-nand-jffs2";
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
    DDX_Control(pDX, IDC_CU, m_CUEdit);
	DDX_Control(pDX, IDC_ORDER, m_ORDEREdit);
    DDX_Control(pDX, IDC_FIRMWARE_IMAGE, m_RomPathStaticText);
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
    // iResult = WSAStartup( MAKEWORD( 2, 2), &wsaData );
    if ( WSAStartup(wVersionRequested, &wsaData ) != 0 ) {
        AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
    } else {
        //if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
            HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)) {
            LOGE("WSAStartup error, version not match");
            WSACleanup( );
        } else {
            mWSAInitialized = TRUE;
        }
    }

    CListCtrl* listView = (CListCtrl*)GetDlgItem (IDC_LV_TFTP);

    listView->InsertColumn(FD_PEER,     _T("Peer"), LVCFMT_LEFT, 130);
    listView->InsertColumn(FD_FILE,     _T("File"), LVCFMT_LEFT, 180);
    listView->InsertColumn(FD_START,    _T("Start"), LVCFMT_LEFT, 80);
    listView->InsertColumn(FD_PROGRESS, _T("Progress"), LVCFMT_LEFT, 60);
    listView->InsertColumn(FD_BYTES,    _T("Bytes"), LVCFMT_LEFT, 100);
    listView->InsertColumn(FD_TOTAL,    _T("Total"), LVCFMT_LEFT, 100);
    listView->InsertColumn(FD_TIMEOUT,  _T("Timeout"), LVCFMT_LEFT, 80);


	Line_edit=(CEdit*)GetDlgItem(IDC_Error_Message);
	m_progMac2.SetRange(0,Progress_range);

	m_progMac2.SetBarColor(RGB(255,255,0));
	m_progMac2.SetTextColor(RGB(0,0,0));
	m_progMac2.SetStep(10);

    CString config = mModulePath + _T("Product\\") + CONFIG_FILE_PATH;
	GetPrivateProfileString(_T("MISC"), _T("CURef"), _T(""), s_CommercialRef,20,config);
	if(strlen(s_CommercialRef) < 20 && strlen(s_CommercialRef) > 10)
	{
        m_CUEdit.SetWindowText(s_CommercialRef);
	}
	GetPrivateProfileString(_T("MISC"), _T("Order"), _T(""), s_Order,20,config);
	if(strlen(s_Order) < 20 && strlen(s_Order) > 3)
	{
        m_ORDEREdit.SetWindowText(s_Order);
	}
	GetPrivateProfileString(_T("MISC"), _T("PTS"), _T(""), s_PTS,3,config);
	if(strlen(s_PTS) != 3)
	{
        strcpy(s_PTS,"");
	}
	GetPrivateProfileString(_T("MISC"), _T("PTS"), _T(""), s_PTS_new, 4, config);
    s_PTS_new[3] = 0;

	//GetPrivateProfileString(_T("MISC"), _T("SSIDPrefix"), _T(""), s_SSID_Prefix, 20, config);
	//s_SSID_Prefix[strlen(s_SSID_Prefix)] = 0;

    GetPrivateProfileString(_T("MISC"), _T("NetworkSegment"), _T("192.168.1"),
                        m_NetworkSegment, IPADDR_BUFFER_LEN,config);

    LOGD("CU REF : %s", s_CommercialRef);
    LOGD("PTS : %s", s_PTS_new);
    LOGD("network segment : %s", m_NetworkSegment);

	int i=-1;
	i = GetPrivateProfileInt("CheckTrace", "CHECKTRACE", 0, "C:\\HDT_LIFECONNECT\\DLoadCfg.ini");
	if(i == 1)
	{
		b_checkTrace = true;
	}
	else
	{
		b_checkTrace = false;
	}
	i=-1;
	i = GetPrivateProfileInt("MISC", "getIMEIFromDatabase", 0, "D:\\HDT\\HDT.ini");
	if(i==1||i == 0)
	{
		iGetIMEIFromDatabase = i;
	}
	else
	{
		iGetIMEIFromDatabase = 0;
	}

	if(iGetIMEIFromDatabase == 1)
	{
		GetPrivateProfileString("BTWifi", "CompanyID", "", s_NAPUAP[0], 7, "D:\\HDT\\HDT.ini");
	}
	else
	{
		if(LoadFactoryBDRange())
		{
			AfxMessageBox("Get WIFI file fail!");
			return false;
		}
	}

	if(!PathFileExists(WS_LABEL_DIR)) {
		CreateDirectory(WS_LABEL_DIR, NULL);
	}
    GetDlgItem(ID_Start)->EnableWindow(TRUE);
    m_RomPathStaticText.SetWindowText(mRomPath);
    GetHostIpAddr();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

 DWORD WINAPI CDownloadDlg::NetworkSniffer(LPVOID lpPARAM) {
    CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
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
        ip_addr.Format("%s.%d", m_NetworkSegment, i);
        pcIpAddr = ip_addr.GetString();
        if (pcIpAddr == mHostIPAddr)
            continue;
        if(Ping(pcIpAddr)) {
            if (0 == ResolveIpMac(pcIpAddr, mac)) {
            m_pCoordinator->AddDevice(CDevLabel(mac, string(pcIpAddr)) , NULL);
				if(mac != mac_old)
				{
					mac_old = mac;
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

void CDownloadDlg::ReleaseThreadSyncSemaphore() {
    ReleaseSemaphore(m_SyncSemaphore, 1, NULL);
}
DWORD CDownloadDlg::Schedule() {
    if (!m_pCoordinator->IsEmpty()) {
    Send_Comand_Thread= CreateThread(NULL,0,
        Thread_Send_Comand,this,0,&Send_Comand_Thread_ID);
   // WaitForSingleObject(Send_Comand_Thread, INFINITE);
    }
    //ReleaseThreadSyncSemaphore();
    ReleaseSemaphore(m_SyncSemaphore, 1, NULL);

    //SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    return 0;
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
    char const * content = BuildHttpServerResponse(mRomPath.GetString(), &length);

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

void CDownloadDlg::OnBnClickedButtonBrowse()
{
    CFileDialog dlgFile(TRUE);
    CString fileName;
    const int c_cMaxFiles = 100;
    const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
    char *p = fileName.GetBuffer(c_cbBuffSize);
    OPENFILENAME& ofn = dlgFile.GetOFN( );
    ofn.Flags |= OFN_ALLOWMULTISELECT;
    ofn.lpstrFilter = _T("Image Files (*.sam;*.uImage;*.jffs2)|*.sam;*.uImage;*.jffs2|") _T("All Files (*.*)|*.*||");
    ofn.lpstrFile = p;
    ofn.nMaxFile = c_cbBuffSize;

    if (dlgFile.DoModal())
    {
        mRomPath = dlgFile.GetPathName();
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
    //::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_BUTTON_Browse,mRomPath);
	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_FIRMWARE_IMAGE,mRomPath);
    if(!PathFileExists(mRomPath)) {
        ::MessageBox(NULL, mRomPath ,_T("Check Fireware file"),MB_OK);
        return;
    }
}

void CDownloadDlg::OnBnClickedStart() {
    CString S_Commercial;
	CString S_OrderLocal;
    m_progMac2.SetPos(0);
    m_progMac2.SetBarColor(RGB(255,255,0));
    m_progMac2.SetWindowText(_T(" "));
    m_progMac2.Invalidate(FALSE);
    if(mRomPath=="") {
        ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
        return;
    }

    ClearMessage();

    memset(s_CommercialRef, 0, 21);
	memset(s_Order, 0, 21);
    //    CEdit *edit1=(CEdit*)GetDlgItem(IDC_CU);
    m_CUEdit.GetWindowText(S_Commercial);
    if(S_Commercial.GetLength() > 20 || S_Commercial.GetLength() < 10) {
        ::MessageBox(NULL,_T("The commercial is invalid!"),_T("Input Commercial Ref"),MB_OK);
        return;
    }
    for(int i = 0; i < S_Commercial.GetLength();i++) {
        s_CommercialRef[i] = S_Commercial[i];
    }
	m_ORDEREdit.GetWindowText(S_OrderLocal);
    if(S_OrderLocal.GetLength() > 20 || S_OrderLocal.GetLength() < 3) {
        ::MessageBox(NULL,_T("The Order is invalid!"),_T("Input Order"),MB_OK);
        return;
    }
    for(int i = 0; i < S_OrderLocal.GetLength();i++)
	{
        s_Order[i] = S_OrderLocal[i];
    }
    GetDlgItem(ID_Start)->EnableWindow(false);
    error_message=_T("Search Datacard..., please wait...\r\n");
    ::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
    is_downloading=false;
    downloading_successfull=false;
	b_download = false;
    m_NetworkSnifferThreadHandle = CreateThread(NULL,0,NetworkSniffer,this,0,&m_NetworkSnifferThreadID);
    //SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
 //m_pCoordinator->AddDevice(CDevLabel(string("FC-4D-D4-D2-BA-84"), string("192.168.1.10")) , NULL);
 //Schedule();
//    Server_Listen_Thread=CreateThread(NULL,0,Thread_Server_Listen,this,0,&Server_Listen_Thread_ID);
	 _beginthread ( StartTftpd32Services, 0, (void *)GetSafeHwnd());

    GetDlgItem(IDC_BUTTON_Browse)->EnableWindow(false);
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
#define COMMAN_REBOOT "reboot send_data 254 0 0 5 0 0 0\n"
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
			if(b_checkTrace)
			{
				if(s_trace[15] != '1')
				{
					msg.Format(_T("Check traceability fail!"));
					goto ERROR;
				}
			}
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
		if(strncmp(s_OldWIFINo,"000000",6) == 0 || strncmp(s_OldWIFINo,s_NAPUAP[0],6)!=0)
		{
			if(iGetIMEIFromDatabase == 1)
				i_ret = GetWIFIfromDatabase(0,s_PCBNo,s_NewWIFINo);
			else
				i_ret = GetBDAddr(s_NewWIFINo);
			if(i_ret)
			{
				closesocket(sock);
		        dc->RemoveDevice(dev);
				msg.Format(_T("Get WIFI MAC fail!"));
				goto ERROR;
			}
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
		else
		{
			strcpy(s_NewWIFINo,s_OldWIFINo);
		}
		GaliSNfromWIFI(s_SN,s_NewWIFINo);

        //i_ret = tn.send_telnet_data(COMMAN_UPDATE, strlen(COMMAN_UPDATE));//send command 'ls /\r\n'
        //i_ret = tn.receive_telnet_data(buf, BUFSIZE);
        //i_ret = tn.send_telnet_data(COMMAN_REBOOT, strlen(COMMAN_REBOOT));//send command 'ls /\r\n'
        //i_ret = tn.receive_telnet_data(buf, BUFSIZE);
		i_ret = GenSAV_NEW(s_SN, s_PCBNo, s_PTS, "", "",
				"", 0, 0, s_NewWIFINo,"", "", "", "",s_memo);
		if(i_ret)
		{
			msg.Format(_T("Generate SAV fail!err message:%s"),s_memo);
			goto ERROR;
		}

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
    GetIp Get_Ip(m_NetworkSegment);
    bool IsGetIp=false;
    mHostIPAddr.clear();
    for(int i = 0; i< 3; i++) {
        if(Get_Ip.GetHostIP(mHostIPAddr)) {
            IsGetIp=true;
            UpdateMessage(_T("Get Ip successfully!"));
            break;
        }
        Sleep(2000);
    }
    if(IsGetIp==false) {
        UpdateMessage(_T("Get IP failed!"));
    }
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

SOCKET CDownloadDlg::CreateSocket(const char *ip_addr,  u_short port) {
    SOCKET sockClient = INVALID_SOCKET;
    if (mWSAInitialized == FALSE) {
        UpdateMessage(_T("Server Startup failed!"));
        return sockClient;
    }

    //CString cstr(download_comand.data());
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
       // HandleDownloadException(_T("Connect device failed!"), sockClient);
       LOGE("Connect device %s failed!", ip_addr);
        sockClient = INVALID_SOCKET;
    }
    return sockClient;
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

void CDownloadDlg::HandleDownloadException(CString msg, SOCKET &sock) {
    UpdateMessage(msg);
    GetDlgItem(ID_Start)->EnableWindow(true);
    ::MessageBox(NULL, msg, _T("Download"),MB_OK);
    is_downloading=false;
    downloading_successfull=false;

    if (sock != INVALID_SOCKET)
        closesocket(sock);
}

// Create a pTftpGui structure and fill it with the msg
int CDownloadDlg::GuiTFTPNew (const struct S_TftpTrfNew *pTrf)
{
    struct S_TftpGui *pTftpGui;
    pTftpGui = (struct S_TftpGui *)calloc (1, sizeof *pTftpGui);
    pTftpGui->dwTransferId = pTrf->dwTransferId;
    pTftpGui->stat = pTrf->stat;
#ifdef MSVC
    pTftpGui->filename = _strdup (pTrf->szFile);
#else
    pTftpGui->filename = strdup (pTrf->szFile);
#endif
    pTftpGui->opcode = pTrf->opcode;
    pTftpGui->stg_addr = pTrf->from_addr;

    pTftpGui->next = pTftpGuiFirst;
    // places the leaf at the head of the structure
    pTftpGuiFirst = pTftpGui;

//GetDlgItem (IDC_LV_TFTP)->m_hWnd,
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
    stat.stat =pTrf->stat;
    stat.dwTransferId = pTrf->dwTransferId;
    time_t dNow;
    time(&dNow);
    GuiTFTPStat(&stat, dNow);

    // search mathing internal structure and get previous member
    for ( pTftpPrev=NULL, pTftpGui=pTftpGuiFirst ;
         pTftpGui != NULL && pTftpGui->dwTransferId != pTrf->dwTransferId ;
         pTftpGui = pTftpGui->next )
        pTftpPrev = pTftpGui;

    // in the service, the GUI may have missed the begining of the transfer
    if (pTftpGui==NULL)  return 0;

    // detach leaf
    if (pTftpPrev != NULL)  pTftpPrev->next = pTftpGui->next ;
    else                    pTftpGuiFirst   = pTftpGui->next ;

    // now we can play with the leaf : it belongs no more to the linked list
    // update stat
    pTftpGui->stat = pTrf->stat;
    //GetDlgItem (IDC_LV_TFTP)->m_hWnd,
    Gui_TftpReporting (pTftpGuiFirst);

    // free allocation
    free (pTftpGui->filename);
    free (pTftpGui);
    LOGD ("GUI: transfer destroyed\n");

    // recall TftpReporting : it will notice the process //GetDlgItem (hMainWnd, IDC_LV_TFTP),
    Gui_TftpReporting (pTftpGuiFirst);
    return 0;
} // GuiTFTPEnd


int CDownloadDlg::GuiTFTPStat (struct subStats *pTrf, time_t dNow)
{
    struct S_TftpGui *pTftpGui;
    // search mathing internal structure
    for ( pTftpGui=pTftpGuiFirst ;
         pTftpGui!=NULL && pTftpGui->dwTransferId != pTrf->dwTransferId ;
         pTftpGui = pTftpGui->next );
    if (pTftpGui == NULL) return -1;
    assert ( pTftpGui != NULL ) ;
    pTftpGui->stat = pTrf->stat;

    time (& pTftpGui->stat.dLastUpdate) ;

    HWND hGWnd;
    char szTitle [_MAX_PATH+sizeof " from 255.255.255.255 "];

    // do not update gauge window if last update has been done in the current second
    // NB: another feature is to avoid division by 0
    if (pTftpGui->stat.dLastUpdate == dNow)  return -1;

    // update progress bar
//    hGWnd = GetDlgItem (hMainWnd, IDC_TRF_PROGRESS);
//    if (pTftpGui->stat.dwTransferSize>100)
//        SendMessage (hGWnd, PBM_SETPOS,
//                     pTftpGui->stat.dwTotalBytes/(pTftpGui->stat.dwTransferSize/100),
//                     0);

    ((CProgressCtrl *)GetDlgItem(IDC_TRF_PROGRESS))->SetPos(pTftpGui->stat.dwTotalBytes/(pTftpGui->stat.dwTransferSize/100));

    // Update stat text
    wsprintf (szTitle, "%d Bytes %s \t %d Bytes/sec",
              pTftpGui->stat.dwTotalBytes,
              (pTftpGui->opcode == TFTP_RRQ) ? "sent" : "rcvd",
              pTftpGui->stat.dwTotalBytes / (dNow-pTftpGui->stat.StartTime) );

 //   SetWindowText (GetDlgItem (hMainWnd, IDC_FILE_STATS), szTitle);
    GetDlgItem(IDC_FILE_STATS)->SetWindowText(szTitle);
    GetDlgItem(IDC_FILE_SIZE)->SetWindowText(pTftpGui->filename);
//    ::Invalidate( hMainWnd );
//    ::UpdateWindow( hMainWnd );
    return 0;
} // GuiTFTPStat

LRESULT CDownloadDlg::OnMessageTftpInfo(WPARAM wParam, LPARAM lParam) {
    switch(wParam) {
    case C_TFTP_TRF_STAT:
        {
            struct S_TftpTrfStat *trf_stat = (struct S_TftpTrfStat *)lParam;
            for (int Ark=0 ; Ark<trf_stat->nbTrf ; Ark++)
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
    LvItem.iItem = Pos;      // numéro de l'item
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
} // static int AddNewTftpItem


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
            // SetTimer (hListV,
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
    error_message.Empty();// = "";
    UpdateMessage(error_message);
    //GetDlgItem(ID_Start)->EnableWindow(false);
}

void CDownloadDlg::UpdateMessage(CString errormsg){
    CString msg;
    if (errormsg.GetLength() == 0) {
        Line_edit->Clear();
        return;
    } else if (error_message.GetLength() == 0) {
        error_message = errormsg;
        msg = error_message;
    } else {
    error_message += "\r\n";
    error_message += errormsg;
    msg = error_message;
    }
    LOGE("%s", errormsg.GetString());
    ::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message, msg);
    Line_edit->SetWindowText(msg);
	Line_edit->SetSel(0,-1);
	Line_edit->SetFocus();
}

void CDownloadDlg::OnClose() {
	if(is_downloading==true) {
		::MessageBoxA(NULL,"Downloading... cannot close tool!","MBO1",MB_SYSTEMMODAL);
		return;
	}
	//if (::MessageBoxA(NULL,"Sure to exit?","Exit",MB_OKCANCEL|MB_ICONQUESTION|MB_SYSTEMMODAL ) != IDOK)
	//{
	//	return;
	//}
	//Sleep(1000);
	CDialogEx::OnClose();
    //ExitThread(m_NetworkSnifferThreadID);
    TerminateThread(m_NetworkSnifferThreadHandle, 1);
}

BOOL CDownloadDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
    if (dwData == 0)
    {
        //LOGD("OnDeviceChange, dwData == 0 .EventType: 0x%x", nEventType);
        return FALSE;
    }

    DEV_BROADCAST_HDR* phdr = (DEV_BROADCAST_HDR*)dwData;
    PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)phdr;

    //DEBUG("OnDeviceChange, EventType: 0x%x, DeviceType 0x%x", nEventType, phdr->dbch_devicetype);

    if (nEventType == DBT_DEVICEARRIVAL) {
        switch( phdr->dbch_devicetype ) {
        case DBT_DEVTYP_DEVNODE:
            LOGW("OnDeviceChange, get DBT_DEVTYP_DEVNODE");
            break;

        case DBT_DEVTYP_PORT:
            {
                LOGI("device arrive, DBT_DEVTYP_PORT");
                break;
            }
        }
    } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
        switch (phdr->dbch_devicetype) {
        case DBT_DEVTYP_PORT:
            {
                LOGI("device removed, DBT_DEVTYP_PORT");
                break;
            }
        }
    }

    return TRUE;
}
