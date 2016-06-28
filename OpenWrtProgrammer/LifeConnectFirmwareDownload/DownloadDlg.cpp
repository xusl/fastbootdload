
// DownloadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Download.h"
#include "DownloadDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOWNLOAD_SERVER_PORT     80
#define DOWNLOAD_SERVER_IP       "172.19.42.1"

// CAboutDlg dialog used for App About


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
	: CDialogEx(CDownloadDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	strFile = _T("");
	server_state=false;
	error_message="";
	Progress_range=350;
	is_downloading=false;
	downloading_successfull=false;
}

void CDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESS1, m_progMac2);
}

BEGIN_MESSAGE_MAP(CDownloadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Browse, &CDownloadDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(ID_Start, &CDownloadDlg::OnBnClickedStart)
	ON_WM_CLOSE()
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

	Line_edit=(CEdit*)GetDlgItem(IDC_Error_Message);
	m_progMac2.SetRange(0,Progress_range);

	m_progMac2.SetBarColor(RGB(255,255,0));
	m_progMac2.SetTextColor(RGB(0,0,0));
	m_progMac2.SetStep(10);
    GetDlgItem(ID_Start)->EnableWindow(TRUE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}
DWORD WINAPI CDownloadDlg::Thread_Server_Listen(LPVOID lpPARAM) {
	CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
	pThis->server_listen();
	return 1;
}
void CDownloadDlg::server_listen()
{
    SOCKET sockConn = INVALID_SOCKET;
    SOCKET sockSrv  = INVALID_SOCKET;
    CString msg;
    size_t  length = 0;
    __int64 iResult;
    WSADATA wsaData;
    iResult = WSAStartup( MAKEWORD( 2, 2), &wsaData );
    if (iResult != NO_ERROR) {
        server_state=false;
        UpdateMessage(_T("Server Startup failed!"));
        return ;
    }
    char const * content = BuildHttpServerResponse(strFile.GetString(), &length);

    if(content == NULL) {
        UpdateMessage(_T("Open file failed!\n"));
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
    addrSrv.sin_port = htons(DOWNLOAD_SERVER_PORT);
    iResult = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if (iResult == SOCKET_ERROR) {
        msg.Format(_T("Bind socket failed!"), DOWNLOAD_SERVER_PORT);
        HandleServerException(msg, sockConn, sockSrv, &content);
        return ;
    }
    msg.Format(_T("bind socket success on port :%d"), DOWNLOAD_SERVER_PORT);
    UpdateMessage(msg);
    if (listen(sockSrv, SOMAXCONN) == SOCKET_ERROR) {
        HandleServerException(_T("server listen failed!"), sockConn, sockSrv, &content);
        return ;
    }

    UpdateMessage(_T("Listening on socket..."));
    SOCKADDR_IN  addrClient;
    exitSocket = false;
    while(true) {
        unsigned long on = 1;
        char recvBuf[101]={0};
        if(exitSocket) {
            break;
        }
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
            return ;
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
    WSACleanup();
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

	 /*CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files (*.sam)|*.sam||"), NULL);
	 if (dlgFile.DoModal())
	 {
		 strFile = dlgFile.GetPathName();
	 }
	 int len=strFile.GetLength()+10;
	 memset(file_name,0,len);

	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Browse,strFile);*/
	strFile = "C:\\HDT_MUSICBOX\\Product\\firmware.sam";
	if(!PathFileExists(strFile))
	{
		::MessageBox(NULL,_T("Can't find C:\\HDT_MUSICBOX\\Product\\firmware.sam!"),_T("Check Fireware file"),MB_OK);
	}

	if(strFile!="" )
	{
		Server_Listen_Thread=CreateThread(NULL,0,Thread_Server_Listen,this,0,&Server_Listen_Thread_ID);
		GetDlgItem(IDC_BUTTON_Browse)->EnableWindow(false);
	}

}

void CDownloadDlg::OnBnClickedStart() {
    CString S_Commercial;
    m_progMac2.SetPos(0);
    m_progMac2.SetBarColor(RGB(255,255,0));
    m_progMac2.SetWindowText(_T(" "));
    m_progMac2.Invalidate(FALSE);
    if(strFile=="") {
        ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
        return;
    }
    memset(s_CommercialRef, 0, 14);
    CEdit *edit1=(CEdit*)GetDlgItem(IDC_CU);
    edit1->GetWindowText(S_Commercial);
    if(S_Commercial.GetLength() > 20 || S_Commercial.GetLength() < 10)
    {
        ::MessageBox(NULL,_T("The commercial is invalid!"),_T("Input Commercial Ref"),MB_OK);
        return;
    }
    for(int i = 0; i < S_Commercial.GetLength();i++)
    {
        s_CommercialRef[i] = S_Commercial[i];
    }
    GetDlgItem(ID_Start)->EnableWindow(false);
    error_message=_T("Search Datacard..., please wait...\r\n");
    ::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
    is_downloading=false;
    downloading_successfull=false;
    //	WIFI_NAME_Thread= CreateThread(NULL,0,Thread_Check_Wifi,this,0,&WIFI_NAME_Thread_ID);
    //while(is_downloading == false)
    {
        Sleep(1000);
    }
    //Send_Comand_Thread= CreateThread(NULL,0,Thread_Send_Comand,this,0,&Send_Comand_Thread_ID);
}
DWORD WINAPI CDownloadDlg::Thread_Send_Comand(LPVOID lpPARAM) {
    CDownloadDlg *pThis = (CDownloadDlg *)lpPARAM;
    pThis->OnSend_Comand();
    return 1;
}

void CDownloadDlg::OnSend_Comand() {
    char s_SN[16]      = {0};
    char s_SSID[51]    = {0};
    char MAC_label[20] = {0};
    SOCKET sockClient = INVALID_SOCKET;

    if(strFile=="" ) {
        ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
        return;
    }
    ClearMessage();

    WSADATA wsaData;
    if ( WSAStartup(MAKEWORD(1, 1), &wsaData ) != 0 ) {
        HandleDownloadException(_T("WSAStartup failed"), sockClient, FALSE);
        return ;
    }
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup( );
        return ;
    }

    GetIp Get_Ip;
    string Ip;
    string Firmware_name=strFile.GetString();
    int tmp=Firmware_name.find_last_of("\\");
    Firmware_name=Firmware_name.substr(tmp+1,Firmware_name.length());

    bool IsGetIp=false;
    for(int i = 0; i< 10; i++) {
        if((Get_Ip.GetAdapter())&&(Get_Ip.IP()!="")&&(Get_Ip.IP()!="0.0.0.0")) {
            Ip=Get_Ip.IP();
            IsGetIp=true;
            UpdateMessage(_T("Get Ip successfully!\n"));
            break;
        }
        Sleep(2000);
    }
    if(IsGetIp==false) {
        HandleDownloadException(_T("Get IP failed!\r\n"), sockClient);
        return ;
    }
    string download_comand="update update -u \"http://"+Ip+"/"+Firmware_name+
                            "/\" -f --no-cert-check --no-device-check\r\n\r\n";

    const char * Str_download_comand=download_comand.c_str();
    int Comand_length=strlen(Str_download_comand)+1;
    //CString cstr(download_comand.data());
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr(DOWNLOAD_SERVER_IP);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(23);
    __int64 iResult;
    sockClient = socket(AF_INET, SOCK_STREAM, 0);
    //int timeout=50000;
    //iResult=setsockopt(sockClient,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(int));
    //if(SOCKET_ERROR==iResult)
    //return ;
    Sleep(2000);
    iResult = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(iResult==SOCKET_ERROR) {
        HandleDownloadException(_T("Connect device failed!"), sockClient);
        return ;
    }
    char recvBuf[101] = {'\0'};
    recv(sockClient, recvBuf, 100, 0);
    //bool is_set_time_out=SetTimeOut(sockClient, 50000, true);
    send(sockClient, Str_download_comand, Comand_length, 1);

    string Send_result;
    int i=0;
    int bytes = 0;

    do {
        memset(recvBuf, '\0', 101);
        bytes = recv(sockClient, recvBuf, 100, 0);
        if(strstr(recvBuf,"root@Qualcomm")!=NULL) {
            if(strstr(recvBuf,"Writing data")!=NULL) {
                UpdateMessage(recvBuf);
            }
        }

        i++;
        Send_result+=recvBuf;
        m_progMac2.SetPos(i);
        //m_progMac2.Invalidate(FALSE);
        if((Send_result.find("[processCommand] Processing send"))!=-1) {
            UpdateMessage(_T("send software successfully!\n"));
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
            UpdateMessage(_T("send software failed!\n"));
            m_progMac2.SetBarColor(RGB(255,50,50));
            m_progMac2.Invalidate(FALSE);
            GetDlgItem(ID_Start)->EnableWindow(true);
            downloading_successfull=false;
            is_downloading=false;
            ::MessageBox(NULL,_T("Download failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
            break;
        }
    }while(bytes  > 0);

    closesocket(sockClient);
    WSACleanup();
}

void CDownloadDlg::HandleDownloadException(CString msg, SOCKET &sock, bool cleanWSA) {
        UpdateMessage(msg);
        GetDlgItem(ID_Start)->EnableWindow(true);
        ::MessageBox(NULL, msg, _T("Download"),MB_OK);
        is_downloading=false;
        downloading_successfull=false;
        if (cleanWSA) {
            if (sock != INVALID_SOCKET)
            closesocket(sock);
            WSACleanup();
        }
}

void CDownloadDlg::ClearMessage(void) {
    error_message.Empty();// = "";
    UpdateMessage(error_message);
    GetDlgItem(ID_Start)->EnableWindow(false);
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
    error_message += "\n";
    error_message += errormsg;
    msg = error_message;
    }
//    ::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message, msg);
    Line_edit->SetWindowText(msg);
	Line_edit->SetSel(0,-1);
	Line_edit->SetFocus();
}

void CDownloadDlg::OnClose()
{
	if(is_downloading==true||downloading_successfull==true)
	{
		::MessageBoxA(NULL,"Downloading... cannot close tool!","MBO1",MB_SYSTEMMODAL);
		return;
	}
	if (::MessageBoxA(NULL,"Sure to exit?","Exit",MB_OKCANCEL|MB_ICONQUESTION|MB_SYSTEMMODAL ) != IDOK)
	{
		return;
	}
	exitSocket = true;
	Sleep(1000);
	CDialogEx::OnClose();
}
