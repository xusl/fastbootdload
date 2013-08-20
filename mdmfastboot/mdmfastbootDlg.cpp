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
#include "devguid.h"



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

static UINT usb_work(LPVOID wParam);

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
  m_bInit = FALSE;
  InitSettingConfig();
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


BOOL CmdmfastbootDlg::SetPortDialogs(UINT nType, int x, int y,  int w, int h)
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

      port = &(m_workdata + r * C_NUM + c)->ctl;
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

BOOL CmdmfastbootDlg::InitUsbWorkData(void)
{
  UsbWorkData* workdata;
  int i= 0;
  for (; i < m_nPort; i++) {
    workdata = m_workdata + i;
    workdata->hWnd = this;
    workdata->ctl.Create(IDD_PORT_STATE, this);
    workdata->ctl.Init(i);
    workdata->usb = NULL;
    workdata->usb_sn = ~1L;
    workdata->stat = USB_STAT_IDLE;
    //workdata->img = m_image;
    workdata->work = NULL;
    ZeroMemory(workdata->flash_partition, sizeof(workdata->flash_partition));
  }
  return TRUE;
}


BOOL CmdmfastbootDlg::IsHaveUsbWork(void) {
  UsbWorkData* workdata;
  int i= 0;
  for (; i < m_nPort; i++) {
    workdata = m_workdata + i;
    if (workdata->stat == USB_STAT_SWITCH ||
        workdata->stat == USB_STAT_WORKING)
        return TRUE ;
  }
  return FALSE;
}

UINT CmdmfastbootDlg::UsbWorkStat(UsbWorkData *data) {
  if (data == NULL) {
    ERROR("Invalid parameter");
    return FALSE;
  }

  return data->stat;
}

BOOL CmdmfastbootDlg::FinishUsbWorkData(UsbWorkData *data) {
  if (data == NULL) {
    ERROR("Invalid parameter");
    return FALSE;
  }
  data->stat = USB_STAT_FINISH;
  //data->work = NULL;
  //KILL work timer.
  KillTimer(data->usb_sn);
  return TRUE;
}

BOOL CmdmfastbootDlg::AbortUsbWorkData(UsbWorkData *data) {
  if (data == NULL) {
    ERROR("Invalid parameter");
    return FALSE;
  }
  data->stat = USB_STAT_ERROR;
  KillTimer(data->usb_sn);
  return TRUE;
}

BOOL CmdmfastbootDlg::CleanUsbWorkData(UsbWorkData *data) {
  if (data == NULL) {
    ERROR("Invalid parameter");
    return FALSE;
  }

  data->usb = NULL;
  data->usb_sn = ~1L;
  data->stat = USB_STAT_IDLE;
  data->work = NULL;
  data->ctl.Reset();
  ZeroMemory(data->flash_partition, sizeof(data->flash_partition));

  INFO("Schedule next work!");
  AdbUsbHandler(FALSE);

  return TRUE;
}

BOOL CmdmfastbootDlg::SetUsbWorkData(UsbWorkData *data, usb_handle * usb) {
  if (data == NULL || usb == NULL) {
    ERROR("Invalid parameter");
    return FALSE;
  }

  if (data->stat = USB_STAT_SWITCH) {
    //clear switch timer
    KillTimer(usb_port_address(usb));
  }

  usb_set_work(usb, TRUE);
  data->usb = usb;
  data->usb_sn =usb_port_address(usb);
  data->stat = USB_STAT_WORKING;
  data->work = AfxBeginThread(usb_work, data);

  if (data->work != NULL) {
    data->work->m_bAutoDelete = TRUE;
    SetTimer(data->usb_sn, work_timeout * 1000, NULL);
  } else {
    CRITICAL("Can not begin thread!(0x%x)", data->usb_sn);
    usb_set_work(usb, FALSE);
    CleanUsbWorkData(data);
  }
  return TRUE;
}

BOOL CmdmfastbootDlg::SwitchUsbWorkData(UsbWorkData *data) {
  if (data == NULL) {
    ERROR("Invalid parameter");
    return FALSE;
  }
  data->usb = NULL;
  data->work = NULL;
  data->stat = USB_STAT_SWITCH;
  /*Set switch timeout*/
  SetTimer(data->usb_sn, switch_timeout * 1000, NULL);
  return TRUE;
}

/*
* Get usbworkdata that in free or in switch.
*/
UsbWorkData * CmdmfastbootDlg::GetUsbWorkData(long usb_sn) {
  int i= 0;

  // first search the before, for switch device.
  for (; i < m_nPort; i++) {
    //if (m_workdata[i].usb_sn == usb_sn && m_workdata[i].usb == NULL)
    if ((m_workdata +i)->stat == USB_STAT_SWITCH && (m_workdata + i)->usb_sn == usb_sn)
      return m_workdata + i;
  }

  for (i=0; i < m_nPort; i++) {
    //if (m_workdata[i].usb_sn == ~1L && m_workdata[i].usb == NULL)
    if ((m_workdata + i)->stat == USB_STAT_IDLE)
      return m_workdata + i;
  }

  return NULL;
}

/*
* get usbworkdata by usb_sn, so the device is in switch or in working
* or done?
*/
UsbWorkData * CmdmfastbootDlg::FindUsbWorkData(long usb_sn) {
  int i= 0;

  // first search the before, for switch device.
  for (; i < m_nPort; i++) {
    if ((m_workdata+i)->usb_sn == usb_sn)
      return m_workdata + i;
  }

  return NULL;
}

BOOL CmdmfastbootDlg::UpdatePackageInfo(void) {
  const FlashImageInfo* img = m_image->image_enum_init();
  int item = 0;
  m_imglist->DeleteAllItems();
  for(;img != NULL; ) {
    m_imglist->InsertItem(item,img->partition);
    m_imglist->SetItemText(item,1,img->lpath);
    item ++;
    img = m_image->image_enum_next(img);
  }

  m_image->get_pkg_a5sw_kern_ver(m_LinuxVer);
  m_image->get_pkg_qcn_ver(m_QCNVer);
  m_image->get_pkg_fw_ver(m_FwVer);

  UpdateData(FALSE);
  return TRUE;
}

BOOL CmdmfastbootDlg::SetWorkStatus(BOOL bwork, BOOL bforce) {
  if(!bforce && m_bWork == bwork) {
    WARN("Do not need to chage status.");
    return FALSE;
  }

  GetDlgItem(IDC_BTN_START)->EnableWindow(!bwork);
  GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(!bwork);
  GetDlgItem(IDC_BTN_STOP)->EnableWindow(bwork);
  m_bWork = bwork;
  return TRUE;

}

BOOL CmdmfastbootDlg::InitSettingConfig()
{
  LPCTSTR lpFileName;

  int data_len;

  int auto_work;

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


  m_image = new flash_image(lpFileName);

  m_schedule_remove = GetPrivateProfileInt(L"app", L"schedule_remove",1,lpFileName);
  switch_timeout = GetPrivateProfileInt(L"app", L"switch_timeout", 300,lpFileName);
  work_timeout = GetPrivateProfileInt(L"app", L"work_timeout",600,lpFileName);

  m_flashdirect = GetPrivateProfileInt(L"app", L"flashdirect",1,lpFileName);

  m_nPort = GetPrivateProfileInt(L"app", L"port_num",1,lpFileName);
  m_nPortRow = GetPrivateProfileInt(L"app", L"port_row",1,lpFileName);

  m_nPort = m_nPort > PORT_NUM_MAX ?  PORT_NUM_MAX : m_nPort;
  m_nPortRow = m_nPortRow > m_nPort ? m_nPort :  m_nPortRow;

  auto_work = GetPrivateProfileInt(L"app",L"autowork", 0, lpFileName);
  m_bWork = auto_work;

  if(log_file) free(log_file);
  if(log_tag) delete log_tag;
  if(log_level) delete log_level;

  return TRUE;
}

// CmdmfastbootDlg 消息处理程序

BOOL CmdmfastbootDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  ::SetProp(m_hWnd, JRD_MDM_FASTBOOT_TOOL_APP, (HANDLE)1);//for single instance

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

  InitUsbWorkData();
  SetUpAdbDevice(NULL, 0);

  m_bInit = TRUE;
  ShowWindow(SW_MAXIMIZE);

  //init thread pool begin.
  HRESULT hr = m_dlWorkerPool.Initialize(NULL, THREADPOOL_SIZE);
  m_dlWorkerPool.SetTimeout(30 * 1000);
  if(!SUCCEEDED(hr))
  {
    ERROR("Failed to init thread pool!");
    return FALSE;
  }
  //init thread pool end.

  m_port = ((CListCtrl*)GetDlgItem(IDC_LIST_PORT));

  m_port->InsertColumn(0, _T("Composite"),LVCFMT_LEFT, 100);
  m_port->InsertColumn(1, _T("Adb"), LVCFMT_LEFT, 100);
  build_port_map(m_port);


m_imglist = ((CListCtrl*)GetDlgItem(IDC_IMAGE_LIST));
m_imglist->InsertColumn(0, _T("Partition"),LVCFMT_LEFT, 50);
m_imglist->InsertColumn(1, _T("File Name"),LVCFMT_LEFT, 600);

  //GetDlgItem(IDC_EDIT_PACKAGE_PATH)->SetWindowText(m_image->get_package_dir());
  m_PackagePath = m_image->get_package_dir();
  UpdatePackageInfo();

  //注释设备通知，不能放在构造函数，否则 RegisterDeviceNotification 返回78.
  RegisterAdbDeviceNotification();
  SetWorkStatus(m_bWork, TRUE);
  adb_usb_init();
  //SetUpAdbDevice(NULL, 0);
  AdbUsbHandler(true);

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
    WARN("OnDeviceChange, dwData == 0 .EventType: 0x%x", nEventType);
    return FALSE;
  }

  DEV_BROADCAST_HDR* phdr = (DEV_BROADCAST_HDR*)dwData;
  PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)phdr;

  DEBUG("OnDeviceChange, EventType: 0x%x, DeviceType 0x%x",
        nEventType, phdr->dbch_devicetype);

  if (nEventType == DBT_DEVICEARRIVAL) {
    switch( phdr->dbch_devicetype ) {
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
        SetUpAdbDevice(pDevInf, dwData);
        AdbUsbHandler(true);
        break;
      }
    }
  } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
    if (phdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

      ASSERT(lstrlen(pDevInf->dbcc_name) > 4);

      long sn = usb_host_sn(pDevInf->dbcc_name);
      sn = get_adb_composite_device_sn(sn);
      UsbWorkData * data = FindUsbWorkData(sn);
      UINT stat;

      if (data == NULL) {
        WARN("Can not find usbworkdata for %d, schedule remove is %d", sn, m_schedule_remove);
        return FALSE;
      }

      stat = UsbWorkStat(data);
      if (stat == USB_STAT_WORKING) {
        // the device is plugin off when in working, that is because some accident.
        //the accident power-off in switch is handle by the timer.
        //thread pool notify , exit
        if (data->work != NULL)
            data->work->PostThreadMessage( WM_QUIT, NULL, NULL );
        usb_close(data->usb);
      } else if (stat == USB_STAT_FINISH) {
         if (!m_schedule_remove) {
            ERROR("We do not set m_schedule_remove, "
                "but in device remove event we can found usb work data");
            return TRUE;
        }
      } else if (stat == USB_STAT_ERROR) {
        usb_close(data->usb);
      } else {
        return TRUE;
      }

        CleanUsbWorkData(data);
    }
  }

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
    // remove manually, do not schedule next device into this UI port.
    AbortUsbWorkData(data);
    data->ctl.SetInfo(PROMPT_TEXT, uiInfo->sVal);
    break;

  case REBOOT_DEVICE:
    if (data->usb != NULL) {
        usb_switch_device(data->usb);
        usb_close(data->usb);
    }

    SwitchUsbWorkData(data);
    data->ctl.SetInfo(PROMPT_TEXT, uiInfo->sVal);
    break;

  case FLASH_DONE:
    usb_close(data->usb);
    FinishUsbWorkData(data);

    if (!m_schedule_remove) {
    // schedule next port now, and when  app receice device remove event,
    // the current finished port is not in workdata set.
        sleep(1);
        CleanUsbWorkData(data);
    }
    //data->ctl.SetInfo(PROMPT_TEXT, uiInfo->sVal);
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
  UsbWorkData * data = FindUsbWorkData(nIDEvent);
  WARN("PORT %d switch device timeout", nIDEvent);

  if (data == NULL) {
    ERROR("PORT %d switch device timeout", nIDEvent);
    return ;
  }

  if (data->stat = USB_STAT_SWITCH) {
    remove_switch_device(nIDEvent);
  } else if (data->stat = USB_STAT_WORKING){
    if (data->work != NULL) //Delete, or ExitInstance
      data->work->PostThreadMessage( WM_QUIT, NULL, NULL );
    usb_close(data->usb);
  }

  CleanUsbWorkData(data);
  KillTimer(data->usb_sn);
}


UINT CmdmfastbootDlg::ui_text_msg(UsbWorkData* data, UI_INFO_TYPE info_type, PCCH msg) {
  UIInfo* info = new UIInfo();

  info->infoType = info_type;
  info->sVal = msg;
  data->hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info,
                          (LPARAM)data);
  return 0;
}

UINT CmdmfastbootDlg::do_adb_shell_command(adbhost& adb, UsbWorkData* data, PCCH command,
                                              UI_INFO_TYPE info_type)
{
  PCHAR resp = NULL;
  int  resp_len;
  int ret;
  ret = adb.shell(command, (void **)&resp, &resp_len);
  if (ret ==0 && resp != NULL) {
    if (UI_DEFAULT == info_type) {
      ui_text_msg(data, PROMPT_TITLE, command);
      ui_text_msg(data, PROMPT_TEXT, resp);
    } else {
      ui_text_msg(data, info_type, resp);
    }
    free(resp);
    return 0;
  }

  return -1;
}

#define VERSION_CMP_TEST
UINT CmdmfastbootDlg::adb_hw_check(adbhost& adb, UsbWorkData* data) {
  PCHAR resp = NULL;
  int  resp_len;
  int ret;
  ret = adb.shell("hwinfo_check", (void **)&resp, &resp_len);
  if (ret ==0 && resp != NULL) {
    ret = strcmp(resp, "match");
    if (ret == 0) {
      ui_text_msg(data, PROMPT_TEXT, "hardware is match!");
    } else {
      ui_text_msg(data, PROMPT_TITLE, "hwinfo_check return ");
      ui_text_msg(data, PROMPT_TEXT, resp);
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
  resp = strdup("A5:mismatch,Q6:match,QCN:match");
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
    // ui_text_msg(data, PROMPT_TITLE, command);
    //ui_text_msg(data, PROMPT_TEXT, resp);
    //ret = ret >= 0 ? 0 : ret;
    free(resp);
  }
  return ret;

}

UINT CmdmfastbootDlg::sw_version_parse(UsbWorkData* data,PCCH key, PCCH value) {
   PWCHAR a5_partition[] = {L"boot", L"system", L"userdata", L"aboot"};
   PWCHAR q6_partition[] = {L"dsp1", L"dsp2", L"dsp3", L"mibib", L"sbl2", L"rpm"};
   PWCHAR *partition;
   int count,i;
   int index;

   if (stricmp(value, "mismatch")) {
    ERROR("Not valid value %d", value);
    return 0;
   }

  if (stricmp(key , "a5") == 0) {
    partition = a5_partition;
    count = sizeof(a5_partition)/ sizeof(a5_partition[0]);
    ui_text_msg(data, PROMPT_TEXT, "A5 firmware can update");
  } else if (stricmp(key , "q6") == 0) {
    partition = q6_partition;
    count = sizeof(q6_partition)/ sizeof(q6_partition[0]);
    ui_text_msg(data, PROMPT_TEXT, "Q6 firmware can update.");
  } else if (stricmp(key , "qcn")) {
    ui_text_msg(data, PROMPT_TEXT, "QCN can update.");
    return 1;
  } else {
    ERROR("Not valid key %d", key);
    return 0;
  }

  for (index = 0;  index < PARTITION_NUM_MAX; index++)
    if (NULL == data->flash_partition[index])
      break;

  for (i =0; i < count && index < PARTITION_NUM_MAX; i++) {
     data->flash_partition[index] =
    data->hWnd->m_image->get_partition_info(*(partition+i), NULL, NULL);
     if (data->flash_partition[index] != NULL)
      index++;
  }

  return 1;
}

UINT CmdmfastbootDlg::usb_work(LPVOID wParam) {
  UsbWorkData* data = (UsbWorkData*)wParam;
  usb_handle * handle;
  flash_image  *img;

  if (data == NULL || data->usb == NULL ||  data->hWnd == NULL ||
      data->hWnd->m_image == NULL) {
    ERROR("Bad parameter");
    return -1;
  }

  handle = data->usb;
  img = data->hWnd->m_image;
  int result;
  usb_dev_t status = usb_status( handle);
  PCHAR title = new char[32];

  if (usb_port_dummy_sn(handle) == data->usb_sn)
  snprintf(title, 32, "Port %X", data->usb_sn);
  else
  snprintf(title, 32, "Port %X<=>%X", data->usb_sn, usb_port_dummy_sn(handle));

  ui_text_msg(data, TITLE, title);
  delete []title;

  if (status == DEVICE_CHECK) {
    adbhost adb(handle , usb_port_address(handle));
    const wchar_t *conf_file = img->get_package_config();
    char *conf_file_char = WideStrToMultiStr(conf_file);

    do_adb_shell_command(adb,data, "cat /proc/version",LINUX_VER);
    do_adb_shell_command(adb,data, "cat /etc/version",SYSTEM_VER);
    do_adb_shell_command(adb,data, "cat /usr/version",USERDATA_VER);

    if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(conf_file) &&  conf_file_char != NULL) {
      adb.sync_push(conf_file_char, "/tmp/config.xml");
      ui_text_msg(data, PROMPT_TEXT, "Copy config.xml to /tmp/config.xml.");
      delete conf_file_char;

      result = adb_hw_check(adb,data);
      if (result != 0) {
        WARN("hardware check failed.");
        ui_text_msg(data, ADB_CHK_ABORT, "hardware check failed, abort!");
        return -1;
      }

      result = adb_sw_version_cmp(adb,data);
      if (result < 0) {
        WARN("firmware version check failed.");
        ui_text_msg(data, ADB_CHK_ABORT, "firmware version check failed, abort!");
        return -1;
      } else if (result ==0) {
        WARN("firmware version check failed.");
        ui_text_msg(data, ADB_CHK_ABORT, "firmware version check failed, abort!");
        return -1;
      }

      //todo:: update nv ? 1. change offline mode. 2 . write imei
    } else {
      if (conf_file_char != NULL) delete conf_file_char;
      WARN("no config.xml in the package.");

      if (data->hWnd->m_flashdirect) {
        const FlashImageInfo* imgInfo = img->image_enum_init();
        int item = 0;
        ui_text_msg(data, PROMPT_TEXT, "no config.xml, flash all images in package directly!");

        for(;imgInfo != NULL && item < PARTITION_NUM_MAX; item++) {
          data->flash_partition[item] = imgInfo;
          imgInfo = img->image_enum_next(imgInfo);
        }
      } else {
        ui_text_msg(data, ADB_CHK_ABORT, "no config.xml in the package,  abort!");
      }
      return -1;
    }

#if 0
    ret = adb.shell("cat /proc/version", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, LINUX_VER, resp);
      free(resp);
    }

    ret = adb.shell("cat /etc/version", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, SYSTEM_VER, resp);
      free(resp);
    }

    ret = adb.shell("cat /usr/version", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, USERDATA_VER, resp);
      free(resp);
    }

    ret = adb.shell("trace -r", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, PROMPT_TITLE, "trace return:");
      ui_text_msg(data, PROMPT_TEXT, resp);
      free(resp);
    }

    adb.sync_push("./ReadMe.txt", "/usr");
    ui_text_msg(data, PROMPT_TEXT, "Copy host file ./ReadMe.txt  to /usr.");
    sleep(1);

    adb.sync_pull("/usr/ReadMe.txt", "..");
    ui_text_msg(data, PROMPT_TEXT, "Copy devie file /usr/ReadMe.txt  to .");
    sleep(1);

    ret = adb.shell("hwinfo_check", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, PROMPT_TITLE, "hwinfo_check return:");
      ui_text_msg(data, PROMPT_TEXT, resp);
      free(resp);
    }

    ret = adb.shell("swinfo_compare", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, PROMPT_TITLE, "swinfo_compare return:");
      ui_text_msg(data, PROMPT_TEXT, resp);
      free(resp);
    }

    ret = adb.shell("swinfo_compare", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
      ui_text_msg(data, PROMPT_TITLE, "swinfo_compare return:");
      ui_text_msg(data, PROMPT_TEXT, resp);
      free(resp);
    }
#endif

  do_adb_shell_command(adb,data, "trace -r");
  do_adb_shell_command(adb,data, "backup");
    adb.reboot_bootloader();
    ui_text_msg(data, REBOOT_DEVICE, "reboot bootloader");
  } else if (status == DEVICE_FLASH) {
    fastboot fb(handle);
    FlashImageInfo const * image;

    fb.fb_queue_display("product","product");
    fb.fb_queue_display("version","version");
    fb.fb_queue_display("serialno","serialno");
    fb.fb_queue_display("kernel","kernel");

    for (int index = 0; index < PARTITION_NUM_MAX; index++) {
      image = data->flash_partition[index];
      if (NULL == image)
        break;
      fb.fb_queue_flash(image->partition_str, image->data, image->size);
    }

#if 0
    unsigned size;
    void * img_data;
    if(0 == img->get_partition_info(L"boot", &img_data, &size))
      fb.fb_queue_flash("boot", img_data, size);

    if(0 == img->get_partition_info(L"system", &img_data, &size))
      fb.fb_queue_flash("system", img_data, size);
#endif

    //  fb.fb_queue_reboot();
    fb.fb_execute_queue(handle,data->hWnd, data);

    ui_text_msg(data, FLASH_DONE, " firmware updated!");
  }
  return 0;
}

/*
*
*/
BOOL CmdmfastbootDlg::AdbUsbHandler(BOOL update_device) {
  usb_handle* handle;
  UsbWorkData* data;

  if (!m_bWork) {
    INFO("We do not permit to work now.");
    return FALSE;
  }

  if (update_device)
    find_devices(m_flashdirect);

  for (handle = usb_handle_enum_init();
       handle != NULL ;
       handle = usb_handle_next(handle)) {
    if (!usb_is_work(handle)) {
      data = GetUsbWorkData(usb_port_address(handle));

      if (data == NULL)
        return FALSE;

      //usb_set_work(handle);
      //data->usb = handle;
      //data->usb_sn =usb_port_address(handle);
      SetUsbWorkData(data, handle);

      //AfxBeginThread(usb_work, data);
      //CDownload* pDl = new CDownload(usb_work, data, m_image);
      //m_dlWorkerPool.QueueRequest( (CDlWorker::RequestType) pDl );
      return TRUE;
    }
  }

  return FALSE;
}


void CmdmfastbootDlg::SetUpAdbDevice(
    PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam)
{
#if 0
  // dbcc_name:
  // \\?\USB#Vid_04e8&Pid_503b#0002F9A9828E0F06#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
  // convert to  USB\Vid_04e8&Pid_503b\0002F9A9828E0F06
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
  DEBUG(L"szClass %S", szClass.GetString());
#endif
  // if we are adding device, we only need present devices
  // otherwise, we need all devices
  DWORD dwFlag = (DBT_DEVICEARRIVAL != wParam
                  ? DIGCF_ALLCLASSES : (DIGCF_ALLCLASSES | DIGCF_PRESENT));
  //HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, szClass, NULL, dwFlag);
  HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_USB,  L"USB",NULL, dwFlag);
  long lResult;
  CRegKey reg;
  WCHAR key[MAX_PATH];
  TCHAR value[MAX_PATH];

  if( INVALID_HANDLE_VALUE == hDevInfo )  {
    AfxMessageBox(CString("SetupDiGetClassDevs(): ")
                  + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
    return;
  }

  SP_DEVINFO_DATA* pspDevInfoData =
    (SP_DEVINFO_DATA*)HeapAlloc(GetProcessHeap(), 0, sizeof(SP_DEVINFO_DATA));
  pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
  for(int i=0; SetupDiEnumDeviceInfo(hDevInfo,i,pspDevInfoData); i++)  {
    DWORD DataT ;
    DWORD nSize=0;
    BYTE srv[MAX_PATH];
    TCHAR buf[MAX_PATH];

    if ( !SetupDiGetDeviceInstanceId(hDevInfo, pspDevInfoData, buf, sizeof(buf), &nSize) )  {
      AfxMessageBox(CString("SetupDiGetDeviceInstanceId(): ")
                    + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
      break;
    }

    if ( SetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData, SPDRP_SERVICE,
                                          &DataT, (PBYTE)srv, sizeof(srv), &nSize) ) {
      //DEBUG(" %S, size %d", srv, nSize);
      if (wcsnicmp((const wchar_t *)srv, L"usbccgp",nSize/sizeof(wchar_t))) {
        continue;
      }
    }

    _snwprintf_s(key, MAX_PATH,L"SYSTEM\\ControlSet001\\Enum\\%s", buf);
    lResult = reg.Open(HKEY_LOCAL_MACHINE,key, KEY_READ);
    if (lResult == ERROR_SUCCESS) {
      nSize = MAX_PATH;
      lResult = reg.QueryStringValue(L"ParentIdPrefix", static_cast<LPTSTR>(value), &nSize);

      if (lResult == ERROR_SUCCESS ) {
        add_adb_device(buf, value);
      }
      reg.Close();
    }
  }

  dump_adb_device();

  if ( pspDevInfoData ) {
    HeapFree(GetProcessHeap(), 0, pspDevInfoData);
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
}
#if 0
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
#endif


void CmdmfastbootDlg::OnSize(UINT nType, int cx, int cy)
{
  int dx, dy, dw, dh;

  if (600>cx || 600> cy)
  {
    return;
  }
  CDialog::OnSize(nType, cx, cy);

  //BUTTON
  if (m_bInit) {
    RECT rect;
    GetDlgItem(IDC_BTN_STOP)->GetClientRect(&rect);
    GetDlgItem(IDC_BTN_STOP)->SetWindowPos(0, (cx + 800) /2, cy - 60, rect.right, rect.bottom, 0);

    GetDlgItem(IDCANCEL)->GetClientRect(&rect);
    GetDlgItem(IDCANCEL)->SetWindowPos(0, (cx + 1000 ) /2, cy - 60, rect.right, rect.bottom, 0);

     GetDlgItem(IDC_BTN_START)->GetClientRect(&rect);
    GetDlgItem(IDC_BTN_START)->SetWindowPos(0, (cx + 500) /2, cy - 60, rect.right, rect.bottom, 0);

     GetDlgItem(IDC_SETTING)->GetClientRect(&rect);
    GetDlgItem(IDC_SETTING)->SetWindowPos(0, (cx +300) /2, cy - 60, rect.right, rect.bottom, 0);

     GetDlgItem(IDC_BTN_BROWSE)->GetClientRect(&rect);
    GetDlgItem(IDC_BTN_BROWSE)->SetWindowPos(0, (cx ) /2, cy - 60, rect.right, rect.bottom, 0);

     GetDlgItem(IDC_EDIT_PACKAGE_PATH)->GetClientRect(&rect);
    GetDlgItem(IDC_EDIT_PACKAGE_PATH)->SetWindowPos(0, 100, cy - 60, rect.right, rect.bottom, 0);


     GetDlgItem(IDC_STATIC_PKG)->GetClientRect(&rect);
    GetDlgItem(IDC_STATIC_PKG)->SetWindowPos(0, 20, cy - 60, rect.right, rect.bottom, 0);


    GetDlgItem(IDC_GRP_PKG_INFO)->GetClientRect(&rect);
    dx = rect.right  +  10;
    dy = /*rect.bottom +*/ 6;
    dw = cx - dx  - 10;
    dh = cy -dy - 20 -50;
    SetPortDialogs(nType, dx, dy, dw, dh);
  }
  Invalidate(TRUE);
}

void CmdmfastbootDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	lpMMI->ptMinTrackSize.x   = 800 ;	//主窗口最小宽度
	lpMMI->ptMinTrackSize.y   = 800  ;  //主窗口最小高度
	CDialog::OnGetMinMaxInfo(lpMMI);
}

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
  bi.ulFlags=BIF_USENEWUI;
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
    m_ConfigPath.GetBuffer(MAX_PATH))) {

    delete m_image;
    m_image =  new flash_image(m_ConfigPath.GetString());

UpdatePackageInfo();
    } else {
    AfxMessageBox(L"Package path is not change!", MB_ICONEXCLAMATION);
    }

}

void CmdmfastbootDlg::OnBnClickedStart()
{
  if (SetWorkStatus(TRUE, FALSE)) {
    AdbUsbHandler(true);
  }

  //::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}


void CmdmfastbootDlg::OnBnClickedButtonStop()
{
    SetWorkStatus(FALSE, FALSE);
}

void CmdmfastbootDlg::OnClose()
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
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

  CDialog::OnClose();
}

void CmdmfastbootDlg::OnBnClickedCancel()
{
  DWORD dwExitCode = 0;
#if 0
  CWinThread* threadArry[] = {pThreadPort1, pThreadPort2, pThreadPort3, pThreadPort4};
  for (int i=0; i<sizeof(threadArry)/sizeof(threadArry[0]); i++)
  {
    if (NULL!=threadArry[i])
    {
      GetExitCodeThread(threadArry[i]->m_hThread, &dwExitCode);
      if (dwExitCode == STILL_ACTIVE)
      {
        int iRet = AfxMessageBox(L"Still have active downloading! Exit anyway?", MB_YESNO|MB_DEFBUTTON2);
        if (IDYES==iRet)
        {
          break;
        }
        else
        {
          return;
        }
      }
    }
  }
#endif
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

	// TODO: 在此处添加消息处理程序代码
	::RemoveProp(m_hWnd, JRD_MDM_FASTBOOT_TOOL_APP);	//for single instance
	//Shutdown the thread pool
	m_dlWorkerPool.Shutdown();
    delete m_image;

	StopLogging();
}


void CmdmfastbootDlg::OnBnClickedSetting()
	{
	// TODO: 在此添加控件通知处理程序代码

	}

