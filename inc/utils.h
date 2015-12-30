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
#ifndef _ADB_UTILS_H
#define _ADB_UTILS_H
//In vc9.0 include winsock2.h before windows.h
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
//#include <WinSock.h>
#include <process.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <direct.h>
#include <afxwin.h>

#  define DEFAULT_ADB_PORT 5037
#  define MILLS_SECONDS (1000 * 1000)

#define __inline__
typedef int socklen_t;

#ifndef snprintf
#define snprintf _snprintf
#endif

#define OS_PATH_SEPARATOR '\\'
#define OS_PATH_SEPARATOR_STR "\\"

#define FREE_IF(x)  do {\
                      if ((x) != NULL) {\
                        free((x));\
                        (x) = NULL;\
                      } \
                    }while(0)



/* Get array element count
 */
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef CRITICAL_SECTION          adb_mutex_t;

#define  ADB_MUTEX_DEFINE(x)     adb_mutex_t   x

/* declare all mutexes */
//#define  ADB_MUTEX_DECLARE(x)   extern adb_mutex_t  x;


#define  ADB_MUTEX(x)  InitializeCriticalSection( & x );


static __inline__ void adb_mutex_lock( adb_mutex_t*  lock )
{
    EnterCriticalSection( lock );
}

static __inline__ void  adb_mutex_unlock( adb_mutex_t*  lock )
{
    LeaveCriticalSection( lock );
}

static __inline__ void  adb_sleep_ms( int  mseconds )
{
    Sleep( mseconds );
}


#define  lstat    stat   /* no symlinks on Win32 */

//#define  S_ISLNK(m)   0   /* no symlinks on Win32 */

static __inline__  int    adb_unlink(const char*  path)
{
    int  rc = _unlink(path);

    if (rc == -1 && errno == EACCES) {
        /* unlink returns EACCES when the file is read-only, so we first */
        /* try to make it writable, then unlink again...                  */
        rc = _chmod(path, _S_IREAD|_S_IWRITE );
        if (rc == 0)
            rc = _unlink(path);
    }
    return rc;
}
#undef  unlink
#define unlink  ___xxx_unlink

static __inline__ int  adb_mkdir(const char*  path, int mode)
{
	return _mkdir(path);
}
#undef   mkdir
#define  mkdir  ___xxx_mkdir


static __inline__  const char*  adb_dirstart( const char*  path )
{
    const char*  p  = strchr(path, '/');
    const char*  p2 = strchr(path, '\\');

    if ( !p )
        p = p2;
    else if ( p2 && p2 > p )
        p = p2;

    return p;
}

static __inline__  const char*  adb_dirstop( const char*  path )
{
    const char*  p  = strrchr(path, '/');
    const char*  p2 = strrchr(path, '\\');

    if ( !p )
        p = p2;
    else if ( p2 && p2 > p )
        p = p2;

    return p;
}

static __inline__  int  adb_is_absolute_host_path( const char*  path )
{
    return isalpha(path[0]) && path[1] == ':' && path[2] == '\\';
}

/* bounded buffer functions */

/* all these functions are used to append data to a bounded buffer.
 *
 * after each operation, the buffer is guaranteed to be zero-terminated,
 * even in the case of an overflow. they all return the new buffer position
 * which allows one to use them in succession, only checking for overflows
 * at the end. For example:
 *
 *    BUFF_DECL(temp,p,end,1024);
 *    char*    p;
 *
 *    p = buff_addc(temp, end, '"');
 *    p = buff_adds(temp, end, string);
 *    p = buff_addc(temp, end, '"');
 *
 *    if (p >= end) {
 *        overflow detected. note that 'temp' is
 *        zero-terminated for safety.
 *    }
 *    return strdup(temp);
 */

/* tries to add a character to the buffer, in case of overflow
 * this will only write a terminating zero and return buffEnd.
 */
char*   buff_addc (char*  buff, char*  buffEnd, int  c);

/* tries to add a string to the buffer */
char*   buff_adds (char*  buff, char*  buffEnd, const char*  s);

/* tries to add a bytes to the buffer. the input can contain zero bytes,
 * but a terminating zero will always be appended at the end anyway
 */
char*   buff_addb (char*  buff, char*  buffEnd, const void*  data, int  len);

/* tries to add a formatted string to a bounded buffer */
char*   buff_add  (char*  buff, char*  buffEnd, const char*  format, ... );

/* convenience macro used to define a bounded buffer, as well as
 * a 'cursor' and 'end' variables all in one go.
 *
 * note: this doesn't place an initial terminating zero in the buffer,
 * you need to use one of the buff_ functions for this. or simply
 * do _cursor[0] = 0 manually.
 */
#define  BUFF_DECL(_buff,_cursor,_end,_size)   \
    char   _buff[_size], *_cursor=_buff, *_end = _cursor + (_size)

const char * basename(const char * f_name);
CString GetFileNameFromFullPath(CString FullPath);
void get_my_path(char *s, size_t maxLen);
CString GetAppPath(CString & sPath );
void sleep(int seconds);
long long now(void);
 PWCH MultiStrToWideStr(PCCH pc);
PCHAR WideStrToMultiStr(PCWCH WideStr);


/* normally provided by <cutils/misc.h> */
extern void* load_file(LPCWSTR pathname, unsigned*  psize);
int kill_adb_server(int port );
#endif /* _ADB_UTILS_H */
