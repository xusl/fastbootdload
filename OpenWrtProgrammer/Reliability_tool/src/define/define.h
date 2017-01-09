/*=============================================================================
					   *BASIC TYPE DEFINE*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-08-30  jianwen.he      Init first version

=============================================================================*/

#ifndef DEFINE_H
#define DEFINE_H

#include <QtCore>
//#include "../../feature.h"
//#include "../../typedefine.h"

#define int8		qint8
#define int16		qint16
#define int32		qint32
#define uint8		quint8
#define uint16		quint16
#define uint32		quint32
#define word		quint16
#define JRD_DIAG_PWR_KEY_MMI        1
#define JRD_DIAG_WPS_KEY_MMI        2

//#define Y900
//#define Y860
//#define Y850
//#define Y853
//#define Y854
//#define W800LZ
//#define W800LZ_TEST
//#define Y901
  //#define Y856



typedef enum {
        /* General error */
        EOK = 0,               // No Error
        EFAILED,               // General Error
        EINVALIDPARAM,         // Invalid parameters Error
        ENOMEMORY,             // Out of memory
        EBUFFERBROKEN,

        /* com port layer error */
        EPORTOPEN,             // Port Open Error
        EPORTSEND,             // Port Send Error
        EPORTSENDTIMEOUT,      // Port Send Timeout Error
        EPORTRECEIVE,          // Port Receive Error
        EPORTRECEIVETIMEOUT,   // Port Receive Timeout Error
        EPORTCLOSE,            // Port Close Error

        /* packet layer error */
        EPACKETSEND,           // Packet Send Error
        EPACKETRECEIVECRC,     // Packet Receive CRC Error
        EPACKETRECEIVETOOLARGE,// Packet Receive Too Large Error
        EPACKETRECEIVETOOSHORT,// Packet Receive Too Short Error
        EPACKETBROKEN,         // Packet Broken (overflow)
        EPACKETUNKNOWN,        // Packet Unknown (not support)

        /* command layer general error */
        ECMDSEND,
        ERSPRECEIVE,

        /* command layer EFS operation error */
        EEFSOPHELLO,
        EEFSOPSTAT,
        EEFSOPOPEN,
        EEFSOPREAD,
        EEFSOPWRITE,
        EEFSOPCLOSE,

        /* PRG download command error */
        EACKPACKET,            // can not receive a ack packte, (ACK or NAK)
        EACKFAILED,            // receive a NAK cmd

        /* Data download command error */
        EDATARSP,
        EDATACRC,

        /* Image error */
        EIMAGEFAILED,

        /* Get Device Info failed */
        EGETDEVINFO,

        /* Version */
        EVERSION,

        /* DLPrg error */
        //EDLOADNOP,            // send nop cmd error
        //EDLOADWRITE,          // send write cmd error
        //EDLOADGO,             // send go cmd error

        /* Host download error */
        EMODEUNSUPPORT,         // unknown mode
        EHOSTDLDUMMY,
        EHOSTDLHELLO,         // send hello packet error
        EHOSTDLSECMODE,       // send security mode error
        EHOSTDLPRTNTBL,       // send partition table error
        EHOSTDLPRTNTBLDIFF,   // partition table different (unmatch)
        EHOSTDLOPEN,          // send open packet error
        EHOSTDLWRITE,         // send write packet error
        EHOSTDLCLOSE,         // send close packet error

        EBACKUPNV,
        EBACKUPSIMLOCK,
        EBACKUPTRACE,
        ERESTORENV,
        ERESTORESIMLOCK,
        ERESTORETRACE,
        EHOSTDLDASHBOARD,
        EWRITETOPC,
        EATOPERTE,

        /* module layer error */
        ECUSTBACKUP,
        ECUSTBACKUPNV,
        ECUSTBACKUPSIMLOCK,
        ECUSTBACKUPTRACE,

        EDLOADPRG,

        EDLOADBOOT,
        EDLOADAMSS,
        EDLOADEFS,

        ECUSTRESTORE,
        ECUSTRESTORENV,
        ECUSTRESTORESIMLOCK,
        ECUSTRESTORETRACE,

        EDLDASHBOARD,
        EWRITEQCN,
        EWRITEXML,
        EWRITESIMLOCK,
        EERASEEFS,
        EPARTITION,
        EXMLINCORRECT,
        EMAXERROR = 0xFF,
} TResult;

/*-------------------------------------------------------------------------*/

/* Data size unit */
#define KB (1024)	   // kilobyte
//#define MB (1024*1024)    // megabyte

#ifdef FEATURE_FAST_DLOAD
#define MAX_CMD_BUFFER_LEN (10*KB) // 10KB
#else
#define MAX_CMD_BUFFER_LEN (4*KB)  // 4KB
#endif

#define MAX_RSP_BUFFER_LEN (MAX_CMD_BUFFER_LEN)

//-----------------------------------------------------------------------------
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define  B_PTR(var)  ((byte *) (void *) &(var))
#define  W_PTR(var)  ((word *) (void *) &(var))
#define  D_PTR(var)  ((dword *) (void *) &(var))

#define OK 0
#define SUCCESS(result) ((result == OK) ? true : false)
#define FAILURE(result) ((result != OK) ? true : false)


/*******************************************************/
// memery operation define

#define NEW(ptr, type)							\
do {											\
		ptr = new type;							\
	} while (0)

/* Allocate count*sizeof(type) size of memory for "ptr".
 * Use "RELEASE_ARRAY" to release the memory.
 * Make sure "ptr == NULL" before allocating memory.
 */

#define NEW_ARRAY(ptr, type, count)				\
do {											\
		ptr = new type[count];					\
		if ((ptr) != NULL)						\
		{										\
			memset(ptr, 0, count*sizeof(type));	\
		}										\
	} while(0)


#define RELEASE(pptr)							\
do {											\
	if ((pptr) && *(pptr)) {					\
		delete (*(pptr));						\
		*(pptr) = NULL;							\
	}											\
} while (0)

/* Release (array) memory previously allocated to "*pptr".
 */
#define RELEASE_ARRAY(pptr)						\
do {											\
	if ((pptr) && *(pptr)) {					\
		delete [] *(pptr);						\
		*(pptr) = NULL;							\
	}											\
} while (0)

/*********************************************************************/

#endif          //DEFINE_H
