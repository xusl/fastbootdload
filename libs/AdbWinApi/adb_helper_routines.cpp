/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** \file
  This file consists of implementation of helper routines used
  in the API.
*/

#include "stdafx.h"
#include "adb_api.h"
#include "adb_api_legacy.h"
#include "adb_helper_routines.h"
#include "adb_interface_enum.h"

bool GetSDKComplientParam(AdbOpenAccessType access_type,
                          AdbOpenSharingMode sharing_mode,
                          ULONG* desired_access,
                          ULONG* desired_sharing) {
  if (NULL != desired_access) {
    switch (access_type) {
      case AdbOpenAccessTypeReadWrite:
        *desired_access = GENERIC_READ | GENERIC_WRITE;
        break;

      case AdbOpenAccessTypeRead:
        *desired_access = GENERIC_READ;
        break;

      case AdbOpenAccessTypeWrite:
        *desired_access = GENERIC_WRITE;
        break;

      case AdbOpenAccessTypeQueryInfo:
        *desired_access = FILE_READ_ATTRIBUTES | FILE_READ_EA;
        break;

      default:
        SetLastError(ERROR_INVALID_ACCESS);
        return false;
    }
  }

  if (NULL != desired_sharing) {
    switch (sharing_mode) {
      case AdbOpenSharingModeReadWrite:
        *desired_sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
        break;

      case AdbOpenSharingModeRead:
        *desired_sharing = FILE_SHARE_READ;
        break;

      case AdbOpenSharingModeWrite:
        *desired_sharing = FILE_SHARE_WRITE;
        break;

      case AdbOpenSharingModeExclusive:
        *desired_sharing = 0;
        break;

      default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }
  }

  return true;
}

bool EnumerateDeviceInterfaces(HDEVINFO hardware_dev_info,
                               GUID class_id,
                               bool exclude_removed,
                               bool active_only,
                               AdbEnumInterfaceArray* interfaces) {
  AdbEnumInterfaceArray tmp;
  bool ret = false;

  // Enumerate interfaces on this device
  for (ULONG index = 0; ; index++) {
    SP_DEVICE_INTERFACE_DATA interface_data;
    interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    // SetupDiEnumDeviceInterfaces() returns information about device
    // interfaces exposed by one or more devices defined by our interface
    // class. Each call returns information about one interface. The routine
    // can be called repeatedly to get information about several interfaces
    // exposed by one or more devices.
    if (SetupDiEnumDeviceInterfaces(hardware_dev_info,
                                    0,
                                    &class_id,
                                    index,
                                    &interface_data)) {
      // Satisfy "exclude removed" and "active only" filters.
      if ((!exclude_removed || (0 == (interface_data.Flags & SPINT_REMOVED))) &&
          (!active_only || (interface_data.Flags & SPINT_ACTIVE))) {
        std::wstring dev_name;

        if (GetUsbDeviceName(hardware_dev_info, &interface_data, &dev_name)) {
          try {
            // Add new entry to the array
            tmp.push_back(AdbInstanceEnumEntry(dev_name.c_str(),
                                               interface_data.InterfaceClassGuid,
                                               interface_data.Flags));
          } catch (... ) {
            SetLastError(ERROR_OUTOFMEMORY);
            break;
          }
        } else {
          // Something went wrong in getting device name
          break;
        }
      }
    } else {
      if (ERROR_NO_MORE_ITEMS == GetLastError()) {
        // There are no more items in the list. Enum is completed.
        ret = true;
        break;
      } else {
        // Something went wrong in SDK enum
        break;
      }
    }
  }

  // On success, swap temp array with the returning one
  if (ret)
    interfaces->swap(tmp);

  return ret;
}

bool EnumerateDeviceInterfaces(GUID class_id,
                               ULONG flags,
                               bool exclude_removed,
                               bool active_only,
                               AdbEnumInterfaceArray* interfaces) {
  // Open a handle to the plug and play dev node.
  // SetupDiGetClassDevs() returns a device information set that
  // contains info on all installed devices of a specified class.
  HDEVINFO hardware_dev_info =
    SetupDiGetClassDevs(&class_id, NULL, NULL, flags);

  bool ret = false;

  if (INVALID_HANDLE_VALUE != hardware_dev_info) {
    // Do the enum
    ret = EnumerateDeviceInterfaces(hardware_dev_info,
                                    class_id,
                                    exclude_removed,
                                    active_only,
                                    interfaces);

    // Preserve last error accross hardware_dev_info destruction
    ULONG error_to_report = ret ? NO_ERROR : GetLastError();

    SetupDiDestroyDeviceInfoList(hardware_dev_info);

    if (NO_ERROR != error_to_report)
      SetLastError(error_to_report);
  }

  return ret;
}

bool GetUsbDeviceDetails(
    HDEVINFO hardware_dev_info,
    PSP_DEVICE_INTERFACE_DATA dev_info_data,
    PSP_DEVICE_INTERFACE_DETAIL_DATA* dev_info_detail_data) {
  ULONG required_len = 0;

  // First query for the structure size. At this point we expect this call
  // to fail with ERROR_INSUFFICIENT_BUFFER error code.
  if (SetupDiGetDeviceInterfaceDetail(hardware_dev_info,
                                      dev_info_data,
                                      NULL,
                                      0,
                                      &required_len,
                                      NULL)) {
    return false;
  }

  if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
    return false;

  // Allocate buffer for the structure
  PSP_DEVICE_INTERFACE_DETAIL_DATA buffer =
    reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(malloc(required_len));

  if (NULL == buffer) {
    SetLastError(ERROR_OUTOFMEMORY);
    return false;
  }

  buffer->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

  // Retrieve the information from Plug and Play.
  if (SetupDiGetDeviceInterfaceDetail(hardware_dev_info,
                                      dev_info_data,
                                      buffer,
                                      required_len,
                                      &required_len,
                                      NULL)) {
    *dev_info_detail_data = buffer;
    return true;
  } else {
    // Free the buffer if this call failed
    free(buffer);

    return false;
  }
}

bool GetUsbDeviceName(HDEVINFO hardware_dev_info,
                      PSP_DEVICE_INTERFACE_DATA dev_info_data,
                      std::wstring* name) {
  PSP_DEVICE_INTERFACE_DETAIL_DATA func_class_dev_data = NULL;
  if (!GetUsbDeviceDetails(hardware_dev_info,
                           dev_info_data,
                           &func_class_dev_data)) {
    return false;
  }

  try {
    *name = func_class_dev_data->DevicePath;
  } catch (...) {
    SetLastError(ERROR_OUTOFMEMORY);
  }

  free(func_class_dev_data);

  return !name->empty();
}

bool IsLegacyInterface(const wchar_t* interface_name) {
  // Open USB device for this intefface
  HANDLE usb_device_handle = CreateFile(interface_name,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        OPEN_EXISTING,
                                        0,
                                        NULL);
  if (INVALID_HANDLE_VALUE == usb_device_handle)
    return NULL;

  // Try to issue ADB_IOCTL_GET_USB_DEVICE_DESCRIPTOR IOCTL that is supported
  // by the legacy driver, but is not implemented in the WinUsb driver.
  DWORD ret_bytes = 0;
  USB_DEVICE_DESCRIPTOR descriptor;
  BOOL ret = DeviceIoControl(usb_device_handle,
                             ADB_IOCTL_GET_USB_DEVICE_DESCRIPTOR,
                             NULL, 0,
                             &descriptor,
                             sizeof(descriptor),
                             &ret_bytes,
                             NULL);
  ::CloseHandle(usb_device_handle);

  // If IOCTL succeeded we've got legacy driver underneath.
  return ret ? true : false;
}


PWCHAR GetExternalHubName(HANDLE Hub,ULONG ConnectionIndex){
  BOOL                        success;
  ULONG                       nBytes;
  USB_NODE_CONNECTION_NAME    extHubName;
  PUSB_NODE_CONNECTION_NAME   extHubNameW;
  PWCHAR    hubName = NULL;

  extHubName.ConnectionIndex = ConnectionIndex;
  success = DeviceIoControl(Hub,
                            IOCTL_USB_GET_NODE_CONNECTION_NAME,
                            &extHubName,
                            sizeof(extHubName),
                            &extHubName,
                            sizeof(extHubName),
                            &nBytes,
                            NULL);
  if (!success) {
    fprintf(stderr,"IOCTL_USB_GET_NODE_CONNECTION_NAME length failed!");
    return hubName;
  }

  nBytes = extHubName.ActualLength;
  extHubNameW = (PUSB_NODE_CONNECTION_NAME) malloc(nBytes);
  if (extHubNameW == NULL) {
    fprintf(stderr,"malloc failed!");
    return hubName;
  }

  extHubNameW->ConnectionIndex = ConnectionIndex;
  success = DeviceIoControl(Hub,
                            IOCTL_USB_GET_NODE_CONNECTION_NAME,
                            extHubNameW,
                            nBytes,
                            extHubNameW,
                            nBytes,
                            &nBytes,
                            NULL);
  if (!success) {
    fprintf(stderr,"IOCTL_USB_GET_NODE_CONNECTION_NAME failed!");
  } else {
    hubName = wcsdup(extHubNameW->NodeName) ;
  }
  free(extHubNameW);

  return hubName;
}
BOOL EnumerateHub(PWCHAR hubName,  GUID class_id);
void EnumerateHubPorts(HANDLE hHubDevice,ULONG NumPorts, GUID class_id) {
  ULONG       index;
  BOOL        success;
  PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;

  // 遍历该HUB的所有口. 口索引是基1,不是基0.
  for (index=1; index <= NumPorts; index++) {
    ULONG nBytes;
    // 分配足够的空间以容纳30个管道的连接信息.
    // 端口数为0-15.端口0 是标准的控制端口不在配置描述符中.这样IN 端口和OUT端口都是1到15,
    // 所以每个设备配置最大有30个端口.
    nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
    connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION) malloc(nBytes);
    if (connectionInfo == NULL) {
      fprintf(stderr,"malloc failed!");
      break;
    }

    connectionInfo->ConnectionIndex = index;
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                              connectionInfo,
                              nBytes,
                              connectionInfo,
                              nBytes,
                              &nBytes,
                              NULL);
    if (!success) {
      free(connectionInfo);
      continue;
    }
#if 0
    // 更新连接设备总数
    if (connectionInfo->ConnectionStatus == DeviceConnected) {
      m_iTotalDevicesConnected++;
    }
    if (connectionInfo->DeviceIsHub) {
      m_iTotalHubs++;
    }

    // 如果有设备连接到该口,从该设备取配置描述符.
    // (connectionInfo->ConnectionStatus == DeviceConnected)
#endif
    // 如果连接到端口的设备是外部HUB,取外部HUB的名称并递归枚举.
    if (connectionInfo->DeviceIsHub){
      PWCHAR extHubName = GetExternalHubName(hHubDevice,index);
      if (extHubName != NULL) {
        EnumerateHub(extHubName,class_id);
        free(extHubName);
        //continue;
      }
    }

    free(connectionInfo);
  }
}

BOOL EnumerateHub(PWCHAR hubName,  GUID class_id)
{
  PUSB_NODE_INFORMATION hubInfo = NULL;
  HANDLE                  hHubDevice = INVALID_HANDLE_VALUE;
  PWCHAR                   deviceName = NULL;
  BOOL            success;
  ULONG           nBytes;

  hubInfo = (PUSB_NODE_INFORMATION) malloc(sizeof(USB_NODE_INFORMATION));
  if (hubInfo == NULL)  {
    fprintf(stderr,"No memory!");
    return FALSE;
  }

  deviceName = (PWCHAR)malloc((/*wcslen*/lstrlen(hubName) +1 ) * sizeof(WCHAR)
                        + sizeof(L"\\\\.\\"));
  if (deviceName == NULL)  {
    fprintf(stderr,"No memory!");
    free(hubInfo);
    return FALSE;
  }

  wsprintf(deviceName, L"\\\\.\\%s",hubName);
  hHubDevice = CreateFile(deviceName,
                          GENERIC_WRITE,
                          FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL);

  if (hHubDevice == INVALID_HANDLE_VALUE)  {
    fprintf(stderr,"Open device %s failed!", deviceName);
    free(deviceName);
    free(hubInfo);
    return FALSE;
  }

  success = DeviceIoControl(hHubDevice,
                            IOCTL_USB_GET_NODE_INFORMATION,
                            hubInfo,//?
                            sizeof(USB_NODE_INFORMATION),//?
                            hubInfo,
                            sizeof(USB_NODE_INFORMATION),
                            &nBytes,
                            NULL);
  if (success){
    EnumerateHubPorts(hHubDevice,
                      hubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts,
                      class_id);
  } else {
    fprintf(stderr,"Oop38!");
  }

  free(deviceName);
  free(hubInfo);
  CloseHandle(hHubDevice);
  return success;
}

/*
 * HCNumMax : 127?
 */
BOOL TransverseDevice(UINT HCNumMax, GUID class_id) {
  WCHAR        HCName[16];
  UINT         HCNum;
  HANDLE      hHCDev;
  BOOL                success;
  DWORD               nBytes;
  USB_ROOT_HUB_NAME  hubNameLen;
  PUSB_ROOT_HUB_NAME pRootHubName;

  // enumerate host controller
  for (HCNum = 0; HCNum <HCNumMax; HCNum++) {
    wnsprintf(HCName, sizeof(HCName), L"\\\\.\\HCD%d", HCNum);
    hHCDev = CreateFile(HCName,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if (hHCDev == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "open host controller failed!");
      continue;
    }

    success = DeviceIoControl(hHCDev,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              0,
                              0,
                              &hubNameLen,
                              sizeof(hubNameLen),
                              &nBytes,
                              NULL);
    if (!success) {
      fprintf(stderr, "IOCTL_USB_GET_ROOT_HUB_NAME failed!");
      CloseHandle(hHCDev);
      continue;
    }

    pRootHubName = (PUSB_ROOT_HUB_NAME)malloc(hubNameLen.ActualLength);
    if (pRootHubName == NULL) {
      fprintf(stderr, "malloc failed!");
      CloseHandle(hHCDev);
      continue;
    }
    pRootHubName->ActualLength = hubNameLen.ActualLength;
    success = DeviceIoControl(hHCDev,
                              IOCTL_USB_GET_ROOT_HUB_NAME,NULL,
                              0,
                              pRootHubName,
                              pRootHubName->ActualLength,
                              &nBytes,
                              NULL);

    if (success) {
      EnumerateHub(pRootHubName->RootHubName,class_id);
    } else {
      fprintf(stderr, "IOCTL_USB_GET_ROOT_HUB_NAME failed!");
    }

    CloseHandle(hHCDev);
    free(pRootHubName);
  }
  return TRUE;
}