#ifndef UTILS_H
#define UTILS_H

#include "../define/stdafx.h"
#include "StringTable.h"

QString GetAppPath();
const QString&GetCurrentPath();
extern bool g_bGenerateFTFilesValue;
extern QWaitCondition chickMessBox;

// Get array element count
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))
// Get a mask with nth bits to be 1 
#define LEFT_SHIFT_BITS(n) (1 << (n))

#ifndef Q_OS_WIN32
    int EjectScsi(int fd);
#endif

#endif // UTILS_H
