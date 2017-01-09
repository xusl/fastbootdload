//#include "StdAfx.h"
#include "log.h"
//#include "varargs.h"
#include <stdarg.h>

/* Get array element count 
 */
#ifndef COUNTOF
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

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

CLog::CLog()
{

}

CLog::~CLog()
{
	std::locale::global(loc);//恢复全局locale
	logFile.close();
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
        const char* fname
)
{
	loc = std::locale::global(std::locale("")); //要打开的文件路径含中文，设置全局locale为本地环境	
    //logFile(QString(QLatin1String(fname)));

    logFile.setFileName(fname);
    if(!logFile.open(QIODevice::ReadWrite | QIODevice::Text))
	{
        logFile.close();
        return;
	}
}

void CLog::Debug
(
 const char* f_name, 
 uint32		  line_no, 
 const char* fmtstr,
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_DEBUG, f_name, line_no, fmtstr, args);
    va_end(args);
}

void CLog::Log
(
 const char* f_name, 
 uint32		  line_no, 
 const char* fmtstr
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_LOG, f_name, line_no, fmtstr, args);
    va_end(args);
}

void CLog::Error
(
 const char* f_name, 
 uint32		  line_no, 
 const char* fmtstr,
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_ERROR, f_name, line_no, fmtstr, args);
    va_end(args);
}

void CLog::Warn
(
 const char* f_name, 
 uint32		  line_no, 
 const char* fmtstr,
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_WARNING, f_name, line_no, fmtstr, args);
    va_end(args);
}

void CLog::Critical
(
 const char* f_name, 
 uint32		  line_no, 
 const char* fmtstr,
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_CRITICAL, f_name, line_no, fmtstr, args);
    va_end(args);
}

void CLog::Info
(
 const char* f_name, 
 uint32		 line_no, 
 const char* fmtstr,
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_INFO, f_name, line_no, fmtstr, args);
    va_end(args);
}

void CLog::Memdump
(
 const char* f_name, 
 uint32		  line_no, 
 const char* fmtstr,
 ...
 )
{
    va_list args;
    va_start(args, fmtstr);
    WriteLog(LOG_MASK_MEMDUMP, f_name, line_no, fmtstr, args);
    va_end(args);
}


void CLog::WriteLog
(
 TLogMaskEnumType mask,
 const char*      f_name, 
 uint32		      line_no, 
 const char*      fmtstr,
 va_list          list
 )
{
    QMutexLocker locker(&mutex);

	uint32  len = strlen(f_name);

	char* msg = "";
	switch (mask) 
	{
	case LOG_MASK_DEBUG:
		msg = "DEBUG";
		break;
	case LOG_MASK_LOG:
		msg = "LOG";
		break;
	case LOG_MASK_INFO:
		msg = "INFO";
		break;
	case LOG_MASK_WARNING:
		msg = "WARNING";
		break;
	case LOG_MASK_ERROR:
		msg = "ERROR";
		break;
	case LOG_MASK_CRITICAL:
		msg = "CRITICAL";
		break;
	case LOG_MASK_MEMDUMP:
		msg = "MEMDUMP";
		break;
	default:
		msg = "NONE";
		break;
	}


    if(!logFile.isOpen())
    {
        logFile.open(QIODevice::ReadWrite | QIODevice::Text);
    }

    /*uint32 size = logFile.size();
    logFile.seek(size);
    char buf[256] = {0};
    sprintf(buf, "%s: %s(%d): %s\r\n", msg, f_name, line_no, fmtstr);
    logFile.write(buf, strlen(buf));*/

    QByteArray aTime = QTime::currentTime().toString().toLatin1();

    char *time = aTime.data();


    /*QString aTime = QTime::currentTime().toString();
    const char* time = aTime.toStdString().c_str();*/
    uint32 size = logFile.size();
    logFile.seek(size);
    char buf[256] = {0};
    char buf1[256] = {0};
    sprintf(buf, "%s:%s  %s(%d):----", msg, time, f_name, line_no);
    vsprintf(buf1, fmtstr, list);
    strcat(buf,buf1);
    strcat(buf,"\n");
    logFile.write(buf);
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

void StartLogging
(
        const char* fname
)
{	
	g_pLogInstance = CLog::GetInstance();
	g_pLogInstance->StartLogging(fname);
	INFO(FILE_LINE, "===== Start logging =====");
}

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
	INFO(FILE_LINE, "***** Stop logging *****");
#ifdef FEATURE_LOG_SYS
	delete g_pLogInstance;
	g_pLogInstance = NULL;
#endif
}

//-----------------------------------------------------------------------------


