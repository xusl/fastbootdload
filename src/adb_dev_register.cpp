#include "stdafx.h"

#include "adb_dev_register.h"
#include "comdef.h"
#include "log.h"

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")

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
                    //LOGD("RegQueryValueExW error %S", buffer);
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

