/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-04-10  dawang.xu  Add LOG_MASK_MEMDUMP and related API for memory dump.
2009-04-01  dawang.xu  Define DEBUG/INFO/LOG/WARN/ERROR/CRITICAL as empty when
                       FEATURE_LOG_SYS is not defined;
                       Log caller function name if compiler version supports.
2009-03-12	dawang.xu  Add log mask for configuration and add external call interface
2009-02-18  dawang.xu  Init first version

=============================================================================*/
#ifndef __LOG_H__
#define __LOG_H__

//#include "comdef.h"
#include "header.h"
#include "stdio.h"
#include "string.h"
#include  "stdlib.h"
#include "stdarg.h"
//#include "utils.h"
#define FEATURE_LOG_FILE
#define FEATURE_LOG_SYS
#define FEATURE_LOG_FUNC_NAME

const char * basename(const char * f_name);
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
//-----------------------------------------------------------------------------
/* Get a mask with nth bits to be 1 */
#define LEFT_SHIFT_BITS(n) (1 << (n))

typedef enum {
	LOG_MASK_NONE        = 0,
	LOG_MASK_DEBUG       = LEFT_SHIFT_BITS(0),
	LOG_MASK_LOG         = LEFT_SHIFT_BITS(1),
	LOG_MASK_INFO        = LEFT_SHIFT_BITS(2),
	LOG_MASK_WARNING     = LEFT_SHIFT_BITS(3),
	LOG_MASK_ERROR       = LEFT_SHIFT_BITS(4),
	LOG_MASK_CRITICAL    = LEFT_SHIFT_BITS(5),
	LOG_MASK_MEMDUMP     = LEFT_SHIFT_BITS(6), // for memory allocate/free info
	LOG_MASK_ALL         = LEFT_SHIFT_BITS(7) - 1,
	LOG_MASK_MAX         = LEFT_SHIFT_BITS(31),
} TLogMaskEnumType;

/* IMPORTANT: if you change the following list, don't
 * forget to update the corresponding 'tags' table in
 * the adb_trace_init() function implemented in adb.c
 */
typedef enum {
    TRACE_ADB = 0,
    TRACE_PACKETS,
    TRACE_TRANSPORT,
    TRACE_RWX,
    TRACE_USB,
    TRACE_SYNC,
    TRACE_PROGRAMMER,
} AdbTrace;


#ifdef FEATURE_LOG_FUNC_NAME
#define TRACE_FMT " %20s %04d %20s "
#define LOG_TRACE basename(__FILE__),__LINE__,__FUNCTION__
#else
#define TRACE_FMT " %20s %04d "
#define LOG_TRACE basename(__FILE__),__LINE__
#endif

//-----------------------------------------------------------------------------
void WriteLog(AdbTrace tag, TLogMaskEnumType type, const char* msg, const char* fmtstr, ...);


//-----------------------------------------------------------------------------
#undef DEBUG
#undef LOG
#undef INFO
#undef WARN
#undef ERROR
#undef CRITICAL
#undef MEMDUMP

#ifndef TRACE_TAG
#define TRACE_TAG TRACE_ADB
#endif
//-----------------------------------------------------------------------------
/* For external call
*/
#ifdef FEATURE_LOG_SYS
#define LOGD(fmt, ...)			WriteLog(TRACE_TAG,LOG_MASK_DEBUG, "DBG", TRACE_FMT## fmt, LOG_TRACE, __VA_ARGS__)
#define LOGI(fmt, ...)				WriteLog(TRACE_TAG,LOG_MASK_INFO, "INFO", TRACE_FMT## fmt, LOG_TRACE, __VA_ARGS__)
#define LOG(fmt, ...)				WriteLog(TRACE_TAG,LOG_MASK_LOG, "LOG", TRACE_FMT## fmt, LOG_TRACE, __VA_ARGS__)
#define LOGW(fmt, ...)			WriteLog(TRACE_TAG,LOG_MASK_WARNING, "WARN",TRACE_FMT## fmt, LOG_TRACE, __VA_ARGS__)
#define LOGE(fmt, ...)			WriteLog(TRACE_TAG,LOG_MASK_ERROR, "ERR", TRACE_FMT## fmt, LOG_TRACE, __VA_ARGS__)
#else // !FEATURE_LOG_SYS
#define LOGD(fmt, ...)
#define LOGI(fmt, ...)
#define LOG(fmt, ...)
#define LOGW(fmt, ...)
#define LOGE(fmt, ...)
#endif // FEATURE_LOG_SYS

 void LIBPROGRAMMER StartLogging(char* fname, const char* mask, const char* tags);
 void LIBPROGRAMMER StopLogging(void);

//-----------------------------------------------------------------------------

#endif //__LOG_H__