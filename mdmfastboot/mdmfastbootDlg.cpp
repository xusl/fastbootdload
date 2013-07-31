// mdmfastbootDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "log.h"
#include "mdmfastboot.h"
#include "mdmfastbootDlg.h"
#include "usb_adb.h"
#include "usb_vendors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")

static const GUID usb_class_id[] = {
	//ANDROID_USB_CLASS_ID, adb and fastboot
	{0xf72fe0d4, 0xcbcb, 0x407d, {0x88, 0x14, 0x9e, 0xd6, 0x73, 0xd0, 0xdd, 0x6b}},
	//usb A5DCBF10-6530-11D2-901F-00C04FB951ED  GUID_DEVINTERFACE_USB_DEVICE
	//{0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}},
};

extern UINT do_nothing(void);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CmdmfastbootDlg 对话框
CmdmfastbootDlg::CmdmfastbootDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CmdmfastbootDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CmdmfastbootDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PACKAGE_PATH, m_strPackagePath);
}

BEGIN_MESSAGE_MAP(CmdmfastbootDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_STOP, &CmdmfastbootDlg::OnBnClickedButtonStop)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(UI_MESSAGE_UPDATE_PROGRESS_INFO, OnUpdateProgressInfo)
	ON_MESSAGE(UI_MESSAGE_UPDATE_PACKAGE_INFO, OnUpdatePackageInfo)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CmdmfastbootDlg::OnBnClickedBtnBrowse)
END_MESSAGE_MAP()

UINT __cdecl DemoDownloadThread( LPVOID pParam )
{
	TranseInfo *info = (TranseInfo*)pParam;
	int iProcess = 0;
	sleep(info->portUI->iID-1);
	UIInfo uiInfoV;
	UIInfo uiInfoS;

	UIInfo uiInfoFrw;
	uiInfoFrw.infoType = FIRMWARE_VER;
	uiInfoFrw.sVal = "VX140100BX";
	::PostMessage(info->dlgMain->m_hWnd, UI_MESSAGE_UPDATE_PROGRESS_INFO, info->portUI->iID, (LPARAM)&uiInfoFrw);

	UIInfo uiInfoQCN;
	uiInfoQCN.infoType = QCN_VER;
	uiInfoQCN.sVal = "VA340100BV";
	::PostMessage(info->dlgMain->m_hWnd, UI_MESSAGE_UPDATE_PROGRESS_INFO, info->portUI->iID, (LPARAM)&uiInfoQCN);

	UIInfo uiInfoLinux;
	uiInfoLinux.infoType = LINUX_VER;
	uiInfoLinux.sVal = "VXB4010000";
	::PostMessage(info->dlgMain->m_hWnd, UI_MESSAGE_UPDATE_PROGRESS_INFO, info->portUI->iID, (LPARAM)&uiInfoLinux);

	while (1)
	{
		iProcess++;
		uiInfoV.infoType = PROGRESS_VAL;
		uiInfoV.iVal = iProcess%100;
		::PostMessage(info->dlgMain->m_hWnd, UI_MESSAGE_UPDATE_PROGRESS_INFO, info->portUI->iID, (LPARAM)&uiInfoV);

		uiInfoS.infoType = PROGRESS_STR;
		if (uiInfoV.iVal==5)
		{
			uiInfoS.sVal = "Initial...";
			::PostMessage(info->dlgMain->m_hWnd, UI_MESSAGE_UPDATE_PROGRESS_INFO, info->portUI->iID, (LPARAM)&uiInfoS);
		}
		else if (uiInfoV.iVal==30)
		{
			uiInfoS.sVal = "Downloading...";
			::PostMessage(info->dlgMain->m_hWnd, UI_MESSAGE_UPDATE_PROGRESS_INFO, info->portUI->iID, (LPARAM)&uiInfoS);
		}

		Sleep(10);
	}
	AfxEndThread(0);
	return 0;
}

// CmdmfastbootDlg 消息处理程序

BOOL CmdmfastbootDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//注释设备通知，不能放在构造函数，否则 RegisterDeviceNotification 返回78.
	RegisterAdbDeviceNotification();
	usb_vendors_init();
	find_devices();
	//do_nothing();

    TransverseDevice(6, usb_class_id[0]);

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	// TODO: 在此添加额外的初始化代码
	PortStateUI1.Create(IDD_PORT_STATE, this);
	PortStateUI1.Init(PORT_UI_ID_FIRST);

	PortStateUI2.Create(IDD_PORT_STATE, this);
	PortStateUI2.Init(PORT_UI_ID_SECOND);

	PortStateUI3.Create(IDD_PORT_STATE, this);
	PortStateUI3.Init(PORT_UI_ID_THIRD);

	PortStateUI4.Create(IDD_PORT_STATE, this);
	PortStateUI4.Init(PORT_UI_ID_FOURTH);

	ShowWindow(SW_MAXIMIZE);

#if 1
	//ui test beging
	m_strFrmVer = L"VX140100BX";
	::PostMessage(m_hWnd, UI_MESSAGE_UPDATE_PACKAGE_INFO, FIRMWARE_VER, (LPARAM)&m_strFrmVer);
	m_strQCNVer = L"VA340100BV";
	::PostMessage(m_hWnd, UI_MESSAGE_UPDATE_PACKAGE_INFO, QCN_VER, (LPARAM)&m_strQCNVer);
	m_strLinuxVer = L"VXB4010000";
	::PostMessage(m_hWnd, UI_MESSAGE_UPDATE_PACKAGE_INFO, LINUX_VER, (LPARAM)&m_strLinuxVer);

	TranseInfo1.dlgMain = this;
	TranseInfo1.portUI = &PortStateUI1;
	AfxBeginThread(DemoDownloadThread, &TranseInfo1);
	TranseInfo2.dlgMain = this;
	TranseInfo2.portUI = &PortStateUI2;
	AfxBeginThread(DemoDownloadThread, &TranseInfo2);
	TranseInfo3.dlgMain = this;
	TranseInfo3.portUI = &PortStateUI3;
	AfxBeginThread(DemoDownloadThread, &TranseInfo3);
	TranseInfo4.dlgMain = this;
	TranseInfo4.portUI = &PortStateUI4;
	AfxBeginThread(DemoDownloadThread, &TranseInfo4);
	//end
#endif

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmdmfastbootDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmdmfastbootDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CmdmfastbootDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CmdmfastbootDlg::RegisterAdbDeviceNotification(void) {
   //注册插拔事件
   HDEVNOTIFY hDevNotify;

   DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
   ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
   NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
   NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
   for(int i=0; i<sizeof(usb_class_id)/sizeof(GUID); i++)
   {
      NotificationFilter.dbcc_classguid = usb_class_id[i];
      hDevNotify = RegisterDeviceNotification(this->m_hWnd,//this->GetSafeHwnd(),
		  &NotificationFilter,
		  DEVICE_NOTIFY_WINDOW_HANDLE);
      if( !hDevNotify )
      {
		  DEBUG("RegisterDeviceNotification failed: %d\n", GetLastError());
         return FALSE;
      }
   }
   return TRUE;
}

BOOL CmdmfastbootDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
   if (dwData == 0)
   {
      WARN("OnDeviceChange, dwData == 0 .EventType: 0x%x",
         nEventType);
      return FALSE;
   }

   DEV_BROADCAST_HDR* phdr = (DEV_BROADCAST_HDR*)dwData;
   PDEV_BROADCAST_DEVICEINTERFACE pDevInf =
               (PDEV_BROADCAST_DEVICEINTERFACE)phdr;

   DEBUG("OnDeviceChange, EventType: 0x%x, DeviceType 0x%x",
         nEventType, phdr->dbch_devicetype);

   if (nEventType == DBT_DEVICEARRIVAL)
   {
      switch( phdr->dbch_devicetype )
      {
      case DBT_DEVTYP_DEVNODE:
        WARN("OnDeviceChange, get DBT_DEVTYP_DEVNODE");
        break;
      case DBT_DEVTYP_VOLUME:
         {
            /* enumerate devices and shiftdevice
            */
            break;
         }
      case DBT_DEVTYP_DEVICEINTERFACE:
         {
            UpdateDevice(pDevInf, dwData);
            //find_devices();
            //do_nothing();
            break;
         }
      }
   }
   else if (nEventType == DBT_DEVICEREMOVECOMPLETE)
   {
      if (phdr->dbch_devicetype == DBT_DEVTYP_PORT)
      {
         /* enumerate device and check if the port
          * composite should be removed
          */

      }
   }

   return TRUE;
}

void CmdmfastbootDlg::UpdateDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam)
{
   // dbcc_name:
   // \\?\USB#Vid_04e8&Pid_503b#0002F9A9828E0F06#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
   // convert to
   // USB\Vid_04e8&Pid_503b\0002F9A9828E0F06
   ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
   CString szDevId = pDevInf->dbcc_name+4;
   int idx = szDevId.ReverseFind(_T('#'));
   ASSERT( -1 != idx );
   szDevId.Truncate(idx);
   szDevId.Replace(_T('#'), _T('\\'));
   szDevId.MakeUpper();

   CString szClass;
   idx = szDevId.Find(_T('\\'));
   ASSERT(-1 != idx );
   szClass = szDevId.Left(idx);

   // if we are adding device, we only need present devices
   // otherwise, we need all devices
   DWORD dwFlag = DBT_DEVICEARRIVAL != wParam
      ? DIGCF_ALLCLASSES : (DIGCF_ALLCLASSES | DIGCF_PRESENT);
   HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, szClass, NULL, dwFlag);
   if( INVALID_HANDLE_VALUE == hDevInfo )
   {
      AfxMessageBox(CString("SetupDiGetClassDevs(): ")
                    + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
      return;
   }

   SP_DEVINFO_DATA* pspDevInfoData =
      (SP_DEVINFO_DATA*)HeapAlloc(GetProcessHeap(), 0, sizeof(SP_DEVINFO_DATA));
   pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
   for(int i=0; SetupDiEnumDeviceInfo(hDevInfo,i,pspDevInfoData); i++)
   {
      DWORD DataT ;
      DWORD nSize=0 ;
      TCHAR buf[MAX_PATH];

      if ( !SetupDiGetDeviceInstanceId(hDevInfo, pspDevInfoData, buf, sizeof(buf), &nSize) )
      {
         AfxMessageBox(CString("SetupDiGetDeviceInstanceId(): ")
                       + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
         break;
      }

     DEBUG(L"LOCATEION %d, DeviceInstanceId %S", i, buf);
      if ( szDevId == buf )
      {
         // device found
         if ( SetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData,
                                               SPDRP_LOCATION_INFORMATION,
                                               &DataT, (PBYTE)buf, sizeof(buf), &nSize) ) {
            DEBUG("LOCATEION %S(datatype %d)", buf,DataT);
            // do nothing
         }
         if ( SetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData,
                                               SPDRP_ADDRESS, &DataT,
                                               (PBYTE)buf, sizeof(buf), &nSize) ) {
            DEBUG("ADDRESS %d(datatype %d, size %d)", (unsigned int)buf[0],DataT,nSize);

         }
         if ( SetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData,
                                               SPDRP_BUSNUMBER, &DataT,
                                               (PBYTE)buf, sizeof(buf), &nSize) ) {
            DEBUG("ADDRESS %d(datatype %d, size %d)", (unsigned int)buf[0],DataT,nSize);

         }
         else {
            lstrcpy(buf, _T("Unknown"));
         }
         // update UI
         break;
      }
   }

   if ( pspDevInfoData ) {
      HeapFree(GetProcessHeap(), 0, pspDevInfoData);
   }

   SetupDiDestroyDeviceInfoList(hDevInfo);
}

void GetInterfaceDeviceDetail(HDEVINFO hDevInfoSet) {
  BOOL bResult;
  PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
  SP_DEVICE_INTERFACE_DATA   ifdata;
  char ch[MAX_PATH];
  int i;
  ULONG predictedLength = 0;
  ULONG requiredLength = 0;

  ifdata.cbSize = sizeof(ifdata);

  //   取得该设备接口的细节(设备路径)
  bResult = SetupDiGetInterfaceDeviceDetail(hDevInfoSet,   /*设备信息集句柄*/
                                            &ifdata,   /*设备接口信息*/
                                            NULL,   /*设备接口细节(设备路径)*/
                                            0,   /*输出缓冲区大小*/
                                            &requiredLength,   /*不需计算输出缓冲区大小(直接用设定值)*/
                                            NULL);   /*不需额外的设备描述*/
  /*   取得该设备接口的细节(设备路径)*/
  predictedLength=requiredLength;

  pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)GlobalAlloc(LMEM_ZEROINIT, predictedLength);
  pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
  bResult = SetupDiGetInterfaceDeviceDetail(hDevInfoSet,   /*设备信息集句柄*/
                                            &ifdata,   /*设备接口信息*/
                                            pDetail,   /*设备接口细节(设备路径)*/
                                            predictedLength,   /*输出缓冲区大小*/
                                            &requiredLength,   /*不需计算输出缓冲区大小(直接用设定值)*/
                                            NULL);   /*不需额外的设备描述*/

  if(bResult)
  {
    memset(ch, 0, MAX_PATH);
    /*复制设备路径到输出缓冲区*/
    for(i=0; i<requiredLength; i++)
    {
      ch[i]=*(pDetail->DevicePath+8+i);
    }
    printf("%s\r\n", ch);
  }
//  GlobalFree
}

void CmdmfastbootDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CmdmfastbootDlg::OnSize(UINT nType, int cx, int cy)
{
	if (500>cx || 500> cy)
	{
		return;
	}
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(NULL == PortStateUI1.m_hWnd)
		return;
	int space = 20;
	int iWide = cx/2 - space;
	int iHigh = cy/2 - 5*space;
	int iTop = 100;
	//GetDlgItem(IDC_GRP_PKG_INFO)->SetWindowPos(0, space, space, cy-space*2, 50, 0);
	PortStateUI1.SetWindowPos(0, space, iTop+space, iWide-space, iHigh, 0);
	PortStateUI2.SetWindowPos(0, cx/2 + space, iTop+space, iWide-space, iHigh, 0);
	PortStateUI3.SetWindowPos(0, space, cy/2 + space*2, iWide-space, iHigh, 0);
	PortStateUI4.SetWindowPos(0, cx/2 + space, cy/2 + space*2, iWide-space, iHigh, 0);

	//BUTTON
	RECT rect;
	GetDlgItem(IDC_BTN_STOP)->GetClientRect(&rect);
	GetDlgItem(IDC_BTN_STOP)->SetWindowPos(0, cx - 400, cy - 50, rect.right, rect.bottom, 0);
	GetDlgItem(IDCANCEL)->SetWindowPos(0, cx - 300, cy - 50, rect.right, rect.bottom, 0);
	GetDlgItem(IDOK)->SetWindowPos(0, cx - 200, cy - 50, rect.right, rect.bottom, 0);

	Invalidate(TRUE);
}

void CmdmfastbootDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	lpMMI->ptMinTrackSize.x   = 1000 ;	//主窗口最小宽度
	lpMMI->ptMinTrackSize.y   = 700  ;  //主窗口最小高度
	CDialog::OnGetMinMaxInfo(lpMMI);
}

LRESULT CmdmfastbootDlg::OnUpdateProgressInfo(WPARAM wParam, LPARAM lParam)
{
	int iUiSerial = (int)wParam;
	UIInfo* uiInfo = (UIInfo*)lParam;
	switch(iUiSerial)
	{
	case PORT_UI_ID_FIRST:
		UpdatePortUI(PortStateUI1, uiInfo);
		break;
	case PORT_UI_ID_SECOND:
		UpdatePortUI(PortStateUI2, uiInfo);
		break;
	case PORT_UI_ID_THIRD:
		UpdatePortUI(PortStateUI3, uiInfo);
		break;
	case PORT_UI_ID_FOURTH:
		UpdatePortUI(PortStateUI4, uiInfo);
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CmdmfastbootDlg::OnUpdatePackageInfo(WPARAM wParam, LPARAM lParam)
{
	UI_INFO_TYPE infoType = (UI_INFO_TYPE)wParam;
	CString* pStrInfo = (CString*)lParam;
	switch(infoType)
	{
	case FIRMWARE_VER:
		GetDlgItem(IDC_EDIT_FRM_VER_MAIN)->SetWindowText(pStrInfo->GetBuffer());
		break;
	case QCN_VER:
		GetDlgItem(IDC_EDIT_QCN_VER_MAIN)->SetWindowText(pStrInfo->GetBuffer());
		break;
	case LINUX_VER:
		GetDlgItem(IDC_EDIT_LINUX_VER_MAIN)->SetWindowText(pStrInfo->GetBuffer());
		break;
	default:
		break;
	}
	return 0;
}

void CmdmfastbootDlg::UpdatePortUI(CPortStateUI& portUI, UIInfo* uiInfo)
{
	switch(uiInfo->infoType)
	{
	case PROGRESS_VAL:
		portUI.SetProgress(uiInfo->iVal);
			break;
	case PROGRESS_STR:
		portUI.SetInfo(PROGRESS_STR, uiInfo->sVal);
			break;
	case FIRMWARE_VER:
		portUI.SetInfo(FIRMWARE_VER, uiInfo->sVal);
			break;
	case QCN_VER:
		portUI.SetInfo(QCN_VER, uiInfo->sVal);
			break;
	case LINUX_VER:
		portUI.SetInfo(LINUX_VER, uiInfo->sVal);
			break;
	default:
		break;
	}
}
void CmdmfastbootDlg::OnBnClickedBtnBrowse()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str = _T("Img File (*.img)|All Files (*.*)|*.*||");

	CFileDialog cfd( TRUE,
		NULL,
		NULL,
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
		(LPCTSTR)str,
		NULL);

	if (cfd.DoModal() == IDOK)
	{
		m_strPackagePath = cfd.GetPathName();
		UpdateData(FALSE);	
	}
}
