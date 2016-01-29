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

#include "resource.h"
#include "PortStateUI.h"
#include "mdmfastbootDlg.h"
#include "fastbootflash.h"

using namespace std;

#include <msxml.h>
#include <atlstr.h>
#import "msxml6.dll" raw_interfaces_only

flash_image::flash_image(const wchar_t* config):
  image_list(NULL),
  image_last(NULL),
  a5sw_kern_ver("Unknown"),
  a5sw_sys_ver("Unknown"),
  a5sw_usr_ver("Unknown"),
  fw_ver("Unknown"),
  qcn_ver("Unknown"),
  nv_buffer(NULL),
  nv_num(0),
  nv_cmd(NULL)
{
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return ;
  }

  data_len = GetPrivateProfileString(PKG_SECTION,
                                     PKG_PATH,
                                     GetAppPath(path).GetString(),
                                     pkg_dir,
                                     MAX_PATH,
                                     config);

  if (pkg_dir[data_len - 1] != L'\\' ) {
    if ( data_len > MAX_PATH - 2) {
    ERROR("bad package directory in the section path.");
        return ;
    }
    pkg_dir[data_len] = L'\\';
    pkg_dir[data_len + 1] = L'\0';
    data_len++;
    }

  wcsncpy(pkg_conf_file, pkg_dir, data_len+1);
  wcsncat(pkg_conf_file, PKG_CONFIG_XML, sizeof(pkg_conf_file) / sizeof(pkg_conf_file[0]) - data_len);

  wcsncpy(pkg_qcn_file, pkg_dir, data_len+1);
  wcsncat(pkg_qcn_file, PKG_STATIC_QCN, sizeof(pkg_qcn_file) / sizeof(pkg_qcn_file[0]) - data_len);

  read_config(config);
  read_package_version(pkg_conf_file);
}

flash_image::~flash_image() {
  reset(TRUE);
}

int flash_image::read_config(const wchar_t* config) {
  wchar_t partition_tbl[PARTITION_TBL_LEN] = {0};
  wchar_t filename[MAX_PATH];
  wchar_t *partition;
  size_t partition_len;
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return -1;
  }

  data_len = GetPrivateProfileString(PARTITIONTBL_SECTION,
                                     NULL,
                                     NULL,
                                     partition_tbl,
                                     PARTITION_TBL_LEN,
                                     config);

  if (data_len == 0) {
    WARN("no %S exist, load default partition table.", config);
    wchar_t *imgs[] = {
      L"mibib", L"sbl1.mbn",
      L"sbl2", L"sbl2.mbn",
      L"rpm", L"rpm.mbn",
      L"dsp1", L"dsp1.mbn",
      L"dsp3", L"dsp3.mbn",
      L"dsp2", L"dsp2.mbn",
      L"aboot", L"appsboot.mbn",
      L"boot", L"boot-oe-msm9615.img",
      L"system", L"9615-cdp-image-9615-cdp.yaffs2",
      L"userdata", L"9615-cdp-usr-image.usrfs.yaffs2",
    };

    for (int i = 0; i < sizeof(imgs)/ sizeof(imgs[0]); i += 2) {
        //parameter push stack from right to left
        add_image(imgs[i], imgs[i+1], TRUE, config);
    }

    set_package_dir(GetAppPath(path).GetString(), config);
    return 0;
  }

  partition = partition_tbl;
  partition_len = wcslen(partition);

  while (partition_len > 0) {
    data_len = GetPrivateProfileString(PARTITIONTBL_SECTION,
                                       partition,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);
    if (data_len > 0) {
//        path.Empty();
        path = pkg_dir;
        //path += L'\\';
        path += filename;
      add_image(partition, path.GetBuffer(), 0, config);
      path.ReleaseBuffer();
    }

    partition = partition + partition_len + 1;
    partition_len = wcslen(partition);
  }

  return 0;
}


BOOL flash_image::unpack_download_img(const wchar_t *lpath) {
    uint32 size;
    BYTE* pImgData = (BYTE*)load_file(lpath, &size);
     char filesInfoBuf[FILEINFO_HEAD_LEN];
        memset(filesInfoBuf, 0, FILEINFO_HEAD_LEN);
        memcpy(filesInfoBuf, pImgData + VERSION_HEAD_LEN, FILEINFO_HEAD_LEN);
    FilePosInfoS* fileInfo = (FilePosInfoS *)filesInfoBuf;
    return TRUE;
}

int flash_image::add_image( wchar_t *partition, const wchar_t *lpath, BOOL write, const wchar_t* config)
{

  FlashImageInfo* img = NULL;

  if (partition == NULL || lpath == NULL) {
    ERROR("Bad parameter");
    return -1;
  }

  img = (FlashImageInfo *)calloc(1, sizeof(FlashImageInfo));
  //img = (FlashImageInfo *)malloc(sizeof(FlashImageInfo));
  if (img == NULL) ERROR("out of memory");
  //memset(img, 0, sizeof(FlashImageInfo));
  img->data = load_file(lpath, &img->size);

  if (img->data == NULL) {
    ERROR("can not load data from file %S for  partition %S", lpath, partition);
    free(img);
    return -1;
  }
  int iDl = GetPrivateProfileInt(PARTITIONTBL_DL,
									  partition,
									  1,
									  config);

  img->need_download = (1==iDl)?true:false;
  img->partition = wcsdup(partition);
  img->partition_str = WideStrToMultiStr(partition);
  img->lpath = wcsdup(lpath);

  if (image_last != NULL)
    image_last->next = img;
  else
    image_list = img;

  image_last = img;

  DEBUG("Load data from file %S for partition %S", lpath, partition);

  if (write && config != NULL) {
    WritePrivateProfileString(PARTITIONTBL_SECTION,partition,lpath,config);
  }

  return 0;
}

const wchar_t * flash_image::get_package_dir(void) {
    return pkg_dir;
}

const wchar_t * flash_image::get_package_config(void) {
    return pkg_conf_file;
}


const wchar_t * flash_image::get_package_qcn_path(void) {
  return pkg_qcn_file;
}

BOOL flash_image::set_package_dir(const wchar_t * dir, const wchar_t* config, BOOL release) {
    if(dir == NULL) {
        ERROR("Bad Parameter.");
        return FALSE;
    }

    if (wcscmp(dir, pkg_dir) == 0) {
        INFO("Package directory is not changed.");
        return FALSE;
    }

    wcscpy(pkg_dir, dir);

    if (config != NULL)
    WritePrivateProfileString(PKG_SECTION, PKG_PATH, dir ,config);

#if 0
    if(release) {
        reset(FALSE);
        read_config(config);
        read_package_version(PKG_CONFIG_XML);
    }
#endif

    return TRUE;
}

const FlashImageInfo* flash_image::get_partition_info(wchar_t *partition, void **ppdata, unsigned *psize) {
  FlashImageInfo* img;

  // ASSERT( ppdata == NULL || psize == NULL );

  for (img = image_list; img; img = img->next) {
    if (wcscmp(partition, img->partition) == 0) {
      if(ppdata != NULL)
        *ppdata = img->data;
      if(psize != NULL)
        *psize = img->size;
      return img;
    }
  }
  return NULL;
}

const FlashImageInfo* flash_image::image_enum_init (void) {
    return image_list;
}

const FlashImageInfo* flash_image::image_enum_next (const FlashImageInfo* img) {
    if (img == NULL)
        return NULL;

    return img->next;
}

BOOL flash_image::qcn_cmds_enum_init (char *cmd) {
  if (cmd == NULL)
    cmd = "nv write";

  if (nv_cmd == NULL || strcmp(nv_cmd, cmd) != 0)  {
     FREE_IF(nv_cmd);
     nv_cmd = strdup(cmd);
     if(nv_cmd == NULL) {
      ERROR("OUT OF MEMORY");
      return FALSE;
     }

     if (nv_buffer != NULL) {
      QcnParser::PutNVWriteCommands(nv_buffer, nv_num);
      nv_buffer = NULL;
      nv_num = 0;
      }
  }

  if (nv_buffer == NULL)
   {
  QcnParser chQcn;

  if (chQcn.OpenDocument(pkg_qcn_file) == FALSE)
  {
    return FALSE;
  }
  return chQcn.GetNVWriteCommands(nv_cmd, &nv_buffer, &nv_num);
 // chQcn.PutNVWriteCommands(nvBuf, dwLens);
  }

    return TRUE;
}

const char* flash_image::qcn_cmds_enum_next (unsigned int index) {
  if (nv_buffer == NULL) {
    ERROR("No nv cmds buffer, may be should call qcn_cmds_enum_init first!");
    return NULL;
  }

  if (index >= nv_num) {
    ERROR("index (%d) is exceed nv number (%d)!", index, nv_num);
    return NULL;
  }

  return *(nv_buffer + index);
}

void flash_image::read_package_version(const wchar_t * package_conf){
  CComPtr<MSXML2::IXMLDOMDocument> spDoc;
  CComPtr<MSXML2::IXMLDOMNodeList> spNodeList;
  CComPtr<MSXML2::IXMLDOMElement> spElement;
  CComBSTR strTagName;
  VARIANT_BOOL bFlag;
  long lCount;
  HRESULT hr;

  ::CoInitialize(NULL);
  hr = spDoc.CoCreateInstance(__uuidof(MSXML2::DOMDocument));    //创建文档对象
  hr = spDoc->load(CComVariant(package_conf), &bFlag);       //load xml文件
  hr = spDoc->get_documentElement(&spElement);   //获取根结点
  if (spElement == NULL) {
    ERROR("No %S exist", package_conf);
    return;
    }
  hr = spElement->get_tagName(&strTagName);

  //cout << "------TagName------" << CString(strTagName) << endl;

  hr = spElement->get_childNodes(&spNodeList);   //获取子结点列表
  hr = spNodeList->get_length(&lCount);

  for (long i=0; i<lCount; ++i) {
    CComVariant varNodeValue;
    CComPtr<MSXML2::IXMLDOMNode> spNode;
    MSXML2::DOMNodeType NodeType;
    CComPtr<MSXML2::IXMLDOMNodeList> spChildNodeList;

    hr = spNodeList->get_item(i, &spNode);         //获取结点
    hr = spNode->get_nodeType(&NodeType);     //获取结点信息的类型

    if (NODE_ELEMENT == NodeType) {
      long childLen;
      hr = spNode->get_childNodes(&spChildNodeList);
      hr = spChildNodeList->get_length(&childLen);

      //cout << "------NodeList------" << endl;

      for (int j=0; j<childLen; ++j) {
        CComPtr<MSXML2::IXMLDOMNode> spChildNode;
        CComBSTR value;
        CComBSTR name;

        hr = spChildNodeList->get_item(j, &spChildNode);
        hr = spChildNode->get_nodeName(&name);            //获取结点名字
        hr = spChildNode->get_text(&value);                //获取结点的值
        //cout << CString(name) << endl;
        //cout << CString(value) << endl << endl;

        parse_pkg_sw(CString(name), CString(value));

        spChildNode.Release();
      }
    }

    spNode.Release();
    spChildNodeList.Release();
  }

  spNodeList.Release();
  spElement.Release();
  spDoc.Release();
  ::CoUninitialize();
}


 int flash_image::parse_pkg_sw(CString & node, CString & text) {
    if (node == L"Linux_Kernel_Ver")
        a5sw_kern_ver = text;

    else if (node == L"Linux_SYS_Ver")
        a5sw_sys_ver = text;

    else if (node == L"Linux_UserData_Ver")
        a5sw_usr_ver = text;

    else if (node == L"Q6_Resource_Ver")
        fw_ver = text;

    else if (node == L"QCN")
        qcn_ver = text;

    return 0;
}
 int flash_image::parse_pkg_hw(CString & node, CString & text) {
    return 0;
}

BOOL flash_image::get_pkg_a5sw_sys_ver(CString &version) {
    version = a5sw_sys_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_a5sw_usr_ver(CString &version) {
    version = a5sw_usr_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_a5sw_kern_ver(CString &version) {
    version = a5sw_kern_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_qcn_ver(CString &version) {
    version = qcn_ver;
    return TRUE;
}
BOOL flash_image::get_pkg_fw_ver(CString &version) {
    version = fw_ver;
    return TRUE;
}

BOOL flash_image::set_download_flag(CString strPartitionName, bool bDownload) {
	BOOL bRet = 0;
	FlashImageInfo* img = image_list;
	for(;img != NULL; ) {
		if (0 == wcscmp(img->partition, strPartitionName.GetBuffer()))
		{
			img->need_download = bDownload;
			bRet = 1;
			break;
		}
		img = img->next;
	}
	return bRet;
}

BOOL flash_image::reset(BOOL free_only) {
    FlashImageInfo *img;
    for (img = image_list; img; img = image_list) {
      image_list = img->next;
      if (img->partition != NULL) {
        free(img->partition);
        img->partition = NULL;
      }

      if (img->partition_str != NULL) {
        delete img->partition_str;
        img->partition_str = NULL;
      }

      if (img->lpath != NULL) {
        free(img->lpath);
        img->lpath = NULL;
      }

      if (img->data != NULL) {
        free(img->data);
        img->data = NULL;
      }

      free(img);
      img = NULL;
    }

    image_list = NULL;

    if (!free_only) {
      a5sw_kern_ver=("Unknown"),
      a5sw_sys_ver=("Unknown"),
      a5sw_usr_ver=("Unknown"),
      fw_ver=("Unknown"),
      qcn_ver=("Unknown");
      if (nv_buffer != NULL) {
      QcnParser::PutNVWriteCommands(nv_buffer, nv_num);
      nv_buffer = NULL;
      nv_num = 0;
      }
      FREE_IF (nv_cmd);
    }

	return TRUE;
}


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

int fastboot::cb_default(CWnd* hWnd, void* data, Action *a, int status, char *resp)
{
    if (status) {
        port_text_msg(hWnd, data, "FAILED (%s)\n", resp);
    } else {
        uint64 split = now();
        port_text_msg(hWnd, data, "OKAY [%7.3fs]\n", (split - a->start));
        a->start = split;
    }
    return status;
}

int fastboot::cb_check(CWnd* hWnd, void* data, Action *a, int status, char *resp, int invert)
{
    const char **value = (const char **)a->data;
    unsigned count = a->size;
    unsigned n;
    int yes;
    char msg[256]={0};
    int msg_len = 0;
    int write_len;

    if (status) {
        port_text_msg(hWnd, data, "FAILED (%s)\n", resp);
        return status;
    }

    yes = match(resp, value, count);
    if (invert) yes = !yes;

    if (yes) {
        long long split = now();
        port_text_msg(hWnd, data,"OKAY [%7.3fs]\n", (split - a->start));
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
    port_text_msg(hWnd, data, msg);
    return -1;
}

UINT fastboot::port_text_msg(CWnd* hWnd,void* data, const char *fmt,  ... ) {

    va_list ap;
    va_start(ap, fmt);
    char buffer[256]={0};

    //snprintf is not work properly .  vsprintf or _snprintf or _vsnprintf is OK.
    _vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    UIInfo* info = new UIInfo();

    info->infoType = PROMPT_TEXT;
    info->sVal = buffer;
    hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                  (WPARAM)info,
                  (LPARAM)data);
	UsbWorkData* wd = (UsbWorkData*) data;
	INFO("%s: %s", wd->devIntf->GetDevTag(), buffer);

    return 0;
}


UINT fastboot::port_progress(CWnd* hWnd,void* data, int process ) {
    UIInfo* info = new UIInfo();

    info->infoType = PROGRESS_VAL;
    info->iVal = process;
    hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                  (WPARAM)info,
                  (LPARAM)data);
    return 0;
}

int fastboot::cb_require(CWnd* hWnd, void* data, Action *a, int status, char *resp)
{
    return cb_check(hWnd, data, a, status, resp, 0);
}

int fastboot::cb_reject(CWnd* hWnd, void* data, Action *a, int status, char *resp)
{
    return cb_check(hWnd, data, a, status, resp, 1);
}

int fastboot::cb_do_nothing(CWnd* hWnd, void* data, Action *a, int status, char *resp)
{
    fprintf(stderr,"\n");
    return 0;
}

int fastboot::cb_display(CWnd* hWnd, void* data, Action *a, int status, char *resp)
{
    if (status) {
        port_text_msg(hWnd, data, "%s FAILED (%s)\n", a->cmd, resp);
        return status;
    }
    //fprintf(stderr, "%s: %s\n", (char*) a->data, resp);
    port_text_msg(hWnd, data, "%s: %s", (char*) a->data, resp);
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
    a->data = strdup(prettyname);
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

void fastboot::fb_execute_queue(usb_handle *usb,CWnd* hWnd, void* data)
{
    Action *a;
    char resp[FB_RESPONSE_SZ+1];
    int status;
    long long start = -1;
    unsigned total_size = image_size();
    unsigned flashed_size = 0;
	bool bIsBreak = false;

    a = action_list;
    resp[FB_RESPONSE_SZ] = 0;

    for (a = action_list; a; a = a->next) {
        a->start = now();
        if (start < 0) start = a->start;
        if (a->msg) {
            //fprintf(stderr,"%s...\n",a->msg);
            port_text_msg(hWnd, data, "%s...",a->msg);
        }
        if (a->op == OP_DOWNLOAD) {
            status = fb_download_data(usb, a->data, a->size);
            status = a->func(hWnd, data, a, status, status ? fb_get_error() : "");
            if (status)
				bIsBreak = true;
            flashed_size += a->size;
        } else if (a->op == OP_COMMAND) {
            status = fb_command(usb, a->cmd);
			status = a->func(hWnd, data, a, status, status ? fb_get_error() : "");
			if (status)
				bIsBreak = true;

            if (total_size == 0) {
              CRITICAL("Error total_size is 0");
              return;
            }

            port_progress(hWnd, data, (int) (100.0 * (double) flashed_size / total_size ));
        } else if (a->op == OP_QUERY) {
            status = fb_command_response(usb, a->cmd, resp);
			status = a->func(hWnd, data, a, status, status ? fb_get_error() : resp);
			if (status)
				bIsBreak = true;
        } else if (a->op == OP_NOTICE) {
            //fprintf(stderr,"%s\n",(char*)a->data);
            port_text_msg(hWnd, data, "%s\n",(char*)a->data);
        } else {
            ERROR("bogus action");
        }
		if (bIsBreak) {
			break;
		}

    }

	if (bIsBreak)
	{
		port_text_msg(hWnd, data, "download breaked. total time: %.3fs\n",
          (now() - start) / MILLS_SECONDS);
	}
	else
	{
		port_text_msg(hWnd, data, "finished. total time: %.3fs\n",
          (now() - start) / MILLS_SECONDS);
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

