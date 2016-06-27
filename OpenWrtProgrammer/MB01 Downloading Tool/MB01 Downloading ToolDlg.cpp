
// MB01 Downloading ToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MB01 Downloading Tool.h"
#include "MB01 Downloading ToolDlg.h"
#include "afxdialogex.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//const int software_size=50000000;
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


// CMB01DownloadingToolDlg dialog




CMB01DownloadingToolDlg::CMB01DownloadingToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMB01DownloadingToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	strFile = _T(""); 
	server_state=false;
	error_message="";
	Progress_range=350;
	is_downloading=false;
	downloading_successfull=false;
}

CMB01DownloadingToolDlg::~CMB01DownloadingToolDlg(void)
{

}
void CMB01DownloadingToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESS1, m_progMac2);
}

BEGIN_MESSAGE_MAP(CMB01DownloadingToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Browse, &CMB01DownloadingToolDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(ID_Start, &CMB01DownloadingToolDlg::OnBnClickedStart)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CMB01DownloadingToolDlg message handlers

BOOL CMB01DownloadingToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString TemStr("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
	char	s_Label[50];
	WCHAR 	WS_Label[50]={0};
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
	

	((CButton*)GetDlgItem(IDC_CHECKTRACE))->SetCheck(TRUE);
	
	//GetPrivateProfileString((LPCWSTR)"MISC", (LPCWSTR)"CURef", (LPCWSTR)"", (LPWSTR)TemStr.GetBuffer(20),20,(LPCWSTR)"C:\\HDT_MUSICBOX\\Product\\Config.ini");
	GetPrivateProfileString(TEXT("MISC"), TEXT("CURef"), TEXT(""), (LPWSTR)TemStr.GetBuffer(14),14,TEXT("C:\\HDT_MUSICBOX\\Product\\Config.ini"));
	if(TemStr != "")
	{
		WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)TemStr, -1,(LPSTR)s_CommercialRef, 14, NULL, FALSE);  
	}
	if(strlen(s_CommercialRef)  < 14 && strlen(s_CommercialRef)  > 10)
	{
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_CU,(LPCWSTR)TemStr);
	}
	TemStr.ReleaseBuffer();
	GetPrivateProfileString(TEXT("MISC"), TEXT("PTS"), TEXT(""), (LPWSTR)TemStr.GetBuffer(4),4,TEXT("C:\\HDT_MUSICBOX\\Product\\Config.ini"));
	if(TemStr != "")
	{
		WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)TemStr, -1,(LPSTR)s_PTS_new, 4, NULL, FALSE);  
		s_PTS_new[3] = 0;
	}
	TemStr.ReleaseBuffer();
	GetPrivateProfileString(TEXT("MISC"), TEXT("SSIDPrefix"), TEXT(""), (LPWSTR)TemStr.GetBuffer(20),20,TEXT("C:\\HDT_MUSICBOX\\Product\\Config.ini"));
	if(TemStr != "")
	{
		WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)TemStr, -1,(LPSTR)s_SSID_Prefix, 20, NULL, FALSE);  
		s_SSID_Prefix[strlen(s_SSID_Prefix)] = 0;
	}
	TemStr.ReleaseBuffer();

	strcpy(s_Label,"C:\\Label\\");
	int len = MultiByteToWideChar(CP_ACP,0,s_Label,strlen(s_Label),NULL,0);   
	MultiByteToWideChar(CP_ACP,0,s_Label,strlen(s_Label),WS_Label,len);
	if(!PathFileExists(WS_Label))
	{
		CreateDirectory(WS_Label, NULL);
	}
   //WIFI_NAME_Thread= CreateThread(NULL,0,Thread_Check_Wifi,this,0,&WIFI_NAME_Thread_ID);
   GetDlgItem(ID_Start)->EnableWindow(TRUE); 
	return TRUE;  // return TRUE  unless you set the focus to a control
}
DWORD WINAPI CMB01DownloadingToolDlg::Thread_Server_Listen(LPVOID lpPARAM)
{

	CMB01DownloadingToolDlg *pThis = (CMB01DownloadingToolDlg *)lpPARAM;
	pThis->server_listen();
	return 1;
}
void CMB01DownloadingToolDlg::server_listen()
 {
	__int64 iResult;
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD( 2, 2);
	iResult = WSAStartup( wVersionRequested, &wsaData );
	if ( iResult != NO_ERROR )
	{
		server_state=false;
		error_message+=_T("Server Startup failed!\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		return ;
	}
	char* content = NULL;
	__int64  nLen = 0;
	
	
	FILE *pF  = fopen(file_name, "rb" );
	if(pF==NULL) 
	{
		//perror(file_name);
		error_message+=_T("Open file failed!\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		return ;
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
	const __int64 length=header.size()+nLen+1;
	rewind(pF);
	content= (char*) malloc(sizeof(char)*length);
	if(!content) 
	{
		error_message+=_T("Failed to allocate memory!\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		return ;
	}
	memset(content, 0, length);
	memcpy(content, header.c_str(), header.size());
	//char ch;
	int len = header.size();
	
	 fread(content + len, 1, nLen, pF);
	 content[length]='\0';
	fclose(pF);
	//printf("Header length is %d\n", header.size());
	//printf("Header is \n%s\n", header.c_str());
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockSrv == INVALID_SOCKET) 
	{
		wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
		error_message+=_T("Create server socket failed!\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		WSACleanup();
		free(content);
		return ;
	}
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(80);
	iResult = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (iResult == SOCKET_ERROR) 
	{
		
		error_message+=_T("Bind socket failed!\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		free(content);
		closesocket(sockSrv);
		WSACleanup();
		
		return ;
	}
	error_message+=_T("bind socket success\r\n");
	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
	Line_edit->SetSel(0,-1);     
	Line_edit->SetFocus(); 
	if (listen(sockSrv, SOMAXCONN) == SOCKET_ERROR) 
	{
		
		error_message+=_T("server listen failed\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		free(content);
		closesocket(sockSrv);
		WSACleanup();
		
		return ;
	}
	
	error_message+=_T("Listening on socket...\r\n");
	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
	Line_edit->SetSel(0,-1);     
	Line_edit->SetFocus(); 
	SOCKADDR_IN  addrClient;
	exitSocket = false;
	while(true)
	{
		if(exitSocket)
		{
			break;
		}
		int sin_size = sizeof(struct sockaddr_in); 
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &sin_size);
		error_message+=_T("Send softwre ..., please wait...\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		unsigned long on = 1;
		ioctlsocket(sockConn, FIONBIO, &on);
		char recvBuf[101]={0};
		//char recvBuf1[101]={0};
		string IsSend;
		int i=0;
		
		int bytes ;
		do 
		{
			memset(recvBuf, 0, 101);
			bytes  = recv(sockConn, recvBuf, 100, 0);
		}while(bytes  > 0);
		//IsSend=recvBuf1;
		printf("\n");

		iResult = send(sockConn, content , length , 0);
		
		if (iResult == SOCKET_ERROR) 
		{
			error_message+=_T("Send software failed!\r\n");
			::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			Line_edit->SetSel(0,-1);     
			Line_edit->SetFocus(); 
			free(content);
			closesocket(sockConn);
			closesocket(sockSrv);
			WSACleanup();
			
			return ;
		}
		printf("xxxxxxxxxxxxxxxx Send %d byte\r\n", iResult);
		//memset(recvBuf, 0, 100);
		
		
		do 
		{
			memset(recvBuf, 0, 101);
			bytes  = recv(sockConn, recvBuf, 100, 0);
			printf("%s", recvBuf);
		}while(bytes  > 0);
		error_message+=_T("Send softwre successfully!\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 

		closesocket(sockConn);
	}
	closesocket(sockSrv);
	free(content);
	WSACleanup();
	
	return ;
}



void CMB01DownloadingToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMB01DownloadingToolDlg::OnPaint()
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
HCURSOR CMB01DownloadingToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI CMB01DownloadingToolDlg::Thread_Check_Wifi(LPVOID lpPARAM)
{
	CMB01DownloadingToolDlg *pThis = (CMB01DownloadingToolDlg *)lpPARAM;
	pThis->Check_Wifi();
	return 1;
}

void CMB01DownloadingToolDlg::Check_Wifi()
{	
	WCHAR WIFI_Name[256]={0};
	string ST_Port_number;
	bool get_wifi_name = false;
	bool get_trace  = false;
	bool check_trace = false;
	int len = 0;
	char tem[20]={0};
	char s_errormessage[256];
	char s_PTS_Local[4]={0};
	char s_New_Trace[50];
		
	dwBeginTime = ::GetTickCount();
	memset(s_PCBNo, 0, 16);
	memset(s_WIFI, 0, 13);
		
	while(true)
	{
		if(is_downloading==false )
		{
			get_trace = wifi_connecting.get_Trace(s_Trace);
			if(get_trace)
			{
				check_trace = ((CButton*)GetDlgItem(IDC_CHECKTRACE))->GetCheck();
				if(check_trace)
				{
					if(strlen(s_Trace) < 16)
					{
						sprintf(s_errormessage,"The length of trace:%d is invalid!",strlen(s_Trace));
						::MessageBoxA(NULL,s_errormessage,"MBO1",MB_SYSTEMMODAL);
						GetDlgItem(ID_Start)->EnableWindow(true);
						break;
					}
					strncpy(s_PCBNo, s_Trace, 15);
					s_PCBNo[15] = 0;
					strncpy(s_MMIFlag, s_Trace+15, 1);
					s_MMIFlag[1] = 0;
					if(s_MMIFlag[0] != '1')
					{
						::MessageBoxA(NULL,"MMI test flag is invalid!","MBO1",MB_SYSTEMMODAL);
						break;
					}
					if(strlen(s_Trace) == 32)
					{
						strncpy(s_PTS_Local,s_Trace+28,3);
						s_PTS_Local[3] = 0;
					}
				}
				else
				{
					if(strlen(s_Trace)>15)
					{
						strncpy(s_PCBNo, s_Trace, 15);
						s_PCBNo[15] = 0;
						strncpy(s_MMIFlag, s_Trace+15, 1);
						s_MMIFlag[1] = 0;						
					}
					else
					{
						strcpy(s_PCBNo, "XXXXXXXXXXXXXXX");
					}
				}				
			}
			else
			{
				::MessageBoxA(NULL,"Can't get trace infomation!\n Please try it again","MBO1",MB_SYSTEMMODAL);
				GetDlgItem(ID_Start)->EnableWindow(true);
				break;
			}
			get_wifi_name=wifi_connecting.Get_WIFI_Name(WIFI_Name,ST_Port_number);
			if(get_wifi_name)
			{				
				if(WIFI_Name!='\0')
				{
					/*if(downloading_successfull)
					{
						if((_tcscmp(Original_Wifi_name,WIFI_Name))==0)
						{
							OnSend_Resset_Comand();
						}
						else
						{
							//::MessageBox(NULL,_T("You cannnot remove the device before reseting!\n Please try it again"),_T("Reset"),MB_OK);
							//CWnd::MessageBox(_T("You cannnot remove the device before reseting!\n Please try it again"));
							::MessageBoxA(NULL,"You cannnot remove the device before reseting!\n Please try it again","MBO1",MB_SYSTEMMODAL);
							downloading_successfull=false;
						}
					}
					else*/
					{
						/*AddSpaceToString(s_PCBNo, 15);
						AddSpaceToString(s_CommercialRef, 13);
						AddSpaceToString(s_MMIFlag, 1);
						AddSpaceToString(s_PTS_new, 3);	
						sprintf(s_New_Trace,"%s%s%s%s",s_PCBNo,s_MMIFlag,s_CommercialRef,s_PTS_new);
						wifi_connecting.set_Trace(s_New_Trace);*/

						WCHAR Prot_Number[256]={0};
						const char* tem=ST_Port_number.c_str();
						int len = MultiByteToWideChar(CP_ACP,0,tem,strlen(tem),NULL,0);   
						wifi_connecting.Get_WIFI_MAC(s_WIFI);
						MultiByteToWideChar(CP_ACP,0,tem,strlen(tem),Prot_Number,len);  
						Prot_Number[len]='\0';
						GetDlgItem(IDC_WIFI_Name)->SetWindowText(WIFI_Name);
						GetDlgItem(IDC_Com_port)->SetWindowText(Prot_Number);
						//::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_WIFI_Name,WIFI_Name);
						//::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Com_port,Prot_Number);
						//GetDlgItem(ID_Start)->EnableWindow(true);
						is_downloading = true;
						_tcscpy(Original_Wifi_name,WIFI_Name);
						Send_Comand_Thread= CreateThread(NULL,0,Thread_Send_Comand,this,0,&Send_Comand_Thread_ID);
						break;
					}
					
				}				
			}
			else
			{
				//GetDlgItem(ID_Start)->EnableWindow(false);
				::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_WIFI_Name,_T("NO WIFI"));
			}
		}
		//else
			//GetDlgItem(ID_Start)->EnableWindow(false);

		Sleep(1000);

}
	return ;
}



void CMB01DownloadingToolDlg::OnBnClickedButtonBrowse()
{
	 
	 /*CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files (*.sam)|*.sam||"), NULL); 
	 if (dlgFile.DoModal()) 
	 { 
		 strFile = dlgFile.GetPathName(); 
	 } 
	 int len=strFile.GetLength()+10;   
	 memset(file_name,0,len);
	 WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)strFile, -1,NULL, 0, NULL, FALSE);
	 WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)strFile, -1,(LPSTR)file_name, len, NULL, FALSE);   
	 file_name[len]='\0';
	 
	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Browse,strFile);*/
	strFile = "C:\\HDT_MUSICBOX\\Product\\firmware.sam";
	if(!PathFileExists(strFile))
	{
		::MessageBox(NULL,_T("Can't find C:\\HDT_MUSICBOX\\Product\\firmware.sam!"),_T("Check Fireware file"),MB_OK);
	}
	int len=strFile.GetLength()+10;
	memset(file_name,0,len);
	WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)strFile, -1,NULL, 0, NULL, FALSE);
	WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)strFile, -1,(LPSTR)file_name, len, NULL, FALSE);   
	file_name[len]='\0';

	if(strFile!="" )
	{
		Server_Listen_Thread=CreateThread(NULL,0,Thread_Server_Listen,this,0,&Server_Listen_Thread_ID);
		GetDlgItem(IDC_BUTTON_Browse)->EnableWindow(false);
	}
	
}

void CMB01DownloadingToolDlg::OnBnClickedStart()
{
	CString S_Commercial;
	m_progMac2.SetPos(0);
	m_progMac2.SetBarColor(RGB(255,255,0));
	m_progMac2.SetWindowText(_T(" "));
	m_progMac2.Invalidate(FALSE);
	if(strFile=="" )
	{
		 ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
		 return;	 
	}
	memset(s_CommercialRef, 0, 14);
	 CEdit *edit1=(CEdit*)GetDlgItem(IDC_CU);
	 edit1->GetWindowText(S_Commercial);
	if(S_Commercial.GetLength()  > 20 || S_Commercial.GetLength()  < 10)
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
	WIFI_NAME_Thread= CreateThread(NULL,0,Thread_Check_Wifi,this,0,&WIFI_NAME_Thread_ID);
	//while(is_downloading == false)
	{
		Sleep(1000);
	}
	//Send_Comand_Thread= CreateThread(NULL,0,Thread_Send_Comand,this,0,&Send_Comand_Thread_ID);
}

DWORD WINAPI CMB01DownloadingToolDlg::Thread_Send_Comand(LPVOID lpPARAM)
	{

	CMB01DownloadingToolDlg *pThis = (CMB01DownloadingToolDlg *)lpPARAM;
	pThis->OnSend_Comand();
	return 1;
}

void CMB01DownloadingToolDlg::OnSend_Comand()
{
	char s_SN[16]={0};
	char	s_SSID[51];
	char MAC_label[20]={0};
		
	error_message="";
	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
	Line_edit->SetSel(0,-1);     
	Line_edit->SetFocus(); 
	if(strFile=="" )
	{
		 ::MessageBox(NULL,_T("Please select software!"),_T("select software"),MB_OK);
		 return;
	 
	}
	GetDlgItem(ID_Start)->EnableWindow(false);
	WORD wVersionRequested;  
	WSADATA wsaData;    
	int err;  
	wVersionRequested = MAKEWORD( 1, 1 );   
	err = WSAStartup( wVersionRequested, &wsaData );  
	if ( err != 0 )
	{  
		error_message+=_T("WSAStartup failed");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		GetDlgItem(ID_Start)->EnableWindow(true);
		::MessageBox(NULL,_T("WSAStartup failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
		is_downloading=false;
		downloading_successfull=false;
		return ;          
	}  
	if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 )
	{  
		
		WSACleanup( );  
		return ;   
	}  

	 
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	 GetIp Get_Ip;
	 const int MaxRetry=10;
    string Ip;
	string Firmware_name=file_name;
	int tmp=Firmware_name.find_last_of("\\");
	Firmware_name=Firmware_name.substr(tmp+1,Firmware_name.length());
	bool b_getLog = false;
	bool b_showLog = false;
	
	bool IsGetIp=false;
	for(int i=0;i<MaxRetry;i++)
	{
		if((Get_Ip.GetAdapter())&&(Get_Ip.IP()!="")&&(Get_Ip.IP()!="0.0.0.0"))
		{
			Ip=Get_Ip.IP();
			IsGetIp=true;
			error_message+=_T("Get Ip successfully!\n");
			::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			Line_edit->SetSel(0,-1);     
			Line_edit->SetFocus(); 
			break;
		}
		Sleep(2000);       
	}
	if(IsGetIp==false)
	{
		error_message+=_T("Get IP failed!\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		GetDlgItem(ID_Start)->EnableWindow(true);
		::MessageBox(NULL,_T("Get IP failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
		is_downloading=false;
		downloading_successfull=false;
		return ;
	}
	string download_comand="update update -u \"http:\/\/"+Ip+"\/"+Firmware_name+"\/\" -f --no-cert-check --no-device-check\r\n\r\n";
	
	const char * Str_download_comand=download_comand.c_str();
	int Comand_length=strlen(Str_download_comand)+1;
	CString cstr(download_comand.data());
	//::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,cstr);
	SOCKADDR_IN addrSrv;  
	addrSrv.sin_addr.S_un.S_addr = inet_addr("172.19.42.1");        
	addrSrv.sin_family = AF_INET;  
	addrSrv.sin_port = htons(23);  
	__int64 iResult;
	//int timeout=50000;
	//iResult=setsockopt(sockClient,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(int));
	//if(SOCKET_ERROR==iResult)
		//return ;
	Sleep(2000);
	iResult = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));  
	if(iResult==SOCKET_ERROR)
	{  
		error_message+=_T("Connect device failed!\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		GetDlgItem(ID_Start)->EnableWindow(true);
		::MessageBox(NULL,_T("Connect device failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
		downloading_successfull=false;
		is_downloading=false;
		WSACleanup( );  
		return ;   
	}
	char recvBuf[101];
	memset(recvBuf, 0, 101);
	recv(sockClient, recvBuf, 100, 0);  
	recvBuf[100]='\0';
	//bool is_set_time_out=SetTimeOut(sockClient, 50000, true); 
	send(sockClient, Str_download_comand, Comand_length, 1); 
	//if(SOCKET_ERROR==iResult)
		//return ;
	
	string Send_result;
	int i=0;
	int bytes ;
	
	do 
	{
		memset(recvBuf, 0, 101);
		bytes  = recv(sockClient, recvBuf, 100, 0);
		recvBuf[100]='\0';
		if(strstr(recvBuf,"root@Qualcomm")!=NULL)
		{
			b_getLog = true;
		}
		if(b_getLog)
		{
			error_message+=recvBuf;
			if(strstr(recvBuf,"Writing data")!=NULL)
			{
				//b_showLog = true;
			}
			if(b_showLog)
			{
				::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
				Line_edit->SetSel(0,-1);     
				Line_edit->SetFocus(); 
			}
		}
		
		i++;
		Send_result+=recvBuf;
		m_progMac2.SetPos(i);
		if((Send_result.find("[processCommand] Processing send"))!=-1)
		{
			//::MessageBox(NULL,_T("Send software finished!, Please wait for the device update and then reset!"),_T("select software"),MB_OK);
			//::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			//for(int j=i;j<Progress_range+200;j++)
			{
				//m_progMac2.SetPos(Progress_range);
				//m_progMac2.Invalidate(FALSE);
			}

			error_message+=_T("send software successfully!\n");
			::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			Line_edit->SetSel(0,-1);     
			Line_edit->SetFocus(); 
			//GetDlgItem(ID_Start)->EnableWindow(true);
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
			get_Wifi_connect();
			//is_downloading=false;
			//downloading_successfull=true;
			//return;
			//closesocket(sockClient);  
			//WSACleanup();  
			break;
		}  
		//else
		if((Send_result.find("[ERROR]"))!=-1)
		{
			error_message+=_T("send software failed!\n");
			::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			Line_edit->SetSel(0,-1);     
			Line_edit->SetFocus(); 
			m_progMac2.SetBarColor(RGB(255,50,50));
			//m_progMac2.SetBarColor(RGB(255,0,0));
			m_progMac2.Invalidate(FALSE);
			//m_progMac2.SetBarColor(RGB(255,0,0));
			GetDlgItem(ID_Start)->EnableWindow(true);
			//is_downloading=false;
			downloading_successfull=false;
			is_downloading=false;
			::MessageBox(NULL,_T("Download failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
						
			//closesocket(sockClient);  
			//WSACleanup();  
			break;
		}
	}while(bytes  > 0);
	   
}  

void CMB01DownloadingToolDlg::OnSend_Resset_Comand()
{
	 
	WORD wVersionRequested;  
	WSADATA wsaData;    
	int err;  
	char s_SN[16]={0};
	bool b_getLog = false;
	char	s_SSID[51];
	char MAC_label[20]={0};
		
	wVersionRequested = MAKEWORD( 1, 1 );   
	err = WSAStartup( wVersionRequested, &wsaData );  
	if ( err != 0 )
	{
		error_message+=_T("WSAStartup failed");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		GetDlgItem(ID_Start)->EnableWindow(true);
		::MessageBox(NULL,_T("WSAStartup failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
		downloading_successfull=false;
		return ;          
	}  
	if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 )
	{  
		GetDlgItem(ID_Start)->EnableWindow(true);
		WSACleanup( );  
		return ;   
	}
	
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	string Reset_comand="EVENT=event_factory_reset \/etc\/statemgr\r\n\r\n";
	
	const char * Str_reset_comand=Reset_comand.c_str();
	int Comand_length=strlen(Str_reset_comand)+1;
	CString cstr(Reset_comand.data());
	
	SOCKADDR_IN addrSrv;  
	addrSrv.sin_addr.S_un.S_addr = inet_addr("172.19.42.1");        
	addrSrv.sin_family = AF_INET;  
	addrSrv.sin_port = htons(23);  
	__int64 iResult;

	Sleep(10000);
	iResult=connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)); 
	if(iResult==SOCKET_ERROR)
	{  
		error_message+=_T("Connect device failed!\r\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		GetDlgItem(ID_Start)->EnableWindow(true);
		::MessageBox(NULL,_T("Connect device failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
		downloading_successfull=false;
		is_downloading=false;
		WSACleanup( );  
		return ;   
	}
	char recvBuf[101];  
	int bytes ;
	string revalue;
	//recv(sockClient, recvBuf, 100, 0);
	do{
		memset(recvBuf, 0, 101);
		bytes  = recv(sockClient, recvBuf, 100, 0);
		if(bytes==SOCKET_ERROR)
		{  
			error_message+=_T("Received data failed!\r\n");
		    ::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		    Line_edit->SetSel(0,-1);     
		    Line_edit->SetFocus(); 
			GetDlgItem(ID_Start)->EnableWindow(true);
			::MessageBox(NULL,_T("Received data failed!, Please remove the device and then try it again!"),_T("Download"),MB_OK);
			downloading_successfull=false;
			is_downloading=false;
			WSACleanup( );  
			return ;   
		}

		recvBuf[100]='\0';
		if(strstr(recvBuf,"root@Qualcomm")!=NULL)
		{
			b_getLog = true;
		}
		if(b_getLog)
		{
			error_message+=recvBuf;
			::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			Line_edit->SetSel(0,-1);     
			Line_edit->SetFocus(); 
		}
		revalue+=recvBuf;
		if(revalue.find("/\#")!=-1)
			break;
	}while(bytes  > 0);   
	
	send(sockClient, Str_reset_comand, Comand_length, 1); 
	
	string Send_result;
	bool reset_ok=false;
	
	for(int i=0;i<5;i++)
	{
		memset(recvBuf, 0, 101);
		bytes  = recv(sockClient, recvBuf, 100, 0);
		recvBuf[100]='\0';
		error_message+=recvBuf;
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		Send_result+=recvBuf;
		if((Send_result.find("/\#"))!=-1)
		{
			//DWORD dwEndTime = ::GetTickCount();
			//DWORD dwSpaceTime = (dwEndTime - dwBeginTime)/1000;
			error_message+=_T("Reset successfully!\n");
			::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
			Line_edit->SetSel(0,-1);     
			Line_edit->SetFocus(); 
				
			m_progMac2.SetPos(Progress_range);
			m_progMac2.SetBarColor(RGB(0,255,0));
			m_progMac2.Invalidate(FALSE);			
			DWORD dwEndTime = ::GetTickCount();
			DWORD dwSpaceTime = (dwEndTime - dwBeginTime)/1000;
			//sprintf(s_SSID,"%s_%s_AJ",s_SSID_Prefix,s_WIFI+6);
			strupr(s_WIFI);
			GaliSNfromWIFI(s_SN,s_WIFI);
			sprintf(MAC_label, "%c%c%s%c%c%s%c%c%s%c%c%s%c%c%s%c%c", 
								s_WIFI[0], s_WIFI[1], ":", s_WIFI[2], s_WIFI[3], ":", 
								s_WIFI[4], s_WIFI[5], ":", s_WIFI[6], s_WIFI[7], ":", 
								s_WIFI[8], s_WIFI[9], ":", s_WIFI[10], s_WIFI[11]); 
			DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,Original_Wifi_name,-1,NULL,0,NULL,FALSE);
			WideCharToMultiByte(CP_OEMCP, NULL, (LPCWSTR)Original_Wifi_name, -1,(LPSTR)s_SSID, dwNum, NULL, FALSE);   
			GenSAV(s_SN,s_CommercialRef,s_PCBNo,s_PTS_new,"","","",0,"HDT",dwSpaceTime,0,MAC_label,s_SSID,"","","");
			GetDlgItem(ID_Start)->EnableWindow(true);
			is_downloading=false;
			downloading_successfull=false;
			reset_ok=true;
			::MessageBoxA(NULL,"Download finished, please remove the device for next!","MBO1",MB_SYSTEMMODAL);
			break;
			//return;
			//closesocket(sockClient);  
			//WSACleanup();  
			//break;
		}
		Sleep(1000);	
	}   
	if(reset_ok==false)
	{
		error_message+=_T("Reset failed!\n");
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
		Line_edit->SetSel(0,-1);     
		Line_edit->SetFocus(); 
		m_progMac2.SetBarColor(RGB(255,50,50));
		GetDlgItem(ID_Start)->EnableWindow(true);
		is_downloading=false;
		downloading_successfull=false;
		::MessageBox(NULL,_T("Reset failed!, Please remove the device and then try it again!"),_T("Reset"),MB_OK);
			//closesocket(sockClient);  
			//WSACleanup();  
			//break;
	}
} 

void CMB01DownloadingToolDlg::OnBnClickedStop()
{
	is_downloading = true;
	error_message="";
	::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_Error_Message,error_message);
	
}

void CMB01DownloadingToolDlg::get_Wifi_connect()
{
	//WCHAR Original_Wifi_name [256]={0};
	
	WCHAR WIFI_Name[256]={0};
	string ST_Port_number;
	bool get_wifi_name = false;
	WCHAR WS_MAC[13]={0};

	int len = MultiByteToWideChar(CP_ACP,0,s_WIFI,strlen(s_WIFI),NULL,0);   
	MultiByteToWideChar(CP_ACP,0,s_WIFI,strlen(s_WIFI),WS_MAC,len);  
	WS_MAC[len]='\0'; 
		
	while(true)
	{
		//if(is_downloading==false )
		{
			get_wifi_name=wifi_connecting.Get_WIFI_Name(WIFI_Name,ST_Port_number,1);
			if(get_wifi_name)
			{				
				if(WIFI_Name!='\0')
				{
					//if(downloading_successfull)
					{
						if(_tcsstr(WIFI_Name,WS_MAC+6)!=NULL)
						{
							_tcscpy(Original_Wifi_name,WIFI_Name);
							OnSend_Resset_Comand();
							break;
						}
					}					
				}
			}
			else
			{
				::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd,IDC_WIFI_Name,_T("NO WIFI"));
			}
		}

		Sleep(10000);

}
	return ;
}

void CMB01DownloadingToolDlg::OnClose()
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