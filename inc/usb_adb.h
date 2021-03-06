#ifndef _USB_ADB_H
#define _USB_ADB_H

#include "device.h"

#define MAX_PAYLOAD 4096

#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_AUTH 0x48545541

#define A_VERSION 0x01000000        // ADB protocol version

#define ADB_VERSION_MAJOR 1         // Used for help/version information
#define ADB_VERSION_MINOR 0         // Used for help/version information

#define ADB_SERVER_VERSION    26    // Increment this when we want to force users to start a new adb server

//#define DEFAULT_ADB_PORT 5037
//#define DEFAULT_ADB_LOCAL_TRANSPORT_PORT 5555

#define ADB_CLASS              0xff
#define ADB_SUBCLASS           0x42

#define ADB_PROTOCOL           0x01
#define FB_PROTOCOL            0x03

//fastboot operate
#define OP_DOWNLOAD   1
#define OP_COMMAND    2
#define OP_QUERY      3
#define OP_NOTICE     4

//typedef struct usb_handle usb_handle;

void adb_usb_init(void);

/// Enumerates present and available interfaces (devices), opens new ones and
/// registers usb transport for them.
void find_devices(BOOL flashdirect);

/* usb host/client interface */
int usb_write(usb_handle *h, const void *data, int len);
int usb_read(usb_handle *h, void *data, int len, bool fulfill);
int usb_close(usb_handle *h);
void usb_kick(usb_handle *h);

usb_handle* usb_handle_enum_init(void);
usb_handle* usb_handle_next(usb_handle* usb);
void usb_set_work(usb_handle* usb, BOOL bwork);

BOOL usb_is_work(usb_handle* usb);

#endif /* _USB_ADB_H */
