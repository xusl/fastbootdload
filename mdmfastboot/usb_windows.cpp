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
#include "fastbootflash.h"
#include "usb_vendors.h"



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
  usb_dev_t status;
  bool work;
  long usb_sn;

  /// Mask for determining when to use zero length packets
  unsigned zero_mask;
};

typedef struct adb_device_t {
   adb_device_t *prev;
   adb_device_t *next;
   long adb_sn;  // adb device serial number that allocate by Windows.
   long cd_sn;    // serial number of composite device which adb device in.
}adb_device;

typedef struct dev_switch_t {
  dev_switch_t *prev;
  dev_switch_t *next;
    long        usb_sn;
    usb_dev_t   status;
    long        time;
} dev_switch_t;

/// Class ID assigned to the device by androidusb.sys
static const GUID usb_class_id = ANDROID_USB_CLASS_ID;
//static volatile bool exist = false;
/// List of opened usb handles
static usb_handle handle_list = {
  //.prev = &handle_list,
  //.next = &handle_list,
  &handle_list,
  &handle_list,
};

static dev_switch_t switch_list = {
  &switch_list,
  &switch_list,
};

static adb_device_t adbdev_list= {
    &adbdev_list,
    &adbdev_list,
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

/// Gets interface (device) name for an opened usb handle
const wchar_t *usb_name(usb_handle* handle);

void adb_usb_init( void )
{
    ADB_MUTEX(usb_lock);
	usb_vendors_init();
}


adb_device_t* is_adb_device_exist(long cd_sn) {
  adb_device_t* adb;
  for(adb = adbdev_list.next; adb != &adbdev_list; adb = adb->next) {
    if (adb->cd_sn == cd_sn) {
      return adb;
    }
  }

  return NULL;
}

void dump_adb_device(void) {
  adb_device_t* adb;
  INFO("Begin dump host installed adb device driver\n================");
  for(adb = adbdev_list.next; adb != &adbdev_list; adb = adb->next) {
        INFO("0x%x (composite)<==> 0x%x(adb)", adb->cd_sn, adb->adb_sn);
  }
  INFO("End dump host installed adb device driver\n=================");
}

long get_adb_composite_device_sn(long adb_sn) {
  adb_device_t* adb;
  for(adb = adbdev_list.next; adb != &adbdev_list; adb = adb->next) {
    if (adb->adb_sn == adb_sn) {
        INFO("adb_sn 0x%x convert composite sn 0x%x", adb_sn, adb->cd_sn);
      return adb->cd_sn;
    }
  }
  return adb_sn;
}

//5&10cd67f3&0&4 5&10cd67f3&0
long extract_serial_number(wchar_t * sn, wchar_t **ppstart =NULL, wchar_t **ppend= NULL) {
  size_t len;
  wchar_t *pstart, *pend;
  if (sn == NULL) {
    ERROR("Bad Parameter");
    return -1;
  }

  len = wcslen (sn);

  pstart = (wchar_t*)wcschr(sn, L'&');
  if (pstart == NULL || pstart - sn >= len) {
    ERROR("Can not find first '&'.");
    return -1;
  }

  pstart++;
  pend = wcschr(pstart , L'&');
  if (pend == NULL || pend - sn >= len) {
    ERROR("Can not find first '&'.");
    return -1;
  }

  if (ppstart != NULL)
    *ppstart = pstart;
  if (ppend != NULL)
    *ppend = pend;
  return wcstol(pstart , &pend, 16);
}

// Usually, adb is a composite device interface, and fastboot is a single interface device.
// So the adb interface 's host number is differ from fastboot. But it's composite's host number
// is equal fastboot's host number.
// adb composite device USB\VID_1BBB&PID_0192\5&10CD67F3&0&4 have key ParentIdPrefix
// with 6&29f04a5a&0. And adb interface's host number prefix with 6&29f04a5a&0. Then,
// fastboot host number contain 10CD67F3.
int add_adb_device(wchar_t *ccgp, wchar_t *parentId) {
  long cd_sn, adb_sn;

  if (ccgp == NULL || parentId == NULL) {
    ERROR("null parameter");
    return -1;
  }

  size_t len = wcslen (ccgp);
  if(wcslen (ccgp) < 22)
    return -1;
  cd_sn = extract_serial_number(ccgp + 22);
  adb_sn = extract_serial_number(parentId);

  if (cd_sn <= 0 || adb_sn <= 0 ) {
    ERROR("Invalid cd_sn %l or adb_sn %l", cd_sn, adb_sn);
    return -1;
  }

  if (is_adb_device_exist(cd_sn) == NULL) {
    adb_device_t* adb = (adb_device_t*)malloc(sizeof(adb_device_t));

    if (adb == NULL) {
      ERROR("Out of Memory");
      return -1;
    }
    adb->adb_sn = adb_sn;
    adb->cd_sn = cd_sn;

    adb->next = &adbdev_list;
    adb->prev = adbdev_list.prev;
    adb->prev->next = adb;
    adb->next->prev = adb;
  } else {
  DEBUG("ADB_DEVICE has already exist.");
    }
  return 0;
}


// \\?\usb#vid_18d1&pid_d00d#5&10cd67f3&0&4#{f72fe0d4-cbcb-407d-8814-9ed673d0dd6b}
// convert to
// 10cd67f3
long usb_host_sn(const wchar_t* dev_name, wchar_t** psn) {
  wchar_t * snb, *sne, * sn;

  size_t len = wcslen (dev_name); //lstrlen, lstrcmp()
  if(wcsnicmp(L"\\\\?\\usb#",dev_name,8) || len < 26 + 40) {
    ERROR("Not invalid dev name.");
    return 0;
  }

  snb = (wchar_t*)wcschr(dev_name + 26, L'&');
  if (snb == NULL || snb - dev_name >= len)
    return 0;

  snb++;
  sne = wcschr(snb , L'&');
  if (sne == NULL || sne - dev_name >= len)
    return 0;

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


int usb_switch_device(usb_handle* handle) {
  dev_switch_t* dev = (dev_switch_t*)malloc(sizeof(dev_switch_t));

  if (dev == NULL) {
	  ERROR("Out of Memory");
	  return -1;
  }
  dev->usb_sn = handle->usb_sn;
  dev->status = handle->status;
  dev->time = now();

    // Not in the list. Add this handle to the list.
  dev->next = &switch_list;
  dev->prev = switch_list.prev;
  dev->prev->next = dev;
  dev->next->prev = dev;

  return 0;
}

/*
* return 0: success, 1 failed.
*/
int known_switch_device(usb_handle* handle) {
  dev_switch_t* dev;

  // Iterate through the list looking for the name match.
  for(dev = switch_list.next; dev != &switch_list; dev = dev->next) {
    if (handle->usb_sn != dev->usb_sn)
      continue;

    handle->status = dev->status;

    if ((dev->next != dev) && (dev->prev != dev)) {
      //link the next node and previous node
      dev->next->prev = dev->prev;
      dev->prev->next = dev->next;
      //unlink self from the list
      dev->prev = dev;
      dev->next = dev;
    }

    free(dev);
    return 1;
  }
  return 0;
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


int register_new_device(usb_handle* handle) {
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

  if (known_switch_device(handle)) {
    if(handle->status == DEVICE_CHECK && protocol == FB_PROTOCOL) {
        handle->status = DEVICE_FLASH;
    } else if (handle->status == DEVICE_FLASH && protocol == ADB_PROTOCOL) {
        handle->status = DEVICE_CONFIGURE;
    } else {
        ERROR("Device status switch error, \n"
        "Do not know how to switch from status %d with new protocol %d"
        handle->status, protocol);
        goto register_new_device_out;
    }
  } else if (protocol == ADB_PROTOCOL) {
    handle->status = DEVICE_CHECK;
  } else if (protocol == FB_PROTOCOL) {
    ERROR("We do not permit fastboot as the first device status!");
    goto register_new_device_out;
    //handle->status = DEVICE_FLASH;
  }

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
            //DEBUG("AdbWriteEndpointSync returned %d, errno: %d\n", ret, errno);
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
      DEBUG("usb_read got: %ld, expected: %d, errno: %d, ret: %d",
	  	read, xfer, errno, ret);
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
        break;
      }
    }
  } else {
    ERROR("usb_read NULL handle");
    SetLastError(ERROR_INVALID_HANDLE);
  }

  WARN("usb_read failed: %d", errno);

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

const wchar_t *usb_name(usb_handle* handle) {
  if (NULL == handle) {
    SetLastError(ERROR_INVALID_HANDLE);
    errno = ERROR_INVALID_HANDLE;
    return NULL;
  }

  return handle->interface_name;
}

long usb_port_address(usb_handle* handle) {
  if (NULL == handle) {
    SetLastError(ERROR_INVALID_HANDLE);
    errno = ERROR_INVALID_HANDLE;
    return ~1L;
  }

  return handle->usb_sn;
}

usb_dev_t usb_status(usb_handle* handle) {
     if (NULL == handle) {
        return DEVICE_UNKNOW;
     }
    return handle->status;
}

int recognized_device(usb_handle* handle) {
  USB_DEVICE_DESCRIPTOR device_desc;
  USB_INTERFACE_DESCRIPTOR interf_desc;

  if (NULL == handle)
    return 0;

  long sn = usb_host_sn(handle->interface_name, NULL);
  if (sn != 0) {
    handle->usb_sn = get_adb_composite_device_sn(sn);
    //DEBUG("%S serial number %x", interface_name, sn);
  }  else {
    //if we do not support multiple, sn is not from host allocation.
    // todo:: add a judge?
    ERROR("%S serial number %x", handle->interface_name, sn);
    return 0;
  }

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
        if (NULL == handle) continue;
        // Lets see if this interface (device) belongs to us
        if (recognized_device(handle)) {
          char serial_number[512];
          unsigned long serial_number_len = sizeof(serial_number);
          DEBUG("adding a new device %s", next_interface->device_name);
          if (AdbGetSerialNumber(handle->adb_interface,
                                serial_number,
                                &serial_number_len,
                                true)) {
            // Lets make sure that we don't duplicate this device
            if (register_new_device(handle)) {
         //    AfxBeginThread(run, (void*)handle);
            } else {
              DEBUG("register_new_device failed for %s\n", next_interface->device_name);
              usb_cleanup_handle(handle);
              free(handle);
            }
          } else {
            DEBUG("cannot get serial number\n");
            usb_cleanup_handle(handle);
            free(handle);
          }
        }else {
          usb_cleanup_handle(handle);
          free(handle);
        }
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
    if (usb == NULL || usb == &handle_list)
        return NULL;
    return usb->next;
}

void usb_set_work(usb_handle* usb) {
    if (usb != NULL)
      usb->work = TRUE;
}

bool usb_is_work(usb_handle* usb) {
     if (usb != NULL)
        return usb->work;

     return false;
}
