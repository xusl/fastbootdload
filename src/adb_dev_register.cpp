#include "stdafx.h"

#include "adb_dev_register.h"
#include "comdef.h"
#include "Psapi.h"
#include "log.h"

#pragma	 comment(lib,"setupapi.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Psapi.lib")
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
BOOL GetDeviceByGUID(std::vector<CString>& devicePaths, const GUID *ClassGuid) {
  HDEVINFO hDevInfo = SetupDiGetClassDevs(ClassGuid,
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

  for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, ClassGuid, i, &ifcData); ++ i) {
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
    }
    delete pDetData;
    pDetData = NULL;
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
  return TRUE;
}

DWORD FindProcess(wchar_t *strProcessName, CString &AppPath)
{
	DWORD aProcesses[1024], cbNeeded, cbMNeeded;
	HMODULE hMods[1024];
	HANDLE hProcess;
	wchar_t szProcessName[MAX_PATH];

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )  return 0;
	for(int i=0; i< (int) (cbNeeded / sizeof(DWORD)); i++)
	{
		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
		EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbMNeeded);
		GetModuleFileNameEx( hProcess, hMods[0], szProcessName,sizeof(szProcessName));

		if(wcsstr(szProcessName, strProcessName))
		{
			AppPath = szProcessName;
			return(aProcesses[i]);
		}
	}
	return 0;
}

BOOL StopAdbServer(){
	int iTemp = 0;
	DWORD adbProcID;
	CString adbPath;
	adbProcID = FindProcess(L"adb.exe", adbPath);
	if (0 != adbProcID)
	{
		//stop adb;
		//If the function succeeds, the return value is greater than 31.
		iTemp = WinExec(adbPath + " kill-server", SW_HIDE);
		if (31 < iTemp)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}
