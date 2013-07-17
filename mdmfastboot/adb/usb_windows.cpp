/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include "stdafx.h"
#include "sysdeps.h"
#include <windows.h>
#include <winerror.h>
#include <errno.h>
#include <usb100.h>
#include <adb_api.h>
#include <stdio.h>

#define   TRACE_TAG  TRACE_USB
#include "adb.h"
#include "../adbhost.h"
//#include "fastboot.h"

#define MAX_ADB_DEVICE  256
 adbhost* device[MAX_ADB_DEVICE]={0,};
static int device_count = 0;

/** Structure usb_handle describes our connection to the usb device via
  AdbWinApi.dll. This structure is returned from usb_open() routine and
  is expected in each subsequent call that is accessing the device.
*/
struct usb_handle {
  /// Previous entry in the list of opened usb handles
  usb_handle *prev;

  /// Next entry in the list of opened usb handles
  usb_handle *next;

  /// Handle to USB interface
  ADBAPIHANDLE  adb_interface;

  /// Handle to USB read pipe (endpoint)
  ADBAPIHANDLE  adb_read_pipe;

  /// Handle to USB write pipe (endpoint)
  ADBAPIHANDLE  adb_write_pipe;

  /// Interface name
  wchar_t*         interface_name;

  int interface_protocol;

  /// Mask for determining when to use zero length packets
  unsigned zero_mask;
};

/// Class ID assigned to the device by androidusb.sys
static const GUID usb_class_id = ANDROID_USB_CLASS_ID;
static volatile bool exist = false;
/// List of opened usb handles
static usb_handle handle_list = {
  //.prev = &handle_list,
  //.next = &handle_list,
  &handle_list,
  &handle_list,
};

/// Locker for the list of opened usb handles
ADB_MUTEX_DEFINE( usb_lock );

/// Checks if there is opened usb handle in handle_list for this device.
int known_device(const wchar_t* dev_name);

/// Checks if there is opened usb handle in handle_list for this device.
/// usb_lock mutex must be held before calling this routine.
int known_device_locked(const wchar_t* dev_name);

/// Registers opened usb handle (adds it to handle_list).
int register_new_device(usb_handle* handle);

/// Checks if interface (device) matches certain criteria
int recognized_device(usb_handle* handle);



/// Entry point for thread that polls (every second) for new usb interfaces.
/// This routine calls find_devices in infinite loop.
unsigned int device_poll_thread(void* unused);

/// Initializes this module
void usb_init();

/// Cleans up this module
void usb_cleanup();

/// Opens usb interface (device) by interface (device) name.
usb_handle* do_usb_open(const wchar_t* interface_name);

/// Writes data to the opened usb handle
int usb_write(usb_handle* handle, const void* data, int len);

/// Reads data using the opened usb handle
int usb_read(usb_handle *handle, void* data, int len);

/// Cleans up opened usb handle
void usb_cleanup_handle(usb_handle* handle);

/// Cleans up (but don't close) opened usb handle
void usb_kick(usb_handle* handle);

/// Closes opened usb handle
int usb_close(usb_handle* handle);

/// Gets interface (device) name for an opened usb handle
const char *usb_name(usb_handle* handle);

int known_device_locked(const wchar_t* dev_name) {
  usb_handle* usb;

  if (NULL != dev_name) {
    // Iterate through the list looking for the name match.
    for(usb = handle_list.next; usb != &handle_list; usb = usb->next) {
      // In Windows names are not case sensetive!
      if((NULL != usb->interface_name) &&
         (0 == _wcsicmp(usb->interface_name, dev_name))) {
        return 1;
      }
    }
  }

  return 0;
}

int known_device(const wchar_t* dev_name) {
  int ret = 0;

  if (NULL != dev_name) {
    adb_mutex_lock(&usb_lock);
    ret = known_device_locked(dev_name);
    adb_mutex_unlock(&usb_lock);
  }

  return ret;
}

int register_new_device(usb_handle* handle) {
  if (NULL == handle)
    return 0;

  adb_mutex_lock(&usb_lock);

  // Check if device is already in the list
  if (known_device_locked(handle->interface_name)) {
    adb_mutex_unlock(&usb_lock);
    return 0;
  }

  // Not in the list. Add this handle to the list.
  handle->next = &handle_list;
  handle->prev = handle_list.prev;
  handle->prev->next = handle;
  handle->next->prev = handle;

  adb_mutex_unlock(&usb_lock);

  return 1;
}

unsigned int device_poll_thread(void* unused) {
  D("Created device thread\n");

  while(!exist) {
    find_devices();
    adb_sleep_ms(1000);
  }

  return 0;
}

void usb_init() {
  adb_thread_t tid;

  if(adb_thread_create(&tid, device_poll_thread, NULL)) {
    fatal_errno("cannot create input thread");
  }
}

void usb_cleanup() {
	exist = true;
}
/// Opens usb interface (device) by interface (device) name.
usb_handle* do_usb_open(const wchar_t* interface_name) {
  // Allocate our handle
  usb_handle* ret = (usb_handle*)malloc(sizeof(usb_handle));
  if (NULL == ret)
    return NULL;

  memset(ret, 0, sizeof(usb_handle));

  // Set linkers back to the handle
  ret->next = ret;
  ret->prev = ret;

  // Create interface.
  ret->adb_interface = AdbCreateInterfaceByName(interface_name);

  if (NULL == ret->adb_interface) {
    free(ret);
    errno = GetLastError();
    return NULL;
  }

  // Open read pipe (endpoint)
  ret->adb_read_pipe =
    AdbOpenDefaultBulkReadEndpoint(ret->adb_interface,
                                   AdbOpenAccessTypeReadWrite,
                                   AdbOpenSharingModeReadWrite);
  if (NULL != ret->adb_read_pipe) {
    // Open write pipe (endpoint)
    ret->adb_write_pipe =
      AdbOpenDefaultBulkWriteEndpoint(ret->adb_interface,
                                      AdbOpenAccessTypeReadWrite,
                                      AdbOpenSharingModeReadWrite);
    if (NULL != ret->adb_write_pipe) {
      // Save interface name
      unsigned long name_len = 0;

      // First get expected name length
      AdbGetInterfaceName(ret->adb_interface,
                          NULL,
                          &name_len,
                          true);
      if (0 != name_len) {
        ret->interface_name = (wchar_t*)malloc(sizeof (wchar_t) * name_len);

        if (NULL != ret->interface_name) {
          // Now save the name
          if (AdbGetInterfaceName(ret->adb_interface,
                                  ret->interface_name,
                                  &name_len,
                                  true)) {
            // We're done at this point
            return ret;
          }
        } else {
          SetLastError(ERROR_OUTOFMEMORY);
        }
      }
    }
  }

  // Something went wrong.
  errno = GetLastError();
  usb_cleanup_handle(ret);
  free(ret);
  SetLastError(errno);

  return NULL;
}

#if 1
int usb_write(usb_handle* handle, const void* _data, int len) {
    unsigned long time_out = 500 + len * 8;
    unsigned long written = 0;
    unsigned count = 0;
    int ret;
    char *data = (char *)_data;

    D("usb_write %d\n", len);
    if (NULL != handle) {
        // Perform write
        while(len > 0) {
            int xfer = (len > 4096) ? 4096 : len;
            ret = AdbWriteEndpointSync(handle->adb_write_pipe,
                                   (void*)data,
                                   (unsigned long)xfer,
                                   &written,
                                   time_out);
            errno = GetLastError();
            D("AdbWriteEndpointSync returned %d, errno: %d\n", ret, errno);
            if (ret == 0) {
                // assume ERROR_INVALID_HANDLE indicates we are disconnected
                if (errno == ERROR_INVALID_HANDLE)
                usb_kick(handle);
                return -1;
            }

            count += written;
            len -= written;
            data += written;

            if (len == 0) {
				// Make sure that we've written what we were asked to write
		      //D("usb_write got: %ld, expected: %d\n", written, len);
		      //if (written == (unsigned long)len) {
		        if(handle->zero_mask && (count & handle->zero_mask) == 0) {
		          // Send a zero length packet
		          AdbWriteEndpointSync(handle->adb_write_pipe,
		                               (void*)data,
		                               0,
		                               &written,
		                               time_out);
		        }
                return count;
        	}
        }
    } else {
        D("usb_write NULL handle\n");
        SetLastError(ERROR_INVALID_HANDLE);
    }

    D("usb_write failed: %d\n", errno);

    return -1;
}
#else
int usb_write(usb_handle* handle, const void* data, int len) {
  unsigned long time_out = 500 + len * 8;
  unsigned long written = 0;
  int ret;

  D("usb_write %d\n", len);
  if (NULL != handle) {
    // Perform write
    ret = AdbWriteEndpointSync(handle->adb_write_pipe,
                               (void*)data,
                               (unsigned long)len,
                               &written,
                               time_out);
    errno = GetLastError();

    if (ret) {
      // Make sure that we've written what we were asked to write
      D("usb_write got: %ld, expected: %d\n", written, len);
      if (written == (unsigned long)len) {
        if(handle->zero_mask && (len & handle->zero_mask) == 0) {
          // Send a zero length packet
          AdbWriteEndpointSync(handle->adb_write_pipe,
                               (void*)data,
                               0,
                               &written,
                               time_out);
        }
        return 0;
      }
    } else {
      // assume ERROR_INVALID_HANDLE indicates we are disconnected
      if (errno == ERROR_INVALID_HANDLE)
        usb_kick(handle);
    }
  } else {
    D("usb_write NULL handle\n");
    SetLastError(ERROR_INVALID_HANDLE);
  }

  D("usb_write failed: %d\n", errno);

  return -1;
}
#endif

int usb_read(usb_handle *handle, void* _data, int len) {
  unsigned long time_out = 500 + len * 8;
  unsigned long read = 0;
  char *data = (char *)_data;
  int ret;

  D("usb_read %d\n", len);
  if (NULL != handle) {
    while (len > 0) {
      int xfer = (len > 4096) ? 4096 : len;

      ret = AdbReadEndpointSync(handle->adb_read_pipe,
                                  (void*)data,
                                  (unsigned long)xfer,
                                  &read,
                                  time_out);
      errno = GetLastError();
      D("usb_read got: %ld, expected: %d, errno: %d, ret: %d\n",
	  	read, xfer, errno, ret);
      if (ret) {
        data += read;
        len -= read;

        if (len == 0)
          return 0;
      } else if (errno != ERROR_SEM_TIMEOUT) {
        // assume ERROR_INVALID_HANDLE indicates we are disconnected
        if (errno == ERROR_INVALID_HANDLE)
          usb_kick(handle);
        break;
      }
    }
  } else {
    D("usb_read NULL handle\n");
    SetLastError(ERROR_INVALID_HANDLE);
  }

  D("usb_read failed: %d\n", errno);

  return -1;
}

void usb_cleanup_handle(usb_handle* handle) {
  if (NULL != handle) {
    if (NULL != handle->interface_name)
      free(handle->interface_name);
    if (NULL != handle->adb_write_pipe)
      AdbCloseHandle(handle->adb_write_pipe);
    if (NULL != handle->adb_read_pipe)
      AdbCloseHandle(handle->adb_read_pipe);
    if (NULL != handle->adb_interface)
      AdbCloseHandle(handle->adb_interface);

    handle->interface_name = NULL;
    handle->adb_write_pipe = NULL;
    handle->adb_read_pipe = NULL;
    handle->adb_interface = NULL;
  }
}

void usb_kick(usb_handle* handle) {
  if (NULL != handle) {
    adb_mutex_lock(&usb_lock);

    usb_cleanup_handle(handle);

    adb_mutex_unlock(&usb_lock);
  } else {
    SetLastError(ERROR_INVALID_HANDLE);
    errno = ERROR_INVALID_HANDLE;
  }
}

int usb_close(usb_handle* handle) {
  D("usb_close\n");

  if (NULL != handle) {
    // Remove handle from the list
    adb_mutex_lock(&usb_lock);

    if ((handle->next != handle) && (handle->prev != handle)) {
      handle->next->prev = handle->prev;
      handle->prev->next = handle->next;
      handle->prev = handle;
      handle->next = handle;
    }

    adb_mutex_unlock(&usb_lock);

    // Cleanup handle
    usb_cleanup_handle(handle);
    free(handle);
  }

  return 0;
}

const char *usb_name(usb_handle* handle) {
  if (NULL == handle) {
    SetLastError(ERROR_INVALID_HANDLE);
    errno = ERROR_INVALID_HANDLE;
    return NULL;
  }

  return (const char*)handle->interface_name;
}

int recognized_device(usb_handle* handle) {
  USB_DEVICE_DESCRIPTOR device_desc;
  USB_INTERFACE_DESCRIPTOR interf_desc;

  if (NULL == handle)
    return 0;

  // Check vendor and product id first

  if (!AdbGetUsbDeviceDescriptor(handle->adb_interface,
                                 &device_desc)) {
    return 0;
  }

  // Then check interface properties

  if (!AdbGetUsbInterfaceDescriptor(handle->adb_interface,
                                    &interf_desc)) {
    return 0;
  }

  // Must have two endpoints
  if (2 != interf_desc.bNumEndpoints) {
    return 0;
  }

  // expand to add interface protocol
  handle->interface_protocol = interf_desc.bInterfaceProtocol;

  if (is_adb_interface(device_desc.idVendor, device_desc.idProduct,
      interf_desc.bInterfaceClass, interf_desc.bInterfaceSubClass, interf_desc.bInterfaceProtocol)) {

    if(interf_desc.bInterfaceProtocol == ADB_PROTOCOL) {
      AdbEndpointInformation endpoint_info;
      // assuming zero is a valid bulk endpoint ID
      if (AdbGetEndpointInformation(handle->adb_interface, 0, &endpoint_info)) {
        handle->zero_mask = endpoint_info.max_packet_size - 1;
      }
    }

    return 1;
  }

  if (is_fastboot_interface(device_desc.idVendor, device_desc.idProduct,
      interf_desc.bInterfaceClass, interf_desc.bInterfaceSubClass, interf_desc.bInterfaceProtocol)) {
  	return 1;
  }

  return 0;
}

void find_devices() {
  usb_handle* handle = NULL;
  char entry_buffer[2048];
  AdbInterfaceInfo* next_interface = (AdbInterfaceInfo*)(&entry_buffer[0]);
  unsigned long entry_buffer_size = sizeof(entry_buffer);

  // Enumerate all present and active interfaces.
  ADBAPIHANDLE enum_handle =
    AdbEnumInterfaces(usb_class_id, true, true, true);

  if (NULL == enum_handle)
    return;

  while (AdbNextInterface(enum_handle, next_interface, &entry_buffer_size)) {
    // Lets see if we already have this device in the list
    if (!known_device(next_interface->device_name)) {
      // This seems to be a new device. Open it!
        handle = do_usb_open(next_interface->device_name);
        if (NULL != handle) {
        // Lets see if this interface (device) belongs to us
        if (recognized_device(handle)) {
          char serial_number[512];
          unsigned long serial_number_len = sizeof(serial_number);
          D("adding a new device %s\n", next_interface->device_name);
          if (AdbGetSerialNumber(handle->adb_interface,
                                serial_number,
                                &serial_number_len,
                                true)) {
            // Lets make sure that we don't duplicate this device
            if (register_new_device(handle)) {
             // register_usb_transport(handle, serial_number, 1);
				//todo ::::
				if (handle->interface_protocol == ADB_PROTOCOL) {
					 ;//device[device_count] = new adbhost(handle, device_count ++);
				} else if (handle->interface_protocol == FB_PROTOCOL) {
				  D("nothing to do for fastboot now!");
				}

            } else {
              D("register_new_device failed for %s\n", next_interface->device_name);
              usb_cleanup_handle(handle);
              free(handle);
            }
          } else {
            D("cannot get serial number\n");
            usb_cleanup_handle(handle);
            free(handle);
          }
        }else {
          usb_cleanup_handle(handle);
          free(handle);
        }
      }
    }

    entry_buffer_size = sizeof(entry_buffer);
  }

  AdbCloseHandle(enum_handle);
}

UINT run(LPVOID handle) {
	adbhost adb((usb_handle*)handle , 0xbeef);
	adb.process();
	return 0;
}

UINT do_nothing() {
	usb_handle* usb;
    // Iterate through the list looking for the name match.
    for(usb = handle_list.next; usb != &handle_list; usb = usb->next) {
		AfxBeginThread(run, (void*)usb);
    }

	return 0;
}