/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-03-17  dawang.xu  add some error code
2009-02-06  dawang.xu  init first version

=============================================================================*/
#ifndef __ERROR_H__
#define __ERROR_H__

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
	EERASEEFS,

	EMAXERROR = 0xFF,
} TResult;

//#define SUCCESS(result) ((result == EOK) ? TRUE : FALSE)
//#define FAILURE(result) ((result != EOK) ? TRUE : FALSE)

#endif //__ERROR_H__
