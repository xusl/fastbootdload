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
#include "stdafx.h"

#define TRACE_TAG               TRACE_USB
#include "atlbase.h"
#include "usb_vendors.h"
#include "adb_dev_register.h"

#include <stdio.h>

#  define WIN32_LEAN_AND_MEAN
#  include "windows.h"
#  include "shlobj.h"
#include "limits.h"
#include "dirent.h"

#include "adb.h"



/** built-in vendor list */
usbid_t builtInIds[] = {

    //VENDOR_ID_QUALCOMM
    {VENDOR_ID_ALCATEL, 0x0197},
    {VENDOR_ID_ALCATEL, 0x0196},
    {VENDOR_ID_ALCATEL, 0x0195},
    {VENDOR_ID_ALCATEL, 0x0192},  //9x15 adb
    {VENDOR_ID_ALCATEL, 0x007b},  //9x15 adb only
    {VENDOR_ID_GOOGLE,0xd00d},    //fastboot
    //{VENDOR_ID_SAMSUNG, 0x6866},
};

#define BUILT_IN_ID_COUNT    (sizeof(builtInIds)/sizeof(builtInIds[0]))

/* max number of supported vendor ids (built-in + 3rd party). increase as needed */
#define ID_COUNT_MAX         128

usbid_t gUSBIds[ID_COUNT_MAX];
unsigned gUSBIdCount = 0;

int get_adb_usb_ini(char* buff, size_t len);

void usb_vendors_init(void) {
  char temp[PATH_MAX];
  char *seperate;
  long pid, vid;
  if (ID_COUNT_MAX < BUILT_IN_ID_COUNT) {
    fprintf(stderr, "ID_COUNT_MAX not big enough for built-in vendor list.\n");
    exit(2);
  }

  /* add the built-in vendors at the beginning of the array */
  memcpy(gUSBIds, builtInIds, sizeof(builtInIds));

  /* default array size is the number of built-in vendors */
  gUSBIdCount = BUILT_IN_ID_COUNT;

  if (ID_COUNT_MAX == BUILT_IN_ID_COUNT)
    return;

  get_adb_usb_ini(temp, sizeof(temp)) ;
  FILE * f = fopen(temp, "rt");

  if (f != NULL) {
    /* The vendor id file is pretty basic. 1 vendor id per line.
       Lines starting with # are comments */
    while (fgets(temp, sizeof(temp), f) != NULL) {
      if (temp[0] == '#')
        continue;

      vid = strtol(temp, &seperate, 0);
      if (errno == EINVAL || errno == ERANGE || vid > INT_MAX || vid < 0) {
        fprintf(stderr, "Invalid content in %s. Quitting.\n", ANDROID_ADB_INI);
        continue;
      }
      pid = strtol(seperate+1, NULL, 0);
      if (errno == EINVAL || errno == ERANGE || pid > INT_MAX || pid < 0) {
        fprintf(stderr, "Invalid content in %s. Quitting.\n", ANDROID_ADB_INI);
        continue;
      }

      gUSBIds[gUSBIdCount].vid = (int)vid;
      gUSBIds[gUSBIdCount].pid = (int)pid;

      /* make sure we don't go beyond the array */
      if (gUSBIdCount++ == ID_COUNT_MAX) {
        break;
      }
    }
    fclose(f);
  }else {
    fprintf(stderr, ANDROID_ADB_INI " is not exist.\n");
    fprintf(stderr, "You can add more vid/pid in this file.\n");
    fprintf(stderr, "such as:\n");
    fprintf(stderr, "0x1bbb 0x0192\n");
    fprintf(stderr, "line begin with \'#\' is comment\n");
  }
  check_regedit_usbflags(gUSBIds, gUSBIdCount);
}


/* fills buff with the path to the adb vendor id file. returns 0 if success */
int get_adb_usb_ini(char* buff, size_t len) {
  const char* home = getenv("ANDROID_SDK_HOME");
  if (home != NULL) {
    return _snprintf(buff, len, "%s\\%s\\%s",
                     home, ANDROID_PATH, ANDROID_ADB_INI) >= len ? 1 : 0;
  } else {
    char path[MAX_PATH];
    //SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path);
    get_my_path(path, sizeof(path));
    return _snprintf(buff, len, "%s\\%s",
                     path, ANDROID_ADB_INI)>= len ? 1 : 0;
  }
  return 0;
}

