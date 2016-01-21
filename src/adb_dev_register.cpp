#include "stdafx.h"

#include "adb_dev_register.h"
#include "comdef.h"
#include "log.h"

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")

//\\?\usbstor#disk&ven_onetouch&prod_link4&rev_2.31#6&21c8898b&1&0123456789abcdef&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
//For id , it is "6&21c8898b&1&0123456789abcdef&0".
//"6&21c8898b&1 is assigned by system, it is call parentIdPrefix, it is assigned by Windows.
// "0123456789abcdef" is the usb serial number in usb description, report by device,
//the last "0" is the port, assign by windows.
CDevLabel::CDevLabel(const wchar_t * devPath, const wchar_t* usbBus, bool useBusPath) {
    CopyDeviceDescPath(devPath, usbBus);
    mUseBusPath = useBusPath;
    GenEffectiveSnPort();
}
CDevLabel::~CDevLabel() {
    //FREE_IF(mDevPath);
    //FREE_IF(mBusControllerPath);
    mEffectiveSn = ~1;
    mEffectivePort = ~1;
}

bool CDevLabel::SetUseControllerPathFlag(bool useBus)
{
    mUseBusPath = useBus;
    GenEffectiveSnPort();
    return true;
}

void CDevLabel::CopyDeviceDescPath(const wchar_t * devPath, const wchar_t* usbBus) {
    if (devPath != NULL) {
        mDevPath = _wcsdup(devPath);

    if(mDevPath == NULL)
            LOGE("strdup failed");
    } else {
        mDevPath = NULL;
    }
    if (usbBus != NULL) {
        mBusControllerPath = wcsdup(usbBus);
        if(mBusControllerPath == NULL)
            LOGE("strdup failed");
    } else {
        mBusControllerPath = NULL;
    }
}

const wchar_t * CDevLabel::GetEffectivePath()
{
    if (mUseBusPath)
        return mBusControllerPath;
    else
        return mDevPath;
}

bool CDevLabel::SetEffectiveSnPort(long sn, long port)
{
    mEffectiveSn = sn;
    mEffectivePort = port;
    return true;
}
bool CDevLabel::GenEffectiveSnPort(void){
    const wchar_t *effectivePath = GetEffectivePath();
    if (effectivePath == NULL ) {
        LOGE("Can not get effective path");
        return false;
    }
    mEffectiveSn = usb_host_sn(effectivePath);
    mEffectivePort = usb_host_sn_port(effectivePath);
    return true;
}

bool CDevLabel::operator ==(CDevLabel & dev) {
    const wchar_t *effectivePath = GetEffectivePath();
    const wchar_t *effectivePath2 = dev.GetEffectivePath();
    if (effectivePath == NULL || effectivePath2 == NULL)
        return false;
    return (0 == wcscmp(effectivePath, effectivePath2) );//stricmp
}

CDevLabel & CDevLabel::operator =(const CDevLabel & dev) {
    FREE_IF(mDevPath);
    FREE_IF(mBusControllerPath);
    CopyDeviceDescPath(dev.mDevPath, dev.mBusControllerPath);
    mUseBusPath = dev.mUseBusPath;
    mEffectiveSn = dev.mEffectiveSn;
    mEffectivePort = dev.mEffectivePort;
    return *this;
}


//https://msdn.microsoft.com/en-us/library/windows/desktop/aa363432%28v=vs.85%29.aspx
BOOL RegisterAdbDeviceNotification(IN HWND hWnd, OUT HDEVNOTIFY *phDeviceNotify) {
   //×¢²á²å°ÎÊÂ¼þ
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
      if (wcsnicmp((const wchar_t *)srv, L"usbccgp",nSize/sizeof(wchar_t))) {
        continue;
      }
    }

    _snwprintf_s(key, MAX_PATH,L"SYSTEM\\CurrentControlSet\\Enum\\%s", buf);
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

//SERVICE : L"disk", L"usbccgp"
BOOL GetDevLabelByGUID(CONST GUID *pClsGuid, PCWSTR service, std::vector<CDevLabel>& labels)
{
    // if we are adding device, we only need present devices
    // otherwise, we need all devices
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    long lResult;
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
            continue;
        }

        TCHAR buffer[_MAX_PATH] = {0};
        TCHAR key[MAX_PATH] = {0};
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_SERVICE,
                                              NULL, (PBYTE)buffer, sizeof(buffer), &nSize)) {
            LOGE("get SPDRP_SERVICE failed");
            continue;
        }
        if (wcsnicmp((const wchar_t *)buffer, service, nSize/sizeof(wchar_t))) {
            LOGE("SPDRP_SERVICE return is not '%S'.", service);
            continue;
        }

        if (!SetupDiGetDeviceInstanceId(hDevInfo, &devdata, buffer, sizeof(buffer), &nSize)) {
            LOGE("SetupDiGetDeviceInstanceId(): %S", _com_error(GetLastError()).ErrorMessage());
            continue;
        }
        _snwprintf_s(key, MAX_PATH,L"SYSTEM\\CurrentControlSet\\Enum\\%s", buffer);

        HKEY regHandle = NULL;
        lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,key,0, KEY_READ, &regHandle);
        if (lResult != ERROR_SUCCESS) {
            LOGE("RegOpenKeyExW error code:%d",lResult);
            continue ;
        }
        nSize = MAX_PATH;
        lResult = RegQueryValueExW(regHandle, L"ParentIdPrefix", NULL, NULL,
                                   (LPBYTE)&buffer, &nSize);

        if (lResult != ERROR_SUCCESS) {
            LOGE("RegOpenKeyExW error code:%d",lResult);
            continue ;
        }

        //lResult = RegQueryValueExW(regHandle, L"PortName", NULL, NULL,
        //                           (LPBYTE)&szValue, &nSize);


        LOGW("DEV PATH %S, parentID %S", pDetData->DevicePath, buffer);

        RegCloseKey(regHandle);
        FREE_IF(pDetData);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return TRUE;
}
