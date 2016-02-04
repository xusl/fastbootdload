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
#include <windows.h>
#include <winerror.h>
#include <errno.h>
#include <usb100.h>
#include <adb_api.h>
#include <stdio.h>

#define   TRACE_TAG  TRACE_USB
#include "adb.h"
#include "adbhost.h"
#include "device.h"
#include "usb_vendors.h"
#include "adb_dev_register.h"

//static volatile bool exist = false;
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

/// Opens usb interface (device) by interface (device) name.
usb_handle* do_usb_open(const wchar_t* interface_name);

/// Writes data to the opened usb handle
//int usb_write(usb_handle* handle, const void* data, int len);

/// Reads data using the opened usb handle
//int usb_read(usb_handle *handle, void* data, int len);

/// Cleans up opened usb handle
void usb_cleanup_handle(usb_handle* handle);

/// Cleans up (but don't close) opened usb handle
void usb_kick(usb_handle* handle);

/// Closes opened usb handle
int usb_close(usb_handle* handle);


void adb_usb_init( void )
{
    ADB_MUTEX(usb_lock);
	usb_vendors_init();
}


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


int register_new_device(usb_handle* handle, BOOL flashdirect) {
  int ret = 0;
  int protocol;

  if (NULL == handle)
    return ret;

  protocol = handle->interface_protocol;

  adb_mutex_lock(&usb_lock);

  // Check if device is already in the list
  if (known_device_locked(handle->interface_name)) {
    goto register_new_device_out;
  }

  #if 0
  if (protocol == ADB_PROTOCOL) {
    handle->status = DEVICE_CHECK;
  } else if (protocol == FB_PROTOCOL) {
   if (flashdirect) {
    handle->status = DEVICE_FLASH;
    } else {
    ERROR("We do not permit fastboot as the first device status!");
    goto register_new_device_out;
    }
  }
#endif

  // Not in the list. Add this handle to the list.
  handle->next = &handle_list;
  handle->prev = handle_list.prev;
  handle->prev->next = handle;
  handle->next->prev = handle;
  handle->work = FALSE;
  ret = 1;

register_new_device_out:
  adb_mutex_unlock(&usb_lock);

  return ret;
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
                          false/*true*/);
      if (0 != name_len) {
        ret->interface_name = (wchar_t*)malloc(sizeof (wchar_t) * name_len);

        if (NULL != ret->interface_name) {
          // Now save the name, generally, ret->interface_name is equal to interface_name
          if (AdbGetInterfaceName(ret->adb_interface,
                                  ret->interface_name,
                                  &name_len,
                                   false/*true*/)) {
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

    //DEBUG("usb_write %d", len);
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
            if (ret == 0) {
                // assume ERROR_INVALID_HANDLE indicates we are disconnected
                if (errno == ERROR_INVALID_HANDLE)
                usb_kick(handle);
                WARN("AdbWriteEndpointSync returned %d, errno: %d\n", ret, errno);
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
        ERROR("usb_write NULL handle");
        SetLastError(ERROR_INVALID_HANDLE);
    }

    WARN("usb_write failed: %d", errno);

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

int usb_read(usb_handle *handle, void* _data, int len, bool fulfill) {
  unsigned long time_out = 500 + len * 8;
  unsigned long read = 0;
  char *data = (char *)_data;
  int ret;

  //DEBUG("usb_read %d", len);
  if (NULL != handle) {
    while (len > 0) {
      int xfer = (len > 4096) ? 4096 : len;

      ret = AdbReadEndpointSync(handle->adb_read_pipe,
                                  (void*)data,
                                  (unsigned long)xfer,
                                  &read,
                                  time_out);
      errno = GetLastError();
      if (ret) {
        if (!fulfill)
            return read;

        data += read;
        len -= read;

        if (len == 0)
          return 0;
      } else if (errno != ERROR_SEM_TIMEOUT) {
        // assume ERROR_INVALID_HANDLE indicates we are disconnected
        if (errno == ERROR_INVALID_HANDLE)
          usb_kick(handle);
        WARN("usb_read got: %ld, expected: %d, errno: %d, ret: %d", read, xfer, errno, ret);
        break;
      }
    }
  } else {
    ERROR("usb_read NULL handle");
    SetLastError(ERROR_INVALID_HANDLE);
  }

  WARN("usb_read failed: %d(%s)", errno, strerror(errno));

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
  DEBUG("usb_close");

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

void find_devices(BOOL flashdirect) {
    usb_handle* handle = NULL;
    char entry_buffer[2048];
    AdbInterfaceInfo* next_interface = (AdbInterfaceInfo*)(&entry_buffer[0]);
    unsigned long entry_buffer_size = sizeof(entry_buffer);
    //const GUID usb_class_id = ANDROID_USB_CLASS_ID;

    // Enumerate all present and active interfaces.
    ADBAPIHANDLE enum_handle =
        AdbEnumInterfaces(GUID_DEVINTERFACE_ADB, true, true, true);

    if (NULL == enum_handle) {
        WARN("find_devices: Enumeration initialize failed");
        return;
    }

    while (AdbNextInterface(enum_handle, next_interface, &entry_buffer_size)) {
        // Lets see if we already have this device in the list
        if (known_device(next_interface->device_name)) {
            INFO("%S is not ours adb interface.", next_interface->device_name);
            entry_buffer_size = sizeof(entry_buffer);
            continue;
        }

        // This seems to be a new device. Open it!
        handle = do_usb_open(next_interface->device_name);
        if (NULL == handle) {
            WARN("Open %S failed", next_interface->device_name);
            entry_buffer_size = sizeof(entry_buffer);
            continue;
        }
        // Lets see if this interface (device) belongs to us
        if (0 == recognized_device(handle)) {
            ERROR("DO NOT recognized_device: %S.", handle->interface_name);
            usb_cleanup_handle(handle);
            free(handle);
            entry_buffer_size = sizeof(entry_buffer);
            continue;
        }

        char serial_number[512];
        unsigned long serial_number_len = sizeof(serial_number);
        INFO("adding a new device %S", next_interface->device_name);
        if (!AdbGetSerialNumber(handle->adb_interface,
                                serial_number,
                                &serial_number_len,
                                true)) {
            WARN("cannot get serial number\n");
            usb_cleanup_handle(handle);
            free(handle);
            entry_buffer_size = sizeof(entry_buffer);
            continue;
        }

        // Lets make sure that we don't duplicate this device
        if (register_new_device(handle, flashdirect)) {
            //    AfxBeginThread(run, (void*)handle);
        } else {
            WARN("register_new_device failed for %s\n", next_interface->device_name);
            usb_cleanup_handle(handle);
            free(handle);
        }

        entry_buffer_size = sizeof(entry_buffer);
    }

    AdbCloseHandle(enum_handle);
}

usb_handle* usb_handle_enum_init(void) {
    usb_handle* usb = handle_list.next;
    if (usb == &handle_list)
        return NULL;
    return usb;
    //return handle_list.next;
}

usb_handle* usb_handle_next(usb_handle* usb) {
    if (usb == NULL || usb->next == &handle_list || usb == &handle_list)
        return NULL;
    return usb->next;
}

void usb_set_work(usb_handle* usb,  BOOL bwork) {
    if (usb != NULL)
      usb->work = bwork;
}

BOOL usb_is_work(usb_handle* usb) {
     if (usb != NULL)
        return usb->work;

     return FALSE;
}
