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

#include "comdef.h"
#include "utils.h"

#ifdef FEATURE_LOG_FILE
#include <iostream>
#include <fstream>

using std::ios_base;
using std::ofstream;
using std::endl;
#endif //FEATURE_LOG_FILE

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
	LOG_MASK_ALL         = LEFT_SHIFT_BITS(6) - 1,

	LOG_MASK_MEMDUMP     = LEFT_SHIFT_BITS(7), // for memory allocate/free info

	LOG_MASK_MAX         = LEFT_SHIFT_BITS(31),
} TLogMaskEnumType;

/* IMPORTANT: if you change the following list, don't
 * forget to update the corresponding 'tags' table in
 * the adb_trace_init() function implemented in adb.c
 */
typedef enum {
    TRACE_ADB = 0,
    TRACE_SOCKETS,
    TRACE_PACKETS,
    TRACE_TRANSPORT,
    TRACE_RWX,
    TRACE_USB,
    TRACE_SYNC,
    TRACE_SYSDEPS,
    TRACE_JDWP,
} AdbTrace;


#ifdef FEATURE_LOG_FUNC_NAME
#define TRACE_FMT "    %20s  %04d %20s    "
#define LOG_TRACE basename(__FILE__),__LINE__,__FUNCTION__
#else
#define TRACE_FMT "   %20s  %04d    "
#define LOG_TRACE basename(__FILE__),__LINE__
#endif

//-----------------------------------------------------------------------------

class CLog
{
public:
	~CLog();

public:
	void StartLogging(const wchar_t* logname, unsigned int mask = LOG_MASK_ALL);

	void Debug(const char* fmtstr, ...);
	void Log(const char* fmtstr, ...);
	void Info(const char* fmtstr, ...);
	void Warn(const char* fmtstr, ...);
	void Error(const char* fmtstr, ...);
	void Critical(const char* fmtstr, ...);
	void Memdump(const char* fmtstr, ...);
	int AdbTraceMask();

public:
	static CLog* GetInstance(void);

private:
	CLog();
	void WriteLog(const char* msg, const char* fmtstr, va_list& args);

  void adb_trace_init(void);

private:
	static CLog* pLogInstance;

private:

#ifdef FEATURE_LOG_FILE
	ofstream         stream;
#endif

	unsigned int     mask;
	unsigned int     adb_trace_mask;
	std::locale		   loc;
};

extern CLog* g_pLogInstance;
//-----------------------------------------------------------------------------

#undef DEBUG
#undef LOG
#undef INFO
#undef WARN
#undef ERROR
#undef CRITICAL
#undef MEMDUMP

//-----------------------------------------------------------------------------
/* For external call
*/
#ifdef FEATURE_LOG_SYS
#define DEBUG(fmt, ...)			g_pLogInstance->Debug(TRACE_FMT# fmt,  LOG_TRACE, __VA_ARGS__)
#define INFO(fmt, ...)				g_pLogInstance->Info(TRACE_FMT# fmt, LOG_TRACE, __VA_ARGS__)
#define LOG(fmt, ...)				g_pLogInstance->Log(TRACE_FMT# fmt, LOG_TRACE, __VA_ARGS__)
#define WARN(fmt, ...)			g_pLogInstance->Warn(TRACE_FMT# fmt, LOG_TRACE, __VA_ARGS__)
#define ERROR(fmt, ...)			g_pLogInstance->Error(TRACE_FMT# fmt, LOG_TRACE, __VA_ARGS__)
#define CRITICAL(fmt, ...)		g_pLogInstance->Critical(TRACE_FMT# fmt, LOG_TRACE, __VA_ARGS__)
#define MEMDUMP(fmt, ...)  g_pLogInstance->Memdump(TRACE_FMT# fmt, LOG_TRACE, __VA_ARGS__)
#else // !FEATURE_LOG_SYS
#define DEBUG(fmt, ...)
#define INFO(fmt, ...)
#define LOG(fmt, ...)
#define WARN(fmt, ...)
#define ERROR(fmt, ...)
#define CRITICAL(fmt, ...)
#define MEMDUMP(fmt, ...)
#endif // FEATURE_LOG_SYS

/* define ADB_TRACE to 1 to enable tracing support, or 0 to disable it */

#define  ADB_TRACE    1

#if ADB_TRACE
#define ADB_TRACING  ((g_pLogInstance->AdbTraceMask() & (1 << TRACE_TAG)) != 0)

  /* you must define TRACE_TAG before using this macro */
  #define  D(...)                                      \
        do {                                           \
            if (ADB_TRACING)                           \
                fprintf(stderr, __VA_ARGS__ );         \
        } while (0)
#else
#define  D(...)          ((void)0)
#define  ADB_TRACING     0
#endif


#if !TRACE_PACKETS
#define print_packet(tag,p) do {} while (0)
#else
void print_packet(const char *label, apacket *p);

#endif
/* util stuff */
void die(const char *fmt, ...);
void fatal(const char *fmt, ...);
void fatal_errno(const char *fmt, ...);

/*redirect stderr to adb.log*/
void RedirectStdIO(void);

void StartLogging(const wchar_t* fname, unsigned int mask = LOG_MASK_ALL);
void StartLogging(void);
void StopLogging(void);

//-----------------------------------------------------------------------------

#endif //__LOG_H__

