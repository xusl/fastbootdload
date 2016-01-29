#include "stdafx.h"

#include "adb_dev_register.h"
#include "comdef.h"
#include "log.h"

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")

DeviceInterfaces::DeviceInterfaces() :
    mActiveIntf(NULL),
    mAdb(NULL),
    mDiag(NULL),
    mFastboot(NULL),
    mDeviceActive(TRUE)
{
    const char *default_tag = "N/A Device";
    strncpy_s(mTag, DEV_TAG_LEN, default_tag, strlen(default_tag));
}

DeviceInterfaces::DeviceInterfaces(const DeviceInterfaces & devIntf) {
    DeviceInterfaces::operator =(devIntf);
}

CDevLabel* DeviceInterfaces::GetActiveIntf() const{
    return mActiveIntf;
}

CDevLabel* DeviceInterfaces::GetAdbIntf() const {
    return mAdb;
}

CDevLabel* DeviceInterfaces::GetDiagIntf() const {
    return mDiag;
}

CDevLabel* DeviceInterfaces::GetFastbootIntf() const {
    return mFastboot;
}

BOOL DeviceInterfaces::GetDeviceStatus() const {
    return mDeviceActive;
}

VOID DeviceInterfaces::SetDeviceStatus(BOOL status) {
    mDeviceActive = status;
}

VOID DeviceInterfaces::SetActiveIntf( CDevLabel* intf) {
    mActiveIntf = intf;
}

CDevLabel* DeviceInterfaces::SetAdbIntf(CDevLabel& intf) {
    DELETE_IF(mAdb);
    mAdb = new CDevLabel(intf);
    return mAdb;
}

CDevLabel* DeviceInterfaces::SetDiagIntf(CDevLabel& intf) {
    DELETE_IF(mDiag);
    mDiag = new CDevLabel(intf);
    return mDiag;
}

CDevLabel* DeviceInterfaces::SetFastbootIntf(CDevLabel& intf) {
    DELETE_IF(mFastboot);
    mFastboot = new CDevLabel(intf);
    return mFastboot;
}

VOID DeviceInterfaces::UpdateDevTag() {
    long sn;
    long port;
    if(mAdb != NULL) {
        mAdb->GetEffectiveSnPort(&sn, &port);
    } else if(mFastboot != NULL) {
        mFastboot->GetEffectiveSnPort(&sn, &port);
    }

    if(mDiag != NULL) {
        //wsprintf
    snprintf(mTag, DEV_TAG_LEN, "COM%d (0X%X.%d)", mDiag->GetComPortNum(), sn, port);
    } else {
     snprintf(mTag, DEV_TAG_LEN, "Device: 0X%X (%d)", sn, port);
    }
}

const char *DeviceInterfaces::GetDevTag() const {
    return mTag;
}

int DeviceInterfaces::GetDevId() {
    long sn;
    long port;

    if(mDiag != NULL)
        return mDiag->GetComPortNum();

    if(mActiveIntf != NULL) {
        mActiveIntf->GetEffectiveSnPort(&sn, &port);
        return sn+port;
    }

    if(mAdb != NULL) {
        mAdb->GetEffectiveSnPort(&sn, &port);
        return sn+port;
    }

    if(mFastboot != NULL) {
        mFastboot->GetEffectiveSnPort(&sn, &port);
        return sn+port;
    }
    return 0xdeadbeef;
}

BOOL DeviceInterfaces::SetIntf(CDevLabel& dev, TDevType type, BOOL updateActiveIntf) {
    CDevLabel* pDev = NULL;
    if (type == DEVTYPE_DIAGPORT) {
        pDev = SetDiagIntf(dev);
    } else if(type == DEVTYPE_ADB) {
        pDev = SetAdbIntf(dev);
    } else if(type == DEVTYPE_FASTBOOT) {
        pDev = SetFastbootIntf(dev);
    }
    if(pDev != NULL && updateActiveIntf)
        SetActiveIntf(pDev);
    UpdateDevTag();
    return TRUE;
}

bool DeviceInterfaces::operator ==(const DeviceInterfaces * const & devIntf) const{
    LOGE("DO DeviceInterfaces::operator ==");
    if (this == devIntf)
        return true;

    CDevLabel* adb = devIntf->GetAdbIntf();
    CDevLabel* diag = devIntf->GetDiagIntf();
    CDevLabel* fb = devIntf->GetFastbootIntf();

    if (mAdb != NULL && (mAdb->Match(adb) || mAdb->Match(diag) ||mAdb->Match(fb)))
        return true;

    if (mDiag != NULL && (mDiag->Match(diag) || mDiag->Match(adb) ||mDiag->Match(fb)))
        return true;

    if (mFastboot != NULL && (mFastboot->Match(fb) ||mFastboot->Match(adb) || mFastboot->Match(diag)))
        return true;

    return false;
}

bool DeviceInterfaces::MatchDevPath(const wchar_t * devPath) const {
    LOGE("DO DeviceInterfaces::operator == 2");
    if ( *mAdb == devPath || *mDiag == devPath ||
        *mFastboot == devPath)
        return true;
    return false;
}

DeviceInterfaces & DeviceInterfaces::operator =(const DeviceInterfaces & devIntfs) {
    mActiveIntf = devIntfs.GetActiveIntf();
    mAdb = devIntfs.GetAdbIntf();
    mDiag = devIntfs.GetDiagIntf();
    mFastboot = devIntfs.GetFastbootIntf();
    mDeviceActive = devIntfs.GetDeviceStatus();
    strcpy(mTag, devIntfs.GetDevTag());
    return *this;
}

VOID DeviceInterfaces::DeleteMemory(VOID) {
    DELETE_IF(mAdb);
    DELETE_IF(mDiag);
    DELETE_IF(mFastboot);
}

DeviceInterfaces::~DeviceInterfaces()
{
    mActiveIntf = NULL;
}


DeviceCoordinator::DeviceCoordinator() :
    mDevintfList()
{
    ;
}

DeviceCoordinator::~DeviceCoordinator() {
    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        DeviceInterfaces* item = *it;
        item->DeleteMemory();
        delete item;
        //*it = NULL;
    }
    mDevintfList.clear();
}

BOOL DeviceCoordinator::GetDevice(const wchar_t * const devPath, DeviceInterfaces** outDevIntf) {
#if 0
    list<DeviceInterfaces>::iterator it = find(mDevintfList.begin(), mDevintfList.end(), devPath);
    if (it != mDevintfList.end()) {
        *outDevIntf = *it;
        return TRUE;
    }
    return FALSE;
#endif
    ASSERT(devPath != NULL);
    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        if((*it)->MatchDevPath(devPath) && (*it)->GetDeviceStatus()) {
            *outDevIntf = *it;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL DeviceCoordinator::CreateDevice(CDevLabel& dev, TDevType type, DeviceInterfaces** outDevIntf) {
    DeviceInterfaces temp;
    temp.SetIntf(dev, type);

    list<DeviceInterfaces*>::iterator it;
    //it = find(mDevintfList.begin(), mDevintfList.end(), temp);
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        DeviceInterfaces* item = *it;
        if(temp == item) break;
    }
    if (it != mDevintfList.end()) {
        LOGI("Update exist device");
        (*it)->SetDeviceStatus(TRUE);
        (*it)->SetIntf(dev, type);
        if (outDevIntf != NULL)
            *outDevIntf = *it;
    } else {
        LOGI("Create new device");
        DeviceInterfaces* newDevIntf = new DeviceInterfaces(temp);
        newDevIntf->SetDeviceStatus(TRUE);
        AddDevice(newDevIntf);
        if (outDevIntf != NULL)
            *outDevIntf = newDevIntf;
    }

    return TRUE;
}

BOOL DeviceCoordinator::AddDevice(DeviceInterfaces* const &devIntf)  {
    mDevintfList.push_back(devIntf);
    return TRUE;
}

BOOL DeviceCoordinator::RemoveDevice(DeviceInterfaces* const & devIntf)  {
    mDevintfList.remove(devIntf);
    devIntf->DeleteMemory();
    delete devIntf;
    return TRUE;
}


//\\?\usbstor#disk&ven_onetouch&prod_link4&rev_2.31#6&21c8898b&1&0123456789abcdef&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
//For id , it is "6&21c8898b&1&0123456789abcdef&0".
//"6&21c8898b&1 is assigned by system, it is call parentIdPrefix, it is assigned by Windows. we use the second parent is enough.
// "0123456789abcdef" is the usb serial number in usb description, report by device,
// if "SYSTEM\\CurrentControlSet\\Control\\UsbFlags" is set, the Windows will abandon this value.
// so , device id reduces , "6&21c8898b&1&0"
//the last "0" is the port, assign by windows.
CDevLabel::CDevLabel(const wchar_t * devPath,
                     const wchar_t* parentIdPrefix, bool useParentIdPrefix, const wchar_t * name):
    mPortNum(~1),
    mEffectiveSn(~1),
    mEffectivePort (~1)
{
    SetServiceName(name);
    CopyDeviceDescPath(devPath, parentIdPrefix);
    mUseParentIdPrefix = useParentIdPrefix;

    memset(mDevId, 0, sizeof mDevId);
    if (devPath != NULL){
        wchar_t delimits[] = {L'#', L'#', L'#'};
        wchar_t *sne = (wchar_t *)devPath;
        wchar_t *snb;
        int len = wcslen(devPath);
        for (int i = 0; i < sizeof(delimits)/sizeof(wchar_t); i++, sne++) {
            wchar_t sep = delimits[i];
            snb = sne;
            sne = (wchar_t*)wcschr(sne, sep);
            if (sne == NULL || sne - devPath >= len) {
                LOGE("In step %d , %c is not found", i, sep);
                return;
            }
        }
        len = sne - snb - 1;
        wcsncpy_s(mDevId, DEV_ID_LEN, snb, len);

    mEffectiveSn = usb_host_sn(devPath);
    mEffectivePort = usb_host_sn_port(devPath);
    } else {
        LOGE("Can not get invalid device id");
    }

    memset(mMatchId, 0, sizeof mMatchId);
    if (mUseParentIdPrefix && parentIdPrefix != NULL) {
        SetMatchId(parentIdPrefix);
    } else if (wcslen(mDevId) > 0) {
      wchar_t *sne = ( wchar_t*)wcsrchr(mDevId , L'&');
      if (sne != NULL)
          wcsncpy_s(mMatchId, DEV_ID_LEN, mDevId, sne - mDevId);
      else
        LOGE("Can not find '&' in dev path.");
    }else {
        LOGE("Can not get invalid device id");
    }

    //LOGI("CDevLabel:: ID:\t%S\n\tDevice Path:%S \n\tParentIDPrefix:%S",
    //     mDevId, devPath, parentIdPrefix);
#if 0
    LOGD("CDevLabel:: ID: %S", mDevId);
    LOGD("\tDevice Path: %S", devPath);
    LOGD("\tParentIDPrefix: %S", parentIdPrefix);
    LOGD("\tmMatchId: %S", mMatchId);
#endif
}

CDevLabel::CDevLabel(const CDevLabel & dev) {
    //LOGI("Enter copy constructor");
    CDevLabel::operator=(dev);
}

CDevLabel::~CDevLabel() {
    FREE_IF(mDevPath);
    FREE_IF(mParentIdPrefix);
    mEffectiveSn = ~1;
    mEffectivePort = ~1;
    //LOGI("~CDevLabel:: %S : %S", mDevPath, mParentIdPrefix);
}

void CDevLabel::CopyDeviceDescPath(const wchar_t * devPath, const wchar_t* parentIdPrefix) {
    if (devPath != NULL) {
        mDevPath = _wcsdup(devPath);

        if(mDevPath == NULL)
            LOGE("strdup failed");
    } else {
        mDevPath = NULL;
    }

    if (parentIdPrefix != NULL) {
        mParentIdPrefix = _wcsdup(parentIdPrefix);
        if(mParentIdPrefix == NULL)
            LOGE("strdup failed");
    } else {
        mParentIdPrefix = NULL;
    }
}

VOID CDevLabel::FreeBuffer() {
    FREE_IF(mDevPath);
    FREE_IF(mParentIdPrefix);
}

const wchar_t * CDevLabel::GetDevPath() const
{
    return mDevPath;
}

const wchar_t * CDevLabel::GetParentIdPrefix() const
{
    return mParentIdPrefix;
}

const wchar_t * CDevLabel::GetMatchId() const
{
    return mMatchId;
}

bool CDevLabel::SetMatchId(const wchar_t * matchId){
    if (matchId == NULL )
        return false;
    int len =wcslen(matchId);
    if (len == 0 || len >= DEV_MATCHID_LEN) {
        LOGE("matchId length exceed buffer length");
        return false;
    }
    return (0 == wcsncpy_s(mMatchId, DEV_MATCHID_LEN, matchId, len));
}

const wchar_t * CDevLabel::GetDevId() const {
    return mDevId;
}

bool CDevLabel::SetDevId(const wchar_t * devId) {
    if (devId == NULL )
        return false;
    int len =wcslen(devId);
    if (len == 0 || len >= DEV_ID_LEN) {
        LOGE("devId length exceed buffer length");
        return false;
    }
    return (0 == wcsncpy_s(mDevId, DEV_ID_LEN, devId, len));
}

const wchar_t * CDevLabel::GetServiceName()  const {
    return mServiceName;
}

bool CDevLabel::SetServiceName(const wchar_t * name) {
    if (name == NULL) {
        memset(mServiceName, 0, sizeof mServiceName);
        return false;
    }
    int len = wcslen(name);
    if (len == 0 || len >= DEV_SERVICE_LEN) {
        LOGE("name length exceed buffer length");
        return false;
    }
    wcsncpy_s(mServiceName, DEV_SERVICE_LEN, name, len);
    return true;
}

bool CDevLabel::GetEffectiveSnPort(long *sn, long *port) {
    *sn = mEffectiveSn;
    *port = mEffectivePort;
    return true;
}

bool CDevLabel::SetEffectiveSnPort(long sn, long port)
{
    mEffectiveSn = sn;
    mEffectivePort = port;
    return true;
}

bool CDevLabel::SetComPort(const wchar_t *portName) {
    //LOGI("PortName %S", portName);
    //swscanf
    swscanf_s(portName, _T("COM%d"), &mPortNum);
    //LOGI("Set comport %d", mPortNum);
    return true;
}

int CDevLabel::GetComPortNum() const {
    //LOGI("return port %d", mPortNum);
    return mPortNum;
}

bool CDevLabel::MatchDevPath(const wchar_t * devPath) {
    LOGE("DO CDevLabel::operator ==");
    const wchar_t *effectivePath = GetDevPath();
    if (effectivePath != NULL && devPath != NULL &&
        0 == wcscmp(effectivePath, devPath))
        return true;
    return false;
}


bool CDevLabel::Match(const CDevLabel * const & dev) const {
    const wchar_t *effectivePath = NULL;
    const wchar_t *effectivePath2 = NULL;
    LOGE("DO CDevLabel::operator == 2");
    if (dev == NULL)
        return false;
    if (this == dev)
        return true;

    effectivePath = GetDevPath();
    effectivePath2 = dev->GetDevPath();

    if (effectivePath != NULL && effectivePath2 != NULL &&
        0 == wcscmp(effectivePath, effectivePath2)) //stricmp
        return true;


//TPST Upgrading device path:
//\\?\usb#vid_1bbb&pid_007a#5&10cd67f3&0&3#{86e0d1e0-8089-11d0-9ce4-08003e301f73}
//Adb
//\\?\usb#vid_1bbb&pid_0196#5&10cd67f3&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
//fastboot
//\\?\usb#vid_18d1&pid_d00d#5&10cd67f3&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}

//In Debug mode:
//adb interface
// parent: 6&1e805b40&1
// Devpath: \\?\usb#vid_1bbb&pid_0196#5&10cd67f3&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
//com port
//parent: (null)
// Devpath: \\?\usb#vid_1bbb&pid_0196&mi_02#6&1e805b40&1&0002#{86e0d1e0-8089-11d0-9ce4-08003e301f73}
    effectivePath = GetDevId();
    effectivePath2 = dev->GetDevId();
    if (effectivePath != NULL && effectivePath2 != NULL &&
        (0 == wcscmp(effectivePath, effectivePath2) ))
        return true;//stricmp

    if (mUseParentIdPrefix || dev->GetParentIdPrefix()) {
        effectivePath = GetMatchId();
        effectivePath2 =dev->GetMatchId();
    }

    if (effectivePath != NULL && effectivePath2 != NULL &&
        (0 == wcscmp(effectivePath, effectivePath2) ))
        return true;

    return false;
}


bool CDevLabel::operator ==(const CDevLabel & dev) const {
    LOGE("DO CDevLabel::operator == 3");
    return Match(&dev);
}

CDevLabel & CDevLabel::operator =(const CDevLabel & dev) {
    //FREE_IF(mDevPath);
    //FREE_IF(mParentIdPrefix);
    CopyDeviceDescPath(dev.GetDevPath(), dev.GetParentIdPrefix());
    mUseParentIdPrefix = dev.mUseParentIdPrefix;
    mEffectiveSn = dev.mEffectiveSn;
    mEffectivePort = dev.mEffectivePort;
    mPortNum = dev.GetComPortNum();
    wcscpy_s(mDevId, dev.GetDevId());
    wcscpy_s(mMatchId, dev.GetMatchId());
    wcscpy_s(mServiceName, dev.GetServiceName());
    return *this;
}


//https://msdn.microsoft.com/en-us/library/windows/desktop/aa363432%28v=vs.85%29.aspx
BOOL RegisterAdbDeviceNotification(IN HWND hWnd, OUT HDEVNOTIFY *phDeviceNotify) {
   //注册插拔事件
   HDEVNOTIFY hDevNotify;

   DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
   if (hWnd == NULL) {
    	DEBUG("RegisterDeviceNotification: Bad parameter.\n");
      return FALSE;
   }
   ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
   NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
   NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
   for(int i=0; i<sizeof(usb_class_id)/sizeof(GUID); i++)
   {
      NotificationFilter.dbcc_classguid = usb_class_id[i];
      hDevNotify = RegisterDeviceNotification(hWnd,
		  &NotificationFilter,
		  DEVICE_NOTIFY_WINDOW_HANDLE);
      if( !hDevNotify )
      {
    		  DEBUG("RegisterDeviceNotification failed: %d\n", GetLastError());
         return FALSE;
      }
   }
   //A bit ugly: usb_class_id must have only one element.
   if (NULL != phDeviceNotify)
        *phDeviceNotify = hDevNotify;
   return TRUE;
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


void SetUpAdbDevice(
    PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam)
{
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
      if (_wcsnicmp((const wchar_t *)srv, L"usbccgp",nSize/sizeof(wchar_t))) {
        continue;
      }
    }

    _snwprintf_s(key, MAX_PATH,L"SYSTEM\\CurrentControlSet\\Enum\\%s", buf);
    lResult = reg.Open(HKEY_LOCAL_MACHINE,key, KEY_READ);
    if (lResult == ERROR_SUCCESS) {
      nSize = MAX_PATH;
      lResult = reg.QueryStringValue(L"ParentIdPrefix", static_cast<LPTSTR>(value), &nSize);

      if (lResult == ERROR_SUCCESS ) {
        //add_adb_device(buf, value);
      }
      reg.Close();
    }
  }

  //dump_adb_device();

  if ( pspDevInfoData ) {
    HeapFree(GetProcessHeap(), 0, pspDevInfoData);
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
}

/*
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
*/

//5&10cd67f3&0&4 5&10cd67f3&0
long extract_serial_number(wchar_t * sn, wchar_t **ppstart , wchar_t **ppend) {
  unsigned int len;
  wchar_t *pstart, *pend;
  if (sn == NULL) {
    ERROR("Bad Parameter");
    return -1;
  }

  len = wcslen (sn);

  pstart = (wchar_t*)wcschr(sn, L'&');
  if (pstart == NULL || pstart - sn >= len) {
    //ERROR("Can not find first '&'.");
    return -1;
  }

  pstart++;
  pend = wcschr(pstart , L'&');
  if (pend == NULL || pend - sn >= len) {
    //ERROR("Can not find first '&'.");
    return -1;
  }

  if (ppstart != NULL)
    *ppstart = pstart;
  if (ppend != NULL)
    *ppend = pend;
  return wcstol(pstart , &pend, 16);
}

// \\?\usb#vid_18d1&pid_d00d#5&10cd67f3&0&4#{f72fe0d4-cbcb-407d-8814-9ed673d0dd6b}
// convert to
// 10cd67f3
long usb_host_sn(const wchar_t* dev_name, wchar_t** psn) {
    wchar_t * snb, *sne, * sn;
    int len = wcslen (dev_name); //lstrlen, lstrcmp()
    if(_wcsnicmp(L"\\\\?\\",dev_name,4) ) {
        LOGE("Not valid dev name: %S.", dev_name);
        return 0;
    }

    //strtok is not suitable;
    wchar_t delimits[] = {L'#', L'#', L'&', L'&'};
    sne = (wchar_t *)dev_name;
    for (int i = 0; i < sizeof(delimits)/sizeof(wchar_t) ; i++, sne++) {
        wchar_t sep = delimits[i];
        snb = sne;
        sne = (wchar_t*)wcschr(sne, sep);
        if (sne == NULL || sne - dev_name >= len) {
            LOGE("In step %d , %c is not found", i, sep);
            return 0;
        }
    }

    len = sne - snb;
    if (len <= 0)
        return 0;

    if (psn) {
        sn = (wchar_t*) malloc((len + 1) * sizeof(wchar_t));
        if (sn == NULL) {
            ERROR("NO memory");
        } else {
            wcsncpy(sn, snb , len);
            *(sn + len) = L'\0';
        }
        *psn = sn;
    }

    return wcstol(snb, &sne, 16);
}

long usb_host_sn_port(const wchar_t* dev_name) {
     wchar_t* begin , *end;
    if (dev_name == NULL)
        return 0;

      begin = ( wchar_t*)wcsrchr(dev_name , L'&');

        if (begin == NULL )
            return 0;

        begin ++;
          end = begin + 1;
  if (*(begin + 1) == L'#' && *(begin - 3) == L'&')
    return wcstol(begin , &end, 16);

  return 0;
}


//\\?\ide#cdromhp_dvd-rom_ts-h353c_____________________h430____#4&32dd3bc3&0&0.1.0#{53f56308-b6bf-11d0-94f2-00a0c91efb8b}
//\\?\ide#diskhitachi_hds721050cla362_________________jp2oa3gh#4&32dd3bc3&0&0.0.0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
//\\?\usbstor#disk&ven_onetouch&prod_mobilebroadband&rev_2.31#7&15b65f8e&0&0bdffe1f22df&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
/* if ClassGuid is NULL, will enumerate also devices. */
BOOL GetDeviceByGUID(std::vector<CString>& devicePaths, const GUID *pClsGuid) {
  HDEVINFO hDevInfo = SetupDiGetClassDevs(pClsGuid,
                                          NULL,
                                          NULL,
                                          DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfo == INVALID_HANDLE_VALUE)  {
    WARN("SetupDiGetClassDevs return invalid handle.");
    return FALSE;
  }

  devicePaths.clear();

  SP_DEVICE_INTERFACE_DATA ifcData;
  ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pClsGuid, i, &ifcData); ++ i) {
    DWORD dwDetDataSize = 0;
    SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

    // Get buffer size first
    SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, &dwDetDataSize, NULL);

    SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;
    if (dwDetDataSize == 0) {
      WARN("SetupDiGetDeviceInterfaceDetail Get empty data.");
      continue;
    }
    pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new BYTE[dwDetDataSize]);
    memset(pDetData, 0, dwDetDataSize*sizeof(char));
    pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    if(SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData,
                                       dwDetDataSize, NULL, &devdata)) {
      devicePaths.push_back(pDetData->DevicePath);
      #if 0
      WCHAR buffer[_MAX_PATH];
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_FRIENDLYNAME, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("Friendlyname %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_SERVICE, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_SERVICE %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_DEVICEDESC, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_DEVICEDESC %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_HARDWAREID, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_HARDWAREID %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_LOCATION_INFORMATION, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_LOCATION_INFORMATION %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_PHYSICAL_DEVICE_OBJECT_NAME %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_LOCATION_PATHS, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_LOCATION_PATHS %S", buffer);
      }
      if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata,
           SPDRP_ADDRESS, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
        INFO("SPDRP_ADDRESS %S", buffer);
      }
      INFO("== ==== == == == == == == == == == == == == == == == == == == ==");
      #endif
    }
    delete pDetData;
    pDetData = NULL;
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
  return TRUE;
}

BOOL GetDevLabelByGUID(CONST GUID *pClsGuid, PCWSTR service,
        vector<CDevLabel>& labels,  bool useParentIdPrefix)
{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    if(pClsGuid == NULL || service == NULL)
        return FALSE;

    hDevInfo = SetupDiGetClassDevs(pClsGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if( INVALID_HANDLE_VALUE == hDevInfo )  {
        LOGE("SetupDiGetClassDevs: %S", _com_error(GetLastError()).ErrorMessage());
        return FALSE;
    }

    SP_DEVICE_INTERFACE_DATA ifcData;
    ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pClsGuid, i, &ifcData); ++ i) {
        DWORD nSize=0;
        SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

        // Get buffer size first
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, &nSize, NULL);
        if (nSize == 0) {
            LOGE("SetupDiGetDeviceInterfaceDetail Get empty data.");
            continue;
        }

        SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;
        pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new BYTE[nSize]);
        if(NULL == pDetData) {
            LOGE("new arrary failed. No memory.");
            continue;
        }
        memset(pDetData, 0, nSize*sizeof(BYTE));
        pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if(!SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData,
                                            nSize, NULL, &devdata)) {
            LOGE("Get empty data.");
            FREE_IF(pDetData);
            continue;
        }

        HKEY hKey = NULL;
        TCHAR buffer[_MAX_PATH] = {0};
        TCHAR portName[_MAX_PATH] = {0};
        TCHAR key[MAX_PATH] = {0};
        PCWCH pParentIdPrefix = NULL;
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_SERVICE,
                                              NULL, (PBYTE)buffer, sizeof(buffer), &nSize)) {
            LOGE("get SPDRP_SERVICE failed");
            FREE_IF(pDetData);
            continue;
        }
        if (_wcsnicmp((const wchar_t *)buffer, service, nSize/sizeof(wchar_t))) {
            LOGE("SPDRP_SERVICE return '%S', does not match '%S'.", buffer, service);
            FREE_IF(pDetData);
            continue;
        }

        if (!SetupDiGetDeviceInstanceId(hDevInfo, &devdata, buffer, sizeof(buffer), &nSize)) {
            LOGD("SetupDiGetDeviceInstanceId(): %S", _com_error(GetLastError()).ErrorMessage());
        } else {
            _snwprintf_s(key, MAX_PATH,L"SYSTEM\\CurrentControlSet\\Enum\\%s", buffer);
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                LOGD("RegOpenKeyExW error");
            } else {
                nSize = sizeof buffer;
                if (RegQueryValueExW(hKey, L"ParentIdPrefix", NULL, NULL,
                                     (LPBYTE)&buffer, &nSize) != ERROR_SUCCESS) {
                    LOGD("RegQueryValueExW error %S", buffer);
                } else {
                    pParentIdPrefix = buffer;
                }
                RegCloseKey(hKey);
            }
        }

        if(useParentIdPrefix && pParentIdPrefix == NULL) {
            FREE_IF(pDetData);
            continue;
        }

        CDevLabel devId(pDetData->DevicePath, pParentIdPrefix, useParentIdPrefix, service);

        hKey = SetupDiOpenDevRegKey(hDevInfo, &devdata, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if (hKey == NULL) {
            LOGE("SetupDiOpenDevRegKey %S failed",pDetData->DevicePath);
            continue;
        }

        nSize = sizeof(portName);
        if (RegQueryValueExW(hKey, L"PortName", NULL, NULL,(LPBYTE)&portName,
                             &nSize) == ERROR_SUCCESS) {
            devId.SetComPort(portName);
        }

        //LOGW("DEV PATH %S, parentID %S", pDetData->DevicePath, buffer);
        labels.push_back(devId);
        RegCloseKey(hKey);
        FREE_IF(pDetData);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return TRUE;
}


/*
* same pid, vid, and serial number, will case Windows blue screen.
*/
void check_regedit_usbflags(usbid_t USBIds[], unsigned count){
  CRegKey reg;
  WCHAR szName[256];
  BYTE szValue;
  DWORD dwCount = sizeof(szValue);

  long lResult = reg.Open(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\UsbFlags");
  if (lResult != ERROR_SUCCESS) {
    ERROR("error code:%d",lResult);
    return ;
  }

  for (unsigned i = 0; i <= count; i++) {
    if (i == count ) {
      //wcsdup
      _snwprintf_s(szName, sizeof(szName)/sizeof(szName[0]), L"GlobalDisableSerNumGen");
    } else {
      _snwprintf_s(szName, sizeof(szName)/sizeof(szName[0]), L"IgnoreHWSerNum%04X%04X",
                   USBIds[i].vid, USBIds[i].pid);
    }
    //ZeroMemory(szValue, sizeof(szValue));
    szValue = 0;

    lResult = reg.QueryBinaryValue(szName, &szValue, &dwCount);
    //DEBUG("dwCount is %d, lResult %d" , dwCount, lResult);
#if 0
    if (lResult == ERROR_FILE_NOT_FOUND) {
      // _snwprintf_s(szName,
      //    sizeof(szName),
      //    L"SYSTEM\\ControlSet001\\Control\\UsbFlags\\IgnoreHWSerNum%X%X",
      //    0X1BBB,
      //    0X0192);
      //lResult = reg.Create(HKEY_LOCAL_MACHINE, szName );
      WCHAR szValue[1]={1};
      lResult = reg.SetKeyValue(szName, szValue, szName);
      DEBUG("Create lResult %d" , lResult);
    }
#endif
    if (lResult != ERROR_SUCCESS || szValue != 1) {
      szValue = 1;
      dwCount = sizeof(szValue);
      lResult = reg.SetBinaryValue(szName, &szValue, dwCount);
      //TODO:: Notify USER to remove usb device. for assign id.
      //DEBUG("SetBinaryValue lResult %d" , lResult);
    }
  }

  reg.Flush();
  reg.Close();
}

