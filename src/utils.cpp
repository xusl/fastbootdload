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
#include "define.h"
#include "utils.h"
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Psapi.h"

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "ws2_32.lib")

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

CString GetDirName(CString path) {
    CString dirName = _T("");
    wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];

//	GetCurrentDirectory(MAX_PATH, currdir);
	_wsplitpath_s(path.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, 0, 0, 0, 0);

	dirName = drive;
	dirName += dir;

	return dirName;
}

CString GetBaseName(CString path) {
    CString baseName = _T("");
    wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

//	GetCurrentDirectory(MAX_PATH, currdir);
	_wsplitpath_s(path.GetString(), 0, 0, 0, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

	baseName = fname;
	baseName += ext;

	return baseName;
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
CString TrimPathDelimitor(CString &path) {
	int index = -1;
	CString normal = path;
	while((index = normal.ReverseFind(PATH_DELIMITER)) == (normal.GetLength() -1))
		normal = normal.Left(index );		
	return normal;
}

CString UpDir(CString & originPath) {
	CString target = TrimPathDelimitor(originPath);
//  target = originPath.Left(originPath.GetLength()-1);
    return target.Left(target.ReverseFind(PATH_DELIMITER));	
}

//if c:/bar/foo, return foo
CString CurrentDirName(CString & originPath) {
	CString target = TrimPathDelimitor(originPath);
//  target = originPath.Left(originPath.GetLength()-1);
    return target.Right(target.GetLength() - target.ReverseFind(PATH_DELIMITER) - 1);	
}

void ScanDir (const wchar_t *szDirectory, const wchar_t *pattern, list<CString>& files, BOOL useSpec, BOOL recursively)
{
    HANDLE      hFind;
    WIN32_FIND_DATA  FindData;
    wchar_t     szFileSpec [_MAX_PATH + 5] = { 0};
#if 0
    FILETIME    FtLocal;
    SYSTEMTIME  SysTime;
    wchar_t        szLine [256];
    wchar_t        szDate [sizeof "jj/mm/aaaa"];
#endif
    szFileSpec [_MAX_PATH - 1] = 0;

	if (szDirectory == NULL || wcslen(szDirectory) == 0) {
		LOGE("Invalid parameter");
		return;
	}

    lstrcpyn (szFileSpec, szDirectory, _MAX_PATH);
	if (szFileSpec[wcslen(szFileSpec) -1 ] != PATH_DELIMITER) {
			lstrcat (szFileSpec, _T("\\"));
	}
	
    if (useSpec) {
	    lstrcat (szFileSpec, pattern);
	} else {		
		lstrcat (szFileSpec, _T("*.*"));				
	}
	
    hFind = FindFirstFile (szFileSpec, &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        LOGE("scan dir initialize failed %S, %S. %d", szDirectory, szFileSpec, GetLastError());
        return;
    }

    do {		
        CString item = szDirectory;
        item += PATH_DELIMITERS;//_T("\\");
        item += FindData.cFileName;
		
        // display only files, skip directories
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(0 == lstrcmp(FindData.cFileName, _T("..")) || 0 == lstrcmp(FindData.cFileName, _T(".")))
				continue;
			if (recursively )
				ScanDir(item.GetString(), pattern, files, useSpec, recursively);
            continue;
    	}
#if 0
        FileTimeToLocalFileTime (& FindData.ftCreationTime, & FtLocal);
        FileTimeToSystemTime (& FtLocal, & SysTime);
        GetDateFormat (LOCALE_SYSTEM_DEFAULT,
                       DATE_SHORTDATE,
                       & SysTime,
                       NULL,
                       szDate, sizeof szDate);
        szDate [sizeof "jj/mm/aaaa" - 1]=0;    // truncate date
        FindData.cFileName[62] = 0;      // truncate file name if needed
        // dialog structure allow up to 64 char
        wsprintf (szLine, "%s\t%s\t%d",
                  FindData.cFileName, szDate, FindData.nFileSizeLow);
#endif
	    if (!useSpec) {
			if (pattern != NULL && wcslen(pattern) >= 0 && lstrcmp(pattern, FindData.cFileName) != 0)
				continue;
    	}
		
        LOGD("Find file %S", FindData.cFileName);
		files.push_back(item);		
    }while (FindNextFile (hFind, & FindData));
    FindClose (hFind);
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

long long now(void)
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
  return (long long)(tmpres / 10);
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
        LOGE("Convert string %S failed.", WideStr);
        return NULL;
    }

    return MultiStr;
}

int CharToBSTR(PCCH inParam, BSTR *outParam){
    ULONG size;
    int retVal = -1;
    WCHAR *tmp = NULL;

    size = MultiByteToWideChar(CP_OEMCP, 0, inParam, -1, tmp, 0);
    tmp = (WCHAR*)GlobalAlloc(GMEM_ZEROINIT, size * sizeof(WCHAR));
    retVal = MultiByteToWideChar(CP_OEMCP, 0, inParam, -1, tmp, size);

    if ( 0 != retVal ) {
        retVal = 1;
        *outParam = SysAllocString(tmp);
    }

    GlobalFree(tmp);

    return retVal;
}

BOOL CStringToString(CString& source, string &sink) {
#ifdef UNICODE
  PCHAR buffer = WideStrToMultiStr(source.GetString());
  if (buffer == NULL) {
      return FALSE;
  }
  sink = buffer;
  delete [] buffer;
#else
  sink = source.GetString();
#endif
  return TRUE;
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
        LOGE("load_file: file open %S failed (rc=%ld)\n", fn, GetLastError());
        return NULL;
    }

    file_size = GetFileSize( file, NULL );
    data      = NULL;

    if (file_size > 0) {
        data = (char*) malloc( file_size + 1 );
        if (data == NULL) {
            LOGE("load_file: could not allocate %ld bytes\n", file_size );
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
                    LOGE("load_file: could not read %ld bytes from '%s'\n", file_size, fn);
                    free(data);
                    data      = NULL;
                    file_size = 0;
                }
            }
        }
    } else {
        LOGE("load_file: file empty or negative size %ld\n", file_size);
    }
    CloseHandle( file );

    *_sz = (unsigned) file_size;
    return  data;
}

static int  _winsock_init;

static void
_cleanup_winsock( void )
{
    WSACleanup();
}

static void
_init_winsock( void )
{
    if (!_winsock_init) {
        WSADATA  wsaData;
        int      rc = WSAStartup( MAKEWORD(2,2), &wsaData);
        if (rc != 0) {
            LOGE( "adb: could not initialize Winsock" );
        }
        atexit( _cleanup_winsock );
        _winsock_init = 1;
    }
}

SOCKET ConnectServer(const char *ip_addr,  u_short port) {
    SOCKET sockClient = INVALID_SOCKET;
    SOCKADDR_IN addrSrv;
    __int64 iResult;

//    if (mWSAInitialized == FALSE) {
//        UpdateMessage(_T("Server Startup failed!"));
//        return sockClient;
//    }

    addrSrv.sin_addr.S_un.S_addr = inet_addr(ip_addr);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //int timeout=50000;
    //iResult=setsockopt(sockClient,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(int));
    //if(SOCKET_ERROR==iResult)
    //    return INVALID_SOCKET;

    iResult = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    if(iResult==SOCKET_ERROR) {
        closesocket(sockClient);
        LOGE("Connect device %s failed!", ip_addr);
        sockClient = INVALID_SOCKET;
    }
    return sockClient;
}

int kill_adb_server(int port )
{
  struct sockaddr_in addr;
  SOCKET  s;
  int type = SOCK_STREAM;

  if (!_winsock_init)
    _init_winsock();

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  s = socket(AF_INET, type, 0);
  if(s == INVALID_SOCKET) {
    DEBUG("kill_adb_server: could not create socket\n" );
    return -1;
  }

  if(connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    DEBUG("kill_adb_server: could not connect to %s:%d", type != SOCK_STREAM ? "udp" : "tcp", port );
    return -1;
  }
  //snprintf( f->name, sizeof(f->name), "%d(lo-client:%s%d)", _fh_to_int(f), type != SOCK_STREAM ? "udp:" : "", port );
  //D( "socket_loopback_client: port %d type %s => fd %d", port, type != SOCK_STREAM ? "udp" : "tcp", _fh_to_int(f) );
  // return _fh_to_int(f);
  const char *service = "host:kill";
  char tmp[5];
  int len= strlen(service);

  DEBUG("kill_adb_server: %s", service);

  snprintf(tmp, sizeof tmp, "%04x", len);
  send(s, tmp, 4, 0);
  send(s, service, len, 0);

  shutdown( s, SD_BOTH );
  closesocket( s );
  _cleanup_winsock();
  return  0;
}

DWORD FindProcess(wchar_t *strProcessName, CString &AppPath)
{
	DWORD aProcesses[1024], cbNeeded, cbMNeeded;
	HMODULE hMods[1024];
	HANDLE hProcess;
	wchar_t szProcessName[MAX_PATH];

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )  return 0;
	for(int i=0; i< (int) (cbNeeded / sizeof(DWORD)); i++)
	{
		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
		EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbMNeeded);
		GetModuleFileNameEx( hProcess, hMods[0], szProcessName,sizeof(szProcessName));

		if(wcsstr(szProcessName, strProcessName))
		{
			AppPath = szProcessName;
			return(aProcesses[i]);
		}
	}
	return 0;
}

BOOL StopAdbServer(){
	int iTemp = 0;
	DWORD adbProcID;
	CString adbPath;
	adbProcID = FindProcess(L"adb.exe", adbPath);
	if (0 != adbProcID)
	{
		//stop adb;
		//If the function succeeds, the return value is greater than 31.
		iTemp = WinExec(adbPath + " kill-server", SW_HIDE);
		if (31 < iTemp)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}
