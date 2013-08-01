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
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  TRACE_TAG  TRACE_TRANSPORT
#include "adb.h"

#include "usb_vendors.h"

#define fix_endians(p) do {} while (0)
unsigned host_to_le32(unsigned n)
{
    return n;
}


int check_header(apacket *p)
{
    if(p->msg.magic != (p->msg.command ^ 0xffffffff)) {
        D("check_header(): invalid magic\n");
        return -1;
    }

    if(p->msg.data_length > MAX_PAYLOAD) {
        D("check_header(): %d > MAX_PAYLOAD\n", p->msg.data_length);
        return -1;
    }

    return 0;
}

int check_data(apacket *p)
{
    unsigned count, sum;
    unsigned char *x;

    count = p->msg.data_length;
    x = p->data;
    sum = 0;
    while(count-- > 0) {
        sum += *x++;
    }

    if(sum != p->msg.data_check) {
        return -1;
    } else {
        return 0;
    }
}


static int remote_read(apacket *p, atransport *t)
{
    if(usb_read(t->usb, &p->msg, sizeof(amessage), true)){
        D("remote usb: read terminated (message)\n");
        return -1;
    }

    fix_endians(p);

    if(check_header(p)) {
        D("remote usb: check_header failed\n");
        return -1;
    }

    if(p->msg.data_length) {
        if(usb_read(t->usb, p->data, p->msg.data_length,true)){
            D("remote usb: terminated (data)\n");
            return -1;
        }
    }

    if(check_data(p)) {
        D("remote usb: check_data failed\n");
        return -1;
    }

    return 0;
}

static int remote_write(apacket *p, atransport *t)
{
    unsigned size = p->msg.data_length;

    fix_endians(p);

    if(usb_write(t->usb, &p->msg, sizeof(amessage)) != sizeof(amessage)) {
        D("remote usb: 1 - write terminated\n");
        return -1;
    }
    if(p->msg.data_length == 0) return 0;
    if(usb_write(t->usb, &p->data, size) != size) {
        D("remote usb: 2 - write terminated\n");
        return -1;
    }

    return 0;
}

static void remote_close(atransport *t)
{
    usb_close(t->usb);
    t->usb = 0;
}

static void remote_kick(atransport *t)
{
    usb_kick(t->usb);
}

void init_usb_transport(atransport *t, usb_handle *h, int state)
{
    D("transport: usb\n");
    t->close = remote_close;
    t->kick = remote_kick;
    t->read_from_remote = remote_read;
    t->write_to_remote = remote_write;
    t->sync_token = 1;
    t->connection_state = state;
    t->type = kTransportUsb;
    t->usb = h;
//    ADB_MUTEX(transport_lock);
}

int is_adb_interface(int vid, int pid, int usb_class, int usb_subclass, int usb_protocol)
{
    unsigned i;
    for (i = 0; i < gUSBIdCount; i++) {
        if (vid == gUSBIds[i].vid) {
            if (usb_class == ADB_CLASS && usb_subclass == ADB_SUBCLASS &&
                    usb_protocol == ADB_PROTOCOL) {
                return ADB_PROTOCOL;
            }

            return 0;
        }
    }

    return 0;
}


/// Checks if interface (device) matches certain criteria
int is_fastboot_interface(int vid, int pid, int usb_class, int usb_subclass, int usb_protocol)
{
    if(usb_class != ADB_CLASS ) return 0;
    if(usb_subclass != ADB_SUBCLASS) return 0;
    if(usb_protocol != FB_PROTOCOL) return 0;

    for (unsigned i = 0; i < gUSBIdCount; i++) {
        if (vid == gUSBIds[i].vid) {
                return FB_PROTOCOL;
        }
    }

    return 0;
}
