#ifndef __LOG_H__
#define __LOG_H__

#include "../define/stdafx.h"
#define FEATURE_LOG_SYS

#define FILE_LINE __FILE__,__LINE__

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

//-----------------------------------------------------------------------------

class CLog
{
public:
	~CLog();

public:
	void StartLogging(const char* logname);
	
	void Debug(
			const char* f_name, 
			uint32       line_no,
            const char* fmtstr,
            ...
			);
	
	void Log(
			const char* f_name, 
			uint32		 line_no, 
            const char* fmtstr,
            ...
			);
	
	void Info(
			const char* f_name, 
			uint32       line_no, 
            const char* fmtstr,
            ...
			);
	
	void Warn(
			const char* f_name, 
			uint32       line_no,
            const char* fmtstr,
            ...
			);
	
	void Error(
			const char* f_name, 
			uint32		 line_no,
            const char* fmtstr,
            ...
			);
	
	void Critical(
			const char* f_name, 
			uint32       line_no, 
            const char* fmtstr,
            ...
			);

	void Memdump(
			const char* f_name,
			uint32       line_no,
            const char* fmtstr,
            ...
			);

public:
	static CLog* GetInstance(void);

private:
	CLog();
	void WriteLog(
			TLogMaskEnumType mask,
            const char*  f_name,
			uint32       line_no, 
            const char*  fmtstr,
            va_list      list
			);

private:
	static CLog* pLogInstance;

private:
	uint32           mask;
    QMutex       mutex;
	std::locale		 loc; 
    QFile        logFile;
};

extern CLog* g_pLogInstance;
//-----------------------------------------------------------------------------

//#undef DEBUG
//#undef LOG
//#undef INFO
//#undef WARN
//#undef ERROR
//#undef CRITICAL
//#undef MEMDUMP

//-----------------------------------------------------------------------------
/* For external call
*/
#ifdef FEATURE_LOG_SYS

#define DEBUG    (g_pLogInstance->Debug)
#define INFO     (g_pLogInstance->Info)
#define LOG      (g_pLogInstance->Log)
#define WARN     (g_pLogInstance->Warn)
#define ERROR    (g_pLogInstance->Error)
#define CRITICAL (g_pLogInstance->Critical)
#define MEMDUMP  (g_pLogInstance->Memdump)

#else // FEATURE_LOG_SYS

#define DEBUG
#define INFO
#define LOG
#define WARN
#define ERROR
#define CRITICAL
#define MEMDUMP

#endif // FEATURE_LOG_SYS

void StartLogging(const char* fname);
void StopLogging(void);

//-----------------------------------------------------------------------------

#endif //__LOG_H__

