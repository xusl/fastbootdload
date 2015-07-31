/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2013-07-10  xusl       Reimplement All API.
2009-04-01  dawang.xu  Add FEATURE_LOG_SYS for log printing functions body;
                       log caller function name if compiler version supports.
2009-03-26  dawang.xu  Change printed filename from 16 to 20 chars
2009-03-12  dawang.xu  Re-implement log instance to allow configuration
2009-02-18  dawang.xu  Init first version

=============================================================================*/

#include "StdAfx.h"

#include "log.h"
#include <process.h>//for getpid()
//-----------------------------------------------------------------------------

#ifdef FEATURE_THREAD_SYNC
#include <afxmt.h>
CCriticalSection g_Lock;
#endif

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------


/* global variables
*/
CLog* g_pLogInstance = NULL;

//-----------------------------------------------------------------------------

/* static variables
*/
CLog* CLog::pLogInstance = NULL;

CLog* CLog::GetInstance()
{
	if (pLogInstance == NULL)
	{
		pLogInstance = new CLog;
	}
	return pLogInstance;
}

//-----------------------------------------------------------------------------

CLog::CLog() : mask (LOG_MASK_NONE)
{
}

CLog::~CLog()
{
	std::locale::global(loc);
	INFO("***** Stop logging *****");
#ifdef FEATURE_LOG_FILE
	if (this->stream.is_open())
	{
		this->stream.close();
	}
	CLog::pLogInstance = NULL;
#endif
}

void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr,"\n");
    va_end(ap);
    exit(1);
}


void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(-1);
}

void fatal_errno(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: %s: ", strerror(errno));
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(-1);
}

/*===========================================================================
DESCRIPTION
	Start logging message to App and/or local disk file according to the
	logging mask.

DEPENDENCIES
	None

RETURN VALUE
	None

SIDE EFFECTS
	Logging file will be open if file logging supported. This cannot be called
	again before StopLogging is called, otherwise it takes no effect.

===========================================================================*/

void CLog::StartLogging
(
	const wchar_t* fname,
	const char* mask,
	const char* tags
)
{
	loc = std::locale::global(std::locale(""));


#ifdef FEATURE_LOG_FILE
	/* If the stream has been opened, do nothing */
	if (this->stream.is_open())
	{
		return;
	}
	this->stream.open(fname, ios_base::app);
#endif
	//this->mask = mask;

    log_tags_init(tags);
	log_level_init(mask);

	INFO("===== Start logging =====");
}


/*===========================================================================
DESCRIPTION
	This is the core implementation of logging system. It writes logging time,
	log level, filename, line number and message to App and/or local disk file.

DEPENDENCIES
	None

RETURN VALUE
	None

SIDE EFFECTS
	None

===========================================================================*/

void CLog::WriteLog
(
    AdbTrace tag,
    TLogMaskEnumType type,
	const char* msg,
	const char* fmtstr,
	...
)
{
#ifdef FEATURE_LOG_SYS

#define MAX_BUF_LEN     (4096 * 8)

  int   nBuf;
  char  szBuffer[MAX_BUF_LEN] = {0};
#define OPT_LOG
#ifndef OPT_LOG
#define FORMAT_SIZE     (256)
  char  szFormat[FORMAT_SIZE] = {0};
#endif
  if (this == NULL)
  {
    return;
  }

  if (!(this->mask & type))
  {
    return;
  }

  if(!((AdbTraceMask() & (1 << tag)) != 0)) {
    return;
  }

  if (fmtstr == NULL || msg == NULL)
  {
    return;
  }

  va_list args;
  va_start(args, fmtstr);

  SYSTEMTIME time;
  GetLocalTime(&time);

  //_snprintf(buf,MAX_BUF_LEN, "%4d-%02d-%02d %02d:%02d:%02d.%03d  %8s  %s",
  //			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute,
  //			time.wSecond, time.wMilliseconds,  msg, fmtstr);
#ifndef OPT_LOG
  nBuf = _snprintf(szFormat, COUNTOF(szFormat), "%02d:%02d:%02d %8s %s",
                   time.wHour, time.wMinute,time.wSecond, msg, fmtstr);
#else
#ifdef FEATURE_LOG_FILE
#ifdef FEATURE_THREAD_SYNC
  g_Lock.Lock();
#endif

  stream << setw(2) << time.wHour ;
  stream << ":" << setw(2) << time.wMinute ;
  stream << ":" << setw(2) << time.wSecond;
  stream << " " << setw(8) << msg;

#ifdef FEATURE_THREAD_SYNC
  g_Lock.Unlock();
#endif
#endif //FEATURE_LOG_FILE
#endif
#ifndef OPT_LOG
  if (nBuf < COUNTOF(szFormat)) {
    nBuf = _vsnprintf(szBuffer, COUNTOF(szBuffer), szFormat, args);
  } else {
    nBuf = _snprintf(szBuffer, COUNTOF(szBuffer), "Log Error, format buffer length is smaller than format.");
  }
#else
  nBuf = _vsnprintf(szBuffer, COUNTOF(szBuffer), fmtstr, args);
#endif

  va_end(args);

  //ASSERT(nBuf >= 0);


#ifdef FEATURE_LOG_FILE
#ifdef FEATURE_THREAD_SYNC
  g_Lock.Lock();
#endif
  if (nBuf == -1) {
    stream << basename(__FILE__)<< ", " <<__LINE__ << "Log Error, data buffer is smaller than data." << endl;
  } else {
    stream << szBuffer << endl;
  }
#ifdef FEATURE_THREAD_SYNC
  g_Lock.Unlock();
#endif
#endif //FEATURE_LOG_FILE


#if _DEBUG
  strcat(szBuffer, "\n");
  afxDump << szBuffer;
#endif // _DEBUG

#endif // FEATURE_LOG_SYS
}

int CLog::AdbTraceMask() {
	return adb_trace_mask;
}

/* read a comma/space/colum/semi-column separated list of tags
 * from the ADB_TRACE environment variable and build the trace
 * mask from it. note that '1' and 'all' are special cases to
 * enable all tracing
 */
void CLog::log_tags_init(const char*  p)
{
    //const char*  p = getenv("ADB_TRACE");
    const char*  q;

    static const struct {
        const char*  tag;
        int           flag;
    } tags[] = {
        { "1", 0 },
        { "all", 0 },
        { "adb", TRACE_ADB },
        { "packets", TRACE_PACKETS },
        { "rwx", TRACE_RWX },
        { "usb", TRACE_USB },
        { "sync", TRACE_SYNC },
        { "transport", TRACE_TRANSPORT },
        { "fb", TRACE_FB },
        { NULL, 0 }
    };

    if (p == NULL) {
#ifdef _DEBUG
            adb_trace_mask = ~0;
#endif
            return;
	}

    /* use a comma/column/semi-colum/space separated list */
    while (*p) {
        int  len, tagn;

        q = strpbrk(p, " ,:;");
        if (q == NULL) {
            q = p + strlen(p);
        }
        len = q - p;

        for (tagn = 0; tags[tagn].tag != NULL; tagn++)
        {
            int  taglen = strlen(tags[tagn].tag);

            if (len == taglen && !memcmp(tags[tagn].tag, p, len) )
            {
                int  flag = tags[tagn].flag;
                if (flag == 0) {
                    adb_trace_mask = ~0;
                    return;
                }
                adb_trace_mask |= (1 << flag);
                break;
            }
        }
        p = q;
        if (*p)
            p++;
    }
}


/* read a comma/space/colum/semi-column separated list of tags
 * from the ADB_TRACE environment variable and build the trace
 * mask from it. note that '1' and 'all' are special cases to
 * enable all tracing
 */
void CLog::log_level_init(const char*  p)
{
    const char*  q;

    static const struct {
        const char*  tag;
        int           flag;
    } tags[] = {
        { "1", 0 },
        { "all", LOG_MASK_ALL },
        { "debug", LOG_MASK_DEBUG },
        { "log", LOG_MASK_LOG },
        { "info", LOG_MASK_INFO },
        { "warn", LOG_MASK_WARNING},
        { "error", LOG_MASK_ERROR},
        { "critical", LOG_MASK_CRITICAL},
        { "memdump", LOG_MASK_MEMDUMP},
        { NULL, 0 }
    };

    if (p == NULL) {
#ifdef _DEBUG
            mask = LOG_MASK_ALL;
#endif
            return;
	}

    /* use a comma/column/semi-colum/space separated list */
    while (*p) {
        int  len, tagn;

        q = strpbrk(p, " ,:;");
        if (q == NULL) {
            q = p + strlen(p);
        }
        len = q - p;

        for (tagn = 0; tags[tagn].tag != NULL; tagn++)
        {
            int  taglen = strlen(tags[tagn].tag);

            if (len == taglen && !memcmp(tags[tagn].tag, p, len) )
            {
                int  flag = tags[tagn].flag;
                if (flag == 0) {
                    mask = LOG_MASK_ALL;
                    return;
                }
                mask |= flag;
                break;
            }
        }
        p = q;
        if (*p)
            p++;
    }
}


//把标准输出重定向到文件adb.log
void RedirectStdIO(const wchar_t *filename)
{
    char    temp[ MAX_PATH ];
    FILE*   fnul;
    FILE*   flog;

    GetTempPathA( sizeof(temp) - 8, temp );
    strcat( temp, "adb.log" );

    /* Win32 specific redirections */
    fnul = fopen( "NUL", "rt" );
    if (fnul != NULL)
        stdin[0] = fnul[0];

    if (filename == NULL)
	    flog = fopen( temp, "at" );
	else {
		char * fn = WideStrToMultiStr((PWCHAR)filename);
		flog = fopen(fn , "at" );
		delete fn;
	}

    if (flog == NULL)
        flog = fnul;

    setvbuf( flog, NULL, _IONBF, 0 );

    stdout[0] = flog[0];
    stderr[0] = flog[0];
    fprintf(stderr,"\n\n--- adb starting (pid %d) ---\n", _getpid());

}

void StartLogging(const wchar_t* fname,
	const char* mask,
	const char* tags) {
	SYSTEMTIME time;
	CString sPath = L"";
	GetLocalTime(&time);
	GetAppPath(sPath);

	wchar_t filename[MAX_PATH] = {0};
	wsprintf(filename, L"%smdmfastboot_%d%02d%02d.log", sPath.GetString(),
		time.wYear, time.wMonth, time.wDay);

	if (fname == NULL || fname == L"")
	{
		fname = filename;
	}
#ifdef FEATURE_LOG_SYS
	g_pLogInstance = CLog::GetInstance();
	g_pLogInstance->StartLogging(fname, mask, tags);
#endif

	//RedirectStdIO(fname);
}

//-----------------------------------------------------------------------------

/*===========================================================================
DESCRIPTION
	Start logging messages to App and/or local disk file.

DEPENDENCIES
	None

RETURN VALUE
	None

SIDE EFFECTS
	A singleton global CLog instance will be created.

===========================================================================*/



/*===========================================================================
DESCRIPTION
	Stop logging to App and/or local disk file.

DEPENDENCIES
	None

RETURN VALUE
	None

SIDE EFFECTS
	Global CLog instance will be released.

===========================================================================*/

void StopLogging(void)
{
#ifdef FEATURE_LOG_SYS
	delete g_pLogInstance;
	g_pLogInstance = NULL;
#endif
}


//-----------------------------------------------------------------------------


