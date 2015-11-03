
// FreeportLiveDeployDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FreeportLiveDeploy.h"
#include "FreeportLiveDeployDlg.h"
#include "afxdialogex.h"
#include "Difxapi.h"
#include "log.h"
#include <string.h>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma 	  comment(lib,"Difxapi.lib")

//void (WINAPI * DIFXLOGCALLBACK) DIFLOGCALLBACK;

 void WINAPI AdbDifLog(
   DIFXAPI_LOG Event,
   DWORD       Error,
   PCWSTR      EventDescription,
   PVOID       CallbackContext
)
{
CFreeportLiveDeployDlg* dlg =  (CFreeportLiveDeployDlg* )CallbackContext;
INFO("Error %d,  Event %d,  Description %S", Error, Event, EventDescription);

switch(Event) {
 case DIFXAPI_SUCCESS:
//  dlg->m_hDevchangeTips->SetWindowText(EventDescription);
 //dlg->m_hDevchangeTips->SetWindowText(_T("adb driver installed."));
 break;

 case DIFXAPI_ERROR:
 case DIFXAPI_WARNING:
 // dlg->m_hDevchangeTips->SetWindowText(_T("adb driver is not installed properly. Please check the log."));
 break;

 //case DIFXAPI_INFO:
 default:
// dlg->m_hDevchangeTips->SetWindowText(EventDescription);
 break;
 }

 }

int ControlUSBNIC(const TCHAR * path_filter, int control_code);
// CFreeportLiveDeployDlg dialog

CFreeportLiveDeployDlg::CFreeportLiveDeployDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFreeportLiveDeployDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CFreeportLiveDeployDlg::~CFreeportLiveDeployDlg() {
    StopLogging();
}

void CFreeportLiveDeployDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFreeportLiveDeployDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_WM_TIMER()
	ON_MESSAGE(UI_MESSAGE_INIT_DEVICE, &CFreeportLiveDeployDlg::OnInitDevice)
    ON_BN_CLICKED(IDOK, &CFreeportLiveDeployDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFreeportLiveDeployDlg message handlers

BOOL CFreeportLiveDeployDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
  StartLogging(L"ModioLTECase_microSDPatch.log", "log,info,warn,error", "all");
  //StartLogging(L"ModioLTECase_microSDPatch.log", "all", "all");
  m_bSwitchDisk = FALSE;
  m_hUSBHandle = NULL;

  GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
  GetDlgItem(IDC_CONFIRM_NOTE)->ShowWindow(SW_HIDE);

  m_hDevchangeTips = (CStatic *)GetDlgItem(IDC_STATIC_DEVCHANGE_TIPS);
  m_hDevchangeTips->SetWindowText(_T("Please connect AT&&T Modio LTE Case to computer via USB cable."));

  RegisterAdbDeviceNotification(this->m_hWnd, &this->hDeviceNotify);
  adb_usb_init();

#if 1
  kill_adb_server(DEFAULT_ADB_PORT);
  SetTimer(TIMER_PUSH_FILES, 1000, NULL);
#else
  if (kill_adb_server(DEFAULT_ADB_PORT) == 0) {
    SetTimer(TIMER_PUSH_FILES, 1000, NULL);
  } else {
    LiveDeploy(TRUE);
  }
#endif

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFreeportLiveDeployDlg::OnPaint()
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
HCURSOR CFreeportLiveDeployDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


usb_handle* CFreeportLiveDeployDlg::GetUsbHandle() {
  usb_handle* handle;
  find_devices(true);
  for (handle = usb_handle_enum_init();
       handle != NULL ;
       handle = usb_handle_next(handle)) {
    break;
  }
  return handle;
}

VOID CFreeportLiveDeployDlg::LiveDeploy(BOOL trySwitchDisk) {
  if (m_hUSBHandle == NULL)
    m_hUSBHandle = GetUsbHandle();
  if (m_hUSBHandle != NULL) {
    LRESULT result = 0;
    //GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
    adbhost adb(m_hUSBHandle, usb_port_address(m_hUSBHandle));
    m_hDevchangeTips->SetWindowText(_T("Get adb interface, now do send files."));
    result += PushFile(adb, "data\\fatlabel", "/usr/sbin/fatlabel");
    result += PushFile(adb, "data\\formatsdcard.sh", "/usr/oem/formatsdcard.sh");
    result += PushFile(adb, "data\\umount.sh", "/usr/oem/umount.sh");
    result += PushFile(adb, "data\\restartusb.sh", "/usr/oem/restartusb.sh");
    if (result == 0) {
        m_hDevchangeTips->SetWindowText(_T("Patch successfully applied!"));
    } else {
        m_hDevchangeTips->SetWindowText(_T("Failed To apply patch! Please check the log."));
    }
    GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
    GetDlgItem(IDCANCEL)->SetWindowText(_T("Close"));
  } else // if (trySwitchDisk) {
  {
  //if user remove the device when install driver.
  m_hDevchangeTips->SetWindowText(_T("Please connect AT&&T Modio LTE Case to computer via USB cable."));
    //PostMessage(UI_MESSAGE_INIT_DEVICE, (WPARAM)0, (LPARAM)NULL);
  }
}

LRESULT CFreeportLiveDeployDlg::PushFile(adbhost & adb, const char *lpath, const char *rpath) {
  CString prompt;
  PCHAR resp = NULL;
  int resp_len;
  int result = 0;
  //m_hDevchangeTips->GetWindowText(prompt);
  //prompt += _T("\n"); //this result in none text display
  prompt += lpath;
  prompt += _T(" => ");
  prompt += rpath;
  m_hDevchangeTips->SetWindowText(prompt);
  LOG("%S", prompt);
  result = adb.sync_push(lpath, rpath);

  if (result != 0) {
    ERROR("PushFile %s failed", lpath);
    return 1;
  }

  CStringA command = "chmod 755 ";
  command += rpath;

  result = adb.shell(command, (void **)&resp, &resp_len);
  if (result != 0) {
    result = 1;
  }
  return 0;
}
BOOL CFreeportLiveDeployDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
  if (dwData == 0)
  {
    DEBUG("OnDeviceChange, dwData == 0 .EventType: 0x%x", nEventType);
    return FALSE;
  }

  DEV_BROADCAST_HDR* phdr = (DEV_BROADCAST_HDR*)dwData;
  PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)phdr;

  INFO("OnDeviceChange, EventType: 0x%x, DeviceType 0x%x",
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
        PDEV_BROADCAST_DEVICEINTERFACE pDevInf =
               (PDEV_BROADCAST_DEVICEINTERFACE)phdr;
        //    UpdateDevice(pDevInf, dwData);
        WARN("OnDeviceChange, get DBT_DEVTYP_VOLUME");
        //we handle this event when app is launched,
        //if device plug in before app start, we receive this, because we switch device,
        // and volumn (disk device) will enumerate with adb interface.
        // We do this, for if device is release build, it run without adb, so we need to switch.
        // test 1: launch app, plug in device, to see whether switch device.
        // test 2: run adb server, plugin device , luanch app.
        // test 3: run adb server, launch app, plugin device (if app launcher, adb can not start server).
        // test 3 is not necessary, because user can not very quick to pulgin device after launcher the app.
        if (m_bSwitchDisk == FALSE && m_hUSBHandle == NULL) {
          DEBUG("SET TIMER_SWITCH_DISK");
          m_hDevchangeTips->SetWindowText(_T("AT&&T Modio LTE Case detected."));
          SetTimer(TIMER_SWITCH_DISK, 5000, NULL);
        }
        break;
      }
    case DBT_DEVTYP_DEVICEINTERFACE:
      {
        if (m_bSwitchDisk == FALSE) {
          DEBUG("KILL TIMER_SWITCH_DISK");
          KillTimer(TIMER_SWITCH_DISK);
        } else {
          //In win7 x64, when after InstallAdbDriver, the devmgmt.msc will
          //rescan the device, and some product have always present mass
          //storage devie, even do switch device status.
          //m_bSwitchDisk = FALSE;
          //m_hDevchangeTips->SetWindowText(_T(""));
        }

        //KillTimer(TIMER_INSTALL_ADB_DRIVER);
        //LiveDeploy(FALSE);
      }
      break;
    }
  } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
    if (phdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

      ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
      if (m_hUSBHandle != NULL) {
        usb_close(m_hUSBHandle);
        m_hUSBHandle = NULL;
      }

      m_bSwitchDisk = FALSE;
      //In win10 64, after install adb driver, system notify remove event.
      //m_hDevchangeTips->SetWindowText(_T("AT&&T Modio LTE Case removed."));
    }
  }

  return TRUE;
}
void CFreeportLiveDeployDlg::OnTimer(UINT_PTR nIDEvent)
{
  CDialog::OnTimer(nIDEvent);
  if (nIDEvent == TIMER_PUSH_FILES) {
    DEBUG("HANDLE TIMER_PROFILE_LIST TIMER");
    m_hUSBHandle = GetUsbHandle();
    if (m_hUSBHandle != NULL) {
      ConfirmMessage();
    } else {
      PostMessage(UI_MESSAGE_INIT_DEVICE, (WPARAM)0, (LPARAM)NULL);
    }
  } else if (nIDEvent == TIMER_SWITCH_DISK){
    DEBUG("HANDLE TIMER_SWITCH_DISK TIMER");
    PostMessage(UI_MESSAGE_INIT_DEVICE, (WPARAM)0, (LPARAM)NULL);
  } else if(nIDEvent == TIMER_INSTALL_ADB_DRIVER) {
    //InstallAdbDriver();
  }

  KillTimer(nIDEvent);
}

//https://msdn.microsoft.com/en-us/library/windows/hardware/ff544813%28v=vs.85%29.aspx
LRESULT CFreeportLiveDeployDlg::InstallAdbDriver(void) {
  BOOL reboot;
  PCTSTR DriverPackageInfPath  = _T("usb_driver\\android_winusb.inf");
  SetDifxLogCallback(AdbDifLog, this);
  m_hDevchangeTips->SetWindowText(_T("Installing Adb driver ... ..."));

  DWORD  Flags = 0x00000000;

  OSVERSIONINFOEX osver;
  osver.dwOSVersionInfoSize = sizeof(osver);
  //获取版本信息
  if (! GetVersionEx((LPOSVERSIONINFO)&osver))  {
    WARN("GetVersion failed");
  } else {
    INFO("OS Version %d.%d", osver.dwMajorVersion, osver.dwMinorVersion);
    if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1) {
      //Win XP
      Flags = DRIVER_PACKAGE_FORCE | DRIVER_PACKAGE_LEGACY_MODE;
    }
  }

  DEBUG("Installing adb driver");
  DWORD retCode = DriverPackageInstall(DriverPackageInfPath ,
                                       Flags,
                                       NULL,
                                       &reboot);
  switch(retCode) {
  case CERT_E_EXPIRED:
    DEBUG("DriverPackageInstall:  The signing certificate is expired.");
    break;
  case CRYPT_E_FILE_ERROR:
    DEBUG("DriverPackageInstall:  The catalog file for the specified driver package was not found.");
    break;
  case ERROR_FILE_NOT_FOUND:
    DEBUG("DriverPackageInstall:  The INF file  %S was not found.", DriverPackageInfPath);
    break;
  case ERROR_FILENAME_EXCED_RANGE:
    DEBUG("DriverPackageInstall:  The INF file path, in characters,  is greater than the maximum supported path length.");
    break;
  case ERROR_INVALID_NAME:
    DEBUG("DriverPackageInstall:  The specified INF file path is not valid.");
    break;
  case TRUST_E_NOSIGNATURE:
    DEBUG("DriverPackageInstall:  The driver package is not signed.");
    break;
  case ERROR_NO_DEVICE_ID:
    DEBUG("DriverPackageInstall:  The driver package does not specify a hardware identifier or "
          "compatible identifier that is supported by the current platform. ");
    break;
  default:
    DEBUG("DriverPackageInstall:  return code %d.", retCode);
    break;
  }

  //if (retCode == 0) {
  //UnregisterDeviceNotification(hDeviceNotify);
  //RegisterAdbDeviceNotification(this->m_hWnd, &this->hDeviceNotify);
  LiveDeploy(FALSE);
  //}
  return 0;
}

LRESULT  CFreeportLiveDeployDlg::OnInitDevice(WPARAM wParam, LPARAM lParam) {
  std::vector<CString> devicePath;
  GetDeviceByGUID(devicePath, &GUID_DEVINTERFACE_DISK);
  size_t cdromSize = devicePath.size();

  for(size_t i = 0; i < cdromSize; i++)
  {
    CString path = devicePath[i];
    if (path.Find(_T("\\\\?\\usbstor#")) == -1) {
      LOG("Fix DISK %S:", path);
    } else {
      path.MakeUpper();
      if (path.Find(_T("ONETOUCH")) == -1 && path.Find(_T("ALCATEL")) == -1 &&
        path.Find(_T("VEN_AT&T&PROD_MODIO")) == -1) {
        LOG("USB Stor %S is not target device.",path);
      } else {
        CSCSICmd scsi = CSCSICmd();
        LOG("do switch device %S", devicePath[i]);
        //SetTimer(TIMER_INSTALL_ADB_DRIVER, 1000, NULL);
        m_hDevchangeTips->SetWindowText(_T("Toggle USB Ports of AT&&T Modio LTE Case."));
        scsi.SwitchToDebugDevice(devicePath[i]);
        m_bSwitchDisk = TRUE;

        ConfirmMessage();
        //scsi.SwitchToDebugDevice(_T("\\\\?\\H:"));
        break;
      }
    }
  }
  devicePath.clear();

  return 0;
}

BOOL CFreeportLiveDeployDlg::ToggleConfirmWindow(BOOL show) {
  int nCmdShow ;
  if (show)
    nCmdShow = SW_SHOW;
  else
    nCmdShow = SW_HIDE;

  GetDlgItem(IDOK)->ShowWindow(nCmdShow);
  GetDlgItem(IDC_CONFIRM_NOTE)->ShowWindow(nCmdShow);

  GetDlgItem(IDC_STATIC_APPLY)->ShowWindow(nCmdShow);
  GetDlgItem(IDC_STATIC_OK)->ShowWindow(nCmdShow);
  GetDlgItem(IDC_STATIC_CLICK)->ShowWindow(nCmdShow);

  if (show)
    nCmdShow = SW_HIDE;
  else
    nCmdShow = SW_SHOWNORMAL;
  GetDlgItem(IDCANCEL)->ShowWindow(nCmdShow);
  m_hDevchangeTips->ShowWindow(nCmdShow);

  return TRUE;
}

VOID CFreeportLiveDeployDlg::ConfirmMessage(VOID)
{
  CStatic * emphasize = (CStatic *)GetDlgItem(IDC_STATIC_OK);
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
  ToggleConfirmWindow(TRUE);
}

void CFreeportLiveDeployDlg::OnBnClickedOk()
{
  //CDialogEx::OnOK();
  LOG("Disable RNDIS USB NIC");
  ControlUSBNIC(_T("VID_1BBB&PID_0196"), DICS_DISABLE);
  ToggleConfirmWindow(FALSE);
  if (m_hUSBHandle != NULL) {
    LiveDeploy(FALSE);
  } else {
    InstallAdbDriver();
  }
}

/*++
Routine Description:
    Callback for use by Enable/Disable/Restart
    Invokes DIF_PROPERTYCHANGE with correct parameters
    uses SetupDiCallClassInstaller so cannot be done for remote devices
    Don't use CM_xxx API's, they bypass class/co-installers and this is bad.

    In Enable case, we try global first, and if still disabled, enable local

Arguments:
    Devs     uniquely identify the device
    DevInfo
    controlcode:

Return Value:
    EXIT_xxxx
--*/
int ControlDevice(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo,int controlCode) {
  int ret = 0;
  SP_PROPCHANGE_PARAMS pcp;
  SP_DEVINSTALL_PARAMS devParams;

  switch(controlCode) {
  case DICS_ENABLE:
    // enable both on global and config-specific profile
    // do global first and see if that succeeded in enabling the device
    // (global enable doesn't mark reboot required if device is still
    // disabled on current config whereas vice-versa isn't true)
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = controlCode;
    pcp.Scope = DICS_FLAG_GLOBAL;
    pcp.HwProfile = 0;
    // don't worry if this fails, we'll get an error when we try config-
    // specific.
    if(SetupDiSetClassInstallParams (Devs,DevInfo,&pcp.ClassInstallHeader,sizeof(pcp))) {
      SetupDiCallClassInstaller (DIF_PROPERTYCHANGE,Devs,DevInfo);
    }
    // now enable on config-specific
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = controlCode;
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.HwProfile = 0;
    break;

  default:
    // operate on config-specific profile
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = controlCode;
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.HwProfile = 0;
    break;
  }

  if(!SetupDiSetClassInstallParams(Devs, DevInfo, &pcp.ClassInstallHeader, sizeof(pcp)) ||
     !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, Devs, DevInfo)) {
    // failed to invoke DIF_PROPERTYCHANGE
    ret = ::GetLastError();
    INFO("fail to invoke DIF_PROPERTYCHANGE:%d", ret);
  } else {
    // see if device needs reboot
    devParams.cbSize = sizeof(devParams);
    if(SetupDiGetDeviceInstallParams(Devs, DevInfo, &devParams) &&
       (devParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))) {
      INFO("need to reboot");
    } else {
      // appears to have succeeded
      INFO("restart driver succeed");
    }
  }
  return ret;
}

//enum the device whose device id matched the path_filter and control the
//device with control_code;
//path_filter: the filter string to enum the device
//control_code: can be DICS_ENABLE/DICS_DISABLE/DICS_PROPCHANGE/DICS_START/DICS_STOP
//              which indicating a change in a device's state. Here use
//              DICS_ENABLE to enable, DICS_DISABLE to disable and
//              DICS_PROPCHANGE to restart the device;
int ControlUSBNIC(const TCHAR * path_filter, int control_code) {
  int   ret = -1;
  GUID  Guid = GUID_DEVCLASS_NET;// = GUID_DEVCLASS_HIDCLASS;
  DWORD size = 0;
  //::SetupDiClassGuidsFromNameEx(_T("NET"), &Guid, 1, &size, NULL, NULL);

  HDEVINFO hDevInfo;
  hDevInfo = SetupDiGetClassDevs(&Guid, NULL, NULL, DIGCF_PRESENT); // 枚举已存在（DIGCF_PRESENT）的hidclass类型设备
  if (hDevInfo == INVALID_HANDLE_VALUE) {
    return ret;
  }
  DWORD devIndex = 0;
  SP_DEVINFO_DATA did;
  did.cbSize = sizeof(SP_DEVINFO_DATA);
  //枚举设备信息
  for (devIndex = 0; SetupDiEnumDeviceInfo(hDevInfo, devIndex, &did); ++devIndex) {
    DWORD dwDetDataSize = 0;
    TCHAR devId[1024] = {0};

    if(SetupDiGetDeviceInstanceId(hDevInfo, &did, devId, sizeof(devId), &dwDetDataSize)) {
      DEBUG("device:%S", devId);
      // Add the link to the list of all DFU devices
      if((_tcsstr(devId, path_filter) == NULL) )
        continue;
      INFO("find the device with path:%S", path_filter);
      ret = ControlDevice(hDevInfo, &did, control_code);
    }
  }
  SetupDiDestroyDeviceInfoList(hDevInfo);

  return ret;
}
