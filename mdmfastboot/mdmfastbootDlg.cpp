// mdmfastbootDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "log.h"
#include "mdmfastboot.h"
#include "mdmfastbootDlg.h"
#include "fastbootflash.h"
#include "adbhost.h"
#include "usb_vendors.h"
#include "DiagPST.h"
#include "AdbPST.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")

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


// CmdmfastbootDlg 对话框
CmdmfastbootDlg::CmdmfastbootDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CmdmfastbootDlg::IDD, pParent),
    mPSTManager(CmdmfastbootDlg::RunDevicePST)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_bInit = FALSE;
  m_updated_number = 0;
  m_module_name = MODULE_M801;
#ifdef INLINE_SETTING
  m_SetDlg.m_pParent = this;
#endif
}

CmdmfastbootDlg::~CmdmfastbootDlg() {
//delete mPSTManager;
}

void CmdmfastbootDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    //DDX_Control(pDX, IDC_CB_PLATFORM, m_project);

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
  int portsNum = mPSTManager.GetPortNum();

  R_NUM = mPSTManager.GetPortRows();// mAppConf.GetUiPortRowCount();
  C_NUM = portsNum / R_NUM;

  if (R_NUM * C_NUM < portsNum) {
    R_NUM > C_NUM ? C_NUM ++ :  R_NUM++;
  }

  pw = w / C_NUM;
  ph = h / R_NUM;

  for (r = 0; r < R_NUM; r++) {
    for (c = 0; c < C_NUM; c++) {
      if (r * C_NUM + c >= portsNum) break;

      port = mPSTManager.GetPortUI(r * C_NUM + c);// m_workdata[r * C_NUM + c]->pCtl;
      if (port == NULL) continue;
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
  if (IDC_EDIT_PACKAGE_PATH == nID && mPSTManager.GetPortNum() == 1) {
    w = rect.right < 300 ? 300 : rect.right;
  }
  w = rect.right < 100 ? 100 : rect.right;
  h = rect.bottom < 25 ? 25 : rect.bottom;
  handle->SetWindowPos(0, x, y, w, h, SWP_NOSENDCHANGING | SWP_NOSIZE);
  return TRUE;
}

BOOL CmdmfastbootDlg::UpdatePackageInfo(BOOL update) {
    flash_image       *m_image = mPSTManager.GetProjectPackage();
    const wchar_t     *qcn = NULL;
    const FlashImageInfo *img = NULL;
    int item = 0;

    if (m_image == NULL)
        return FALSE;

    img = m_image->image_enum_init();
    qcn = mPSTManager.get_package_qcn_path();

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
    //GetPrivateProfileString(L"app",L"module",L"M850",moduleName, MAX_PATH,m_ConfigPath);
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
    if(!bforce && mPSTManager.IsWork() == bwork) {
        WARN("Do not need to chage status.");
        return FALSE;
    }

    if (!bwork && mPSTManager.IsWork() && mPSTManager.IsHaveUsbWork()) {
        int iRet = AfxMessageBox(L"Still have active downloading! Exit anyway?",
                                 MB_YESNO|MB_DEFBUTTON2);
        if (IDYES!=iRet)
        {
            return FALSE;
        }
    }

    if (bwork) {
        mPSTManager.Reset();
    }

	m_imglist->EnableWindow(!bwork);
    GetDlgItem(IDC_BTN_START)->EnableWindow(!bwork);
    GetDlgItem(IDCANCEL)->EnableWindow(!bwork);
    GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(!bwork);
    GetDlgItem(IDC_BTN_STOP)->EnableWindow(bwork);

#ifdef INLINE_SETTING
    if (!mPSTManager.IsAfterSaleMode()) {
        m_SetDlg.EnableSettings(!bwork);
    }
#endif
    mPSTManager.SetWork(bwork);
    return TRUE;

}

#ifdef INLINE_SETTING
BOOL CmdmfastbootDlg::InitSettingDlg(void) {
    //CWnd * handle = GetDlgItem(IDC_SETTING);
    //handle->EnableWindow(FALSE);

    //m_SetDlg.Attach(this->m_hWnd);
    //m_SetDlg.ModifyStyle(WS_POPUP | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU,
    //          WS_CHILD | WS_VISIBLE | DS_CENTER, 0);
    //m_SetDlg.EnableWindow(TRUE);

    if (!mPSTManager.IsAfterSaleMode())
      return FALSE;

    m_SetDlg.SetFlashDirectData(&m_flashdirect);
    m_SetDlg.SetScheduleData(&m_fix_port_map);
    m_SetDlg.SetForeUpdateData(&m_forceupdate);
    //m_SetDlg.UpdateData();
    m_SetDlg.Create(IDD_SETTINGS, this);
    m_SetDlg.ShowWindow(WS_CHILDWINDOW);//Invalidate();
    return TRUE;
}
#endif

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
  if (mPSTManager.IsSuperMode()){
      SetIcon(m_hIcon, TRUE);			// 设置大图标
      SetIcon(m_hIcon, FALSE);		// 设置小图标
  } else {
      HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SUPERMODE));
      //SendMessage(WM_SETICON, ICON_BIG, (LPARAM)hicon);
      SetIcon(hIcon, TRUE);
      SetIcon(hIcon, FALSE);
  }

  m_project = (CComboBox *)GetDlgItem(IDC_CB_PLATFORM);
  m_project->ResetContent();
  int result = m_project->AddString(_T("HH70"));
//  if (result != CB_ERR && result != CB_ERRSPACE)
//    m_project->SetItemDataPtr(result, 0);
  m_project->AddString(_T("HH40"));
  m_project->SetCurSel(0);//SelectString(0, );
#ifdef INLINE_SETTING
  InitSettingDlg();
#endif

  //for (int i = 0; i < mAppConf.GetUiPortTotalCount(); i++) {
  //  m_workdata[i] = new UsbWorkData(i, this, &mDevCoordinator, &mAppConf, &m_LocalConfigXml, m_image);
  //}
  mPSTManager.Initialize(this);
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

  if (mPSTManager.IsAfterSaleMode()) {
    GetDlgItem(IDC_BTN_BROWSE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_EDIT_PACKAGE_PATH)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_PKG)->ShowWindow(SW_HIDE);
  }
  m_PackagePath = mPSTManager.GetPackage();
  UpdatePackageInfo(FALSE);

  //注释设备通知，不能放在构造函数，否则 RegisterDeviceNotification 返回87,因为构造函数中m_hWnd还没被初始化为有效值.
  RegisterAdbDeviceNotification(m_hWnd);
  SetWorkStatus(mPSTManager.IsWork(), TRUE);
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
                //HandleDeviceArrived(pDevInf->dbcc_name);
                mPSTManager.HandleComDevice();
                break;
            }
        case DBT_DEVTYP_DEVICEINTERFACE:
            LOGI("device arrive, DBT_DEVTYP_DEVICEINTERFACE");
            //SetTimer(TIMER_EVT_USBADB, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
            //HandleDeviceArrived(pDevInf->dbcc_name);
            mPSTManager.EnumerateAdbDevice();

            break;
        }
    } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
        switch (phdr->dbch_devicetype) {
        case DBT_DEVTYP_DEVICEINTERFACE:
            {
                mPSTManager.HandleDeviceRemoved(pDevInf, dwData);
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
                LOGI("device removed, DBT_DEVTYP_PORT");
                break;
            }
        }
    }

    return TRUE;
}


LRESULT CmdmfastbootDlg::OnDeviceInfo(WPARAM wParam, LPARAM lParam)
{
    UsbWorkData* data = (UsbWorkData*)lParam;
    UIInfo* uiInfo = (UIInfo*)wParam;

    if (uiInfo == NULL) {
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
        data->pCtl->SetInfo(PROMPT_TEXT, uiInfo->sVal);
        break;

    case REBOOT_DEVICE:
        //data->SwitchDev(switch_timeout);
        data->pCtl->SetInfo(PROMPT_TEXT, uiInfo->sVal);
        data->pCtl->SetInfo(PROMPT_TITLE, CString(""));
        break;

    case FLASH_DONE:
        if (!uiInfo->sVal.IsEmpty()) {
            data->pCtl->SetInfo(PROMPT_TEXT, uiInfo->sVal);
            data->pCtl->SetInfo(PROMPT_TITLE, CString(""));
        } else {
            CString prompt;
            prompt.Format(_T("Update device sucessfully, elapse %.3fSeconds"), data->GetElapseSeconds());
            data->pCtl->SetInfo(PROMPT_TITLE, prompt);
        }
        m_updated_number ++;
        mPSTManager.FlashDeviceDone(data);
        break;

    case TITLE:
        data->pCtl->SetTitle(uiInfo->sVal);
        break;

    case PROGRESS_VAL:
        data->pCtl->SetProgress(uiInfo->iVal);
        break;

    default:
        data->pCtl->SetInfo(uiInfo->infoType, uiInfo->sVal);
    }

    delete uiInfo;
    return 0;
}

/*nIDEvent is the usb sn*/
void CmdmfastbootDlg::OnTimer(UINT_PTR nIDEvent) {
  UsbWorkData * data = (UsbWorkData *)nIDEvent;

  if (data == NULL) {
    ERROR("USB %p switch device timeout", nIDEvent);
    //return ;
  }

  LOGW("%s switch device timeout", data->mActiveDevIntf->GetDevTag());
  return ;
#if 0 //comment @2017.1.3
  if (data->GetStatus() == USB_STAT_SWITCH) {
    //remove_switch_device(nIDEvent);
  } else if (data->GetStatus()  == USB_STAT_WORKING){
    if (data->work != NULL) //Delete, or ExitInstance
      data->work->PostThreadMessage( WM_QUIT, NULL, NULL );
    usb_close(data->usb);
  }

  data->Clean(mDevCoordinator.IsEmpty());
  ScheduleDeviceWork();
  //KillTimer((UINT_PTR)data);
#endif
}


BOOL CmdmfastbootDlg::SetupDevice(int evt) {
    switch(evt) {
    case TIMER_EVT_ADBKILLED:
        mPSTManager.RejectCDROM();
        if(!mPSTManager.EnumerateAdbDevice())
            mPSTManager.HandleComDevice();
        break;
    case TIMER_EVT_REJECTCDROM:
        mPSTManager.RejectCDROM();
        break;
    case TIMER_EVT_COMPORT:
        mPSTManager.HandleComDevice();
        break;
    case TIMER_EVT_USBADB:
        mPSTManager.EnumerateAdbDevice();
        break;
    default:
        break;
    }
    // if (evt != TIMER_EVT_REJECTCDROM)
    //     ScheduleDeviceWork();

    //mDevCoordinator.Dump();
    return TRUE;
}
 void CALLBACK CmdmfastbootDlg::DeviceEventTimerProc(
   HWND hWnd,      // handle of CWnd that called SetTimer
   UINT nMsg,      // WM_TIMER
   UINT_PTR nIDEvent,   // timer identification
   DWORD dwTime    // system time
)
{
    CmdmfastbootDlg  *dlg = (CmdmfastbootDlg*)hWnd;
    ::KillTimer(hWnd, nIDEvent);//must kill timer explicit, otherwise, the timer will expire periodically.
    dlg->SetupDevice(nIDEvent) ;
}

/*
* update flow is done here. Do update business logically.
*/
UINT CmdmfastbootDlg::RunDevicePST(LPVOID wParam) {
    UsbWorkData* data = (UsbWorkData*)wParam;
    usb_handle * handle;
    flash_image  *img;
    DeviceInterfaces *dev;
    int result;
    usb_dev_t status;
    BOOL useAdb = TRUE;
    BOOL flashdirect = TRUE;
    AppConfig      *config;

    if (data == NULL ||  !data->CheckValid()) {
        data->ui_text_msg(FLASH_DONE, "Bad parameter");
        return -1;
    }

    dev = data->mActiveDevIntf;
    img = data->mProjectPackage;
    config = data->mPAppConf;
    status = dev->GetDeviceStatus();
    useAdb = config->IsUseAdb();
    flashdirect = config->GetFlashDirectFlag();

    data->ui_text_msg(TITLE, dev->GetDevTag());
    if (status == DEVICE_PLUGIN) {
        DiagPST pst(data, data->GetXmlParser(), img->GetFileBuffer());
        data->ui_text_msg(PROMPT_TITLE, "Begin download by Diag");
        bool result = pst.DownloadCheck();
        if(result)
            result = pst.RunTimeDiag();

        if(result || pst.IsEmergencyDownloadMode()) {
            if (!pst.IsEmergencyDownloadMode() && useAdb) {
                int count = 0;
                do {
                    if (count != 0)
                        SLEEP(3000);
                    data->UpdateUsbHandle(FALSE, flashdirect);
                }while(data->usb == NULL && count++ < 5);
            }
            /*
            * If device enter TPST status, we does not find adb device.
            */
            if (!pst.IsEmergencyDownloadMode() && useAdb && data->usb != NULL) {
                    AdbPST adbPST(config->GetForceUpdateFlag(), m_module_name);
                    adbPST.Reboot(data, dev);
                //} else {
                //    result = FALSE;
                //    LOGE("There are no adb device when use adb reboot");
                //}
            } else {
                result = pst.DownloadPrg(data->mPAppConf);
                if(result) {
                    result = pst.DownloadImages(img);
                }
            }
        } else {
            LOGE("download check failed");
        }
        if(result) {
            dev->SetDeviceStatus(DEVICE_FLASH);
            data->ui_text_msg(REBOOT_DEVICE, "Enter fastboot");
            data->WaitForDevSwitchEvt();
        } else {
            data->ui_text_msg(FLASH_DONE, "Diag PST occur error! Please check log");
            return 0;
        }

    }

    handle = data->usb;
    status = dev->GetDeviceStatus();
    if (handle == NULL) {
        data->ui_text_msg(FLASH_DONE, "Bad parameter");
        return -1;
    }

    if (status == DEVICE_CHECK) {
        AdbPST pst(data->mPAppConf->GetForceUpdateFlag(), m_module_name);
//        pst.DoPST(data, img, dev);

    } else if (status == DEVICE_FLASH) {
        fastboot fb(handle);
        FlashImageInfo const * image;
        data->ui_text_msg(PROMPT_TITLE, "fastboot download");

        fb.fb_queue_display("product","product");
        fb.fb_queue_display("version","version");
        fb.fb_queue_display("serialno","serialno");
        fb.fb_queue_display("kernel","kernel");
#if 0
        //this is check new image version and firmware version by adb
        for (int index = 0; index < data->partition_nr; index++) {
            image = data->flash_partition[index];
            if (image->need_download) {
                fb.fb_queue_flash(image->partition_str, image->data, image->size);
            }
        }
#endif
        image = img->image_enum_init();
        for(;image != NULL ; ) {
            if (image->need_download) {
                fb.fb_queue_flash(image->partition_str, image->data, image->size);
            }
            image = img->image_enum_next(image);
        }

        //  fb.fb_queue_reboot();
        fb.fb_execute_queue(handle, data, img->GetDiagDlImgSize());
        data->ui_text_msg(FLASH_DONE, NULL);
        dev->SetDeviceStatus(DEVICE_REMOVED);
    }
    return 0;
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

    if (mPSTManager.GetPortNum() == 1) {
      dx = rect.right + 20;
      if (mPSTManager.IsAfterSaleMode()) {
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

#ifdef INLINE_SETTING
        dy = 5;
        m_SetDlg.SetWindowPos(NULL, dx, dy, 280, 220, 0);
#endif

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

#ifdef INLINE_SETTING
      dy = rect.bottom + 20;
      if (!mPSTManager.IsAfterSaleMode()) {
        m_SetDlg.SetWindowPos(NULL, rect.left, dy, 280, 220, 0);
      }
	  //m_SetDlg.ShowWindow(SW_HIDE);   //hide set dialog by zhanghao 20160112
#endif
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
static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM , LPARAM lpData)
{
    if(uMsg == BFFM_INITIALIZED)
    {
       // CTreeCtrl   treePath;
       // HTREEITEM   hItemSel;
       // int id = GetDlgCtrlID(hWnd);
        ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
      //  treePath.SubclassWindow(::GetDlgItem(hWnd, 0x3741));
      //  hItemSel    = treePath.GetSelectedItem();
      //  treePath.Expand(hItemSel, TVE_COLLAPSE);
      //  treePath.UnsubclassWindow();
    }
    return 0;
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
  CString path;
  //GetDlgItemText(uid, szPath, MAX_PATH);
  ZeroMemory(&bi,sizeof(BROWSEINFO));
  //bi.pidlRoot = SHParsePidlFromPath("E:\\");
  bi.hwndOwner=GetSafeHwnd();
  bi.pszDisplayName= m_PackagePath.GetBuffer(MAX_PATH);
  bi.lpszTitle=L"Select Package folder";
  bi.lpfn   =   BrowseCallbackProc;
  bi.ulFlags=BIF_USENEWUI | BIF_NONEWFOLDERBUTTON | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
  if(PathFileExists(m_PackagePath.GetString())) {
    path = GetDirName(m_PackagePath);
    bi.lParam = (LPARAM)path.GetString();
  } else {
    GetAppPath(path);
    bi.lParam = (LPARAM)path.GetString();
  }
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

    if (mPSTManager.ChangePackage(m_PackagePath.GetString()))
    		/*m_ConfigPath.GetBuffer(MAX_PATH))*/ {
    	UpdatePackageInfo();
    } else {
        AfxMessageBox(L"Package path is not change!", MB_ICONEXCLAMATION);
    }
}

void CmdmfastbootDlg::OnBnClickedStart()
{
	if (m_imglist->GetItemCount()<1)
	{
		AfxMessageBox(L"Please select a valid package directory!", MB_OK);
		return;
	}
	GetMenu()->EnableMenuItem(ID_FILE_M850, MF_DISABLED|MF_GRAYED);
	GetMenu()->EnableMenuItem(ID_FILE_M801, MF_DISABLED|MF_GRAYED);
	if (SetWorkStatus(TRUE, FALSE)) {
	//AdbUsbHandler(true);
    SetupDevice(TIMER_EVT_ADBKILLED);
	}

  //::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void CmdmfastbootDlg::OnBnClickedButtonStop()
{
	GetMenu()->EnableMenuItem(ID_FILE_M850, MF_ENABLED);
	GetMenu()->EnableMenuItem(ID_FILE_M801, MF_ENABLED);
    SetWorkStatus(FALSE, FALSE);
    mPSTManager.Reset();
}

void CmdmfastbootDlg::OnClose()
{
  //CDialog::OnClose();

  if (mPSTManager.IsHaveUsbWork())
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

  if (mPSTManager.IsHaveUsbWork())
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

	StopLogging();
}


void CmdmfastbootDlg::OnBnClickedSetting()
{
	//m_pMainWnd = &dlg;
# if 1
CSettingsDlg settings ;
	INT_PTR nResponse = settings.DoModal();
	if (nResponse == IDOK)
	{
      LOGE("SS");
	}
	else if (nResponse == IDCANCEL)
	{

	}
    #else

	m_SetDlg.Create(IDD_SETTINGS, NULL);
	    m_SetDlg.ModifyStyle(
              WS_CHILD | WS_VISIBLE | DS_CENTER,
              WS_POPUP | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU,
              SWP_SHOWWINDOW);
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
		mPSTManager.SetDownload(strPartitionName, bSelected);
	}

	*pResult = 0;
}

void CmdmfastbootDlg::OnFileM850()
{
	CopyFile(L".\\mdmconfig_M850.ini", L".\\mdmconfig.ini", FALSE);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYPOSITION|MF_CHECKED);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(1,MF_BYPOSITION|MF_UNCHECKED);
//	InitSettingConfig();
//	UpdatePackage();
}

void CmdmfastbootDlg::OnFileM801()
{
	CopyFile(L".\\mdmconfig_M801.ini", L".\\mdmconfig.ini", FALSE);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(1,MF_BYPOSITION|MF_CHECKED);
	GetMenu()->GetSubMenu(0)->CheckMenuItem(0,MF_BYPOSITION|MF_UNCHECKED);
//	InitSettingConfig();
//	UpdatePackage();
}
