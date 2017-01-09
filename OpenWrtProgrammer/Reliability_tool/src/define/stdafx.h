#include <QtGui>
#include <QtCore>

#include "define.h"

#ifdef Q_OS_WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define _AFXDLL

#define SLEEP(a) Sleep(a)

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxres.h>
#include <Winreg.h>
#include <windows.h>
#else

//#include <syswait.h>
#ifdef Q_OS_MAC
    #define SLEEP(a) sleep(a/1000);
#endif


#endif //Q_OS_WIN32
