// GetProfileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GetProfile.h"
#include "GetProfileDlg.h"
#include "log.h"
#include "adbhost.h"
#include <string.h>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CGetProfileDlg 对话框




CGetProfileDlg::CGetProfileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetProfileDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGetProfileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGetProfileDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROFILE, &CGetProfileDlg::OnLvnItemchangedListProfile)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PROFILE, &CGetProfileDlg::OnNMClickListProfile)
	ON_MESSAGE(UI_MESSAGE_INIT_DEVICE, &CGetProfileDlg::OnInitDevice)
END_MESSAGE_MAP()


// CGetProfileDlg 消息处理程序

BOOL CGetProfileDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

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
  StartLogging(L"GetProfile.log", "all", "all");
  m_bSwitchDisk = FALSE;
  m_DeviceProfilePath = "/usr/bin/profile/match";
  m_hProfileList = ((CListCtrl*)GetDlgItem(IDC_LIST_PROFILE));
  m_hProfileList->InsertColumn(0, _T("Profiles"),LVCFMT_LEFT, 80);
  //m_hProfileList->SetExtendedStyle(LVS_EX_CHECKBOXES);//设置控件有勾选功能

  m_hProfileDataList = ((CListBox*)GetDlgItem(IDC_LIST_PROFILE_DATA));
  m_hProfileName = (CStatic *)GetDlgItem(IDC_STATIC_PROFILE_NAME);
  GetDlgItem(IDOK)->ShowWindow(SW_HIDE);

  RegisterAdbDeviceNotification(this->m_hWnd);
  adb_usb_init();

  if (kill_adb_server(DEFAULT_ADB_PORT) == 0) {
    SetTimer(TIMER_PROFILE_LIST, 1000, NULL);
  } else {
    GetProfilesList(TRUE);
  }

  return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGetProfileDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGetProfileDlg::OnPaint()
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
HCURSOR CGetProfileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


usb_handle* CGetProfileDlg::GetUsbHandle() {
  usb_handle* handle;
  find_devices(true);
  for (handle = usb_handle_enum_init();
       handle != NULL ;
       handle = usb_handle_next(handle)) {
    break;
  }
  return handle;
}

BOOL CGetProfileDlg::ParseProfilesList(char * content ,
                                       PCHAR lineDelim, PCHAR recordDelim) {
  char *str1, *str2, *token, *subtoken;
  char *saveptr1, *saveptr2;
  int j;

  for (j = 1, str1 = content; ; j++, str1 = NULL) {
    token = strtok_s(str1, lineDelim, &saveptr1);
    if (token == NULL)
      break;
    //  printf("%d: %s\n", j, token);

    for (str2 = token; ; str2 = NULL) {
      subtoken = strtok_s(str2, recordDelim, &saveptr2);
      if (subtoken == NULL)
        break;
      m_pProfiles.push_back(subtoken);
      // LOG(" --> %s", subtoken);
    }
  }
  return TRUE;
}


BOOL CGetProfileDlg::ParseContent(char * content,  PCHAR lineDelim, std::vector<PCCH>& dataOut) {
  char *str1, *token;
  char *saveptr1;
  int j;

  for (j = 1, str1 = content; ; j++, str1 = NULL) {
    token = strtok_s(str1, lineDelim, &saveptr1);
    if (token == NULL)
      break;
    dataOut.push_back(token);

  }
  return TRUE;
}


BOOL CGetProfileDlg::CheckDeviceProfilePath(usb_handle* handle) {
#define BUFFER_LEN (PATH_MAX + 64)
  const char *candidate[] = {
    "/usr/bin/profile/cust",
    "/usr/bin/profile/match",
    "/jrd-resource/resource/profile/cust",
    "/jrd-resource/resource/profile/match",
    NULL
  };
  const char * path =candidate[0];
  CHAR buffer[BUFFER_LEN];
  PCHAR resp = NULL;
  BOOL got = FALSE;
  int resp_len;
  if (handle == NULL) {
    ERROR("No adb device found.");
    return FALSE;
  }
  adbhost adb(handle , usb_port_address(handle));
  for (int index = 0; path != NULL; path= candidate[++index])
  {
    memset(buffer, 0, BUFFER_LEN);
    snprintf(buffer,BUFFER_LEN, "if [ -e %s ]; then echo T ; else echo F; fi", path);
    adb.shell(buffer, (void **)&resp, &resp_len);
    if (resp == NULL) {
      LOG("Occur adb error! No response get.");
      return FALSE;
    } else if (strncmp(resp, "T", 1) == 0) {
      LOG("Set profile path as %s.", path);
      m_DeviceProfilePath = path;
      got = TRUE;
    } else {
      WARN("Path '%s' is not exit in device",  path);
    }
    free(resp);
    resp = NULL;
    if (got)
      break;
  }

  return got;
}

VOID CGetProfileDlg::GetProfilesList(BOOL trySwitchDisk) {
  m_hUSBHandle = GetUsbHandle();
  if (m_hUSBHandle != NULL) {
  CheckDeviceProfilePath(m_hUSBHandle);
  DoGetProfilesList(m_hUSBHandle);
  } else if (trySwitchDisk) {
  PostMessage(UI_MESSAGE_INIT_DEVICE, (WPARAM)0, (LPARAM)NULL);
  }
}

VOID CGetProfileDlg::DoGetProfilesList(usb_handle* handle) {
  PCHAR resp = NULL;
  int resp_len;
  int ret;

  if (handle == NULL) {
    ERROR("No adb device found.");
    return;
  }
  adbhost adb(handle , usb_port_address(handle));
  //ret = adb.shell("cat /proc/version", (void **)&resp, &resp_len);

  CStringA command = "ls -1 ";
  command += m_DeviceProfilePath;
  //LOG("xxxxxxxxxxxxxxxxxxxxx %s", WideStrToMultiStr(command ));
  //LOG("xxxxxxxxxxxxxxxxxxxxx %s", command);

  ret = adb.shell(command, (void **)&resp, &resp_len);
  if (resp == NULL)
    return;

  m_hProfileList->DeleteAllItems();
  m_pProfiles.clear();

  //ParseProfilesList(resp, " \t", "\r\n");
  /*
  * Profile name may contains blank space, such as " "
  */
  ParseContent(resp, "\r\n", m_pProfiles);
  //sort(m_pProfiles.begin(), m_pProfiles.end());
  for (size_t index= 0; index < m_pProfiles.size(); index++) {
    m_hProfileList->InsertItem(index, MultiStrToWideStr(m_pProfiles[index]));
    //LOG(" %d --> %s", index, m_pProfiles[index]);
  }
  free(resp);
}

BOOL CGetProfileDlg::DoPokeProfile(usb_handle* handle, PCHAR profileName, PCHAR *data) {
  if (handle == NULL) {
    ERROR("DoPokeProfile: No adb device found.");
    return FALSE;
  }
  if (profileName == NULL || data == NULL) {
    ERROR("DoPokeProfile: Bad parameter.");
    return FALSE;
  }

  adbhost adb(handle , usb_port_address(handle));
#if 1
  PCHAR resp = NULL;
  int  resp_len;
  int ret;
  CStringA command = "cat \"" + m_DeviceProfilePath + "/" + profileName + "\"";
  ret = adb.shell(command, (void **)&resp, &resp_len);
  //LOG("Response %s", resp);
  if (resp == NULL)
    return FALSE;
  *data = resp;
#else
  CStringA path = m_DeviceProfilePath;
  path += "/";
  path += profileName;
  LOG("Profile path is %s", path);
  adb.sync_pull(path, ".");
#endif
  return TRUE;
}

BOOL CGetProfileDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
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
        if (m_bSwitchDisk == FALSE) {
          SetTimer(TIMER_SWITCH_DISK, 5000, NULL);
        }
        break;
      }
    case DBT_DEVTYP_DEVICEINTERFACE:
      {
        if (m_bSwitchDisk == FALSE) {
          KillTimer(TIMER_SWITCH_DISK);
        } else {
        m_bSwitchDisk = FALSE;
        }
        GetProfilesList(FALSE);
      }
      break;
    }
  } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
    if (phdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

      ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
      if (m_hUSBHandle != NULL)
        usb_close(m_hUSBHandle);

      m_hProfileDataList->ResetContent();
      m_hProfileName->SetWindowText(_T(""));
      m_hProfileList->DeleteAllItems();
      m_pProfiles.clear();
    }
  }

  return TRUE;
}

void CGetProfileDlg::OnDestroy()
	{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	StopLogging();
	}

void CGetProfileDlg::OnLvnItemchangedListProfile(NMHDR *pNMHDR, LRESULT *pResult)
	{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	}

void CGetProfileDlg::OnNMClickListProfile(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  // TODO: 在此添加控件通知处理程序代码
  *pResult = 0;
  CString profile = m_hProfileList->GetItemText(pNMLV->iItem, 0);
  //CString profile = m_pProfiles->GetAt(pNMLV->iItem);

  //LOG("Click item %s", WideStrToMultiStr(profile));
  PCHAR data = NULL;
  DoPokeProfile(m_hUSBHandle, WideStrToMultiStr(profile), &data);
  if (data != NULL) {
    LOG("Profile data %s", data);
    CString profileData = MultiStrToWideStr(data);
    m_hProfileDataList->ResetContent();

    // ParseProfileContent(data, " \t", "\r\n");

    int curPos = 0;
    CString resToken = profileData.Tokenize(_T("\r\n"),curPos);
    while (resToken != _T("")){
      m_hProfileDataList->AddString(resToken);
      resToken = profileData.Tokenize(_T("\r\n"), curPos);
    };

    m_hProfileName->SetWindowText(profile);
    free(data);
  }
}

void CGetProfileDlg::OnTimer(UINT_PTR nIDEvent)
{
  // TODO: 在此添加消息处理程序代码和/或调用默认值
  CDialog::OnTimer(nIDEvent);
  if (TIMER_PROFILE_LIST) {
    GetProfilesList(TRUE);
  } else if (TIMER_SWITCH_DISK){
  }

  KillTimer(nIDEvent);
}

LRESULT  CGetProfileDlg::OnInitDevice(WPARAM wParam, LPARAM lParam) {
#if 0
  size_t count = GetLogicalDriveStrings(0,NULL);
  TCHAR *pDriveStrings = new TCHAR[count+sizeof(_T(""))];
  GetLogicalDriveStrings(count,pDriveStrings);
  for (TCHAR* sDrivePath = pDriveStrings; *sDrivePath; sDrivePath += _tcslen(sDrivePath)+1)
  {
    LOG("Drive %s:", sDrivePath);
    if (GetDriveType(sDrivePath) == DRIVE_REMOVABLE) {
      TCHAR szPath[100] = _T("////.//");
      ::_tcscat(szPath,sDrivePath);
      int nSize = ::_tcslen(szPath);
      szPath[nSize-1] = '/0';
    }
  }
  delete[]   pDriveStrings;
#endif

  std::vector<CString> devicePath;
  GetDeviceByGUID(devicePath, &GUID_DEVINTERFACE_DISK);
  int cdromSize = devicePath.size();

  for(int i = 0; i < cdromSize; i++)
  {
    CString path = devicePath[i];
    if (path.Find(_T("\\\\?\\usbstor#")) == -1) {
      LOG("Fix DISK %S:", path);
    } else {

      path.MakeUpper();
      if (path.Find(_T("ONETOUCH")) == -1 && path.Find(_T("ALCATEL")) == -1)
        LOG("USB Stor %S is not alcatel",path);
      else {
        CSCSICmd scsi = CSCSICmd();
        LOG("do switch device %S", devicePath[i]);
        scsi.SwitchToDebugDevice(devicePath[i]);
        m_bSwitchDisk = TRUE;
        //scsi.SwitchToDebugDevice(_T("\\\\?\\H:"));
        break;
      }
    }
  }
  devicePath.clear();

  return 0;
}
#if 0
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
#endif
