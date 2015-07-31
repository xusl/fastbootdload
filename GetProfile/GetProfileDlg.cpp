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
    GetProfilesList();
  }

  PostMessage(UI_MESSAGE_INIT_DEVICE, (WPARAM)0, (LPARAM)NULL);
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

VOID CGetProfileDlg::GetProfilesList(VOID) {
  m_hUSBHandle = GetUsbHandle();
  CheckDeviceProfilePath(m_hUSBHandle);
  DoGetProfilesList(m_hUSBHandle);
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

  ParseProfilesList(resp, " \t", "\r\n");
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
        PDEV_BROADCAST_DEVICEINTERFACE pDevInf =
               (PDEV_BROADCAST_DEVICEINTERFACE)phdr;
            UpdateDevice(pDevInf, dwData);
        break;
      }
    case DBT_DEVTYP_DEVICEINTERFACE:
      {
        GetProfilesList();
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

  GetProfilesList();

  KillTimer(nIDEvent);
}


LRESULT  CGetProfileDlg::OnInitDevice(WPARAM wParam, LPARAM lParam) {
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

  std::vector<CString> cdromList;
  cdromList.clear();
  EnumCDROM(cdromList);

  uint8 cmdBuf[CDB6GENERIC_LENGTH] = {0x16, 0xf5, 0x0, 0x0, 0x0, 0x0};
  cmdBuf[1] = 0xf9;
  BOOL bOk = false;
  int cdromSize = cdromList.size();
  //for(int i = 0; i < cdromSize; i++)
  {
    bOk = Send(_T("\\\\?\\H:"), cmdBuf, sizeof(cmdBuf));
  }
  cdromList.clear();


  //CSCSICmd scsi = CSCSICmd();
  //scsi.Send("\\\\?\\H:");

  return 0;
}

BOOL CGetProfileDlg::Send(LPCWSTR devname,const uint8* cmd,uint32 cmdLen)
{
	BOOL result = FALSE;
	HANDLE handle = NULL;

	handle = CreateFile(devname,
                      GENERIC_WRITE | GENERIC_READ,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING, 0, NULL);

	if (handle == INVALID_HANDLE_VALUE) {
    ERROR("Open device %S failed.", devname);
		return result;
	}

	result = SendCmd(handle, cmd, cmdLen, 10);

	CloseHandle(handle);

	return result;
}

BOOL CGetProfileDlg::SendCmd(HANDLE handle, const uint8* cmd, uint32 len, uint64 timeout)
{
  BOOL result = FALSE;
  SCSI_PASS_THROUGH_WITH_BUFFERS sptdwb;
  uint64 length = 0;
  uint64 returned = 0;

  ZeroMemory(&sptdwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
  sptdwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
  sptdwb.spt.PathId = 0;
  sptdwb.spt.TargetId = 1;
  sptdwb.spt.Lun = 0;
  sptdwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
  sptdwb.spt.DataTransferLength = 192;
  sptdwb.spt.TimeOutValue = timeout;
  sptdwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
  sptdwb.spt.SenseInfoLength = SPT_SENSE_LENGTH;
  sptdwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
  sptdwb.spt.CdbLength = CDB6GENERIC_LENGTH;
  memcpy(sptdwb.spt.Cdb, cmd, len);
  length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf)
    + sptdwb.spt.DataTransferLength;

  result = DeviceIoControl(handle,
                        IOCTL_SCSI_PASS_THROUGH,
                        &sptdwb,
                        sizeof(SCSI_PASS_THROUGH),
                        &sptdwb,
                        length,
                        &returned,
                        NULL);
  if (result) {
    ERROR("##DeviceIoControl OK!");
  } else {
    ERROR("**DeviceIoControl fails!");
  }

  return result;
}

void CGetProfileDlg::UpdateDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam)
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
  if(INVALID_HANDLE_VALUE == hDevInfo) {
    AfxMessageBox(CString("SetupDiGetClassDevs(): ")
                  + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
    return;
  }

  SP_DEVINFO_DATA* pspDevInfoData = (SP_DEVINFO_DATA*)HeapAlloc(
                                        GetProcessHeap(), 0, sizeof(SP_DEVINFO_DATA));
  pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
  for(int i=0; SetupDiEnumDeviceInfo(hDevInfo,i,pspDevInfoData); i++) {
    DWORD DataT ;
    DWORD nSize=0 ;
    TCHAR buf[MAX_PATH];

    if (!SetupDiGetDeviceInstanceId(hDevInfo, pspDevInfoData, buf, sizeof(buf), &nSize)) {
      AfxMessageBox(CString("SetupDiGetDeviceInstanceId(): ")
                    + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
      break;
    }

    if ( szDevId == buf ) {
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

void CGetProfileDlg::GetInterfaceDeviceDetail(HDEVINFO hDevInfoSet)
{
  BOOL bResult;
  PSP_DEVICE_INTERFACE_DETAIL_DATA   pDetail   =NULL;
  SP_DEVICE_INTERFACE_DATA   ifdata;
  WCHAR ch[MAX_PATH];
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

  pDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)GlobalAlloc(LMEM_ZEROINIT,   predictedLength);
  pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
  bResult = SetupDiGetInterfaceDeviceDetail(hDevInfoSet,   /*设备信息集句柄*/
                                            &ifdata,   /*设备接口信息*/
                                            pDetail,   /*设备接口细节(设备路径)*/
                                            predictedLength,   /*输出缓冲区大小*/
                                            &requiredLength,   /*不需计算输出缓冲区大小(直接用设定值)*/
                                            NULL);   /*不需额外的设备描述*/

  if(bResult) {
    memset(ch, 0, MAX_PATH);
    /*复制设备路径到输出缓冲区*/
    for(i=0; i<requiredLength; i++) {
      ch[i]=*(pDetail->DevicePath+8+i);
    }
    printf("%s\r\n", ch);
  }
}


void CGetProfileDlg::EnumCDROM(std::vector<CString>& m_Cdroms) {
  m_Cdroms.clear();
  char szDevDesc[256] = {0};
  GUID guidDev = {0x53f56308L, 0xb6bf, 0x11d0,{0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};

  HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

  hDevInfo = SetupDiGetClassDevs(&guidDev,
                                 NULL,
                                 NULL,
                                 DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfo == INVALID_HANDLE_VALUE)  {
    return ;
  }

  if ( INVALID_HANDLE_VALUE == hDevInfo)
    return;

  SP_DEVINFO_DATA devInfoElem;
  devInfoElem.cbSize = sizeof(SP_DEVINFO_DATA);
  SP_DEVICE_INTERFACE_DATA ifcData;
  ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  DWORD dwDetDataSize = 0;

  SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

  for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guidDev, i, &ifcData); ++ i) {
    SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

    // Get buffer size first
    SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, &dwDetDataSize, NULL);

    if (dwDetDataSize != 0) {
      pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new char[dwDetDataSize]);
      pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    }

    BOOL bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData, dwDetDataSize, NULL, &devdata);
    if(bOk) {
      WCHAR buf[512] = {0};
      memcpy(buf, pDetData->DevicePath, sizeof (WCHAR) * wcslen(pDetData->DevicePath));
      m_Cdroms.push_back(buf);
      delete pDetData;
      pDetData = NULL;
    }
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
}
