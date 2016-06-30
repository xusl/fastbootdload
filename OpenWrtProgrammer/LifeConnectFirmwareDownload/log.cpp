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
#include <stdafx.h>
#include "log.h"
#include <stdio.h>
#include <process.h>//for getpid()
#include <windows.h>
//#define FEATURE_THREAD_SYNC
//-----------------------------------------------------------------------------
#ifdef FEATURE_THREAD_SYNC
#include <afxmt.h>
CCriticalSection g_Lock;
#endif
static unsigned int     mask = 0;
static unsigned int     adb_trace_mask = 0;
static FILE* gLogFp = NULL;

static void log_tags_init(const char* tags);
static void log_level_init(const char* p);

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

void WriteLog
(
    AdbTrace tag,
    TLogMaskEnumType type,
	const char* msg,
	const char* fmtstr,
	...
)
{
#define MAX_BUF_LEN     (4096 * 8)
  int   nBuf;
  char  szBuffer[MAX_BUF_LEN] = {0};

#define FORMAT_SIZE     (256*4)
  char  szFormat[FORMAT_SIZE] = {0};
  //      char timestr[40];
   // time_t clk = time( NULL );
    SYSTEMTIME time;
  va_list args;
  if (!(mask & type))
  {
    return;
  }

  if(!((adb_trace_mask & (1 << tag)) != 0)) {
    return;
  }

  if (fmtstr == NULL || msg == NULL)
  {
    return;
  }
  va_start(args, fmtstr);

  GetLocalTime(&time);
  nBuf = _snprintf(szFormat, COUNTOF(szFormat), "%02d:%02d:%02d %6s %s\n",
                   time.wHour, time.wMinute,time.wSecond, msg, fmtstr);

#ifdef FEATURE_THREAD_SYNC
  g_Lock.Lock();
#endif
  vfprintf(gLogFp, szFormat, args);
  va_end(args);
  fflush(gLogFp);
  //ASSERT(nBuf >= 0);

#ifdef FEATURE_THREAD_SYNC
  g_Lock.Unlock();
#endif
}


/* read a comma/space/colum/semi-column separated list of tags
 * from the ADB_TRACE environment variable and build the trace
 * mask from it. note that '1' and 'all' are special cases to
 * enable all tracing
 */
static void log_tags_init(const char*  p)
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
        { "fb", TRACE_PROGRAMMER },
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
static void log_level_init(const char*  p)
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


void StartLogging(char * filename,
	const char* mask,
	const char* tags) {
    if (gLogFp != NULL)
        return;

    log_tags_init(tags);
	log_level_init(mask);

    gLogFp = fopen( filename, "a" );
	LOGI("=================== Start logging =================");
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
    if (gLogFp != NULL)
        fclose(gLogFp);
        gLogFp = NULL;
}


//-----------------------------------------------------------------------------


