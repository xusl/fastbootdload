/*
 * Copyright (C) 2007 The Android Open Source Project
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
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

#ifndef __ADB_H
#define __ADB_H

#include <limits.h>
#include "log.h"
#include "usb_adb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct amessage {
    unsigned command;       /* command identifier constant      */
    unsigned arg0;          /* first argument                   */
    unsigned arg1;          /* second argument                  */
    unsigned data_length;   /* length of payload (0 is allowed) */
    unsigned data_check;    /* checksum of data payload         */
    unsigned magic;         /* command ^ 0xffffffff             */
}amessage;

typedef struct apacket
{
    apacket *next;

    unsigned len;
    unsigned char *ptr;

    amessage msg;
    unsigned char data[MAX_PAYLOAD];
}apacket;

/* a transport object models the connection to a remote device or emulator
** there is one transport per connected device/emulator. a "local transport"
** connects through TCP (for the emulator), while a "usb transport" through
** USB (for real devices)
**
** note that kTransportHost doesn't really correspond to a real transport
** object, it's a special value used to indicate that a client wants to
** connect to a service implemented within the ADB server itself.
*/
typedef enum transport_type {
        kTransportUsb,
        kTransportLocal,
        kTransportAny,
        kTransportHost,
} transport_type;

typedef struct atransport
{
    atransport *next;
    atransport *prev;

    int (*read_from_remote)(apacket *p, atransport *t);
    int (*write_to_remote)(apacket *p, atransport *t);
    void (*close)(atransport *t);
    void (*kick)(atransport *t);

    int ref_count;
    unsigned sync_token;
    int connection_state;
    transport_type type;
    void *key;

    /* usb handle or socket fd as needed */
    usb_handle *usb;

    /* used to identify transports for clients */
   // char *serial;
    char *product;
    int kicked;
} atransport;
void send_packet(apacket *p, atransport *t);

/* initialize a transport object's func pointers and state */
void init_usb_transport(atransport *t, usb_handle *usb, int state);

void   kick_transport( atransport*  t );
void transport_unref(atransport *t);

/* packet allocator */
apacket *get_apacket(void);
void put_apacket(apacket *p);

int check_header(apacket *p);
int check_data(apacket *p);

/* convenience wrappers around read/write that will retry on
** EINTR and/or short read/write.  Returns 0 on success, -1
** on error or EOF.
*/
int readx(int fd, void *ptr, size_t len);
int writex(int fd, const void *ptr, size_t len);

/* used for USB device detection */
int is_adb_interface(int vid, int pid, int usb_class, int usb_subclass, int usb_protocol);
int is_fastboot_interface(int vid, int pid, int usb_class, int usb_subclass, int usb_protocol);

#define CS_ANY       -1
#define CS_OFFLINE    0
#define CS_BOOTLOADER 1
#define CS_DEVICE     2
#define CS_HOST       3
#define CS_RECOVERY   4
#define CS_NOPERM     5 /* Insufficient permissions to communicate with the device */
#define CS_UNAUTHORIZED 7

#define CHUNK_SIZE (64*1024)

#ifdef __cplusplus
}
#endif
#endif
