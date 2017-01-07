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
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <sys/time.h>
#include <log.h>

#include "PST.h"
//#include "mdmfastbootDlg.h"
#include "fastbootflash.h"

using namespace std;


int fastboot::match(char *str, const char **value, unsigned count)
{
   // const char *val;
    unsigned n;
   // int len;

    for (n = 0; n < count; n++) {
        const char *val = value[n];
        int len = strlen(val);
        int match;

        if ((len > 1) && (val[len-1] == '*')) {
            len--;
            match = !strncmp(val, str, len);
        } else {
            match = !strcmp(val, str);
        }

        if (match) return 1;
    }

    return 0;
}

int fastboot::cb_default(void* data, Action *a, int status, char *resp)
{
    if (status) {
        port_text_msg(data, "FAILED (%s)", resp);
    } else {
        long long split = now();
        long long elapse = (split - a->start);

        //%lld, refer to  adbhost::END;  %I64d
        port_text_msg(data, "OKAY [%lld.%03llds].",
                elapse / MILLS_SECONDS, (elapse % MILLS_SECONDS) / MICRO_SECONDS);
        a->start = split;
    }
    return status;
}

int fastboot::cb_check(void* data, Action *a, int status, char *resp, int invert)
{
    const char **value = (const char **)a->data;
    unsigned count = a->size;
    unsigned n;
    int yes;
    char msg[256]={0};
    int msg_len = 0;
    int write_len;

    if (status) {
        port_text_msg(data, "FAILED (%s)", resp);
        return status;
    }

    yes = match(resp, value, count);
    if (invert) yes = !yes;

    if (yes) {
        long long split = now();
        long long elapse = (split - a->start);
        port_text_msg(data,"OKAY [%lld.%03llds]",
            elapse / MILLS_SECONDS, (elapse % MILLS_SECONDS) / 1000LL);
        a->start = split;
        return 0;
    }

    write_len = _snprintf(msg, sizeof(msg), "FAILED\n\nDevice %s is '%s'.\nUpdate %s '%s'",
        a->cmd + 7, resp, invert ? "rejects" : "requires", value[0]);
    msg_len += write_len;
    //fprintf(stderr,"FAILED\n\n");
    //fprintf(stderr,"Device %s is '%s'.\n", a->cmd + 7, resp);
    //fprintf(stderr,"Update %s '%s'",
    //        invert ? "rejects" : "requires", value[0]);
    for (n = 1; n < count && msg_len < sizeof(msg); n++) {
        //fprintf(stderr," or '%s'", value[n]);
        write_len = _snprintf(msg + msg_len, sizeof(msg) - msg_len, " or '%s'", value[n]);
        if (write_len > 0) {
            msg_len += write_len;
        } else {
            msg_len = sizeof(msg);
            break;
        }
    }
    //fprintf(stderr,".\n\n");
    if (msg_len < sizeof(msg))
        _snprintf(msg + msg_len, sizeof(msg) - msg_len, ".\n\n");
    port_text_msg(data, msg);
    return -1;
}

UINT fastboot::port_text_msg(void* data, const char *fmt,  ... ) {
    va_list ap;
    va_start(ap, fmt);
    char buffer[256]={0};

    //snprintf is not work properly .  vsprintf or _snprintf or _vsnprintf is OK.
    _vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

	UsbWorkData* wd = (UsbWorkData*) data;
    wd->SetInfo(PROMPT_TEXT, buffer);
	INFO("%s: %s", wd->GetDevTag(), buffer);

    return 0;
}

int fastboot::cb_require(void* data, Action *a, int status, char *resp)
{
    return cb_check(data, a, status, resp, 0);
}

int fastboot::cb_reject(void* data, Action *a, int status, char *resp)
{
    return cb_check(data, a, status, resp, 1);
}

int fastboot::cb_do_nothing(void* data, Action *a, int status, char *resp)
{
    fprintf(stderr,"\n");
    return 0;
}

int fastboot::cb_display(void* data, Action *a, int status, char *resp)
{
    if (status) {
        port_text_msg(data, "%s FAILED (%s)", a->cmd, resp);
        return status;
    }
    //fprintf(stderr, "%s: %s\n", (char*) a->data, resp);
    port_text_msg(data, "%s: %s", (char*) a->data, resp);
    return 0;
}


fastboot::~fastboot() {
    remove_action();
}

fastboot::fastboot(usb_handle * handle):action_list(0), action_last(0) {
}

char *fastboot::mkmsg(const char *fmt, ...)
{
    char buf[256];
    char *s;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    s = strdup(buf);
    if (s == 0) ERROR("out of memory");
    return s;
}

void fastboot::remove_action() {
  Action *a;

  for (a = action_list; a; a = action_list) {
    action_list = a->next;

#if 0
    if (a->data) {
      free(a->data);
      a->data = NULL;
    }
#else
    a->data = NULL;
#endif

    if (a->msg) {
      free((void *)a->msg);
      a->msg = NULL;
    }
    a->func = NULL;
    a->next = NULL;
    free(a);
    a = NULL;
  }
  action_list = 0;
  action_last = 0;
}

Action * fastboot::queue_action(unsigned op, const char *fmt, ...)
{
    Action *a;
    va_list ap;

    a = (Action *)calloc(1, sizeof(Action));
    if (a == 0) ERROR("out of memory");

    va_start(ap, fmt);
    vsprintf(a->cmd, fmt, ap);
    va_end(ap);

    if (action_last) {
        action_last->next = a;
    } else {
        action_list = a;
    }
    action_last = a;
    a->op = op;
    a->func = cb_default;
    a->start = -1;
    return a;
}

void fastboot::fb_queue_erase(const char *ptn)
{
    Action *a;
    a = queue_action(OP_COMMAND, "erase:%s", ptn);
    a->msg = mkmsg("erasing '%s'", ptn);
}

void fastboot::fb_queue_flash(const char *ptn, void *data, unsigned sz)
{
    Action *a;

    a = queue_action(OP_DOWNLOAD, "");
    a->data = data;
    a->size = sz;
    a->msg = mkmsg("sending '%s' (%d KB)", ptn, sz / 1024);

    a = queue_action(OP_COMMAND, "flash:%s", ptn);
    a->msg = mkmsg("writing '%s'", ptn);
}
void fastboot::fb_queue_require(const char *var, int invert, unsigned nvalues, const char **value)
{
    Action *a;
    a = queue_action(OP_QUERY, "getvar:%s", var);
    a->data = value;
    a->size = nvalues;
    a->msg = mkmsg("checking %s", var);
    a->func = invert ? cb_reject : cb_require;
    if (a->data == 0) ERROR("out of memory");
}



void fastboot::fb_queue_display(const char *var, const char *prettyname)
{
    Action *a;
    a = queue_action(OP_QUERY, "getvar:%s", var);
    a->data = (void *)prettyname;// strdup(prettyname);
    if (a->data == 0) ERROR("out of memory");
    a->func = cb_display;
}


void fastboot::fb_queue_reboot(void)
{
    Action *a = queue_action(OP_COMMAND, "reboot");
    a->func = cb_do_nothing;
    a->msg = mkmsg("rebooting");
}

void fastboot::fb_queue_command(const char *cmd, const char *msg)
{
    Action *a = queue_action(OP_COMMAND, cmd);
    a->msg = msg;
}

void fastboot::fb_queue_download(const char *name, void *data, unsigned size)
{
    Action *a = queue_action(OP_DOWNLOAD, "");
    a->data = data;
    a->size = size;
    a->msg = mkmsg("downloading '%s'", name);
}

void fastboot::fb_queue_notice(const char *notice)
{
    Action *a = queue_action(OP_NOTICE, "");
    a->data = (void*) notice;
}

unsigned fastboot::image_size(void) {
   Action *a;
   unsigned total_size = 0;

    for (a = action_list; a; a = a->next) {
        total_size += a->size;
    }

    return total_size;
}

void fastboot::fb_execute_queue(usb_handle *usb, void* data, int pad_size)
{
    Action *a;
    char resp[FB_RESPONSE_SZ+1];
    int status;
    long long start = -1;
    long long elapse = 0;
    unsigned total_size = image_size() + pad_size;
    unsigned flashed_size = 0 + pad_size;
	bool bIsBreak = false;

    a = action_list;
    resp[FB_RESPONSE_SZ] = 0;

    for (a = action_list; a; a = a->next) {
        a->start = now();
        if (start < 0) start = a->start;
        if (a->msg) {
            //fprintf(stderr,"%s...\n",a->msg);
            port_text_msg(data, "%s...",a->msg);
        }
        if (a->op == OP_DOWNLOAD) {
            status = fb_download_data(usb, a->data, a->size);
            status = a->func(data, a, status, status ? fb_get_error() : "");
            if (status)
				bIsBreak = true;
            flashed_size += a->size;
        } else if (a->op == OP_COMMAND) {
            status = fb_command(usb, a->cmd);
			status = a->func(data, a, status, status ? fb_get_error() : "");
			if (status)
				bIsBreak = true;

            if (total_size == 0) {
              CRITICAL("Error total_size is 0");
              return;
            }

            ((UsbWorkData*)data)->SetProgress((int) (100.0 * (double) flashed_size / total_size ));
        } else if (a->op == OP_QUERY) {
            status = fb_command_response(usb, a->cmd, resp);
			status = a->func(data, a, status, status ? fb_get_error() : resp);
			if (status)
				bIsBreak = true;
        } else if (a->op == OP_NOTICE) {
            //fprintf(stderr,"%s\n",(char*)a->data);
            port_text_msg(data, "%s",(char*)a->data);
        } else {
            ERROR("bogus action");
        }
		if (bIsBreak) {
			break;
		}

    }

	elapse = now() - start;
    if (bIsBreak)
	{
		port_text_msg(data, "fastboot download breaked. total time: %lld.%03llds",
          elapse / MILLS_SECONDS, (elapse % MILLS_SECONDS) / 1000LL);
	}
	else
	{
		port_text_msg(data, "fastboot download finished. total time: %lld.%03llds",
          elapse / MILLS_SECONDS, (elapse % MILLS_SECONDS) / 1000LL);
	}
}

/////////////////////////////////////////////////////////////////////////////
//fastboot protocol

char *fastboot::fb_get_error(void)
{
    return ERRBUF;
}

int fastboot::check_response(usb_handle *usb, unsigned size,
                          unsigned data_okay, char *response)
{
    unsigned char status[65];
    int r;

    for(;;) {
        r = usb_read(usb, status, 64, false);
        if(r < 0) {
            sprintf(ERRBUF, "status read failed (%s)", strerror(errno));
            return -1;
        }
        status[r] = 0;

        if(r < 4) {
            sprintf(ERRBUF, "status malformed (%d bytes)", r);
            return -1;
        }

        if(!memcmp(status, "INFO", 4)) {
            fprintf(stderr,"(bootloader) %s\n", status + 4);
            continue;
        }

        if(!memcmp(status, "OKAY", 4)) {
            if(response) {
                strcpy(response, (char*) status + 4);
            }
            return 0;
        }

        if(!memcmp(status, "FAIL", 4)) {
            if(r > 4) {
                sprintf(ERRBUF, "remote: %s", status + 4);
            } else {
                strcpy(ERRBUF, "remote failure");
            }
            return -1;
        }

        if(!memcmp(status, "DATA", 4) && data_okay){
            unsigned dsize = strtoul((char*) status + 4, 0, 16);
            if(dsize > size) {
                strcpy(ERRBUF, "data size too large");
                return -1;
            }
            return dsize;
        }

        strcpy(ERRBUF,"unknown status code");
        break;
    }

    return -1;
}

int fastboot::_command_send(usb_handle *usb, const char *cmd,
                         const void *data, unsigned size,
                         char *response)
{
    int cmdsize = strlen(cmd);
    int r;

    if(response) {
        response[0] = 0;
    }

    if(cmdsize > 64) {
        sprintf(ERRBUF,"command too large");
        return -1;
    }

    if(usb_write(usb, cmd, cmdsize) != cmdsize) {
        sprintf(ERRBUF,"command write failed (%s)", strerror(errno));
        return -1;
    }

    if(data == 0) {
        return check_response(usb, size, 0, response);
    }

    r = check_response(usb, size, 1, 0);
    if(r < 0) {
        return -1;
    }
    size = r;

    if(size) {
        r = usb_write(usb, data, size);
        if(r < 0) {
            sprintf(ERRBUF, "data transfer failure (%s)", strerror(errno));
            return -1;
        }
        if(r != ((int) size)) {
            sprintf(ERRBUF, "data transfer failure (short transfer)");
            return -1;
        }
    }

    r = check_response(usb, 0, 0, 0);
    if(r < 0) {
        return -1;
    } else {
        return size;
    }
}

int fastboot::fb_command(usb_handle *usb, const char *cmd)
{
    return _command_send(usb, cmd, 0, 0, 0);
}

int fastboot::fb_command_response(usb_handle *usb, const char *cmd, char *response)
{
    return _command_send(usb, cmd, 0, 0, response);
}

int fastboot::fb_download_data(usb_handle *usb, const void *data, unsigned size)
{
    char cmd[64];
    int r;

    sprintf(cmd, "download:%08x", size);
    r = _command_send(usb, cmd, data, size, 0);

    if(r < 0) {
        return -1;
    } else {
        return 0;
    }
}

