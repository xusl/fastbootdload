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
    SetTimer(0, 1000, NULL);
  } else {
    m_hUSBHandle = GetUsbHandle();
    DoGetProfilesList(m_hUSBHandle);
  }

  //PostMessage(UI_MESSAGE_INIT_DEVICE, (WPARAM)0, (LPARAM)NULL);
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


BOOL CGetProfileDlg::ParseProfileContent(char * content ,
                                         PCHAR lineDelim) {
  char *str1, *token;
  char *saveptr1;
  int j;

  for (j = 1, str1 = content; ; j++, str1 = NULL) {
    token = strtok_s(str1, lineDelim, &saveptr1);
    if (token == NULL)
      break;
    m_pProfileData.push_back(token);

  }
  return TRUE;
}

VOID CGetProfileDlg::DoGetProfilesList(usb_handle* handle) {
  PCHAR resp = NULL;
  int  resp_len;
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
  ParseProfilesList(resp, " \t", "\r\n");
  //sort(m_pProfiles.begin(), m_pProfiles.end());
  for (size_t index= 0; index < m_pProfiles.size(); index++) {
    m_hProfileList->InsertItem(index, MultiStrToWideStr(m_pProfiles[index]));
    // LOG(" --> %s", m_pProfiles[index]);
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
  CStringA command = "cat ";
  command += m_DeviceProfilePath;
  command += "/";
  command += profileName;
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
        break;
      }
    case DBT_DEVTYP_DEVICEINTERFACE:
      {
        m_hUSBHandle = GetUsbHandle();
        DoGetProfilesList(m_hUSBHandle);
      }
      break;
    }
  } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
    if (phdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

      ASSERT(lstrlen(pDevInf->dbcc_name) > 4);

      long sn = usb_host_sn(pDevInf->dbcc_name);
      sn = get_adb_composite_device_sn(sn);
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

  m_hUSBHandle = GetUsbHandle();
  DoGetProfilesList(m_hUSBHandle);

  KillTimer(nIDEvent);
}


LRESULT  CGetProfileDlg::OnInitDevice(WPARAM wParam, LPARAM lParam) {
  return 0;
}
