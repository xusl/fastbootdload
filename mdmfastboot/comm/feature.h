/*=============================================================================
DESCRIPTION:
	Define global features to allow function configuration for downloading.
	This is a top header file and should not include any other files.

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2010-02-10  jianwen.he  Init first version
=============================================================================*/
#ifndef __FEATURE_H__
#define __FEATURE_H__

#include "target.h"

/*User level define*/
#ifdef FEATURE_TPST
	#define FEATURE_USER_LEVEL2
#endif

#ifdef FEATURE_ADSU
	#define FEATURE_USER_LEVEL1
#endif
/*Enable logging system*/
#define FEATURE_LOG_SYS

/*Support multi file download*/
//#define FEATURE_MULTI_FILE

/*Support IMG file download*/
#define FEATURE_IMG_FILE

/*Support logging to disk file*/
#ifdef FEATURE_LOG_SYS
#define FEATURE_LOG_FILE
#endif

#ifdef FEATURE_LOG_SYS
#define FEATURE_THREAD_SYNC
#endif

/* VC6 doesn't support for __FUNCTION__ */
#if (_MSC_VER > 1200)
#define FEATURE_LOG_FUNC_NAME
#endif

/* When the partition table is different, whether the download
 * still be supported
 */
#define FEATURE_PRTNTBL_DIFF_DLOAD

#endif //__FEATURE_H__

