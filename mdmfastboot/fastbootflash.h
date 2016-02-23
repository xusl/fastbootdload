/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _FASTBOOT_H_
#define _FASTBOOT_H_

#include "usb_adb.h"
#include <map>

//#ifdef __cplusplus
//extern "C" {
//#endif

#define FB_COMMAND_SZ 64
#define FB_RESPONSE_SZ 64
#define VERSION_STR_LEN 16


typedef struct Action
{
    unsigned op;
    struct Action *next;

    char cmd[64];
    void *data;
    unsigned size;

    const char *msg;
    int (*func)(void* data, Action *a, int status, char *resp);

    long long start;
}Action;

class fastboot {
public:
	 fastboot(usb_handle * handle);
	 ~fastboot();

   /* protocol.c - fastboot protocol */
   int fb_command(usb_handle *usb, const char *cmd);
   int fb_command_response(usb_handle *usb, const char *cmd, char *response);
   int fb_download_data(usb_handle *usb, const void *data, unsigned size);
   char *fb_get_error(void);

   /* engine.c - high level command queue engine */

   void fb_queue_flash(const char *ptn, void *data, unsigned sz);
   void fb_queue_erase(const char *ptn);
   void fb_queue_require(const char *var, int invert, unsigned nvalues, const char **value);
   void fb_queue_display(const char *var, const char *prettyname);
   void fb_queue_reboot(void);
   void fb_queue_command(const char *cmd, const char *msg);
   void fb_queue_download(const char *name, void *data, unsigned size);
   void fb_queue_notice(const char *notice);
   void fb_execute_queue(usb_handle *usb,void* data, int pad_size);

private:
   Action *action_list;
   Action *action_last;
   char ERRBUF[128];

   Action *queue_action(unsigned op, const char *fmt, ...);
	 void remove_action();
   unsigned image_size(void);
   char *mkmsg(const char *fmt, ...);

   int check_response(usb_handle *usb, unsigned size,  unsigned data_okay, char *response);
   int _command_send(usb_handle *usb, const char *cmd,
                     const void *data, unsigned size,
                     char *response);
   static int cb_display(void* data, Action *a, int status, char *resp);
   static int cb_do_nothing(void* data, Action *a, int status, char *resp);
   static int cb_reject(void* data, Action *a, int status, char *resp);
   static int cb_require(void* data, Action *a, int status, char *resp);

   static UINT port_text_msg(void* data, const char *fmt,  ... );
   static int cb_check(void* data, Action *a, int status, char *resp, int invert);
   static int cb_default(void* data, Action *a, int status, char *resp);
   static int match(char *str, const char **value, unsigned count);
};
//#ifdef __cplusplus
//}
//#endif

#endif
