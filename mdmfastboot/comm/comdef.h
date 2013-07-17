/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-04-02  dawang.xu  Add feature.h in case the common definition might be
                       changed according to some features; add definition for
                       KB and MB.
2009-02-06  dawang.xu  Init first version

=============================================================================*/
#ifndef __COMDEF_H__
#define __COMDEF_H__

#include "feature.h"

typedef signed char        int8;
typedef unsigned char      uint8;
typedef signed short       int16;
typedef unsigned short     uint16;
typedef signed int         int32;
typedef unsigned int       uint32;
typedef signed long        int64;
typedef unsigned long      uint64;
typedef unsigned char      byte;
typedef unsigned short     word;
typedef unsigned long      dword;

/* Indicator of the params */
#define IN
#define OUT

#define OK 0
#define SUCCESS(result) ((result == OK) ? TRUE : FALSE)
#define FAILURE(result) ((result != OK) ? TRUE : FALSE)

/* Data size unit */
#define KB (1024)	   // kilobyte
#define MB (1024*1024) // megabyte


#endif //__COMDEF_H__

