/*
 * Copyright (C) 2008 The Android Open Source Project
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
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char*
buff_addc (char*  buff, char*  buffEnd, int  c)
{
    int  avail = buffEnd - buff;

    if (avail <= 0)  /* already in overflow mode */
        return buff;

    if (avail == 1) {  /* overflowing, the last byte is reserved for zero */
        buff[0] = 0;
        return buff + 1;
    }

    buff[0] = (char) c;  /* add char and terminating zero */
    buff[1] = 0;
    return buff + 1;
}

char*
buff_adds (char*  buff, char*  buffEnd, const char*  s)
{
    int  slen = strlen(s);

    return buff_addb(buff, buffEnd, s, slen);
}

char*
buff_addb (char*  buff, char*  buffEnd, const void*  data, int  len)
{
    int  avail = (buffEnd - buff);

    if (avail <= 0 || len <= 0)  /* already overflowing */
        return buff;

    if (len > avail)
        len = avail;

    memcpy(buff, data, len);

    buff += len;

    /* ensure there is a terminating zero */
    if (buff >= buffEnd) {  /* overflow */
        buff[-1] = 0;
    } else
        buff[0] = 0;

    return buff;
}

char*
buff_add  (char*  buff, char*  buffEnd, const char*  format, ... )
{
    int      avail;

    avail = (buffEnd - buff);

    if (avail > 0) {
        va_list  args;
        int      nn;

        va_start(args, format);
        nn = vsnprintf( buff, avail, format, args);
        va_end(args);

        if (nn < 0) {
            /* some C libraries return -1 in case of overflow,
             * but they will also do that if the format spec is
             * invalid. We assume ADB is not buggy enough to
             * trigger that last case. */
            nn = avail;
        }
        else if (nn > avail) {
            nn = avail;
        }

        buff += nn;

        /* ensure that there is a terminating zero */
        if (buff >= buffEnd)
            buff[-1] = 0;
        else
            buff[0] = 0;
    }
    return buff;
}


const char * basename(const char * f_name) {
	/* split file name out from f_name */
	int len = strlen(f_name);
	int i = len;
	while (i-- > 0)
	{
		if (f_name[i-1] == '\\')
		{
			break;
		}
	}
	if (i == len || i <= 0)
	{
		return f_name;
	}

	return f_name + i;
}

CString GetFileNameFromFullPath(CString FullPath)   
{   
	int Where;   
	Where = FullPath.ReverseFind('\\');
	CString FileName = FullPath.Right(FullPath.GetLength() - 1 - Where);
	return FileName;   
}

CString GetAppPath(CString & sPath )
{
	wchar_t path_buffer[MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];

//	GetCurrentDirectory(MAX_PATH, currdir);
	GetModuleFileName(NULL, path_buffer, MAX_PATH);
	_wsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, 0, 0, 0, 0);

	sPath = drive;
	sPath += dir;

	return sPath;
}

void get_my_path(char *exe, size_t maxLen)
{
    char  *r;

    /* XXX: should be GetModuleFileNameA */
//    if (GetModuleFileNameA(NULL, exe, maxLen) > 0) {
    if( GetCurrentDirectoryA(maxLen, exe) > 0) {
        r = strrchr(exe, '\\');
        if (r != NULL)
            *r = '\0';
    } else {
        exe[0] = '\0';
    }
}


#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}

long now(void)
{
#if 0
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000;
#else

  FILETIME ft;
  unsigned __int64 tmpres = 0;

  GetSystemTimeAsFileTime(&ft);

  tmpres |= ft.dwHighDateTime;
  tmpres <<= 32;
  tmpres |= ft.dwLowDateTime;
  return (long)(tmpres / 10000000UL);
#endif
}

// called from fastboot.c
void sleep(int seconds)
{
    Sleep(seconds * 1000);
}

 PWCH MultiStrToWideStr(PCCH pc)
{
    ULONG nBytes;
  	PWCH wcs;

    nBytes = MultiByteToWideChar(CP_ACP,0,pc, -1, NULL, 0);
    if (nBytes == 0) return NULL;

    wcs = new WCHAR[nBytes];
    if(!wcs) return NULL;

    nBytes = MultiByteToWideChar(CP_ACP,0, pc,-1,wcs,nBytes);
    if (nBytes == 0)
    {
        delete [] wcs;
        return NULL;
    }

    return wcs;
}

PCHAR WideStrToMultiStr(PCWCH WideStr)
{
    ULONG nBytes;
	PCHAR MultiStr;

    nBytes = WideCharToMultiByte(CP_ACP,0,WideStr, -1, NULL, 0, NULL, NULL);
    if (nBytes == 0) return NULL;

    MultiStr = new char[nBytes];
    if(!MultiStr) return NULL;

    nBytes = WideCharToMultiByte(CP_ACP,0,WideStr,-1,MultiStr,nBytes,NULL,NULL);
    if (nBytes == 0)
    {
        delete [] MultiStr;
        return NULL;
    }

    return MultiStr;
}

/**************************************************************************/
/**************************************************************************/
/*****                                                                *****/
/*****      replaces libs/cutils/load_file.c                          *****/
/*****                                                                *****/
/**************************************************************************/
/**************************************************************************/

void *load_file(LPCWSTR fn, unsigned *_sz)
{
    HANDLE    file;
    char     *data;
    DWORD     file_size;

    file = CreateFile( fn,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL );

    if (file == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "load_file: file open failed (rc=%ld)\n", GetLastError());
        return NULL;
    }

    file_size = GetFileSize( file, NULL );
    data      = NULL;

    if (file_size > 0) {
        data = (char*) malloc( file_size + 1 );
        if (data == NULL) {
            fprintf(stderr, "load_file: could not allocate %ld bytes\n", file_size );
            file_size = 0;
        } else {
            DWORD  out_bytes;

            if ( !ReadFile( file, data, file_size, &out_bytes, NULL ) ||
                 out_bytes != file_size )
            {
                int retry_failed = 0;

                if (GetLastError() == ERROR_NO_SYSTEM_RESOURCES) {
                    /* Attempt to read file in 10MB chunks */
                    DWORD bytes_to_read = file_size;
                    DWORD bytes_read    = 0;
                    DWORD block_size    = 10*1024*1024;

                    SetFilePointer( file, 0, NULL, FILE_BEGIN );

                    while (bytes_to_read > 0) {
                        if (block_size > bytes_to_read) {
                            block_size = bytes_to_read;
                        }

                        if (!ReadFile( file, data+bytes_read,
                                       block_size, &out_bytes, NULL ) ||
                            out_bytes != block_size) {
                            retry_failed = 1;
                            break;
                        }
                        bytes_read    += block_size;
                        bytes_to_read -= block_size;
                    }
                } else {
                    retry_failed = 1;
                }

                if (retry_failed) {
                    fprintf(stderr, "load_file: could not read %ld bytes from '%s'\n", file_size, fn);
                    free(data);
                    data      = NULL;
                    file_size = 0;
                }
            }
        }
    } else {
        fprintf(stderr, "load_file: file empty or negative size %ld\n", file_size);
    }
    CloseHandle( file );

    *_sz = (unsigned) file_size;
    return  data;
}