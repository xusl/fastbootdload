/*
 * Copyright (C) 2007 The Android Open Source Project
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

/* this file contains system-dependent definitions used by ADB
 * they're related to threads, sockets and file descriptors
 */
#ifndef _ADB_SYSDEPS_H
#define _ADB_SYSDEPS_H


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

#ifdef __cplusplus
extern "C" {
#endif

//xusl add
#define __inline__
typedef int socklen_t;
#ifndef snprintf
#define snprintf _snprintf
#endif
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
//linux define in file unistd.h
#define F_OK 0 /* Test for existence. */

#define OS_PATH_SEPARATOR '\\'
#define OS_PATH_SEPARATOR_STR "\\"

typedef CRITICAL_SECTION          adb_mutex_t;
//typedef int          adb_mutex_t;
#define  ADB_MUTEX_DEFINE(x)     adb_mutex_t   x

/* declare all mutexes */
#define  ADB_MUTEX(x)   extern adb_mutex_t  x;
#include "mutex_list.h"

//extern
void  adb_sysdeps_init(void);

static __inline__ void adb_mutex_lock( adb_mutex_t*  lock )
{
    EnterCriticalSection( lock );
}

static __inline__ void  adb_mutex_unlock( adb_mutex_t*  lock )
{
    LeaveCriticalSection( lock );
}

typedef struct __adb_thread{ unsigned  tid; }  adb_thread_t;

//typedef  void*  (*adb_thread_func_t)(void*  arg);

//typedef  void (*win_thread_func_t)(void*  arg);

typedef AFX_THREADPROC adb_thread_func_t;
typedef AFX_THREADPROC win_thread_func_t;

static __inline__ int  adb_thread_create( adb_thread_t  *thread, adb_thread_func_t  func, void*  arg)
{
	/*
    thread->tid = _beginthread( (win_thread_func_t)func, 0, arg );
    if (thread->tid == (unsigned)-1L) {
        return -1;
    }
	*/
	CWinThread* tp = AfxBeginThread(func, arg);
	if (tp == NULL)
		return -1;

	thread->tid = tp->m_nThreadID;
    return 0;
}

static __inline__ void  close_on_exec(int  fd)
{
    /* nothing really */
}

extern void  disable_tcp_nagle(int  fd);

#define  lstat    stat   /* no symlinks on Win32 */

//#define  S_ISLNK(m)   0   /* no symlinks on Win32 */

static __inline__  int    adb_unlink(const char*  path)
{
    int  rc = unlink(path);

    if (rc == -1 && errno == EACCES) {
        /* unlink returns EACCES when the file is read-only, so we first */
        /* try to make it writable, then unlink again...                  */
        rc = chmod(path, _S_IREAD|_S_IWRITE );
        if (rc == 0)
            rc = unlink(path);
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

extern int  adb_open(const char*  path, int  options);
extern int  adb_creat(const char*  path, int  mode);
extern int  adb_read(int  fd, void* buf, int len);
extern int  adb_write(int  fd, const void*  buf, int  len);
extern int  adb_lseek(int  fd, int  pos, int  where);
extern int  adb_shutdown(int  fd);
extern int  adb_close(int  fd);

static __inline__ int  unix_close(int fd)
{
    return close(fd);
}
#undef   close
#define  close   ____xxx_close

static __inline__  int  unix_read(int  fd, void*  buf, size_t  len)
{
    return read(fd, buf, len);
}
#undef   read
#define  read  ___xxx_read

static __inline__  int  unix_write(int  fd, const void*  buf, size_t  len)
{
    return write(fd, buf, len);
}
#undef   write
#define  write  ___xxx_write

static __inline__ int  adb_open_mode(const char* path, int options, int mode)
{
    return adb_open(path, options);
}

static __inline__ int  unix_open(const char*  path, int options,...)
{
    if ((options & O_CREAT) == 0)
    {
        return  open(path, options);
    }
    else
    {
        int      mode;
        va_list  args;
        va_start( args, options );
        mode = va_arg( args, int );
        va_end( args );
        return open(path, options, mode);
    }
}
#define  open    ___xxx_unix_open


/* normally provided by <cutils/misc.h> */
extern void*  load_file(const char*  pathname, unsigned*  psize);

/* normally provided by <cutils/sockets.h> */
extern int socket_loopback_client(int port, int type);
extern int socket_network_client(const char *host, int port, int type);
extern int socket_loopback_server(int port, int type);
extern int socket_inaddr_any_server(int port, int type);

/* normally provided by "fdevent.h" */

#define FDE_READ              0x0001
#define FDE_WRITE             0x0002
#define FDE_ERROR             0x0004
#define FDE_DONT_CLOSE        0x0080

typedef struct fdevent fdevent;

typedef void (*fd_func)(int fd, unsigned events, void *userdata);

fdevent *fdevent_create(int fd, fd_func func, void *arg);
void     fdevent_destroy(fdevent *fde);
void     fdevent_install(fdevent *fde, int fd, fd_func func, void *arg);
void     fdevent_remove(fdevent *item);
void     fdevent_set(fdevent *fde, unsigned events);
void     fdevent_add(fdevent *fde, unsigned events);
void     fdevent_del(fdevent *fde, unsigned events);
void     fdevent_loop();

struct fdevent {
    fdevent *next;
    fdevent *prev;

    int fd;
    unsigned short state;
    unsigned short events;

    fd_func func;
    void *arg;
};

static __inline__ void  adb_sleep_ms( int  mseconds )
{
    Sleep( mseconds );
}

extern int  adb_socket_accept(int  serverfd, struct sockaddr*  addr, socklen_t  *addrlen);

#undef   accept
#define  accept  ___xxx_accept

static __inline__  int  adb_socket_setbufsize( int   fd, int  bufsize )
{
    int opt = bufsize;
    return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(opt));
}

extern int  adb_socketpair( int  sv[2] );

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


#ifdef __cplusplus
}
#endif


#endif /* _ADB_SYSDEPS_H */
