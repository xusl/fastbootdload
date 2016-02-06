// mdmfastbootDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "log.h"
#include "mdmfastboot.h"
#include "mdmfastbootDlg.h"
#include "usb_adb.h"
#include "fastbootflash.h"
#include "adbhost.h"
#include "usb_vendors.h"
#include "DiagPST.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")

//static UINT usb_work(LPVOID wParam);
MODULE_NAME CmdmfastbootDlg::m_module_name;

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


UsbWorkData::UsbWorkData(int index, CmdmfastbootDlg *dlg, DeviceCoordinator *coordinator) {
    hWnd = dlg;
    ctl.Create(IDD_PORT_STATE, dlg);
    ctl.Init(index);
    usb = NULL;
    stat = USB_STAT_IDLE;
    //workdata->img = m_image;
    work = NULL;
    update_qcn = FALSE;
    partition_nr = 0;
    devIntf = NULL;
    ZeroMemory(flash_partition, sizeof(flash_partition));
    pCoordinator = coordinator;
}

UsbWorkData::~UsbWorkData() {
    ;
}

BOOL UsbWorkData::IsIdle() {
    if (stat == USB_STAT_SWITCH ||
        stat == USB_STAT_WORKING)
        return FALSE;
    return TRUE;
}

BOOL UsbWorkData::Clean(BOOL noCleanUI) {
  usb = NULL;
  devIntf->SetDeviceStatus(DEVICE_UNKNOW);
  stat = USB_STAT_IDLE;
  work = NULL;
  update_qcn = FALSE;
  partition_nr = 0;
  ZeroMemory(flash_partition, sizeof(flash_partition));
  if(!noCleanUI)
      ctl.Reset();
    return TRUE;
}

BOOL UsbWorkData::Reset(VOID) {
    if (stat == USB_STAT_WORKING) {
      if (work != NULL) //Delete, or ExitInstance
        work->PostThreadMessage( WM_QUIT, NULL, NULL );
      usb_close(usb);
      hWnd->KillTimer((UINT_PTR)this);
    } else if (stat == USB_STAT_SWITCH) {
      //remove_switch_device(workdata->usb_sn);
      hWnd->KillTimer((UINT_PTR)this);
    } else if (stat == USB_STAT_FINISH) {
      ;
    } else if (stat == USB_STAT_ERROR) {
      usb_close(usb);
    } else {
      return FALSE;
    }
    Clean(FALSE);
    return TRUE;
}

BOOL UsbWorkData::Abort(VOID) {
      stat = USB_STAT_ERROR;
  hWnd->KillTimer((UINT_PTR)this);
    return TRUE;
}

BOOL UsbWorkData::Start(DeviceInterfaces* pDevIntf, UINT nElapse, BOOL flashdirect) {
    ASSERT(pDevIntf != NULL);
    devIntf = pDevIntf;

  //TODO::
  usb = devIntf->GetUsbHandle(flashdirect);
  usb_set_work(usb, TRUE);
    devIntf->SetAttachStatus(true);
  stat = USB_STAT_WORKING;
  work = AfxBeginThread(CmdmfastbootDlg::usb_work, this);

  if (work != NULL) {
    INFO("Schedule work for %s!", devIntf->GetDevTag());
    work->m_bAutoDelete = TRUE;
     hWnd->SetTimer((UINT_PTR)this, nElapse * 1000, NULL);
  } else {
    CRITICAL("%s : Can not begin thread!", devIntf->GetDevTag());
    usb_set_work(usb, FALSE);
    Clean();
  }
    return TRUE;
}

BOOL UsbWorkData::Finish(VOID) {
  stat = USB_STAT_FINISH;
  //data->work = NULL;
  //KILL work timer.
  hWnd->KillTimer((UINT_PTR)this);

    return TRUE;
}


BOOL UsbWorkData::SwitchDev(UINT nElapse) {
  usb = NULL;
  work = NULL;
  stat = USB_STAT_SWITCH;
  /*Set switch timeout*/
  hWnd->SetTimer((UINT_PTR)this, nElapse * 1000, NULL);
    return TRUE;
}


UINT UsbWorkData::ui_text_msg(UI_INFO_TYPE info_type, PCCH msg) {
  UIInfo* info = new UIInfo();

  //if (FLASH_DONE && data->hWnd->m_fix_port_map)
  //  sleep(1);

  info->infoType = info_type;
  info->sVal = msg;
  hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info,
                          (LPARAM)this);
  return 0;
}

UINT UsbWorkData::SetProgress(int progress) {
        UIInfo* info = new UIInfo();

    info->infoType = PROGRESS_VAL;
    info->iVal = progress;
    hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                  (WPARAM)info,
                  (LPARAM)this);
    return 0;
}

// CmdmfastbootDlg 对话框
CmdmfastbootDlg::CmdmfastbootDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CmdmfastbootDlg::IDD, pParent),
	m_WorkDev(),
	mDevCoordinator()
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_bInit = FALSE;
  m_updated_number = 0;
  m_module_name = MODULE_M801;
  m_SetDlg.m_pParent = this;
  m_image = NULL;
  InitSettingConfig();
  for (int i = 0; i < m_nPort; i++) {
    m_workdata[i] = NULL;
  }
}

CmdmfastbootDlg::~CmdmfastbootDlg() {
  for (int i = 0; i < m_nPort; i++) {
    if (m_workdata[i] != NULL) {
       delete m_workdata[i];
       m_workdata[i] = NULL;
    }
  }
}

void CmdmfastbootDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PACKAGE_PATH, m_PackagePath);
	DDX_Text(pDX, IDC_EDIT_FRM_VER_MAIN, m_FwVer);
	DDX_Text(pDX, IDC_EDIT_QCN_VER_MAIN, m_QCNVer);
	DDX_Text(pDX, IDC_EDIT_LINUX_VER_MAIN, m_LinuxVer);
}

BEGIN_MESSAGE_MAP(CmdmfastbootDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_STOP, &CmdmfastbootDlg::OnBnClickedButtonStop)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(UI_MESSAGE_DEVICE_INFO, &CmdmfastbootDlg::OnDeviceInfo)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CmdmfastbootDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_START, &CmdmfastbootDlg::OnBnClickedStart)
	ON_COMMAND(ID_ABOUT, &CmdmfastbootDlg::OnAbout)
	ON_COMMAND(ID_HELP, &CmdmfastbootDlg::OnHelp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CmdmfastbootDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SETTING, &CmdmfastbootDlg::OnBnClickedSetting)
	ON_WM_SIZING()
	ON_WM_MEASUREITEM()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_IMAGE_LIST, &CmdmfastbootDlg::OnLvnItemchanged)
	ON_COMMAND(ID_FILE_M850, &CmdmfastbootDlg::OnFileM850)
	ON_COMMAND(ID_FILE_M801, &CmdmfastbootDlg::OnFileM801)
END_MESSAGE_MAP()

void CmdmfastbootDlg::OnHelp()
{
	AfxMessageBox(L"Help info...");
}

void CmdmfastbootDlg::OnAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


BOOL CmdmfastbootDlg::SetPortDialogs(int x, int y,  int w, int h)
{
  //int size = sizeof(m_workdata) / sizeof(m_workdata[0]);
  int r, c, pw, ph;
  CPortStateUI*  port;
  int R_NUM, C_NUM;

  R_NUM = m_nPortRow;
  C_NUM = m_nPort / R_NUM;

  if (R_NUM * C_NUM < m_nPort) {
    R_NUM > C_NUM ? C_NUM ++ :  R_NUM++;
  }

  pw = w / C_NUM;
  ph = h / R_NUM;

  for (r = 0; r < R_NUM; r++) {
    for (c = 0; c < C_NUM; c++) {
      if (r * C_NUM + c >= m_nPort) break;

      port = &(m_workdata[r * C_NUM + c]->ctl);
      port->SetWindowPos(0,
                         x + c * pw,
                         y + r * ph,
                         pw,
                         ph,
                         0);
    }
  }

  return true;
}

BOOL CmdmfastbootDlg::SetDlgItemPos(UINT nID, int x, int y) {
  RECT rect;
  int w, h;
  CWnd * handle = GetDlgItem(nID);
  if (handle == NULL) {
    ERROR("NO Item %d", nID);
    return FALSE;
  }

  handle->GetClientRect(&rect);
  //w = x + rect.right - rect.left;
  //h = y +  rect.bottom - rect.top;
  if (IDC_EDIT_PACKAGE_PATH == nID && m_nPort == 1) {
    w = rect.right < 300 ? 300 : rect.right;
  }
  w = rect.right < 100 ? 100 : rect.right;
  h = rect.bottom < 25 ? 25 : rect.bottom;
  handle->SetWindowPos(0, x, y, w, h, SWP_NOSENDCHANGING | SWP_NOSIZE);
  return TRUE;
}


BOOL CmdmfastbootDlg::IsHaveUsbWork(void) {
  UsbWorkData* workdata;
  int i= 0;
  for (; i < m_nPort; i++) {
    workdata = m_workdata[i];
    if (!workdata->IsIdle())
        return TRUE ;
  }
  return FALSE;
}

/*
* get usbworkdata by usb_sn, the device is in switch or in working
* or done, but not idle.
*/
UsbWorkData * CmdmfastbootDlg::FindUsbWorkData(wchar_t *devPath) {
     DeviceInterfaces* devIntf;
    if(mDevCoordinator.GetDevice(devPath, &devIntf)) {
        devIntf->SetDeviceStatus(DEVICE_UNKNOW);
        LOGI("Remove device %S", devPath);
    }

  // first search the before, for switch device.
  for ( int i= 0; i < m_nPort; i++) {
      DeviceInterfaces* item = m_workdata[i]->devIntf;
    if (item->MatchDevPath(devPath)&&
      (m_workdata[i]->stat != USB_STAT_IDLE))
      return m_workdata[i];
  }

  return NULL;
}

BOOL CmdmfastbootDlg::UpdatePackageInfo(BOOL update) {
  const wchar_t * qcn = m_image->get_package_qcn_path();
  const FlashImageInfo* img = m_image->image_enum_init();
  int item = 0;

  m_UpdateDownloadFlag = update;

  m_imglist->DeleteAllItems();
  for(;img != NULL; ) {
    m_imglist->InsertItem(item,img->partition);
    m_imglist->SetItemText(item,1,GetFileNameFromFullPath(img->lpath));

    item ++;
    img = m_image->image_enum_next(img);
  }

  if (qcn != NULL) {
    HANDLE    file;
    file = CreateFile( qcn,GENERIC_READ,FILE_SHARE_READ,
                       NULL, OPEN_EXISTING,
                       0, NULL );
    if (file != INVALID_HANDLE_VALUE) {
      m_imglist->InsertItem(item,L"qcn");
      m_imglist->SetItemText(item,1,qcn);
      CloseHandle(file);
    }
  }
  img = m_image->image_enum_init();

  for(;img != NULL; )
  {
      for(item = 0; m_imglist->GetItemCount()> item; item++) {
      CString strPartitionName = m_imglist->GetItemText(item, 0);
        if ( strPartitionName == img->partition) {
	        m_imglist->SetCheck(item, img->need_download);
        }
      }
	  img = m_image->image_enum_next(img);
  }

  m_image->get_pkg_a5sw_kern_ver(m_LinuxVer);
  m_image->get_pkg_qcn_ver(m_QCNVer);
  m_image->get_pkg_fw_ver(m_FwVer);

  wchar_t moduleName[MAX_PATH];
  GetPrivateProfileString(L"app",L"module",L"M850",moduleName, MAX_PATH,m_ConfigPath);
  m_strModuleName = moduleName;
  if (-1 != m_strModuleName.Find(L"M850"))
  {
	  m_module_name = MODULE_M850;
	  GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYPOSITION|MF_CHECKED);
	  GetMenu()->GetSubMenu(0)->CheckMenuItem(1,MF_BYPOSITION|MF_UNCHECKED);
  }
  else
  {
	  GetMenu()->GetSubMenu(0)->CheckMenuItem(1,MF_BYPOSITION|MF_CHECKED);
	  GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYPOSITION|MF_UNCHECKED);
  }
  UpdateData(FALSE);
  m_UpdateDownloadFlag = TRUE;
  return TRUE;
}

BOOL CmdmfastbootDlg::SetWorkStatus(BOOL bwork, BOOL bforce) {
  if(!bforce && m_bWork == bwork) {
    WARN("Do not need to chage status.");
    return FALSE;
  }

  if (m_bWork && IsHaveUsbWork()) {
    int iRet = AfxMessageBox(L"Still have active downloading! Exit anyway?",
                  MB_YESNO|MB_DEFBUTTON2);
    if (IDYES!=iRet)
    {
      return FALSE;
    }
  }

  if (bwork) {
      for (int i= 0; i < m_nPort; i++) {
        m_workdata[i]->Reset();
      }
  }

  GetDlgItem(IDC_BTN_START)->EnableWindow(!bwork);
  GetDlgItem(IDCANCEL)->EnableWindow(!bwork);
  GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(!bwork);
  GetDlgItem(IDC_BTN_STOP)->EnableWindow(bwork);
   if (m_pack_img == FALSE) {
    m_SetDlg.EnableSettings(!bwork);
   }
  m_bWork = bwork;
  return TRUE;

}

BOOL CmdmfastbootDlg::InitSettingConfig()
{
  LPCTSTR lpFileName;
  int data_len;
  wchar_t log_conf[MAX_PATH + 128];
  wchar_t* log_file = NULL;
  char *log_tag = NULL;
  char *log_level = NULL;

  GetAppPath(m_ConfigPath);
  m_ConfigPath += L"mdmconfig.ini";

  lpFileName = m_ConfigPath.GetString();
  //  m_ConfigPath.GetBuffer(int nMinBufLength)

  //read configuration for log system and start log.
  data_len = GetPrivateProfileString(L"log",L"file",NULL,log_conf, MAX_PATH,lpFileName);
  if (data_len) log_file = wcsdup(log_conf);
  data_len = GetPrivateProfileString(L"log",L"tag",L"all",log_conf, MAX_PATH,lpFileName);
  if (data_len) log_tag = WideStrToMultiStr(log_conf);
  data_len = GetPrivateProfileString(L"log",L"level",NULL,log_conf,MAX_PATH,lpFileName);
  if (data_len) log_level = WideStrToMultiStr(log_conf);
  StartLogging(log_file, log_level, log_tag);

  if(log_file) free(log_file);
  if(log_tag) delete log_tag;
  if(log_level) delete log_level;

  //construct update software package. get configuration about partition information.
  if (NULL!=m_image)
  {
	  delete m_image;
  }
  m_image = new flash_image(lpFileName);

  //init app setting.
  m_pack_img = GetPrivateProfileInt(L"app", L"pack_img", 1,lpFileName);;
  m_fix_port_map = GetPrivateProfileInt(L"app", L"fix_port_map",1,lpFileName);
  switch_timeout = GetPrivateProfileInt(L"app", L"switch_timeout", 300,lpFileName);
  work_timeout = GetPrivateProfileInt(L"app", L"work_timeout",600,lpFileName);

  m_flashdirect = GetPrivateProfileInt(L"app", L"flashdirect", 1,lpFileName);
  m_forceupdate = GetPrivateProfileInt(L"app", L"forceupdate", 0,lpFileName);
  m_bWork = GetPrivateProfileInt(L"app",L"autowork", 0, lpFileName);

  //layout setting.
  m_nPort = GetPrivateProfileInt(L"app", L"port_num",1,lpFileName);
  m_nPortRow = GetPrivateProfileInt(L"app", L"port_row",1,lpFileName);
  if ( m_nPort < 1 )
    m_nPort = 1;
  else if (m_nPort > PORT_NUM_MAX)
    m_nPort = PORT_NUM_MAX;

  if (m_nPortRow < 1 )
    m_nPortRow  = 1;
  else if (m_nPortRow > m_nPort)
    m_nPortRow =  m_nPort;

  if (m_pack_img) {
    m_forceupdate = TRUE; /*Now fw build system can not handle config.xml, so set it to true*/
  }

  CString path;

  unsigned int size;
  GetAppPath(path);
  path += "/config1.xml";
  void *data = load_file(path.GetString(), &size);
  XmlParser parser1;
  parser1.Parse("<?wsx version \"1.0\" ?><smil> \
           <media src = \"welcome1.asf\"/>cdcddddddddd</smil>");


  XmlParser parser;

  GetAppPath(path);
  path += "/config.xml";
  parser.Parse(path.GetString());
  string refs;
  parser.getElementsByTagName(L"RECOVERYFS", refs);
  LOGE("RECOVERYFS value %sxxxxxxxxxxxxxxxxxxxxx", refs.c_str());


  return TRUE;
}


BOOL CmdmfastbootDlg::InitSettingDlg(void) {
    //CWnd * handle = GetDlgItem(IDC_SETTING);
    //handle->EnableWindow(FALSE);

    //m_SetDlg.Attach(this->m_hWnd);
    //m_SetDlg.ModifyStyle(WS_POPUP | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU,
    //          WS_CHILD | WS_VISIBLE | DS_CENTER, 0);
    //m_SetDlg.EnableWindow(TRUE);
    if (m_pack_img)
      return FALSE;

    m_SetDlg.SetFlashDirectData(&m_flashdirect);
    m_SetDlg.SetScheduleData(&m_fix_port_map);
    m_SetDlg.SetForeUpdateData(&m_forceupdate);
    //m_SetDlg.UpdateData();
    m_SetDlg.Create(IDD_SETTINGS, this);
    m_SetDlg.ShowWindow(WS_CHILDWINDOW);//Invalidate();
    return TRUE;
}

// CmdmfastbootDlg 消息处理程序

BOOL CmdmfastbootDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  ::SetProp(m_hWnd, JRD_MDM_FASTBOOT_TOOL_APP, (HANDLE)1);//for single instance

  if (!StopAdbServer())
  {
	  EndDialog(0);
	  return TRUE;
  }

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

  InitSettingDlg();

  for (int i = 0; i < m_nPort; i++) {
    m_workdata[i] = new UsbWorkData(i, this, &mDevCoordinator);
  }
  //SetUpDevice(NULL, 0, &GUID_DEVCLASS_USB,  _T("USB"));
  //SetUpAdbDevice(NULL, 0);

  m_bInit = TRUE;
  m_UpdateDownloadFlag = TRUE;
  ShowWindow(SW_MAXIMIZE);
::SetWindowLong(m_hWnd,GWL_STYLE,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX);
#if 0
  HRESULT hr = m_dlWorkerPool.Initialize(NULL, THREADPOOL_SIZE);
  m_dlWorkerPool.SetTimeout(30 * 1000);
  if(!SUCCEEDED(hr))
  {
    ERROR("Failed to init thread pool!");
    return FALSE;
  }
  #endif

  m_imglist = ((CListCtrl*)GetDlgItem(IDC_IMAGE_LIST));
  m_imglist->InsertColumn(0, _T("Partition/QCN"),LVCFMT_LEFT, 90);
  m_imglist->InsertColumn(1, _T("File Name"),LVCFMT_LEFT, 280);
  m_imglist->SetExtendedStyle(m_imglist->GetExtendedStyle() |LVS_EX_CHECKBOXES);//设置控件有勾选功能

  if (m_pack_img) {
    GetDlgItem(IDC_BTN_BROWSE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_EDIT_PACKAGE_PATH)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_PKG)->ShowWindow(SW_HIDE);
  }
  m_PackagePath = m_image->get_package_dir();
  UpdatePackageInfo(FALSE);

  //注释设备通知，不能放在构造函数，否则 RegisterDeviceNotification 返回87,因为构造函数中m_hWnd还没被初始化为有效值.
  RegisterAdbDeviceNotification(m_hWnd);
  SetWorkStatus(m_bWork, TRUE);
  adb_usb_init();

  if (kill_adb_server(DEFAULT_ADB_PORT) == 0) {
    SetTimer(TIMER_EVT_ADBKILLED, 1000, &DeviceEventTimerProc);
  } else {
  SetupDevice(TIMER_EVT_ADBKILLED);
  }

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

BOOL CmdmfastbootDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
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
            WARN("OnDeviceChange, get DBT_DEVTYP_DEVNODE");
            break;
        case DBT_DEVTYP_VOLUME:
            {
                /* enumerate devices and shiftdevice
                * When device, twice this message , one is mass storage (composite device),
                * another is cd-rom. use timer to delay, by reset timer, we only need enumerate
                * one time actually.
                */
                LOGI("OnDeviceChange, get DBT_DEVTYP_VOLUME");
                SetTimer(TIMER_EVT_REJECTCDROM, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
                break;
            }
        case DBT_DEVTYP_PORT:
            {
                LOGI("device arrive, DBT_DEVTYP_PORT");
                //SetTimer(TIMER_EVT_COMPORT, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
               HandleComDevice();
            ScheduleDeviceWork(m_flashdirect);
            break;
            }
        case DBT_DEVTYP_DEVICEINTERFACE:
            LOGI("device arrive, DBT_DEVTYP_DEVICEINTERFACE");
                //SetTimer(TIMER_EVT_USBADB, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
            EnumerateAdbDevice();
            ScheduleDeviceWork(m_flashdirect);

                break;
        }
    } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
        switch (phdr->dbch_devicetype) {
        case DBT_DEVTYP_DEVICEINTERFACE:
            {
               // RemoveDevice(pDevInf, dwData);
                break;
            }
        case DBT_DEVTYP_VOLUME:
            {
                /* enumerate devices and shiftdevice
                */
                LOGI("OnDeviceChange, get DBT_DEVTYP_VOLUME");
                break;
            }
        case DBT_DEVTYP_PORT:
            {
                LOGI("device arrive, DBT_DEVTYP_PORT");
                break;
            }
        }
    }

    return TRUE;
}

BOOL CmdmfastbootDlg::RemoveDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam) {

    ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
    UsbWorkData * data = FindUsbWorkData(pDevInf->dbcc_name);
    if (data == NULL) {
        WARN("Can not find usbworkdata for %S", pDevInf->dbcc_name);
        return FALSE;
    }

    UINT stat = data->stat;
    if (stat == USB_STAT_WORKING) {
        // the device is plugin off when in working, that is because some accident.
        //the accident power-off in switch is handle by the timer.
        //thread pool notify , exit
        if (data->work != NULL)
            data->work->PostThreadMessage( WM_QUIT, NULL, NULL );

        KillTimer((UINT_PTR)data);
        usb_close(data->usb);
    } else if (stat == USB_STAT_FINISH) {
        if (!m_fix_port_map) {
            ERROR("We do not set m_fix_port_map, "
                  "but in device remove event we can found usb work data");
            return TRUE;
        }
    } else if (stat == USB_STAT_ERROR) {
        usb_close(data->usb);
    } else {
        return TRUE;
    }

    data->Clean(mDevCoordinator.IsEmpty());
    ScheduleDeviceWork(m_flashdirect);

    return TRUE;
}

LRESULT CmdmfastbootDlg::OnDeviceInfo(WPARAM wParam, LPARAM lParam)
{
  UsbWorkData* data = (UsbWorkData*)lParam;
  UIInfo* uiInfo = (UIInfo*)wParam;

  if ( uiInfo == NULL) {
    ERROR("Invalid wParam");
    return -1;
  }

  if (data == NULL ) {
    ERROR("Invalid lParam");
    delete uiInfo;
    return -1;
  }

  switch(uiInfo->infoType ) {
  case ADB_CHK_ABORT:
    // WHEN ABORT, the device need remove manually, do not schedule next device into this UI port.
    data->Abort();
    data->ctl.SetInfo(PROMPT_TEXT, uiInfo->sVal);
    break;

  case REBOOT_DEVICE:
    if (data->usb != NULL) {
      //usb_switch_device(data->usb);
      usb_close(data->usb);
    }

    data->SwitchDev(switch_timeout);
    data->ctl.SetInfo(PROMPT_TEXT, uiInfo->sVal);
    data->ctl.SetInfo(PROMPT_TITLE, CString(""));
    break;

  case FLASH_DONE:
    usb_close(data->usb);

    mDevCoordinator.RemoveDevice(data->devIntf);

      m_updated_number ++;
  if (NULL == mDevCoordinator.IsEmpty()) {
    AfxMessageBox(L"All devices is updated!");
  }

    if (!uiInfo->sVal.IsEmpty()) {
      data->ctl.SetInfo(PROMPT_TEXT, uiInfo->sVal);
      data->ctl.SetInfo(PROMPT_TITLE, CString(""));
    }

    if (m_fix_port_map) {
    // schedule next port now, and when  app receice device remove event,
    // the current finished port is not in workdata set, for new device in the
    // same port can not bootstrap in 1 seconds, even when there are no more
    // idle device in other physical port.
    //BUT THIS PRESUME IS NEED TEST.
        sleep(1);

    data->Clean(mDevCoordinator.IsEmpty());
    ScheduleDeviceWork(m_flashdirect);
    }
    break;

  case TITLE:
    data->ctl.SetTitle(uiInfo->sVal);
    break;

  case PROGRESS_VAL:
    data->ctl.SetProgress(uiInfo->iVal);
    break;

  default:
    data->ctl.SetInfo(uiInfo->infoType, uiInfo->sVal);
    }

  delete uiInfo;
  return 0;
}

/*nIDEvent is the usb sn*/
void CmdmfastbootDlg::OnTimer(UINT_PTR nIDEvent) {
  UsbWorkData * data = (UsbWorkData *)nIDEvent;

  if (data == NULL) {
    ERROR("USB %p switch device timeout", nIDEvent);
    return ;
  }

  WARN("%s switch device timeout", data->devIntf->GetDevTag());

  if (data->stat = USB_STAT_SWITCH) {
    //remove_switch_device(nIDEvent);
  } else if (data->stat = USB_STAT_WORKING){
    if (data->work != NULL) //Delete, or ExitInstance
      data->work->PostThreadMessage( WM_QUIT, NULL, NULL );
    usb_close(data->usb);
  }

  data->Clean(mDevCoordinator.IsEmpty());
  ScheduleDeviceWork(m_flashdirect);
  KillTimer((UINT_PTR)data);
}


BOOL CmdmfastbootDlg::EnumerateAdbDevice(VOID) {
    usb_handle* handle;
    vector<CDevLabel> AdbDev;
    vector<CDevLabel> FbDev;
    vector<CDevLabel>::iterator iter;
    bool match = false;

    GetDevLabelByGUID(&GUID_DEVINTERFACE_USB_DEVICE, SRV_USBCCGP, AdbDev, true);
    GetDevLabelByGUID(&GUID_DEVINTERFACE_USB_DEVICE, SRV_WINUSB, FbDev, false);

    //GetDevLabelByGUID(&GUID_DEVINTERFACE_ADB, SRV_USBCCGP, AdbDev, true);
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_ADB, SRV_WINUSB, FbDev, false);

    find_devices(m_flashdirect);
    handle = usb_handle_enum_init();
    for (; handle != NULL; handle = usb_handle_next(handle)) {
        match = false;
        for (iter = AdbDev.begin(); iter != AdbDev.end();  ++ iter){
            CDevLabel adb(handle->interface_name);
            if (iter->Match(&adb)) {
            //if (iter->MatchDevPath(handle->interface_name)) {
                iter->Dump("adb interface");
                handle->dev_intfs = mDevCoordinator.AddDevice(*iter, DEVTYPE_ADB);
                handle->dev_intfs->SetAdbHandle(handle);
                match = true;
                break;
            }
        }
        if(match)
            continue;
        //fastboot
        for (iter = FbDev.begin(); iter != FbDev.end(); ++ iter){
            CDevLabel fb(handle->interface_name);
            if (iter->Match(&fb)) {
            //if (iter->MatchDevPath(handle->interface_name)) {
                iter->Dump("fastboot interface");
                handle->dev_intfs = mDevCoordinator.AddDevice(*iter, DEVTYPE_FASTBOOT);
                handle->dev_intfs->SetFastbootHandle(handle);
                usb_dev_t status = handle->dev_intfs->GetDeviceStatus();
                if ((m_flashdirect && status == DEVICE_PLUGIN) ||
                    status == DEVICE_PST ||
                    status == DEVICE_CHECK) {
                    handle->dev_intfs->SetDeviceStatus(DEVICE_FLASH);
                }
                break;
            }
        }
    }

    AdbDev.clear();
    FbDev.clear();

#if 0
    GetDevLabelByGUID(&GUID_DEVINTERFACE_ADB, SRV_WINUSB, devicePath, false);
    for (iter = devicePath.begin();iter != devicePath.end(); ++ iter){
        LOGI("class %S %S",iter->GetParentIdPrefix(), iter->GetDevPath());
    }
#endif
    return TRUE;
}

BOOL CmdmfastbootDlg::HandleComDevice(VOID) {
    vector<CDevLabel> devicePath;
    GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_JRDUSBSER, devicePath, false);
    //for  COM1, GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //GetDevLabelByGUID(&GUID_DEVCLASS_PORTS , SRV_SERIAL, devicePath, false);
    vector<CDevLabel>::iterator iter;
    DeviceInterfaces* devintf;

    for (iter = devicePath.begin(); iter != devicePath.end();++iter) {
        iter->Dump(__FUNCTION__);
        devintf = mDevCoordinator.AddDevice(*iter, DEVTYPE_DIAGPORT);
    }
    devicePath.clear();
    return TRUE;
}

BOOL CmdmfastbootDlg::RejectCDROM(VOID){
    vector<CDevLabel> devicePath;
    vector<CDevLabel>::iterator iter;
    CSCSICmd scsi = CSCSICmd();
    GetDevLabelByGUID(&GUID_DEVINTERFACE_CDROM, SRV_CDROM, devicePath, false);
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_DISK, SRV_DISK, devicePath, false);

    for(iter = devicePath.begin();iter != devicePath.end(); ++ iter) {
        CString path = iter->GetDevPath();
        iter->Dump( __FUNCTION__ " : " __FILE__);
        if (path.Find(_T("\\\\?\\usbstor#")) == -1) {
            //LOGI("Fix DISK %S:", path);
            continue;
        }
        path.MakeUpper();
        if (path.Find(_T("ONETOUCH")) == -1 && path.Find(_T("ALCATEL")) == -1) {
            //LOGI("USB Stor %S is not alcatel",path);
            continue;
        }

#if 0
        int  devSize = m_WorkDev.size();
        for(int j=0; j < devSize; j++) {
            if (*iter == m_WorkDev[j]) {
                LOGI("Device is have handle, %S", path);
                continue;
            }
        }
#endif

        scsi.SwitchToDebugDevice(path);
        //scsi.SwitchToTPSTDeivce(path);
        //m_WorkDev.push_back(*iter);
    }
    devicePath.clear();
    return TRUE;
}

BOOL CmdmfastbootDlg::SetupDevice(int evt) {
    switch(evt) {
        case TIMER_EVT_ADBKILLED:
            RejectCDROM();
            EnumerateAdbDevice();
            HandleComDevice();
            ScheduleDeviceWork(m_flashdirect);
            break;
        case TIMER_EVT_REJECTCDROM:
            RejectCDROM();
            break;
        case TIMER_EVT_COMPORT:
            HandleComDevice();
            ScheduleDeviceWork(m_flashdirect);
            break;
        case TIMER_EVT_USBADB:
            EnumerateAdbDevice();
            ScheduleDeviceWork(m_flashdirect);
            break;
        default:
            break;
        }
   // if (evt != TIMER_EVT_REJECTCDROM)
   //     ScheduleDeviceWork(m_flashdirect);

    mDevCoordinator.Dump();
    return TRUE;
}
 void CALLBACK CmdmfastbootDlg::DeviceEventTimerProc(
   HWND hWnd,      // handle of CWnd that called SetTimer
   UINT nMsg,      // WM_TIMER
   UINT_PTR nIDEvent,   // timer identification
   DWORD dwTime    // system time
)
{
    CmdmfastbootDlg  *dlg = (CmdmfastbootDlg*)hWnd ;
    ::KillTimer(hWnd, nIDEvent);//must kill timer explicit, otherwise, the timer will expire periodically.
    dlg->SetupDevice(nIDEvent) ;
}


UINT CmdmfastbootDlg::adb_shell_command(adbhost& adb, UsbWorkData* data, PCCH command,
                                              UI_INFO_TYPE info_type)
{
  PCHAR resp = NULL;
  int  resp_len;
  int ret;
  ret = adb.shell(command, (void **)&resp, &resp_len);
  if (ret ==0 && resp != NULL) {
    if (UI_DEFAULT == info_type) {
      data->ui_text_msg(PROMPT_TITLE, command);
      data->ui_text_msg(PROMPT_TEXT, resp);
    } else {
      data->ui_text_msg(info_type, resp);
    }
    free(resp);
    return 0;
  }

  return -1;
}

//"nv read %d" , id
//"nv write %d %s" ,id, value
UINT CmdmfastbootDlg::adb_update_NV(adbhost& adb, UsbWorkData* data,  flash_image  *const image) {
  unsigned int index = 0;
  const char* cmd;
    PCHAR resp = NULL;
  int  resp_len;
  int ret;

  if (data == NULL || NULL == image) {
    ERROR("Bad parameter.");
    return -1;
  }

  if (!data->update_qcn) {
    INFO("Do not need update qcn.");
    return -1;
  }
  //TODO:: first enter offline-mode.

  if (image->qcn_cmds_enum_init(NULL) == FALSE) {
    ERROR("qcn_cmds_enum_init FAILED.");
    return -1;
  }

  while ((cmd = image->qcn_cmds_enum_next(index)) != NULL) {
    //DEBUG("cmd is %s", cmd);
    ret = adb.shell(cmd, (void **)&resp, &resp_len);
    FREE_IF(resp);
   index++;
  }

  return 0;
}

UINT CmdmfastbootDlg::adb_write_IMEI(adbhost& adb, UsbWorkData* data) {
    PCHAR resp = NULL;
    int  resp_len;
    int ret;
    ret = adb.shell("nv write 15002 1", (void **)&resp, &resp_len);

    FREE_IF(resp);

    //imei 15 letters.
    adb.shell("imeiop write 860440020101686", (void **)&resp, &resp_len);
    FREE_IF(resp);
    ret = adb.shell("nv write 15002 0", (void **)&resp, &resp_len);
    FREE_IF(resp);
    return 0;
}

//#define VERSION_CMP_TEST
UINT CmdmfastbootDlg::adb_hw_check(adbhost& adb, UsbWorkData* data) {
  PCHAR resp = NULL;
  int  resp_len;
  int ret;
  ret = adb.shell("hwinfo_check", (void **)&resp, &resp_len);
  if (ret ==0 && resp != NULL) {
    ret = strcmp(resp, "match");
    if (ret == 0) {
      data->ui_text_msg( PROMPT_TEXT, "hardware is match!");
    } else {
      data->ui_text_msg(PROMPT_TITLE, "hwinfo_check return ");
      data->ui_text_msg(PROMPT_TEXT, resp);
      WARN("hwinfo_check return \"%s\"", resp);
    }

    free(resp);
 #ifdef VERSION_CMP_TEST
     return 0;
 #else
    return ret;
 #endif
  } else {
    return -1;
  }
}

UINT CmdmfastbootDlg::adb_sw_version_cmp(adbhost& adb, UsbWorkData* data){
  PCHAR resp = NULL;
  int  resp_len;
  int ret;
  ret = adb.shell("swinfo_compare", (void **)&resp, &resp_len);

  //test code
  #ifdef VERSION_CMP_TEST
  free(resp);
  resp = strdup("A5:match,Q6:match,QCN:mismatch");
  #endif

  if (ret ==0 && resp != NULL) {
    //A5:mismatch,Q6:match,QCN:mismatch
    // strtok_s OR strtok split string by replcae delimited char into '\0'
    // so constant string is not applied.
    char *result, *sub_result, *token, *subtoken;
    char *context, *sub_context;

    for (result = resp; ; result = NULL) {
      token = strtok_s(result, ",", &context);
      if (token == NULL) {
        ERROR("%s contain none \",\"", result);
        break;
      }

      for (sub_result = token; ; sub_result = NULL) {
        subtoken = strtok_s(sub_result, ":", &sub_context);
        if (subtoken == NULL) {
          ERROR("%s contain none \":\"", sub_result);
        } else {
          ret+=sw_version_parse(data, sub_result, sub_context);
        }
        break;
      }
    }
    free(resp);
  }
  return ret;

}

UINT CmdmfastbootDlg::sw_version_parse(UsbWorkData* data,PCCH key, PCCH value) {
   PWCHAR a5_partition[] = {L"boot", L"system", L"modem", L"aboot", L"jrdresource", L"rpm",L"efs2",L"cdrom",L"tz",L"sbl"};
   PWCHAR q6_partition[] = {L"dsp1", L"dsp2", L"dsp3", L"mibib", L"sbl2", L"adsp", L"qdsp", L"mba",  L"sdi"};
   PWCHAR *partition;
   int count,i;
  // int index;

   if (stricmp(value, "mismatch")) {
    INFO("Not valid value %s", value);
    return 0;
   }

  if (stricmp(key , "a5") == 0) {
    partition = a5_partition;
    count = sizeof(a5_partition)/ sizeof(a5_partition[0]);
    data->ui_text_msg(PROMPT_TEXT, "A5 firmware can update");
  } else if (stricmp(key , "q6") == 0) {
    partition = q6_partition;
    count = sizeof(q6_partition)/ sizeof(q6_partition[0]);
    data->ui_text_msg(PROMPT_TEXT, "Q6 firmware can update.");
  } else if (stricmp(key , "qcn") == 0) {
    data->ui_text_msg(PROMPT_TEXT, "QCN can update.");
    data->update_qcn = TRUE;
    return 1;
  } else {
    ERROR("Not valid key %d", key);
    return 0;
  }

  //for (index = 0;  index < PARTITION_NUM_MAX; index++)
  //  if (NULL == data->flash_partition[index])
  //    break;

  for (i =0; i < count && data->partition_nr < PARTITION_NUM_MAX; i++) {
     data->flash_partition[data->partition_nr] =
    data->hWnd->m_image->get_partition_info(*(partition+i), NULL, NULL);
     if (data->flash_partition[data->partition_nr] != NULL) {
      data->partition_nr++;
     } else {
      ERROR("Partition %S : no data available.", *(partition+i));
     }
  }

  return 1;
}

/*
* update flow is done here. Do update business logically.
*/
UINT CmdmfastbootDlg::usb_work(LPVOID wParam) {
    UsbWorkData* data = (UsbWorkData*)wParam;
    usb_handle * handle;
    flash_image  *img;
    DeviceInterfaces *dev;
    int result;
    BOOL ignore_version;
    BOOL force_update;
    usb_dev_t status ;

    if (data == NULL || data->usb == NULL ||  data->hWnd == NULL ||
        data->hWnd->m_image == NULL) {
        ERROR("Bad parameter");
        return -1;
    }

    dev = data->devIntf;
    handle = data->usb;
    img = data->hWnd->m_image;
    //TODO:: an option for this is better., such as force_update.
    //when m_flashdirect set , force_update must set.
    ignore_version = data->hWnd->m_flashdirect;
    force_update = data->hWnd->m_forceupdate;
    status = dev->GetDeviceStatus();

    data->ui_text_msg(TITLE, dev->GetDevTag());
    if (status == DEVICE_PLUGIN) {
        DiagPST pst(data);
    if(!pst.DownloadCheck())    {
dev->SetDeviceStatus(DEVICE_CHECK);
return 0;
    }

    if(!pst.RunTimeDiag())
    {
dev->SetDeviceStatus(DEVICE_CHECK);
return 0;
    }
    if(!pst.Calculate_length())
    {
dev->SetDeviceStatus(DEVICE_CHECK);
return 0;
    }

    if(!pst.DownloadPrg())
    {
dev->SetDeviceStatus(DEVICE_CHECK);
return 0;
    }
    if(!pst.DownloadImages())
    {
dev->SetDeviceStatus(DEVICE_CHECK);
return 0;
    }

        dev->SetDeviceStatus(DEVICE_CHECK);
    } else if (status == DEVICE_CHECK) {
        adbhost adb(handle , dev->GetDevId());
        const wchar_t *conf_file = img->get_package_config();
        char *conf_file_char ;

        adb_shell_command(adb,data, "cat /proc/version",LINUX_VER);
        adb_shell_command(adb,data, "cat /etc/version",SYSTEM_VER);
        adb_shell_command(adb,data, "cat /usr/version",USERDATA_VER);

        if (force_update) {
            sw_version_parse(data, "a5", "mismatch");
            sw_version_parse(data, "q6", "mismatch");
            sw_version_parse(data, "qcn", "mismatch");
        } else {
            if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(conf_file)) {
                data->ui_text_msg(ADB_CHK_ABORT, "no config.xml in the package, abort!");
                return -1;
            }

            conf_file_char = WideStrToMultiStr(conf_file);
            if (conf_file_char == NULL) {
                data->ui_text_msg(ADB_CHK_ABORT, "out of memory, abort!");
                return -1;
            }

            adb.sync_push(conf_file_char, "/tmp/config.xml");
            data->ui_text_msg(PROMPT_TEXT, "Copy config.xml to /tmp/config.xml.");
            delete conf_file_char;

            result = adb_hw_check(adb,data);
            if (result != 0) {
                data->ui_text_msg(ADB_CHK_ABORT, "hardware check failed, abort!");
                return -1;
            }

            result = adb_sw_version_cmp(adb,data);
            if (result < 0) {
                data->ui_text_msg(ADB_CHK_ABORT, "firmware version check failed, abort!");
                return -1;
            } else if (result ==0) {
                data->ui_text_msg(FLASH_DONE, "firmware is the NEWEST. DO NOT UPDATE.");
                return 0;
            }
        }

        //prepare , do something that before flash image.
        //adb_update_NV(adb, data, img);
        //adb_write_IMEI(adb, data);
        //adb_shell_command(adb,data, "trace -r");
        //adb_shell_command(adb,data, "backup");
        if (data->partition_nr > 0) {
            adb.reboot_bootloader(m_module_name);
            data->ui_text_msg(REBOOT_DEVICE, "reboot bootloader");
        } else {
            // that is mean just update qcn
            data->ui_text_msg(FLASH_DONE, "Finish. QCN updated.");
        }
    } else if (status == DEVICE_FLASH) {
        fastboot fb(handle);
        FlashImageInfo const * image;

        fb.fb_queue_display("product","product");
        fb.fb_queue_display("version","version");
        fb.fb_queue_display("serialno","serialno");
        fb.fb_queue_display("kernel","kernel");

        for (int index = 0; index < data->partition_nr; index++) {
            image = data->flash_partition[index];
            if (image->need_download) {
                fb.fb_queue_flash(image->partition_str, image->data, image->size);
            }
        }

        if (data->partition_nr == 0 && ignore_version) {
            image = img->image_enum_init();

            for(;image != NULL ; ) {
                if (image->need_download) {
                    fb.fb_queue_flash(image->partition_str, image->data, image->size);
                }
                image = img->image_enum_next(image);
            }
        }

        //  fb.fb_queue_reboot();
        fb.fb_execute_queue(handle, data);

        data->ui_text_msg(FLASH_DONE, NULL);
        dev->SetDeviceStatus(DEVICE_REMOVED);
    }
    return 0;
}


BOOL CmdmfastbootDlg::ScheduleDeviceWork(BOOL flashdirect) {
    usb_handle* handle;
    UsbWorkData* data;

    if (!m_bWork) {
        // INFO("do not work now.");
        return FALSE;
    }

    /*
     * Get usbworkdata that in free or in switch.
     * fix_map means whether logical port (portstatui) fix map physical port.
     * this feature will be useful when multiport port download, it help operator
     * to make sure whether the device in a specific is flashed.
     */
    DeviceInterfaces* idleDev;
    UsbWorkData* workdata;
    BOOL NonePort;
    while(NULL != (idleDev = mDevCoordinator.GetValidDevice())) {
        NonePort= TRUE;
        for (int i=0; i < m_nPort; i++) {
            workdata = m_workdata[i];
            if (!workdata->IsIdle())
                continue;

            DeviceInterfaces* item = m_workdata[i]->devIntf;
            if (m_fix_port_map == FALSE || item == NULL || item->Match(idleDev)) {
                workdata->Start(idleDev, work_timeout, flashdirect);
                NonePort = FALSE;
                break;
            }
        }
        if (NonePort)
            break;
    }

    return TRUE;
}


void CmdmfastbootDlg::OnSize(UINT nType, int cx, int cy)
{
  int dx, dy, dw, dh;
  //Remove for overland window issue by zhanghao 20160112
  /*
  if (m_nPort > 1 && (800 > cx || 800 > cy))
  {
    return;
  } else if (m_nPort == 1 && ( 500 > cx || 400 > cy)) {
	  INFO("x=%s  ", "112299");
    return;
  }*/
  //end to remove.
  CDialog::OnSize(nType, cx, cy);

  if (m_bInit) {
    RECT rect;

    GetDlgItem(IDC_GRP_PKG_INFO)->GetClientRect(&rect);

    if (m_nPort == 1) {
      dx = rect.right + 20;
      if (m_pack_img ) {
        dy = cy - 100;
        SetDlgItemPos(IDC_BTN_STOP, dx+240, dy);
        SetDlgItemPos(IDCANCEL, dx + 120, dy);
        SetDlgItemPos(IDC_BTN_START, dx , dy);
        //SetDlgItemPos(IDC_SETTING, dx, dy);

        dy = 5;
        dw = cx - dx - 10;
        dh = cy -dy - 120;
      } else {
        dy = cy - 50;
        SetDlgItemPos(IDC_BTN_BROWSE, cx - 80, dy);
        SetDlgItemPos(IDC_EDIT_PACKAGE_PATH, 60, dy);
        SetDlgItemPos(IDC_STATIC_PKG, 00, dy);

        dy = cy - 100;
        SetDlgItemPos(IDC_BTN_STOP, dx+240, dy);
        SetDlgItemPos(IDCANCEL, dx + 120, dy);
        SetDlgItemPos(IDC_BTN_START, dx , dy);
        //SetDlgItemPos(IDC_SETTING, dx, dy);

        dy = 5;
        m_SetDlg.SetWindowPos(NULL, dx, dy, 280, 220, 0);

        dy = 230;
        dw = cx - dx - 10;
        dh = cy -dy - 100 -20;
      }
    } else {
      dx = rect.right + 8;
      dy = cy - 90;
      SetDlgItemPos(IDC_BTN_STOP, (cx + 800) /2-200, dy);
      SetDlgItemPos(IDCANCEL, (cx + 1000 ) /2-200, dy);
      SetDlgItemPos(IDC_BTN_START, (cx + 500) /2-200, dy);
      //SetDlgItemPos(IDC_SETTING, (cx +300) /2, cy - 60);
      SetDlgItemPos(IDC_BTN_BROWSE, (cx -120) /2, dy);
      SetDlgItemPos(IDC_EDIT_PACKAGE_PATH, 100,dy);
      SetDlgItemPos(IDC_STATIC_PKG, 20, dy);

      dy = rect.bottom + 20;

      if (m_pack_img == FALSE) {
        m_SetDlg.SetWindowPos(NULL, rect.left, dy, 280, 220, 0);
      }
	  m_SetDlg.ShowWindow(SW_HIDE);   //hide set dialog by zhanghao 20160112

      dy = 5;
      dw = cx - dx - 10;
      dh = cy -dy - 20 -70;
    }

    SetPortDialogs(dx, dy, dw, dh);
  }
  //Invalidate(TRUE);
}
//Remove by zhang hao 20160112
/*void CmdmfastbootDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  if (m_nPort > 1) {
    lpMMI->ptMinTrackSize.x   = 800 ;
    lpMMI->ptMinTrackSize.y   = 800 ;
  } else {
    if (m_pack_img ) {
      lpMMI->ptMaxSize.x   = 700 ;
      lpMMI->ptMaxSize.y   = 500 ;
      lpMMI->ptMaxTrackSize.x   = 700 ;
      lpMMI->ptMaxTrackSize.y   = 500 ;
      lpMMI->ptMinTrackSize.x   = 700 ;
      lpMMI->ptMinTrackSize.y   = 500 ;
    } else {
      lpMMI->ptMaxSize.x   = 700 ;
      lpMMI->ptMaxSize.y   = 650 ;
      lpMMI->ptMaxTrackSize.x   = 700 ;
      lpMMI->ptMaxTrackSize.y   = 650 ;
      lpMMI->ptMinTrackSize.x   = 700 ;
      lpMMI->ptMinTrackSize.y   = 650 ;
    }
  }
  CDialog::OnGetMinMaxInfo(lpMMI);
}
*/
void CmdmfastbootDlg::OnBnClickedBtnBrowse()
{
#if 0
  // TODO: 在此添加控件通知处理程序代码
  CString str = _T("Img File (*.img)|*.img|All Files (*.*)|*.*||");

  CFileDialog cfd( TRUE,
                  NULL,
                  NULL,
                  OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
                  (LPCTSTR)str,
                  NULL);
  CDirDialog

    if (cfd.DoModal() == IDOK)
    {
      m_PackagePath = cfd.GetPathName();
      UpdateData(FALSE);
    }
#endif


  BROWSEINFO bi;

  ZeroMemory(&bi,sizeof(BROWSEINFO));
  //bi.pidlRoot = SHParsePidlFromPath("E:\\");
  bi.hwndOwner=GetSafeHwnd();
  bi.pszDisplayName= m_PackagePath.GetBuffer(MAX_PATH);
  bi.lpszTitle=L"Select Package folder";
  bi.ulFlags=BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
  LPITEMIDLIST idl= SHBrowseForFolder(&bi);
  if(idl==NULL)
    return;
  m_PackagePath.ReleaseBuffer();
  SHGetPathFromIDList(idl,m_PackagePath.GetBuffer(MAX_PATH));
  m_PackagePath.ReleaseBuffer();

  if(m_PackagePath[m_PackagePath.GetLength()-1]!=L'\\')
    m_PackagePath+=L'\\';

  LPMALLOC pMalloc;
  if(SUCCEEDED(SHGetMalloc(&pMalloc)))
  {
    pMalloc->Free(idl);
    pMalloc->Release();
  }

  if( m_image->set_package_dir(m_PackagePath.GetBuffer(MAX_PATH),
                               m_ConfigPath.GetBuffer(MAX_PATH)))
  {

    delete m_image;
    m_image =  new flash_image(m_ConfigPath.GetString());
    UpdatePackageInfo();
  } else {
    AfxMessageBox(L"Package path is not change!", MB_ICONEXCLAMATION);
  }
}

void CmdmfastbootDlg::OnBnClickedStart()
{
	if (m_imglist->GetItemCount()<1)
	{
		AfxMessageBox(L"Please select a valid package directory!",MB_OK);
		return;
	}
	m_imglist->EnableWindow(FALSE);
	GetMenu()->EnableMenuItem(ID_FILE_M850, MF_DISABLED|MF_GRAYED);
	GetMenu()->EnableMenuItem(ID_FILE_M801, MF_DISABLED|MF_GRAYED);
	if (SetWorkStatus(TRUE, FALSE)) {
	//AdbUsbHandler(true);
    ScheduleDeviceWork(m_flashdirect);
	}

  //::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void CmdmfastbootDlg::OnBnClickedButtonStop()
{
	m_imglist->EnableWindow(TRUE);
	GetMenu()->EnableMenuItem(ID_FILE_M850, MF_ENABLED);
	GetMenu()->EnableMenuItem(ID_FILE_M801, MF_ENABLED);
    SetWorkStatus(FALSE, FALSE);
}

void CmdmfastbootDlg::OnClose()
{
  //CDialog::OnClose();

  if (IsHaveUsbWork())
  {
    int iRet = AfxMessageBox(L"Still have active downloading! Exit anyway?",
    MB_YESNO|MB_DEFBUTTON2);
    if (IDYES!=iRet)
    {
      return;
    }
  }
  OnCancel();
}

void CmdmfastbootDlg::OnBnClickedCancel()
{
  DWORD dwExitCode = 0;

  if (IsHaveUsbWork())
  {
    int iRet = AfxMessageBox(L"Still have active downloading! Exit anyway?",
                    MB_YESNO|MB_DEFBUTTON2);
    if (IDYES!=iRet)
    {
      return;
    }
  }

  OnCancel();

}

void CmdmfastbootDlg::OnDestroy()
{
	CDialog::OnDestroy();

	::RemoveProp(m_hWnd, JRD_MDM_FASTBOOT_TOOL_APP);	//for single instance
	//Shutdown the thread pool
	//m_dlWorkerPool.Shutdown();
  delete m_image;

	StopLogging();
}


void CmdmfastbootDlg::OnBnClickedSetting()
{
	//m_pMainWnd = &dlg;
	m_SetDlg.Create(IDD_SETTINGS, NULL);
	    m_SetDlg.ModifyStyle(
              WS_CHILD | WS_VISIBLE | DS_CENTER,
              WS_POPUP | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU,
              SWP_SHOWWINDOW);
# if 0
	INT_PTR nResponse = m_SetDlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{

	}
  #endif
}


void CmdmfastbootDlg::OnSizing(UINT fwSide, LPRECT pRect)
	{
	//CDialog::OnSizing(fwSide, pRect);

	// TODO: 在此处添加消息处理程序代码
	}

void CmdmfastbootDlg::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
	{
	// TODO: 在此添加专用代码和/或调用基类

	CDialog::HtmlHelp(dwData, nCmd);
	}

void CmdmfastbootDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
	}

void CmdmfastbootDlg::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString strPartitionName;
	bool bSelected = false;
	bool bSelectStatChange = false;

    if (!m_UpdateDownloadFlag)
        return;

	if((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(1)) /* old state : unchecked */
		&& (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(2)) /* new state : checked */
		)
	{
		bSelectStatChange = true;
		bSelected = true;
		ERROR("Item %d is checked", pNMLV->iItem);
	}
	else if((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(2)) /* old state : checked */
		&& (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(1)) /* new state : unchecked */
		)
	{
		bSelectStatChange = true;
		bSelected = false;
		ERROR("Item %d is unchecked", pNMLV->iItem);
	}
	else
	{
		ERROR("Item %d does't change the check-status", pNMLV->iItem);
	}

	if (bSelectStatChange)
	{
		strPartitionName = m_imglist->GetItemText(pNMLV->iItem, 0);
		m_image->set_download_flag(strPartitionName, bSelected);
		WritePrivateProfileString(PARTITIONTBL_DL,strPartitionName.GetBuffer(),bSelected?L"1":L"0",m_ConfigPath.GetBuffer());
	}

	*pResult = 0;
}

void CmdmfastbootDlg::UpdatePackage()
{
	if (m_PackagePath.IsEmpty())
	{
		return;
	}
	m_image->set_package_dir(m_PackagePath.GetBuffer(),
		m_ConfigPath.GetBuffer(MAX_PATH));
	if (NULL!=m_image)
	{
		delete m_image;
	}
	m_image =  new flash_image(m_ConfigPath.GetString());
	UpdatePackageInfo();
}

void CmdmfastbootDlg::OnFileM850()
{
	CopyFile(L".\\mdmconfig_M850.ini", L".\\mdmconfig.ini", FALSE);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYPOSITION|MF_CHECKED);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(1,MF_BYPOSITION|MF_UNCHECKED);
	InitSettingConfig();
	UpdatePackage();
}

void CmdmfastbootDlg::OnFileM801()
{
	CopyFile(L".\\mdmconfig_M801.ini", L".\\mdmconfig.ini", FALSE);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(1,MF_BYPOSITION|MF_CHECKED);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYPOSITION|MF_UNCHECKED);
	InitSettingConfig();
	UpdatePackage();
}
