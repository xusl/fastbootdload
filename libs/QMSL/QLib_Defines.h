/******************************************************************************/
/**
Program: QMSL

	$Id: //depot/HTE/QDART/QMSL/QLib_Defines.h#200 $

\brief Definition for data types, structures, and used for the QLib.

 \b QUALCOMM  \b PROPRIETARY

	This document contains propriety information, and except with written
	permission of Qualcomm INC, such information shall not be
	published, or disclosed to others, or used for any purpose, and the
	document shall not be duplicated in whole or in part.

	Copyright (c) 2004-2008 QUALCOMM Incorporated.
	All Rights Reserved.
	Qualcomm Confidential and Proprietary


\code
List of sections contained in this document:
	QMSL - Connection/disconnection
	Diagnostic - General Asynchronous message Logging
	Diagnostic - Specific log messages
	Diagnostic - phone version info
	Diagnostic - reading embedded error information
	Diagnostic - General command defines
	Diagnostic - EFS Subsystem
	Diagnostic - NV Subsystem
	Diagnostic - GSDI Subsystem
	Diagnostic - Streaming Download subsystem
	Diagnostic - GPS Subsystem
	Diagnostic - Call Manager subsytem
	Diagnostic - GSM subsytem
	Diagnostic - Handset subsytem
	Diagnostic - MediaFLO subsystem
	Diagnostic - CGPS subsystem
	Diagnostic - ISDB-T subsystem
	FTM - Command ID's
	FTM - System Modes
	FTM - RF Modes
	FTM - RF Definitions
	FTM - GSM RF
	FTM - cdma2000 non-signaling log parser definitions
	FTM - cdma2000 non-signaling
	FTM - EVDO non-signaling log parser definitions
	FTM - EVDO non-signaling
	FTM - Bluetooth
	FTM - AGPS
	FTM - PMIC
	FTM - Customer Extensions
	FTM - Audio
	FTM - Camera
	FTM - Log
	FTM - WCDMA BER
	FTM - HSDPA BLER
	FTM - GSM BER
	FTM - EGPRS BLER
	FTM - Common RF
	FTM - MediaFLO
	FTM - GPS
	Diagnostic - UBM DVBH
	FTM - WLAN
	FTM - Definitions for multiple FTM modes
	QMSL - Text logging flags
	QMSL - Time Out defintions
	QMSL - Calibration Data Manager
	QMSL - NV Tool
\endcode

\note
Compiler:  Microsoft Visual C++ v6.0 SP4
*******************************************************************************/

#if !defined(_QLIB_DEFINES_H)
#define _QLIB_DEFINES_H

#pragma pack(1)        /* DM <--> DMSS packet definitions are byte packed */

/*===========================================================================
                       Definition of primitive data types
===========================================================================*/
#if !defined( QMSL_NO_REDEFINITION_OF_UNSIGNED_INTEGER_TYPES )

	typedef unsigned char byte;			//!<' Definition of unsigned 8-bit type
	typedef unsigned short word ;		//!<' Definition of unsigned 16-bit type
	typedef unsigned long dword ;		//!<' Definition of unsigned 32-bit type


#endif	// #if !defined( QMSL_NO_REDEFINITION_OF_UNSIGNED_INTEGER_TYPES )


#if !defined( QMSL_NO_REDEFINITION_OF_BOOLEAN_TYPES )
	typedef unsigned char boolean;
	typedef unsigned char BOOLEAN;
#endif

/******************************************************************************
                       Basic Definitions
*******************************************************************************/


#if !defined( TRUE )
	#define TRUE   1   //!<' Boolean true value.
#endif

#if !defined( FALSE )
	#define FALSE  0   //!<' Boolean false value.
#endif

#if !defined( ON )
	#define  ON   1    //!<' On value.
#endif

#if !defined( OFF )
	#define  OFF  0    //!<' Off value.
#endif

//! Size of the ITEM DATA field for an NV request
#define DIAG_NV_ITEM_DATA_SIZE 128

//! Maximum size of diagnostic packet
#define DIAG_MAX_PACKET_SIZE 4096


//! Definition of the COM port value that will be used to "auto detect" the COM port
#define QLIB_COM_AUTO_DETECT ((word)0xFFFF)


/**
	 Definition of the COM port value that allows a NULL or empty connection, in
	 other words a phone does not have to be connected.  Only commands that do not
	 send a request/response will work in this mode. For example the mode is designed
	 to allow a DLF log file to playback while the QMSL measurent state machines are
	 active (e.g. cdma2000 SER/FER report)

	WARNING: this device type is only valid in QPHONEMS mode (set using QLIB_SetLibraryMode() )
*/
#define QLIB_COM_NULL ((word)0xFFFE)

//! Definition of SUCCESS for the FTM STATUS field
#define DIAG_FTM_STATUS_SUCCESS 0

//! Definition of FAIL for the FTM STATUS field
#define DIAG_FTM_STATUS_FAIL 1

/******************************************************************************
						QMSL - Connection/disconnection
*******************************************************************************/
/**
	Packet mode definitions for QLIB_SetPacketMode

*/
typedef enum
{
	QLIB_PacketMode_BothDiag	= 0,	//!<' Both library and phone in Diagnostic mode
	QLIB_PacketMode_BothAT		= 1,	//!<' Both library and phone in AT mode
	QLIB_PacketMode_LibAT		= 2		//!<' Library in AT mode, phone is not controlled
} QLIB_PacketMode_Enum;

#define QLIB_LIB_MODE_QPHONEMS false	//!<' Internal engine, QPHONEMS, also used for user defined transport
#define QLIB_LIB_MODE_QPST     true		//!<' Use QPST for packet transport
/******************************************************************************
						Diagnostic - General Request/Response
*******************************************************************************/

/*--------------------------------------------------------------------------

  Command Codes between the Diagnostic Monitor and the mobile. Packets
  travelling in each direction are defined here, while the packet templates
  for requests and responses are distinct.  Note that the same packet id
  value can be used for both a request and a response.  These values
  are used to index a dispatch table in diag.c, so

  DON'T CHANGE THE NUMBERS ( REPLACE UNUSED IDS WITH FILLERS ). NEW IDs
  MUST BE ASSIGNED AT THE END.

  This list is originally taken from the file DiagDefines.h, then reduced
  to only include items necessary for this library.

  If more items are desired or if new diag commands are added then this list
  should be resync'd.


  Note: The prefix _ is used here because the function names are exactly
        the enumeration ID, as used in the embedded code

----------------------------------------------------------------------------*/

#define _DIAG_VERNO_F			0	//!<' Version Number Request/Response
#define _DIAG_ESN_F				1	//!<' Mobile Station ESN Request/Response
#define _DIAG_MEMORY_PEEK_BYTE_F 2	//!<' Memory peek request/response (8-bit)
#define _DIAG_MEMORY_PEEK_WORD_F 3	//!<' Memory peek request/response (16-bit)
#define _DIAG_MEMORY_PEEK_DWORD_F 4	//!<' Memory peek request/response (32-bit)
#define _DIAG_MEMORY_POKE_BYTE_F 5	//!<' Memory poke request/response (8-bit)
#define _DIAG_MEMORY_POKE_WORD_F 6	//!<' Memory poke request/response (16-bit)
#define _DIAG_MEMORY_POKE_DWORD_F 7	//!<' Memory poke request/response (32-bit)
#define _DIAG_STATUS_REQUEST_F	12	//!<'Status Request/Response
#define _DIAG_STATUS_F			14	//!<' Phone status
#define _DIAG_LOG_F				16  //!<' Log packet Request/Response

#define _DIAG_BAD_CMD_F			19	//!<' Invalid Command Response
#define _DIAG_BAD_PARM_F		20	//!<' Invalid parmaeter Response
#define _DIAG_BAD_LEN_F			21	//!<' Invalid packet length Response
#define _DIAG_BAD_MODE_F		24	//!<' Packet not allowed in this mode

/* 22-23 Reserved */

/* Packet not allowed in this mode
   ( online vs offline )                      */
#define DIAG_BAD_MODE_F     24

#define _DIAG_MSG_F				31	//!<' Request for msg report
#define _DIAG_HS_KEY_F			32	//!<' Handset Emulation -- keypress
#define _DIAG_NV_READ_F			38	//!<' Read NV item
#define _DIAG_NV_WRITE_F		39	//!<' Write NV item
#define _DIAG_CONTROL_F			41	//!<' Mode change request
#define _DIAG_ERR_READ_F		42	//!<' Read error list
#define _DIAG_ERR_CLEAR_F		43	//!<' Clear error list
#define _DIAG_GET_DIPSW_F		47 	//!<' Retreive dipswitch
#define _DIAG_SET_DIPSW_F		48	//!<' Set dipswitch
#define _DIAG_VOC_PCM_LB_F		49  //!<' Start/Stop Vocoder PCM loopback
#define _DIAG_VOC_PKT_LB_F		50  //!<' Start/Stop Vocoder PKT loopback
#define _DIAG_CALL_ORIGINATION		53  //!<' Start a Mobile Origination call, e.g., SO55 IS2000 loopback
#define _DIAG_DLOAD_F			58	//!<' Switch to download mode
#define _DIAG_SPC_F				65  //!<' Send the Service Prog. Code to allow SP
#define _DIAG_SERIAL_MODE_CHANGE 68 //!<' Switch mode from diagnostic to data
#define _DIAG_EXT_LOGMASK_F		93	//!<' Extended logmask for > 32 bits.
#define _DIAG_EVENT_REPORT_F    96  //!<' Static Event reporting.
#define _DIAG_SUBSYS_CMD_F		75  //!<' Subssytem dispatcher (extended diag cmd)
#define _DIAG_NV_WRITE_ONLINE_F	76	//!<' Write to NV location without going Offline
#define _DIAG_IS2000_STATUS_F   112	//!<' IS-2000 Status
#define _DIAG_LOG_CONFIG_F		115	//!<' Logging configuration packet
#define _DIAG_EXT_MSG_F			121 //!<' Extended msg report
#define _DIAG_PROTOCOL_LOOPBACK_F  123 //!<' Diagnostics protocol loopback.
#define _DIAG_EXT_BUILD_ID_F    124	//!<' Extended build ID
#define _DIAG_EXT_MSG_CONFIG_F  125 //!<' Request for extended msg report
#define _DIAG_SECURITY_FREEZE_F 0xff	//!<' Request for Sirius security freeze (not defined yet)
#define _DIAG_MAX_F				126	//!<' Number of packets defined.
#define _DIAG_SUBSYS_CMD_VER_2_F	128
#define _DIAG_EVENT_MASK_GET_F		129	//!<' Get event mask
#define _DIAG_EVENT_MASK_SET_F		130	//!<' Set event mask

/**
	Diagnostic Subsystems, from Services\diagcmd.h

*/
typedef enum
{
	_DIAG_SUBSYS_OEM		= 0,
	_DIAG_SUBSYS_ZREX		= 1,
	_DIAG_SUBSYS_SD			= 2,
	_DIAG_SUBSYS_BT			= 3,
	_DIAG_SUBSYS_WCDMA		= 4,
	_DIAG_SUBSYS_HDR		= 5,
	_DIAG_SUBSYS_DIABLO		= 6,
	_DIAG_SUBSYS_TREX		= 7,
	_DIAG_SUBSYS_GSM		= 8,
	_DIAG_SUBSYS_UMTS		= 9,
	_DIAG_SUBSYS_HWTC		= 10,
	_DIAG_SUBSYS_FTM		= 11,
	_DIAG_SUBSYS_REX		= 12,
	_DIAG_SUBSYS_GPS		= 13,
	_DIAG_SUBSYS_WMS		= 14,
	_DIAG_SUBSYS_CM			= 15,
	_DIAG_SUBSYS_HS			= 16,
	_DIAG_SUBSYS_AUDIO_SETTINGS   = 17,
	_DIAG_SUBSYS_DIAG_SERV	= 18,
	_DIAG_SUBSYS_EFS		= 19,
	_DIAG_SUBSYS_PORT_MAP_SETTINGS  = 20,
	_DIAG_SUBSYS_MEDIAPLAYER= 21,
	_DIAG_SUBSYS_QCAMERA	= 22,
	_DIAG_SUBSYS_MOBIMON	= 23,
	_DIAG_SUBSYS_GUNIMON	= 24,
	_DIAG_SUBSYS_LSM		= 25,
	_DIAG_SUBSYS_QCAMCORDER	= 26,
	_DIAG_SUBSYS_MUX1X		= 27,
	_DIAG_SUBSYS_DATA1X		= 28,
	_DIAG_SUBSYS_SRCH1X		= 29,
	_DIAG_SUBSYS_CALLP1X	= 30,
	_DIAG_SUBSYS_APPS		= 31,
	_DIAG_SUBSYS_SETTINGS	= 32,
	_DIAG_SUBSYS_GSDI		= 33,
	_DIAG_SUBSYS_TMC		= 34,
	_DIAG_SUBSYS_USB		= 35,
	_DIAG_SUBSYS_PM			= 36,
	_DIAG_SUBSYS_DEBUG		= 37,
	_DIAG_SUBSYS_QTV		= 38,
	_DIAG_SUBSYS_CLKRGM		= 39,
	_DIAG_SUBSYS_DEVICES	= 40,
	_DIAG_SUBSYS_WLAN		= 41,
	_DIAG_SUBSYS_PS_DATA_LOGGING=42,
	_DIAG_SUBSYS_MFLO		= 43,
	_DIAG_SUBSYS_DTV		= 44,
	_DIAG_SUBSYS_RRC		= 45,
	_DIAG_SUBSYS_PROF		= 46,
	_DIAG_SUBSYS_TCXOMGR	= 47,
	_DIAG_SUBSYS_NV			= 48,
	_DIAG_SUBSYS_AUTOCONFIG	= 49,
	_DIAG_SUBSYS_PARAMS		= 50,
	_DIAG_SUBSYS_MDDI		= 51,
	_DIAG_SUBSYS_DS_ATCOP	= 52,
	_DIAG_SUBSYS_L4LINUX	= 53,
	_DIAG_SUBSYS_MVS		= 54,
	_DIAG_SUBSYS_CNV		= 55,
	_DIAG_SUBSYS_LAST
} DiagPkt_Subsys_cmd_enum_type;

/******************************************************************************
						Diagnostic - General Asynchronous message Logging
*******************************************************************************/

//! Structure to define storage area for a response messages in a queue
typedef struct
{
	word iMessageLength;

	// This object can be accessed as a single large buffer of bytes...
	byte iData[DIAG_MAX_PACKET_SIZE];

} ResponseMessage;

/*===========================================================================*/
/**
	enum log_equip_id_enum_type

	\brief Log Equipment IDs. The number is represented by 4 bits.
*/
/*===========================================================================*/
typedef enum
{
  LOG_EQUIP_ID_OEM   = 0,	//!<' 3rd party OEM (licensee) use
  LOG_EQUIP_ID_1X    = 1,	//!<' Traditional 1X line of products
  LOG_EQUIP_ID_RSVD2 = 2,
  LOG_EQUIP_ID_RSVD3 = 3,
  LOG_EQUIP_ID_WCDMA = 4,	//!<' WCDMA
  LOG_EQUIP_ID_GSM   = 5,	//!<' GSM
  LOG_EQUIP_ID_MSP   = 6,
  LOG_EQUIP_ID_UMTS  = 7,	//!<' UMGS
  LOG_EQUIP_ID_TDMA  = 8,	//!<' TDMA

  LOG_EQUIP_ID_LAST_DEFAULT = LOG_EQUIP_ID_TDMA,
  LOG_EQUIP_ID_MAX = 16

} log_equip_id_enum_type;

/*===========================================================================*/
/**
	enum log_operation_id_enum_type

	\brief List of log configuration operations
*/
typedef enum
{
	LOG_CONFIG_OP_DisableLogging,	//!<' 0 = Disable logging service
	LOG_CONFIG_OP_RetrieveIDs,		//!<' 1 = Retrieve ID ranges
	LOG_CONFIG_OP_Reserved,			//!<' 2 = (Reserved)
	LOG_CONFIG_OP_SetLogMask,		//!<' 3 = Set log mask
	LOG_CONFIG_OP_GetLogMask		//!<' 4 = Get log mask

} log_operation_id_enum_type;

#define LOG_CODE_FTM_2			0x117C	//!<' FTM Version 2 log file
#define LOG_CODE_LAST_ITEM_1X	0x02E8	//!<' Last item number for 1X, log code 1.

//! Number of bytes used for time stamp in diagnostic async messages
#define QMSL_DIAG_TIME_STAMP_SIZE 8

/*===========================================================================*/
/**
	structure for an ExtendedMessage, as described in CDMA ICD 3.4.121 Extended Message

*/
/*===========================================================================*/
typedef struct
{
	byte CMD_CODE;		//!< ' Message ID. The DMSS sets CMD_CODE to 121 for this message.
	byte TS_TYPE;		//!< ' Time stamp type; an enumeration indicating the format of the TIME field
	byte NUM_ARGS;		//!< ' The number of 32-bit arguments listed in the ARGS field.
	byte DROP_CNT;		//!< ' Total number of messages dropped between this message and the previous one
	byte TIME[QMSL_DIAG_TIME_STAMP_SIZE];		//!< ' Time the message was originally generated (not transmitted)
	word LINE_NUMBER;	//!< ' Line number identifying location of the message in the file denoted by FILE_NAME
	word SS_ID;			//!< ' Subsystem identifier; see Section 3.4.125.2 for description
	dword SS_MASK;		//!< ' Subsystem mask; see Section 3.4.125.2 for description
	dword ARG_1;		//!< ' Array of 32-bit signed arguments, corresponding to the printf-style FMT_STRING
	dword ARG_2;		//!< '
	dword ARG_3;		//!< '
	char FMT_STRING[100];		//!< ' NULL-terminated ASCII string containing a printf()-style format,
	                            //!< '  string, with formatting specifiers
	char FILE_NAME[100];		//!< ' NULL-terminated ASCII file name string identifying the location of message


} ExtendedMessage;


/*===========================================================================*/
/**
	Structure of a log message, as described in WCDMA ICD section 4.1 - Log record structure
*/
/*===========================================================================*/

typedef struct
{
	byte CMD_CODE;
	byte MORE;
	word length;
	word length2;		// Not sure why length is being put in twice...could be a receieve data error
	word log_item;
	byte iTimeStamp[QMSL_DIAG_TIME_STAMP_SIZE];
	byte iLogMsgData[DIAG_MAX_PACKET_SIZE - 6] ;

} LogMessage;

/*===========================================================================*/
/**
	Structure of a streaming message, as described in WCDMA ICD 3.5.17.2 - Message Response Message
*/
/*===========================================================================*/

typedef struct
{
	byte CMD_CODE;		// (31) for a streaming message
	word QUANTITY;
	dword DROP_CNT;
	dword TOTAL_MSGS;
	byte MESSAGE_LEVEL;
	char FILE_NAME[13];
	word LINE_NUMBER;
	byte FMT_STRING[40];
	dword CODE1;
	dword CODE2;
	dword CODE3;
	byte TIME[QMSL_DIAG_TIME_STAMP_SIZE];
} MessageResponse;

/**
	Subsystem dispatcher header, request or response.
*/
typedef struct
{
  byte	cmdCode ;             //!<' = DIAG_SUBSYS_CMD_F
  byte	subsysId ;            //!<' = diagpkt_subsys_cmd_enum_type
  word	subsysCmdCode ;      //!<' Subsystem-specific command code.
} diagpkt_subsys_header_type;


/**
	FTM Subsystem dispatcher header, request with extended information
*/
typedef struct
{
	diagpkt_subsys_header_type Diag_Header;

	word cmd_id;			//!<' ftm_hdr.cmd_id;
	word data_len;			//!<' ftm_hdr.data_len
	word rsp_pkt_size;		//!<' ftm_hdr.rsp_pkt_size

} ftm_extended_request_header_type;

/**
	FTM Subsystem dispatcher header, response with extended information + status
*/
typedef struct
{
	ftm_extended_request_header_type ftm_extended_header;
	word status;

} ftm_extended_response_header_type;

/*===========================================================================*/
/**
	Structure of a streaming event, as described in WCDMA ICD 3.4.95 Event Report
*/
/*===========================================================================*/

typedef struct
{
	byte CMD_CODE;		//!<' (96) for a streaming event
	word iLength;		//!<' # of bytes in the EVENTS field
	byte aiEvents[ DIAG_MAX_PACKET_SIZE - 3];	//!<' EVENTS field, list of event items
} Event_struct;

/*===========================================================================*/
/**
	Structure of a the EVENT item defintion, as described in WCDMA ICD 3.4.95 Event Report
*/
/*===========================================================================*/

typedef struct
{
	word EVENT_ID;		//!<' # of event ID
	word iLength;		//!<' # of bytes in the EVENTS field
	byte iTime[QMSL_DIAG_TIME_STAMP_SIZE];		//!<' time event was generated, this is possibly a 2-byte field
	byte iPayloadEn;	//!<' Is the payload enabled?
	byte aiPayload[DIAG_MAX_PACKET_SIZE];	//!<' payload, generally not used, size is variable

} EventItem_struct;

/**
	Bit field maping of the EVENT_ID field
*/
typedef struct
{



	//! Unique event ID
	word event_id : 12;

	word reserved : 1;

	/**
		Flag to specify whether PAYLOAD _DATA is present in event
		\code
		0 = No payload data
		1 = One byte of payload data, no PAYLOAD_LENGTH field
		2 = Two bytes of payload data, no PAYLOAD_LENGTH field
		3 = One byte PAYLOAD_LENGTH field specifying length of payload data to follow it
		\endcode
	*/
	word payload_length_flag : 2;

	/**
		\code
		0 = Full system time stamp is 8 bytes
		1 = Truncated time stamp is 2 bytes
		\endcode
	*/
	word time_length : 1;

} EventIdBitField;


/*===========================================================================*/
/**
	enum time_length_enum_type

	\brief Enumeration for the time stamp in CDMA DMSS ICD 80-V1294-1 3.4.95
*/
typedef enum
{
	FULL_SYS_TIME = 0,
	TRUNCATED_SYS_TIME,
	MAX_SYS_TIME = 0xff
}time_length_enum_type;

/*===========================================================================*/
/**
	enum time_stamp_length

	\brief Enumeration for the time stamp length (bytes) in CDMA DMSS ICD 80-V1294-1 3.4.95
*/
typedef enum
{
	TRUNCATED_TIMESTAMP = 2,
	FULL_SYSTEM_TIMESTAMP = 8
}time_stamp_length;

/*===========================================================================*/
/**
	enum payload_length_data_flag

	\brief Enumeration for the payload_length_data_flag in CDMA DMSS ICD 80-V1294-1 3.4.95
*/
typedef enum
{
	NO_PAYLOAD_DATA = 0,
	ONE_BYTE_PAYLOAD_DATA,
	TWO_BYTE_PAYLOAD_DATA,
	VAR_LENGTH_PAYLOAD_DATA,
	MAX_PAYLOAD_DATA = 0xff
}payload_length_data_flag;



/*===========================================================================*/
/**
	struct QMSL_Event_Element_Struct

	\brief Structure to define how QMSL will store events in the event queue
*/
#define QMSL_EVENT_PAYLOAD_DATA_MAX 256
typedef struct
{
	unsigned char time_length;	//!< # of bytes used to store type, 8 = full system time (FULL_SYSTEM_TIMESTAMP), 2=Truncated time (TRUNCATED_TIMESTAMP)
	unsigned char time[QMSL_DIAG_TIME_STAMP_SIZE];		//!< 8 or 2 bytes of system time, format depends upon time_length field
	unsigned short event_id;	//!< 12-bit unique ID of event
	unsigned char payload_len;	//!< # of bytes stored in payload_len
	unsigned char payload_data[ QMSL_EVENT_PAYLOAD_DATA_MAX ];	//!< payload data, payload_len field determines # of valid bytes
} QMSL_Event_Element_Struct;


//! Wildcard event ID
#define QMSL_EVENT_ID_WILDCARD 0xFFFF

/******************************************************************************
						Diagnostic - Specific log messages
*******************************************************************************/


/*===========================================================================*/
/**
	Structure for interpretting log code 0x7005, MMGPS report measurement

		This log message has an array of these items.  The overall structure
		is defined by MMGPS_ReportMeasurement.
*/
/*===========================================================================*/
typedef struct
{
	byte	SV_ID;
	byte	HEALTH;
	byte	ELEVATION;
	byte	AZIMUTH;
	word	SNR;
	word	CNO;
	word	LATENCY_MS;
	byte	PRE_INT;
	word	POST_INT;
	dword	MILLISECOND;
	dword	SUB_MILLISECOND;
	word	SV_TIME_UNC;
	dword	SV_SPEED;
	word	SV_SPEED_UNC;
	word	MEAS_STATUS;
	byte	CHANNEL_STATE;
	byte	GOOD_OBSERVATIONS;
	byte	TOTAL_OBSERVATIONS;
	word	RESERVED;
} MMGPS_ReportMeasurement_MeasurementRow;


#define DIAG_MMGPS_NUM_MEASUREMENT_REPORTS 13

/*===========================================================================*/
/**
	Structure for interpretting log code 0x7005, MMGPS report measurement

*/
/*===========================================================================*/
typedef struct
{
	dword	FCOUNT;
	MMGPS_ReportMeasurement_MeasurementRow aMeasurements[ DIAG_MMGPS_NUM_MEASUREMENT_REPORTS ];
} MMGPS_ReportMeasurement;


/*===========================================================================*/
/**
	Structure for interpretting log code 0x701f, MMGPS report measurement

		This log message has an array of these items.  The overall structure
		is defined by MMGPS_ReportMeasurement.
*/
/*===========================================================================*/
typedef struct
{
	byte	SV_ID;
	byte	HEALTH;
	byte	ELEVATION;
	byte	AZIMUTH;
	word	SNR;
	word	CNO;
	word	LATENCY_MS;
	byte	PRE_INT;
	word	POST_INT;
	dword	MILLISECOND;
	dword	SUB_MILLISECOND;
	word	SV_TIME_UNC;
	dword	SV_SPEED;
	word	SV_SPEED_UNC;
	word	MEAS_STATUS;
	byte	CHANNEL_STATE;
	byte	GOOD_OBSERVATIONS;
	byte	TOTAL_OBSERVATIONS;
	dword	RESERVED;		// The size of this field is the difference between version 1 and 3
} MMGPS_ReportMeasurement_MeasurementRow_V3;

#define DIAG_MMGPS_NUM_MEASUREMENT_REPORTS_V3 32
/*===========================================================================*/
/**
	Structure for interpretting log code 0x701f, MMGPS report measurement

*/
/*===========================================================================*/
typedef struct
{
	dword	FCOUNT;
	word LENGTH;
	byte VERSION;
	byte RESERVE1[8];
	byte NUMBER_SVS;	// 1 to 32

	MMGPS_ReportMeasurement_MeasurementRow aMeasurements[ DIAG_MMGPS_NUM_MEASUREMENT_REPORTS_V3 ];
} MMGPS_ReportMeasurement_V3;

/**
	Structure of the Bluetooth local version, as defined in "BLUETOOTH SPECIFICATION Version 1.1"
*/
typedef struct
{
	byte  BT_EventHeader[6];	//!<'  Header information associated with the Bluetooth event
	byte  BT_Status;			//!<'  Should == BT_BE_SUCCESS == 0x00
	byte  HCI_Version;			//!<'  Should == (BT_HCI_VERSION_1_1 == 0x01) ||(BT_HCI_VERSION_1_2 = 0x02)
	word HCI_Revision;			//!<'  Should == 0
	byte  LMP_Version;			//!<'  For 1.1: (BT_LMP_VERSION_1_0 == 0x01), for 2.0: BT_LMP_VERSION_1_2
	word Manufacturer_Name;		//!<'  For QCOM: 29 (0x1D)
	word LMP_Subversion;		//!<'  BT_LMP_SUB_VER_NUM_0 == 0
} Bluetooth_LocalVersion_struct;


//! Maximum size of a QMSL Text Log
#define QLIB_MAX_TEXT_LOG_SIZE 1000

/**
	Definition of the LOG code #id that will be used to indicate that a certain log is actually
	a QMSL Text log, not a normal phone log.

	The idea is that text messages from the library will be sent the the QMSL client using the
	asynchronous log mechanism.  Log messages will be generated of command code #16, which is
	defined in the parimary ICD's for CDMA and WCDMA.

*/
#define QLIB_TEXT_LOG_CODE 0xFFFF

/**
	Structure of the QMSL text messages, which are sent through the async logging interface
*/
typedef struct
{
	/**
		Log level is a 16-bit field that indicates what level this log code is.

		The levels are defined in the pre-processor constants list that includes
		LOG_IO and LOG_FN.

			#define LOG_IO			0x0001	// data IO (data bytes)
			#define LOG_FN			0x0002	// function calls with parameters

	*/
	word iLogLevel;
	word iReserved;									//!<'  Reserved for future use
	long iTimer_ms;									//!<'  # of milliseconds since the log was started
	byte  sLogText[ QLIB_MAX_TEXT_LOG_SIZE ];		//!<'  The text string

} QMSL_TextLog_struct;

/******************************************************************************
						Diagnostic - phone version info
*******************************************************************************/

/*============================================================================

PACKET   diag_verno_rsp_type

ID       DIAG_VERNO_F

PURPOSE  Sent by the DMSS, contains version and date information

============================================================================*/
#define QLIB_VERNO_DATE_STRLEN 11				//!<' Length of date field (bytes)
#define QLIB_VERNO_TIME_STRLEN  8				//!<' Length of time field (bytes)
#define QLIB_VERNO_DIR_STRLEN   8				//!<' Length of version directory (bytes)


/*===========================================================================*/
/**
	\brief Version information structure.

	To be used with the QLIB_DIAG_VERNO_F command.
*/
/*===========================================================================*/
struct QLIB_PhoneResp_Version
{
  byte cmd_code;                              //!<' Command code
  char comp_date[ QLIB_VERNO_DATE_STRLEN ];   //!<' Compile date Jun 11 1991
  char comp_time[ QLIB_VERNO_TIME_STRLEN ];   //!<' Compile time hh:mm:ss
  char rel_date [ QLIB_VERNO_DATE_STRLEN ];   //!<' Release date
  char rel_time [ QLIB_VERNO_TIME_STRLEN ];   //!<' Release time
  char ver_dir  [ QLIB_VERNO_DIR_STRLEN ];    //!<' Version directory
  byte scm;                                   //!<' Station Class Mark
  byte mob_cai_rev;                           //!<' CAI rev
  byte mob_model;                             //!<' Mobile Model
  word mob_firm_rev;                          //!<' Firmware Rev
  byte slot_cycle_index;                      //!<' Slot Cycle Index
  byte voc_maj;                               //!<' Vocoder major version
  byte voc_min;								  //!<' Vocoder minor version
};


/******************************************************************************
		Diagnostic - reading embedded error information
*******************************************************************************/

/*===========================================================================*/
/**
	\brief Error record information structure.

	To be used with the QLIB_DIAG_ERR_READ_F command.
*/
/*===========================================================================*/
typedef struct
{
	/**
		Error address.  Index (0-19) of this error record; sequential
	*/
	byte iAddress;

	/**
		Error count
		Number of times this error has been recorded. If zero, this error
		record is empty; this saturates at 255
	*/
	byte iCount;

	/**
		Root name of file.
		The root name of the file where the error was detected, such as
		mobile for mobile.c. Longfile names are truncated to the first
		eight characters
	*/
	byte sFileName[ 8 ];

	/**
		Line number within file.
		The line number within FILE_NAME where the error was detected
	*/
	word iLineNum;

	/**
		Error fatal indicator
		If nonzero, the error was fatal. If zero, the error was nonfatal
	*/
	byte bFatal;

} QLIB_DIAG_Err_Read_Element;

#define QLIB_NUM_ERR_READ_ELEMENTS 20

/*===========================================================================*/
/**
	\brief Error record information structure.

	To be used with the QLIB_DIAG_ERR_READ_F command.
*/
/*===========================================================================*/
typedef struct
{
	/**
		Error count.  The number of ERR_RECORDS that have valid error data.
	*/
	word iErrorCount;

	/**
		Errors ignored count
		Number of errors ignored since last DMSS reset. This happens when another
		error record cannot be allocated, or the COUNT for a given error is saturated.
		ERR_RECORDS = 20 of the following five records
	*/
	word iErrorIgnored;

	//! Array of error read elements
	QLIB_DIAG_Err_Read_Element aErrorRecords[ QLIB_NUM_ERR_READ_ELEMENTS ];
} QLIB_DIAG_Err_Read;


/******************************************************************************
						Diagnostic - General command defines
*******************************************************************************/

/*===========================================================================*/
/**
PACKET   diag_control_req_type

ID       DIAG_CONTROL_F

PURPOSE  Sent by DM to direct the DMSS to go offline or reset
         These are defined in services/cm/cmdiag.h

RESPONSE DMSS changes mode or resets
*/
/*===========================================================================*/
typedef enum
{
    MODE_OFFLINE_A_F = 0,	//!<' Go to offline analog
    MODE_OFFLINE_D_F = 1,	//!<' Go to offline digital
    MODE_RESET_F = 2,		//!<' Reset. Only exit from offline
    MODE_FTM_F = 3,			//!<' FTM mode
    MODE_ONLINE_F = 4,		//!<' Go to Online
	MODE_LPM_F = 5,			//!<' Low Power Mode (if supported)
	MODE_POWER_OFF_F = 6,	//!<' Power off (if supported)
    MODE_MAX_F = 7			//!<' Last (and invalid) mode enum value

} mode_enum_type;


/*===========================================================================*/
/**
	enum nv_stat_enum_type

	\brief States defined as possible return values for an NV function.

	This definition was taken from NV.H and should be resync'd if the list in that
	file changes.
/*
/*===========================================================================*/
typedef enum
{
    NV_DONE_S,          //!<' Request completed okay
    NV_BUSY_S,          //!<' Request is queued
    NV_BADCMD_S,        //!<' Unrecognizable command field
    NV_FULL_S,          //!<' The NVM is full
    NV_FAIL_S,          //!<' Command failed, reason other than NVM was full
    NV_NOTACTIVE_S,     //!<' Variable was not active
    NV_BADPARM_S,       //!<' Bad parameter in command block
    NV_READONLY_S,      //!<' Parameter is write-protected and thus read only
    NV_BADTG_S,         //!<' Item not valid for Target
    NV_NOMEM_S,         //!<' free memory exhausted
    NV_NOTALLOC_S,      //!<' address is not a valid allocation
    NV_STAT_ENUM_PAD = 0x7FFF     //!<' Pad to 16 bits on ARM
} nv_stat_enum_type;


/*===========================================================================*/
/**
Enums for the return values of the command DIAG_STATUS_F


*/
/*===========================================================================*/
typedef enum
{
    DIAG_STATUS_OFFLINE = 0,	//!<' OFFLINE Status
    DIAG_STATUS_ONLINE = 1	,	//!<' ONLINE Status
    DIAG_STATUS_LPM = 2	,		//!<' Low Power Status
    DIAG_STATUS_UNKNOWN = 99	//!<' Last (and invalid) mode enum value
} diag_status_enum_type;


/*===========================================================================
                       NV Item enumerations

  The NV ID's are not published in QMSL, in order to avoid synchronization
  issues with new builds of software.  The enumerations should be taken
  from the SERVCIES\NV\NV.H file in the embedded software, which will always
  have the correct and up-to-date enumeration ID's for a specific target.
===========================================================================*/


/*===========================================================================*/
/**
	enum KeyPressID

	\brief Enumeration of key press identifiers

	These definitions came from PHONEI.H.  If more key press ID's are added then
	the list should be resync'd
*/
/*===========================================================================*/
typedef enum
{
  HS_NONE_K = 0,        //!<'  No more keycodes available
  HS_ON_HOOK_K,         //!<'  phone has gone on-hook
  HS_OFF_HOOK_K,        //!<'  phone has gone off-hook
  HS_RING_VOL_0_K,      //!<'  ringer volume 0 (Keep 0-2 in order)
  HS_RING_VOL_1_K,      //!<'  ringer volume 1 ( and sequential!!)
  HS_RING_VOL_2_K,      //!<'  ringer volume 2
  HS_EAR_UP_K,          //!<'  earpiece up
  HS_EAR_UP_END_K,      //!<'  earpiece up + end pressed
  HS_EAR_DOWN_K,        //!<'  earpiece down
  HS_PF1_K,             //!<'  softkey #1 (left-most)
  HS_PF2_K,             //!<'  softkey #2 (right-most)
  HS_MSG_K,             //!<'  message waiting
  HS_POUND_K = 0x23,    //!<'  '#' key, ASCII '#'
  HS_STAR_K = 0x2A,     //!<'  '*' key, ASCII '*'
  HS_0_K = 0x30,        //!<'  '0' key, ASCII '0'
  HS_1_K = 0x31,        //!<'  '1' key, ASCII '1'
  HS_2_K = 0x32,        //!<'  '2' key, ASCII '2'
  HS_3_K = 0x33,        //!<'  '3' key, ASCII '3'
  HS_4_K = 0x34,        //!<'  '4' key, ASCII '4'
  HS_5_K = 0x35,        //!<'  '5' key, ASCII '5'
  HS_6_K = 0x36,        //!<'  '6' key, ASCII '6'
  HS_7_K = 0x37,        //!<'  '7' key, ASCII '7'
  HS_8_K = 0x38,        //!<'  '8' key, ASCII '8'
  HS_9_K = 0x39,        //!<'  '9' key, ASCII '9'
  HS_SEND_K = 0x50,     //!<'  Send key
  HS_END_K = 0x51,      //!<'  End key or Power key (Based on Target)
  HS_CLR_K = 0x52,      //!<'  Clear key
  HS_STO_K = 0x53,      //!<'  Store key
  HS_UP_K = 0x54,       //!<'  Up-arrow key was pressed
  HS_DOWN_K = 0x55,     //!<'  Down-arrow key was pressed
  HS_MUTE_K = 0x56,     //!<'  Mute Key
  HS_RCL_K = 0x57,      //!<'  Recall key
  HS_SD1_K = 0x58,      //!<'  speed dial #1
  HS_SD2_K = 0x59,      //!<'  speed dial #2
  HS_SD3_K = 0x5A,      //!<'  speed dial #3
  HS_MENU_K = 0x5B,     //!<'  Menu key
  HS_ALPHA_K = 0x5C,    //!<'  Alpha key
  HS_DEC_POUND_K,       //!<'  Decoded # DTMF
  HS_DEC_STAR_K,        //!<'  Decoded * DTMF
  HS_DEC_0_K,           //!<'  Decoded 0 DTMF
  HS_DEC_1_K,           //!<'  Decoded 1 DTMF
  HS_DEC_2_K,           //!<'  Decoded 2 DTMF
  HS_DEC_3_K,           //!<'  Decoded 3 DTMF
  HS_DEC_4_K,           //!<'  Decoded 4 DTMF
  HS_DEC_5_K,           //!<'  Decoded 5 DTMF
  HS_DEC_6_K,           //!<'  Decoded 6 DTMF
  HS_DEC_7_K,           //!<'  Decoded 7 DTMF
  HS_DEC_8_K,           //!<'  Decoded 8 DTMF
  HS_DEC_9_K,           //!<'  Decoded 9 DTMF
  HS_DEC_A_K,           //!<'  Decoded A DTMF
  HS_DEC_B_K,           //!<'  Decoded B DTMF
  HS_DEC_C_K,           //!<'  Decoded C DTMF
  HS_DEC_D_K,           //!<'  Decoded D DTMF
  HS_PWR_ON_K,          //!<'  Power key is switched 'on'
  HS_PWR_OFF_K,         //!<'  Power key is switched 'off'
  HS_PWR_K,             //!<'  Power key
  HS_INFO_K,            //!<'  Info key
  HS_FREE_K,            //!<'  Phone was placed in hands-free cradle
  HS_PHONE_K,           //!<'  Phone was lifted from hands-free cradle
  HS_IGN_OFF_K,         //!<'  Ignition was turned off
  HS_IGN_ON_K,          //!<'  Ignition was turned on
  HS_TMR_ON_K,          //!<'  Power-up timer 'on' pseudo-key
  HS_TMR_OFF_K,         //!<'  Power-up timer 'off' pseudo-key
  HS_BAD_BATT_K,        //!<'  The car battery is failing
  HS_EXT_PWR_ON_K,      //!<'  External power was turned on
  HS_EXT_PWR_OFF_K,     //!<'  External power was turned off
  HS_REDIAL_K,          //!<'  Redial key
  HS_RSSI_K,            //!<'  RSSI key
  HS_HFK_CORD_K,        //!<'  Coil cord carkit
  HS_HFK_NOCORD_K,      //!<'  Non-coil cord carkit
  HS_HEADSET_K,         //!<'  Headset connected
  HS_HEADSET_OFF_K,     //!<'  Headset disconnected
  HS_DEV_UNKNOWN_K,     //!<'  Unknown device ID
  HS_EXT_DEV_K,         //!<'  ONES detector finds external device
  HS_CHARGING_ON_K,     //!<'  Key to note battery charging active
  HS_CHARGING_OFF_K,    //!<'  Key to note battery charging stopped
  HS_LEFT_K,			//!<'  Left soft Key
  HS_RIGHT_K,			//!<'  Right soft Key
  HS_APPLICATION_K,		//!<'  Application key
  HS_RIGHT_SELECT_K,	//!<'  Right hand select key
  HS_LEFT_SELECT_K,		//!<'  Left hand select key
  HS_BROWSER_K,			//!<'  Browser key
  HS_LO_RES_CAMERA_K,	//!<'  Low resolution Camera key
  HS_HI_RES_CAMERA_K,	//!<'  High resolution Camera key
  HS_PTT_K,				//!<'  Push to talk
  HS_RELEASE_K = 0xff   //!<'  Key to note that all keys are up
} KeyPressID;

/*===========================================================================*/
/**
	enum KeyPressEvent

	\brief Enumeration of key press identifiers

	These definitions came from PHONEI.H.  If more key press ID's are added then
	the list should be resync'd
*/
/*===========================================================================*/
typedef enum
{
	DIAG_KeyPress_Down		= 1,
	DIAG_KeyPress_Up		= 2,
	DIAG_KeyPress_Unknown	= 3
} KeyPressEventType;


//! Structure for defining a key press event
typedef struct
{
	KeyPressID eKeyID;
	KeyPressEventType eKeyEvent;
} KeyPressEvent;



/******************************************************************************
						Diagnostic - EFS Subsystem
*******************************************************************************/


/**
	EFS sub system commands
*/
typedef enum
{
	_EFS2_DIAG_HELLO	= 0,		//!<' Parameter negotiation packet
	_EFS2_DIAG_QUERY	= 1,		//!<' Send information about EFS2 params
	_EFS2_DIAG_OPEN		= 2,		//!<' Open a file
	_EFS2_DIAG_CLOSE	= 3,		//!<' Close a file
	_EFS2_DIAG_READ		= 4,		//!<' Read a file
	_EFS2_DIAG_WRITE	= 5,		//!<' Write a file
	_EFS2_DIAG_SYMLINK	= 6,		//!<' Create a symbolic link
	_EFS2_DIAG_READLINK	= 7,		//!<' Read a symbolic link
	_EFS2_DIAG_UNLINK	= 8,		//!<' Remove a symbolic link or file
	_EFS2_DIAG_MKDIR	= 9,		//!<' Create a directory
	_EFS2_DIAG_RMDIR	= 10,		//!<' Remove a directory
	_EFS2_DIAG_OPENDIR	= 11,		//!<' Open a directory for reading               */
	_EFS2_DIAG_READDIR	= 12,		//!<' Read a directory                           */
	_EFS2_DIAG_CLOSEDIR	= 13,		//!<' Close an open directory                    */
	_EFS2_DIAG_RENAME	= 14,		//!<' Rename a file or directory                 */
	_EFS2_DIAG_STAT		= 15,		//!<' Obtain information about a named file      */
	_EFS2_DIAG_LSTAT	= 16,		//!<' Obtain information about a symbolic link   */
	_EFS2_DIAG_FSTAT	= 17,		//!<' Obtain information about a file descriptor */
	_EFS2_DIAG_CHMOD	= 18,		//!<' Change file permissions                    */
	_EFS2_DIAG_STATFS	= 19,		//!<' Obtain file system information             */
	_EFS2_DIAG_ACCESS	= 20,		//!<' Check a named file for accessibility       */
	_EFS2_DIAG_NAND_DEV_INFO	= 21,	//!<' Get NAND device info */
	_EFS2_DIAG_FACT_IMAGE_START	= 22,	//!<' Start EFS2 data output for Factory Image */
	_EFS2_DIAG_FACT_IMAGE_READ	= 23,	//!<' Get EFS2 data for Factory Image */
	_EFS2_DIAG_FACT_IMAGE_END	= 24,	//!<' End EFS2 data output for Factory Image */
	_EFS2_DIAG_PREP_FACT_IMAGE	= 25,	//!<' Prepare file system for image dump */
	_EFS2_DIAG_PUT		= 26,			//!<'
	_EFS2_DIAG_GET		= 27,			//!<'
	_EFS2_DIAG_ERROR	= 28,			//!<'
	_EFS2_DIAG_EXTENDED_INFO	= 29	//!<' Get Extra information. */

} EFS2_CMD_Enum;

/**
	Data types of extended EFS information--string or integer
*/
typedef enum
{
	EFS2_EXT_Kind_Int		= 0,	//!<' Extended Information will be returned as an integer
	EFS2_EXT_Kind_String	= 1		//!<' ExtendedInformation will be returned as a string

} EFS2_EXT_Kind_Enum;

/**
	Currently defined queries. Querying information that is not available
	will return an efs2_diag_error_rsp_type packet with the error code set
	to FS_DIAG_UNAVAILABLE_INFO. All queries expect 'path' to be set to an
	EFS2 path that determines which filesystem the query applies to. The
	path must be null terminated.

*/


/**
	Types of extended EFS information queries
*/
typedef enum
{
	/**
	Return the flash device name. 'data' is ignored Results in a string.
	*/
	EFS2_DIAG_EXT_QUERY_DEVICE_NAME			= 0,

	/**
		Return a field of the device ID. 'data' indicates which ID
		field to return. '0' is always the Maker ID, and '1' begins the
		device-specific ID. The client should always request values
		until the FS_DIAG_UNAVAILABLE_INFO is returned, indicating there
		are no more ID values.
	*/
	EFS2_DIAG_EXT_QUERY_DEVICE_ID			= 1,

	/**
		Return the number of blocks in this EFS2 partition. These are
		FLASH blocks (called sectors for NOR). 'data' is ignored.
	*/
	EFS2_DIAG_EXT_QUERY_FILESYSTEM_BLOCKS	= 2,


	/**
		Return the total number of blocks in the flash device containing
		this filesystem. 'data' is ignored.
	*/
	EFS2_DIAG_EXT_QUERY_TOTAL_BLOCKS		= 3,

	/**
		Return the number of pages per block for this device. This
		query is only valid for devices that have a fixed block size.
		For devices with varying block sizes, the TBD request must be
		used. 'data' is ignored.
	*/
	EFS2_DIAG_EXT_QUERY_PAGES_PER_BLOCK		= 4,

	/**
		Return the number of bytes in a single page for this device.
		'data' is ignored'.
	*/
	EFS2_DIAG_EXT_QUERY_PAGE_SIZE			= 5,

	/**
		Return the bus width of the flash device used in this
		filesystem. 'data' is ignored.
	*/
	EFS2_DIAG_EXT_QUERY_FLASH_BUS_WIDTH		= 6,


	/**
		Return string description of flash type ("NAND", "NOR", etc).
	*/
	EFS2_DIAG_EXT_QUERY_FLASH_TYPE			= 7

} EFS2_EXT_QueryType_Enum;


/******************************************************************************
						Diagnostic - NV Subsystem
*******************************************************************************/

/**
	Multiple NV write Subsystem commands
*/
typedef enum
{
	_DIAG_NV_MNVW_Write = 0
} MNVW_CMD_Enum;

// Maximum payload size for multiple NV write.  Leave 100 bytes margin so that HDLC characters
// can be expand with escape sequences.  This still might not be enough if one NV field has
// numerous 0x7e or 0x7d values
#define DIAG_MNVW_MAX_PAYLOAD_SIZE 1520  - 100

/**
	Status types for Multiple NV Write
*/
typedef enum
{
	DIAG_MNVW_Status_OK,				//!<' 0 - Request NV write items completed OK
	DIAG_MNVW_Status_InvalidTotal,		//!<' 1 - Request total item number invalid
	DIAG_MNVW_Status_InvalidLength,		//!<' 2 - Request total item length limit exceeded
	DIAG_MNVW_Status_MismatchLength,	//!<' 3 - Request item lengths mismatched with total item length
	DIAG_MNVW_Status_Error				//!<' 4 - Request NV write items failed with ERROR
} MNVW_Status_Enum;

/**
	Structure for MNV Diag Command response header
*/
typedef struct
{
	byte  iCMD_Code;
	byte  iSubSysId;
	word iSubSysCmdCode;
	word iNvItemTotal;
	word iNvItemTotalLength;
	word iNvStatusTotal;
	byte iNvItemDataTotal;

} MVNW_Response_Struct;

/**
	Structure for individual NV items in the MNV Diag Command response
*/
typedef struct
{
	byte  iNvItemLength;
	word iNvItem;
	byte  iNvItemStatus;
	byte  iNvItemData;
} MVNW_Response_NvItem_Struct;

/**
	CNV Subsystem commands
*/
typedef enum
{
	_DIAG_NV_CNV_DIAG_BACKUP_F = 0
} CNV_CMD_Enum;

/**
	Structure for Diag  Command version 2 responses
*/
typedef struct
{
	byte  iCMD_Code;
	byte  iSubSysId;
	word iSubSysCmdCode;
	dword  iCommandStatus;
	word iDelayedResponseId;
	word iDelayedResponseCount;

} DIAG_CMD_Ver2_Resp;

/**
	For the "command status" result on Diag Version 2 commands.
*/
#define DIAG_V2_SUCCESS 0

/**
	For multiple NV Write,
*/
#define NV_WRITE_BATCH_ITEM_NO_ERROR -1

//	---------------------------------------------------------
//	defines for log codes
//
//	---------------------------------------------------------

//! AAGPS Measurement log
#define _DIAG_LOG_AAGPS_MEASUREMENT 0x7005

//! AAGPS Measurement log version 3
#define _DIAG_LOG_AAGPS_MEASUREMENT_V3 0x701F



/******************************************************************************
						Diagnostic - GSDI Subsystem
*******************************************************************************/

//! GSDI Generic command, for the SUBSYS_CMD_CODE field
#define GSDIDIAG_GENERIC_COMMAND 0X00000000

/**
	GSDIDIAG Commands
*/
typedef enum
{
    //GSDIDIAG_MIN_CMD =-1
    _GSDIDIAG_SIM_READ_CMD                       = 0x00,
    _GSDIDIAG_SIM_WRITE_CMD                      = 0x01,
    _GSDIDIAG_SIM_INCREASE_CMD                   = 0x02,
    _GSDIDIAG_GET_FILE_ATTRIBUTES_CMD            = 0x04,
    _GSDIDIAG_GET_PIN_STATUS_CMD                 = 0x05,
    _GSDIDIAG_VERIFY_PIN_CMD                     = 0x06,
    _GSDIDIAG_CHANGE_PIN_CMD                     = 0x07,
    _GSDIDIAG_UNBLOCK_PIN_CMD                    = 0x08,
    _GSDIDIAG_DISABLE_PIN_CMD                    = 0x09,
    _GSDIDIAG_ENABLE_PIN_CMD                     = 0x0A,
    _GSDIDIAG_GET_SIM_CAPABILITES_CMD            = 0x0C,
    _GSDIDIAG_ILLEGAL_SIM_CMD                    = 0x0F,
    _GSDIDIAG_SIM_SEARCH_CMD                     = 0x10,
    _GSDIDIAG_SELECT_CMD                         = 0x12,
    _GSDIDIAG_GET_IMAGE_CMD                      = 0x14,
    _GSDIDIAG_SIM_REFRESH_CMD                    = 0x15,
    _GSDIDIAG_GET_AVAILABLE_APPS_CMD             = 0x17,
    _GSDIDIAG_ACTIVATE_FEATURE_INDICATOR_CMD     = 0x40,
    _GSDIDIAG_DEACTIVATE_FEATURE_INDICATOR_CMD   = 0x41,
    _GSDIDIAG_GET_FEATURE_INDICATION_CMD         = 0x42,
    _GSDIDIAG_SET_FEATURE_INDICATOR_DATA_CMD     = 0x43,
    _GSDIDIAG_GET_FEATURE_INDICATOR_KEY_CMD      = 0x44,
    _GSDIDIAG_UNBLOCK_FEATURE_INDICATOR_CMD      = 0x45,
    _GSDIDIAG_GET_PERM_FEATURE_INDICATION_CMD    = 0x46,
    _GSDIDIAG_PERM_DISABLE_FEATURE_INDICATOR_CMD = 0x47,
    _GSDIDIAG_GENERIC_CMD                        = 0x48,
    _GSDIDIAG_GET_ATR_CMD                        = 0x49,
    _GSDIDIAG_SIM_OPEN_CHANNEL_CMD               = 0x50,
    _GSDIDIAG_SIM_CLOSE_CHANNEL_CMD              = 0x51,
    _GSDIDIAG_SIM_SEND_CHANNEL_DATA_CMD          = 0x52,
    _GSDIDIAG_ATCSIM_RAW_APDU_CMD                = 0x53,
    _GSDIDIAG_SIM_USIM_RUIM_SEARCH_CMD           = 0x54,
    _GSDIDIAG_GET_ECC_CMD                        = 0x55,
    _GSDIDIAG_SECURE_DEVICE_INFO_CMD             = 0x56,
    _GSDIDIAG_SIM_SECURE_STORE_CMD               = 0x57,
    _GSDIDIAG_ROOT_KEY_WRITE_CMD                 = 0x58,
    _GSDIDIAG_ROOT_KEY_READ_CMD                  = 0x59,
    _GSDIDIAG_SIM_SECURE_READ_CMD                = 0x5A,
    _GSDIDIAG_VERIFY_PHONE_CODE_CMD              = 0x5B,
    _GSDIDIAG_CHANGE_PHONE_CODE_CMD              = 0x5C,
    _GSDIDIAG_UNBLOCK_PHONE_CODE_CMD             = 0x5D,
    _GSDIDIAG_SET_PHONE_CODE_CMD                 = 0x5E,
    _GSDIDIAG_GET_LIB_VERSION_CMD                = 0x5F,
	_GSDIDIAG_ACTIVATE_ONCHIP_SIM_CMD            = 0x61,
    _GSDIDIAG_SEND_APDU_CMD                      = 0x99,
	_GSDIDIAG_GET_FEATURE_INDICATOR_DATA_CMD     = 0xAE,
    _GSDIDIAG_MAX_CMD                            = 0xFF

} GSDI_DIAG_CMD_Enum;

/**
	Values for the APS_AS_TRANSPORT field
*/
typedef enum
{
	GSDI_DIAG_APS_As_Transport_CmdResponse,	//!< ' Response is in Command Response Header (Pass or Fail)
	GSDI_DIAG_APS_As_Transport_DelayedResponse	//!< ' Response is in the Response Payload.
} GSDI_DIAG_APS_AsTransport_Enum;


/**
	Header for the GSDI Response
\code
	Field				Length
	------				------
	CMD_CODE 				1
	SUBSYS_ID				1
	SUBSYS_CMD_CODE			2
	COMMAND_STATUS			4
	DELAYED_RESPONSE_ ID	2
	DELAYED_RESPONSE_COUNT	2
	SUBCOMMAND				4
	APS_AS_TRANSPORT		4
	STATUS					4  <--- This one seems to be missing

	typedef PACKED struct gsdidiag_generic_cmd_rsp_type
	{
		typedef PACKED struct  gsdidiag_rsp_header_type
		{
			uint8  command_code;
			uint8  subsys_id;
			uint16 subsys_cmd_code;
			uint32 status;
			uint16 delayed_rsp_id;
			uint16 rsp_count;
			uint32 aps_as_transport;
		} rsp_header;

		typedef PACKED struct
		{
		   uint32 subcommand;
		   uint32 gsdi_status;
		}gsdidiag_subcmd_rsp_hdr_type;


		PACKED union gsdidiag_rsp_payload_type
		{
		   gsdidiag_rsp_data_payload_type         data;
		   gsdidiag_rsp_channel_data_payload_type channel_data;
		   gsdidiag_rsp_atcsim_payload_type       atcsim_data;
		   gsdidiag_rsp_sim_read_payload_type     sim_read_data;
		   gsdidiag_rsp_sim_get_ecc_type          ecc_data;
		   uint8 num_retries;
		   byte rec_num_accessed;
		   gsdidiag_root_key_rsp_type             root_key_write;
		   gsdidiag_root_key_rsp_type             root_key_read;
		   gsdidiag_sim_secure_read_rsp_type      sim_secure_read;
		   gsdidiag_get_lib_version_rsp_type      lib_version;
		}rsp_payload;

	} response;
\endcode
*/
typedef struct
{
	byte  iCMD_Code;
	byte  iSubSysId;
	word iSubSysCmdCode;
	dword  iCommandStatus;
	word iDelayedResponseId;
	word iDelayedResponseCount;
	dword  iApsAsTransport;
	dword  iSubCommand;
	dword  iStatus;


} GSDI_DIAG_DelayedResponseHeader;

/*
	GSDI Request Header

	typedef PACKED struct
	{
		typedef PACKED struct gsdidiag_sim_get_file_attributes_cmd_req_type
		{
			typedef PACKED struct gsdidiag_cmd_header_type
			{
				uint8 command_code;
				uint8 subsys_id;
				uint16 subsys_cmd_code;
			} header;

			uint8  sim_slot_number;
			uint8  sim_app_id_len;
			uint8  sim_app_id[GSDI_MAX_APP_ID_LEN];
			uint32 sim_elementary_file;
			char   efs_filename[1];
		} cmd_header;

		typedef PACKED struct gsdidiag_req_payload_type
		{
			typedef PACKED struct gsdidiag_subcmd_req_hdr_type
			{
			   uint32 subcommand;
			   uint32 gsdi_status;
			} sub_cmd_hdr;

			PACKED union
			{
				gsdidiag_two_pin_cmd_type      change_pin;
				gsdidiag_channel_req_cmd_type  channel_req;
				gsdidiag_sim_read_req_cmd_type sim_read_req;
				gsdidiag_sim_usim_ruim_search  sim_usim_ruim_search_req;
				gsdidiag_sim_secure_store_cmd_type sim_secure_store_req;
				gsdidiag_root_key_write_req_cmd_type root_key_write_req;
				gsdidiag_verify_phone_code_cmd_type  verify_phone_code_req;
				gsdidiag_change_phone_code_cmd_type  change_phone_code_req;
				gsdidiag_unblock_phone_code_cmd_type unblock_phone_code_req;
				gsdidiag_set_phone_code_cmd_type     set_phone_code_req;
			}request;
		}req_payload;



	}gsdidiag_generic_cmd_req_type;

*/


/**
	GSDI Response structure for commands that are not "delayed response"
*/
typedef struct
{
	byte  iCMD_Code;
	byte  iSubSysId;
	word iSubSysCmdCode;
	dword  iCommandStatus;
	word iTransactionID;
	byte iSIM_SlotNumber;

} GSDI_DIAG_ResponseHeader;


/**
	Enum of status values for GSDI commands
*/
typedef enum
{
	 GSDI_DIAG_STAT_Success				= 0,  //!< ' COMMAND COMPLETED SUCCESSFULLY
	 GSDI_DIAG_STAT_GeneralError		= 1,  //!< ' GENERAL ERROR:  No other information specified
	 GSDI_DIAG_STAT_AccessDenied		= 2,  //!< ' ACCESS DENIED:  Security Procedures performed thus far don't satisfy restrictions
	 GSDI_DIAG_STAT_NotFound			= 3,  //!< ' NOT FOUND:  SIM Elementary File or Directory File was not found.
	 GSDI_DIAG_STAT_IncompatPinStauts	= 4,  //!< ' INCOMPATIBLE PIN STATUS:  Trying to perform a PIN Action for a PIN that is not in the correct state.  For example, trying to Disable an already Disabled PIN.
	 GSDI_DIAG_STAT_IncorrectCode		= 5,  //!< ' INCORRECT CODE:  The Code provided is incorrect for the PIN ID used.
	 GSDI_DIAG_STAT_CodeBlocked			= 6,  //!< ' CODE BLOCKED:  The Number of retries has been exceeded and the PIN is now blocked.
	 GSDI_DIAG_STAT_IncreaseImpossible	= 7,  //!< ' INCREASE IMPOSSIBLE:  The INCREASE Command has failed due to a Bad Value
	 GSDI_DIAG_STAT_IncorrectParameters = 8,  //!< ' INCORRECT PARAMETERS:  The SIM detected an incorrect parameter for the action performed.
	 GSDI_DIAG_STAT_NotSupported		= 9,  //!< ' NOT SUPPORTED:  Not allowed by GSDI due to Service Table Values
	 GSDI_DIAG_STAT_NotInitialized		= 10, //!< ' NOT INIT:  Cache has not been INIT
	 GSDI_DIAG_STAT_IllegalSim			= 11, //!< ' SUCCESS BUT ILLEGAL SIM:  The SIM has been marked as an illegal SIM for the network, but the actions are succeeding on the SIM.
	 GSDI_DIAG_STAT_SimTechnicalProblems = 14,//!< ' SIM TECHNICAL PROBLEMS:  The SIM malfunctioned.
	 GSDI_DIAG_STAT_NoEfSelected		= 15, //!< ' NO EF SELECTED:  Attempted to read a file without first selecting it.
	 GSDI_DIAG_STAT_EfInconsistent		= 16, //!< ' EF INCONSISTENT:  Trying to Read a Record when the file is a Binary or similar type error.
	 GSDI_DIAG_STAT_UnknownInstClass	= 17, //!< ' UNKNOWN INST CLASS:  Trying to send a UICC Command to a GSM/RUIM Card or vice versa.
	 GSDI_DIAG_STAT_IncorrectLength		= 21, //!< ' INCORRECT LENGTH:  A Bad length was provided in the command.
	 GSDI_DIAG_STAT_DriversTimedOut		= 22, //!< ' DRIVERS TIMED OUT:  The Drivers communicating with the Card Timed Out.
	 GSDI_DIAG_STAT_CodePermBlocked		= 23  //!< ' CODE PERM BLOCKED:  The Unblock Code has been blocked.  Need Administrative assistance to unblock it.
} GSDI_DIAG_StatusEnum;

/**
	For the "command status" result on GSDI commands.
*/
#define GSDI_SUCCESS DIAG_V2_SUCCESS

/**
	Enum of dealyed response counts GSDI commands
*/
typedef enum
{
	GSDI_DIAG_ResponseCount_First				= 0,	//!< ' 0 - First Response (Immediate)
	GSDI_DIAG_ResponseCount_Delayed				= 1		//!< ' 1 - Second Response  (Delayed)

} GSDI_DIAG_ResponseCountEnum;

/**
	Enum for GSDI SIM Index mode

	Copied from embedded source code file gsdidiag.h
*/
typedef enum
{
	GSDI_DIAG_IndexNone		= 0x00000000,
	GSDI_DIAG_IndexAbsolute = 0x00000001,
	GSDI_DIAG_IndexCurrent	= 0x00000002,
	GSDI_DIAG_IndexNext		= 0x00000003,
	GSDI_DIAG_IndexPrevious = 0x00000004
} GSDI_DIAG_SIM_IndexMode;


/**
    Enumerated type:  gsdidiag_elementary_file_enum_type
      Define Filenames Available through GSDI - DIAG Interface
      CDMA RUIM Related Files Range:  0x00000001 - 0x000000FF
      GSM  SIM  Related Files Range:  0x00001000 - 0x0000FF00
      UMTS USIM Related Files Range:  0x00010000 - 0x00FF0000
      TELECOM   Related Files Range:  0x01000000 - 0xFF000000

  Copied from embedded source code file gsdidiag.h
*/

typedef enum
{
    //GSDIDIAG_MIN_CMD =-1

    GSDIDIAG_MF_ICCID                             = 0x00000001,
    GSDIDIAG_MF_ELP                               = 0x00000002,
    GSDIDIAG_MF_DIR                               = 0x00000003,
    GSDIDIAG_MF_ARR                               = 0x00000004,
    GSDIDIAG_MF_PL                                = 0x00000005,

    GSDIDIAG_CDMA_CC                              = 0x00000100,
    GSDIDIAG_CDMA_IMSI_M                          = 0x00000101,
    GSDIDIAG_CDMA_IMSI_T                          = 0x00000102,
    GSDIDIAG_CDMA_ANALOG_HOME_SID                 = 0x00000103,
    GSDIDIAG_CDMA_ANALOG_OP_PARAMS                = 0x00000104,
    GSDIDIAG_CDMA_ANALOG_LOCN_AND_REGN_IND        = 0x00000105,
    GSDIDIAG_CDMA_HOME_SID_NID                    = 0x00000106,
    GSDIDIAG_CDMA_ZONE_BASED_REGN_IND             = 0x00000107,
    GSDIDIAG_CDMA_SYS_REGN_IND                    = 0x00000108,
    GSDIDIAG_CDMA_DIST_BASED_REGN_IND             = 0x00000109,
    GSDIDIAG_CDMA_ACCOLC                          = 0x0000010A,
    GSDIDIAG_CDMA_CALL_TERM_MOD_PREF              = 0x0000010B,
    GSDIDIAG_CDMA_SCI                             = 0x0000010C,
    GSDIDIAG_CDMA_ANALOG_CHAN_PREF                = 0x0000010D,
    GSDIDIAG_CDMA_PRL                             = 0x0000010E,
    GSDIDIAG_CDMA_RUIM_ID                         = 0x0000010F,
    GSDIDIAG_CDMA_SVC_TABLE                       = 0x00000110,
    GSDIDIAG_CDMA_SPC                             = 0x00000111,
    GSDIDIAG_CDMA_OTAPA_SPC_ENABLE                = 0x00000112,
    GSDIDIAG_CDMA_NAM_LOCK                        = 0x00000113,
    GSDIDIAG_CDMA_OTASP_OTAPA_FEATURES            = 0x00000114,
    GSDIDIAG_CDMA_SERVICE_PREF                    = 0x00000115,
    GSDIDIAG_CDMA_ESN_ME                          = 0x00000116,
    GSDIDIAG_CDMA_RUIM_PHASE                      = 0x00000117,
    GSDIDIAG_CDMA_PREF_LANG                       = 0x00000118,
    GSDIDIAG_CDMA_UNASSIGNED_1                    = 0x00000119,
    GSDIDIAG_CDMA_SMS                             = 0x0000011A,
    GSDIDIAG_CDMA_SMS_PARAMS                      = 0x0000011B,
    GSDIDIAG_CDMA_SMS_STATUS                      = 0x0000011C,
    GSDIDIAG_CDMA_SUP_SVCS_FEATURE_CODE_TABLE     = 0x0000011D,
    GSDIDIAG_CDMA_UNASSIGNED_2                    = 0x0000011E,
    GSDIDIAG_CDMA_HOME_SVC_PVDR_NAME              = 0x0000011F,
    GSDIDIAG_CDMA_UIM_ID_USAGE_IND                = 0x00000120,
    GSDIDIAG_CDMA_ADM_DATA                        = 0x00000121,
    GSDIDIAG_CDMA_MSISDN                          = 0x00000122,
    GSDIDIAG_CDMA_MAXIMUM_PRL                     = 0x00000123,
    GSDIDIAG_CDMA_SPC_STATUS                      = 0x00000124,

    GSDIDIAG_GSM_LP                               = 0x00000200,
    GSDIDIAG_GSM_IMSI                             = 0x00000201,
    GSDIDIAG_GSM_KC                               = 0x00000202,
    GSDIDIAG_GSM_PLMN                             = 0x00000203,
    GSDIDIAG_GSM_HPLMN                            = 0x00000204,
    GSDIDIAG_GSM_ACM_MAX                          = 0x00000205,
    GSDIDIAG_GSM_SST                              = 0x00000206,
    GSDIDIAG_GSM_ACM                              = 0x00000207,
    GSDIDIAG_GSM_GID1                             = 0x00000208,
    GSDIDIAG_GSM_GDI2                             = 0x00000209,
    GSDIDIAG_GSM_SPN                              = 0x0000020A,
    GSDIDIAG_GSM_PUCT                             = 0x0000020B,
    GSDIDIAG_GSM_CBMI                             = 0x0000020C,
    GSDIDIAG_GSM_BCCH                             = 0x0000020D,
    GSDIDIAG_GSM_ACC                              = 0x0000020E,
    GSDIDIAG_GSM_FPLMN                            = 0x0000020F,
    GSDIDIAG_GSM_LOCI                             = 0x00000210,
    GSDIDIAG_GSM_AD                               = 0x00000211,
    GSDIDIAG_GSM_PHASE                            = 0x00000222,
    GSDIDIAG_GSM_VGCS                             = 0x00000223,
    GSDIDIAG_GSM_VGCSS                            = 0x00000224,
    GSDIDIAG_GSM_VBS                              = 0x00000225,
    GSDIDIAG_GSM_VBSS                             = 0x00000226,
    GSDIDIAG_GSM_EMLPP                            = 0x00000227,
    GSDIDIAG_GSM_AAEM                             = 0x00000228,
    GSDIDIAG_GSM_CBMID                            = 0x00000229,
    GSDIDIAG_GSM_ECC                              = 0x0000022A,
    GSDIDIAG_GSM_CBMIR                            = 0x0000022B,
    GSDIDIAG_GSM_DCK                              = 0x0000022C,
    GSDIDIAG_GSM_CNL                              = 0x0000022D,
    GSDIDIAG_GSM_NIA                              = 0x0000022E,
    GSDIDIAG_GSM_KCGPRS                           = 0x0000022F,
    GSDIDIAG_GSM_LOCIGPRS                         = 0x00000230,
    GSDIDIAG_GSM_SUME                             = 0x00000231,
    GSDIDIAG_GSM_PLMNWAT                          = 0x00000232,
    GSDIDIAG_GSM_OPLMNWACT                        = 0x00000233,
    GSDIDIAG_GSM_CPBCCH                           = 0x00000234,
    GSDIDIAG_GSM_INVSCAN                          = 0x00000235,
    GSDIDIAG_GSM_RPLMNACT                         = 0x00000236,
    GSDIDIAG_GSM_SAI                              = 0x00000237,
    GSDIDIAG_GSM_SLL                              = 0x00000238,
    GSDIDIAG_GSM_MEXE_ST                          = 0x00000239,
    GSDIDIAG_GSM_ORPK                             = 0x0000023A,
    GSDIDIAG_GSM_ARPK                             = 0x0000023B,
    GSDIDIAG_GSM_TPRPK                            = 0x0000023C,
    GSDIDIAG_GSM_IMG                              = 0x0000023D,
    GSDIDIAG_GSM_IMG1INST1                        = 0x0000023E,
    GSDIDIAG_GSM_IMG1INST2                        = 0x0000023F,
    GSDIDIAG_GSM_IMG1INST3                        = 0x00000240,
    GSDIDIAG_GSM_IMG2INST1                        = 0x00000241,
    GSDIDIAG_GSM_IMG2INST2                        = 0x00000242,
    GSDIDIAG_GSM_IMG2INST3                        = 0x00000243,
    GSDIDIAG_GSM_IMG3INST1                        = 0x00000244,
    GSDIDIAG_GSM_IMG3INST2                        = 0x00000245,
    GSDIDIAG_GSM_IMG3INST3                        = 0x00000246,
    GSDIDIAG_GSM_HPLMNWACT                        = 0x00000247,

    GSDIDIAG_TELECOM_ADN                          = 0x00000300,
    GSDIDIAG_TELECOM_FDN                          = 0x00000301,
    GSDIDIAG_TELECOM_SMS                          = 0x00000302,
    GSDIDIAG_TELECOM_CCP                          = 0x00000303,
    GSDIDIAG_TELECOM_ECCP                         = 0x00000304,
    GSDIDIAG_TELECOM_MSISDN                       = 0x00000305,
    GSDIDIAG_TELECOM_SMSP                         = 0x00000306,
    GSDIDIAG_TELECOM_SMSS                         = 0x00000307,
    GSDIDIAG_TELECOM_LND                          = 0x00000308,
    GSDIDIAG_TELECOM_SDN                          = 0x00000309,
    GSDIDIAG_TELECOM_EXT1                         = 0x0000030A,
    GSDIDIAG_TELECOM_EXT2                         = 0x0000030B,
    GSDIDIAG_TELECOM_EXT3                         = 0x0000030C,
    GSDIDIAG_TELECOM_BDN                          = 0x0000030D,
    GSDIDIAG_TELECOM_EXT4                         = 0x0000030E,
    GSDIDIAG_TELECOM_SMSR                         = 0x0000030F,
    GSDIDIAG_TELECOM_CMI                          = 0x00000310,
    GSDIDIAG_TELECOM_SUME                         = 0x00000311,
    GSDIDIAG_TELECOM_ARR                          = 0x00000312,
    GSDIDIAG_TELECOM_PBR                          = 0x00000313,
    GSDIDIAG_TELECOM_CCP1                         = 0x00000314,
    GSDIDIAG_TELECOM_UID                          = 0x00000315,
    GSDIDIAG_TELECOM_PSC                          = 0x00000316,
    GSDIDIAG_TELECOM_CC                           = 0x00000317,
    GSDIDIAG_TELECOM_PUID                         = 0x00000318,
    GSDIDIAG_TELECOM_ADN1                         = 0x00000319,
    GSDIDIAG_TELECOM_GRP                          = 0x0000031A,
    GSDIDIAG_TELECOM_GRP1                         = 0x0000031B,
    GSDIDIAG_TELECOM_GAS                          = 0x0000031C,
    GSDIDIAG_TELECOM_SNE                          = 0x0000031D,
    GSDIDIAG_TELECOM_SNE1                         = 0x0000031E,
    GSDIDIAG_TELECOM_EMAIL                        = 0x0000031F,
    GSDIDIAG_TELECOM_EMAIL1                       = 0x00000320,
    GSDIDIAG_TELECOM_IAP                          = 0x00000321,
    GSDIDIAG_TELECOM_IAP1                         = 0x00000322,
    GSDIDIAG_TELECOM_PBC                          = 0x00000323,

    GSDIDIAG_TELECOM_GRAPHICS_IMG                 = 0x00000400,

    GSDIDIAG_USIM_LI                              = 0x00000500,
    GSDIDIAG_USIM_IMSI                            = 0x00000501,
    GSDIDIAG_USIM_KEYS                            = 0x00000502,
    GSDIDIAG_USIM_KEYSPS                          = 0x00000503,
    GSDIDIAG_USIM_PLMNWACT                        = 0x00000504,
    GSDIDIAG_USIM_UPLMNSEL                        = 0x00000505,
    GSDIDIAG_USIM_HPLMN                           = 0x00000506,
    GSDIDIAG_USIM_ACM_MAX                         = 0x00000507,
    GSDIDIAG_USIM_UST                             = 0x00000508,
    GSDIDIAG_USIM_ACM                             = 0x00000509,
    GSDIDIAG_USIM_GID1                            = 0x0000050A,
    GSDIDIAG_USIM_GID2                            = 0x0000050B,
    GSDIDIAG_USIM_SPN                             = 0x0000050C,
    GSDIDIAG_USIM_PUCT                            = 0x0000050D,
    GSDIDIAG_USIM_CMBI                            = 0x0000050E,
    GSDIDIAG_USIM_ACC                             = 0x0000050F,
    GSDIDIAG_USIM_FPLMN                           = 0x00000510,
    GSDIDIAG_USIM_LOCI                            = 0x00000511,
    GSDIDIAG_USIM_AD                              = 0x00000512,
    GSDIDIAG_USIM_CBMID                           = 0x00000513,
    GSDIDIAG_USIM_ECC                             = 0x00000514,
    GSDIDIAG_USIM_CBMIR                           = 0x00000515,
    GSDIDIAG_USIM_PSLOCI                          = 0x00000516,
    GSDIDIAG_USIM_FDN                             = 0x00000517,
    GSDIDIAG_USIM_SMS                             = 0x00000518,
    GSDIDIAG_USIM_MSISDN                          = 0x00000519,
    GSDIDIAG_USIM_SMSP                            = 0x0000051A,
    GSDIDIAG_USIM_SMSS                            = 0x0000051B,
    GSDIDIAG_USIM_SDN                             = 0x0000051C,
    GSDIDIAG_USIM_EXT2                            = 0x0000051D,
    GSDIDIAG_USIM_EXT3                            = 0x0000051E,
    GSDIDIAG_USIM_SMSR                            = 0x0000051F,
    GSDIDIAG_USIM_ICI                             = 0x00000520,
    GSDIDIAG_USIM_OCI                             = 0x00000521,
    GSDIDIAG_USIM_ICT                             = 0x00000522,
    GSDIDIAG_USIM_OCT                             = 0x00000523,
    GSDIDIAG_USIM_EXT5                            = 0x00000524,
    GSDIDIAG_USIM_CCP2                            = 0x00000525,
    GSDIDIAG_USIM_EMLPP                           = 0x00000526,
    GSDIDIAG_USIM_AAEM                            = 0x00000527,
    GSDIDIAG_USIM_GMSI                            = 0x00000528,
    GSDIDIAG_USIM_HIDDENKEY                       = 0x00000529,
    GSDIDIAG_USIM_BDN                             = 0x0000052A,
    GSDIDIAG_USIM_EXT4                            = 0x0000052B,
    GSDIDIAG_USIM_CMI                             = 0x0000052C,
    GSDIDIAG_USIM_EST                             = 0x0000052D,
    GSDIDIAG_USIM_ACL                             = 0x0000052E,
    GSDIDIAG_USIM_DCK                             = 0x0000052F,
    GSDIDIAG_USIM_CNL                             = 0x00000530,
    GSDIDIAG_USIM_START_HFN                       = 0x00000531,
    GSDIDIAG_USIM_THRESHOLD                       = 0x00000532,
    GSDIDIAG_USIM_OPLMNWACT                       = 0x00000533,
    GSDIDIAG_USIM_OPLMNSEL                        = 0x00000534,
    GSDIDIAG_USIM_HPLMNWACT                       = 0x00000535,
    GSDIDIAG_USIM_ARR                             = 0x00000536,
    GSDIDIAG_USIM_RPLMNACT                        = 0x00000537,
    GSDIDIAG_USIM_NETPAR                          = 0x00000538,
    GSDIDIAG_USIM_ADN                             = 0x00000539,
    GSDIDIAG_USIM_ADN1                            = 0x0000053A,
    GSDIDIAG_USIM_GAS                             = 0x0000053B,
    GSDIDIAG_USIM_GAS1                            = 0x0000053C,
    GSDIDIAG_USIM_GRP1                            = 0x0000053D,
    GSDIDIAG_USIM_SNE                             = 0x0000053E,
    GSDIDIAG_USIM_SNE1                            = 0x0000053F,
    GSDIDIAG_USIM_EMAIL                           = 0x00000540,
    GSDIDIAG_USIM_EMAIL1                          = 0x00000541,
    GSDIDIAG_USIM_IAP                             = 0x00000542,
    GSDIDIAG_USIM_IAP1                            = 0x00000543,

    GSDIDIAG_USIM_PHONEBOOK_PBR                   = 0x00000600,
    GSDIDIAG_USIM_PHONEBOOK_CCP1                  = 0x00000601,
    GSDIDIAG_USIM_PHONEBOOK_UID                   = 0x00000602,
    GSDIDIAG_USIM_PHONEBOOK_PSC                   = 0x00000603,
    GSDIDIAG_USIM_PHONEBOOK_CC                    = 0x00000604,
    GSDIDIAG_USIM_PHONEBOOK_PUID                  = 0x00000605,
    GSDIDIAG_USIM_PHONEBOOK_PBC                   = 0x00000606,

    GSDIDIAG_USIM_GSMACCESS_KC                    = 0x00000700,
    GSDIDIAG_USIM_GSMACCESS_KCGPRS                = 0x00000701,
    GSDIDIAG_USIM_GSMACCESS_CPBCCH                = 0x00000702,
    GSDIDIAG_USIM_GSMACCESS_INVSCAN               = 0x00000703,

    GSDIDIAG_USIM_MEXE_MEXE_ST                    = 0x00000800,
    GSDIDIAG_USIM_MEXE_ORPK                       = 0x00000801,
    GSDIDIAG_USIM_MEXE_ARPK                       = 0x00000802,
    GSDIDIAG_USIM_MEXE_TPRPK                      = 0x00000803
  //reserved
} GSDI_DIAG_SIM_ElementaryFile_Enum;


/**
Enumerated type for GSDI SIM Simple Req

    Defines Index Modes Available for Commands.
        0 = User specified values for SIM_INDEX_MODE, SIM_
            OFFSET, SIM_REC_NUM, SIM_DATA_LEN: user has to
            deal with chunking: for record based files, user has to
            specify SIM_REC_NUM
        1 = Phone uses appropriate values; also returns the
            complete data in a file; user is transparent to chunking

	Copied from embedded source code file gsdidiag.h

*/

typedef enum
{
    GSDIDIAG_SIM_SIMPLE_REQ_ONE_STEP = 0x00,	// Mobile software performs only one step of preparing data
    GSDIDIAG_SIM_SIMPLE_REQ_ALL_STEPS = 0x01	// Mobile software merges all data into a complete file

}GSDI_DIAG_SimpleReq_Enum;

/**
Enumerated type for GSDI indexing modes

    Defines Index Modes Available for Commands.
    GSDIDIAG_NO_INDEX        :  Applicable to Binary/Transparent Files
    GSDIDIAG_INDEX_ABSOLUTE  :  Applicable to Linear Fixed / Cyclic Files
    GSDIDIAG_INDEX_NEXT      :  Applicable to Cyclic Files
    GSDIDIAG_INDEX_PREVIOUS  :  Applicable to Cyclic Files

	Copied from embedded source code file gsdidiag.h

*/

typedef enum
{
    GSDIDIAG_NO_INDEX       = 0x00,
    GSDIDIAG_INDEX_ABSOLUTE = 0x01,
    GSDIDIAG_INDEX_CURRENT  = 0x02,
    GSDIDIAG_INDEX_NEXT     = 0x03,
    GSDIDIAG_INDEX_PREVIOUS = 0x04,

}GSDI_DIAG_IndexModes_Enum;


/**
    Enumerated personality types to be used with GSDI functions.

	Copied from embedded source code file gsdidiag.h
*/
typedef enum
{
    GSDIDIAG_PERSO_NW      = 0x00,
    GSDIDIAG_PERSO_NS      = 0x01,
    GSDIDIAG_PERSO_SP      = 0x02,
    GSDIDIAG_PERSO_CP      = 0x03,
    GSDIDIAG_PERSO_SIM     = 0x04
} GSDI_DIAG_PersoFeature_Enum;


/**
    Enumerated personality types to be used with GSDI functions.

	Copied from embedded source code file gsdidiag.h
*/
typedef enum
{
	EVENT_GSDI_GET_FILE_ATTRIBUTES	= 999,
	EVENT_GSDI_SIM_READ				= 1000,
	EVENT_GSDI_SIM_WRITE			= 1001,
	EVENT_GSDI_GET_PIN_STATUS		= 1002,
	EVENT_GSDI_VERIFY_PIN			= 1003,
	EVENT_GSDI_UNBLOCK_PIN			= 1004,
	EVENT_GSDI_DISABLE_PIN			= 1005,
	EVENT_GSDI_ENABLE_PIN			= 1006,
	EVENT_GSDI_SIM_INCREASE			= 1007,
	EVENT_GSDI_EXECUTE_APDU_REQ		= 1008,
	EVENT_GSDI_ACTIVATE_FEATURE_IND	= 1037,
	EVENT_GSDI_DEACTIVATE_FEATURE_IND	= 1038,
	EVENT_GSDI_GET_FEATURE_IND			= 1039,
	EVENT_GSDI_SET_FEATURE_DATA			= 1040,
	EVENT_GSDI_UNBLOCK_FEATURE_IND		= 1041,
	EVENT_GSDI_GET_CONTROL_KEY			= 1042,
	EVENT_GSDI_OTA_DEPERSO				= 1043,
	EVENT_GSDI_GET_PERM_FEATURE_IND		= 1044,
	EVENT_GSDI_PERM_DISBALE_FEATURE_IND	= 1045

} GSDI_DIAG_EventId;


/**
	Structure for parsing GSDI event responses
*/
typedef struct
{
	unsigned long CMD_STATUS;		//!<' GSDI_SUCCESS = success, otherwise failure codes in chapter 5 of GSDI ICD
	unsigned short TRANSACTION_ID;	//!<' Transaction ID returned from request response.
	unsigned char data[1];			//!<' First data element of data the other data elments
} GSDIDIAG_EventHeader;

/**
	Enum for GSDI SIM Applications

*/
typedef enum
{
	GSDI_DIAG_Apps_Telecom	= 0x01,
	GSDI_DIAG_Apps_GSM		= 0x02,
	GSDI_DIAG_Apps_USIM		= 0x04,
	GSDI_DIAG_Apps_RUIM		= 0x08,
	GSDI_DIAG_Apps_EMV		= 0x10,
	GSDI_DIAG_Apps_DCS1800	= 0x20
} GSDI_DIAG_SIM_AppsList;


/******************************************************************************
						Diagnostic - Streaming Download subsystem
*******************************************************************************/
/**
	 definitions of legal streaming download protocol commands
*/
typedef enum
{
	HELLO_CMD =          0x1,				//!< ' Hello command                 - sent */
	HELLO_RSP_CMD =      0x2,				//!< ' Hello command response        - recv */
	READ_CMD =           0x3,				//!< ' Read command                  - sent */
	READ_RSP_CMD =       0x4,				//!< ' Read command response         - recv */
	S_WRITE_CMD =        0x5,				//!< ' Simple write command          - sent */
	S_WRITE_RSP_CMD =    0x6,				//!< ' Simple write command response - recv */
	STRM_WRITE_CMD =     0x7,				//!< ' Stream write command          - sent */
	STRM_WRITE_RSP_CMD = 0x8,				//!< ' Stream write command response - recv */
	NOP_CMD =            0x9,				//!< ' NOP command                   - sent */
	NOP_RSP_CMD =        0xa,				//!< ' NOP command response          - recv */
	RESET_CMD =          0xb,				//!< ' Reset command                 - sent */
	RESET_RSP_CMD =      0xc,				//!< ' Reset command response        - recv */
	ERROR_CMD =          0xd,				//!< ' Error response                - recv */
	LOG_CMD =            0xe,				//!< ' Log response                  - recv */
	UNLOCK_CMD =         0xf,				//!< ' Unlock command                - sent */
	UNLOCK_RSP_CMD =    0x10,				//!< ' Unlock command response       - recv */
	POWER_OFF_CMD =     0x11,				//!< ' Power off command             - sent */
	POWER_OFF_RSP_CMD = 0x12,				//!< ' Power off command response    - recv */
	DWNLDR_OPEN_CMD =   0x13,				//!< ' Open downloader               - sent */
	DWNLDR_OPEN_RSP_CMD = 0x14,				//!< ' Open downloader response      - recv */
	DWNLDR_CLOSE_CMD =  0x15,				//!< ' Close downloader              - sent */
	DWNLDR_CLOSE_RSP_CMD = 0x16,			//!< ' Close downloader response     - recv */
	DWNLDR_SECURITY_MODE_CMD = 0x17,		//!< ' Multi-image security mode          - sent
	DWNLDR_SECURITY_MODE_RSP_CMD = 0x18,	//!< ' Multi-image security mode response - recv
	DWNLDR_PRTN_TBL_CMD = 0x19,				//!< ' Multi-image partition command      - sent
	DWNLDR_PRTN_TBL_RSP_CMD = 0x1A,			//!< ' Multi-image partition response     - recv
	DWNLDR_MULTI_IMG_OPEN_CMD = 0x1B,		//!< ' Multi-image open command           - sent
	DWNLDR_MULTI_IMG_OPEN_RSP_CMD = 0x1C,	//!< ' Multi-image open response          - recv
	DWNLDR_FLASH_ERASE_CMD = 0x1D,			//!< ' Erase flash (multi- to single- d/l) cmd
	DWNLDR_FLASH_ERASE_RSP_CMD = 0x1E		//!< ' Erase flash response

} Download_Cmd_Enum;

/******************************************************************************
						Diagnostic - GPS Subsystem
*******************************************************************************/


/*
	GPS sub system commands, from Services\LSM\diag_pdapi.h
*/
typedef enum
{
	_GPS_START_CMD				= 0x0000,
	_GPS_END_CMD				= 0x0001,
	_GPS_STATE_CMD				= 0x0002,
	_GPS_SET_NV_PARAM_CMD		= 0x0003,
	_GPS_GET_LAST_KNOWN_POS_CMD	= 0x0004,

	_GPS_GET_PDSM_PARAMS_CMD	= 0x0006,
	_GPS_PDSM_TCP_SOCKET_CMD	= 0x0007,
	_GPS_DIAG_START_TASK		= 0x0008,
	_GPS_DIAG_END_TASK			= 0x000C,
	_GPS_DIAG_TEST_API_CMD		= 0x000D,
	_GPS_NOTIFY_VERIFY_SEND_USER_ACTION		=	0x0010,
	_GPS_API_CMD				= 0x0011,
	_GPS_SET_TEST_MODE_PARAMS	= 0x0012,
	_GPS_USE_THIS_POS			= 0x0013

} GPS_Cmd_Enum;



/******************************************************************************
						Diagnostic - Call Manager subsytem
*******************************************************************************/


//! Call states, used by QLIB_DIAG_GSM_STATUS_F and QLIB_GetPhoneCallState
typedef enum
{

  CM_CALL_STATE_NONE	= -1,		//!< 'FOR INTERNAL USE OF CM ONLY!
  CM_CALL_STATE_IDLE	= 0,		//!< ' Call is in idle state - i.e. no call
  CM_CALL_STATE_ORIG	= 1,		//!< ' Call is in origination state
  CM_CALL_STATE_INCOM	= 3,		//!< ' Call is in alerting state
  CM_CALL_STATE_CC_IN_PROGRESS = 4,	//!< ' Call is originating but waiting for call control to complete
  CM_CALL_STATE_CONV	= 5,		//!< ' Call is in conversation state
  CM_CALL_STATE_MAX					//!< ' FOR INTERNAL USE OF CM ONLY!

} cm_call_state_enum;


//! System operating modes, from services/cm/sys.h
typedef enum
{
  SYS_OPRT_MODE_NONE	= -1,		//!< ' FOR INTERNAL USE OF CM ONLY!
  SYS_OPRT_MODE_PWROFF	= 0,		//!< ' Phone is powering off
  SYS_OPRT_MODE_FTM		= 1,		//!< ' Phone is in factory test mode
  SYS_OPRT_MODE_OFFLINE	= 2,		//!< ' Phone is offline
  SYS_OPRT_MODE_OFFLINE_AMPS = 3,	//!< ' Phone is offline analog
  SYS_OPRT_MODE_OFFLINE_CDMA = 4,	//!< ' Phone is offline cdma
  SYS_OPRT_MODE_ONLINE = 5,			//!< ' Phone is online
  SYS_OPRT_MODE_LPM = 6,			//!< ' Phone is in LPM - Low Power Mode
  SYS_OPRT_MODE_RESET = 7,			//!< ' Phone is resetting - i.e. power-cycling
  SYS_OPRT_MODE_NET_TEST_GW = 8,	//!< ' Phone is conducting network test for GSM/WCDMA.
  SYS_OPRT_MODE_OFFLINE_IF_NOT_FTM = 9,	//!< ' offline request during powerup.
  SYS_OPRT_MODE_PSEUDO_ONLINE = 10,	//!< ' Phone is pseudo online, tx disabled
  SYS_OPRT_MODE_MAX					//!< ' FOR INTERNAL USE OF CM ONLY!

} sys_oprt_mode_enum;

//! System modes, from services/cm/sys.h
typedef enum
{
  SYS_SYS_MODE_NONE		= -1,		//!< ' FOR INTERNAL USE ONLY!
  SYS_SYS_MODE_NO_SRV	= 0,		//!< ' No service = NV_MODE_INACTIVE
  SYS_SYS_MODE_AMPS		= 1,		//!< ' Analog Mobile Phone System (AMPS) mode
  SYS_SYS_MODE_CDMA		= 2,		//!< ' Code Division Multiple Access (CDMA) mode
  SYS_SYS_MODE_GSM		= 3,		//!< ' Global System for Mobile communications (GSM) mode
  SYS_SYS_MODE_HDR		= 4,		//!< ' High Data Rate (HDR) mode
  SYS_SYS_MODE_WCDMA	= 5,		//!< ' Wideband CDMA (WCDMA) mode
  SYS_SYS_MODE_GPS		= 6,		//!< ' Global Positioning System (GPS) mode
  SYS_SYS_MODE_GW		= 7,		//!< ' GSM and WCDMA mode
  SYS_SYS_MODE_MAX					//!< ' FOR INTERNAL USE ONLY!

} sys_sys_mode_enum;


//! For use by QLIB_DIAG_GSM_STATUS_F, defined in 80-V5295-1, section 3.2.4
typedef enum
{
	GSM_STATUS_CALL_STATE_NONE           = -1,	//!< ' not defined
	GSM_STATUS_CALL_STATE_IDLE           = 0,	//!< ' IDLE
	GSM_STATUS_CALL_STATE_ORIG           = 1,	//!< ' Originating
	GSM_STATUS_CALL_STATE_WAITING        = 2,	//!< ' Waiting
	GSM_STATUS_CALL_STATE_INCOM          = 3,	//!< ' Incoming
	GSM_STATUS_CALL_STATE_CC_IN_PROGRESS = 4,	//!< ' Concurrent incoming/originating
	GSM_STATUS_CALL_STATE_CONV           = 5	//!< ' Conversation
} gsm_call_state_enum;

/**
	Call Origination, see Call Manager ICD 3.4.50

*/

typedef enum
{
	_DIAG_CALL_ORIGINATION_SO_ID_IS96                           = 1,
	_DIAG_CALL_ORIGINATION_SO_ID_8K_LOOPBACK                    = 2,
	_DIAG_CALL_ORIGINATION_SO_ID_MARKOV                         = 3,
	_DIAG_CALL_ORIGINATION_SO_ID_RESERVED                       = 4,
	_DIAG_CALL_ORIGINATION_SO_ID_VOICE_IS96A                    = 5,
	_DIAG_CALL_ORIGINATION_SO_ID_VOICE_13K                      = 6,
	_DIAG_CALL_ORIGINATION_SO_ID_RATESET2_MARKOV                = 7,
	_DIAG_CALL_ORIGINATION_SO_ID_RATESET1_MARKOV                = 8,
	_DIAG_CALL_ORIGINATION_SO_ID_13K_LOOPBACK                   = 9,
	_DIAG_CALL_ORIGINATION_SO_ID_13K_MARKOV                     = 10,
	_DIAG_CALL_ORIGINATION_SO_ID_EVRC                           = 11,
	_DIAG_CALL_ORIGINATION_SO_ID_RATESET2_SMS                   = 14,
	_DIAG_CALL_ORIGINATION_SO_ID_13K_VOICE                      = 17,
	_DIAG_CALL_ORIGINATION_SO_ID_MDR_RS1FWD_RS1REV              = 22,
	_DIAG_CALL_ORIGINATION_SO_ID_MDR_RS1FWD_RS2REV              = 23,
	_DIAG_CALL_ORIGINATION_SO_ID_MDR_RS2FWD_RS1REV              = 24,
	_DIAG_CALL_ORIGINATION_SO_ID_MDR_RS2FWD_RS2REV              = 25,
	_DIAG_CALL_ORIGINATION_SO_ID_MDR_RS1FWD_RS1REV_DATAPATTERN  = 36,
	_DIAG_CALL_ORIGINATION_SO_ID_MDR_RS2FWD_RS2REV_DATAPATTERN  = 37,
	_DIAG_CALL_ORIGINATION_SO_ID_IS2000_MARKOV                  = 54,
	_DIAG_CALL_ORIGINATION_SO_ID_IS2000_LOOPBACK                = 55,
	_DIAG_CALL_ORIGINATION_SO_ID_4GV_NARROWBAND                 = 68,
	_DIAG_CALL_ORIGINATION_SO_ID_SMS                            = 65534
}Diag_call_origination_serice_option_enum_type;

/******************************************************************************
						Diagnostic - HDR subsytem
*******************************************************************************/
//! QCT HDR Subsystem command IDs (80-V1294-2) Table 2.1
typedef enum
{
	_HDR_RST_ACCESS_CNT					= 0,
	_HDR_RST_FWD_STATS					= 1,
	_HDR_RST_RTC_RATE_CNT				= 2,
	_HDR_RST_CONN_ATTEMPT_CNT			= 3,
	_HDR_RST_PAGE_MSG_CNT				= 4,
	_HDR_RST_SESSION_ATTEMPT_CNT		= 5,
	_HDR_RST_RLP_STATS					= 6,
	_HDR_AT_CONFIG						= 7,
	_HDR_STATE							= 8,
	_HDR_DIP_SWITCH						= 9,
	_HDR_MCS_FLOW_COMMAND				= 10,
	_HDR_MCS_DEBUG_COMMAND				= 11,
	_HDR_MCS_CLR_STATS_COMMAND			= 12,
	_HDR_RST_CONNECTED					= 13
} hdr_subsys_cmd_id_enum;

/******************************************************************************
						Diagnostic - GSM subsytem
*******************************************************************************/

// Constants for GSM Diag
#define GSM_DIAG_IMEI_SIZE  9
#define GSM_DIAG_IMSI_SIZE  9
#define GSM_DIAG_LAI_SIZE   5
#define GSM_DIAG_CALL_ORG_IMSI_SIZE 32

/**
	GSM Diagnostic commands, from Services\diag\gsmdiag.h

*/
typedef enum
{
	_GSM_DIAG_VERNO_F           = 0,
	_GSM_STATUS_F               = 1,
	_GSM_TMC_STATE_RETRIEVAL_F  = 2,
	_GSM_DIAG_CALL_ORIG_F       = 3,
	_GSM_DIAG_CALL_END_F        = 4,
	_GSM_GPRS_LOG_PACKET_REQ_F  = 5
} DiagGsmCmd_Enum;

/******************************************************************************
						Diagnostic - MediaFLO subsytem
*******************************************************************************/

/**
	MediaFLO Diagnostic commands, from mflog.h

*/
typedef enum
{
	_MFLO_RST_PLP_STATS					= 0,	//!< ' Reset Total/Bad PLP counters                */
	_MFLO_GET_FLO_STATE					= 1,    //!< ' Get FLO State                               */
    _MFLO_START_FLO						= 2,    //!< ' Start FLO                                   */
    _MFLO_GET_FLO_VERSION_INFO			= 6,	//!< ' Get the FLO H/W & S/W Versions              */
    _MFLO_ACTIVATE_FLOW					= 9,    //!< ' Start decoding FLOW                         */
    _MFLO_DEACTIVATE_FLOW				= 10,   //!< ' Stop decoding FLOW                          */
    _MFLO_GET_BOUND_FLOW_LIST			= 11,   //!< ' Get Bound FLOW List                         */
    _MFLO_GET_MLC_INFO					= 12,   //!< ' Get MLC Info                                */
    _MFLO_GET_RSSI_VALUE				= 13,   //!< ' Get RSSI Value                              */
    _MFLO_GET_MLC_PLP_STAT_DYN_PARAMS	= 14,   //!< ' Get MLC PLPs/CBs stats                      */
    _MFLO_SET_RF_CHNL					= 15,   //!< ' Set RF Channel                              */
    _MFLO_GET_RF_CHNL					= 16,   //!< ' Get RF Channel                              */
    _MFLO_RST_MLC_PLP_STATS				= 17,   //!< ' Reset MLC Total/Bad PLP counters            */
    _MFLO_GET_MLC_DYN_PARAMS			= 18,   //!< ' Get MLC Dynamic Parameters                  */
    _MFLO_GET_OIS_PLP_STAT				= 21,   //!< ' Get PLP stats for OIS channel               */
    _MFLO_RST_OIS_PLP_STAT				= 22,   //!< ' Reset PLP stats for OIS channel             */
    _MFLO_GET_FLO_SUBSTATE				= 39,   //!< ' Get FLO SUB State                           */
    _MFLO_GET_ACTIVE_MLC_LIST			= 40,   //!< ' Get MLCs Info for the all active ones       */
    _MFLO_GET_FLO_PLP_DYN_PARAMS		= 43,   //!< ' Get PLP Params based on FLOW Id             */
    _MFLO_ENABLE_FTAP_PLP_DATA			= 44,   //!< ' Enable PacketRecord logging mode            */
    _MFLO_ENABLE_FTAP_OIS				= 45,   //!< ' Enable OIS logging mode                     */
	_MFLO_ENABLE_WIC_LIC				= 46,   //!< ' Enable FTAP WIC/LIC Logging                 */
    _MFLO_FTAP_ACTIVATE_FLOW			= 48,   //!< ' Activate FTAP FLOW						   */
    _MFLO_GET_CONTROL_CHANNEL_DATA		= 54    //!< ' Enable Control Channel Data logging		   */
} mflo_diag_subsys_cmd_enum_type;

/**
	MediaFLO Log Codes, from mflog.h

*/
#define LOG_MFLO_RSSI_VALUE_DYNAMIC_PARAMS_C		0x11F5
#define LOG_MFLO_FTAP_PACKETRECORD_HEADER_PARAMS_C	0x120C
#define LOG_MFLO_FTAP_PACKETRECORD_PARAMS_C			0x120D
#define LOG_MFLO_FTAP_WOIS_PARAMS_C                 0x120E
#define LOG_MFLO_FTAP_LOIS_PARAMS_C					0x120F
#define LOG_MFLO_FTAP_WID_LID_PARAMS_C				0x1210
#define LOG_MFLO_FDM_RECORDS_C						0x121F

#define MFLO_MAX_LOG_PACKET_SIZE					1542
#define MFLO_MAX_FTAP_PACKET_RECORDS				111
#define MFLO_FIRST_LAYERED_TX_MODE					6
#define MFLO_MAX_MLC_FLOW_MAPPING_RECORDS			80

//! MediaFLO Binding type
typedef enum
{
   MFLO_OVERHEAD   = 0x00,
   RT_VIDEO_CODEC  = 0x01,
   RT_AUDIO_CODEC  = 0x02,
   NRT_MEDIA       = 0x03,
   DATACAST        = 0x04,
   TAP             = 0x05,
   FIXED_TAP       = 0x06,
   NOT_BOUND	   = 0xFF
} mftypes_binding_type;

//! MediaFLO System type
typedef enum
{
   WOIS				= 0x00,		//! Wide System
   WIDE				= 0x00,		//! Wide System
   LOIS				= 0x01,		//! Local System
   LOCAL			= 0x01,		//! Local System
   WIDE_AND_LOCAL	= 0x02
} mftypes_system_type;

//! MediaFLO State type
typedef enum
{
   INVALID_STATE		= 0x00,
   ACQUIRING_STATE		= 0x01,
   IDLE_STATE			= 0x02,
   ACTIVE_STATE			= 0x03,
   UNAVAILABLE_STATE	= 0x04,
   VARIABLE_NOT_SET		= 0xFF	//! Used when a function to get the state fails
} mftypes_state_type;

//! MediaFLO Transmit mode type
typedef enum
{
   REG_QPSK_1_3         = 0,
   MREG_QPSK_1_2        = 1,
   REG_16_QAM_1_3       = 2,
   REG_16_QAM_1_2       = 3,
   REG_16_QAM_2_3       = 4,
   REG_16_QPSK_1_5      = 5,
   LAY_QPSK_1_3_ER_4    = 6,
   LAY_QPSK_1_2_ER_4    = 7,
   LAY_QPSK_2_3_ER_4    = 8,
   LAY_QPSK_1_3_ER_6_25 = 9,
   LAY_QPSK_1_2_ER_6_25 = 10,
   LAY_QPSK_2_3_ER_6_25 = 11
} mftypes_trans_mode_type;

//! MediaFLO Activate Status type
typedef enum
{
   ACTIVATE_FAILED		= 0x00,
   ACTIVATING			= 0x01,
   ACTIVATE_SUCCESS		= 0x02

} mftypes_activate_status_type;

//! MediaFLO Flow Status type
typedef enum
{
   FLOW_NOT_ACTIVATED  	= 0x00,
   FLOW_ACTIVATED		= 0x01,
   FLOW_DEACTIVATING	= 0x02

} mftypes_flow_status_type;

//! MediaFLO MLC Status type
typedef enum
{
   MLC_NOT_ACTIVE  	= 0x00,
   MLC_ACTIVE		= 0x01

} mftypes_mlc_status_type;

//! MediaFLO Record Retrieval Status type
typedef enum
{
   NO_RECORDS_AVAILABLE	= 0x00,
   RECORDS_RETRIEVED	= 0x01

} mftypes_record_retrieval_status_type;

//! MediaFLO Sleep Setting type
typedef enum
{
   SLEEP_AND_SNOOZE_DISABLED		= 0x00,
   SLEEP_ENABLED_SNOOZE_DISABLED	= 0x01,
   SLEEP_DISABLED_SNOOZE_ENABLED	= 0x02,
   SLEEP_ENABLED_SNOOZE_ENABLED		= 0x03

} mftypes_sleep_settings_type;

//! MediaFLO Structure to store MLC PLP statistics
typedef struct
{
	dword BasePlpsPreRsGood;
	dword BasePlpsPreRsErasures;
	dword BasePlpsPostRsGood;
	dword BasePlpsPostRsErasures;
	dword EnhPlpsPreRsGood;
	dword EnhPlpsPreRsErasures;
	dword EnhPlpsPostRsGood;
	dword EnhPlpsPostRsErasures;
	dword BaseCbsPreRsGood;
	dword BaseCbsPreRsErasures;
	dword BaseCbsPostRsGood;
	dword BaseCbsPostRsErasures;
	dword EnhCbsPreRsGood;
	dword EnhCbsPreRsErasures;
	dword EnhCbsPostRsGood;
	dword EnhCbsPostRsErasures;
}mftypes_mlc_plp_stats_type;

//! MediaFLO Structure to store NS PLP statistics
typedef struct
{
	dword PlpsPreRsGood;
	dword PlpsPreRsErasures;
	dword PlpsPostRsGood;
	dword PlpsPostRsErasures;
}mftypes_plp_stats_type;

//! MediaFLO Structure to store NS CB statistics
typedef struct
{
	dword CbsPostRsGood;
	dword CbsPostRsErasures;
}mftypes_cb_stats_type;

//! MediaFLO Protocol task's substate type
typedef enum
{
	SUBSTATE_IDLE		= 0x0,
	SUBSTATE_GET_OIS	= 0x1,
	SUBSTATES_GET_CC	= 0x2
}mftypes_substate;

//! MediaFLO OIS reception mode type
typedef enum
{
	DISABLE_WOIS_AND_LOIS	= 0x0,
	ENABLE_WOIS_ONLY		= 0x1,
	ENABLE_LOIS_ONLY		= 0x2,
	ENABLE_WOIS_AND_LOIS	= 0x3
}mftypes_ois_reception_mode_type;

//! MediaFLO Reed-Solomon code type
typedef enum
{
	RS_16_16_0		= 0x0,
	RS_16_14_2		= 0x1,
	RS_16_12_4		= 0x2,
	RS_16_8_8		= 0x3
}mftypes_reed_solomon_code_type;

//! MediaFLO Structure to store MLC dynamic param info
typedef struct
{
	byte MlcID;
	dword BaseFFTaddress;
	word SymbolOffset;
	byte StartSlotOffset;
	byte MinSlot;
	byte MaxSlot;
	word NumSlots;
	byte NumCodeBlocks;
	mftypes_trans_mode_type TxMode;
	byte OuterCode;
	byte Stream1Bound;
	byte Stream2Bound;
	byte FFTframeCount;
}mftypes_mlc_dyn_params_type;

//! MediaFLO Structure to store info about active flows
typedef struct
{
	dword FlowID;
	byte MlcID;
	byte StreamID;
	byte StreamUsesBothLayers;
	byte ByteInterleaveEnabled;
	mftypes_binding_type BindingType;
	mftypes_system_type System;
}mftypes_active_flow_info;

//! MediaFLO Structure to store info about active MLCs
typedef struct
{
	dword Base_FFT_Address;
	mftypes_trans_mode_type TransmitMode;
	byte OuterCode;
	byte NumCbsInCurrentSF;
	mftypes_mlc_plp_stats_type MlcPlpStats;
	byte Stream1Bound;
	byte Stream2Bound;
}mftypes_active_mlc_info;

//! Structure to associate Flow ID, MLC ID, Tx Mode,
//! Base FTAP PLP per Superframe, Enhancement FTAP PLP per Superframe
typedef struct
{
	dword FlowID;
	byte MLC_ID;
	byte TxMode;
	word PLPperSuperframeBase;
	word PLPperSuperframeEnh;
}mftypes_mlc_schedule;

//! MediaFLO Structure to associate FTAP Sequence Numbers with their corresponding FTAP Test Signature
typedef struct
{
	dword FTAP_SequenceNumber;
	byte FTAP_TestSignature;
}mftypes_seq_vs_sig;

//! MediaFLO Structure to store the TAP Message Content for the FLO Tx Waveform
typedef struct
{
	byte NumberOfSuperframes;				// Number of superframes in the FLO Tx Waveform
	byte NumberOfFlowIDs;					// Number of FlowIDs available to be decoded in the FLO Tx Waveform
	mftypes_mlc_schedule* pMLC_Schedule;	// Structure which associates Flow ID, MLC ID, Tx Mode and PLP per superframe for the FLO Tx Waveform
	mftypes_seq_vs_sig* pSeq_vs_Sig;		// Structure which associates FTAP Sequence Numbers in the FLO Tx Waveform with their corresponding FTAP Test Signature
}mftypes_tap_msg_content;

//! MediaFLO Structure to store data for one Flow ID for one superframe while parsing log packets
typedef struct
{
	dword FlowID;							// Flow ID corresponding to the statistics in this data structure
	word PLPperSuperframeBase;				// Number of Base PLPs in each superframe
	word PLPperSuperframeEnh;				// Number of Enhancement PLPs in each superframe
	mftypes_seq_vs_sig* pLoggedSeqVsSigBase;// Pointer to array of mftypes_seq_vs_sig structures to store Base FTAP Seq Num / FTAP Test Sig pairs
	mftypes_seq_vs_sig* pLoggedSeqVsSigEnh;	// Pointer to array of mftypes_seq_vs_sig structures to store Enhancement FTAP Seq Num / FTAP Test Sig pairs
	word BaseLength;						// Number of Base FTAP Seq Num / FTAP Test Sig pairs recorded
	word EnhLength;							// Number of Enhancement FTAP Seq Num /FTAP Test Sig pairs recorded
	byte* pMatches;							// Pointer to byte array with length PLPperSuperframeBase * Number of Superframes
	word NumBasePLPs;						// Number of Base PLPs recorded
	word NumEnhPLPs;						// Number of Enhancement PLPs recorded
	word NumBaseErasures;					// Number of Base erasures recorded
	word NumEnhErasures;					// Number of Enhancement erasures recorded
	word NumBaseSeqSigErrors;				// Number of Base FTAP Seq Num / FTAP Test Sig errors recorded
	word NumEnhSeqSigErrors;				// Number of Enhancement FTAP Seq Num / FTAP Test Sig errors recorded
}mftypes_current_sf_stats;

//! MediaFLO Error code type
typedef enum
{
	NOT_AVAILABLE								= 0,
	TAP_MSG_CONTENT_NOT_INITIALIZED				= 1,
	NO_LOG_PACKETS								= 2,
	MEMORY_ALLOCATION_ERROR						= 3,
	INVALID_NUMBER_OF_FLOW_IDS					= 4,
	RCVD_LOGPACKET_FOR_INVALID_FLOWID			= 5,
	PLP_STATS_ALL_ZERO							= 6,
	INVALID_REED_SOLOMON_TYPE					= 7,
	INVALID_NUMBER_OF_PLP_RECORDS				= 8,
	MISSING_LOG_PACKETS							= 9,
	PLP_STATS_NOT_UPDATING						= 10
}mftypes_error_code;

//! MediaFLO Type of PER to measure
typedef enum
{
	BASE_ONLY					= 0,
	ENHANCEMENT_ONLY			= 1,
	BASE_AND_ENHANCEMENT		= 2
}mftypes_per_to_measure;

//! MediaFLO Reed Solomon type
typedef enum
{
	PRE_REED_SOLOMON			= 0,
	POST_REED_SOLOMON			= 1,
	PRE_OR_POST_REED_SOLOMON	= 2
}mftypes_reed_solomon;

//! MediaFLO PER measurement result
typedef enum
{
	FAILED				= 0,
	PASSED				= 1,
	KEEP_TESTING		= 2
}mftypes_per_result;

//! MediaFLO control channel message type
typedef enum
{
   MFCC_FDM				= 0x00,
   MFCC_RDM				= 0x01,
   MFCC_NDM				= 0x02,
   MFCC_FILLER			= 0x03,
   MFCC_FMS				= 0xEF
} mftypes_cc_msg_type;

//! MediaFLO Structure for holding the overall PER statistics for the current measurement
typedef struct
{
	dword iFlowID;					//! Flow ID corresponding to the statistics in this data structure
	double dPER_Base;				//! Base Layer Packet Error Rate for all superframes
	double dPER_Enh;				//! Enhancement Layer Packet Error Rate for all superframes
	dword iTotalPLPBase; 			//! Total number of Base Layer PLPs recorded
	dword iTotalPLPEnh; 			//! Total number of Enhancement Layer PLPs recorded
	dword iTotalErrorsBase; 		//! Total number of Base Layer Errors recorded (includes erasures and mismatched FTAP_SequenceNumber / FTAP_TestSignature pairs)
	dword iTotalErrorsEnh; 			//! Total number of Enhancement Layer Errors recorded (includes erasures and mismatched FTAP_SequenceNumber / FTAP_TestSignature pairs)
	dword iTotalErasuresBase;		//! Total number of Base Layer Erasures recorded
	dword iTotalErasuresEnh;		//! Total number of Enhancement Layer Erasures recorded
	dword iTotalSeqSigErrorsBase;	//! Total number of Base Layer FTAP Sequence Number / FTAP Test Signature Errors recorded
	dword iTotalSeqSigErrorsEnh;	//! Total number of Enhancement Layer FTAP Sequence Number / FTAP Test Signature Errors recorded
}mftypes_per_statistics;

//! MediaFLO Structure for holding the overall WID LID statistics for the current measurement
typedef struct
{
	dword iTotalWIDLIDMeasured; 	//! Total number WID and LIDs measured
	dword iTotalErrorsWID; 			//! Total number of WID Errors.
	dword iTotalErrorsLID; 			//! Total number of LID Errors.
	double dWIDErrorRate;           //! WID Error Rate
	double dLIDErrorRate;           //! LID Error Rate
}mftypes_widlid_statistics;

//! MediaFLO Structure used to parse the LOG_MFLO_FTAP_PACKETRECORD_HEADER_PARAMS_C log packets
typedef struct
{
	dword sysTime;
	dword flow_id;
	byte mlc_id;
	byte strm_id;
	byte system;
	byte trans_mode;
	byte rs_code;
	word rssi_int_part;
	byte rssi_fract_part;
	byte rssi_value_positive;
	dword num_PLP_records;
}mftypes_ftap_packet_record_header_params;

//! MediaFLO Structure used to parse individual PLP records. A LOG_MFLO_FTAP_PACKETRECORD_PARAMS_C
//! log packet contains a variable number of individual PLP records.
typedef struct
{
	dword flow_id;
	byte data_record_or_parity;
	byte base_or_enhancement;
	byte preRS_ErasureIndicator;
	byte postRS_ErasureIndicator;
	dword FTAP_SequenceNumber;
	byte FTAP_TestSignature;
}mftypes_ftap_plp_record;

//! MediaFLO Structure used to parse the first four entries in LOG_MFLO_FTAP_PACKETRECORD_PARAMS_C log packets
typedef struct
{
	dword sysTime;
	word log_seq_num;
	byte last_packet;
	word actual_num_plp_records;
}mftypes_ftap_logpkt_header_type;

//! MediaFLO Structure used to parse the LOG_MFLO_FTAP_PACKETRECORD_PARAMS_C log packets
typedef struct
{
	mftypes_ftap_logpkt_header_type logpkt_header;
	mftypes_ftap_plp_record plp_records[MFLO_MAX_FTAP_PACKET_RECORDS];
}mftypes_ftap_packet_record_params;

//! MediaFLO Structure used to parse the first five entries in LOG_MFLO_FDM_RECORDS_C log packets
typedef struct
{
	dword superframe_number;
	word current_seq_num;
	word total_seq_num;
	word num_records_in_this_log;
	word reserved;
}mftypes_fdm_record_header_type;

//! MediaFLO Structure used to parse individual records that map MLC ID to Flow ID in LOG_MFLO_FDM_RECORDS_C
//! log packets. A LOG_MFLO_FDM_RECORDS_C log packet contains a variable number of mapping records.
typedef struct
{
	dword service_id;
	byte flow_desc;
	byte mlc_id;
	byte system;
	byte stream_id;
	byte trans_mode;
	byte outer_code;
	word reserved1;
	dword reserved2;
}mftypes_mfcc_mlc_flow_mapping_info_type;

//! MediaFLO Structure used to parse LOG_MFLO_FDM_RECORDS_C log packets
typedef struct
{
	mftypes_fdm_record_header_type logpkt_header;
	mftypes_mfcc_mlc_flow_mapping_info_type mflog_fdm_mapping[MFLO_MAX_MLC_FLOW_MAPPING_RECORDS];
}mftypes_fdm_records_type;

//! MediaFLO Structure used to store a Control Channel Record
typedef struct
{
	word service_id;
	byte flow_desc;
	byte mlc_id;
	byte system;
	byte stream_id;
	byte trans_mode;
	byte outer_code;
}mftypes_mf_control_channel_record;

//! MediaFLO L-Band Channel Numbers
typedef enum
{
	MFLO_L_BAND_LA_CHANNEL = 0x01,
	MFLO_L_BAND_LB_CHANNEL = 0x02,
	MFLO_L_BAND_LC_CHANNEL = 0x03,
	MFLO_L_BAND_LD_CHANNEL = 0x04,
	MFLO_L_BAND_LE_CHANNEL = 0x05,
	MFLO_L_BAND_LF_CHANNEL = 0x06,
	MFLO_L_BAND_LG_CHANNEL = 0x07,
	MFLO_L_BAND_LH_CHANNEL = 0x08,
	MFLO_L_BAND_LI_CHANNEL = 0x09,
	MFLO_L_BAND_LJ_CHANNEL = 0x0A,
	MFLO_L_BAND_LK_CHANNEL = 0x0B,
	MFLO_L_BAND_LL_CHANNEL = 0x0C,
	MFLO_L_BAND_LM_CHANNEL = 0x0D,
	MFLO_L_BAND_LN_CHANNEL = 0x0E,
	MFLO_L_BAND_LO_CHANNEL = 0x0F,
	MFLO_L_BAND_LP_CHANNEL = 0x10,
	MFLO_L_BAND_LQ_CHANNEL = 0x11,
	MFLO_L_BAND_LR_CHANNEL = 0x12,
	MFLO_L_BAND_LS_CHANNEL = 0x13,
	MFLO_L_BAND_LT_CHANNEL = 0x14,
	MFLO_L_BAND_LU_CHANNEL = 0x15,
	MFLO_L_BAND_LV_CHANNEL = 0x16,
	MFLO_L_BAND_LW_CHANNEL = 0x17
}mftypes_l_band_channel_type;

//! MediaFLO Jammer Detection Modes
typedef enum
{
	MFLO_JD_MODE_AUTO = 0,
	MFLO_JD_MODE_1    = 1,
	MFLO_JD_MODE_2    = 2
}mftypes_jd_mode_type;

//! MediaFLO Jammer Detection States
typedef enum
{
	MFLO_JD_STATE_INVALID      = 0,
	MFLO_JD_STATE_MODE_1       = 1,
	MFLO_JD_STATE_MODE_2       = 2,
	MFLO_JD_STATE_INITIALIZING = 3
}mftypes_jd_state_type;

//! MediaFLO Jammer Detection Substates
typedef enum
{
	MFLO_JD_SUBSTATE_NONE    = 0,
	MFLO_JD_SUBSTATE_POLLING = 1
}mftypes_jd_substate_type;

/******************************************************************************
						Diagnostic - CGPS subsystem
*******************************************************************************/

//! CGPS Subsystem command
#define CGPS_SUBSYSTEM 101	//!< CGPS Diagnostic subystem ID

//! CGPS Packet version
#define CGPS_PACKET_VERSION 1
#define CGPS_GEN8_PACKET_VERSION 50

/**
	Converged GPS commands
*/
typedef enum
{
	_GPS_CMD_CODE_GPS_SA_RF_VERIF_MODE_SWITCH	= 20,	//!< ' CGPS engine enters or exits "Standalone (SA) RF Verification" mode
	_GPS_CMD_CODE_GPS_PRESCRIBED_DWELL			= 21,	//!< ' CGPS engine performs one dwell on one channel
	_GPS_CMD_CODE_GPS_START_IQ_TEST				= 22,	//!< ' CGPS engine starts an IQ collect
	_GPS_CMD_CODE_GPS_SV_TRACK					= 23,	//!< ' CGPS engine tracks the specified SV or all SVs
	_GPS_CMD_CODE_GPS_GEN8_HW_CONFIG			= 26,	//!< ' Configure CGPS Gen8 engine hardware
	_GPS_CMD_CODE_GPS_MULTI_CHANNEL_TRACK		= 27													//
} gps_cmd_code_enum_type;

/**
	Events used by CGPS functions
*/
typedef enum
{
	_EVENT_GPS_PD_COMM_FAILURE		= 1241,
	_EVENT_GPS_PD_COMM_DONE			= 1242,
	_EVENT_GPS_PD_EVENT_END			= 1243,
	_EVENT_GPS_PA_EVENT_CALLBACK	= 1244,
	_EVENT_GPS_PD_CMD_ERR_CALLBACK	= 1245,
	_EVENT_GPS_PA_CMD_ERR_CALLBACK	= 1246,
	_EVENT_GPS_LM_ENTER_SA_RF_VERIF	= 1247,
	_EVENT_GPS_LM_EXIT_SA_RF_VERIF	= 1248,
	_EVENT_GPS_LM_ERROR_SA_RF_VERIF	= 1249,
	_EVENT_GPS_LM_PD_COMPLETE		= 1250,
	_EVENT_GPS_LM_IQ_TEST_COMPLETE	= 1251,
	_EVENT_GPS_SBAS_DEMOD_REPORT 	= 1457
} gps_events_enum_type;

/**
	Logs used by CGPS functions
*/
typedef enum
{
	//! Log code for CGPS Measurement Report
	CGPS_MEASUREMENT_REPORT_LOG			= 0x1371,

	//! Log code for CGPS RF Status
	CGPS_RF_STATUS_REPORT_LOG			= 0x1372,

	//! Log code for IQ and FFT data
	CGPS_IQ_DATA_LOG					= 0x138A,

	//! Log code for prescribed dwell status
	CGPS_PRESCRIBED_DWELL_STATUS_LOG		= 0x1374,

	//! Log code for GPS soft decisions, used during the GPS BER test
	GPS_DEMOD_SOFT_DECISIONS_LOG		= 0x1253,

	//! Log code for GPS SBAS Demodulated bits
	CGPS_SBAS_DEMODULATED_BITS   		= 0x1457,

	//! Log code for GPS SBAS Demod Soft Symbols
	CGPS_SBAS_DEMOD_SOFT_SYMBOLS 		= 0x1458,

	//! Log code for ME Job Info Log
	CGPS_ME_JOB_INFO_LOG 				= 0x13BA,

    GNSS_GPS_MEASUREMENT_REPORT_LOG		= 0x1477,
	GNSS_DEMOD_SOFT_DECISIONS_LOG	 	= 0x1479,
	GNSS_GLONASS_MEASUREMENT_REPORT_LOG	= 0x1480,
	GNSS_GPS_HBW_RXD_MEASUREMENT_LOG	= 0x1481,
	GNSS_PRX_RF_HW_STATUS_REPORT_LOG	= 0x147E,
	GNSS_DRX_RF_HW_STATUS_REPORT_LOG	= 0x147F,

	//! Log code for CGPS WB FFT C/N
	CGPS_WB_FFT_STATS_C_LOG				= 0x1487				
} gps_logs_enum_type;

/**
	Used for the OBS_STATE field of the log message, CGPS_MEASUREMENT_REPORT_LOG = 0x1371,
*/
typedef enum
{
	CGPS_MEASUREMENT_OBS_STATE_IDLE = 0,
	CGPS_MEASUREMENT_OBS_STATE_SEARCH = 1,
	CGPS_MEASUREMENT_OBS_STATE_SEARCH_VERIFY = 2,
	CGPS_MEASUREMENT_OBS_STATE_BIT_EDGE = 3,
	CGPS_MEASUREMENT_OBS_STATE_TRACK_VERIFY = 4,
	CGPS_MEASUREMENT_OBS_STATE_TRACKING = 5,
	CGPS_MEASUREMENT_OBS_STATE_RESTART = 6,
	CGPS_MEASUREMENT_OBS_STATE_DPO_TRACK = 7
} gps_measurement_report_state_enum_type;


/** For C/N measurement, Measurement state should have the following bits on
	bit 0 = submillisecond is valid
	bit 8 = good parity
*/
#define CGPS_MEASUREMENT_MEAS_STATE_CTON_FILTER 0x11

/** CGPS Measurement Status bit definitions   */
#define CGPS_MEAS_STATUS_NULL               0x0000

#define CGPS_MEAS_STATUS_SM_VALID           0x0001
#define CGPS_MEAS_STATUS_SB_VALID           0x0002
#define CGPS_MEAS_STATUS_MS_VALID           0x0004
#define CGPS_MEAS_STATUS_BE_CONFIRM         0x0008

#define CGPS_MEAS_STATUS_VE_VALID           0x0010
#define CGPS_MEAS_STATUS_VE_FINE            0x0020
#define CGPS_MEAS_STATUS_LP_VALID           0x0040
#define CGPS_MEAS_STATUS_LP_POS             0x0080

#define CGPS_MEAS_STATUS_GOOD_DATA          0x0100
#define CGPS_MEAS_STATUS_FROM_RNG_DIFF      0x0200
#define CGPS_MEAS_STATUS_FROM_VE_DIFF       0x0400
#define CGPS_MEAS_STATUS_XCORR              0x0800

#define CGPS_MEAS_STATUS_TENTATIVE          0x1000
#define CGPS_MEAS_STATUS_DONT_USE           0x2000
#define CGPS_MEAS_STATUS_NEED_SIR_CHECK     0x4000
#define CGPS_MEAS_STATUS_ACQ_OR_PROBATION   0x8000


#define CGPS_MEAS_STATUS_ROUND_ROBIN_RX_DIVERSITY	    0x40000
#define CGPS_MEAS_STATUS_RX_DIVERSITY_MEASUREMENT       0x80000

#define CGPS_MEAS_STATUS_LBW_RXD_COMBINED_MEASUREMENT   0x10000
#define CGPS_MEAS_STATUS_HBW_GPS_NU4_MEASUREMENT		0x20000
#define CGPS_MEAS_STATUS_HBW_GPS_NU8_MEASUREMENT	    0x40000
#define CGPS_MEAS_STATUS_HBW_GPS_UNIFORM_MEASUREMENT    0x80000

#define CGPS_MEAS_STATUS_GNSS_MULTIPATH_INDICATOR	    0x100000

/**
	Format for CGPS Measurement Report SV row
	CGPS_MEASUREMENT_REPORT_LOG			= 0x1371,
*/
typedef struct
{
	byte	SV_ID;
	byte	OBS_STATE;			// value 5 = CGPS_MEASUREMENT_OBS_STATE_TRACKING = tracking, this is check for in C/N measurment
	byte	OBS_CNT;
	byte	OBS_GOOD_CNT;
	byte	FILTER_N;
	word	CNO;
	word	LATENCY;
	byte	PRE_INT;
	word	POST_INT;
	dword	UNFILT_MS;
	float	UNFILT_SM;
	float	UNFILT_TIME_UNC;
	float	UNFILT_SPEED;
	float	UNFILT_SPEED_UNC;
	dword	MEAS_STATUS;
	byte	MISC_STATUS;
	dword	MULTIPATH_EST;
	float	AZI_RAD;
	float	ELEV_RAD;
	long	CARRIER_PHASE_CYCLES;
	int	CARRIER_PHASE_FRACT;
	dword	RESERVED1;

} CGPS_MeasurementReportLog_SvRow_Struct;

typedef struct
{
	byte	SV_ID;
	byte	OBS_STATE;			// value 5 = CGPS_MEASUREMENT_OBS_STATE_TRACKING = tracking, this is check for in C/N measurment
	byte	OBS_CNT;
	byte	OBS_GOOD_CNT;
	word 	PARITY_ERROR_CNT;
	byte	FILTER_N;
	word	CNO;
	word	LATENCY;
	byte	PRE_INT;
	word	POST_INT;
	dword	UNFILT_MS;
	float	UNFILT_SM;
	float	UNFILT_TIME_UNC;
	float	UNFILT_SPEED;
	float	UNFILT_SPEED_UNC;
	dword	MEAS_STATUS;
	byte	MISC_STATUS;
	dword	MULTIPATH_EST;
	float	AZI_RAD;
	float	ELEV_RAD;
	long	CARRIER_PHASE_CYCLES;
	int		CARRIER_PHASE_FRACT;
	byte	CycleSlipCount;
	float	FINE_SPEED;
	float	FINE_SPEED_UNC;
	dword	RESERVED1;
} CGPS_GEN8_MeasurementReportLog_SvRow_Struct;

//! Number of SV's in a measurement report
#define CGPS_MAX_NUM_SV_IN_MEASUREMENT_REPORT 16

/**
	Format for CGPS Measurement Report
	CGPS_MEASUREMENT_REPORT_LOG			= 0x1371,
*/
typedef struct
{
	byte iVersion;
	dword iFCOUNT;
	word iGPS_Week;
	dword iGPS_MS;
	dword iTimeBias;
	dword iTimUncertainty;
	dword iFreqBias;
	dword iFreqUncertainty;
	byte iNumSVs;	//!< ' Number of SV's in report, 0...16
	CGPS_MeasurementReportLog_SvRow_Struct aSvRows[ CGPS_MAX_NUM_SV_IN_MEASUREMENT_REPORT ];
} CGPS_MeasurementReportLog_Struct;


typedef struct
{
	byte iVersion;
	dword iFCOUNT;
	word iGPS_Week;
	dword iGPS_MS;
	dword iTimeBias;
	dword iTimUncertainty;
	dword iFreqBias;
	dword iFreqUncertainty;
	byte  iNumSVs;	//!< ' Number of SV's in report, 0...16
	byte  type;
	CGPS_GEN8_MeasurementReportLog_SvRow_Struct aSvRows[ CGPS_MAX_NUM_SV_IN_MEASUREMENT_REPORT ];
} CGPS_GEN8_GpsMeasurementReportLog_Struct;


typedef struct
{
	byte	SV_ID;
	char	SV_FREQ_INDEX;
	byte	OBS_STATE;
	byte	OBS_CNT;
	byte	GOOD_OBS_CNT;
	byte	HEMMING_ERR_CNT;
	byte	FILTER_N;
	word	CNO;
	short	LATENCY;
	byte	PRE_INT;
	word	POST_INT;
	dword	UNFILT_MS;
	float	UNFILT_SM;
	float	UNFILT_TIME_UNC;
	float	UNFILT_SPEED;
	float	UNFILT_SPEED_UNC;
	dword	MEAS_STATUS;
	byte	MISC_STATUS;
	dword	MULTIPATH_EST;
	float	AZI_RAD;
	float	ELEV_RAD;
	long	CARRIER_PHASE_CYCLES;
	int		CARRIER_PHASE_FRACT;
	float	FINE_SPEED;
	float	FINE_SPEED_UNC;
	byte	CycleSlipCount;
	dword	RESERVED1;
} CGPS_GEN8_GlonassMeasurementReportLog_SvRow_Struct;

typedef struct
{
	byte	iVersion;
	dword	iFCOUNT;
	byte	GLO_FOUR_YEAR;
	word	GLO_CAL_DAYS;
	dword	GLO_MSEC;
	float	TIME_BIAS;
	float	TIME_UNC;
	float	FREQ_BIAS;
	float	FREQ_UNC;
	byte	NUMBER_SVS;
	CGPS_GEN8_GlonassMeasurementReportLog_SvRow_Struct aSvRows[ CGPS_MAX_NUM_SV_IN_MEASUREMENT_REPORT ];
} CGPS_GEN8_GlonassMeasurementReportLog_Struct;


/**
	Format for CGPS Measurement Report
	CGPS_MEASUREMENT_REPORT_LOG			= 0x1371,
*/

typedef struct
{
	byte	SV_ID;				// Satellite ID
	byte	OBS_STATE;			// SV observation state
	byte	OBS_CNT;			// Count of all observations, both success and failure
	byte	OBS_GOOD_CNT;		// Count of good observations
	word 	PARITY_ERROR_MASK;	// Parity Error Subframe mask
	byte	FILTER_N;			// Carrier-to-code filtering. # of filter stages
	word	CNO;				// Carrier to Noise in a 1Hz bandwidth, 0.01 dB-Hz per bit
	word	LATENCY;			// Current Fcount minus Fcount at which measurement was effective
	byte	PRE_INT;			// Pre-det interval in ms
	word	POST_INT;			// Number of post-dets
	dword	UNFILT_MS;			// Unfiltered measurement millisecond in user time, Unsigned value in ms.
	float	UNFILT_SM;			// Unfiltered measurement fractional ms in user time.
	float	OFFSET_UNFILT_SM;	// Unfiltered measurement fraction ms offset from LBW GPS.
	float	UNFILT_TIME_UNC;	// Unfiltered SV time uncertainty in nanoseconds
	float	UNFILT_SPEED;		// Unfiltered SV speed in m/s
	float	OFFSET_UNFILT_SPEED;// Unfiltered Sv Speed offset from LBW GPS.
	float	UNFILT_SPEED_UNC;	// Unfiltered SV speed uncertainty in m/s
	dword	MEAS_STATUS;		// Measurement status flags.
	byte	MISC_STATUS;
	dword	MULTIPATH_EST;
	float	AZI_RAD;
	float	ELEV_RAD;
	dword	CARRIER_PHASE_CYCLES;
	word	CARRIER_PHASE_FRACT;
	float	FINE_SPEED;
	float	FINE_SPEED_UNC;
	byte	CycleSlipCount;
	dword	RESERVED1;
} CGPS_GEN8_HBW_MeasurementReportLog_SvRow_Struct;

typedef struct
{
	byte	iVersion;			// Version number of packet = 0
	dword	iFCOUNT;			// Receiver's millisecond count value
	word	iGPS_Week;			// GPS Week number
	dword	iGPS_MS;			// GPS Millisecond into the week
	float	iTimeBias;			// (User time - GPS time) in milliseconds
	float	iTimeUncertainty; 	// Clock time uncertainty in nanoseconds.
	float	iFreqBias;			// Clock frequency bias in m/s
	float	iFreqUncertainty;	// Clock frequency uncertainty in m/s
	byte	iMeasurementType;	// Gps Measurement Type
	byte	iNumSVs;			// Number of SV's in report, 0...16
	CGPS_GEN8_HBW_MeasurementReportLog_SvRow_Struct aSvRows[ CGPS_MAX_NUM_SV_IN_MEASUREMENT_REPORT ];
} CGPS_HBW_MeasurementReportLog_Struct;

typedef enum
{
	CGPS_GEN8_MeasurementType_LBW_GPS_Measurement 	= 0,
	CGPS_GEN8_MeasurementType_HBW_GPS_Measurement 	= 1,
	CGPS_GEN8_MeasurementType_RxD_Measurement		= 2
} CGPS_GEN8_MeasurementType_type;



#define CGPS_GEN8_MiscStatus_MultipathEstValid 1;	// Bit 0, "AND" with MISC_STATUS to determine if Multipath estimate is valid
#define CGPS_GEN8_MiscStatus_DirValid 2	;			// Bit 1, "AND" with MISC_STATUS to determine if Azi and Elev are valid


/**
	Format for CGPS RF Status
	CGPS_RF_STATUS_REPORT_LOG			= 0x1372,
*/
typedef struct
{
	byte iVersion;
	dword iFCOUNT;
	byte iPLL_ChanAndLockStatus;
	short iBP_Mean_I;
	short iBP_Mean_Q;
	unsigned short iBP_Ampl_I;
	unsigned short iBP_Ampl_Q;

	// There are more fields in this log, but these are the only ones supported by QMSL.
	// Please see CGPS ICD for further fields.

} CGPS_RF_StatusLog_Struct;


typedef struct
{
	byte	iVersion;
	dword	iFCOUNT;
	char	RFIC_ID_PRX[12];
	byte	RFIC_MODE;
	byte	RFIC_LIN_STATE;
	byte	RF_PLL_LOCK_STATUS;
	word	RF_VCO_TUNE_CODE;
	byte	RF_DC_OFFSET_I;
	byte	RF_DC_OFFSET_Q;

	char	ADC_GAIN[12];
	float	ADC_SAMPLE_RATE_FREQ;
	byte	ADC_PRE_MEAN_IQ_VALID;
	float	ADC_PRE_MEAN_I;
	float	ADC_PRE_MEAN_Q;

	byte	BP1_LBW_SAMPLE_RATE;
	short	BP1_LBW_MEAN_I;
	short	BP1_LBW_MEAN_Q;
	float	BP1_LBW_AMPL_I;
	float	BP1_LBW_AMPL_Q;
	float	BP1_LBW_FINE_FREQ;
	byte	BP1_LBW_NOTCH_ACTIVE;
	short	BP1_LBW_GROUP_DELAY;

	byte	BP3_GLO_SAMPLE_RATE;
	short	BP3_GLO_MEAN_I;
	short	BP3_GLO_MEAN_Q;
	float	BP3_GLO_AMPL_I;
	float	BP3_GLO_AMPL_Q;
	byte	BP3_GLO_NOTCH_ACTIVE;
	short	BP3_GLO_GROUP_DELAY;

	byte	BP4_HBW_SAMPLE_RATE;
	short	BP4_HBW_MEAN_I;
	short	BP4_HBW_MEAN_Q;
	float	BP4_HBW_AMPL_I;
	float	BP4_HBW_AMPL_Q;
	float	BP4_HBW_FINE_FREQ;
	byte	BP4_HBW_NOTCH_ACTIVE;
	short	BP4_HBW_GROUP_DELAY;

	dword	MND_CNTR_M;
	dword	MND_CNTR_N;
	dword	MND_CNTR_D;
	byte	GNSS_HW_VER;
	dword	RESERVED;
} GNSS_PRX_RF_HW_StatusLog_Struct;



typedef struct
{
	byte	iVersion;
	dword	iFCOUNT;
	char	RFIC_ID_PRX[12];
	byte	RFIC_MODE;
	byte	RFIC_LIN_STATE;
	byte	RF_PLL_LOCK_STATUS;
	word	RF_VCO_TUNE_CODE;
	byte	RF_DC_OFFSET_I;
	byte	RF_DC_OFFSET_Q;

	char	ADC_GAIN[12];
	float	ADC_SAMPLE_RATE_FREQ;
	byte	ADC_PRE_MEAN_IQ_VALID;
	float	ADC_PRE_MEAN_I;
	float	ADC_PRE_MEAN_Q;

	byte	BP2_DRX_SAMPLE_RATE;
	short	BP2_DRX_MEAN_I;
	short	BP2_DRX_MEAN_Q;
	float	BP2_DRX_AMPL_I;
	float	BP2_DRX_AMPL_Q;
	float	BP2_DRX_FINE_FREQ;
	byte	BP2_DRX_NOTCH_ACTIVE;
	short	BP2_DRX_GROUP_DELAY;

	dword	RESERVED;
} GNSS_DRX_RF_HW_StatusLog_Struct;

typedef enum
{
  GNSS_RficMode_GPS_ZIF             = 1,
  GNSS_RficMode_GPS_LIF             = 2,
  GNSS_RficMode_GNSS_Concurrent_LIF = 5
}GNSS_RficMode_type;

typedef enum
{
  GNSS_RficLinearityState_Low  = 0,
  GNSS_RficLinearityState_High = 1
} GNSS_RficLinearityState_type;

typedef enum
{
  GNSS_RfPllLockStatus_Unknown    = 0,
  GNSS_RfPllLockStatus_Locked     = 1,
  GNSS_RfPllLockStatus_NotLocked  = 2
} GNSS_RfPllLockStatus_type;

/**
	Format for CGPS RF Status
	GPS_DEMOD_SOFT_DECISIONS_LOG			= 0x1253
*/
typedef struct
{
    unsigned char Version;
    unsigned char SvPrn;
    unsigned short DemodId;
    unsigned long BitId;
    unsigned short SoftDecision[20];
    unsigned char NumberOfBits;

} CGPS_Demod_Soft_Decisions_Struct;

typedef struct
{
	unsigned char	Version;
	unsigned char 	GnssType;
	byte 			SvPrn;
	dword			BitId;
	unsigned char 	NumberOfBits;
	word		 	SoftDecision[20];
	dword			RESERVED_1;
	dword			RESERVED_2;
} GNSS_Demod_Soft_Decisions_Struct;

typedef enum
{
	GNSS_GnssTypeGPS	 = 1,
	GNSS_GnssTypeGlonass = 2
} GNSS_GnssType_type;

/**
	Format for CGPS ME Job Info
	CGPS_ME_JOB_INFO_LOG 				= 0x13BA
*/
typedef struct
{
  unsigned short w_JobSeqCnt; /* Job sequence count */
  unsigned char  u_JobType;   /* Job Type */
  unsigned char  u_Sv;        /* SV being specified in the Job */
} CGPS_mc_JobIDType;


typedef struct
{
	unsigned char  		u_Version;     /* Version id of this log */
	unsigned char  		u_Cno;         /* baseband C/N0 of detected signal in dBHz. 0 means no signal found */
	unsigned char  		u_Sensitivity; /* baseband sensitivity of the search attempted in dBHz */
	unsigned short 		w_CodePhase;   /* code phase in chipx1 */
	unsigned short 		x_Doppler;     /* doppler in Hz */
	CGPS_mc_JobIDType 	q_JobId;       /* JobId */
	unsigned long int 	q_FCount;      /* FCount */
} CGPS_ME_JobInfo;


/* Enumeration of the various job types */
typedef enum
{
  CGPS_MC_TYPE_MSA_JOB,
  CGPS_MC_TYPE_FAST_SCAN_JOB,
  CGPS_MC_TYPE_DEEP_JOB,
  CGPS_MC_TYPE_SHALLOW_KNOWN_JOB,
  CGPS_MC_TYPE_SHALLOW_UNKNOWN_JOB,
  CGPS_MC_TYPE_SIDE_LOBE_SCAN_JOB,
  CGPS_MC_TYPE_VERIFY_JOB,
  CGPS_MC_TYPE_VERIFY_TRACK_JOB,
  CGPS_MC_TYPE_BIT_EDGE_JOB,
  CGPS_MC_TYPE_TRACK_JOB,
  CGPS_MC_TYPE_FTM_JOB,
  CGPS_MC_TYPE_PD_JOB,                    /* Prescribed dwell job */
  CGPS_MC_TYPE_SBAS_SEARCH_JOB,
  CGPS_MC_TYPE_DPO_JOB,
  CGPS_MC_TYPE_JOB_END = CGPS_MC_TYPE_DPO_JOB
} CGPS_mc_JobEnumType;



/**
	Parameters for QLIB_DIAG_GPS_SA_RF_VERIF_MODE_SWITCH
*/

/**
	Enumeration for QLIB_DIAG_GPS_SA_RF_VERIF_MODE_SWITCH() -> iFlag field
*/
typedef enum
{
    GPS_SA_RF_VERIF_MODE_SWITCH_ENTER_MODE	      = 0x00, //!< ' Enter Standalone RF verification mode
    GPS_SA_RF_VERIF_MODE_SWITCH_EXIT_MODE         = 0x01  //!< ' Exit Standalone RF verification mode
} gps_sa_rf_verif_mode_switch_enum_type;


/**
	Parameters for QLIB_DIAG_GPS_PRESCRIBED_DWELL
*/

/**
	Enumerations for QLIB_DIAG_GPS_PRESCRIBED_DWELL() -> iFlags field -> bit field

		Bit mask to define request.  Bitmask values defined by
			Bit 0 = Send dwell results
			Bit 1 = 0/1 - Chipx1 / Chipx2
			Bit 2 = Report Incoherent Sums
			Bit 3 = Report Coherent Sums
			Bit 4 = 00 - Start now; 1-Use TOA;
			Bit  = Reserved
			Bit 6 = Use RF/TS
			Bit 7 = Initialize TS and BC at start of dwell (Applicable only if TS is used).
			Bit[15:8] = Reserved
*/
typedef enum
{
	GPS_PRESCRIBED_DWELL_NO_RESULTS						= 0x00,	//!< ' Do not send dwell results
	GPS_PRESCRIBED_DWELL_SEND_RESULTS					= 0x01	//!< ' Send dwell results
} gps_prescribed_dwell_results_enum_type;	//Bit 0

typedef enum
{
	GPS_PRESCRIBED_DWELL_CHIPX1							= 0x00,	//!< ' Chipx1
	GPS_PRESCRIBED_DWELL_CHIPX2							= 0x02	//!< ' Chipx2
} gps_prescribed_dwell_chip_rate_enum_type;	//Bit 1

typedef enum
{
	GPS_PRESCRIBED_DWELL_NO_INCOHER_SUMS					= 0x00,	//!< ' Do not report incoherent sums
	GPS_PRESCRIBED_DWELL_REPORT_INCOHER_SUMS				= 0x04	//!< ' Report incoherent sums
} gps_prescribed_dwell_incoher_sums_enum_type;	//Bit 2

typedef enum
{
	GPS_PRESCRIBED_DWELL_NO_COHER_SUMS					= 0x00,	//!< ' Do not report coherent sums
	GPS_PRESCRIBED_DWELL_REPORT_COHER_SUMS				= 0x08	//!< ' Report coherent sums
} gps_prescribed_dwell_coher_sums_enum_type;	//Bit 3

typedef enum
{
	GPS_PRESCRIBED_DWELL_START_NOW						= 0x00,	//!< ' Start now
	GPS_PRESCRIBED_DWELL_USE_TOA						= 0x10	//!< ' Use TOA
} gps_prescribed_dwell_start_time_enum_type;	//Bit[5:4]

typedef enum
{
	GPS_PRESCRIBED_DWELL_USE_RF						= 0x00,	//!< ' Use RF
	GPS_PRESCRIBED_DWELL_USE_TS							= 0x40	//!< ' Use TS
} gps_prescribed_dwell_source_enum_type;		//Bit 6

typedef enum
{
	GPS_PRESCRIBED_DWELL_NO_INIT						= 0x00,	//!< ' Do not initialize TS and BC at start of dwell
	GPS_PRESCRIBED_DWELL_INIT_TS_AND_BC					= 0x80	//!< ' Initialize TS and BC at start of dwell
} gps_prescribed_dwell_source_init_type;		//Bit 7


/**
	Enumeration for QLIB_DIAG_GPS_PRESCRIBED_DWELL() -> iSearchMode field
*/
typedef enum
{
    GPS_PRESCRIBED_DWELL_SEARCH_MODE_0		   = 0x00, //!< ' Pre-Detection Mode 0: Search Time 10ms
    GPS_PRESCRIBED_DWELL_SEARCH_MODE_1         = 0x01, //!< ' Pre-Detection Mode 1: Search Time 20ms
	GPS_PRESCRIBED_DWELL_SEARCH_MODE_2         = 0x02, //!< ' Pre-Detection Mode 2: Search Time 160ms
    GPS_PRESCRIBED_DWELL_SEARCH_MODE_3         = 0x03, //!< ' Pre-Detection Mode 3: Search Time 20ms BET
	GPS_PRESCRIBED_DWELL_SEARCH_MODE_4         = 0x04, //!< ' Pre-Detection Mode 4: Search Time 20ms P75
    GPS_PRESCRIBED_DWELL_SEARCH_MODE_5         = 0x05, //!< ' Pre-Detection Mode 5: Search Time 2ms
	GPS_PRESCRIBED_DWELL_SEARCH_MODE_6         = 0x06  //!< ' Pre-Detection Mode 6: Search Time 2ms BET
} gps_prescribed_dwell_search_mode_enum_type;



#define GPS_SV_TRACK_ALL 0	//!< The SV ID 0 for GPS_SV_TRACK indicates that all SV's should be tracked

/**
	Parameters for QLIB_DIAG_GPS_START_IQ_TEST()
*/

/**
	Enumerations for QLIB_DIAG_GPS_START_IQ_TEST() -> iRequests field -> bit field

		Bit mask to define request.  Bitmask values defined by
			Bit[1:0] = 00-No FFT Report; 01-Report FFT; 10-Report FFT. Use Hanning window; 11=Reserved.
			Bit 2 = 0-Do not report raw IQ samples; 1-Report raw IQ samples.
			Bit 3 = 1-Report narrowband peak profile.
			Bit 4: RTC selection (ignored if phone is in UMTS mode). 0-GPS; 1-CDMA.
			Bit 5: Start Mode. 0-Start immediately; 1-Start synchronous to FCount.
			Bit[7:6]: Reserved.

*/
typedef enum
{
	GPS_START_IQ_TEST_REQ_NO_FFT						= 0x00,	//!< ' No FFT Report.
	GPS_START_IQ_TEST_REQ_FFT							= 0x01,	//!< ' Report FFT.
	GPS_START_IQ_TEST_REQ_FFT_HANNING					= 0x02	//!< ' Report FFT. Use Hanning window.

} gps_start_iq_test_req_fft_enum_type;	//Bit[1:0]

typedef enum
{
	GPS_START_IQ_TEST_REQ_IQ_NO_SAMPLES					= 0x00,	//!< ' Do not report raw IQ samples.
	GPS_START_IQ_TEST_REQ_IQ_REPORT_SAMPLES				= 0x04	//!< ' Report raw IQ samples.

} gps_start_iq_test_req_iq_enum_type;	//Bit2

typedef enum
{
	GPS_START_IQ_TEST_REQ_NO_NARROWBAND_PEAK_PROFILE	= 0x00,	//!< ' Report no narrowband peak profile.
	GPS_START_IQ_TEST_REQ_NARROWBAND_PEAK_PROFILE		= 0x08	//!< ' Report narrowband peak profile.

} gps_start_iq_test_req_peak_profile_enum_type;	//Bit3

typedef enum
{
	GPS_START_IQ_TEST_REQ_RTC_GPS						= 0x00,	//!< ' RTC selection is GPS
	GPS_START_IQ_TEST_REQ_RTC_CDMA						= 0x10	//!< ' RTC selection is CDMA
} gps_start_iq_test_req_rtc_select;	//Bit4

typedef enum
{
	GPS_START_IQ_TEST_REQ_START_IMMED					= 0x00,	//!< ' Start immediately
	GPS_START_IQ_TEST_REQ_START_SYNC_FCOUNT				= 0x20	//!< ' Start synchronous to Fcount
} gps_start_iq_test_req_start_mode;	//Bit5

/**
	Enumerations for QLIB_DIAG_GPS_START_IQ_TEST() -> iTestSourceInfo field -> bit field

		Bit mask to define Test Source information.  Bitmask values defined by
			Bit 0: 0/1 Input to SD is from RF/TS.
			Bit [1:6]: SV Id.
			Bit 7: 0/1 Data OFF/ON.
			Bit 8: 0/1 Noise ON/OFF.
			Bit [15:9]: Reserved
			Bits 1 to 8 are valid only when input to SD from TS.
*/
typedef enum
{
	GPS_START_IQ_TEST_TS_INPUT_RF					= 0x00,	//!< ' Input to SD is RF.
	GPS_START_IQ_TEST_TS_INPUT_TS					= 0x01	//!< ' Input to SD is TS(Test Source).
} gps_start_iq_test_ts_input_enum_type;	//Bit 0

typedef enum
{
	GPS_START_IQ_TEST_TS_SV_ID_0					= 0x00,	//!< ' SV Id = 0
	GPS_START_IQ_TEST_TS_SV_Id_1					= 0x02,	//!< ' SV Id = 1
	GPS_START_IQ_TEST_TS_SV_Id_2					= 0x04,	//!< ' SV Id = 2
	GPS_START_IQ_TEST_TS_SV_Id_3					= 0x06,	//!< ' SV Id = 3
	GPS_START_IQ_TEST_TS_SV_Id_4					= 0x08,	//!< ' SV Id = 4
	GPS_START_IQ_TEST_TS_SV_Id_5					= 0x0A,	//!< ' SV Id = 5
	GPS_START_IQ_TEST_TS_SV_Id_6					= 0x0C,	//!< ' SV Id = 6
	GPS_START_IQ_TEST_TS_SV_Id_7					= 0x0E,	//!< ' SV Id = 7
	GPS_START_IQ_TEST_TS_SV_Id_8					= 0x10,	//!< ' SV Id = 8
	GPS_START_IQ_TEST_TS_SV_Id_9					= 0x12,	//!< ' SV Id = 9
	GPS_START_IQ_TEST_TS_SV_Id_10					= 0x14,	//!< ' SV Id = 10
	GPS_START_IQ_TEST_TS_SV_Id_11					= 0x16,	//!< ' SV Id = 11
	GPS_START_IQ_TEST_TS_SV_Id_12					= 0x18,	//!< ' SV Id = 12
	GPS_START_IQ_TEST_TS_SV_Id_13					= 0x1A,	//!< ' SV Id = 13
	GPS_START_IQ_TEST_TS_SV_Id_14					= 0x1C,	//!< ' SV Id = 14
	GPS_START_IQ_TEST_TS_SV_Id_15					= 0x1E,	//!< ' SV Id = 15
	GPS_START_IQ_TEST_TS_SV_Id_16					= 0x20,	//!< ' SV Id = 16
	GPS_START_IQ_TEST_TS_SV_Id_17					= 0x22,	//!< ' SV Id = 17
	GPS_START_IQ_TEST_TS_SV_Id_18					= 0x24,	//!< ' SV Id = 18
	GPS_START_IQ_TEST_TS_SV_Id_19					= 0x26,	//!< ' SV Id = 19
	GPS_START_IQ_TEST_TS_SV_Id_20					= 0x28,	//!< ' SV Id = 20
	GPS_START_IQ_TEST_TS_SV_Id_21					= 0x2A,	//!< ' SV Id = 21
	GPS_START_IQ_TEST_TS_SV_Id_22					= 0x2C,	//!< ' SV Id = 22
	GPS_START_IQ_TEST_TS_SV_Id_23					= 0x2E,	//!< ' SV Id = 23
	GPS_START_IQ_TEST_TS_SV_Id_24					= 0x30,	//!< ' SV Id = 24
	GPS_START_IQ_TEST_TS_SV_Id_25					= 0x32,	//!< ' SV Id = 25
	GPS_START_IQ_TEST_TS_SV_Id_26					= 0x34,	//!< ' SV Id = 26
	GPS_START_IQ_TEST_TS_SV_Id_27					= 0x36,	//!< ' SV Id = 27
	GPS_START_IQ_TEST_TS_SV_Id_28					= 0x38,	//!< ' SV Id = 28
	GPS_START_IQ_TEST_TS_SV_Id_29					= 0x3A,	//!< ' SV Id = 29
	GPS_START_IQ_TEST_TS_SV_Id_30					= 0x3C,	//!< ' SV Id = 30
	GPS_START_IQ_TEST_TS_SV_Id_31					= 0x3E,	//!< ' SV Id = 31
} gps_start_iq_test_ts_sv_id_enum_type;	//Bit[6:1]

typedef enum
{
	GPS_START_IQ_TEST_TS_DATA_OFF					= 0x00,	//!< ' Data Off
	GPS_START_IQ_TEST_TS_DATA_ON					= 0x80	//!< ' Data On
} gps_start_iq_test_ts_data_enum_type;	//Bit 7

typedef enum
{
	GPS_START_IQ_TEST_TS_NOISE_OFF					= 0x00,	//!< ' Noise Off
	GPS_START_IQ_TEST_TS_NOISE_ON					= 0x100	//!< ' Noise On
} gps_start_iq_test_ts_noise_enum_type;	//Bit 8


/**
	Enumeration for QLIB_DIAG_GPS_START_IQ_TEST() -> iSM_Point field
		This enumeration defines the block from which signal collection must be done.
			0-Dual point SM collect;
			1-Single point SM collect, Channel 0;
			2-Single point SM collect, Channel 1;
			3-Any other point (single point);
			4-255-Reserved
*/
typedef enum
{
	GPS_START_IQ_TEST_SM_DUAL_POINT_SM				= 0x00,	//!< ' Dual point SM collect.
	GPS_START_IQ_TEST_SM_SINGLE_POINT_SM_0			= 0x01,	//!< ' Single point SM collect, Channel 0.
	GPS_START_IQ_TEST_SM_SINGLE_POINT_SM_1			= 0x02,	//!< ' Single point SM collect, Channel 1.
	GPS_START_IQ_TEST_SM_SINGLE_POINT_OTHER			= 0x03	//!< ' Any other point (single point).
} gps_start_iq_test_sm_point_enum_type;


/**
	Enumeration for QLIB_DIAG_GPS_START_IQ_TEST() -> iCollectPoint field
		This enumeration defines the Collection Point as follows:
			If "Dual point" or "Single point, channel 0" is chosen above (iSM_Point):
			Bit[2:0] allow one of 7 collection points to be routed to the lower 16 bits of the test output.
			Bit[5:3] allow one of 7 other collection points to be routed on the higher 16 bits of the test output.
			Test output is a collection of 32-bit words (two int16s).
			If Collection point is "BC signal log", sample frequency is chosen using Bit[7:6]. See below.
*/

/**
	Enumeration for QLIB_DIAG_GPS_START_IQ_TEST() -> iCollectPoint field -> Bit[2:0]
*/

typedef enum
{
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_NONE						= 0x00,	//!< ' Collection Point = None
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_MISC_BITS					= 0x01,	//!< ' Collection Point = Misc Bits (reset, Test Strobe etc.)
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_BC_OUTPUT					= 0x02,	//!< ' Collection Point = BC Output
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_TS_I_NON_QUANT			= 0x03,	//!< ' Collection Point = TS 'I', non-quantized
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_BC_I_FILTER_OUT			= 0x04,	//!< ' Collection Point = BC 'I' filter output
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_BC_I_NON_QUANT			= 0x05,	//!< ' Collection Point = BC 'I', non-quantized
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_BC_SIGNAL_LOG				= 0x06,	//!< ' Collection Point = BC signal log, lower 16 bits
	GPS_START_IQ_TEST_COLLECT_LOWER16BITS_RTC_INPUT					= 0x07	//!< ' Collection Point = RTC input, Chip and cycle count
} gps_start_iq_test_collect_lower16bits_enum_type;	//Bit[2:0], Valid if "Dual point" or "Single point, channel0" is chosen above in gps_start_iq_test_sm_point_enum_type

/**
	Enumeration for QLIB_DIAG_GPS_START_IQ_TEST() -> iCollectPoint field -> Bit[5:3]
*/

typedef enum
{
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_TS_IQ					= 0x00,	//!< ' Collection Point = TS 'IQ'
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_MISC_BITS				= 0x08,	//!< ' Collection Point = Rx 'IQ' input
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_BC_OUTPUT				= 0x0A,	//!< ' Collection Point = Aux 'IQ' input
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_TS_I_NON_QUANT			= 0x18,	//!< ' Collection Point = TS 'Q', non-quantized
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_BC_I_FILTER_OUT			= 0x20,	//!< ' Collection Point = BC 'Q' filter output
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_BC_I_NON_QUANT			= 0x28,	//!< ' Collection Point = BC 'Q', non-quantized
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_BC_SIGNAL_LOG			= 0x30,	//!< ' Collection Point = BC signal log, upper 16 bits
	GPS_START_IQ_TEST_COLLECT_HIGHER16BITS_RTC_INPUT				= 0x38	//!< ' Collection Point = RTC input, FC
} gps_start_iq_test_collect_higher16bits_enum_type;	//Bit[5:3], Valid if "Dual point" or "Single point, channel1" is chosen above in gps_start_iq_test_sm_point_enum_type

/**
	Enumeration for QLIB_DIAG_GPS_START_IQ_TEST() -> iCollectPoint field -> Bit[7:6]
*/
typedef enum
{
	GPS_START_IQ_TEST_COLLECT_SAMPLE_FREQ_4BIT_I_16MHZ				= 0x00,	//!< ' Sample Frequency = 4-bit I at 16.368MHz. Valid if collection point is "BC signal log"
	GPS_START_IQ_TEST_COLLECT_SAMPLE_FREQ_4BIT_Q_16MHZ				= 0x40,	//!< ' Sample Frequency = 4-bit Q at 16.368MHz. Valid if collection point is "BC signal log"
	GPS_START_IQ_TEST_COLLECT_SAMPLE_FREQ_2BIT_IQ_16MHZ				= 0x80,	//!< ' Sample Frequency = 2-bit I and Q at 16.368MHz. Valid if collection point is "BC signal log"
	GPS_START_IQ_TEST_COLLECT_SAMPLE_FREQ_4BIT_IQ_8MHZ				= 0xC0	//!< ' Sample Frequency = 4-bit I and Q at 8.1844MHz. Valid if collection point is "BC signal log"
	} gps_start_iq_test_collect_sample_freq_enum_type;	//Bit[7:6], Valid if collection point is "BC signal log"

/**
	Enumeration for QLIB_DIAG_GPS_START_IQ_TEST() -> iCollectPoint field -> Bit[7:0] if "Other" is chosen in "SM_Point"
*/
typedef enum
{
	GPS_START_IQ_TEST_COLLECT_OTHER_SAMPLE_SERVER_2MHZ				= 0x00,	//!< ' Collection Point = Sample Server, 2MHz. Valid if "Other" is chosen in "SM_Point"
	GPS_START_IQ_TEST_COLLECT_OTHER_SAMPLE_SERVER_1KHZ				= 0x01,	//!< ' Sample Frequency = Sample Server, 1kHz. Valid if "Other" is chosen in "SM_Point"
	GPS_START_IQ_TEST_COLLECT_OTHER_DEC_OFFSET_ESTIMATOR			= 0x02,	//!< ' Sample Frequency = DC Offset estimator. Valid if "Other" is chosen in "SM_Point"
	GPS_START_IQ_TEST_COLLECT_OTHER_AFLT							= 0x03,	//!< ' Sample Frequency = AFLT. Valid if "Other" is chosen in "SM_Point"
	GPS_START_IQ_TEST_COLLECT_OTHER_AFLT_AND_BC						= 0x04	//!< ' Sample Frequency = AFLT and BC simultaneous. Valid if "Other" is chosen in "SM_Point"
} gps_start_iq_test_collect_other_enum_type;	//Bit[7:0], Valid if "Other" is chosen in "SM_Point"

/**
	Below this are CGPS defintitions introduced by QMSL, above are defitions created by the CGPS ICD document
*/


/**
	Structure for parameters for QLIB_DIAG_GPS_START_IQ_TEST()
*/
typedef struct
{
	/**
		Defines the request for FFT, IQ samples, Narrowband peak profile, RTC Selection, and Start Mode.

		These requests are combined into a single byte using the following enumeration values
		\code
           FFT                      - gps_start_iq_test_req_fft_enum_type
           IQ samples               - gps_start_iq_test_req_iq_enum_type
           Narrow band peak profile - gps_start_iq_test_req_peak_profile_enum_type
           RTC Selection            - gps_start_iq_test_req_rtc_select
           Start Mode               - gps_start_iq_test_req_start_mode
		\endcode

		Example:
			gps_start_iq_test_struct_type oIQ_Test;
			oIQ_Test.iRequests = GPS_START_IQ_TEST_REQ_FFT | GPS_START_IQ_TEST_REQ_IQ_REPORT_SAMPLES | GPS_START_IQ_TEST_REQ_START_IMMED;
	*/

	/**
			Bit mask to define requests.  Bitmask values defined by
			Bit[1:0] = 00-No FFT Report; 01-Report FFT; 10-Report FFT. Use Hanning window; 11=Reserved.
			Bit 2 = 0-Do not report raw IQ samples; 1-Report raw IQ samples.
			Bit 3 = 1-Report narrowband peak profile.
			Bit 4: RTC selection (ignored if phone is in UMTS mode). 0-GPS; 1-CDMA.
			Bit 5: Start Mode. 0-Start immediately; 1-Start synchronous to FCount.
			Bit[7:6]: Reserved.
	*/
	unsigned char iRequests;

	/**
		FCount. Unsigned. Applicable only if set to "Start Mode" in "iRequests" field above.
	*/
	unsigned long iFCount;


	/**
			Bit mask to define Test Source information.  Bitmask values defined by
			Bit 0: 0/1 Input to SD is from RF/TS.
			Bit [1:6]: SV Id.
			Bit 7: 0/1 Data OFF/ON.
			Bit 8: 0/1 Noise ON/OFF.
			Bit [9:15]: Reserved
			Bits 1 to 8 are valid only when input to SD from TS.
	*/

	unsigned short iTestSourceInfo;

	/**
		Unsigned value in dBHz. This field is valid only when TS is enabled.
	*/
	unsigned char iTestSourceSignalStrength;

	/**
		Signed value (1000/65536 Hz/Bit). This field is valid only when TS is enabled.
	*/
	//unsigned short iTestSourceDoppler;
	long iTestSourceDoppler; //Changed to long in order to match the Perl file IqTest.pm

	/**
		Number of Samples collected in multiples of 1024 samples, limit is CGPS_SAMPLE_GROUP_COUNT_MAX
	*/
	unsigned short iKiloSamples;

	/**
		Number of FFT integrations. 0:1000-Number of integrations. 1001-65,536-Reserved. Applicable only if FFT reporting is chosen.
	*/
	unsigned short iFFT_Integration;

	/**
		This enumeration defines the block from which signal collection must be done.
		0-Dual point SM collect;
		1-Single point SM collect, Channel 0;
		2-Single point SM collect, Channel 1;
		3-Any other point (single point);
		4-255-Reserved
	*/
	unsigned char iSM_Point;

	/**
		This enumeration defines the Collection Point as follows:
		If "Dual point" or "Single point, channel 0" is chosen above (iSM_Point):
		Bit[2:0] allow one of 7 collection points to be routed to the lower 16 bits of the test output.
		Bit[5:3] allow one of 7 other collection points to be routed on the higher 16 bits of the test output.
		Test output is a collection of 32-bit words (two int16s).
		If Collection point is "BC signal log", sample frequency is chosen using Bit[7:6]. See below.
	*/
	unsigned char iCollectPoint;

	/**
		0:6 - Number of bits to shift samples in GPS Searcher.
		7:255 - Reserved.
		Shift 0 shifts for for weak signals
		Shift 6 shifts for for strong signals
	*/
	unsigned char iScale;

	/**
		Initial Collection Start Delay. Range: 0 to 65535 ms.
	*/
	unsigned short iStartDelay;

	/**
		Wideband FFT Center Frequency. Signed: Sample_Rate/2^32 per bit.
		Range: -Sample_Rate/2 to Sample_Rate/2 Hz.
		Applicable only when collect point are after cordic.
		Else center frequency is zero.
	*/
	unsigned long iWidebandCenterFreq;


	/**
		Narrowband FFT Center Frequency. Signed: Sample_Rate/2^32 per bit.
		Range: -Sample_Rate/2 to Sample_Rate/2 Hz.
		Applicable only when collect point are after cordic.
		Else center frequency is zero.
	*/
	unsigned long iNarrowbandCenterFreq;

	/**
		Number of Wideband Peaks to identify. Range: 0 to 255.
	*/
	unsigned char iNumWidebandPeaks;

	/**
		Number of Samples on Each Side of Peak to Include in Peak Sum. Range: 0 to 255.
	*/
	unsigned char iNumWidebandAdjSamples;

	/**
		Number of Narrowband Peaks to identify. Range: 0 to 255.
	*/
	unsigned char iNumNarrowbandPeaks;

	/**
		Number of Samples on Each Side of Peak to Include in Peak Sum. Range: 0 to 255.
	*/

	unsigned char iNumNarrowbandAdjSamples;

} gps_start_iq_test_struct_type;


/**
	Commands for QLIB_DIAG_GPS_SV_TRACK() request
*/
typedef enum
{
	GPS_SV_TRACK_START_CLEAR_ALL	= 1,				//!< ' CGPS engine starts tracking and clears any satellites currently being tracked
	GPS_SV_TRACK_START_NO_CLEAR		= 2,				//!< ' CGPS engine starts tracking without clearing the satellites currently being tracked
	GPS_SV_TRACK_STOP_CLEAR_ALL		= 3,				//!< ' CGPS engine stops tracking and clears any satellites currently being tracked
	GPS_SV_TRACK_STOP_NO_CLEAR		= 4,					//!< ' CGPS engine stops tracking without clearing the satellites currently being tracked
	GPS_SV_TRACK_START_CLEAR_ALL_NO_BITEDGE_DETECTION = 5,	//!< ' CGPS engine starts tracking, clears all, and does not use bit edge detection
	GPS_SV_TRACK_START_NO_CLEAR_NO_BITEDGE_DETECTION = 6,	//!< ' CGPS engine starts tracking, does not clear, and does not use bit edge detection
	GPS_SV_TRACK_START_CLEAR_ALL_FORCE_DPO = 7,				//!< ' CGPS engine starts tracking in Dynamic Power Optimization mode clearing current status
	GPS_SV_TRACK_START_NO_CLEAR_FORCE_DPO = 8,				//!< ' CGPS engine starts tracking in Dynamic Power Optimization mode without clearing current status
	GPS_SV_TRACK_START_SBAS_CLEAR_ALL = 9,					//!< ' CGPS engine starts tracking in SBAS Mode clearing current status
	GPS_SV_TRACK_START_SBAS_NO_CLEAR = 10,					//!< ' CGPS engine starts tracking in SBAS Mode without clearing current status
	GPS_SV_TRACK_START_SBAS_CLEAR_ALL_NO_SYMBOL_EDGE_DETECTION = 11,	//!< ' CGPS engine starts tracking in SBAS Mode, clears current status, no SED
	GPS_SV_TRACK_START_SBAS_NO_CLEAR_NO_SYMBOL_EDGE_DETECTION = 12		//!< ' CGPS engine starts tracking in SBAS Mode without clearing current status, no SED
} gps_sv_track_command_enum_type;


/**
	CGPS Event types

	These are events, which can be receipt of logs or events. They constitute a particular event
	in the CGPS behavior for QMSL.

*/
typedef enum
{
	/*
		Enter / exit verification mode.  When entering or exiting the verification mode
		it is possible to recieve either an error, or the event which indicates the mode
		switch was successful
	*/
	CGPS_Enter_Verification_Mode,				//!<' Mapped to event _EVENT_GPS_LM_ENTER_SA_RF_VERIF
	CGPS_Exit_Verification_Mode,				//!<' Mapped to event _EVENT_GPS_LM_EXIT_SA_RF_VERIF
	CGPS_Verification_Mode_Error,				//!<' Mapped to event _EVENT_GPS_LM_ERROR_SA_RF_VERIF

	CGPS_IQ_FFT_DataReceived,					//!<' Mapped to log CGPS_IQ_DATA_LOG
	CGPS_IQ_Test_Complete,						//!<' Mapped to event _EVENT_GPS_LM_IQ_TEST_COMPLETE
	CGPS_Dwell_Complete,						//!<' Mapped to event _EVENT_GPS_LM_PD_COMPLETE
	CGPS_Dwell_Status,							//!<' Mapped to log CGPS_PRESCRIBED_DWELL_STATUS_LOG

	CGPS_GPS_MeasurementReceived,				//!<' Mapped to log CGPS_MEASUREMENT_REPORT_LOG
	CGPS_RF_Status_Recieved,					//!<' Mapped to log CGPS_RF_STATUS_REPORT_LOG

	CGPS_Demodulation_Soft_Decision,			//!<' Mapped to log GPS_DEMOD_SOFT_DECISIONS_LOG

	GNSS_GPS_MeasurementReceived,      			//!<' Mapped to log GNSS_GPS_MEASUREMENT_REPORT_LOG
	GNSS_GLONASS_MeasurementReceived,  			//!<' Mapped to log GNSS_GLONASS_MEASUREMENT_REPORT_LOG
	GNSS_GPS_HBW_RXD_MeasurementReceived,     	//!<' Mapped to log GNSS_GPS_HBW_RXD_MEASUREMENT_LOG
	GNSS_DEMOD_MeasurementReceived,				//!<' Mapped to log GNSS_DEMOD_SOFT_DECISIONS_LOG
	GNSS_PRX_RF_Status_Recieved,    			//!<' Mapped to log GNSS_PRX_RF_HW_STATUS_REPORT_LOG
	GNSS_DRX_RF_Status_Recieved,    			//!<' Mapped to log GNSS_DRX_RF_HW_STATUS_REPORT_LOG

	CGPS_WB_FFT_STATS_C_Complete,				//!<' Mapped to log LOG_CGPS_WB_FFT_STATS_C

	CGPS_QMSL_Event_Max							//!<' Used to determine the last event ID for sizing arrays and error checking
} 
QMSL_CGPS_Event_Enum;


/**
	CGPS IQ FFT Common Info
*/
typedef struct
{
  byte  iVersion;
  byte  iRespType;
  dword iTotal_Pkts;
} CGPS_IQ_FFT_CommonInfo;

/**
	CGPS IQ Config Info
*/
typedef struct
{
	// SM Point
	byte iSmPoint;

	//! Collection point
	byte iCollectPoint;

	//! Number of sample groups (groups of 1024 samples)
	word iNumSamples_1k;

	//! Total number of samples--iNumSamples_1k * 1024
	dword iNumSamples;

	//! Number of integrations, as reported by log 0x138A.  This value is known to not be valid
	word iIntegrations_FromLog;

	//! Peak power, converted to dB
	float dPeakPower_dB;

	//! Misc config info
	byte iFlags;

	//! IQ Frequency, copied as-is from the log
	unsigned long iIQ_BP_Freq;

	//! FFT frequency converted using the formula p_ConfigPkt->z_IqDM_Config.l_BP_Freq * 2.046e6/(1<<19)
	float dFFT_BP_Freq;

	//! Was hanning window used
	unsigned char bHanningWindowUsed;

	//! FFT Center
	unsigned long iFFT_CenterFreq;

} CGPS_IQ_FFT_ConfigInfo;


/**
	CGPS Status structure for IQ and FFT Data configuration
*/
typedef struct
{
	//! SM data Point
	byte iSmPt;

	//! Collected data point
	byte iCollectPt;

	//! Input file name (not used)
	char sInFileName[1024];

	//! IQ Output file name
	char sOutIqFileName[1024];

	//! FFT Output file name
	char sOutFftFileName[1024];

	//! Flag to track if IQ file should be created, if the name is not empty
	unsigned char bShouldIQ_FileBeCreated;

	//! Flag to track if FFT file should be created, if the name is not empty
	unsigned char bShouldFFT_FileBeCreated;

	//! Flag to track whether IQ File is created
	unsigned char bIqFileCreated;

	//! Flag to track whether FFT File is created
	unsigned char bFftFileCreated;

	//! Common IQ FFT Info
	CGPS_IQ_FFT_CommonInfo oCommonInfo;

	//! Common Config Info
	CGPS_IQ_FFT_ConfigInfo oConfigInfo;

	//! Number of IQ Measured samples
	unsigned long iNum_IQ_Measured_Samples;

	//! Number of FFT Measured samples
	unsigned long iNum_FFT_Measured_Samples;

	//! Number of FFT Calculated values
	unsigned long iNum_FFT_Calculated_Values;

	//! Time stamp of the first IQ data log
	byte iFirstIQ_LogTimeStamp[QMSL_DIAG_TIME_STAMP_SIZE];

} CGPS_IQ_FFT_DataConfig;


//! Maximum number of GPS Measurement report logs that will be stored
#define CGPS_MAX_MEASUREMENTS_STORAGE 1000

//! Number of samples in a 1k sample group
#define CGPS_IQ_SAMPLE_GROUP_SIZE 1024

//! Maximum number of 1K IQ Samples
#define CGPS_IQ_SAMPLE_GROUP_COUNT_MAX 450

//! Maximum number of 1K FFT Samples (reported from mobile)
#define CGPS_FFT_SAMPLE_GROUP_COUNT_MAX 32

//! Maximum number of IQ samples per capture
#define CGPS_MAX_IQ_SAMPLES_PER_CAPTURE ( CGPS_IQ_SAMPLE_GROUP_COUNT_MAX * CGPS_IQ_SAMPLE_GROUP_SIZE )

//! Maximum size of IQ data returned from the phone including all collections (0..1000), 1k sample sizes (1..450)
//! The realistic limit is 10 groups of 450k samples
#define CGPS_MAX_IQ_SAMPLES_PER_IQ_CAPTURE_COMMAND ( 10 * CGPS_IQ_SAMPLE_GROUP_COUNT_MAX * CGPS_IQ_SAMPLE_GROUP_SIZE)

//! Maximum number of calculated FFT points based on the maximum IQ sample size
#define CGPS_MAX_CALCULATED_FFT_SAMPLES_PER_CAPTURE ( CGPS_MAX_IQ_SAMPLES_PER_CAPTURE )

//! Minimum SV ID
#define CGPS_MIN_SV_ID 1

//! Maximum SV ID
#define CGPS_MAX_SV_ID 32

//! Minimum SV ID
#define CGPS_MIN_SBAS_SV_ID 120

//! Maximum SV ID
#define CGPS_MAX_SBAS_SV_ID 138

/**
	Measurement summary (calculated values) structure for CGPS measurement reports
	.
*/
typedef struct
{
	byte iSV_ID;				//!<' SV ID

	dword iNumReports;			//!<' Total number of reports that have been found

	// C/N
	double dC_N_Sum;		//!<' Sum of all C/N measurements, in integer form
	double dC_N_SumSquares;		//!<' Sum of the squares of C/N measurements
	double dC_N_Average_dB;		//!<' Average dB value of C_N
	double dC_N_StdDev_dB;		//!<' Standard deviation dB value of C_N
	double dC_N_Min_dB;			//!<' Minimum dB value of C_N
	double dC_N_Max_dB;			//!<' Maximum dB value of C_N

	// Speed
	double dSpeed_Sum;			//!<' Sum of all speed measurements
	double dSpeed_SumSquares;	//!<' Sum of the squares of speed
	double dSpeed_Average_mps;	//!<' Average meters per second value of Speed
	double dSpeed_StdDev_mps;	//!<' Standard deviation meters per second value of speed
	double dSpeed_Min_mps;		//!<' Minimum mps value of Speed
	double dSpeed_Max_mps;		//!<' Maximum mps value of Speed

	// Acceleration
	double dLastAcceleration;		//!<' The last calculated acceleration
	double dLastSpeed;				//!<' The last recorded speed
	dword  iLastFCOUNT;				//!<' The last recorded FCOUNT
	double dMaxAcceleration;		//!<' Maximum calculated acceleration
	dword iMaxAccelerationFCOUNT;	//!<' FCOUNT at maximum calculated acceleration
	double dMaxAccelerationTime;		//!<' iMaxAccelerationTime * 0.001

	// System processing loss
	double dSystemProcessingLoss;	//!<' 173.9 - 130.0 - dC_N_Average_dB

	// Average Acceleration
	double dLastAvgAcceleration;		//!<' The last calculated acceleration
	double dLastSpeedBuffer[12];		//!<' The last recorded speed
	dword  iLastFCOUNTBuffer[12];		//!<' The last recorded FCOUNT
	double dMaxAvgAcceleration;			//!<' Maximum calculated acceleration
	dword  iMaxAvgAccelerationFCOUNT;	//!<' FCOUNT at maximum calculated acceleration
	double dMaxAvgAccelerationTime;		//!<' iMaxAccelerationTime * 0.001
	word   iLastBufferIndex;			//!<' The last recorded speed/FCOUNT index for
										//!<' circular buffer of 12 consecutive valid values
	double dLast_5Second_AvgAcceleration;		//!<' The last calculated acceleration
	double dLast_5Second_SpeedBuffer[12];		//!<' The last recorded speed
	dword  iLast_5Second_FCOUNTBuffer[12];		//!<' The last recorded FCOUNT
	double dMax_5Second_AvgAcceleration;			//!<' Maximum calculated acceleration
	dword  iMax_5Second_AvgAccelerationFCOUNT;	//!<' FCOUNT at maximum calculated acceleration
	double dMax_5Second_AvgAccelerationTime;		//!<' iMaxAccelerationTime * 0.001
	word   iLast_5Second_BufferIndex;			//!<' The last recorded speed/FCOUNT index for
										//!<' circular buffer of 5 consecutive valid values

	// Need to add Doppler
} CGPS_MeasurementReportInfo;

/**
	Summary structure for CGPS RF Status reports (CGPS_RF_STATUS_REPORT_LOG).
	Keeps track of BP Mean I and Q and BP Amplitude I and Q, which are required
	for the CGPS AGC test.


*/
typedef struct
{
	dword iNumReports;			//!<' Total number of reports that have been found

	//
	// Stats for BP Mean
	//
	double dSum_BP_Mean_I;		//!<' Sum of all BP Mean I values
	double dSum_BP_Mean_Q;		//!<' Sum of all BP Mean Q values

	double dSumSq_BP_Mean_I;	//!<' Sum of squares of all BP Mean I values
	double dSumSq_BP_Mean_Q;	//!<' Sum of squares of all BP Mean Q values

	double dMin_BP_Mean_I;		//!<' Minimum of all BP Mean I values
	double dMin_BP_Mean_Q;		//!<' Minimum of all BP Mean Q values

	double dMax_BP_Mean_I;		//!<' Maximum of all BP Mean I values
	double dMax_BP_Mean_Q;		//!<' Maximum of all BP Mean Q values

	double dAvg_BP_Mean_I;		//!<' Average of all BP Mean I values
	double dAvg_BP_Mean_Q;		//!<' Average of all BP Mean Q values

	double dStdDev_BP_Mean_I;	//!<' Standard deviation of all BP Mean I values
	double dStdDev_BP_Mean_Q;	//!<' Standard deviation of all BP Mean Q values


	//
	// Stats for BP Amplitude
	//
	double dSum_BP_Ampl_I;		//!<' Sum of all BP Amplitude I values
	double dSum_BP_Ampl_Q;		//!<' Sum of all BP Amplitude Q values

	double dSumSq_BP_Ampl_I;	//!<' Sum of squares of all BP Amplitude I values
	double dSumSq_BP_Ampl_Q;	//!<' Sum of squares of all BP Amplitude Q values

	double dMin_BP_Ampl_I;		//!<' Minimum of all BP Amplitude I values
	double dMin_BP_Ampl_Q;		//!<' Minimum of all BP Amplitude Q values

	double dMax_BP_Ampl_I;		//!<' Maximum of all BP Amplitude I values
	double dMax_BP_Ampl_Q;		//!<' Maximum of all BP Amplitude Q values

	double dAvg_BP_Ampl_I;		//!<' Average of all BP Amplitude I values
	double dAvg_BP_Ampl_Q;		//!<' Average of all BP Amplitude Q values

	double dStdDev_BP_Ampl_I;	//!<' Standard deviation of all BP Amplitude I values
	double dStdDev_BP_Ampl_Q;	//!<' Standard deviation of all BP Amplitude Q values

} CGPS_BP_AGC_RF_StatusInfo;

/**
	Structure to map CGPS_WB_FFT_STATS_C (0x1487) 
*/
typedef struct
{
	unsigned long	iCNoDBHz;	//!<' Signal strength calculated in 0.1 dBHz as part of WBIQ test
	long			iFreq;      //!<' Frequency in Hz calculated as part of WBIQ test			
} CGPS_WB_FFT_Stats;


#define GNSS_LBW_AGC  0
#define GNSS_RXD_AGC  1
#define GNSS_GLO_AGC  2
#define GNSS_HBW_AGC  3
/**
	Structure to map the log data for the CGPS prescribed dwell log (CGPS_PRESCRIBED_DWELL_STATUS_LOG, log 0x1374).
*/
typedef struct
{
	unsigned char VERSION;
	unsigned char RESP_TYPE;
	unsigned long TOTAL_PACKETS;
	unsigned long PACKET_NUM;
	unsigned char SV_ID;
	unsigned long GPS_RTC;
	unsigned char STATUS_FLAG;
	unsigned char SRCH_MODE_INDEX;
	unsigned short POST_DETECTION_N;
	unsigned short CNO_DB;
	unsigned long CODE_PHASE;
	unsigned long DOPP;
	unsigned long MULTIPATH_EST;
	unsigned long RESERVED;


} CGPS_PrescribedDwell_LogStruct;

//! Constants for the RESP_TYPE values in CGPS_PrescribedDwell_LogStruct
typedef enum
{
	CGPS_PrescribedDwell_Type_DwellResults	= 0,
	CGPS_PrescribedDwell_Type_IncohSums = 1,
	CGPS_PrescribedDwell_Type_CohSums = 2
} CGPS_PrescribedDwell_ResponseType;

//! Bit mask for STATUS_FLAG values in CGPS_PrescribedDwell_LogStruct
typedef enum
{
	CGPS_PrescribedDwell_Status_ValidPeak	= 1,	//!< '  Bit 0, 2^0 = valid peak indicator
	CGPS_PrescribedDwell_Status_Multipath	= 2		//!< '  Bit 1, 2^1 = multipath indicator
} CGPS_PrescribedDwell_Status;


//! Constants for the SRCH_MODE_INDEX values in CGPS_PrescribedDwell_LogStruct
typedef enum
{
	CGPS_PrescribedDwell_SearchMode_10ms	= 0,
	CGPS_PrescribedDwell_SearchMode_20ms	= 1,
	CGPS_PrescribedDwell_SearchMode_160ms	= 2
} CGPS_PrescribedDwell_SearchMode;

typedef struct
{
	byte	Version;
	dword	FCOUNT;
	dword	SEQ_NUM;
	byte	RxD_FLAG;
	byte	STATUS_FLAG;
	char	SV;
	float	DOPP_HZ;
	float	DOPP_UNC_HZ;
	float	CNO_DBHZ;
	float	CODE_PHASE_CHIPS;
	float	CODE_PHASE_UNC_CHIPS;
	byte	SRCH_MODE;
	word	NUM_POSTD;
	byte	GLO_HW_CHAN;
	byte	PFA;
	word	DWELL_CNT;
	word	TOTAL_NUM_DWELLS;
	word	PASS_DWELL_CNT;
	dword	RESERVED;
} CGPS_GEN8_PrescribedDwell_LogStruct;


//! Constants for the SRCH_MODE_INDEX values in CGPS_GEN8_PrescribedDwell_LogStruct
typedef enum
{
	GPS_LBW_Chipx1_10ms_PDI                =  0,
	GPS_LBW_Chipx2_10ms_PDI                =  1,
	GPS_LBW_Chipx1_20ms_PDI                =  2,
	GPS_LBW_Chipx2_20ms_PDI                =  3,
	GPS_LBW_Chipx2_160ms_PDI               =  4,
	GPS_LBW_Chipx2_BET                     =  5,
	GPS_LBW_Chipx1_20ms_PDI_750_Hz         =  6,
	GPS_LBW_Chipx2_20ms_PDI_750_Hz         =  7,
	GPS_LBW_Chipx2_2ms_PDI                 =  8,
	GPS_LBW_Chipx2_2ms_PDI_BET             =  9,
	GPS_HBW_Chipx20_20ms_PDI               = 10,
	GPS_HBW_Chipx20_20ms_PDI_NU4           = 11,
	GPS_HBW_Chipx20_20ms_PDI_NU8           = 12,
	GLO_Chipx1_5ms_PDI                     = 13,
	GLO_Chipx2_5ms_PDI                     = 14,
	GLO_Chipx1_10ms_PDI                    = 15,
	GLO_Chipx2_10ms_PDI                    = 16,
	GLO_Chipx1_20ms_PDI                    = 17,
	GLO_Chipx2_20ms_PDI                    = 18,
	GLO_Chipx2_BET_10                      = 19,
	GLO_Chipx2_BET_20                      = 20,
	GLO_Chipx1_20ms_PDI_750_Hz             = 21,
	GLO_Chipx2_20ms_PDI_750_Hz             = 22
} CGPS_GEN8_PrescribedDwell_SearchMode;

//! Bit mask for STATUS_FLAG values in CGPS_PrescribedDwell_LogStruct
typedef enum
{
	CGPS_GEN8_PrescribedDwell_Status_ValidPeak	= 1,	//!< '  Bit 0, 2^0 = valid peak indicator
	CGPS_GEN8_PrescribedDwell_Status_Automatic	= 2,
	CGPS_GEN8_PrescribedDwell_Status_Truncated	= 4,
	CGPS_GEN8_PrescribedDwell_Status_Randomized	= 8
} CGPS_GEN8_PrescribedDwell_Status;

typedef enum
{
	CGPS_PrescribedDwell_RxD_Type_NoRxD = 0,
	CGPS_PrescribedDwell_RxD_Type_RxD_Linked = 1,
	CGPS_PrescribedDwell_RxD_Type_RxD_Only = 2
} CGPS_GEN8_RxD_Type;

/**
	Summary structure for CGPS RF Status reports (CGPS_RF_STATUS_REPORT_LOG).
	Keeps track of BP Mean I and Q, which are required for the CGPS AGC test.

*/
typedef struct
{
	//! Total number of event reports that have been found, this is based on counting the number of times that event
	//! _EVENT_GPS_LM_PD_COMPLETE #1250 is received
	dword iNumEventReports;

	/*
		For this release of QMSL, the CGPS_PRESCRIBED_DWELL_STATUS_LOG, log 0x1374, is not being
		parsed, so certain parameters will not be reported in this structure.  In the next
		major release, these values will be filled in.
	*/
	//! Total number of log reports that have been found, this is based on counting the number of times that the
	//! CGPS_PRESCRIBED_DWELL_STATUS_LOG, 0x1374, is received
	dword iNumLogReports;

	//! Total number of valid peaks that have been found by counting STATUS_FLAG==1
	dword iNumValidPeaks;

	//! Maximum C/No measured
	double dMaxCNO;

	//! Average C/No measured
	double dAverageCNO;

	//! Standard deviation of C/No
	double dStdDevCNO;

	//! Sum of CNO
	double dSumCNO;

	//! Sum of CNO squares (for standard deviation)
	double dSumOfSquaresCNO;

	// Last search mode reported -- srch_mode field
	CGPS_PrescribedDwell_LogStruct oLastPrescribedDwellStatus;



} CGPS_PrescribedDwell_StatusInfo;

/**
*/
/**
	Statistics for GPS Demodulation soft decisions, used for GSP BER testing,
	based on log GPS_DEMOD_SOFT_DECISIONS_LOG, 0x1253
*/
typedef struct
{
	//! Total number of logs received
	dword iNumLogReports;

	//! Total number of bits received
	dword iNumBits;

	//! Total number of bit errors
	dword iNumBitErrors;

	//! Calculated BER
	double dBER;

	// Copy of the last reported log, GPS_DEMOD_SOFT_DECISIONS_LOG, 0x1253
	CGPS_Demod_Soft_Decisions_Struct oLastDemodSoftDecisionLog;



} CGPS_BER_StatusInfo;


/**
	Measurement and status structure for CGPS
*/
typedef struct
{
	//! Keeps track of whether an event has occurred
	unsigned char abHasEventOccurred[ CGPS_QMSL_Event_Max ];

	//! Configuration that is retrieved from the log messages for CGPS IQ Data Capture and FFT
	CGPS_IQ_FFT_DataConfig oIQ_FFT_DataConfig;

	/**
		C/N and Doppler measurement structure, includes all "GPS Measurement Report" log data for each SV,
		indexed by SV ID.  The zero index is not used.
	*/
	CGPS_MeasurementReportInfo oMeasurementReports[ CGPS_MAX_SV_ID +  1];

	//! Baseband Processor AGC statistics from the "RF Status" log
	CGPS_BP_AGC_RF_StatusInfo oRF_Status_AGC;

	//! Prescribed dwell status
	CGPS_PrescribedDwell_StatusInfo oDwellStatus;

	//! BER Status
	CGPS_BER_StatusInfo oBER_Status;

	CGPS_BP_AGC_RF_StatusInfo	oGNSS_BP_RF_Status_AGC[4];

	//! WB FFT Status (Log 0x1487)
	CGPS_WB_FFT_Stats oWB_FFT_Stats;

} CGPS_Status_Struct;


#define CGPS_MAX_SBAS_DECODED_WORDS 16

typedef  struct
{
	dword	q_BlankingStartRtc;
	word	w_NumBlankedSymbs;
}CGPS_SbasBlankingInfoType_Struct;

typedef struct
{
	byte	u_SvPrnNum;
	dword	q_GpsRTC;  /* GPSRTC of first sample */
	byte	u_LogCnt;
	word	w_PreamblePosSamp;
	byte	u_SbasVitFlags;
	CGPS_SbasBlankingInfoType_Struct    z_SbasBlankingInfo[2];
	word	w_NumVitDecBits;
	dword	q_DecBits[CGPS_MAX_SBAS_DECODED_WORDS];
}CGPS_SbasDecodedWords_Struct;


#define CGPS_MAX_SBAS_SOFT_DEC 500
typedef struct
{
	unsigned char		u_SvPrnNum;
	unsigned long int	q_GpsRTC;  /* GPSRTC of first sample */
	unsigned char		u_LogCnt;
	unsigned short		w_BlankStartIdx;
	unsigned short		w_NumBlankedSamp;
	unsigned long int	q_CostasDerDopp;
	unsigned char		u_CostasFlags;
	unsigned char		u_Reserved[10];
	unsigned short		w_NumSoftSymbs;
	signed char			b_SoftDecisions[CGPS_MAX_SBAS_SOFT_DEC];
}CGPS_SbasDemodSoftSymbols_Struct;


typedef enum{
	CGPS_SBAS_DEMOD_STATUS_INACTIVE = 0,
	CGPS_SBAS_DEMOD_STATUS_NO_BE_DET,
	CGPS_SBAS_DEMOD_STATUS_NO_PRE_DET,
	CGPS_SBAS_DEMOD_STATUS_NO_PEAK,
	CGPS_SBAS_DEMOD_STATUS_DPO_DET,
	CGPS_SBAS_DEMOD_STATUS_CRC_FAIL,
	CGPS_SBAS_DEMOD_STATUS_CRC_PASS,
	CGPS_SBAS_DEMOD_STATUS_FRAME_SKIPPED
}CGPS_SbasDemodStatus_type;

typedef struct
{
	unsigned char	SvPrn;
	unsigned char	DemodStatus;
	unsigned short	NumberOfSamples;
	unsigned short	BlankingStartIndex;
	unsigned short	BlankingEndDuration;
	unsigned char	CNo;
}CGPS_SbasDemodReportEvent_Struct;


typedef struct
{
	unsigned char u_RfMode;
	unsigned char u_RfLinearity;
	unsigned char u_RfRxdOnTimeSec;
	unsigned char u_RfRxdOffTimeSec;
    unsigned char u_RfAdcConfig;
}CGPS_Gen8_HwConfig_Struct_type;

typedef struct
{
	unsigned char	u_TestMode;
	unsigned char	u_GloHwChan;
	unsigned char	u_GpsOrSbasSvId;
	char 			b_GloFreqencyId;
	unsigned char	u_GpsSbasBitEdgeAction;
	unsigned char	u_GloBitEdgeAction;
    unsigned char	u_Command;
}CGPS_Gen8_SvTrack_Struct_type;


typedef struct
{
	unsigned char	TEST_CONTROL;
	unsigned short	FLAGS;
	unsigned short	PACKET_NUM;
	unsigned char	RF_STATUS_LOG_RATE_HZ;
	unsigned char	COLLECT_MODE;
	char			GLO_R1_CHAN;
	unsigned char	GLO_HW_CHAN;
	unsigned short	SAMPLE_CNT_1K;
	unsigned char	INTEGRATIONS;
	unsigned long	CENTER_FREQ_NBIQ;
} CGPS_Gen8_StartIqTest_struct_type;


typedef struct
{
	unsigned char	CMD_CONTROL_FLAG;
	unsigned long	SEQ_NUM;
	unsigned char	GPS_RxD_FLAG;
	unsigned char	GPS_CONTROL_FLAG;
	unsigned char	GPS_SV;
	long			GPS_DOPP_HZ;
	unsigned long	GPS_DOPP_WIN_HZ;
	unsigned long	GPS_CODE_PHASE_CENTER_CHIPX32;
	unsigned short	GPS_CODE_PHASE_WIN_CHIPX1;
	unsigned char	GPS_SRCH_MODE;
	unsigned short	GPS_NUM_POSTD;
	unsigned char	GPS_PFA;
	unsigned long	GPS_RESERVED; // Must be set to 0
	unsigned char	GLO_CONTROL_FLAG;
	char			GLO_FREQ_K;
	long			GLO_DOPP_HZ;
	unsigned long	GLO_DOPP_WIN_HZ;
	unsigned long	GLO_CODE_PHASE_CENTER_CHIPX32;
	unsigned short	GLO_CODE_PHASE_WIN_CHIPX1;
	unsigned char	GLO_SRCH_MODE;
	unsigned short	GLO_NUM_POSTD;
	unsigned char	GLO_HW_CHAN;
	unsigned char	GLO_PFA;
	unsigned long	GLO_RESERVED; // Must be set to 0
	unsigned short	DWELL_CNT;
	unsigned short	TOTAL_NUM_DWELLS;
	unsigned short	PASS_DWELL_CNT;
	unsigned long	RESERVED; // Must be set to 0
} CGPS_Gen8_PrescribedDwell_struct_type;




/******************************************************************************
						Diagnostic - ISDB-T subsytem
*******************************************************************************/

typedef enum
{
	_DIAG_ISDB_UBM_L1_ACQ_CMD					= 100,	//!< ' Acquire									*/
	_DIAG_ISDB_UBM_L1_RESET_CMD					= 101,	//!< ' Reset									*/
	_DIAG_ISDB_UBM_L1_ACQ_STATUS				= 102,	//!< ' Acquire Status?							*/
	_DIAG_ISDB_UBM_L1_POWERUP_CMD				= 103,	//!< ' Power Up									*/
	_DIAG_ISDB_UBM_L1_POWERDOWN_CMD				= 104,	//!< ' Power Down								*/
	_DIAG_ISDB_UBM_L1_ROUTE_Y1Y2_DEBUG_BUS		= 105,	//!< ' Route Y1Y2 Samples to the Debug Bus		*/
	_DIAG_ISDB_UBM_L1_RS1_PACKET_CNT_CMD		= 106,	//!< ' Collect Packet Counter Statistics		*/
	_DIAG_ISDB_UBM_L1_RECORD_TSIF_PACKETS_CMD	= 107,	//!< ' Record TSIF Packets						*/
	_DIAG_ISDB_UBM_L1_CONFIGURE_CIR_LOG_PARAMS	= 108,	//!< ' Configure No. of Frames per CIR Log		*/
	_DIAG_ISDB_UBM_L1_PRBS_ONE_BIT_CMD			= 109,	//!< ' Setup VBER one-bit error Test			*/
	_DIAG_ISDB_UBM_L1_PRBS_ONE_BIT_STATUS		= 110,	//!< ' 1 Bits Counted in PRBS one-bit Test		*/
	_DIAG_ISDB_UBM_L1_ENABLE_LNA_UPDATE			= 111,	//!< ' Enable LNA Update Loop					*/
	_DIAG_ISDB_UBM_L1_DISABLE_LNA_UPDATE		= 112,	//!< ' Disable LNA Update Loop					*/
	_DIAG_ISDB_UBM_L1_CFG_JAMMER_CMD			= 113,	//!< ' Change pre-FFT Jammer Parameters			*/
	_DIAG_ISDB_UBM_L1_RSSI_STATUS				= 114,	//!< ' Return Current RSSI Information			*/
	_DIAG_ISDB_UBM_L1_RECOVERY_CMD				= 115,	//!< ' Enter the Recovery State					*/
	_DIAG_ISDB_UBM_L1_CFG_RDSP_LOG_CMD			= 116,	//!< ' Configure RDSP Data Logging				*/
	_DIAG_ISDB_UBM_L1_CFG_POST_FFT_JAMMER_CMD	= 117,	//!< ' Configure Post-FFT Jammer Parameters		*/
	_DIAG_ISDB_UBM_L1_USE_RSSI_CAL_CMD			= 118,	//!< ' Enable/Disables RSSI calibration data	*/
	_DIAG_ISDB_UBM_L1_SET_LNA_CMD				= 119,	//!< ' Set the LNA State						*/
	_DIAG_ISDB_UBM_L1_SET_PERIODIC_LOG_ADDR		= 120,	//!< ' Set the Address for RDSP Log Items		*/
	_DIAG_ISDB_UBM_L1_ANT_TUNE_PDM_CONTROL_CMD	= 122,	//!< ' Control the Antenna Tune PDM Pin			*/
	_DIAG_ISDB_UBM_L1_RDSP_WRITE_CMD			= 123,	//!< ' Write specific RDSP registers			*/
	_DIAG_ISDB_UBM_L1_GET_TMCC_FRAME_INFO_CMD	= 124,	//!< ' Get the TMCC Information Field			*/
	_DIAG_ISDB_UBM_L1_CFG_ITEM					= 125	//!< ' Set/Read a given L1 Configuration Item	*/
} isdbt_diag_subsys_cmd_enum_type;

typedef enum
{
	ISDB_UBM_L1_CFG_EBI2_WS							=0,		//!< EBI2 waitstates
	ISDB_UBM_L1_CFG_USE_PLLS						=1,		//!< whether to use RUMI's PLLs or System clock
	ISDB_UBM_L1_CFG_CTRL_PID_EXT_MODE				=2,		//!< whether to use MPE bank memory for Control PID
															//!< 0 -> Don’t use bank memory
															//!< 1 -> Use bank memory
	ISDB_UBM_L1_CFG_CTRL_PID_EBI2_MODE				=3,		//!< whether to read control PIDs via EBI2
															//!< 0 -> use TSIF to read PIDs
															//!< 1 -> use EBI2 to read PIDs
	ISDB_UBM_L1_CFG_PAGE_MODE						=4,		//!< whether to use page mode for EBI2 comms to MBP.
															//!< 0 -> Don’t use page mode
															//!< 1 -> Use page mode
	ISDB_UBM_L1_CFG_USE_MODE2_CPCE					=5,		//!< whether to use channel estimation for mode 2
															//!< 0 -> do not use channel estimation
															//!< 1 -> use channel estimation
	ISDB_UBM_L1_CFG_DEBUG							=6,		//!< whether to turn on CFG debugging messages
															//!< 0 -> Turn off CFG messages
															//!< 1 -> Turn on CFG messages
	ISDB_UBM_L1_HANDLE_L3_ACK						=7,		//!< whether to handle acks intended for L3 by L1
															//!< 0 -> Handle L3 acks
															//!< 1 -> Do not handle L3 acks
	ISDB_UBM_L1_CFG_USE_ACQ_TIMEOUT					=8,		//!< whether to use the acquisition timeout
															//!< 0 -> Do not use acq timeout
															//!< 1 -> Use acq timeout
	ISDB_UBM_L1_CFG_SNOOZE							=9,		//!< whether to allow snoozing
															//!< 0 -> Disable snooze
															//!< 1 -> Enable snooze
	ISDB_UBM_L1_CFG_SLEEP							=10,	//!< whether to allow sleeping
															//!< 0 -> Disable sleep
															//!< 1 -> Enable sleep
	ISDB_UBM_L1_CFG_BW								=11,	//!< Set the channel bandwidth.
															//!< 0 -> UBM_L1_BW_5_MHZ
															//!< 1 -> UBM_L1_BW_6_MHZ
															//!< 2 -> UBM_L1_BW_7_MHZ
															//!< 3 -> UBM_L1_BW_8_MHZ
	ISDB_UBM_L1_RX_FRONT_WARMUP						=12,	//!< whether to do rx front warmup
															//!< 0 -> Don’t use RX front end warmup
															//!< 1 -> Do RX front end warmup
	ISDB_UBM_L1_CHANGE_TCXO_PDM						=13,	//!< Whether to do TCXO corrections
															//!< 0 -> do not change TCXO PDM
															//!< 1 -> change TCXO PDM
	ISDB_UBM_L1_MAP_ADDR_12_18						=14,	//!< Whether to map A12-18 on debug bus  no matter whether page mode is used
															//!< 0 -> do not map A12-18
															//!< 1 -> do map A12 -18
	ISDB_UBM_L1_FAP_LAP_CORR_MASK					=15,	//!< Bit mask to enable/disable FAP/LAP correction for particular mode/guard combinations.
															//!< To enable FAP/LAP correction for mode i, guard j, set bit number (i-1)*4+j to 1.
															//!< Otherwise set it to 0. Default is 0x100, for mode 3 guard 0 only (8k, 1/4 guard).
	ISDB_UBM_L1_SIGNAL_MONITOR						=16,	//!< Signal Monitor.  Currently not used.  Default value is 0.
	ISDB_UBM_L1_CFG_INPUT_MODE						=17,	//!< Input mode type.
															//!< 0 -> RF mode
															//!< 1 -> IQ mode
															//!< 2 -> Y1Y2 mode
	ISDB_UBM_L1_CFG_RDSP_CLK_SRC					=18,		//!< RDSP clock source
															//!< 0 -> Internal clock source (by PLL).
															//!< 1 -> External clock source.
	ISDB_UBM_L1_CFG_T_SCAN_LOCKOUT_MS				=19,	//!< Scan lockout time in ms.
	ISDB_UBM_L1_CFG_T_SCAN_NO_AGC_MS				=20,	//!< Time between scans for a signal that does not cross ISDB_UBM_L1_CFG_SCAN_NO_SIGNAL_THRESH_DB_Q6
	ISDB_UBM_L1_CFG_SCAN_NO_SIGNAL_THRESH_DB_Q6		=21,	//!< Threshold to determine whether there is a signal
	ISDB_UBM_L1_CFG_T_SCAN_AGC_MS					=22,	//!< Time between successive AGC scans
	ISDB_UBM_L1_CFG_T_SCAN_TPS_MS					=23,	//!< Time between successive TPS scans
	ISDB_UBM_L1_CFG_SCAN_ENABLED_THRESH_DB_Q6		=24,	//!< Threshold in dB Q6 where to start scanning
	ISDB_UBM_L1_CFG_HANDOFF_THRESH_DB_Q6			=25,	//!< Threshold in dB Q6 to handoff
	ISDB_UBM_L1_CFG_ACQ_TIMEOUT_MS					=26,	//!< Time for acquisition timeout in ms
	ISDB_UBM_L1_CFG_SIGNAL_LOST_THRESH_Q7			=27,	//!< Signal lost threshold
	ISDB_UBM_L1_CFG_LOST_DROP_TIME_MS				=28,	//!< Signal lost drop timer in ms
	ISDB_UBM_L1_CFG_SIGNAL_RECOVERED_SAMPLE_TIME_MS	=29,	//!< Signal lost sampling time coming out of recovery mode
	ISDB_UBM_L1_CFG_RECOVERY_RETRIES				=30,	//!< Number of recovery attempts
	ISDB_UBM_L1_CFG_TOTAL_RECOVERY_CNT				=31,	//!< Number of total recovery sessions
	ISDB_UBM_L1_CFG_ACQ_BAILOUT						=32,	//!< Whether acquisition bailout is enabled
	ISDB_UBM_L1_CFG_PRE_JAMMER_CFG					=33,	//!< Pre-FFT Jammer Configuration (ubm_l1_cfg_pre_jammer_config_type)
	ISDB_UBM_L1_CFG_JAMMER_NOTCH_GAIN				=34,	//!< Jammer Notch Gain
	ISDB_UBM_L1_CFG_POST_JAMMER_CFG					=35,	//!< Post-FFT Jammer Configuration (ubm_l1_cfg_post_jammer_config_type)
	ISDB_UBM_L1_CFG_CPCE_ENABLED					=36,	//!< Whether CPCE is enabled
	ISDB_UBM_L1_CFG_EARLY_TERM_EN					=37,	//!< Whether early termination is enabled
	ISDB_UBM_L1_CFG_FAP_LAP_SENS					=38,	//!< Threshold value for FAP/LAP sensitivity
	ISDB_UBM_L1_CFG_MAX_CAND_PATHS					=39,	//!< Maximum number of candidate paths
	ISDB_UBM_L1_CFG_SIGNAL_RECOVERED_THRESH_Q7		=40,	//!< Threshold at which the recovery count is reset
	ISDB_UBM_L1_CFG_DISBALE_CPCE_IIR				=41,	//!< Whether channel estimation IIR should be disabled
	ISDB_UBM_L1_CFG_TT_CIR_THRESH					=42,	//!< Time tracking CIR threshold
	ISDB_UBM_L1_CFG_DISABLE_FINE_TT					=43,	//!< Whether to disable fine time tracking
	ISDB_UBM_L1_CFG_FULL_SLEEP_REACQ				=44,	//!< Whether to run full reacquisition
	ISDB_UBM_L1_CFG_SLOW_TIME_TRACK					=45,	//!< Slow time tracking
	ISDB_UBM_L1_CFG_ENABLE_PAL_JAMMER				=46,	//!< Whether to manually setup a PAL jammer notch
	ISDB_UBM_L1_CFG_JAMMER_DETECT_THRESH			=47,	//!< Jammer detection threshold
	ISDB_UBM_L1_CFG_CPCE_LNA_THRESH					=48,	//!< At and above LNA state where CPCE is enabled
	ISDB_UBM_L1_CFG_SLOW_FREQ_TRACKING				=49,	//!< Whether to use slow frequency tracking
	ISDB_UBM_L1_CFG_CE_IRR_FILT_GAIN				=50,	//!< Channel estimate IIR filter gain
	ISDB_UBM_L1_CFG_CPCE_CHECK_SYM_COUNT			=51,	//!< Number of symbols for summing CPCE candidate paths. 0 for disable summing.
	ISDB_UBM_L1_CFG_CPCE_CAND_PATH_SUM_THRESH		=52,	//!< Path sum threshold for disabling
	ISDB_UBM_L1_CFG_ADD_RAMPUP_SYMBOLS				=53		//!< Additional symbols to add to rampup time
} ubm_isdb_configuration_items;

typedef enum
{
	ISDB_UBM_L1_MODE_2							= 2,
	ISDB_UBM_L1_MODE_3							= 3,
	ISDB_UBM_L1_MODE_ALL						= 255
} ubm_isdb_mode;

typedef enum
{
	ISDB_UBM_L1_GUARD_4							= 0,
	ISDB_UBM_L1_GUARD_8							= 1,
	ISDB_UBM_L1_GUARD_16						= 2,
	ISDB_UBM_L1_GUARD_ALL						= 255
} ubm_isdb_guard;

typedef enum
{
	ISDB_RDSP_LOG_CPCE							= 0,
	ISDB_RDSP_LOG_CPCE_UTILIZED_PATHLIST		= 1,
	ISDB_RDSP_LOG_CPCE_CANDIDATE_PATHLIST		= 2,
	ISDB_RDSP_LOG_CIR							= 3,
	ISDB_RDSP_LOG_ACQUISITION_LIKELIHOOD		= 4,
	ISDB_RDSP_LOG_JAMMER_FFT_OUTPUT				= 5,
	ISDB_RDSP_LOG_TDSP_PERIODIC_LOGGING			= 6,
	ISDB_RDSP_LOG_TMCC_INFO						= 7
} ubm_isdb_cfg_rdsp_log;

typedef enum
{
	ISDB_RDSP_WRITE_RX_FRONT_EE_INCR_THRESH		= 0,
	ISDB_RDSP_WRITE_RX_FRONT_EE_DECR_THRESH		= 1,
	ISDB_RDSP_WRITE_RX_FRONT_LNA_GAIN_STEPS		= 2,
	ISDB_RDSP_WRITE_RX_FRONT_GAIN_ADJUST		= 3
} ubm_isdb_rdsp_write_item;

/******************************************************************************
						Diagnostic - MBP RF subsytem
*******************************************************************************/

typedef enum
{
	_MBP_RF_SET_CFG_ITEM_CMD				= 12,	//!< ' Set Configuration Item
	_MBP_RF_SET_OPERATION_MODE_CMD			= 21	//!< ' Set Operation Mode
} mbp_diag_subsys_cmd_enum_type;

//! DVB-H/ISDB-T MBP Configuration Items
typedef enum
{
	MBP_RF_CFG_SPUR_CANCELLATION_ENABLED	= 13,	//"< ' Spur Cancellation Control (0=disable,1=enable)
	MBP_RF_CFG_ISDB_NB_JD_ENABLED			= 17,	//"< ' ISDB-T Narrowband Jammer Detection Control (0=disable,1=enable)
	MBP_RF_CFG_ISDB_WB_JD_ENABLED			= 18,	//"< ' ISDB-T Wideband Jammer Detection Control (0=disable,1=enable)
	MBP_RF_CFG_DVBH_NB_JD_ENABLED			= 19,	//"< ' DVB-H Narrowband Jammer Detection Control (0=disable,1=enable)
	MBP_RF_CFG_DVBH_WB_JD_ENABLED			= 20	//"< ' DVB-H Wideband Jammer Detection Control (0=disable,1=enable)
}mbprftypes_cfg_item_type;

//! DVB-H/ISDB-T Jammer Detection Modes
typedef enum
{
	MBP_RF_JD_MODE_AUTO = 0,
	MBP_RF_JD_MODE_1    = 1,
	MBP_RF_JD_MODE_2    = 2
}mbprftypes_jd_mode_type;

/******************************************************************************
						Diagnostic - Handset subsytem
*******************************************************************************/
/**
	Handset Diagnostic commands, from Services\hs\gsmdiag.h
*/
typedef enum
{
	_HS_DISPLAY_GET_PARAMS_CMD		= 0,	//!< ' Get display information (height, width, bpp)
	_HS_DISPLAY_CAPTURE_DELAYED_CMD	= 2		//!< ' Capture screen contents
} DiagHsCmd_Enum;

/******************************************************************************
						FTM - Command ID's
*******************************************************************************/

/*===========================================================================*/
/**
	\brief Enumeration to choose between the command formats.

	Currently, the two formats are using CMD_CODE = 59 and CMD_CODE = 75
	59 is used for MSM5xxx targets and 75 was introduced for MSM6xxx targets
*/
/*===========================================================================*/
typedef enum
{
	FTM_COMMAND_75 = 75,	//!<' Command code 75 is used for MSM6100 and later
	FTM_COMMAND_59 = 59,	//!<' Command code 59 is used for 5500 and 60xx
	FTM_COMMAND_TYPE_INVALID = -1	//!<' Invalid indicator
}  FTM_Command_Type;

/**
	FTM Commands enumeration.

	These are renamed with an _ prefix in order to avoid conflict with
	the function names.

*/
typedef enum
{
	_FTM_SET_PDM				= 0,	//!< Sets a PDM to a value
	_FTM_SET_TX_ON				= 2,	//!< Sets the transmit circuitry ON
	_FTM_SET_TX_OFF				= 3,	//!< Sets the transmit circuitry OFF
	_FTM_SET_MODE				= 7,	//!< Sets the phones operating mode
	_FTM_SET_CHAN				= 8,	//!< Sets the phones current channel
	_FTM_GET_SYNTH_STATE		= 16,	//!< Gets the current synthesizer lock state
	_FTM_FM_TX_ST				= 37,	//!< Sends signaling tone
	_FTM_CDMA_CW_WAVEFORM		= 50,	//!< Turns on/off I/Q spreading of CDMA reverse	link
	_FTM_GET_RX_AGC				= 53,	//!< Gets AGC value of Rx AGC loop
	_FTM_SET_PA_RANGE			= 54,	//!< Sets PA range
	_FTM_SET_LNA_RANGE			= 58,	//!< Sets LNA range
	_FTM_GET_ADC_VAL			= 59,	//!< Gets ADC value
	_FTM_SET_LNA_OFFSET			= 81,	//!< Sets LNA offset
	_FTM_GET_CDMA_IM2			= 114,	//!< Gets IM2 IQ values
	_FTM_SET_DVGA_OFFSET		= 111,	//!< Sets DVGA offset
	_FTM_SET_FREQUENCY_SENSE_GAIN = 115,//!< Sets Frequency Sense Gain
	_FTM_TX_SWEEP_CAL			= 116,	//!< FTM Tx Sweep Cal
	_FTM_GET_DVGA_OFFSET		= 117,	//!< Gets DVGA Offset
	_FTM_GET_LNA_OFFSET			= 118,	//!< Gets LNA Offset
	_FTM_GET_HDET_FROM_TX_SWEEP_CAL = 119, //!< FTM Get HDET from Tx Cal
	_FTM_GET_FM_IQ				= 120,	//!< Gets FM IQ
	_FTM_SET_SECONDARY_CHAIN	= 121,	//!< Set secondary Rx mode
	_FTM_GET_CTON				= 122,	//!< Get Rx Carrier / Noise ratio
	_FTM_SECOND_CHAIN_TEST_CALL	= 123,	//!< Set the call test mode for the second Rx
	_FTM_SET_HDET_TRACKING		= 124,	//!< Track Tx power to a specified HDET value
	_FTM_CONFIGURE_TX_SWEEP_CAL = 125,	//!< Configure the Tx Sweep command parameters
	_FTM_GET_CDMA_IM2_WITH_SUPPRESSION = 126, //!<Get CDMA IM2 with suppression
	_FTM_GET_ALL_HDET_FROM_TX_SWEEP_CAL=127,	//!< Return all of the HDET values measured in the last TX SWEEP
	_FTM_SET_RF_POWER_MODE		= 128,	//!<' Set RF mode to High,Mid or Low power mode
	_FTM_SET_TX_SWEEP_STEP_SIZE = 129,	//!<' Configure CDMA/WCDMA Tx AGC PDM Step size for Tx sweep
	_FTM_TX_RX_FREQ_CAL_SWEEP	= 130,	//!<' CDMA Tx Rx Frequency measurement
	_FTM_LOAD_RF_NV				= 131,  //!<' load RF NV from NV
	_FTM_DO_INTERNAL_DEVICE_CAL	= 132,	//!<' Perform internal device cal
	_FTM_DO_DC_CAL				= 133,	//!<' Initialize the DC accumulators.
	_FTM_DO_XO_DC_CAL		    = 134,	//!<! Coarse DC frequency calibration for XO targets
	_FTM_DO_XO_FT_CURVE_CAL     = 135,	//!<! Fine DC frequency and FT Curve Calibration for XO targets
	_FTM_SET_TX_WAVEFORM		= 136,	//!< Generates the specified waveform with given attributes.
	_FTM_SET_CALIBRATION_STATE  = 137,  //!<! Set the calibration state to active or inactive
	_FTM_GET_THERM = 138, //!<! Read back thermistor value
	_FTM_GET_AGC_RESOLUTION_TX	= 139,   //!<! Get Tx AGC Resolution Information
	_FTM_GET_AGC_RESOLUTION_RX	= 140,   //!<! Get Rx AGC Resolution Information
	_FTM_SELECT_RX_ANTENNA = 141,		//!<' Select the antenna source for 2nd receiver for FTS
	_FTM_GET_ENH_THERM = 142,//!<! Read back 32bit thermistor value
	_FTM_DO_IQ_MISMATCH_CAL = 143, //!<! Receiver IQ mismatch calibration
	_FTM_GET_ALL_HDET_FROM_TX_SWEEP_CAL_V2=144,	//!< Return all of the HDET values measured in the last TX SWEEP
	_FTM_DO_ICI_CAL = 145, //!<! Receiver ICI calibration
	_FTM_SET_BLOCK_MASK = 153, //!< Sets the block mask and NV Block table for the RF Mode set in the SET_MODE command
	_FTM_DO_ENH_XO_DC_CAL = 154,	//!<' Coarse DC frequency calibration for ENH XO targets
	_FTM_DO_ENH_XO_FT_CURVE_CAL = 155,	//!<' Fine DC frequency and FT Curve Calibration for ENH XO targets
	_FTM_SET_PA_PROFILE			= 212,
	_FTM_CONF_MODULATE_DATA		= 215,
	_FTM_SET_TRANSMIT_CONT		= 216,
	_FTM_SET_TRANSMIT_BURST		= 217,
	_FTM_SET_RX_BURST			= 225,
	_FTM_SET_TX_POWER_LEVEL		= 227,
	_FTM_GET_RSSI				= 230,
	_FTM_SET_PA_START_DELTA		= 231,
	_FTM_SET_PA_STOP_DELTA		= 232,
	_FTM_SET_PA_DAC_INPUT		= 233,
	_FTM_SET_RX_CONTINUOUS		= 234,	//!< Set Rx continuous
	_FTM_SET_TX_CAL_SWEEP		= 235,	//!< Tx cal sweep

	// GSM Polar Calibration
	_FTM_DO_GSM_AUTOCAL			= 236,	//!< Do autocal
	_FTM_SET_PATH_DELAY			= 237,	//!< Set the path delay for AMAM/AMPM
	_FTM_SET_AMAM_TABLE_GAIN	= 238,	//!< Set the gain for the AMAM table
	_FTM_SET_AMAM_TABLE_DC		= 239,	//!< Set the DC offset for the AMAM table
	_FTM_SET_TX_FRAME_MATRIX	= 240,	//!< Set the TX frame matrix
	_FTM_ENABLE_POLAR_REF_CAL	= 241,	//!< Enable polar path reference calibration
	_FTM_SET_TWOTONE_FREQ_OFFSET= 242,	//!< Sets the frequency offset for the two tone transmit
	_FTM_DO_CALPATH_RSB			= 243,	//!< Measure calpath RSB
	_FTM_DO_RTR6250_RSB			= 244,	//!< Load VI firmware for RSB
	_FTM_SET_OPLL_BW			= 245,	//!< set Chargepump current registers
	_FTM_RX_GAIN_RANGE_CAL		= 246,	//!< GSM Rx Gain Range Calibration (multiple gain states in one command)
	_FTM_GSM_SET_LINEAR_RGI		= 247,  //!< GSM Set Linear RF Gain Index

	_FTM_BASEBAND_BW_CAL		= 250,	//!< Baseband Bandwidth calibration
	_FTM_TX_KV_CAL					= 251,  //!< Kvco Calibration
	_FTM_SET_GSM_LINEAR_PA_RANGE = 252, //!< GSM PA Range

	// EGPRS non-signaling Command ID's
	_FTM_EGPRS_BER_START_GPRS_IDLE_MODE_REQ = 256,	//!< EGPRS - Idle mode request
	_FTM_EGPRS_BER_ESTABLISH_SRB_LOOPBACK	= 257,	//!< EGPRS - establish SRB loopback
	_FTM_EGPRS_BER_CONFIGURE_DL_TBF			= 258,	//!< EGPRS - configure DL temporary block flow
	_FTM_EGPRS_BER_CONFIGURE_UL_TBF			= 259,	//!< EGPRS - configure UL temporary block flow
	_FTM_EGPRS_BER_RELEASE_ALL_TBF			= 260,	//!< EGPRS - release all TBF
	_FTM_EGPRS_BER_RELEASE_UL_TBF			= 261,	//!< EGPRS - release all UL TBF
   _FTM_EGPRS_BER_EXEC_VFS_REQ         = 264,   //!< EGPRS - execute VFS

	// WCDMA HSDPA command ID's
    _FTM_HSDPA_BLER_CMD_START_HS			= 256,	//!< HSDPA -
    _FTM_HSDPA_BLER_CMD_STOP_HS				= 257,	//!< HSDPA -
    _FTM_HSDPA_BLER_CMD_RECONFIG_HS			= 258,	//!< HSDPA -
    _FTM_HSDPA_BLER_CMD_CONFIG_HS_PDSCH		= 259,	//!< HSDPA -
    _FTM_HSDPA_BLER_CMD_CONFIG_HS_SCCH		= 260,	//!< HSDPA -
    _FTM_HSDPA_BLER_CMD_CONFIG_HS_DSCH		= 261,	//!< HSDPA -
    _FTM_HSDPA_BLER_CMD_CONFIG_HS_DPCCH		= 262,	//!< HSDPA -



	/* FTM GSM Extended Command IDs */
	_FTM_GSM_POLAR_TX_SWEEP_CAL = 300,		//!< FTM GSM TX Sweep calibration
	_FTM_TX_RSB_DC_SWEEP = 301,				//!< GSM Tx RSB DC Sweep calibration
	_FTM_GSM_TX_DETECTOR_CAL = 302,			//!< GSM Tx Detector Cal
	_FTM_GSM_TX_4X_FMOD_SWEEP = 303,		//!< GSM Sweep Tx 4xFMod calibration
	_FTM_TX_CS_SWEEP = 304,					//!< GSM Tx CS Sweep calibration

	_FTM_GSM_TX_ENVDC_CS_SWEEP = 308,		//!< GSM Envelope DC Carrier Suppression Sweep
	_FTM_TX_CFG2_AMAM_SWEEP = 309,			//!< EDGE Tx Predistortion Calibration
	_FTM_GSM_TX_GAIN_SWEEP = 310,			//!< GSM Tx DA Calibration
	_FTM_GSM_RX_FREQ_CAL_SWEEP	= 311,		//!<' GSM Rx Frequency measurement

	_FTM_SET_WCDMA_SECONDARY_CHAIN = 121	//!< Enable/disable secondary Rx


} FTM_Command_ID_Type;

/******************************************************************************
						FTM - System Modes
*******************************************************************************/

/**
	\brief Enumeration of FTM Mode ID's, to be used int the "mode_id" field of the
	       FTM header
*/
typedef enum
{
	FTM_MODE_ID_CDMA_1X		= 0,		//!<' RF CDMA 1X mode - RX0
	FTM_MODE_ID_WCDMA		= 1,		//!<' RF WCDMA mode
	FTM_MODE_ID_GSM			= 2,		//!<' RF GSM Mode
	FTM_MODE_ID_CDMA_1X_RX1	= 3,		//!<' RF CDMA 1X mode - RX1
	FTM_MODE_ID_BLUETOOTH	= 4,		//!<' Bluetooth
	FTM_MODE_ID_CDMA_1X_CALL= 7,		//!<' CALL CDMA 1X mode
	FTM_MODE_ID_HDR_C		= 8,		//!<' HDC non signaling
	FTM_MODE_ID_LOGGING		= 9,		//!<' FTM Logging
	FTM_MODE_ID_AGPS		= 10,		//!<' Async GPS
	FTM_MODE_ID_PMIC        = 11,		//!<' PMIC FTM Command
	FTM_MODE_GSM_BER		= 13,		//!<' GSM BER
	FTM_MODE_ID_AUDIO		= 14,		//!<' Audio FTM
	FTM_MODE_ID_CAMERA		= 15,		//!<' Camera
	FTM_MODE_WCDMA_BER		= 16,		//!<' WCDMA BER
	FTM_MODE_ID_GSM_EXTENDED_C = 17,	//!<' GSM Extended commands
	FTM_MODE_CDMA_API_V2	= 18,		//!<' CDMA RF Cal API v2
	FTM_MODE_ID_MF_C		= 19,		//!<' MediaFLO
	FTM_MODE_RF_COMMON		= 20,		//!<' RF Common
	FTM_MODE_WCDMA_RX1		= 21,		//!<' WCDMA diversity RX (RX1)
	FTM_MODE_WLAN			= 22,		//!<' WLAN FTM
	FTM_MODE_QFUSE			= 24,		//!<' QFUSE FTM
	FTM_MODE_ID_MF_NS		= 26,		//!<' MediaFLO NS FTM
	FTM_MODE_GPS			= 28,		//!<' GPS FTM
	FTM_MODE_ID_DVBH		=240,		//!<' DVB-H Diag
	FTM_MODE_ID_ISDBT		=240,		//!<' ISDB-T Diag
	FTM_MODE_ID_MBP			=241,		//!<' MBP RF Diag
	FTM_MODE_SE_BER			= 27,		//!<' FTM SE BER
	FTM_MODE_ID_PRODUCTION	= 0x8000,	//!<' Production FTM
	FTM_MODE_ID_LTM			= 0x8001	//!<' LTM
} FTM_Mode_Id_Enum;


/******************************************************************************
						FTM - RF Modes
*******************************************************************************/

/**
	\brief Enumeration of RF Mode ID's, from services\ftm\ftm.h
*/
typedef enum
{

	PHONE_MODE_FM=1,				//!<' FM
	PHONE_MODE_SLEEP=2,				//!<' Sleep Mode
	PHONE_MODE_GPS=3,				//!<' GPS
	PHONE_MODE_GPS_SINAD=4,			//!<' GPS SINAD
	PHONE_MODE_CDMA_800=5,			//!<' CDMA 800
	PHONE_MODE_CDMA_1900=6,			//!<' CDMA 1900
	PHONE_MODE_CDMA_1800=8,			//!<' CDMA 1800
	PHONE_MODE_J_CDMA=14,			//!<' JCDMA
	PHONE_MODE_CDMA_450=17,			//!<' CDMA 450
	PHONE_MODE_CDMA_IMT=19,			//!<' CDMA IMT
	PHONE_MODE_CDMA_1900_EXT=26,	//!<' Secndary CDMA 1900MHz Band, Band Class 14
	PHONE_MODE_CDMA_450_EXT=27,		//!<' CDMA BC 11 (450 Extension)
	PHONE_MODE_CDMA_800_SEC=33,		//!<' Secondary CDMA 800MHz Band, Band Class 10


	PHONE_MODE_WCDMA_IMT=9,			//!<' WCDMA IMT, Band I
	PHONE_MODE_GSM_900=10,			//!<' GSM 900
	PHONE_MODE_GSM_1800=11,			//!<' GSM 1800
	PHONE_MODE_GSM_1900=12,			//!<' GSM 1900
	PHONE_MODE_BLUETOOTH=13,		//!<' Bluetooth
	PHONE_MODE_WCDMA_1900A=15,		//!<' WCDMA 1900 A, Band II Add
	PHONE_MODE_WCDMA_1900B=16,		//!<' WCDMA 1900 B, Band II Gen
	PHONE_MODE_GSM_850=18,			//!<' GSM 850
	PHONE_MODE_WCDMA_800=22,		//!<' WCDMA 800, Band V Gen
	PHONE_MODE_WCDMA_800A=23,		//!<' WCDMA 800, Band V Add
	PHONE_MODE_WCDMA_1800=25,		//!<' WCDMA 1800, Band III
	PHONE_MODE_WCDMA_BC4=28,		//!<' WCDMA BC4-used for both Band IV Gen and Band IV Add
	PHONE_MODE_WCDMA_BC8=29,		//!<' WCDMA BC8, Band VIII

	PHONE_MODE_MF_700=30,			//!<' MediaFLO
	PHONE_MODE_WCDMA_BC9=31,		//!<' WCDMA BC9 (1750MHz & 1845MHz), Band IX
	PHONE_MODE_CDMA_BC15=32,		//!<' CDMA Band Class 15
	/*
		QMSL Developers: please modify:
			- QLib.h  ->  QLIB_FTM_SET_MODE()
			- Diag_FTM.cpp  ->  Diag_FTM::FTM_SET_MODE()
			- QLIBFTMPhone.cpp when this list is changed.
	*/
	PHONE_MODE_MAX=255				//!<' Maximum possible mode ID

} FTM_RF_Mode_Enum;

/*
	The following are obsolete RF modes, or unused for Factory:

	PHONE_MODE_FM=1,				//!<' AMPS
	PHONE_MODE_CDMA=0,				//!<' Obsolete
	PHONE_MODE_GPS_SINAD=4,			//!<' GPS SINAD (obsolete)
	PHONE_MODE_CDMA_800=5,			//!<' cdma2000 BC0
	PHONE_MODE_CDMA_1900=6,			//!<' cdma2000 BC1
	PHONE_MODE_HDR=7,				//!<' HDR Call Test
	PHONE_MODE_CDMA_1800=8,			//!<' cdma2000 1800MHz
	PHONE_MODE_J_CDMA=14,			//!<' cdma2000 JCDMA
	PHONE_MODE_CDMA_450=17,			//!<' cdma2000 BC4, 450MHz
	PHONE_MODE_CDMA_IMT=19,			//!<' cdma2000 BC6, IMT band
*/

/******************************************************************************
						FTM - RF Definitions
*******************************************************************************/

//! # of elements for the piHDETvalues parameter in QLIB_FTM_GET_ALL_HDET_FROM_TX_SWEEP_CAL
#define QLIB_FTM_GET_ALL_HDET_FROM_TX_SWEEP_CAL_ARRAY_SIZE 32


/******************************************************************************
						FTM - GSM RF
*******************************************************************************/

/**
	\brief Enumeration of GSM Tx data sources for QLIB_FTM_SET_TRANSMIT_CONT
*/
typedef enum
{
	FTM_GSM_TX_DATA_SOURCE_PSDRND,		//!<' Pseudorandom
	FTM_GSM_TX_DATA_SOURCE_TONE,		//!<' Single tone
	FTM_GSM_TX_DATA_SOURCE_BUFFER,		//!<' Buffer
	FTM_GSM_TX_DATA_SOURCE_TWOTONE		//!<' 2 tone
} FTM_GSM_TX_DataSources_Enum;

/**
	Maximum number of rows to be returned from the GSM Tx Polar Internal calibration log
*/
#define FTM_GSM_TX_POLAR_INTERNAL_AUTOCAL_MAX_DATA_ROWS 500


/**
	\brief Enumeration of LNA states to be used for the iBitMask parameter of QLIB_FTM_RX_GAIN_RANGE_CAL()
*/
typedef enum
{
	FTM_GSM_RX_GAIN_RANGE_0	= 0x01,	//!<' Gain range 0
	FTM_GSM_RX_GAIN_RANGE_1	= 0x02,	//!<' Gain range 1
	FTM_GSM_RX_GAIN_RANGE_2	= 0x03,	//!<' Gain range 2
	FTM_GSM_RX_GAIN_RANGE_3	= 0x04,	//!<' Gain range 3
	FTM_GSM_RX_GAIN_RANGE_4	= 0x05	//!<' Gain range 4

} FTM_GSM_RX_GainRange_Enum;

/**
	Number of return values for the aiRSSI_Result array used in QLIB_FTM_RX_GAIN_RANGE_CAL()
*/
#define FTM_GSM_RX_GAIN_RANGE_CAL_NUM_RETURN_VALUES 8



/******************************************************************************
						FTM - cdma2000 non-signaling log parser definitions
*******************************************************************************/

/**
	Sub packet of RDA2 -- RDA_CHANNELS_INFO_TYPE / INDIVIDUAL_CHANNELS
*/
typedef union
{
	struct
	{
		byte RPICH	: 1;
		byte FCH	: 1;
		byte DCCH	: 1;
		byte SCH0	: 1;
		byte SCCH	: 3;
		byte reserved	: 1;
	} individual_channels;
	byte mask;
} RDA_Channels_Info_Type;

/**
	Sub packet of RDA2 -- CHAN_RC_TYPE
*/
typedef struct
{
	word FCH_rc;
	word DCCH_rc;
	word SCH_rc;
	word reserved0;
	word reserved1;
}Chan_RC;

/**
	Sub packet of RDA2 -- FCH_RDA_SUB_RECORD_TYPE
*/
typedef struct
{
	dword reserved0;
	word  ser;
	byte  decision;		//!< Data rate, according to RDA_CDMA_TRAFFIC_DATA_RATE_TYPE
	byte  reserved1;
}FCH_RDA_Sub_Record_Type;

/**
	Sub packet of RDA2 -- DCCH_RDA_SUB_RECORD_TYPE
*/
typedef struct
{
	dword reserved0;
	dword reserved1;
	word  reserved2;
	word  reserved3;
	word  ser;
	byte decision;		//!< Data rate, according to RDA_CDMA_TRAFFIC_DATA_RATE_TYPE
	byte reserved4[3];
}DCCH_RDA_Sub_Record_Type;

/**
	Enumeration of "decision" types FCH_RDA_SUB_RECORD_TYPE
	available for CDMA system
*/
typedef enum
{
	RDA_DataRate_Erasure,
	RDA_DataRate_OneEighth,
	RDA_DataRate_OneQuarter,
	RDA_DataRate_OneHalf,
	RDA_DataRate_Full,
	RDA_DataRate_Max
} RDA_CDMA_TRAFFIC_DATA_RATE_TYPE;

/**
	Sub-FIELD OF RDA2 -- SCH_RDA_SUB_RECORD_2_TYPE
*/
typedef union
{
	struct
	{
		word	ltu_crc;			//!< ' LTU crc value
		word	ser;				//!< ' SER for the frame
		byte	status;				//!< ' frame CRC
		byte	reserved[3];
	} dev_sch_conv;
	struct
	{
		word	min_llr;			//!< ' Log Likelihood Ratio
		byte	num_iterations;		//!< ' Indicates the number of iterations run by the TD
		byte	crc_pass;			//!< ' 1=Good frame
		byte	reserved[4];
	} dev_sch_turbo;
} sch_code_type;
/* --This definition was removed December 14, 2005
typedef union
{
	struct
	{
		word	ltu_crc;
		byte	status;
		byte	ser;
	} dev_sch_conv;
	struct
	{
		word	min_llr;
		byte	num_iterations;
		byte	crc_pass;
	} dev_sch_turbo;
} sch_code_type;
*/

/**
	Sub packet of RDA2 -- SCH_RDA_SUB_RECORD_2_TYPE
*/
typedef	struct
{
	dword	reserved0;
	sch_code_type sch_code;
	word    reserved2;
	word    reserved3;
	word    rate;
	byte    decision;		//!< Data rate, according to RDA_CDMA_TRAFFIC_DATA_RATE_TYPE
	byte    codeType;		//!< (0) FTM_SCH_CODING_CONVOLUTIONAL,	(1) FTM_SCH_CODING_TURBO
} SCH_RDA_Sub_Record_Type;


/**
	Sub-FIELD OF RDA2 -- SCH_RDA_SUB_RECORD_2_TYPE
*/
typedef union
{
	struct
	{
		word	ltu_crc;
		word	ser;
		byte	status;
		byte	reserved[3];
	} dev_sch_conv;
	struct
	{
		word	min_llr;
		byte	num_iterations;
		byte	crc_pass;
		byte	reserved[4];
	} dev_sch_turbo;
} sch_code_2_type;

/**
	Sub packet of RDA2 -- SCH_RDA_SUB_RECORD_2_TYPE

	This structure is the only difference between the
	RDA and RDA2 log packet types
*/
typedef struct
{
	dword	 reserved0;
	sch_code_2_type sch_code;
	word	 reserved2;
	word	 reserved3;
	word	 rate;		//!< Max Data rate of SCH
	byte	 decision;	//!< Data rate, according to RDA_CDMA_TRAFFIC_DATA_RATE_TYPE
	byte	 codeType;	//!< (0) FTM_SCH_CODING_CONVOLUTIONAL,	(1) FTM_SCH_CODING_TURBO
}SCH_RDA_Sub_Record_2_Type;


/**
	Union for the RC field of RDA_SUB_RECORD_HEADER_TYPE
*/
typedef union
{
	struct
	{
		word 	fch_rc	: 3;
		word 	dcch_rc	: 3;
		word 	sch_rc :	3;
		word 	frame_offset : 7;
	} chan_rc;
	word mask;

} Chan_RC_Type;


/**
	Sub packet of RDA2 -- RDA_SUB_RECORD_HEADER_TYPE
*/
typedef struct
{
	Chan_RC_Type			rc;
	RDA_Channels_Info_Type	assigned_channels;

	byte     num_sub_records;
	byte	 reference_buf[1];
}RDA_Sub_Record_Header_Type;



/**
	Searcher log header (logs 0x119A - 0x11A3)
*/
typedef struct
{
	byte iVersion;
	byte iNumSubPackets;
	word iReserved;
	byte iSubPacketsData[1];

} SRCH_TNG_GeneralizedSearcherHeader;

/**
	Searcher sub packet data header (logs 0x119A - 0x11A3)
*/
typedef struct
{
	byte iSubPacketID;
	byte iSubPacketVersion;
	word iSubPacketSize;
	byte iSubPacketData[1];

} SRCH_TNG_GeneralizedSearcherSubPacketHeader;

/**
	Searcher status sub packet data (sub packet type #1)
*/
typedef struct
{
	byte iSearcherState;
	byte iReserved;
	word iForwardRC;

} SRCH_TNG_SearcherStatus;

/**
	RF sub packet data (sub packet type #2)
*/
typedef struct
{
	byte iRX0BandClass;
	byte iRX1BandClass;
	word iRX0CDMAChannel;
	word iRX1CDMAChannel;
	byte iRX0AGC;
	byte iRX1AGC;
	word iTxPower;
	byte iTxGainAdj;
	byte iTxPowerLimit;

} SRCH_TNG_RFStatus;

/**
	Idle Demod Stats sub packet data (sub packet type #18)
*/
typedef struct
{
	byte iChannel;    // 0-PCH, 1-BCCH, 2-FCCCH
	byte iReserved;
	word iFrameSEC;
	dword iFrameEnergy;

} SRCH_TNG_IdleDemodStats;


//! Definition of searcher TNG General Status
#define FTM_CDMA2000_SRC_TNG_STATUS		0x119C

//! Rate Determination Algorithm Frame Information (LOG_CODE 0x10D9)
#define FTM_CDMA2000_RDA2_FRAME_INFO		0x10D9

//! Rate Determination Algorithm Frame Information (LOG_CODE 0x10D9)
#define FTM_CDMA2000_RDA_FRAME_INFO		0x10C9

//! Definition of searcher TNG Demodulation Info (LOG_CODE 0x119D)
#define FTM_CDMA2000_SRC_TNG_DEMOD_INFO	0x119D

//! C2K paging channel symbol rate
#define CDMA2000_PAGING_CHANNEL_SYMBOL_RATE 19200

//! Stucture of SER_Sub_Data record used in Paging MER Message
typedef struct
{
	byte chain;
    word ser;
    byte bad_message; // This message would have been counted in the MER statistics as a failure
}SER_Sub_Data;

//! Max number of accumulated SER_Sub_Data Record
#define MAX_NUM_ACCUM_SER 20

//! Structure of Paging MER Message
typedef struct
{
     word   count; /* number of SER record in the ser array, could use uint8 here, but for alignment, use uint16*/
     byte	pch_rate;
     SER_Sub_Data records[MAX_NUM_ACCUM_SER];
}RXC_SER_Log;

//! Stucture of Paging MER Statistics
typedef struct
{
	long totalPagingSER;
	long totalNumSymbol;
}CDMA2000_Paging_MER_Stat;

//!Max number of RF subpackets
#define CDMA2000_DEMOD_RF_SUBPACKET_BUFFER_SIZE 1000

//! Structure to hold the demod status RF subpacket data
typedef struct
{
	long Rx0_Bandclass[CDMA2000_DEMOD_RF_SUBPACKET_BUFFER_SIZE];
	long Rx1_Bandclass[CDMA2000_DEMOD_RF_SUBPACKET_BUFFER_SIZE];
	double Rx0_AGC_dBm;
	double Rx0_AGC;
	double Rx1_AGC_dBm;
	double Rx1_AGC;
	double TxPower;
	double TxGainAdj;
	double TxPowerLimit;
	double TxPowerLimit_dBm;
	long iTotalRecords;
}CDMA2000_Demod_RF_Stat;


//! cdma2000 Transceiver Resource Manager (TRM) Log (LOG_CODE 0x12E8)
#define CDMA2000_TRANSCEIVER_RESOURCE_MANAGER_LOG 0x12E8

//! Structure of client state in TRM subpkt 0 to 9 and 11 header
typedef struct
{
	byte iLockSate;
	byte iResource;
	byte iReason;
	byte iPriority;
	byte iChain;
	byte iGranted;
	byte iGroup;
	byte iRetain;
}CDMA2000_TRM_Client_State;

//! Structure of cdma2000 Transceiver Resource Manager Log Header
typedef struct
{
	byte iTrmSubPktId;
	byte iVersion;
	byte iClientID;
	byte iClientLogID;
	CDMA2000_TRM_Client_State _oOldClientState;
	CDMA2000_TRM_Client_State _oNewClientState;
}CDMA2000_TRM_Log_Header;

#define CDMA2000_TRM_LOG_PACKET_BUFFER_SIZE 1000

/**
	Structure of FTM2 1X Primary AGC, FTM Log code = LOG_FTM2_LOG_1X_AGC = 0x4, See 80-V9151-1 (Factory Test Mode 1X Logging API Interface Control Document)
*/
typedef struct
{
	word ftm_log_id;
	short rx_agc;
	short tx_gain_ctl;
	short tx_gain_adj;
	short tx_open_loop;
	word tx_agc_adj;
	byte pa_ratchet;
	byte lna_state;
	byte pa_state;
	byte hdet_raw;
	word therm_raw;
	byte therm_scaled;
	byte temp_index;
	byte temp_remainder;
	byte intelliceiver_state;
} FTM_LOG_1X_AGC_Struct;

/**
	Structure of FTM 1 1X Secondary AGC, FTM Log Code = LOG_FTM2_LOG_1X_AGC_C1	= 0x6, See 80-V9151-1 (Factory Test Mode 1X Logging API Interface Control Document)
*/
typedef struct
{
	word ftm_log_id;
	short rx_agc;
	short tx_gain_ctl;
	short tx_gain_adj;
	short tx_open_loop;
	word tx_agc_adj;
	byte pa_ratchet;
	byte lna_state;
	byte pa_state;
	byte hdet_raw;
	word therm_raw;
	byte therm_scaled;
	byte temp_index;
	byte temp_remainder;
	short rx_agc_c1;
	byte lna_state_c1;
	byte intelliceiver_state;
} FTM_LOG_1X_AGC_C1_Struct;


/**
	Number of symbols per frame for FCH
*/
typedef enum
{
	FTM_CDMA2000_NS_NUM_SYMBOLS_RC1_FRAME = 384,
	FTM_CDMA2000_NS_NUM_SYMBOLS_RC2_FRAME = 384,
	FTM_CDMA2000_NS_NUM_SYMBOLS_RC3_FRAME = 768,
	FTM_CDMA2000_NS_NUM_SYMBOLS_RC4_FRAME = 384,
	FTM_CDMA2000_NS_NUM_SYMBOLS_RC5_FRAME = 768

}FTM_CDMA2000_NS_SymbolsPerFrame_Enum;

/**
	Number of symbols per frame for F-DCCH
*/
typedef enum
{
	FTM_CDMA2000_NS_DCCH_NUM_SYMBOLS_RC1_FRAME = 0,
	FTM_CDMA2000_NS_DCCH_NUM_SYMBOLS_RC2_FRAME = 0,
	FTM_CDMA2000_NS_DCCH_NUM_SYMBOLS_RC3_FRAME = 768,
	FTM_CDMA2000_NS_DCCH_NUM_SYMBOLS_RC4_FRAME = 384,
	FTM_CDMA2000_NS_DCCH_NUM_SYMBOLS_RC5_FRAME = 768

}FTM_CDMA2000_NS_DCCH_SymbolsPerFrame_Enum;

/**
	Number of symbols per frame for F-SCH
*/
typedef enum
{
	FTM_CDMA2000_NS_SCH_NUM_SYMBOLS_RC1_FRAME = 0,
	FTM_CDMA2000_NS_SCH_NUM_SYMBOLS_RC2_FRAME = 0,
	FTM_CDMA2000_NS_SCH_NUM_SYMBOLS_RC3_FRAME = 768,
	FTM_CDMA2000_NS_SCH_NUM_SYMBOLS_RC4_FRAME = 384,
	FTM_CDMA2000_NS_SCH_NUM_SYMBOLS_RC5_FRAME = 768

}FTM_CDMA2000_NS_SCH_SymbolsPerFrame_Enum;

/******************************************************************************
						FTM - cdma2000 non-signaling
*******************************************************************************/


/**
	\brief Enumeration of CDMA2000 non-signaling FTM Sub-command ID's
*/
typedef enum
{
	_FTM_CDMA2000_PILOT_ACQ		= 100,	//!<'	Acquire Pilot
	_FTM_CDMA2000_DEMOD_SYNC	= 101,	//!<'	Acquire Sync channel
	_FTM_CDMA2000_DEMOD_FCH		= 102,	//!<'	Assign forward fundamental channel and setup parameters
	_FTM_CDMA2000_DEMOD_FSCH	= 103,	//!<'	Assign forward supplemental channel
	_FTM_CDMA2000_MOD_FCH		= 104,	//!<'   Assign reverse fundamental channel
	_FTM_CDMA2000_MOD_SCH		= 105,	//!<'   Assign reverse supplemental channel
	_FTM_CDMA2000_FCH_LOOPBACK	= 106,	//!<'   Enable loopback on the fundamental channel
	_FTM_CDMA2000_SCH_LOOPBACK	= 107,  //!<'   Enable loopback on the supplemental channel
	_FTM_CDMA2000_CMD_RELEASE	= 108,	//!<'   Deassign all traffic channels
	_FTM_CDMA2000_SET_MODE		= 109,	//!<'   Set Mode
	_FTM_CDMA2000_DEMOD_DCCH	= 110,	//!<' Assign fotward dedicated control channel
	_FTM_CDMA2000_MOD_DCCH		= 111,  //!<' Assign reverse dedicated control channel
	_FTM_CDMA2000_DCCH_LOOPBACK = 112,	//!<' Enable loopback on the dedicated control channel
	_FTM_EVDO_PILOT_ACQ			= 113,	//!<'Acquire HDR forware pilot channel
	_FTM_EVDO_DEMOD_SYNC		= 114,	//!<'Acquire HDR system time
	_FTM_EVDO_DEMOD_CC_MAC_FTC	= 115,	//!<'Demodulate HDR CC/MAC/FTC
	_FTM_EVDO_MOD_ACC			= 116,	//!<'Modulate reverse access channel
	_FTM_EVDO_MOD_TRA			= 117,	//!<'Modulate reverse traffic channel
	_FTM_EVDO_CMD_RELEASE		= 118,	//!<'Deassign HDR channels
	_FTM_CDMA2000_RF_CMDS		= 119,	//!<' RF Subcommands (RF Calibration)
	_FTM_CDMA2000_DEMOD_FCH_DCCH = 120,
	//!<'Assign forward fundamental channel and dedicated control channel simultaneously (Release A)
	_FTM_CDMA2000_MOD_FCH_DCCH = 121,
	//!<'Assign reverse fundamental channel and dedicated control channel simultaneously (Release A)
	_FTM_EVDO_DEMOD_FWD_WITH_NO_REV = 122, //!<'Demodulate Forward Link with No Reverse Link Rev 0
	_FTM_EVDO_SET_IDLE				= 123, //!<'Put the searcher in Idle mode
	_FTM_EVDO_REV_A_CONF_MAC_FOR_FWD_CC_MAC_FTC = 124, //!<'configure the Rev A packet mode
	_FTM_EVDO_REV_A_MOD_ACC			= 125, //!<'Modulate the reverse access channel in IS-896 Rev A mode
	_FTM_EVDO_REV_A_MOD_TRA			= 126, //!<'Modulate the reverse traffic channel in IS-896 Rev A
	_FTM_EVDO_REV_A_DEMOD_FWD_WITH_NO_REV  = 127, //!<'Demodulate Forward Link with No Reverse Link Rev A
	_FTM_FWD_HHO_SC					= 128,	//!<'Handover for cdma2000
	_FTM_EVDO_REV_B_MOD_RTC 		= 131, //!<'Modulate the reverse traffic channel in IS-896 Rev B
	_FTM_EVDO_REV_B_CONFIG_RTC 		= 132, //!<'Config the reverse traffic channel in IS-896 Rev B
	_FTM_EVDO_CONFIG_SAME_CHANNEL	= 133, //!<'Config the same channel params defined in IS-896, including pilot drop, pilot add etc.
	_FTM_EVDO_REV_B_CONFIG_RTC_TX_PATTERN	= 134, //!<'Config the tx data pattern on reverse link carriers for Rev B
	
} FTM_CDMA2000_Cmd_Id_Enum;



/**
	\brief Enumeration of Calibration API V2 (80-V2376-1) FTM Sub-command ID's
*/
typedef enum
{
	_FTM_CDMA_CAL_V2_COMMIT_NV			= 0,			//!<'Commit RF calibration to NV
	_FTM_CDMA_CAL_V2_CAL_DVGA			= 1,			//!<'Calibrate DVGA
	_FTM_CDMA_CAL_V2_CAL_LNA			= 2,			//!<'Calibrate LNA
	_FTM_CDMA_CAL_V2_CAL_IM2			= 3,			//!<'Calibrate IM2
	_FTM_CDMA_CAL_V2_INTELLICEIVER		= 4,				//!<'Calibrate Intelliceiver
	_FTM_CDMA_CAL_V2_INTELLICEIVER_RC_TUNE =5			//!<'Intelliceiver RC Tune calibration

}	FTM_CDMA_CAL_V2_Cmd_Id_Enum;

/**
	cdma2000 Band Class identifiers for QLIB_FTM_CDMA2000_PILOT_ACQ()

	The enumeration values are the same as the band class number.  If an
	enumeration is not available for a new band class number then the
	an integer with the band class number can be used.

*/
typedef enum
{
	FTM_CDMA2000_BAND_BC0	= 0,
	FTM_CDMA2000_BAND_BC1	= 1,
	FTM_CDMA2000_BAND_BC2	= 2,
	FTM_CDMA2000_BAND_BC3	= 3,
	FTM_CDMA2000_BAND_BC4	= 4,
	FTM_CDMA2000_BAND_BC5	= 5,
	FTM_CDMA2000_BAND_BC6	= 6,
	FTM_CDMA2000_BAND_BC7	= 7,
	FTM_CDMA2000_BAND_BC8	= 8,
	FTM_CDMA2000_BAND_BC9	= 9,
	FTM_CDMA2000_BAND_BC10	= 10,
	FTM_CDMA2000_BAND_BC11	= 11,
	FTM_CDMA2000_BAND_BC12	= 12,
	FTM_CDMA2000_BAND_BC13	= 13,
	FTM_CDMA2000_BAND_BC14	= 14,
	FTM_CDMA2000_BAND_BC15	= 15,
	FTM_CDMA2000_BAND_BC16	= 16,
	FTM_CDMA2000_BAND_BC17	= 17


} FTM_CDMA2000_BandClasstype;

/**
	1x dynamic range types

	This is a list of dynamic range configurations used for 1x non-signaling,
  to convert results from AGC units to dB and dBm
*/
typedef enum
{
	FTM_CDMA2000_NS_DR_85_3,		//!<' 85.3dB dynamic range, min_rssi = -106.0dBm
    FTM_CDMA2000_NS_DR_102_4		//!<' 102.4dB dynamic range, min_rssi = -115.0dBm
} FTM_CDMA2000_NS_DR_Types_Enum;


/**
	1x acquisition modes
*/
typedef enum
{
	FTM_CDMA2000_NS_FTM_ACQ_MODE_FULL	= 0,	//!<' Full
	FTM_CDMA2000_NS_FTM_ACQ_MODE_MINI	= 1,	//!<' Mini, not supported, not to be used
	FTM_CDMA2000_NS_FTM_ACQ_MODE_MICRO	= 2		//!<' Micro, not supported, not to be used
} FTM_CDMA2000_NS_PilotAcqMode_Enum;

/**
	1x REV FCH rates
*/
typedef enum
{
	FTM_CDMA2000_NS_REV_FCH_FULL_RATE	= 0,	//!<' Full rate
	FTM_CDMA2000_NS_REV_FCH_HALF_RATE	= 1,	//!<' half rate
	FTM_CDMA2000_NS_REV_FCH_QTR_RATE	= 2,	//!<' quarter rate
	FTM_CDMA2000_NS_REV_FCH_8TH_RATE	= 3		//!<' eighth rate
} FTM_CDMA2000_NS_REV_FCH_Rates_Enum;


/**
	1x FCH rate SETS
*/
typedef enum
{
	FTM_CDMA2000_NS_RC_1	= 1,	//!<' RC1
	FTM_CDMA2000_NS_RC_2	= 2,	//!<' RC2
	FTM_CDMA2000_NS_RC_3	= 3,	//!<' RC3
	FTM_CDMA2000_NS_RC_4	= 4,	//!<' RC4
	FTM_CDMA2000_NS_RC_5	= 5		//!<' RC5
} FTM_CDMA2000_NS_FWD_RC_Enum;



/**
	CDMA2000 SCH Coding types
*/
typedef enum
{
	FTM_SCH_CODING_CONVOLUTIONAL,
	FTM_SCH_CODING_TURBO
} FTM_SCH_CODING_Enum;


/**
	CDMA2000 SCH reverse link data rates
*/
typedef enum
{
	FTM_REV_SCH_1_5_RATE,	//!<' (0 = 1.5 kbps),
	FTM_REV_SCH_2_7_RATE,	//!<'  (1 = 2.7 kbps),
	FTM_REV_SCH_4_8_RATE,	//!<'  (2 = 4.8 kbps),
	FTM_REV_SCH_9_6_RATE,	//!<'  (3 = 9.6 kbps),
	FTM_REV_SCH_19_2_RATE,	//!<' (4 = 19.2 kbps),
	FTM_REV_SCH_38_4_RATE,	//!<'  (5 = 38.4 kbps),
	FTM_REV_SCH_76_8_RATE,	//!<'  (6 = 76.8kbps), and
	FTM_REV_SCH_153_6_RATE	//!<'  (7 = 153.6kbps) respectively.

	// Note that for turbo-encoded frames, the minimum supported rate is FTM_REV_SCH_19_2_RATE.

} FTM_SCH_DataRates_Enum;


/**
	CDMA2000 SCH foward link data rates
*/
typedef enum
{
	FTM_RATE_1X,	//!<' (9.6 kbps)
	FTM_RATE_2X,	//!<' (19.2 kbps)
	FTM_RATE_4X,	//!<' (38.4 kbps)
	FTM_RATE_8X,	//!<' (76.8 kbps)
	FTM_RATE_16X	//!<' (153.6 kbps)
} ftm_spread_rate_type;


/**
	CDMA2000 Non-signaling return status
*/
typedef enum
{
	_FTM_CDMA2000_CMD_SUCCESS		= 0,				//!<'Indicates Success
	_FTM_CDMA2000_INVALID_CMD		= 1,				//!<'Indicates an invalid cmd
	_FTM_CDMA2000_INVALID_STATE		= 2,				//!<'Indicates that FTM cannot process the command in current state
	_FTM_CDMA2000_NO_SUCCESS		= 3					//!<'Indicates failure
} FTM_CDMA2000_CMD_STATUS;

/**
	CDMA2000 Non-signaling Log types

	This is a list of the types of log events that will be monitored during CDMA2000 non-signaling
*/
typedef enum
{

	FTM_CDMA2000_NS_Log_Searcher,				//!<' CDMA2000 Searcher (LOG_CODE 0x119C)
	FTM_CDMA2000_NS_Log_AGC_C0,					//!<' FTM2 log, sub log 5, for Primary AGC
	FTM_CDMA2000_NS_Log_AGC_C1,					//!<' FTM2 log, sub log 7, for Diversity AGC
	FTM_CDMA2000_NS_Log_RDA2,					//!<' CDMA2000 (LOG_CODE 0x10D9), used for SER
	FTM_CDMA2000_NS_Log_RDA,					//!<' CDMA2000 (LOG_CODE 0x10C9), used for SER

	FTM_CDMA2000_NS_Log_MAX						//!<' Maximum # of logs that are being scanned for

}FTM_CDMA2000_NS_Log_Types_Enum;

/**
	CDMA2000 Non-signaling Event types

	This is a list of the types of event types that will be monitored during CDMA2000 non-signaling.

	Generally an event is based on reveiving one asynchronous log message + checking the state
	of one log sub fields.

*/
typedef enum
{
	FTM_CDMA2000_NS_Event_PilotAcq,				//!<' Searcher State = 0x3 (Acquisition of the pilot channel)
	FTM_CDMA2000_NS_Event_RecSync,				//!<' Searcher State = 0x4 (Reception of the sync channel)
	FTM_CDMA2000_NS_Event_Traffic,				//!<' Searcher State = 0x8 (Operation on the traffic channel)

	FTM_CDMA2000_NS_Event_RDA_Update,			//!<' Rate determination algorithm report #1
	FTM_CDMA2000_NS_Event_RDA2_Update,			//!<' Rate determination algorithm report #2
	FTM_CDMA2000_NS_Event_PrimaryAGC,			//!<' Primary AGC
	FTM_CDMA2000_NS_Event_SecondaryAGC,			//!<' Secondary AGC

	FTM_CDMA2000_NS_Event_MAX					//!<' Maximum # of events

} FTM_CDMA2000_NS_Event_Types_Enum;


/**
	Searcher states, as used in "Searcher Status subpacket" in Srch TNG General Status Log (0x119C)
*/
typedef enum
{
	FTM_CDMA2000_Searcher_RawInit			= 0x00,	//!<'  Raw initialization state
	FTM_CDMA2000_Searcher_DeepSleep			= 0x01,	//!<'  Deep sleep in start state
	FTM_CDMA2000_Searcher_InitStateCDMA		= 0x02,	//!<'  Initial state for CDMA operation
	FTM_CDMA2000_Searcher_PilotAcq			= 0x03,	//!<'  Acquisition of the pilot channel
	FTM_CDMA2000_Searcher_RecSync			= 0x04,	//!<'  Reception of the sync channel
	FTM_CDMA2000_Searcher_SyncToPage		= 0x05,	//!<'  Transition from sync to paging channel
	FTM_CDMA2000_Searcher_Paging			= 0x06,	//!<'  Operation on the paging channel
	FTM_CDMA2000_Searcher_SlottedSleep		= 0x07,	//!<'  Slotted mode sleep state
	FTM_CDMA2000_Searcher_Traffic			= 0x08,	//!<'  Operation on the traffic channel
	FTM_CDMA2000_Searcher_RetToSync			= 0x09,	//!<'  Return from paging or traffic to sync channel (unslew)
	FTM_CDMA2000_Searcher_PCG				= 0x0a,	//!<'  Operation in PCG state
	FTM_CDMA2000_Searcher_PowerUp			= 0x0b	//!<'  Powerup state

} FTM_CDMA2000_SearcherStates_Enum;



/**
	Statistics for 1X AGC, structure definition is shared between Primary and secondary antennas,
	but separate instances will be created for both

*/
typedef struct
{
	dword iTotalRecords;	//!< Total number of AGC records that have been received

	//unsigned double ftm_log_id;
	double dRxAGC;				//!< AGC units
	double dRxAGC_linear;		//!< Linear power (mW)
	double dRxAGC_dBm;			//!< Log power (dBm)

	double dTxGainCtl;			//!< AGC units
	double dTxGainCtl_linear;	//!< Linear power (mW)
	double dTxGainCtl_dBm;		//!< Log power (dBm)

	double dTxGainAdj;			//!< AGC units
	double dTxGainAdj_dB;		//!< Log power (dB)

	double dTxOpenLoop;			//!< AGC units
	double dTxOpenLoop_linear;	//!< Linear power (mW)
	double dTxOpenLoop_dBm;		//!< Log power (dBm)

	double dTxAgcAdj;
	double dPA_Ratchet;
	double dLNA_State;
	double dPA_State;
	double dHDET_Raw;
	double dThermRaw;
	double dThermScaled;
	double dTempIndex;
	double dTempRemainder;
	double dIntelliceiverState;
} FTM_LOG_1X_AGC_Statistics;


/**
	SER statistics structure to be used for cdma2000 non-signaling.
*/
typedef struct
{
	long iSumFrames;		//!< Number of frames counted
	long iSumSymbols;		//!< Number of symbols counted

	long iSumFramesErrors;	//!< Number of frames errors counted
	long iSumSymbolErrors;	//!< Number of symbols errors counted

	double dFER;			//!< Calculated FER
	double dSER;			//!< Calculated SER
} FTM_CDMA2000_SER_Statistics;

/**
	Structure to be used for cdma2000 non-signaling status.

	Contains searcher status, AGC statistics, and SER/FER statistics.
*/
typedef struct
{
	//! Array to keep track of whether a certain event has occured, indexed to FTM_CDMA2000_NS_Event_Types_Enum
	byte bHasUpdated[ FTM_CDMA2000_NS_Event_MAX ];

	/**
		Searcher information
	*/
	FTM_CDMA2000_SearcherStates_Enum eSearcherState;

	//FTM_LOG_EVDO_CtoI_Statistics_Struct oEVDO_CtoI_Stats;

	/**
		Data for primary AGC
	*/
	FTM_LOG_1X_AGC_Struct oCDMA2000_AGC_C0;					//!< Most recent Primary AGC information
	FTM_LOG_1X_AGC_Statistics oCDMA2000_AGC_C0_LatestStats;	//!< Most recent Statistics for Primary AGC, transformed from oCDMA2000_AGC_C0
	FTM_LOG_1X_AGC_Statistics oCDMA2000_AGC_C0_SumStats;	//!< Summed Statistics for Primary AGC
	FTM_LOG_1X_AGC_Statistics oCDMA2000_AGC_C0_AvgStats;	//!< Averageed statistics for Secondary AGC

	/**
		Data for secondary AGC
	*/
	FTM_LOG_1X_AGC_C1_Struct oCDMA2000_AGC_C1;			//!< Most recent Secondary AGC information
	FTM_LOG_1X_AGC_Statistics oCDMA2000_AGC_C1_LatestStats;	//!< Most recent Statistics for Secondary AGC, transformed from oCDMA2000_AGC_C1
	FTM_LOG_1X_AGC_Statistics oCDMA2000_AGC_C1_SumStats;	//!< Summed for Secondary AGC
	FTM_LOG_1X_AGC_Statistics oCDMA2000_AGC_C1_AvgStats;	//!< Averageed statistics for Secondary AGC

	/**
		FCH SER statistics
	*/
	FTM_CDMA2000_SER_Statistics oCDMA2000_FCH_SER_Stats;

	/**
		FCH SER statistics per data rate

		Indexed by the enumeration: RDA_CDMA_TRAFFIC_DATA_RATE_TYPE;

	*/
	FTM_CDMA2000_SER_Statistics oCDMA2000_FCH_SER_Stats_perRate[ RDA_DataRate_Max ];

	/**
		DCCH SER statistics
	*/
	FTM_CDMA2000_SER_Statistics oCDMA2000_DCCH_SER_Stats;

	/**
		SCH SER statistics
	*/
	FTM_CDMA2000_SER_Statistics oCDMA2000_SCH_SER_Stats;

}FTM_CDMA2000_NS_Status_Struct;


/******************************************************************************
						FTM - EVDO non-signaling log parser definitions
*******************************************************************************/

//! Definition of log code for 1xEV-DO Finger Info
#define FTM_EVDO_NS_LOG_CODE_FINGER_INFO		0x108A

//! Definition of log code for 1xEV-DO Air Link Summary
#define FTM_EVDO_NS_LOG_CODE_AIR_LINK_SUMMARY	0x1068

//! Definition of log code for 1xEVDO Foward link statistics summary
#define EVDO_FORWARD_LINK_STATISTICS_SUMMARY	0x1084

//! Definition of log code for 1xEVDO Power Logs
#define EVDO_POWER	0x1069

//! Definition of log code for 1xEVDO Rev. A Single-user forward statistics summary
#define EVDO_REVA_SU_FWDSTATS	0x1192

//! Definition of log code for 1xEVDO Rev. B Multi-carrier single-user forward statistics summary
#define EVDO_MC_SU_FWD_STATS	0x12A2

//! Definition of log code for 1xEVDO Rev. B Multi-carrier air link summary
#define EVDO_MC_AIR_LINK_SUMMARY 0x1296

//! Definition of log code for 1xEVDO Rev. B Multi-carrier power
#define EVDO_MC_POWER 0x129C

//! Definition of maximum num of carriers for 1xEVDO Rev. B Multi-carrier
#define EVDO_MC_MAX_NUM_CARRIERS 3

/**
	Structure of FTM2 EVDO Primary AGC, FTM Log code = LOG_FTM2_LOG_HDR_AGC		= 0x5
*/
typedef struct
{
	word ftm_log_id;
	short rx_agc;
	short tx_gain_ctl;
	short tx_gain_adj;
	short tx_open_loop;
	word tx_agc_adj;
	byte pa_ratchet;
	byte lna_state;
	byte pa_state;
	byte hdet_raw;
	word therm_raw;
	byte therm_scaled;
	byte temp_index;
	byte temp_remainder;
	byte intelliceiver_state;
} FTM_LOG_EVDO_AGC_C0_Struct;


/**
	Structure of FTM2 EVDO Secondary AGC, FTM Log Code = LOG_FTM2_LOG_HDR_AGC_C1 = 0x7
*/
typedef struct
{
	word ftm_log_id;
	short rx_agc;
	short tx_gain_ctl;
	short tx_gain_adj;
	short tx_open_loop;
	word tx_agc_adj;
	byte pa_ratchet;
	byte lna_state;
	byte pa_state;
	byte hdet_raw;
	word therm_raw;
	byte therm_scaled;
	byte temp_index;
	byte temp_remainder;
	short rx_agc_c1;
	byte lna_state_c1;
	byte intelliceiver_state;
} FTM_LOG_EVDO_AGC_C1_Struct;

/**
	Structure of the per-finger data used in log code 0x108A
*/
typedef struct
{
	word PilotPN;			//!<'
	dword  RTCOffset;		//!<'
	word C2I;				//!<'
	byte locked_antenna_diversity_fingindex;	//!<'
	byte RPCCellIdx_ASPIdx;						//!<'
	word Antenna0C2I;				//!<' C/I equivalent of the finger for antenna 0; only valid when
									//!<' Diversity is enabled
	word Antenna1C2I;				//!<' C/I equivalent of the finger for antenna 1; only valid when
									//!<' Diversity is enabled
} LOG_1XEVDO_Finger_Ver2_FingerInfo_Struct;

#define LOG_1XEVDO_MAX_FINGERS 12
/**
	Structure of the header information for log code 0x108A,
	at the end is the data for up to 12 fingers
*/
typedef struct
{
	byte srch_state;				//!<' State of the HDRSRCH state machine; refer to for nomenclature
	dword MSTR;						//!<' Mobile station (AT) time reference offset relative to the RTC
									//!<' timebase in units of 1/8 chip

	word MSTRError;					//!<' MSTR offset relative to earliest arriving in-lock and
									//!<' enabled active set pilot in units of 1/8 chip

	word MSTRPilotPN;				//!<' Pilot PN of the finger, which the MSTR is tracking;
									//!<' typically, the earliest arriving finger

	byte NumFingers;		//!<' Number of fingers (12 max)
	LOG_1XEVDO_Finger_Ver2_FingerInfo_Struct fingerData[ LOG_1XEVDO_MAX_FINGERS ];	//!<' Finger data

} LOG_1XEVDO_Finger_Ver2_Info_Struct;


/** Structure of the header information for log code 0x1084
	1xEV-DO Forward Statistics Summery
*/
typedef struct
{
	dword	cc38400_good;
	dword	cc38400_bad;
	dword	cc76800_good;
	dword	cc76800_bad;
	dword	tc38400_good;
	dword	tc38400_bad;
	dword	tc76800_good;
	dword	tc76800_bad;
	dword	tc153600_good;
	dword	tc153600_bad;
	dword	tc307200short_good;
	dword	tc307200short_bad;
	dword	tc307200long_good;
	dword	tc307200long_bad;
	dword	tc614400short_good;
	dword	tc614400short_bad;
	dword	tc614400long_good;
	dword	tc614400long_bad;
	dword	tc921600_good;
	dword	tc921600_bad;
	dword	tc1228800short_good;
	dword	tc1228800short_bad;
	dword	tc1228800long_good;
	dword	tc1228800long_bad;
	dword	tc1843200_good;
	dword	tc1843200_bad;
	dword	tc2457600_good;
	dword	tc2457600_bad;
	dword	tcrate38400[16];
	dword	tcrate76800[8];
	dword	tcrate153600[4];
	dword	tcrate307200short[2];
	dword	tcrate307200long[4];
	dword	tcrate614400short[1];
	dword	tcrate614400long[2];
	dword	tcrate921600[2];
	dword	tcrate1228800short[1];
	dword	tcrate1228800long[2];
	dword	tcrate1843200[1];
	dword	tcrate2457600[1];
	dword	ccrate38400[16];
	dword	ccrate76800[8];
	dword	seq_num;
}LOG_1XEVDO_Forward_Statistics_Summary_Struct;

/**
	Structure of the log message 0x1069, 1xEV-DO Power
*/
typedef struct
{
	byte PAState;				//!<' ATType [Bit3:Bit0]: 1 ?MSM4500.. 2 ?MSM5500;
								//!<' RatchetMode [Bit4]:register shows the state of the Tx power limiting: 0 ?Tx power not limited.. 1 ?Tx power is limited
								//!<' gives the state of the power amplifier.. 0 ?PA off.. 1 ?PA on
	short TxOpenLoopPower;		//!<' Signed; Tx power determined by the open loop adjust mechanism; in units of 1/256 dBm
	short TxClosedLoopAdjus;	//!<' Signed; Tx power determined by the closed loop adjust mechanism; in units of 1/256 dB and in the range of [-128,127] dB
	short TxPilotPower;			//!<' Signed; represents the pilot power that is transmitted over the pilot channel; this is calculated based on the TxOpenLoopPower and the TxClosedLoopAdjust; in units of 1/256 dBm
	short TxTotalPower; 		//!<' Signed; register holds the total Tx power as determined by the entire Tx AGC mechanism; in units of 1/256 dBm
	short RxAGC0;				//!<' Signed; represents the field that holds the total receive power as seen by Antenna 0; in units of 1/256 dBm
	short RxAGC1; 				//!<' Signed; represents this field that holds the total receive power as seen by the Antenna 1; in units of 1/256 dBm

} LOG_1XEVDO_Power;



/** Structure for log code 0x1192
	1xEV-DO Rev. A single user Forward Statistics Summery
*/
typedef struct
{
	dword seq_num;
	dword tc0_128bits_good;
	dword tc0_256bits_good;
	dword tc0_512bits_good;
	dword tc0_1024bits_good;
	dword tc0_bad;
	dword tc1_128bits_good;
	dword tc1_256bits_good;
	dword tc1_512bits_good;
	dword tc1_1024bits_good;
	dword tc1_bad;
	dword tc2_128bits_good;
	dword tc2_256bits_good;
	dword tc2_512bits_good;
	dword tc2_1024bits_good;
	dword tc2_bad;
	dword tc3_128bits_good;
	dword tc3_256bits_good;
	dword tc3_512bits_good;
	dword tc3_1024bits_good;
	dword tc3_bad;
	dword tc4_128bits_good;
	dword tc4_256bits_good;
	dword tc4_512bits_good;
	dword tc4_1024bits_good;
	dword tc4_bad;
	dword tc5_512bits_good;
	dword tc5_1024bits_good;
	dword tc5_2048bits_good;
	dword tc5_bad;
	dword tc6_128bits_good;
	dword tc6_256bits_good;
	dword tc6_512bits_good;
	dword tc6_1024bits_good;
	dword tc6_bad;
	dword tc7_512bits_good;
	dword tc7_1024bits_good;
	dword tc7_2048bits_good;
	dword tc7_bad;
	dword tc8_1024bits_good;
	dword tc8_3072bits_good;
	dword tc8_bad;
	dword tc9_512bits_good;
	dword tc9_1024bits_good;
	dword tc9_2048bits_good;
	dword tc9_bad;
	dword tc10_4096bits_good;
	dword tc10_bad;
	dword tc11_1024bits_good;
	dword tc11_3072bits_good;
	dword tc11_bad;
	dword tc12_4096bits_good;
	dword tc12_bad;
	dword tc13_5120bits_good;
	dword tc13_bad;
	dword tc14_5120bits_good;
	dword tc14_bad;

	dword tc0_128bits[16];
	dword tc0_256bits[16];
	dword tc0_512bits[16];
	dword tc0_1024bits[16];
	dword tc1_128bits[16];
	dword tc1_256bits[16];
	dword tc1_512bits[16];
	dword tc1_1024bits[16];
	dword tc2_128bits[8];
	dword tc2_256bits[8];
	dword tc2_512bits[8];
	dword tc2_1024bits[8];
	dword tc3_128bits[4];
	dword tc3_256bits[4];
	dword tc3_512bits[4];
	dword tc3_1024bits[4];
	dword tc4_128bits[2];
	dword tc4_256bits[2];
	dword tc4_512bits[2];
	dword tc4_1024bits[2];
	dword tc5_512bits[4];
	dword tc5_1024bits[4];
	dword tc5_2048bits[4];
	dword tc6_128bits[1];
	dword tc6_256bits[1];
	dword tc6_512bits[1];
	dword tc6_1024bits[1];
	dword tc7_512bits[2];
	dword tc7_1024bits[2];
	dword tc7_2048bits[2];
	dword tc8_1024bits[2];
	dword tc8_3072bits[2];
	dword tc9_512bits[1];
	dword tc9_1024bits[1];
	dword tc9_2048bits[1];
	dword tc10_4096bits[2];
	dword tc11_1024bits[1];
	dword tc11_3072bits[1];
	dword tc12_4096bits[1];
	dword tc13_5120bits[2];
	dword tc14_5120bits[1];
	dword ccsht_128bits_good;
	dword ccsht_256bits_good;
	dword ccsht_512bits_good;
	dword ccsht_bad;
	dword cc38400_good;
	dword cc38400_bad;
	dword cc76800_good;
	dword cc76800_bad;
	dword ccsht_128bits[4];
	dword ccsht_256bits[4];
	dword ccsht_512bits[4];
	dword ccrate38400[16];
	dword ccrate76800[8];
}LOG_1XEVDO_RevA_SU_FWD_Stats_Struct;

/**
	Traffic channel formulas:
	80-V1294-1 Rev. YH
	section 5.3.269.1:
	PER:
	Througput when served
	Instantaneous throughput
	Instantaneous throughput 1 sec
*/

/** Structure for log code 0x12A2
	1xEV-DO Rev. B multi-carrier single user Forward Statistics Summery
*/
typedef struct
{
	byte version;
	dword log_sequence_number;
	byte carrier_Count;
	byte carrier_id[3];

	dword tc0_128bits_good;
	dword tc0_256bits_good;
	dword tc0_512bits_good;
	dword tc0_1024bits_good;
	dword tc0_bad;
	dword tc1_128bits_good;
	dword tc1_256bits_good;
	dword tc1_512bits_good;
	dword tc1_1024bits_good;
	dword tc1_bad;
	dword tc2_128bits_good;
	dword tc2_256bits_good;
	dword tc2_512bits_good;
	dword tc2_1024bits_good;
	dword tc2_bad;
	dword tc3_128bits_good;
	dword tc3_256bits_good;
	dword tc3_512bits_good;
	dword tc3_1024bits_good;
	dword tc3_bad;
	dword tc4_128bits_good;
	dword tc4_256bits_good;
	dword tc4_512bits_good;
	dword tc4_1024bits_good;
	dword tc4_bad;
	dword tc5_512bits_good;
	dword tc5_1024bits_good;
	dword tc5_2048bits_good;
	dword tc5_bad;
	dword tc6_128bits_good;
	dword tc6_256bits_good;
	dword tc6_512bits_good;
	dword tc6_1024bits_good;
	dword tc6_bad;
	dword tc7_512bits_good;
	dword tc7_1024bits_good;
	dword tc7_2048bits_good;
	dword tc7_bad;
	dword tc8_1024bits_good;
	dword tc8_3072bits_good;
	dword tc8_bad;
	dword tc9_512bits_good;
	dword tc9_1024bits_good;
	dword tc9_2048bits_good;
	dword tc9_bad;
	dword tc10_4096bits_good;
	dword tc10_bad;
	dword tc11_1024bits_good;
	dword tc11_3072bits_good;
	dword tc11_bad;
	dword tc12_4096bits_good;
	dword tc12_bad;
	dword tc13_5120bits_good;
	dword tc13_bad;
	dword tc14_5120bits_good;
	dword tc14_bad;

	dword tc0_128bits[16];
	dword tc0_256bits[16];
	dword tc0_512bits[16];
	dword tc0_1024bits[16];
	dword tc1_128bits[16];
	dword tc1_256bits[16];
	dword tc1_512bits[16];
	dword tc1_1024bits[16];
	dword tc2_128bits[8];
	dword tc2_256bits[8];
	dword tc2_512bits[8];
	dword tc2_1024bits[8];
	dword tc3_128bits[4];
	dword tc3_256bits[4];
	dword tc3_512bits[4];
	dword tc3_1024bits[4];
	dword tc4_128bits[2];
	dword tc4_256bits[2];
	dword tc4_512bits[2];
	dword tc4_1024bits[2];
	dword tc5_512bits[4];
	dword tc5_1024bits[4];
	dword tc5_2048bits[4];
	dword tc6_128bits[1];
	dword tc6_256bits[1];
	dword tc6_512bits[1];
	dword tc6_1024bits[1];
	dword tc7_512bits[2];
	dword tc7_1024bits[2];
	dword tc7_2048bits[2];
	dword tc8_1024bits[2];
	dword tc8_3072bits[2];
	dword tc9_512bits[1];
	dword tc9_1024bits[1];
	dword tc9_2048bits[1];
	dword tc10_4096bits[2];
	dword tc11_1024bits[1];
	dword tc11_3072bits[1];
	dword tc12_4096bits[1];
	dword tc13_5120bits[2];
	dword tc14_5120bits[1];
	byte cc_stats_included;
	//Rest of the log packet is included only if cc_stats_included is 1.
	dword ccsht_128bits_good;
	dword ccsht_256bits_good;
	dword ccsht_512bits_good;
	dword ccsht_bad;
	dword cc38400_good;
	dword cc38400_bad;
	dword cc76800_good;
	dword cc76800_bad;
	dword ccsht_128bits[4];
	dword ccsht_256bits[4];
	dword ccsht_512bits[4];
	dword ccrate38400[16];
	dword ccrate76800[8];
}LOG_1XEVDO_MC_SU_FWD_Stats_Struct;
/**
	Traffic channel formulas:
	80-V1294-1 Rev. YH
	section 5.3.432.1:
	PER:
	Througput when served
	Instantaneous throughput
	Instantaneous throughput 1 sec
*/

/** Structure of each carrier ID for log code 0x1296
	1xEV-DO Rev. B multi-carrier Airlink Summery
*/
typedef struct
{
	byte carrier_id[3];
	byte per_slot[80]; //Note: ICD 80-V1294-1 Rev. YH has a typo of 3x16, it's indeed 5x16.
	byte per_asp[42];
	byte basp_info;
	dword reserved;
	word num_bad_pkts;//of last 16 packet to compute PER
}LOG_1XEVDO_MC_Air_Link_Summary_Carrier_Struct;

/** Structure for log code 0x1296
	1xEV-DO Rev. B multi-carrier Airlink Summery
*/
typedef struct
{
	byte version;
	byte num_carriers;
	byte str;
	byte ref_subaset_locked_mode;
	LOG_1XEVDO_MC_Air_Link_Summary_Carrier_Struct carriers_stat[EVDO_MC_MAX_NUM_CARRIERS];
}LOG_1XEVDO_MC_Air_Link_Summary_Struct;

/**
	Structure for an instance of the pilots in log packet 0x129C
*/
typedef struct
{
	word pn; //!<' PN bits 8:0 - PN offset; bit 9 - reserved; bit 11:10 - contains the antenna diversity mode used for searching the pilot (00 - Ant 0;01 - Ant 1; 10 - ant 0 + ant 1;11 - reserved
	word win_center_x2; //!<' position of window center in chip x2 units;
	word earliest_position_x2; //!<'  position of the earliest peak in chip x2 units
	word peak_position[6]; //!<'  an array of 6 words of 2 bytes each to represent MSTR compensated position of peaks in chip x2 units
	word peak_energy[6]; //!<' eneary of peaks as an array of 6 words of 2 bytes each
	byte com_peak_used; //!<' peaks used for center of mass computation
	byte com_peak_energy_tresh; //!<' energy threshold for peaks considered for center of mass computation
	word com_pos_x2; //!<' center of mass RTC offset in chip x2 units.
}LOG_1XEVDO_MC_Power_Pilot_Struct;

/**
	Structure for an instance of the multi-carriers in log packet 0x129C
*/
typedef struct
{
	byte carrier_id[3]; //!<' bit mask identifying the carrier;
	byte best_asp_index; //!<' best active set pilot index;
	byte num_pilots;//!<' number of pilots per frame;
	LOG_1XEVDO_MC_Power_Pilot_Struct pilots_stat[28];

}LOG_1XEVDO_MC_Power_Carrier_Struct;

/**
	Structure for log code 0x129C
	1xEV-DO Rev. B Multi-carrier Power
*/
typedef struct
{
	byte version; //!<' Version number of the log packet
	byte demod_id_rx_agc1;//!<' Demodulator ID for antenna 1
	word pn; //!<' PN state of the log packet
	byte lna_state; //!<' LNA state of the log packet
	byte pa_state; //!<' State of the power amplifier: 0 - PA off; 1 - PA On.
	word hstr; //!<' current half-slot reference count
	dword mstr_x8; //!<' [bits 17:0 - MSTR of chip x8 units; bits 31:18 reserved
	byte eq_enabled; //!<' bit 0 - flag to indicate if equalizer is ON; bits 7:1 reserved
	byte num_carriers; //!<' number of carriers in the log packet
	LOG_1XEVDO_MC_Power_Carrier_Struct carriers_stat[EVDO_MC_MAX_NUM_CARRIERS];
}LOG_1XEVDO_MC_Power_Struct;



/******************************************************************************
						FTM - EVDO non-signaling
*******************************************************************************/

/**
	EVDO Return Status
*/
typedef enum
{
	_FTM_EVDO_CMD_SUCCESS			= 0,		//!<'Indicate Success
	_FTM_EVDO_CMD_INVALID_STATE		= 1,		//!<'Indicates that FTM cannot process the cmd in current state
	_FTM_EVDO_CMD_NO_SUCCESS		= 2			//!<'Indicates faiure
} FTM_EVDO_CMD_STATUS;

/**
	EVDO Band Class identifiers for QLIB_FTM_EVDO_PILOT_ACQ

	The enumeration values are the same as the band class number.  If an
	enumeration is not available for a new band class number then the
	an integer with the band class number can be used.

*/
typedef enum
{
	FTM_EVDO_BAND_CLASS_CELLULAR	= 0,
	FTM_EVDO_BAND_CLASS_PCS			= 1,
	FTM_EVDO_BAND_BC0	= 0,
	FTM_EVDO_BAND_BC1	= 1,
	FTM_EVDO_BAND_BC2	= 2,
	FTM_EVDO_BAND_BC3	= 3,
	FTM_EVDO_BAND_BC4	= 4,
	FTM_EVDO_BAND_BC5	= 5,
	FTM_EVDO_BAND_BC6	= 6,
	FTM_EVDO_BAND_BC7	= 7,
	FTM_EVDO_BAND_BC8	= 8,
	FTM_EVDO_BAND_BC9	= 9,
	FTM_EVDO_BAND_BC10	= 10,
	FTM_EVDO_BAND_BC11	= 11,
	FTM_EVDO_BAND_BC12	= 12,
	FTM_EVDO_BAND_BC13	= 13,
	FTM_EVDO_BAND_BC14	= 14,
	FTM_EVDO_BAND_BC15	= 15,
	FTM_EVDO_BAND_BC16	= 16,
	FTM_EVDO_BAND_BC17	= 17

} FTM_EVDO_BandClasstype;

/**
	EVDO Band Class ACK type for reverse traffic channel identifiers for QLIB_FTM_EVDO_PILOT_ACQ
*/
typedef enum
{
	FTM_EVDO_IS856_COMPLIANT = 0,       //!<' 0 Normal IS-856 operation: early decode enabled; no ACK override
	FTM_EVDO_IS856_NO_ACK,              //!<' 1 IS-856 but with no early termination; early decode disabled
	FTM_EVDO_IS890_MODE_0,              //!<' 2 First  slot decode mode; ACK each slot (test app mode)
	FTM_EVDO_IS890_MODE_1,              //!<' 3 Full packet decode mode; NAK each slot (test app mode)
	FTM_EVDO_IS890A_MODE_ACK_BPSK,      //!<' 4 Always ACK with bipolar keying (BPSK) modulation
	FTM_EVDO_IS890A_MODE_NAK_BPSK,      //!<' 5 Always NAK with bipolar keying (BPSK) modulation
	FTM_EVDO_IS890A_MODE_ACK_OOK,       //!<' 6 Always ACK with on-off keying (OOK) modulation
	FTM_EVDO_IS890A_MODE_NAK_OOK        //!<' 7 Always NAK with on-off keying (OOK) modulation
} FTM_EVDO_MAC_ACK_Mode_Enum;

/**
	EVDO dynamic range types

	This is a list of dynamic range configurations used for 1xEV-DO non-signaling,
  to convert results from AGC units to dB and dBm
*/
typedef enum
{
    FTM_EVDO_NS_DR_85_3,				//!<' 85.3dB dynamic range, min_rssi = -106.0dBm
    FTM_EVDO_NS_DR_102_4				//!<' 102.4dB dynamic range, min_rssi = -115.0dBm
} FTM_EVDO_NS_DR_Types_Enum;

/**
	EVDO DRC Length
*/
typedef enum
{
	FTM_EVDO_DRC_1_SLOTS = 0,
	FTM_EVDO_DRC_2_SLOTS,
	FTM_EVDO_DRC_4_SLOTS,
	FTM_EVDO_DRC_8_SLOTS
} FTM_EVDO_DRC_LENGTH_Enum;

/**
	EVDO DRC Gating
*/
typedef enum
{
	FTM_EVDO_DRC_CONTINUOUS = 0,
	FTM_EVDO_DRC_DISCONTINOUS
} FTM_EVDO_DRC_GATING_Enum;

/**
	EVDO RPC Step
*/
typedef enum
{
	FTM_EVDO_RPC_STEP_HalfdB = 0,
	FTM_EVDO_RPC_STEP_1dB
} FTM_EVDO_RPC_STEP_Enum;

/**
	EVDO Transmission mode
*/
typedef enum
{
	FTM_EVDO_HIGH_CAPACITY = 0,
	FTM_EVDO_LOW_LATENCY
} FTM_EVDO_TRANS_MODE_Enum;


/**
	EVDO ARQ mode
*/
typedef enum
{
	FTM_EVDO_ARQ_BPSK = 0,
	FTM_EVDO_ARQ_OOK
} FTM_EVDO_ARQ_MODE_Enum;


/**
   	EVDO Search Window Offsets
    	Pilot Drop Timer Values
*/

typedef enum
{
  FTM_EVDO_DROPTIMER_100_MS,
  FTM_EVDO_DROPTIMER_1_SEC,
  FTM_EVDO_DROPTIMER_2_SEC,
  FTM_EVDO_DROPTIMER_4_SEC,
  FTM_EVDO_DROPTIMER_6_SEC,
  FTM_EVDO_DROPTIMER_9_SEC,
  FTM_EVDO_DROPTIMER_13_SEC,
  FTM_EVDO_DROPTIMER_19_SEC,
  FTM_EVDO_DROPTIMER_27_SEC,
  FTM_EVDO_DROPTIMER_39_SEC,
  FTM_EVDO_DROPTIMER_55_SEC,
  FTM_EVDO_DROPTIMER_79_SEC,
  FTM_EVDO_DROPTIMER_112_SEC,
  FTM_EVDO_DROPTIMER_159_SEC,
  FTM_EVDO_DROPTIMER_225_SEC,
  FTM_EVDO_DROPTIMER_319_SEC,
  FTM_EVDO_DROPTIMER_MAX_SIZE
}FTM_EVDO_DROP_TIMER_Enum;


/**
	EVDO Non-signaling Log types

	This is a list of the types of log events that will be monitored during EVDO non-signaling
*/
typedef enum
{
	FTM_EVDO_NS_Log_FingerInfo,				//!<' 1xEV-DO Finger, Ver 2 (LOG_CODE 0x108A)
	FTM_EVDO_NS_Log_AGC_C0,					//!<' FTM2 log, sub log 5, for Primary AGC
	FTM_EVDO_NS_Log_AGC_C1,					//!<' FTM2 log, sub log 7, for Diversity AGC
	FTM_EVDO_NS_Log_PER,					//!<' 1xEV-DO Air Link Summary (LOG_CODE 0x1068), used for PER
	FTM_EVDO_NS_Log_FWD_Stat_Summary,		//!<' 1xEV-DO Forward Statistis Summary (LOG_CODE 0x1084)
	FTM_EVDO_NS_Log_Power_Summary,			//!<' 1xEV-DO Power logs (LOG_CODE 0x1069)
	FTM_EVDO_NS_Log_RevA_SU_FWD_Stat_Summary,	//!<' 1xEV-DO Rev. A Single User Forward Statistis Summary (LOG_CODE 0x1192)
	FTM_EVDO_NS_Log_MC_Airlink_Summary,		//!<' 1xEV-DO Multi-carrier Airlink Summary (LOG_CODE 0x1296)
	FTM_EVDO_NS_Log_MC_SU_FWD_Stat_Summary,		//!<' 1xEV-DO Multi-carrier single user Forward Statistis Summary (LOG_CODE 0x12A2)
	FTM_EVDO_NS_Log_MC_Power_Summary,		//!<' 1xEV-DO Multi-carrier Power (LOG_CODE 0x129C)

	FTM_EVDO_NS_Log_MAX						//!<' Maximum 3 of logs that are being scanned for

} FTM_EVDO_NS_Log_Types_Enum;


/**
	EVDO Non-signaling Event types

	This is a list of the types of event types that will be monitored during EVDO non-signaling.

	Generally an event is based on reveiving one asynchronous log message + checking the state
	of one log sub fields.

*/
typedef enum
{
	FTM_EVDO_NS_Event_Synchronization,		//!<' FingerInfo log, Searcher == "Synchronization" after Aquire Pilot
	FTM_EVDO_NS_Event_Idle,					//!<' FingerInfo log, Searcher == "Idle" after Aquire system time
	FTM_EVDO_NS_Event_Traffic,				//!<' FingerInfo log, Searcher == "Traffic" after Modulate Reverse Traffic
	FTM_EVDO_NS_Event_Deassign,				//!<' FingerInfo log, Searcher == "Inactive" after Deassign
											//!<' (not working on all targets)

	FTM_EVDO_NS_Event_PrimaryAGC,			//!<' FTM2 log, sub log 5, for Primary AGC
	FTM_EVDO_NS_Event_SecondaryAGC,			//!<' FTM2 log, sub log 7, for Diversity AGC
	FTM_EVDO_NS_Event_PER_Update,			//!<' 1xEV-DO Air Link Summary (LOG_CODE 0x1068), used for PER
	FTM_EVDO_NS_Event_C2I_Update,			//!<' FingerInfo log, C2I updates from individual finger reports
	FTM_EVDO_NS_Event_FWD_Stat_Summary_Update,	//!<' 1xEV-DO Forward Statistis Summary (LOG_CODE 0x1084)
	FTM_EVDO_NS_Event_Power_Update,			//!<' 1xEV-DO Power Logs (LOG_CODE 0x1069)
	FTM_EVDO_NS_Event_RevA_SU_FWD_Stat_Summary_Update,	//!<' 1xEV-DO Rev. A single user Forward Statistis Summary (LOG_CODE 0x1192)
	FTM_EVDO_NS_Event_MC_Airlink_Summary_Update,	//!<' 1xEV-DO multi-carrier airlink Summary (LOG_CODE 0x1296)
	FTM_EVDO_NS_Event_MC_SU_FWD_Stat_Summary_Update,	//!<' 1xEV-DO multi-carrier single user Forward Statistis Summary (LOG_CODE 0x12A2)
	FTM_EVDO_NS_Event_MC_Power_Update,	//!<' 1xEV-DO multi-carrier power logs (LOG_CODE 0x129C)

	FTM_EVDO_NS_Event_MAX					//!<' Maximum # of events

} FTM_EVDO_NS_Event_Types_Enum;


/**
	Searcher states, as used in the "SrchState" field of 1xEV-DO Finger log (0x108A)
*/
typedef enum
{
	FTM_EVDO_NS_Searcher_Inactive			= 0x00,	//!<'  Start (inactive)
	FTM_EVDO_NS_Searcher_Acquisition		= 0x01,	//!<'  Acquisition 0x01
	FTM_EVDO_NS_Searcher_Synchronization	= 0x02,	//!<'  Synchronization 0x02
	FTM_EVDO_NS_Searcher_Idle				= 0x03,	//!<'  Idle 0x03
	FTM_EVDO_NS_Searcher_IdleSuspended		= 0x3a,	//!<'  Idle suspended 0x3a
	FTM_EVDO_NS_Searcher_IdleOffFreq		= 0x3c,	//!<'  Idle off-frequency searching (OFS) 0x3c
	FTM_EVDO_NS_Searcher_Sleep				= 0x3e,	//!<'  Sleep 0x3e
	FTM_EVDO_NS_Searcher_Reqacusition		= 0x3f,	//!<'  Reacquisition 0x3f
	FTM_EVDO_NS_Searcher_Traffic			= 0x04,	//!<'  Traffic 0x04
	FTM_EVDO_NS_Searcher_TrafficSuspended	= 0x4a,	//!<'  Traffic suspended 0x4a
	FTM_EVDO_NS_Searcher_Traffic_OFS		= 0x4c,	//!<'  Traffic OFS 0x4c
	FTM_EVDO_NS_Searcher_Invalid			= 0xFF	//!<'  Invalid (defined at PC Library level, not embedded)
} FTM_EVDO_NS_SearcherStates_Enum;


/**
	Structure to contain statistics about the EVDO AGC History for either Primary or diversity AGC.

	One instance of this structure will be created for both "sums" and "averages"
	for both antenna 0 and antenna 1.  This means that a total of 4 will be available:
		1) C0 Sums
		2) C0 Averages
		3) C1 Sums
		4) C1 Averages

	For the following fields, when averages are calculated, the fields are converted to dB:
		double dTxGainAdj;				//!< tx_gain_adj;

	For the following fields, when averages are calculated, the fields are converted to dBm:
		double dRxAGC;					//!< rx_agc field for C0 and rx_agc_c1 for C1 logs
		double dTxGainCtl;				//!< tx_gain_ctl field
		double dTxOpenLoop;				//!< tx_open_loop;
*/
typedef struct
{
	dword iTotalRecords;	//!< Total number of AGC records that have been received

	double dRxAGC;					//!< rx_agc field for C0 and rx_agc_c1 for C1 logs -- AGC units
	double dRxAGC_linear;			//!< Linear power (mW)
	double dRxAGC_dBm;				//!< Log power (dBm)

	double dTxGainCtl;				//!< tx_gain_ctl field -- AGC units
	double dTxGainCtl_linear;		//!< Linear power (mW)
	double dTxGainCtl_dBm;			//!< Log power (dBm)

	double dTxGainAdj;				//!< tx_gain_adj -- AGC units
	double dTxGainAdj_dB;			//!< Log power (dB)

	double dTxOpenLoop;				//!< tx_open_loop -- AGC units
	double dTxOpenLoop_linear;		//!< Linear power (mW)
	double dTxOpenLoop_dBm;			//!< Log power (dBm)

	double dTxAgcAdj;				//!< tx_agc_adj -- AGC units

	double dPA_Ratchet;				//!< pa_ratchet;
	double dLNA_State;				//!< lna_state for C0 and lna_state_c1 for C1
	double dPA_State;				//!< pa_state
	double dHDET_Raw;				//!< hdet_raw;
	double dThermRaw;				//!< therm_raw;
	double dThermScaled;			//!< therm_scaled;
	double dTempIndex;				//!< temp_index
	double dTempRemainder;			//!< temp_remainder
	double dIntelliceiverState;		//!< intelliceiver_state, only for C0

} FTM_EVDO_NS_AGC_Statistics;

/**
	Structure to contain statistics about the C/I measurements
*/
typedef struct
{
	dword iTotalReports;			//!< Total # of C/I reports, each report hold 16 packet results

	long iSumC2I;					//!< Sum of all combined CtoI reports
	double dC2I_dB;					//!< Calculated Combined C2I, C_I0 (dB) = 10 x Log10 (C2I/512)

	long iSumC2I_C0;				//!< Sum of all CtoI reports for primary antenna
	double dC2I_C0_dB;				//!< Calculated C2I for primary antenna, C_I0 (dB) = 10 x Log10 (C2I/512)

	dword iSumC2I_C1;				//!< Sum of all CtoI reports for diversity antenna
	double dC2I_C1_dB;				//!< Calculated C2I for diversity antenna, C_I0 (dB) = 10 x Log10 (C2I/512)

} FTM_LOG_EVDO_CtoI_Statistics_Struct;


/**
	Structure of the log message 0x1068, 1xEV-DO Air Link Summary
*/
typedef struct
{

	byte CDRCBuffer[8];			//!<' DRC indices predicted by the predictor for the 16 slots of
								//!<' the previous frames. Each DRC index is 4 bits. Two DRC
								//!<' indices are combined into 1 byte.

	word BestASPSINRBuffer[16];	//!<' SINR for the BEST ASP for previous frame’s last 16 odd
								//!<' half slots. This represents for both the antennas.
	byte Flags;					//!<' PktRcvdFlag, PAState, BestASPPredicted

	word ASPFilteredSINR[6];	//!<' Filtered SINR for the 6 ASPs [1,6]

	word PilotPNASP[6];			//!<' Pilot PN for the 6 ASPs [1,6]

	short RPC[6];				//!<' Filtered RPC values read from the DSP for six possible
								//!<' ASPs [1,6]. RPC[..] values are IIR filtered by the DSP
								//!<' over past 64 samples.

	short PERInst;				//!<' Instantaneous Packet Error Rate. This value is calculated
								//!<' once every 16 packets are received.
								//!<' PERInst = Number of bad packets / 16; 16384 = 100%

	word PERSequence;			//!<' Packet Error Rate sequence. This represents the number
								//!<' of times the PERInstantaneous value is calculated.

	word HSTR;					//!<' Current half slot reference count (DSP register)

	byte RPCCellIndex[6];		//!<' RPC Cell map for each ASP index

} LOG_1XEVDO_Air_Link_Summary_Struct;


/**
	Structure to contain statistics about the EVDO PER History,
	from (event == FTM_EVDO_NS_Event_PER_Update)
*/
typedef struct
{
	dword iTotalReports;		//!< Total # of PER reports, each report hold 16 packet results

	dword iTotalPackets;		//!< Total # of packets that are included

	double dSumOfPER;			//!< Total of all instantaneous (kept in "bad packets/16" units)


	/*
		Calculated "PER short" average based on all the PER reports and adjusted for 16 packets per report

	*/
	double dPER_Calculated;


} FTM_EVDO_NS_PER_Statistics_Struct;

/**
	Structure to contain statistics about the EVDO Power history.
	Calculated based on Rev.0/A evdo power log 0x1069
*/
typedef struct
{
	dword iTotalReports; // each report contains 2 packets
	dword iTotalPackets;
	byte pa_state;
	double RxAGC0;
	double RxAGC1;
	double TxOpenLoopPower;
	double TxClosedLoopAdjus;
	double TxTotalPower;
	double TxPilotPower;
	dword pa_state_sum;
	double RxAGC0_Linear_sum;
	double RxAGC1_Linear_sum;
	double TxOpenLoopPower_Linear_sum;
	double TxClosedLoopAdjus_Linear_sum;
	double TxTotalPower_Linear_sum;
	double TxPilotPower_Linear_sum;
} FTM_EVDO_NS_Power_Statistics_Struct;

/**
	Structure to contain statistics about the EVDO Throughput history.
	Calculated based on Rev.0/A/B single user forward statistics summary
*/
typedef struct
{
	dword iTotalReports;
	dword iTotalPackets;
	double dThroughputWhenServed;
	double dThroughputInst;
	double dThroughputInst_1s;
	double dPacketErrorRate;
	double dPacketErrorRate_1s;
	dword seq_num;
	dword seq_num_prev;
} FTM_EVDO_NS_Throughput_Statistics_Struct;


/**
	Searcher states, as used in the "SrchState" field of 1xEV-DO Finger log (0x108A)
*/
typedef struct
{
	//! Array to keep track of whether a certain event has occured, indexed to FTM_EVDO_NS_Event_Types_Enum
	byte bHasUpdated[ FTM_EVDO_NS_Event_MAX ];

	/**
		Finger information
	*/
	FTM_EVDO_NS_SearcherStates_Enum eSearcherState;

	FTM_LOG_EVDO_CtoI_Statistics_Struct oEVDO_LatestCtoI_Stats;	//!<' Most recent CtoI report
	FTM_LOG_EVDO_CtoI_Statistics_Struct oEVDO_CtoI_Stats;		//!<' Average CtoI

	/**
		Data for primary AGC
	*/
	FTM_LOG_EVDO_AGC_C0_Struct oEVDO_AGC_C0;				//!<' Most recent Primary AGC information
	FTM_EVDO_NS_AGC_Statistics oEVDO_AGC_C0_LatestStats;	//!<' Most recent primary AGC report, converted to dBm
	FTM_EVDO_NS_AGC_Statistics oEVDO_AGC_C0_SumStats;		//!<' Sums Primary AGC
	FTM_EVDO_NS_AGC_Statistics oEVDO_AGC_C0_AvgStats;		//!<' Averages for Primary AGC


	/**
		Data for seC0ndary AGC
	*/
	FTM_LOG_EVDO_AGC_C1_Struct oEVDO_AGC_C1;				//!<' Most recent secondary AGC information
	FTM_EVDO_NS_AGC_Statistics oEVDO_AGC_C1_LatestStats;	//!<' Most recent secondary AGC report, converted to dBm
	FTM_EVDO_NS_AGC_Statistics oEVDO_AGC_C1_SumStats;		//!<' Sums for secondary AGC
	FTM_EVDO_NS_AGC_Statistics oEVDO_AGC_C1_AvgStats;		//!<' Averages for secondary AGC

	/**
		Data for PER (event == FTM_EVDO_NS_Event_PER_Update)
	*/
	word iLastInstantaneousPER;						//!<' Most recent instantaneous PER recorded,
													//!<' PERInst = Number of bad packets / 16, 16384 = 100%

	FTM_EVDO_NS_PER_Statistics_Struct oPER_Stats;	//!<' PER Statistics

	/**
		Last received Air Link Summary log message
	*/
	LOG_1XEVDO_Air_Link_Summary_Struct oAirLinkSummary;

	/**
		Last received EVDO Forward Statistics Summary
	*/
	LOG_1XEVDO_Forward_Statistics_Summary_Struct oFWDStatSummary;//!<' Most recent forward statistics
	LOG_1XEVDO_Forward_Statistics_Summary_Struct oFWDStatSummary_Prev;//!<'  Second latest packet

	FTM_EVDO_NS_Throughput_Statistics_Struct oThroughput_Stats;
	/**
		Last received EVDO Power. Each log report includes two packets
	*/
	LOG_1XEVDO_Power oPower[2];

	FTM_EVDO_NS_Power_Statistics_Struct oPower_Stats;

	/**
		Last received EVDO Rev. A Single User Forward Statistics Summary
	*/
	LOG_1XEVDO_RevA_SU_FWD_Stats_Struct oRevAFWDStatSummary; //!<' Most recent Rev. A Single User forward statistics
	LOG_1XEVDO_RevA_SU_FWD_Stats_Struct oRevAFWDStatSummary_Prev;//!<'  Second latest packet


	FTM_EVDO_NS_Throughput_Statistics_Struct oThroughput_Stats_RevA;
	/**
		Last received EVDO Multi-carrier Single User Forward Statistics Summary
	*/
	LOG_1XEVDO_MC_SU_FWD_Stats_Struct oMCFWDStatSummary[EVDO_MC_MAX_NUM_CARRIERS];//!<' Most recent Rev. B MC Single User forward statistics
	LOG_1XEVDO_MC_SU_FWD_Stats_Struct oMCFWDStatSummary_Prev[EVDO_MC_MAX_NUM_CARRIERS];//!<' Second latest forward statistics

	FTM_EVDO_NS_Throughput_Statistics_Struct oMCThroughput_Stats[EVDO_MC_MAX_NUM_CARRIERS];//!<' Throughput Statistics for MC carrier 1 - 3.

	/**
		Last received EVDO Multi-carrier Airlink Summary
	*/
	LOG_1XEVDO_MC_Air_Link_Summary_Struct oMCAirLinkSummary;
	FTM_EVDO_NS_PER_Statistics_Struct oMCPER_Stats[EVDO_MC_MAX_NUM_CARRIERS];	//!<' PER Statistics for MC carrier 1 - 3.


	/**
		Last received EVDO Multi-carrier Power
	*/
	LOG_1XEVDO_MC_Power_Struct oMCPower;


} FTM_EVDO_NS_Status_Struct;


//! Define for the bWaitForStatus flag for the EVDO NS functions -- indicate wait for status
#define FTM_EVDO_NS_WaitForStatus TRUE

//! Define for the bWaitForStatus flag for the EVDO NS functions -- indicate no wait for status
#define FTM_EVDO_NS_NoWaitForStatus FALSE


/**
	Bit mask enumeration of measurement logs available for 1XEVD0 non-signaling
*/
typedef enum
{
	FTM_EVDO_NS_LOG_AIR_LINK_SUMMARY = 0x0001,  //!<'  Air link summary 0x1068
	FTM_EVDO_NS_LOG_AGC_C0           = 0x0002,  //!<'  FTM AGC Log for primary Rx (C0)
	FTM_EVDO_NS_LOG_AGC_C1           = 0x0004,  //!<'  FTM AGC Log for secondary Rx (C1)
	FTM_EVDO_NS_LOG_Default          = 0x0003,  //!<'  Default is to include all logs air link summary and AGC Primary Chain FTM logs.
	FTM_EVDO_NS_LOG_POWER 			 = 0x0008,  //!<'  Power Logs 0x1069
	FTM_EVDO_NS_LOG_FWD_STAT 		 = 0x0010,  //!<'  Forward statistics summary 0x1084
	FTM_EVDO_NS_LOG_REVA_SU_FWD_STAT = 0x0020,  //!<'  Rev. A Single User forward statistics summary 0x1192
	FTM_EVDO_NS_LOG_MC_AIR_LINK		 = 0x0040,  //!<'  Rel. B Multi-carrier airlink summary 0x1296
	FTM_EVDO_NS_LOG_MC_SU_FWD_STAT   = 0x0080,  //!<'  Rev. B Multi-carrier Single User forward statistics summary 0x12A2
	FTM_EVDO_NS_LOG_MC_POWER    	 = 0x0100,  //!<'  Rev. B Multi-carrier Power 0x129C
	FTM_EVDO_NS_LOG_ALL				 = 0xFFFB   //!<'  All log codes available for EVDO(excluding Secondary chain FTM AGC log)

} FTM_EVDO_NS_MeasurementLogs_Enum;


/******************************************************************************
						Diagnostic - SHDR/SGPS subsytem
*******************************************************************************/
/**
	Bit mask enumeration of measurement logs available for 1XEVD0 non-signaling
*/
typedef enum
{
	SHDR_LOG_TRM_RESPONSE        = 1,  //!<'  TRM Response Log
	SHDR_LOG_1x_PMER	         = 2,  //!<'  1x Paging MER
	SHDR_LOG_1x_DEMOD_RF_SUBPKT	 = 4,  //!<'  Information from the Demod Status Log: RF SubPacket
	SHDR_LOG_1x_STATUS			 = 8,   //<'  1x Status Logs
	SHDR_LOG_EVDO_POWER  		 = 16,  //!<'  EVDO Power Log
	SHDR_LOG_EVDO_STATUS		 = 32,
	SHDR_LOG_EVDO_FINGER		 = 64,  //!<' EVDO Finget Log Packet
	SHDR_LOG_CGPS				 = 128, //<!<' All CGPS Logs
} FTM_SHDR_MeasurementLogs_Enum;


typedef struct
{
	CDMA2000_TRM_Log_Header oTRMLogs [ CDMA2000_TRM_LOG_PACKET_BUFFER_SIZE ];
	double oPagingMER;
	FTM_CDMA2000_NS_Status_Struct oCDMA2000_Status;
	FTM_EVDO_NS_Status_Struct oEVDO_Staus;
	CDMA2000_Demod_RF_Stat oDemodRFStatus;
	int iTotalTRMRecords;
	CGPS_Status_Struct oCGPS_Status;

} SHDR_SGPS_MeasurementLogs_Struct;

/******************************************************************************
						FTM - Bluetooth
*******************************************************************************/

/**
	\brief Enumeration of Bluetooth FTM Command ID's
*/
typedef enum
{
	_FTM_BT_HCI_USER_CMD		= 0		//!<' Bluetooth HCI User Command.  Use _ to avoid name conflict with function
} FTM_Bluetooth_Cmd_Id_Enum;

/**
	The maximum number of bytes in a Bluetooth FTM payload (transport mode (1 byte) + HCI event header (2 byte) + HCI event payload (255) = 258)
*/
#define FTM_BLUETOOH_LOG_PAYLOAD_MAX_SIZE 258

/**
	The maximum number of Bluetooth FTM Logs
*/
#define FTM_BLUETOOTH_LOG_MAX_LOGS	255

/**
	A Bluetooth FTM Log
*/
typedef struct
{
	byte iTimeStamp[QMSL_DIAG_TIME_STAMP_SIZE];									//!<' AMSS timestamp

	unsigned char aBluetooh_FTM_Log_Payload[FTM_BLUETOOH_LOG_PAYLOAD_MAX_SIZE];	//!<' Bluetooth FTM log payload

	unsigned short iBluetooth_FTM_Load_Payload_Size;							//!<' Size of Bluetooth FTM log payload
} FTM_Bluetooth_Log;

/**
	This buffer stores the Bluetooth FTM logs
*/
typedef struct
{
	FTM_Bluetooth_Log aBluetooh_FTM_Logs [FTM_BLUETOOTH_LOG_MAX_LOGS];			//!<' Array of FTM logs

	unsigned short iNumFTMBluetoothLogs;										//!<' Number of valid Bluetooth FTM logs in aBluetooth_FTM_Logs

	unsigned char  bBufferFull;						                            //!<' 1 indicates the internal buffer is full. Bluetooth FTM logs may have been discarded.  0 indicates the buffer is not full

} FTM_Bluetooth_Logs;

/******************************************************************************
						FTM - AGPS
*******************************************************************************/

/**
	\brief Enumeration of AGS FTM Command ID's
			Use _ to differentiate from the function names
*/
typedef enum
{
	_FTM_AGPS_SET_TEST_MODE			= 0,	//!<' Set AGPS Test Mode
	_FTM_AGPS_SELF_TEST				= 1,	//!<' AGPS Self Test
	_FTM_AGPS_IDLE_MODE				= 2,	//!<' Set AGPS Idle Mode
	_FTM_AGPS_STANDALONE_TEST_MODE	= 3,	//!<' Set AGPS Standalone Test Mode
	_FTM_AGPS_SV_TRACKING			= 4,	//!<' Set AGPS single SV tracking
	_FTM_AGPS_IQ_CAPTURE			= 5,	//!<' Set AGPS IQ Capture
	_FTM_AGPS_GET_IQ_DATA			= 6,		//!<' Set AGPS Get IQ data
	_FTM_AGPS_GET_CTON				= 122	//!<' Get AGPS C/N

} FTM_AGPS_Cmd_Id_Enum;

/******************************************************************************
						FTM - PMIC
*******************************************************************************/
/**
	FTM Subsystem dispatcher header, response with extended information + status
*/
typedef struct
{
	ftm_extended_request_header_type ftm_extended_header;
	byte status;
} ftm_pmic_response_header_type;


/**
	\brief Enumeration of PMIC sub Command ID's
*/
typedef enum
{
	_FTM_PMIC_TEST_RTC		= 0,	//!<' PMIC Real Time Clock
	_FTM_PMIC_TEST_CHG		= 1,	//!<' PMIC Charger control
    _FTM_PMIC_TEST_USB		= 2,	//!<' PMIC USB
    _FTM_PMIC_TEST_AMUX		= 3,	//!<' PMIC Analog Multiplexer
    _FTM_PMIC_TEST_VREG		= 4,	//!<' PMIC Voltage Regulator
    _FTM_PMIC_TEST_INT		= 5,	//!<' PMIC Interrupts
    _FTM_PMIC_TEST_UI		= 6,	//!<' PMIC User Interface Commands
	_FTM_PMIC_TEST_SPKR	    = 8,	//!<' PMIC Speakers
	_FTM_PMIC_TEST_VID		= 9,	//!<' PMIC Video amplifier (VID)
	_FTM_PMIC_TEST_MIC		= 10,	//!<' PMIC Microphone commands
	_FTM_PMIC_TEST_RESET	= 11,	//!<' PMIC Hard reset command resets all of the PMIC hardware blocks, excluding the RTC
	_FTM_PMIC_TEST_MPP		= 12,	//!<' PMIC Test Multi-purpose Pin (MPP)
	_FTM_PMIC_TEST_GEN		= 13,	//!<' PMIC General testing
	_FTM_PMIC_TEST_VREG_EXT  = 15   //!<' PMIC Voltage Regulator Extended (use VREG enum)
} FTM_PMIC_Cmd_Id_Enum;

/**
	\brief Enumeration of PMIC FTM Sub-command ID's
*/
typedef enum
{
	// FTM_PMIC_TEST_RTC group
	_FTM_PMIC_RTC_SET_GET_TIME			= 0,	//!<' PMIC RTC Set/Get time
	_FTM_PMIC_RTC_SET_GET_ALARM_TIME	= 1,	//!<' PMIC RTC Set/Get alarm
	_FTM_PMIC_RTC_STOP					= 2,	//!<' PMIC RTC Stop real time clock
	_FTM_PMIC_RTC_GET_ALARM_STATUS		= 3,	//!<' PMIC RTC Get alarm status
	_FTM_PMIC_RTC_DISABLE_ALARM			= 4,	//!<' PMIC RTC Disable Alarm
	_FTM_PMIC_RTC_SET_GET_TIME_ADJUST	= 5,	//!<' PMIC RTC Get/set time adjustment

	// Charger Commands
	_FTM_PMIC_CHG_SET_SWITCH_STATE		= 0,	//!<' PMIC Charger, Set switch state
    _FTM_PMIC_CHG_SET_CONFIG			= 1,	//!<' PMIC Charger, configure

	// Analog Mux
	_FTM_PMIC_AMUX_CONFIG				= 0,		//!<' PMIC Analog Mux, Configure

	// Voltage Reglator
    _FTM_PMIC_VREG_CONTROL				= 0,	//!<' PMIC Vreg, Control LDO
    _FTM_PMIC_VREG_SET_LEVEL			= 1,	//!<' PMIC Vreg, Set voltage level
    _FTM_PMIC_VREG_CONFIG_SMPS_MODE		= 2,	//!<' PMIC Vreg, configure SMPS mode
    _FTM_PMIC_VREG_CONFIG_LDO_PD_SW		= 3,	//!<' PMIC Vreg, Configure LDO Power Down
    _FTM_PMIC_VREG_LP_MODE_CONTROL		= 4,	//!<' PMIC Vreg, Configure Low Power mode
    _FTM_PMIC_VREG_SMPS_CLK_CONFIG		= 5,	//!<' PMIC Vreg, Configure SMPS clock
	_FTM_PMIC_VREG_GET_LEVEL			= 6,	//!<' PMIC Vreg, Get voltage level
	_FTM_PMIC_VREG_GET_ENUM_LIST		= 7,	//!<' PMIC Vreg, Get a list of vreg enumeration

	// Interrupts
    _FTM_PMIC_INT_GET_RT_STATUS			= 0,	//!<' PMIC Interrupt, Get interrupt status
    _FTM_PMIC_INT_CLR_IRQ				= 1,	//!<' PMIC Interrupt, clear interrupts

	// User Interface
    _FTM_PMIC_UI_SET_DRIVE_LEVEL		= 0,	//!<' PMIC User Interface, set drive level
    _FTM_PMIC_UI_CONFIG_SPKR			= 1,	//!<' PMIC User Interface, configure speaker

	// MIC Interface
	_FTM_PMIC_MIC_DISABLE				= 0,	//!<' PMIC MIC, MIC disable
	_FTM_PMIC_MIC_ENABLE				= 1,	//!<' PMIC MIC, MIC enable
	_FTM_PMIC_MIC_GET_STATUS			= 2,	//!<' PMIC MIC, Get Stats
	_FTM_PMIC_MIC_SET_LEVEL				= 3,	//!<' PMIC MIC, Set voltage level
	_FTM_PMIC_MIC_GET_LEVEL				= 4,	//!<' PMIC MIC, Get voltage level

	// Speaker
	_FTM_PMIC_SPKR_SET_GET_HPF_CORNER_FREQ = 0,  //!<' PMIC Speaker HPF Corner freq


	// Hard reset
	_FTM_PMIC_HARD_RESET				= 0,	//!<' PMIC Hard reset

	// MPP
	_FTM_PMIC_MPP_CONFIG_DIGITAL_IN		= 0,	//!<' PMIC MPP, Config digital input pin
	_FTM_PMIC_MPP_CONFIG_DIGITAL_OUT	= 1,	//!<' PMIC MPP, Config digital output pin
	_FTM_PMIC_MPP_CONFIG_DIGITAL_INOUT	= 2,	//!<' PMIC MPP, Config digital bidirectional pin
	_FTM_PMIC_MPP_CONFIG_ANALOG_IN		= 3,	//!<' PMIC MPP, Config analog input pin
	_FTM_PMIC_MPP_CONFIG_ANALOG_OUT		= 4,	//!<' PMIC MPP, Config analog output pin
	_FTM_PMIC_MPP_CONFIG_I_SINK			= 5,	//!<' PMIC MPP, Config a pin to be current sink

	// General
	_FTM_PMIC_GET_MODEL					= 0		//!<' PMIC General, Get Model


} FTM_PMIC_SubCmd_Id_Enum;


typedef enum
{
	// FTM_PMIC_TEST_RTC group
	FTM_PMIC_SET_VALUE = 0,	//!<' PMIC Set value
	FTM_PMIC_GET_VALUE = 1	//!<' PMIC Get value

} FTM_PMIC_SET_GET_Enum;

/* This type specifies the charger switch types available to control */

typedef enum
{

    FTM_PMIC_CHG_SWITCH_WALL_CHGR            = 0,
    FTM_PMIC_CHG_SWITCH_BATTERY_TRANSISTOR   = 1,
    FTM_PMIC_CHG_SWITCH_WALL_PULSE_CHGR      = 2,
    FTM_PMIC_CHG_SWITCH_VCP                  = 3,
    FTM_PMIC_CHG_SWITCH_BATT_ALARM           = 4,
    FTM_PMIC_CHG_SWITCH_COIN_CELL            = 5,
    FTM_PMIC_CHG_SWITCH_USB_CHGR             = 6,
    FTM_PMIC_CHG_SWITCH_CHG_APP              = 7,      //!<' Charger Application task
    FTM_PMIC_CHG_SWITCH_NUM_SWITCHES

} ftm_pmic_chg_switch_type;


/** This type specifies the charger type available for configuration */
typedef enum
{

    FTM_PMIC_CHG_CONFIG_WALL_PULSE_CHGR            = 0,
    FTM_PMIC_CHG_CONFIG_WALL_PULSE_CHGR_VBAT_DET   = 1,
    FTM_PMIC_CHG_CONFIG_WALL_CHGR_TRANISTOR_LIMITS = 2,
    FTM_PMIC_CHG_CONFIG_TRICKLE_CHARGER            = 3,
    FTM_PMIC_CHG_CONFIG_COIN_CELL_CHGR             = 4,
    FTM_PMIC_CHG_CONFIG_BATT_ALARM                 = 5,
    FTM_PMIC_CHG_CONFIG_USB_TRANSISTOR_LIMITS      = 6,
    FTM_PMIC_CHG_CONFIG_PWR_CTRL_LIMIT             = 7

} ftm_pmic_chg_set_config_type;

/** This type specifies the charger types available for power control limiting configuration */
typedef enum
{

    FTM_PMIC_CHG_PWR_CTRL_LIMIT_WALL = 0,
    FTM_PMIC_CHG_PWR_CTRL_LIMIT_USB  = 1

} ftm_pmic_chg_pwr_ctrl_limit_select_type;

/** This type specifies the time format for the RTC commands */
typedef enum
{

    FTM_PMIC_RTC_MODE_12_HOUR_AM  = 0,
    FTM_PMIC_RTC_MODE_12_HOUR_PM  = 1,
    FTM_PMIC_RTC_MODE_24_HOUR     = 2

} ftm_pmic_rtc_mode_type;



/** This type specifies the VERG SMPS Clock commands */
typedef enum
{

    FTM_PM_VREG_SMPS_CLK_SEL_CMD       = 0,
    FTM_PM_VREG_SMPS_CLK_TCXO_DIV_CMD  = 1,
    FTM_PM_VREG_SMPS_CLK_INVALID_CMD

} ftm_pmic_vreg_smps_config_clk_cmd;



/** This type specifies the pin types for the FTM_SET_PMIC_DRIVE_LEVEL command */
typedef enum
{

    FTM_PMIC_UI_DRV_SEL_LCD = 0,
    FTM_PMIC_UI_DRV_SEL_KPD = 1,
    FTM_PMIC_UI_DRV_SEL_VIB = 2

} ftm_pmic_ui_high_drive_pin_id_type;


/* This type specifies the interface for the set time rtc cmd */

typedef struct
{

    byte      ftm_rtc_month;
    byte      ftm_rtc_day;
    word      ftm_rtc_year;
    byte      ftm_rtc_hour;
    byte      ftm_rtc_min;
    byte      ftm_rtc_sec;

} ftm_pmic_rtc_set_time_cmd_type;

/**
	\brief Enumeration of PMIC RTC Alarm ID's
*/
typedef enum
{
	// FTM_PMIC_TEST_RTC group
	RTC_ALARM_1 = 1,
	RTC_ALARM_2 = 2,
	RTC_ALARM_3 = 3

} FTM_PMIC_RTC_Alarm_Id_Enum;

typedef enum
{
	PMIC_MIC_VOLT_2_00V = 0,
	PMIC_MIC_VOLT_1_93V = 1,
	PMIC_MIC_VOLT_1_80V = 2,
	PMIC_MIC_VOLT_1_73V = 3
} FTM_PMIC_MIC_Voltage_Enum;


/******************************************************************************
						FTM - Audio
*******************************************************************************/
/**
	\brief Enumeration of Audio FTM Sub-command ID's
*/
typedef enum
{
	_FTM_AUDIO_SET_PATH				= 0,	//!<'  Sets up the audio path
	_FTM_AUDIO_SET_VOLUME			= 1,	//!<'  Sets up the volume in the current audio path
	_FTM_AUDIO_DSP_LOOPBACK			= 2,	//!<'  Sets up audio loopback in the DSP
	_FTM_AUDIO_PCM_LOOPBACK			= 3,	//!<'  Sets up audio loopback in the codec
	_FTM_AUDIO_TONES_PLAY			= 4,	//!<'  Plays tones at specified frequency
	_FTM_AUDIO_TONES_STOP			= 5,	//!<'  Stops the tones that are currently playing
	_FTM_AUDIO_NS_CONTROL			= 6,	//!<'  Toggles the noise suppressor ON/OFF
	_FTM_AUDIO_PCM_CAPTURE			= 7,	//!<'  Start a PCM capture
	_FTM_AUDIO_GET_PCM_CAPTURE_DATA	= 8,	//!<'  Get data from a PCM capture
	_FTM_AUDIO_PCM_CAPTURE_STOP		= 9,	//!<'  Clear the PCM capture buffers
	_FTM_AUDIO_SET_CODECTXGAIN_ADJ	= 10	//!<'  Set the CodecTxGainAdjust

} FTM_Audio_SubCmd_Id_Enum;


/**
	\brief Enumeration of Audio Devices for use with the FTM_AUDIO_SET_PATH command
*/
typedef enum
{
	FTM_AUDIO_PATH_HANDSET			= 0,	//!<'  Handset
	FTM_AUDIO_PATH_HANDSFREE		= 1,	//!<'  Handsfree kit
	FTM_AUDIO_PATH_MONO_HEADSET		= 2,	//!<'  Mono headset
	FTM_AUDIO_PATH_STEREO_HEADSET	= 3,	//!<'  Stereo headset
	FTM_AUDIO_PATH_ANALOG_HANDSFREE	= 4,	//!<'  Analog handsfree kit
	FTM_AUDIO_PATH_STEREO_DAC		= 5,	//!<'  Stereo DAC
	FTM_AUDIO_PATH_SPEAKER_PHONE	= 6,	//!<'  Speaker phone
	FTM_AUDIO_PATH_TTY_HANDSFREE	= 7,	//!<'  TTY Handsfree kit
	FTM_AUDIO_PATH_TTY_HEADSET		= 8,	//!<'  TTY headset
	FTM_AUDIO_PATH_TTY_VCO			= 9,	//!<'  TTY VCO
	FTM_AUDIO_PATH_TTY_HCO			= 10,	//!<'  TTY HCO
	FTM_AUDIO_PATH_BT_INTERCOM		= 11,	//!<'  Bluetooth intercom
	FTM_AUDIO_PATH_BT_HEADSET		= 12,	//!<'  Bluetooth headset
	FTM_AUDIO_PATH_BT_AUDIO			= 13,	//!<'  Bluetooth local audio
	FTM_AUDIO_PATH_USB_AUDIO		= 14,	//!<'  USB audio
	FTM_AUDIO_PATH_FM_MONO_HS		= 15,	//!<'  FM Mono headset
	FTM_AUDIO_PATH_FM_STEREO_HS		= 16	//!<'  FM Stereo headset
} FTM_Audio_Device_Id_Enum;


typedef enum
{
	FTM_AUDIO_SND_METHOD_VOICE = 0,			//!<'  Use the device's voice generator
	FTM_AUDIO_SND_METHOD_KEY_BEEP = 1,		//!<'  Use the device's keybeep generator
	FTM_AUDIO_SND_METHOD_MESSAGE = 2,		//!<'  Use the path's ringer, or voice generator
	FTM_AUDIO_SND_METHOD_RING = 3			//!<'  Use the device's ring generator
} FTM_Audio_Sound_Methods_Enum;

/******************************************************************************
						FTM - Camera
*******************************************************************************/

/**
	\brief Enumeration of Camera FTM Sub-command ID's
*/
typedef enum
{
	_FTM_CAMERA_START				= 0,		//!<' Starts camera services
	_FTM_CAMERA_STOP				= 1,		//!<' Stops camera services
	_FTM_CAMERA_SET_PARM			= 2,		//!<' Sets operational parameters for camera sensor
	_FTM_CAMERA_SET_DIMENSIONS		= 3,		//!<' Sets camera dimensions
	_FTM_CAMERA_START_PREVIEW		= 4,		//!<' Enters preview state
	_FTM_CAMERA_STOP_PREVIEW		= 5,		//!<' Exits preview state
	_FTM_CAMERA_TAKE_PICTURE_AND_ENCODE	= 6,	//!<' Takes a picture and encode it
	_FTM_CAMERA_GET_PICTURE			= 7,		//!<' Gets raw or encoded data from phone
	_FTM_CAMERA_GET_PARM			= 8,		//!<' Gets operational parameters for camera sensor
	_FTM_CAMERA_GET_INFO			= 9,		//!<' Reads the camera information
	_FTM_CAMERA_SET_SENSOR_ID		= 10		//!<' Select the camera sensor
} FTM_Camera_SubCmd_Id_Enum;


/**
	\brief Enumeration of Camera Status, which is returned from calling camera services API.
	See also: (80-V5310-1) Camera Services Interface Specification and Operational Description
*/
typedef enum
{
	CAMERA_SUCCESS,
	CAMERA_INVALID_STATE,
	CAMERA_INVALID_PARM,
	CAMERA_INVALID_FORMAT,
	CAMERA_NO_SENSOR,
	CAMERA_NO_MEMORY,
	CAMERA_NOT_SUPPORTED,
	CAMERA_FAILED,
	CAMERA_INVALID_STAND_ALONE_FORMAT,
	CAMERA_MALLOC_FAILED_STAND_ALONE,
} FTM_Camera_Status_Enum;

/**
	\brief Enumeration of Camera Status, for use with the FTM_CAMERA_SET_PARM command.
	See also: (80-V5310-1) Camera Services Interface Specification and Operational Description, Table 2-4.
*/
typedef enum
{
	CAMERA_PARM_STATE,
	CAMERA_PARM_ACTIVE_CMD,
	CAMERA_PARM_ZOOM,
	CAMERA_PARM_ENCODE_ROTATION,
	CAMERA_PARM_SENSOR_POSITION,
	CAMERA_PARM_CONTRAST,
	CAMERA_PARM_BRIGHTNESS,
	CAMERA_PARM_SHARPNESS,
	CAMERA_PARM_EXPOSURE,
	CAMERA_PARM_WB,
	CAMERA_PARM_EFFECT,
	CAMERA_PARM_AUDIO_FMT,
	CAMERA_PARM_FPS,
	CAMERA_PARM_FLASH,
	CAMERA_PARM_RED_EYE_REDUCTION,
	CAMERA_PARM_NIGHTSHOT_MODE,
	CAMERA_PARM_REFLECT,
	CAMERA_PARM_PREVIEW_MODE,
	CAMERA_PARM_ANTIBANDING,
	CAMERA_PARM_THUMBNAIL_WIDTH,
	CAMERA_PARM_THUMBNAIL_HEIGHT,
	CAMERA_PARM_THUMBNAIL_QUALITY
} FTM_Camera_Param_Enum;


/**
	\brief Enumeration of Camera Preview settings for use with the FTM_CAMERA_SET_PARM command.
	See also: (80-V5310-1) Camera Services Interface Specification and Operational Description, Table 2-4.
*/
typedef enum
{
	CAMERA_PREVIEW_MODE_SNAPSHOT,
	CAMERA_PREVIEW_MODE_MOVIE
} FTM_Camera_Param_Preview_Enum;

/**
	\brief Enumeration of Camera orientations
*/
typedef enum
{
	CAMERA_ORIENTATION_LANDSCAPE,
	CAMERA_ORIENTATION_PORTRAIT
} FTM_Camera_Orientation_Enum;


/**
	\brief Enumeration of Camera orientations
*/
typedef enum
{
	CAMERA_RAW,
	CAMERA_JPEG
} FTM_Camera_Format_Enum;

/******************************************************************************
						FTM - Log
*******************************************************************************/


/**
	\brief Enumeration of FTM Logging commands.
*/
typedef enum
{
	_FTM_LOG_ENABLE		= 0,
	_FTM_LOG_DISABLE	= 1
} FTM_Logging_SubCmd_Id_Enum;


/*===========================================================================*/
/**
	enum log_FTM2_id_enum

	\brief Log ID's for the FTM2 log message
*/
/*===========================================================================*/
typedef enum
{
  LOG_FTM2_LOG_PRINTF		= 0x00,		//!<' FTM printf
  LOG_FTM2_LOG_BT			= 0x01,		//!<' FTM Bluetooth
  LOG_FTM2_LOG_DACC_ACCUM	= 0x02,		//!<' DACC accumulator
  LOG_FTM2_LOG_DACC_ACCUM_C0= 0x03,		//!<' DACC accumulator, antenna 2
  LOG_FTM2_LOG_1X_AGC		= 0x4,		//!<' 1X Primary AGC
  LOG_FTM2_LOG_HDR_AGC		= 0x5,		//!<' EVDO Primary AGC
  LOG_FTM2_LOG_1X_AGC_C1	= 0x6,		//!<' 1X Secondary AGC
  LOG_FTM2_LOG_HDR_AGC_C1	= 0x7,		//!<' EVDO Secondary AGC
  LOG_FTM2_LOG_IM2_DFT		= 0x8,
  LOG_FTM2_LOG_FFT			= 0x9,
  LOG_FTM2_GSM_AUTOCAL		= 0xA,		//!<' FTM GSM Auto Calibration
  LOG_FTM2_GSM_THERM_VBATT	= 0xB,		//!<' FTM GSM Therm & Batt
  LOG_FTM2_INTELLICEIVER	= 0xB,		//!<' FTM CDMA Intelliceiver
  LOG_FTM2_LOG_TX_RX_SWEEP	= 0x0B,		//!<' results from FTM_LOG_TX_RX_SWEEP
  LOG_FTM2_LOG_GSM_RX_SWEEP	= 0x0C,		//!<' results from FTM_LOG_GSM_RX_SWEEP
  LOG_FTM2_LOG_HDET_TRACKING   = 0x10,
  LOG_FTM2_LOG_ICI_CAL = 0x20,	//!<' ICI cal log message


  LOG_FTM2_LOG_WCDMA_AGC	= 0x1004,	//!<' FTM WCDMA AGC
  LOG_FTM2_LOG_GSM_BER		= 0x1005,	//!<' FTM GSM BER
  LOG_FTM2_LOG_WCDMA_BER	= 0x1006,	//!<' FTM WCDMA BER
  LOG_FTM2_LOG_WCDMA_AGC_V2 = 0x1007,	//!<' FTM WCDMA AGC V2
  LOG_FTM2_LOG_SE_BER =	0x1009  //!<' FTM WCDMA SE BER

} log_FTM2_id_enum;


//! Size of FTM Log ID
#define LOG_FTM2_LOG_ID_SIZE 2

//! Size of the header information on an FTM2 log packet, including the standard log header+FTM2 log id fields
#define LOG_FTM2_HEADER_SIZE 14

//! Size of Log Response header.  See Log Response in 3.4.13.2 of 80-V1294-1
#define LOG_RESPONSE_HEADER_SIZE 4


/*===========================================================================*/
/**
	Structure of the general FTM2LogMessage, 80-V9147-1_A - FTM Logging API
*/
/*===========================================================================*/

typedef struct
{
	// Portion unique to FTM logs
	word iFTM_LogID;	//!<' Log ID
	byte  Data[DIAG_MAX_PACKET_SIZE];		//!<' Pointer to the data

} FTM2LogMessage;

/*===========================================================================*/
/**
	Structure of the WCDMA FTM2LogMessage, 80-V9147-1_A - FTM Logging API
*/
/*===========================================================================*/

typedef struct
{
	// Portion unique to FTM logs
	word iFTM_LogID;	//!<' Log ID
	short iRX_AGC;
	short iTX_AGC;
	short iTx_AGC_ADJ;
	byte  iLNA_STATE;
	byte  iPA_STATE;
	byte  iHDET;
	word iThemistor;
	byte  iScaledThermistor;
	byte  iTempCompIndex;
	byte  iTempCompRemainder;

} FTM2LogMessage_WCDMA_AGC;

typedef struct
{
	word iFTM_LogID;
	short iRX_AGC;
	short iTX_AGC;
	short iTx_AGC_ADJ;
	byte iLNA_STATE;
	byte iPA_STATE;
	byte iHDET;
	word iThemistor;
	byte iScaledThermistor;
	byte iTempCompIndex;
	byte iTempCompRemainder;
	short iRX_AGC_C1;
	byte iLNA_STATE_C1;

} FTM2LogMessage_WCDMA_AGC_V2;


/**
	Indicator that a default timeout should be used
*/
#define FTM_NONSIG_DEFAULT_TIMEOUT  0



/******************************************************************************
						FTM - WCDMA BER
*******************************************************************************/

/**
	\brief Enumeration of FTM WCDMA BER commands.
*/
typedef enum
{
	_FTM_WCDMA_START_MODE_REQ		= 0,
	_FTM_WCDMA_STOP_MODE_REQ		= 1,
	_FTM_WCDMA_START_IDLE_MODE_REQ	= 2,
	_FTM_WCDMA_ACQUIRE_REQ			= 3,
	_FTM_WCDMA_RMC_DCH_SETUP_REQ	= 4,
	_FTM_WCDMA_RMC_DCH_SETUP_REQ_V2	= 5,
	_FTM_WCDMA_BER_RMC_DCH_TFCS_CONFIG_REQ	 = 6
} FTM_WCDMA_BER__SubCmd_Id_Enum;

/**
	Structure of response packet for certian non-sigaling commands
		CMD_CODE	- Message ID. 		DM sets CMD_CODE to 75
		SUB_SYS_ID	- FTM ID is 11.		Mode ID Mode ID for FTM WCDMA
		Reserved	- 0
		rsp_id		- 0x1002 = FTM_WCDMA_BER
		rsp_cid		- Command ID that generated.
		rsp_scid	- Subcommand ID that generated
		Status		- 0 = DIAG_FTM_STATUS_SUCCESS. 1 = DIAG_FTM_STATUS_SUCCESS
*/
typedef struct
{
	byte  iCMD_Code;
	byte  iSubSysId;
	byte  iModeID;
	byte  iReserved;
	word iRspId;
	word iRspCid;
	word iRspSCid;
	byte  iStatus;

} FTM_BER_Response_Struct;



/*===========================================================================*/
/**
	enum log_FTM2_WCDMA_BER_log_id_enum, 80-V9698-1_A

	\brief Log ID's for the WCDMA BER FTM2 log message
*/
/*===========================================================================*/
typedef enum
{
  LOG_FTM2_WCDMA_START_MODE_CNF			= 0,		//!<' FTM WCDMA BER start WCDMA mode confirmation
  LOG_FTM2_WCDMA_STOP_MODE_CNF			= 1,		//!<' FTM WCDMA BER stop WCDMA mode confirmation
  LOG_FTM2_WCDMA_START_IDLE_MODE_REQ	= 2,		//!<' FTM WCDMA BER start idle mode confirmation
  LOG_FTM2_WCDMA_ACQUIRE_CNF			= 3,		//!<' FTM WCDMA BER acquire confirmation
  LOG_FTM2_WCDMA_CPHY_SETUP_CNF			= 4,		//!<' FTM WCDMA BER CPHY setup confirmation
  LOG_FTM2_WCDMA_PHYCHAN_ESTABLISHED_IND= 5,		//!<' FTM WCDMA BER physical channel established confirmation
  LOG_FTM2_WCDMA_CPHY_ERROR_IND			= 6,		//!<' FTM WCDMA BER pyhsical channel error
  LOG_FTM2_WCDMA_RL_FAILURE_IND			= 7,		//!<' FTM WCDMA BER Reverselink error
  LOG_FTM2_WCDMA_BER_Max
} log_FTM2_WCDMA_BER_log_id_enum;


/*===========================================================================*/
/**
	enum for WCDMA non-signaling Acquisition type
*/
/*===========================================================================*/
typedef enum
{
	FTM_WCDMA_BER_AcqTypeFreqOnly		= 0,	//!<' Frequency valid only
	FTM_WCDMA_BER_AcqTypeFreqAndScrCode	= 1,	//!<' Frequency and scrambling code valid only
	FTM_WCDMA_BER_AcqTypeFreqScrCodePN	= 2		//!<' Frequency, scrambling code, and PN position valid
} FTM_WCDMA_BER_AcqType_Enum;


/*===========================================================================*/
/**
	enum for WCDMA non-signaling Acquisition mode
*/
/*===========================================================================*/
typedef enum
{
	FTM_WCDMA_BER_AcqModeMicro	= 0,	//!<' Micro acquisition (not currently supported)
	FTM_WCDMA_BER_AcqModeFull	= 1		//!<' 1 = Full acquisition freq ARFCN to attempt acquisition. Required for acq_type = 0, 1, or 2.

} FTM_WCDMA_BER_AcqMode_Enum;

/*===========================================================================*/
/**
	enum for WCDMA non-signaling RMC type
*/
/*===========================================================================*/
typedef enum
{
	FTM_WCDMA_BER_RMC_Type12_2kpbs				= 0,	//!<' RMC 12.2 kbps channel,
	FTM_WCDMA_BER_RMC_Type64kbps				= 1,	//!<' RMC 64 kbps channel,
	FTM_WCDMA_BER_RMC_Type384kpbs				= 2,	//!<' RMC 384 kbps channel, (Not currently supported)
	FTM_WCDMA_BER_RMC_Type12_2kpbs_Symmetric	= 3,	//!<' RMC 12.2 kbps channel, block size in the DL and UL is the same
	FTM_WCDMA_BER_RMC_Type64kbps_Symmetric		= 4,	//!<' RMC 64 kbps channel, block size in the DL and UL is the same
    FTM_WCDMA_BER_RMC_Type384kbps_Symmetric		= 5		//!<' RMC 384 kbps channel, block size in the DL and UL is the same

	// Note: For first 3 data rate enumeration, UL CRC bits are looped back from DL
	// Note: For last  3 data rate enumeration, UL CRC is valid and computed based on Rx data bits
} FTM_WCDMA_BER_RMC_Type_Enum;

/*===========================================================================*/
/**
	enum for WCDMA non-signaling Power Algorithm types
*/
/*===========================================================================*/
typedef enum
{
	FTM_WCDMA_BER_PCA_Type_Alg1	= 0,	//!<' Power control algorithm 1
	FTM_WCDMA_BER_PCA_Type_Alg2	= 1		//!<' Power control algorithm 2
} FTM_WCDMA_BER_PCA_Type_Enum;

/*===========================================================================*/
/**
	enum for WCDMA non-signaling Power Algorithm types
*/
/*===========================================================================*/
typedef enum
{
	FTM_WCDMA_BER_PCA_Size_1dB	= 0,	//!<' 1dB for power control algorithm
	FTM_WCDMA_BER_PCA_Size_2dB	= 1		//!<' 2dB for power control algorithm

} FTM_WCDMA_BER_PCA_Size_Enum;


/*===========================================================================*/
/**
	enum for WCDMA non-signaling Power Algorithm types
*/
/*===========================================================================*/
typedef enum
{
    FTM_WCDMA_BER_UL_TFCS_CONFIG = 0,
    FTM_WCDMA_BER_DL_TFCS_CONFIG = 1

} FTM_WCDMA_BER_TFCS_ConfigType;

/**
	Structure to store the state of WCDMA BER.  This structure is used with the functions:
	  QLIB_FTM_WCDMA_BER_ClearStatus()
	  QLIB_FTM_WCDMA_BER_GetStatus()
*/
typedef struct
{
	/**
		Status array, indexed by log_FTM2_WCDMA_BER_log_id_enum

		Will be set to true if a log message was received for that log type
	*/
	byte bHasUpdated[ LOG_FTM2_WCDMA_BER_Max ];

	//
	// Related to ACQUIRE_CNF
	//
		/**
			ACQUIRE_CNF.Status = 0 = Acquisition failed, frequency and scr_code data not valid, 1 = Acquisition successful
		*/
		byte iACQUIRE_CNF_status;

		//! ACQUIRE_CNF.Frequency = Frequency where CPICH was acquired
		dword iACQUIRE_CNF_frequency;

		//! ACQUIRE_CNF.Scr_Code = Primary CPICH scrambling code identified during acquisition
		dword iACQUIRE_CNF_scr_code;

	//
	// Related to CPHY_SETUP_CNF
	//
		/**
			CPHY_SETUP_CNF.status
				0 = CPHY_SETUP request failed
				1 = CPHY_SETUP request successful.
		*/
		byte iCPHY_SETUP_status;

	//
	// Related to PHYCHAN_ESTABLISHED_IND
	//
		/**
			PHYCHAN_ESTABLISHED_IND.status
				0 = Physical channel could not be established.
				1 = Physical channel was successfully established.
		*/
		byte iPHYCHAN_ESTABLISHED_IND_status;

	//
	// WCDMA AGC message
	//
		//! Updated flag for WCDMA AGC
		byte bWCDMA_AGChasUpdated;

		//! Storage for WCDMA AGC
		FTM2LogMessage_WCDMA_AGC oWCDMA_AGC;

}  WCDMA_BER_State;


/*===========================================================================*/
/**
	enum for WCDMA non-signaling N312 parameters
*/
/*===========================================================================*/
typedef enum
{
    FTM_WCDMA_BER_N312_s1    = 0,
    FTM_WCDMA_BER_N312_s50   = 1,
    FTM_WCDMA_BER_N312_s100  = 2,
    FTM_WCDMA_BER_N312_s200  = 3,
    FTM_WCDMA_BER_N312_s400  = 4,
    FTM_WCDMA_BER_N312_s600  = 5,
    FTM_WCDMA_BER_N312_s800  = 6,
    FTM_WCDMA_BER_N312_s1000 = 7,

} ftm_wcdma_ber_n312_type;


/*===========================================================================*/
/**
	enum for WCDMA non-signaling N313 parameters
*/
/*===========================================================================*/
typedef enum
{

    FTM_WCDMA_BER_N313_s1   = 0,
    FTM_WCDMA_BER_N313_s2   = 1,
    FTM_WCDMA_BER_N313_s4   = 2,
    FTM_WCDMA_BER_N313_s10  = 3,
    FTM_WCDMA_BER_N313_s20  = 4,
    FTM_WCDMA_BER_N313_s50  = 5,
    FTM_WCDMA_BER_N313_s100 = 6,
    FTM_WCDMA_BER_N313_s200 = 7

} ftm_wcdma_ber_n313_type;

/*===========================================================================*/
/**
	enum for WCDMA non-signaling N315 parameters
*/
/*===========================================================================*/
typedef enum
{

    FTM_WCDMA_BER_N315_s1   = 0,
    FTM_WCDMA_BER_N315_s50  = 1,
    FTM_WCDMA_BER_N315_s100 = 2,
    FTM_WCDMA_BER_N315_s200 = 3,
    FTM_WCDMA_BER_N315_s400 = 4,
    FTM_WCDMA_BER_N315_s600 = 5,
    FTM_WCDMA_BER_N315_s800 = 6,
    FTM_WCDMA_BER_N315_s1000 = 7

} ftm_wcdma_ber_n315_type;


/******************************************************************************
						FTM - HSDPA BLER
*******************************************************************************/

/** This enumeration specifies the pre-configured HSET list, to be used with QLIB_FTM_HSDPA_BLER_Configure_HS_DSCH */
typedef enum
{
	HSDPA_HSET_1_QPSK = 1,		//!< ' HSET 1, QPSK
	HSDPA_HSET_1_16QAM,			//!< ' HSET1, 16QAM
	HSDPA_HSET_2_QPSK,			//!< ' HSET2, QPSK
	HSDPA_HSET_2_16QAM,			//!< ' HSET2, 16QAM
	HSDPA_HSET_3_QPSK,			//!< ' HSET3, QPSK
	HSDPA_HSET_3_16QAM,			//!< ' HSET3, 16QAM
	HSDPA_HSET_4_QPSK,			//!< ' HSET4, QPST
	HSDPA_HSET_5_QPSK,			//!< ' HSET5, QPST
	HSDPA_HSET_6_QPSK = 0xFF	//!< ' HSET6, QPST
} ftm_hsdpa_ber_hset_type;

/** This type specifies the possible asynchronous event packet types */
typedef enum
{

    FTM_LOG_HSDPA_HS_CHANNEL_START_IND	= 0x40,	//!< ' HSDPA Start completion indicator
    FTM_LOG_HSDPA_HS_CHANNEL_STOP_IND	= 0x41,	//!< ' HSDPA Stop completion indicator
    FTM_LOG_HSDPA_HS_CHANNEL_RECFG_IND	= 0x42,	//!< ' HSDPA Reconfigure completion indicator


	// Identifiers to help with the size of the event list
	FTM_LOG_HSDPA_EVENT_Max,										//!< ' Maximum ID + 1
	FTM_LOG_HSDPA_EVENT_First = FTM_LOG_HSDPA_HS_CHANNEL_START_IND,	//!< ' First valid ID
	FTM_LOG_HSDPA_EVENT_Last = FTM_LOG_HSDPA_EVENT_Max - 1,			//!< ' Last valid ID
	FTM_LOG_HSDPA_EVENT_Count = FTM_LOG_HSDPA_EVENT_Max -
								FTM_LOG_HSDPA_EVENT_First			//!< ' Size of valid list

} log_FTM2_HSDPA_BER_log_id_enum;


/**
	Enumeration of HSDPA channel states
*/
typedef enum
{
    FTM_HSDPA_BLER_HS_CHAN_STATE_INACTIVE,
    FTM_HSDPA_BLER_HS_CHAN_STATE_ACTIVE

} ftm_hsdpa_bler_hs_chan_state_type;

/**
	Enumeration of HSDPA force-stop error states
*/
typedef enum
{
    FTM_HSDPA_BLER_HS_FORCE_STOP__NO_ERROR = 0,
    FTM_HSDPA_BLER_HS_FORCE_STOP__ERROR_OP_IN_PROGRESS,
    FTM_HSDPA_BLER_HS_FORCE_STOP__ERROR_HS_INACTIVE

} ftm_hsdpa_bler_hs_force_stop_error_type;



/**
	Structure to store the state of HSDPA BER.  This structure is used with the functions:
	  QLIB_FTM_HSDPA_BER_ClearStatus()
	  QLIB_FTM_HSDPA_BER_GetStatus()
*/
typedef struct
{
	/**
		Status array, indexed by log_FTM2_WCDMA_BER_log_id_enum

		Will be set to true if a log message was received for that log type
	*/
	byte bHasUpdated[ FTM_LOG_HSDPA_EVENT_Count ];

	//
	// Related to FTM_LOG_HSDPA_HS_CHANNEL_START_IND
	//
		/**
			CHANNEL_START_IND.status = 0 = Start operation failed, 1 = Start operation success
		*/
		byte iCHANNEL_START_IND_status;

	//
	// Related to FTM_LOG_HSDPA_HS_CHANNEL_STOP_IND
	//
		/**
			CHANNEL_STOP_IND.status = 0 = Stop operation failed, 1 = Stop operation success
		*/
		byte iCHANNEL_STOP_IND_status;

	//
	// Related to FTM_LOG_HSDPA_HS_CHANNEL_RECFG_IND
	//
		/**
			CHANNEL_RECFG_IND.ReconfigureStatus = 0 = Reconfigure operation failed, 1 = Reconfigure operation success
		*/
		byte iCHANNEL_RECFG_IND_ReconfigureStatus;


}  HSDPA_BER_State;


/******************************************************************************
						FTM - GSM BER
*******************************************************************************/

/**
	\brief Enumeration of FTM GSM BER commands.
*/
typedef enum
{
	_FTM_GSM_START_MODE_REQ				= 0,
	_FTM_GSM_SELECT_SPECIFIC_BCCH_REQ	= 1,
	_FTM_GSM_START_IDLE_MODE_REQ		= 2,
	_FTM_GSM_CONFIG_LOOPBACK_TYPE_REQ	= 3,
	_FTM_GSM_CHANNEL_ASSIGN_REQ			= 4,
	_FTM_GSM_CHANNEL_RELEASE_REQ		= 5,
	_FTM_GSM_STOP_GSM_MODE_REQ			= 6,
	_FTM_GSM_BER_CHANNEL_ASSIGN_V2_REQ	= 7
} FTM_GSM_BER_SubCmd_Id_Enum;


/**
	\brief Enumerations for GSM Band, for GSM BER tests
*/
typedef enum
{
	FTM_GSM_BER_PGSM_900,
	FTM_GSM_BER_EGSM_900,
	FTM_GSM_BER_PCS_1900,
	FTM_GSM_BER_DCS_1800,
	FTM_GSM_BER_CELL_850
} FTM_GSM_BER_Band_Enum;


/**
	\brief Enumerations for GSM loopback types
*/
typedef enum
{
	FTM_GSM_BER_Disable_Loopback,
	FTM_GSM_BER_Loopback_Type_A,
	FTM_GSM_BER_Loopback_Type_B,
	FTM_GSM_BER_Loopback_Type_C
} FTM_GSM_BER_LoopbackType_Enum;

/**
	\brief Enumerations for channel modes
*/
typedef enum
{
	FTM_GSM_BER_SPEECH_FULL_RATE,
	FTM_GSM_BER_SPEECH_HALF_RATE,
	FTM_GSM_BER_SPEECH_ENHANCED_FULL_RATE,
	FTM_GSM_BER_AMR_FULL_RATE,
	FTM_GSM_BER_AMR_HALF_RATE,
	FTM_GSM_BER_DATA_14_4_KBPS_FULL_RATE,
    FTM_GSM_BER_DATA_9_6_KBPS_FULL_RATE
} FTM_GSM_BER_Channel_Modes_Enum;


/*===========================================================================*/
/**
	enum log_FTM2_GSM_BER_log_id_enum, 80-V3951-1

	\brief Log ID's for the GSM BER FTM2 log message
*/
/*===========================================================================*/
typedef enum
{
  LOG_FTM2_START_GSM_MODE_CNF			= 0,	//!<' Start GSM confirm
  LOG_FTM2_SELECT_SPECIFIC_BCCH_CNF		= 1,	//!<' FTM GSM BER select specific BCCH confirmation
  LOG_FTM2_START_IDLE_MODE_CNF			= 2,	//!<' FTM GSM BER start idle mode confirmation
  LOG_FTM2_CHANNEL_ASSIGN_CNF			= 3,	//!<' FTM GSM BER channel assign confirmation
  LOG_FTM2_CHANNEL_RELEASE_CNF			= 4,	//!<' FTM GSM BER channel release confirmation
  LOG_FTM2_STOP_GSM_MODE_CNF			= 5,	//!<' FTM GSM BER stop GSM mode confirmation
  LOG_FTM2_PH_DATA_IND					= 6,	//!<' FTM GSM BER Ph data indication
  LOG_FTM2_FTM_LOG_GSM_BER_DED_RPT		= 7,	//!<' FTM GSM BER dedicated measurement report

  LOG_FTM2_FTM_LOG_GSM_MAX					//!<' Size of list, not a valid log ID
} log_FTM2_GSM_BER_log_id_enum;


/*===========================================================================*/
/**
	enum log_FTM2_GSM_BER_PH_Channel_Type_enum, 80-V3951-1

	\brief Channel types for the PH_DATA_IND message
*/
/*===========================================================================*/
typedef enum
{
	LOG_FTM2_GSM_PH_IND_DCCH	= 0,	//!<' DCCH
	LOG_FTM2_GSM_PH_IND_BCCH	= 1,	//!<' BCCH
	LOG_FTM2_GSM_PH_IND_RACH	= 2,	//!<' RACH
	LOG_FTM2_GSM_PH_IND_CCCH	= 3,	//!<' CCCH
	LOG_FTM2_GSM_PH_IND_SACCH	= 4,	//!<' SACCH
	LOG_FTM2_GSM_PH_IND_SDCCH	= 5,	//!<' SDCCH
	LOG_FTM2_GSM_PH_IND_FACCH_F	= 6,	//!<' FACCH_F
	LOG_FTM2_GSM_PH_IND_FACCH_H	= 7,	//!<' FACCH_H

	LOG_FTM2_FTM_PH_IND_Chan_Max	//!<' Size of list, not a valid channel ID
} log_FTM2_GSM_BER_PH_Channel_Type_enum;

/**
	Structure to store the state of GSM BER.  This structure is used with the functions:
	  QLIB_FTM_GSM_BER_ClearStatus()
	  QLIB_FTM_GSM_BER_GetStatus()
*/
typedef struct
{
	/**
		Status array, indexed by log_FTM2_GSM_BER_log_id_enum

		Will be set to true if a log message was received for that log type
	*/
	byte bHasUpdated[ LOG_FTM2_FTM_LOG_GSM_MAX ];

	//
	// Related to SELECT_SPECIFIC_BCCH_CNF
	//
		/**
			This confirmation indicates that the select specific BCCH request has completed, and reports
			whether the FCCH/SCH channels were detected, as well as the SCH data.

			0 = FCCH/SCH not found. Acquisition failed. 1 = FCCH/SCH acquired successfully.
		*/
		byte bSCCH_Found;

		//! Data decoded from SCH channel. Data is only valid if ‘bSCCH_Found?is 1.
		dword iSchData;

	//
	// Related to CHANNEL_ASSIGN_CNF
	//
		//! 0 = Success. >0 = Error occurred.
		byte iCHANNEL_ASSIGN_CNF_status;

	//
	// Related to PH_DATA_IND
	//
		/**
			The last channel type message:

			0 = DCCH
			1 = BCCH
			2 = RACH
			3 = CCCH
			4 = SACCH
			5 = SDCCH
			6 = FACCH_F
			7 = FACCH_H

			Not all of these channel_types are used during the BER test.
		*/
		byte iPH_ChannelType;

		/**
			The last status:

			0 = Data block for the associated channel_type has not passed the
			crc_check
			1 = Data block for the associated channel_type has passed the
			crc_check
		*/
		byte iPH_CRC_Pass;

		/**
			Array of channels, indicating whether a certain channel status has been received
			since the last time that status was cleared.
		*/
		byte abHasPH_DATA_ReceivedChannelStatus[ LOG_FTM2_FTM_PH_IND_Chan_Max ] ;

		/**
			Array of channels, indicating channel status.  Only valid if the cooresponding
			abHasRecievedChannelStatus[] array index is set to true, to indicate that a status
			message for this channel has been received since the last time that status was cleared.
		*/
		byte abPH_DATA_LastChannelStatus[ LOG_FTM2_FTM_PH_IND_Chan_Max ] ;


	//
	// Related to GSM_BER_DED_RPT
	//
		//! RxLev_Full as reported by Layer 1
		byte iDED_RPtRxLev_Full;

		//! RxQual_Full as reported by Layer 1
		byte iDED_RPtRxQual_Full;


	//
	// Related to call status, SACCH accumulator which indicates
	//
		//! Starting level for the SACCH counter, will be set to value of 40, but user can override
		short iGSM_BER_SACCH_Accumulator_Max;

		//! Current value of the SACCH counter, decremented 1 for a failed SACCH, incremented 1 for a good SACCH
		//! saturated at 0 on low end and iGSM_BER_SACCH_Count_max on the high end
		short iGSM_BER_SACCH_Accumulator_Value;

		//! Total number of updates that have been made to iGSM_BER_SACCH_Accumulator_Value.
		//! if the value is zero, then no updates have been made so status cannot be determined
		short iGSM_BER_SACCH_Accumulator_TotalUpdates;

}  GSM_BER_State;

/**
	Enumeration to control whether a non-signaling session should be initialized
*/
typedef enum
{
	FTM_GSM_BER_SLOT_0  = 0,		//!<' Slot 0
	FTM_GSM_BER_SLOT_1  = 1,		//!<' Slot 1
	FTM_GSM_BER_SLOT_2  = 2,		//!<' Slot 2
	FTM_GSM_BER_SLOT_3  = 3,		//!<' Slot 3
	FTM_GSM_BER_SLOT_4  = 4,		//!<' Slot 4
	FTM_GSM_BER_SLOT_5  = 5,		//!<' Slot 5
	FTM_GSM_BER_SLOT_6  = 6,		//!<' Slot 6
	FTM_GSM_BER_SLOT_7  = 7			//!<' Slot 7

} FTM_GSM_BER_Slot_Enum;

/******************************************************************************
						FTM - EGPRS BLER
*******************************************************************************/

/**
	Enumeration of EGPRS BER log events
*/
typedef enum
{

    FTM_LOG_EGPRS_BER_TBF_IND           = 0x40,
    FTM_LOG_EGPRS_BER_MAC_PH_DATA_IND,
    FTM_LOG_EGPRS_BER_SERVING_MEAS_IND,

	// Identifiers to help with the size of the event list
	FTM_LOG_EGPRS_EVENT_Max,											//!< ' Maximum ID + 1
	FTM_LOG_EGPRS_EVENT_First	= FTM_LOG_EGPRS_BER_TBF_IND,			//!< ' First valid ID
	FTM_LOG_EGPRS_EVENT_Last	= FTM_LOG_EGPRS_EVENT_Max - 1,			//!< ' Last valid ID
	FTM_LOG_EGPRS_EVENT_Count	= FTM_LOG_EGPRS_EVENT_Max -
									FTM_LOG_EGPRS_EVENT_First			//!< ' Size of valid list

} log_FTM2_EGPRS_BER_log_id_enum;


/**
	This enumeration specifies the types of TBFs that we generate events for
*/

typedef enum
{
    FTM_LOG_EGPRS_BER_TBF_DL,	//!<' 0 = DL TBF
    FTM_LOG_EGPRS_BER_TBF_UL	//!<' 1 = UL TBF

} ftm_log_egprs_tbf_enum;

/**
	This enumeration specifies the sub-event types for the FTM_LOG_EGPRS_BER_TBF_IND EVENT
*/
typedef enum
{

    FTM_LOG_EGPRS_TBF_EVENT_ESTABLISHED,	//!<' TBF Established. TBF type indicated in
											//!<' tbf_type field has been successfully established.

    FTM_LOG_EGPRS_TBF_EVENT_RELEASED,		//!<' TBF Released. TBF type indicated in tbf_type field has been
											//!<' successfully released.

    FTM_LOG_EGPRS_TBF_EVENT_ERROR			//!<' TBF Error. An error has occurred related to the TBF indicated in
											//!<' the tbf_type field. This may require a reconfiguration of the TBF
											//!<' before proceeding.

} ftm_log_egprs_tbf_event_type;


/**
	Structure to store the state of EGPRS BER.  This structure is used with the functions:
	  QLIB_FTM_EGPRS_BER_ClearStatus()
	  QLIB_FTM_EGPRS_BER_GetStatus()
*/
typedef struct
{
	/*

	Events represented:
		FTM_LOG_EGPRS_BER_TBF_IND
		FTM_LOG_EGPRS_BER_MAC_PH_DATA_IND,
		FTM_LOG_EGPRS_BER_SERVING_MEAS_IND,
	*/


	/**
		Status array, indexed by log_FTM2_EGPRS_BER_log_id_enum

		Will be set to true if a log message was received for that log type
	*/
	byte bHasUpdated[ FTM_LOG_EGPRS_EVENT_Count ];

	//
	// Related to FTM_LOG_EGPRS_BER_TBF_IND
	//
		/**
			Type of event the event is for:
			\code
               0 = DL TBF
               1 = UL TBF
			\endcode

			Type can be cast as: ftm_log_egprs_tbf_type

		*/
			byte bTBF_IND_type;

		/**
			This indication informs the user of an event related to either the DL or UL TBF.

			\code
              0 = TBF Established. TBF type indicated in tbf_type field has been
              successfully established.

              1 = TBF Released. TBF type indicated in tbf_type field has been
              successfully released.

              2 = TBF Error. An error has occurred related to the TBF indicated in
              the tbf_type field. This may require a reconfiguration of the TBF
              before proceeding.
			\endcode

			Type can be cast as: ftm_log_egprs_tbf_event_type
	*/
		byte eTBF_IND_UL_event;

	//! DL status, same as the UL values
		byte eTBF_IND_DL_event;


	//
	// Related to FTM_LOG_EGPRS_BER_SERVING_MEAS_IND
	//
		//! RxLev_average as reported by Layer 1. Range should be 0-63.
		byte iSERVING_MEAS_IND_RxLev_average;

}  EGPRS_BER_State;


/******************************************************************************
						FTM - Common RF
*******************************************************************************/

/**
	This enumeration specifies the RX Action list for the FTM_TX_RX_FREQ_CAL_SWEEP function
*/
typedef enum
{
	FTM_TX_RX_FREQ_RX_ACTION_LNA0	= 0,	//!<' - calibrate LNA offset 0
	FTM_TX_RX_FREQ_RX_ACTION_LNA1	= 1,	//!<' - calibrate LNA offset 1
	FTM_TX_RX_FREQ_RX_ACTION_LNA2	= 2,	//!<' - calibrate LNA offset 2
	FTM_TX_RX_FREQ_RX_ACTION_LNA3	= 3,	//!<' - calibrate LNA offset 3
	FTM_TX_RX_FREQ_RX_ACTION_LNA4	= 4,	//!<' - calibrate LNA offset 4
	FTM_TX_RX_FREQ_RX_ACTION_DVGA	= 100,	//!<' - calibrate DVGA offset
	FTM_TX_RX_FREQ_RX_ACTION_NOTHING = 255	//!<' - do nothing

} ftm_tx_rx_freq_cal_sweep_rx_action_enum;

/**
	This enumeration specifies RX chain list for the FTM_TX_RX_FREQ_CAL_SWEEP function
*/
typedef enum
{
	FTM_TX_RX_FREQ_RX_CHAIN_0		= 0,	//!<' - Primary RX
	FTM_TX_RX_FREQ_RX_CHAIN_1		= 1		//!<' - Secondary RX

} ftm_tx_rx_freq_cal_sweep_rx_chain_enum;


/**
	List specifying the rx power mode to use for each step of a given frequency.
	The values are ignored for chipsets that do not support rx power modes.
*/
typedef enum
{
	FTM_TX_RX_FREQ_RX_PM_High		= 0,	//!<' - Rx high power mode
	FTM_TX_RX_FREQ_RX_PM_Medium		= 1,	//!<' - Rx medium power mode
	FTM_TX_RX_FREQ_RX_PM_Low		= 2		//!<' - Rx low power mode

} ftm_tx_rx_freq_cal_sweep_rx_power_mode_enum;



/**
	Maximum number of frequencies
*/
#define FTM_TX_RX_FREQ_MAX_FREQUENCIES 16

/**
	Maximum number of Segments per frequency
*/
#define FTM_TX_RX_FREQ_MAX_SEGMENTS 20

/**
	Structure that contains a super set of the command request parameters and the
	resulting log.  Divided into an inputs section and outputs section.

	To be used with the function QLIB_FTM_TX_RX_FREQ_CAL_SWEEP()
*/

typedef struct
{
/**
Inputs, must be filled in before the QLIB_FTM_TX_RX_FREQ_CAL_SWEEP() function is called, but
        will also be overwritten by the response data.

*/
	/**
		Specifies which rx chain is active for each segment
        \code
        FTM_TX_RX_FREQ_RX_CHAIN_0 = 0, //!<' - Primary RX
        FTM_TX_RX_FREQ_RX_CHAIN_1 = 1  //!<' - Secondary RX
		\endcode
	*/
	word iRxChain;

	/**
		The number of frequencies on which the calibration procedure will be performed. Maximum is 16.
	*/
	byte iNumFreqs;

	/**
		The number of steps per frequency. A step corresponds to a power level at which tx
		and rx measurements may be performed. This number must include one step for switching
		frequency. Maximum value is 6.
	*/
	byte iNumSteps;


	/**
		The length of one power step in units of sleep counter cycles. Minimum value is 655 (20ms).
	*/
	word iStepLength;

	/**
		List of uplink channel numbers on which calibration is to be performed. Must have num_freqs elements.
	*/
	word aiChannelList[ FTM_TX_RX_FREQ_MAX_FREQUENCIES ];

	/**
		List giving the tx pdm to set for each step. Must have num_steps elements.
	*/
	word aiTxPDM_List[ FTM_TX_RX_FREQ_MAX_SEGMENTS ];

	/**
		The expected agc value used as the input to the LNA or DVGA offset calibration routine
		for the primary RX path for each step. If rx_action_list is 255 for a particular step then
		the expected agc value is not used.
	*/
	short aiExpectedRxAGC_ValList[ FTM_TX_RX_FREQ_MAX_SEGMENTS ];

	/**
		List giving the desired PA range for each step at a given frequency. Must have
		num_steps elements. A value of 255 indicates no tx activity will take place for that step.
	*/
	byte aiPA_RangeList[ FTM_TX_RX_FREQ_MAX_SEGMENTS ];

	/**
		Flag indicating whether to take an HDET measurement for each step. Zero
		indicates an HDET measurement will not be taken, a non-zero value indicates an
		HDET measurement will be taken.
	*/
	byte aiReadHDET_List[ FTM_TX_RX_FREQ_MAX_SEGMENTS ];

	/**
		List specifying the rx action to take for each step of a given frequency.

		Valid values are defined by the enumeration ftm_tx_rx_freq_cal_sweep_rx_action_enum:
				\code
                FTM_TX_RX_FREQ_RX_ACTION_LNA0	= 0,	//!<' - calibrate LNA offset 0
                FTM_TX_RX_FREQ_RX_ACTION_LNA1	= 1,	//!<' - calibrate LNA offset 1
                FTM_TX_RX_FREQ_RX_ACTION_LNA2	= 2,	//!<' - calibrate LNA offset 2
                FTM_TX_RX_FREQ_RX_ACTION_LNA3	= 3,	//!<' - calibrate LNA offset 3
                FTM_TX_RX_FREQ_RX_ACTION_LNA4	= 4,	//!<' - calibrate LNA offset 4
                FTM_TX_RX_FREQ_RX_ACTION_DVGA	= 100,	//!<' - calibrate DVGA offset
                FTM_TX_RX_FREQ_RX_ACTION_NOTHING = 255	//!<' - do nothing
				\endcode
	*/
	byte aiRxActionList[ FTM_TX_RX_FREQ_MAX_SEGMENTS ];



	/**
		List specifying the rx power mode to use for each step of a given frequency.
		The values are ignored for chipsets that do not support rx power modes.
		Valid values are:
        \code
        FTM_TX_RX_FREQ_RX_PM_High		= 0,	//!<' - Rx high power mode
        FTM_TX_RX_FREQ_RX_PM_Medium		= 1,	//!<' - Rx medium power mode
        FTM_TX_RX_FREQ_RX_PM_Low		= 2		//!<' - Rx low power mode
		\endcode
	*/
	byte aiRxPowerMode[ FTM_TX_RX_FREQ_MAX_SEGMENTS ];


/**
Output, will be filled in by QLIB_FTM_TX_RX_FREQ_CAL_SWEEP()

*/
	/**
		 The result of the requested rx calibration on the primary receive path (if any) for
		 a given step and frequency. The results are ordered as each step for the first frequency,
		 followed by each step for the second frequency, etc.
	*/
	word aiRxResults[FTM_TX_RX_FREQ_MAX_FREQUENCIES][ FTM_TX_RX_FREQ_MAX_SEGMENTS ];

	/**
		The HDET value for each step and each frequency. If read_hdet_list is not set to one
		for a particular step than hdet_results should be ignored for the corresponding steps.
		The results are ordered as each step for the first frequency, followed by each step for
		the second frequency, etc.
	*/
	word aiHDET_Results[FTM_TX_RX_FREQ_MAX_FREQUENCIES][ FTM_TX_RX_FREQ_MAX_SEGMENTS ];

	/**
		This value will be set to a 1 if the response log received from the mobile includes
		the extended fields for rx_chain_list and rx_lpm_list.

		If this value is zero, then the values of aiRxChainList and aiRxPowerMode should
		not be used
	*/
	unsigned char bIsExtendedFormat;

} FTM_Tx_Rx_Freq_Cal_Sweep_Request_Response;


#define MAX_GSM_RX_SWEEP_FRAMES 100
#define GSM_RX_SWEEP_SLOT_PER_FRAME 8

/**
	FTM GSM Rx sweep calibration state enumeration
*/
typedef enum
{
	L1_FTM_RX_SWEEP_STATUS_NULL						= 1,	//!<' Status null
	L1_FTM_RX_SWEEP_STATUS_FCCH_STARTED				= 2,	//!<' FCCH acquisition started
	L1_FTM_RX_SWEEP_STATUS_FCCH_FAIL				= 3,	//!<' FCCH acquisition failed
	L1_FTM_RX_SWEEP_STATUS_FCCH_SUCCESS				= 4,	//!<' FCCH acquisition successful
	L1_FTM_RX_SWEEP_STATUS_SCH_STARTED				= 5,	//!<' SCH acquisition started
	L1_FTM_RX_SWEEP_STATUS_SCH_FAIL					= 6,	//!<' SCH acquisition failed
	L1_FTM_RX_SWEEP_STATUS_SCH_SUCCESS				= 7,	//!<' SCH acquisition successful
	L1_FTM_RX_SWEEP_STATUS_POWER_MEAS_STARTED		= 8,	//!<' RSSI power measurements started
	L1_FTM_RX_SWEEP_STATUS_POWER_MEAS_ABORTED		= 9,	//!<' RSSI power measurements aborted
	L1_FTM_RX_SWEEP_STATUS_POWER_MEAS_COMPLETE		= 10	//!<' RSSI power measurements done - Success
} ftm_gsm_rx_freq_cal_sweep_state_type;

/**
	Structure that is part of the FTM response packet for GSM Rx Sweep Calibration.
	Contains the status and number of frames in the packet.
*/
typedef struct
{
	word	iStatus;			//<!' GSM Rx Sweep status
	word	iNumFrames;		//<!' Number of frames in the response packet
} ftm_gsm_rx_freq_cal_sweep_response_hdr;

/**
	Structure that is part of the FTM response packet for GSM Rx Sweep Calibration.
	Contains the contents of a single frame in the response.
*/
typedef struct
{
	dword  aRSSI[GSM_RX_SWEEP_SLOT_PER_FRAME];		//<!' RSSI measurements for each measure
} ftm_gsm_rx_freq_cal_sweep_response_frame;

/**
	Structure that contains a super set of the command request parameters and the
	resulting log.  Divided into an inputs section and outputs section.

	To be used with the function QLIB_FTM_GSM_RX_FREQ_CAL_SWEEP()
*/
typedef struct
{
/**
Inputs: must be filled in when QLIB_FTM_GSM_RX_FREQ_CAL_SWEEP() is called
*/
	//! FCCH/SCH channel
	word iFchChan;
	//! FCCH/SCH band
	word iFchBand;
	//! Number of measurement frames
	word iNumFrames;
	//! Band list for all frames
	word aBand[MAX_GSM_RX_SWEEP_FRAMES];
	//! Channel list for all frames
	word aChan[MAX_GSM_RX_SWEEP_FRAMES];
	//! Gain range list for slots in all frames
	byte aGainState[MAX_GSM_RX_SWEEP_FRAMES][GSM_RX_SWEEP_SLOT_PER_FRAME];
/**
Outputs: will be filled by QLIB_FTM_GSM_RX_FREQ_CAL_SWEEP()
*/
	//! Sweep result status
	ftm_gsm_rx_freq_cal_sweep_state_type  iStatus;
	//! RSSI measurements
	dword aRSSI[MAX_GSM_RX_SWEEP_FRAMES][GSM_RX_SWEEP_SLOT_PER_FRAME];
} FTM_GSM_Rx_Freq_Cal_Sweep_Request_Response;

#define FTM_TCXOMGR_CAL_NUM_SAMPLES   127       /* max number of samples to record                       */

/**
	XO Calibration to write calibration data to NV or not
*/
typedef enum
{
	FTM_TCXOMGR_CAL_RECORD_NONE = 0x0,                 /* do not record            */
	FTM_TCXOMGR_CAL_RECORD_RAM  = 0x1,                 /* record to RAM            */
	FTM_TCXOMGR_CAL_RECORD_NV   = 0x3                 /* record to RAM and NV     */
}ftm_tcxomgr_cal_record_enum_type;

typedef enum
{
  FTM_TCXOMGR_CAL_FT_QUAL_NOT_CAL,                   /* uncalibrated             */
  FTM_TCXOMGR_CAL_FT_QUAL_FAC1,                      /* 56 PPM mode              */
  FTM_TCXOMGR_CAL_FT_QUAL_FT1,                       /*  5 PPM mode              */
  FTM_TCXOMGR_CAL_FT_QUAL_FT2,                       /*  3 PPM mode              */
  FTM_TCXOMGR_CAL_FT_QUAL_FAILED	                   /* 56 PPM mode
                                                    (Fac Cal Failed, phone
                                                     should have failed in
                                                     Factory)                */
} ftm_tcxomgr_cal_ft_qual_enum_type;


/**
	XO Calibration return status state
*/
typedef enum
{
  FTM_TCXOMGR_CAL_STATE_NOT_CAL,                     /* intial uncalibrated state*/
  FTM_TCXOMGR_CAL_STATE_COARSE,                      /* Coarse tuning completed  */
  FTM_TCXOMGR_CAL_STATE_SAMP,                        /* Samples Captured         */
  FTM_TCXOMGR_CAL_STATE_C1C2C3,                      /* C1C2C3 determined        */
  FTM_TCXOMGR_CAL_STATE_FINE,                        /* Fine tuning completed    */
  FTM_TCXOMGR_CAL_STATE_COMPLETED,                   /* Fac Cal completed        */
  FTM_TCXOMGR_CAL_STATE_FAILED                      /* Fac Cal failed           */
} ftm_tcxomgr_cal_state_enum_type;

/**
	FTM calibration state enumeration
*/
typedef enum
{
	CAL_STATE_INACTIVE = 0,
    CAL_STATE_ACTIVE = 1
} ftm_cal_state_type;


typedef struct
{
  dword          xo;				/* 2^-10 C, XTAL temp       */
  dword          pmic;              /* 2^-10 C, PMIC temp       */
  dword          pa;                /* 2^-10 C, PA temp         */
} ftm_tcxomgr_temp_set_type;


typedef struct
{
  ftm_tcxomgr_temp_set_type      min;                /* min temps encountered in calibration */
  ftm_tcxomgr_temp_set_type      max;                /* max temps encountered in calibration */
} ftm_tcxomgr_cal_temp_range_type;

/**
	XO calibraiton fac data type
*/
typedef struct
{
  dword                      c3;                 /* units of 2^-32 ppm/C^3  */
  dword                      c2;                 /* units of 2^-28 ppm/C^2  */
  dword                      c1;                 /* units of 2^-24 ppm/C    */
  dword                      c0;                 /* units of 2^-12 ppm      */
  dword						 t0;                 /* Ref XO Temp term units of 2^-10 C       */
  dword						 t0p;                /* Ref PMIC Temp term units of 2^-10 C       */
  dword                      p;                  /* PMIC compensation factor*/
  byte						 ft_qual_ind;		 /* quality indicator of the coefficients       */
  unsigned char               xo_trim;           /* corresponding xo_trim val for coefficients   */
} ftm_tcxomgr_cal_fac_data_type;

/**
	XO calibration info type
*/
typedef struct
{
  unsigned char						init_xo_trim;       /* pre FacCal xo_trim val   */
  unsigned char						dc_xo_trim;         /* post Coarse DC xo_trim   */
  unsigned char                     fin_xo_trim;        /* post Fine DC xo_trim     */
  unsigned char                     final_xo_trim;      /* post FacCal xo_trim val  */
  dword								s_ram_cap_cnt;      /* num of SRAM captures     */
  dword								freq_est_calc_cnt;  /* num freq est calculations*/
  ftm_tcxomgr_temp_set_type			init_temp;          /* initial temp (XO,PMIC,PA)*/
  ftm_tcxomgr_temp_set_type			final_temp;         /* final temp (XO,PMIC,PA)  */
  ftm_tcxomgr_cal_temp_range_type	temp_range;        /* temp range encountered   */
} ftm_tcxomgr_cal_info_type;

/**
	XO calibration FT Sample type
*/
typedef struct
{
  ftm_tcxomgr_temp_set_type     xo_temp[ FTM_TCXOMGR_CAL_NUM_SAMPLES ];		/* 2^-10 C, XTAL temp       */
  dword							xo_freq[ FTM_TCXOMGR_CAL_NUM_SAMPLES ];		/* 2^-12 ppm, XO frequency  */
  dword							c1_est;										/* estimate for coef C1     */
  dword							num_samples;								/* num samples taken        */
  ftm_tcxomgr_cal_temp_range_type temp_range;								/* temp range encountered   */
} ftm_tcxomgr_cal_ft_sample_type;

/**
	FTM request packet for XO calibration
*/
typedef struct
{
	word  iReserved;			//<!' Reserved set to 0
	dword iOffset;				//<!' Frequency offset (Hz)
	dword iTemp_Span;			//<!' time span (ms)
	word  iMin_ft_samples;		//<!' min sample (3)
	word  iMax_ft_samples;		//<!' max sample (100)
	word  iSubstage_Time;		//<!' Substage time
	word  iPdm_value;			//<!' Tx PDM
	word iPA_range;				//<!' PA range
	word  iTimeOut;				//<!' timeout (ms)
	dword  iWrite_NV;			//<!' 0 to write NV
}	FTM_XO_Cal_Request;


/**
	FTM response packet for XO calibration
*/
typedef struct {
  byte cmd;											   //<' Command ID (75)
  byte subsys;										   //<' Subsystem (11)
  word mode;										   //<' Mode ID (20)
  word ftm_cmd;										   //<' FTM command id
  word req_len;										   //<' FTM request length (not used)
  word resp_len;									   //<' FTM response length (not used)
  word state;											//<!' State of coefficients
  ftm_tcxomgr_cal_fac_data_type   nom_coef;				//<!' nominal coefficients
  word reserved;
  ftm_tcxomgr_cal_fac_data_type   cal_coef;				//<!' calibrated coefficients
  word reserved1;
  ftm_tcxomgr_cal_info_type 	 tcxomgr_cal_info_pkt;	//<!' calibration information
} FTM_XO_Cal_Response;

typedef struct
{
	word  iReserved1;			//<!' Reserved set to 0
	dword iOffset;				//<!' Frequency offset (Hz)
	dword iTemp_Span;			//<!' time span (ms)
	word  iMin_ft_samples;		//<!' min sample (3)
	word  iMax_ft_samples;		//<!' max sample (100)
	word  iSubstage_Time;	  	//<!' Substage time
	word  iTimeOut;				//<!' timeout (ms)
	byte  iTechnology;		    //<!' Technology. 0 - 1x,1- WCDMA,2 - GSM. (enums are not bytes).
    byte  iReserved2;			//<!' Reserved set to 0
    word  iReserved3;			//<!' Reserved set to 0
	dword iRxFreq;				//<!' Rx Frequency in kHz.
	dword  iWrite_NV;			//<!' 0 to write NV
}	FTM_ENH_XO_Cal_Request;

/**
	FTM Tx waveform attribute type
*/
typedef enum
{
  CW_OFFSET_ZER0_KHZ = 0,
  CW_OFFSET_19_2_KHZ = 1
}ftm_tx_waveform_attrib_type;

/**
	FTM Tx waveform type
*/
typedef enum
{
  CDMA_WAVEFORM = 0,
  CW_WAVEFORM = 1
}ftm_tx_waveform_type;
/******************************************************************************
						FTM - MediaFLO
*******************************************************************************/
/**
	\brief Enumeration of MediaFLO Command IDs
*/
typedef enum
{
	_FTM_MF_GET_RSSI_CAL_POINT				= 400,	//!< MediaFLO - get Rssi Cal Point
	_FTM_MF_GET_RX_RSSI						= 401,	//!< MediaFLO - get Rssi value
	_FTM_MF_GET_AGC_STATE					= 402,	//!< MediaFLO - get AGC state
	_FTM_MF_WRITE_CAL_DATA					= 403,	//!< MediaFLO - write cal data to NV
	_FTM_MF_LNA_SELECT						= 406,	//!< MediaFLO - select LNA
	_FTM_MF_RF_SWITCH_CONFIG				= 407,	//!< MediaFLO - switch control pins
	_FTM_MF_SYNTH_LOCK						= 408,	//!< MediaFLO - synthesizer lock state
	_FTM_MF_SET_GAIN_STATE					= 411,	//!< MediaFLO - set gain state
	_FTM_MF_TUNE_PLL						= 413,	//!< MediaFLO - tune PLL
	_FTM_MF_GET_IM2							= 415,	//!< MediaFLO - perform IM2 cal
} FTM_MF_Cmd_Id_Enum;

typedef enum
{
	_FTM_UBM_MF_GET_RSSI_CAL_POINT			= 0,	//!< MediaFLO - get Rssi Cal Point
	_FTM_UBM_MF_GET_RSSI					= 1,	//!< MediaFLO - get Rssi value
	_FTM_UBM_MF_GET_AGC_STATE				= 2,	//!< MediaFLO - get AGC state
	_FTM_UBM_MF_WRITE_CAL_DATA_TO_NV		= 3,	//!< MediaFLO - write cal data to NV
	_FTM_UBM_MF_SET_RF_ON					= 4,	//!< MediaFLO - Turn on RF_ON
	_FTM_UBM_MF_RF_POWER_UP_RF				= 5,	//!< MediaFLO - Power Up RF
	_FTM_UBM_MF_LNA_SELECT					= 6,	//!< MediaFLO - LNA Selection
	_FTM_UBM_MF_RF_SET_ANT_TUNE				= 7,	//!< MediaFLO - Set Antenna Tune
	_FTM_UBM_MF_SYNTH_LOCK					= 8,	//!< MediaFLO - synthesizer lock state
	_FTM_UBM_MF_SET_LNA						= 9,	//!< MediaFLO - Enable Disable LNA
	_FTM_UBM_MF_TUNE_RF_IN_HZ				= 10,	//!< MediaFLO - Tune RF in Hz
	_FTM_UBM_MF_SET_GAIN_STATE				= 11,	//!< MediaFLO - set gain state
	_FTM_UBM_MF_TOGGLE_TCXO_ON				= 12,	//!< MediaFLO - Toggle TCXO On
	_FTM_UBM_MF_TUNE_PLL					= 13,	//!< MediaFLO - tune PLL
	_FTM_UBM_MF_LOAD_RSSI_CAL_POINT			= 14,	//!< MediaFLO - Load RSSI Cal Point
	_FTM_UBM_MF_SET_AGC_DEFAULT				= 16,	//!< MediaFLO - Set AGC default
	_FTM_UBM_MF_RF_INITIALIZE				= 17,	//!< MediaFLO - RF initialise
	_FTM_UBM_MF_LOAD_AGC_CAL_CHAN_LIST		= 18,	//!< MediaFLO - Load AGC Calibration Channel List
	_FTM_UBM_MF_POWER_DOWN_RF				= 19,	//!< MediaFLO - Power Down
	_FTM_UBM_MF_SET_DEMOD_CONFIG			= 20	//!< MediaFLO - set demodulation configuration
} FTM_UBM_MF_Cmd_Id_Enum;

typedef enum
{
	_FTM_MFLO_NS_SET_NS_MODE						= 0,   //!< MediaFLO NS - enable FLO demod
	_FTM_MFLO_NS_ACQUIRE_FLO_SYSTEM					= 1,   //!< MediaFLO NS - acquire FLO system
	_FTM_MFLO_NS_ACTIVATE_FLOW						= 2,   //!< MediaFLO NS - activate FLO
	_FTM_MFLO_NS_DEACTIVATE_FLOW					= 3,   //!< MediaFLO NS - deactivate FLO
	_FTM_MFLO_NS_GET_STREAM_INFO					= 4,   //!< MediaFLO NS - get stream info
	_FTM_MFLO_NS_GET_MLC_DYN_PARAMS					= 5,   //!< MediaFLO NS - get MLC dynamic params
	_FTM_MFLO_NS_GET_OIS_STATS						= 6,   //!< MediaFLO NS - get OIS demodulation statistics
	_FTM_MFLO_NS_RESET_MLC_PLP_STATS				= 7,   //!< MediaFLO NS - reset MLC PLP stats
	_FTM_MFLO_NS_RESET_ALL_PLP_STATS				= 8,   //!< MediaFLO NS - reset all PLP stats
	_FTM_MFLO_NS_GET_ALL_ACTIVE_FLOW_IDS			= 9,   //!< MediaFLO NS - get all active flow ids
	_FTM_MFLO_NS_GET_CONTROL_CHANNEL_RECORDS		= 10,  //!< MediaFLO NS - get control channel records
	_FTM_MFLO_NS_GET_AIS_CHANNEL					= 11,  //!< MediaFLO NS - get AIS channel
	_FTM_MFLO_NS_DEACTIVATE_ALL_FLOWS				= 12,  //!< MediaFLO NS - deactivate all flows
	_FTM_MFLO_NS_TUNE_FREQ_AND_BW					= 13,  //!< MediaFLO NS - tune freq and bw
	_FTM_MFLO_NS_GET_RSSI							= 14,  //!< MediaFLO NS - get RSSI
	_FTM_MFLO_NS_SET_FLO_SLEEP						= 15,  //!< MediaFLO NS - set FLO sleep
	_FTM_MFLO_NS_GET_OIS_STATE						= 16,  //!< MediaFLO NS - get OIS State
	_FTM_MFLO_NS_INITIALIZE_RF						= 17,  //!< MediaFLO NS - initialize NS mode
	_FTM_MFLO_NS_SET_CONFIG_OPTIONS					= 18,  //!< MediaFLO NS - set demodulation configuration
	_FTM_MFLO_NS_RESET_OIS_STATS					= 19,  //!< MediaFLO NS - reset the OIS statistics
	_FTM_MFLO_NS_GET_MLC_DYN_PARAMS_LOG				= 20,  //!< MediaFLO NS - get the MLC dynamic parameters
	_FTM_MFLO_NS_GET_FLOW_PLP_DYN_PARAMS			= 21,  //!< MediaFLO NS - get the PLP statistics based on a Flow ID
	_FTM_MFLO_NS_ENABLE_FTAP_PLP_DATA				= 22,  //!< MediaFLO NS - start/stop FTAP data packet records logging
	_FTM_MFLO_NS_ENABLE_FTAP_OIS_RECEPTION_MODE		= 23,  //!< MediaFLO NS - start/stop FTAP OIS reception test mode
	_FTM_MFLO_NS_ENABLE_FTAP_WICLIC_RECEPTION_MODE	= 24,  //!< MediaFLO NS - start/stop FTAP WICLIC reception test mode
	_FTM_MFLO_NS_SET_JAMMER_DETECT_MODE				= 25,  //!< MediaFLO NS - set Jammer Detection Mode
	_FTM_MFLO_NS_FREEZE_GAIN_STATE					= 26,  //!< MediaFLO NS - freeze the AGC gain state
	_FTM_MFLO_NS_ENABLE_AFC_OUTER_LOOP				= 27,  //!< MediaFLO NS - enable the AFC outer loop
	_FTM_MFLO_NS_DISABLE_AGC_GAIN_TRACKING			= 28,  //!< MediaFLO NS - disable AGC Gain Tracking
	_FTM_MFLO_NS_GET_JAMMER_DETECT_STATE			= 29   //!< MediaFLO NS - get the Jammer Detection State

} FTM_MF_NS_Cmd_Id_Enum;

typedef enum
{
	FTM_MFLO_US		= 0,
	FTM_MFLO_VHF	= 1,
	FTM_MFLO_UHF	= 2,
	FTM_MFLO_L_BAND = 3
} ftm_mflo_rf_mode;

typedef enum
{
	FTM_MFLO_5_MHZ_BW	= 5,
	FTM_MFLO_6_MHZ_BW	= 6,
	FTM_MFLO_7_MHZ_BW	= 7,
	FTM_MFLO_8_MHZ_BW	= 8
} ftm_mflo_bandwidth;

typedef enum
{
	FTM_MFLO_2K_FFT = 0,
	FTM_MFLO_4K_FFT = 1,
	FTM_MFLO_8K_FFT = 2
} ftm_mflo_fft_size;

typedef enum
{
	FTM_MFLO_1_16_CP_LENGTH = 0,
	FTM_MFLO_1_8_CP_LENGTH = 1,
	FTM_MFLO_3_16_CP_LENGTH = 2,
	FTM_MFLO_1_4_CP_LENGTH = 3
} ftm_mflo_cp_length;

// IC Mode definition for MBP1600/MBP1610
typedef enum
{
	FTM_MFLO_IC_DISABLED = 0,
	FTM_MFLO_IC_BYPASSED = 1,
	FTM_MFLO_IC_ENABLED = 2,
	FTM_MFLO_IC_DYNAMIC = 3
} ftm_mflo_ic_mode;

// IC Mode definition for MBP2600/MBP2700
typedef enum
{
	FTM_MFLO_IC_2_DISABLED = 0,
	FTM_MFLO_IC_2_ENABLED = 1,
	FTM_MFLO_IC_2_DYNAMIC = 3
} ftm_mflo_ic_mode_2;

typedef enum
{
	FTM_MFLO_DMTT_DISABLED = 0,
	FTM_MFLO_DMTT_LEGACY = 1,
	FTM_MFLO_DMTT_W_TDM2_RDSP_NOT_USED = 2,
	FTM_MFLO_DMTT_W_TDM2_RDSP_USED = 3,
	FTM_MFLO_DMTT_TPC_BASED = 4
} ftm_mflo_dmtt_mode;

typedef enum
{
	FTM_MFLO_NS_OUTER_CODE_NONE = 0,
	FTM_MFLO_NS_OUTER_CODE_RS_7_8 = 1,
	FTM_MFLO_NS_OUTER_CODE_RS_3_4 = 2,
	FTM_MFLO_NS_OUTER_CODE_RS_1_2 = 3,
	FTM_MFLO_NS_OUTER_CODE_REP_4 = 4
} ftm_mflo_mlc_outer_code;

typedef enum
{
	FTM_MFLO_NS_ENABLE_WOIS_ONLY = 0x0,
	FTM_MFLO_NS_ENABLE_LOIS_ONLY = 0x1,
	FTM_MFLO_NS_ENABLE_WOIS_AND_LOIS = 0x2,
	FTM_MFLO_NS_DISABLE_WOIS_AND_LOIS = 0x3
} ftm_mflo_ois_test_mode_type;

typedef struct
{
  dword  num_good_pre_rs;
  dword  num_erasure_pre_rs;
  dword  num_good_post_rs;
  dword  num_erasure_post_rs;
} ftm_mflo_stats_type;

//! Structure to define storage area for a mediaFLO NS FTM request/response messages
typedef struct
{
  dword flow_id;
  byte   binding;
  byte  mlc_id;
  byte  strm_id;
  byte  strm_uses_both_layers;
  byte  byte_interleave_enabled;
  byte  system;
  dword base_fft_addr;
  byte  trans_mode;
  byte  outer_code;
  byte  num_cbs_in_curr_sf;
  union
  {
    ftm_mflo_stats_type plp_stats[2];  /* 8 uint32s */
    dword gen_1_w[8];
  } ftm_mflo_var_data_1;
  union
  {
    ftm_mflo_stats_type cb_stats[2];   /* 8 uint32s */
    dword gen_2_w[8];
  } ftm_mflo_var_data_2;
  byte  strm1_bound;
  byte  strm2_bound;
  word strm_lens[2];
} ftm_mflo_type;

//! Structure to parse WIC and LIC packets
typedef struct
{
	int reserved;
	char WID;
	char LID;
}wid_lid_packet;

typedef enum
{
	MFLO_T_UHF1		= 0,	/* MBP2600 */
	MFLO_F_LBAND	= 1,	/* MBP2700 LBAND */
	MFLO_F_LNA1		= 2,	/* MBP2700 LNA1 */
	MFLO_F_LNA2		= 3,	/* MBP2700 LNA2 */
} ftm_mflo_rf_lna_type;

/******************************************************************************
						FTM - GPS
*******************************************************************************/

//! GPS FTM Packet version
#define GPS_FTM_PACKET_VERSION 0

/**
	GPS FTM commands
*/
typedef enum
{
	_FTM_GPS_SET_LINEARITY_MODE			= 0,	//!< ' This command sets the desired linearity mode
	_FTM_GPS_ENABLE_SS_DC_CORRECTION	= 1,	//!< ' This command enables or disables periodic DC correction, with 1ms period
	_FTM_GPS_INIT_FOR_IM2_CAL			= 2,	//!< ' This command prepares the GPS RFIC for the DC-based IM2 calibration
	_FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION= 3,	//!< ' This command triggers a single iteration of the DC-based IM2 calibration
    _FTM_GPS_GET_BP_MEAN_AND_AMPL		= 4,	//!< ' This command returns BP_Mean and BP_Ampl from the Aries GPS engine
	_FTM_GPS_GET_CN0					= 5		//!< ' This command returns the measured C/No ratio in tenths of dB
	} gps_ftm_cmd_code_enum_type;

/**
	Enumeration for QLIB_FTM_GPS_SET_LINEARITY_MODE() -> iLinearityMode field
*/
typedef enum
{
	FTM_GPS_SET_LINEARITY_MODE_LL		= 0x00,   //!< ' Low Linearity Mode
	FTM_GPS_SET_LINEARITY_MODE_HL		= 0x01,   //!< ' High Linearity Mode
	FTM_GPS_SET_LINEARITY_MODE_HL_AWS	= 0x02    //!< ' High Linearity Mode for AWS band
} ftm_gps_linearity_mode_enum_type;

/**
	Enumeration for QLIB_FTM_GPS_ENABLE_SS_DC_CORRECTION() -> iEnable_DC_correction field
*/
typedef enum
{
	FTM_GPS_DISABLE_SS_DC_CORRECTION		= 0x00,  //!< ' Disable periodic steady-state DC correction
    FTM_GPS_ENABLE_SS_DC_CORRECTION			= 0x01   //!< ' Enable periodic steady-state DC correction
} ftm_gps_enable_ss_dc_correction_enum_type;

/**
	Enumeration for QLIB_FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION() -> iI_Q_channel_id field
*/
typedef enum
{
	FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION_CHANNEL_I		= 0x00,  //!< ' Perform single IM2cal iteration on I-channel
    FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION_CHANNEL_Q		= 0x01   //!< ' Perform single IM2cal iteration on Q-channel
} ftm_gps_do_single_im2_channel_enum_type;

/**
	Enumeration for QLIB_FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION() -> iDC_cancellation_method field
*/
typedef enum
{
	FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION_METHOD_WITH_DC_DACS		= 0x00,  //!< ' Cancel DC component using DC DACs
    FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION_METHOD_WITH_IM2_DACS	= 0x01   //!< ' Cancel DC component using IM2 DACs
} ftm_gps_do_single_im2_cal_iteration_method_enum_type;

/**
	Enumeration for QLIB_FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION() -> iDAC_search_space field
*/
typedef enum
{
	FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION_FULL_SEARCH_SPACE		= 0x00,  //!< ' Full search space (applies to DAC code)
    FTM_GPS_DO_SINGLE_IM2_CAL_ITERATION_VICINITY_SEARCH_SPACE	= 0x01   //!< ' Vicinity search around previous optimim value (applies to DAC code)
} ftm_gps_do_single_im2_cal_iteration_search_space_enum_type;


/******************************************************************************
						Diag - UBM DVBH
*******************************************************************************/
/**
	\brief Enumeration of DVBH Command IDs
*/
typedef enum
{
	_DIAG_DVBH_UBM_L1_ACQ_CMD					=0,
	_DIAG_DVBH_UBM_L1_CFG_CTRL_PID_CMD			=1,
	_DIAG_DVBH_UBM_L1_CFG_DATA_PID_CMD			=2,
	_DIAG_DVBH_UBM_L1_CFG_ITEM					=5,
	_DIAG_DVBH_UBM_L1_ACQ_STATUS				=7,
	_DIAG_DVBH_UBM_L1_PRBS_ONE_BIT_CMD			=8,
	_DIAG_DVBH_UBM_L1_PRBS_ONE_BIT_STATUS		=9,
	_DIAG_DVBH_UBM_L1_POWERUP_CMD				=10,
	_DIAG_DVBH_UBM_L1_POWERDOWN_CMD				=11,
	_DIAG_DVBH_UBM_L1_WAKEUP_NOW_CMD			=12,
	_DIAG_DVBH_UBM_L1_ENABLE_LNA_UPDATE			=15,
	_DIAG_DVBH_UBM_L1_DISABLE_LNA_UPDATE		=16,
	_DIAG_DVBH_UBM_L1_PACKET_CNT_CMD			=17,
	_DIAG_DVBH_UBM_L1_CFG_JAMMER_CMD			=18,
	_DIAG_DVBH_UBM_L1_RSSI_STATUS				=19,
	_DIAG_DVBH_UBM_L1_CFG_POST_FFT_JAMMER_CMD	=23,
	_DIAG_DVBH_UBM_L1_USE_RSSI_CAL_CMD			=26,
	_DIAG_DVBH_UBM_L1_SET_LNA_CMD				=27,
	_DIAG_DVBH_UBM_L1_PREPARE_FOR_CW_CAL		=33,
	_DIAG_DVBH_UBM_L1_DISABLE_DSP_LOOPS_CMD		=35
} DIAG_DVBH_UBM_L1_Cmd_Id_Enum;

typedef enum
{
	UBM_L1_MODE_1		= 1,
	UBM_L1_MODE_2		= 2,
	UBM_L1_MODE_3		= 3,
	UBM_L1_MODE_ALL		= 255
} ubm_dvbh_mode;
typedef enum
{
	UBM_L1_GUARD_4		=0,
	UBM_L1_GUARD_8		=1,
	UBM_L1_GUARD_16		=2,
	UBM_L1_GUARD_32		=3,
	UBM_L1_GUARD_ALL	=255
} ubm_dvbh_guard;
typedef enum
{
	UBM_L1_BW_5			=5,
	UBM_L1_BW_6			=6,
	UBM_L1_BW_7			=7,
	UBM_L1_BW_8			=8,
} ubm_dvbh_bandwidth;
typedef enum
{
	UBM_L1_CFG_EBI2_WS				=0,				//!< EBI2 waitstates
	UBM_L1_CFG_USE_PLLS				=1,				//!< whether to use RUMI's PLLs or System clock
	UBM_L1_CFG_CTRL_PID_EXT_MODE	=2,				//!< whether to use MPE bank memory for Control PID
													//!< 0 -> Don’t use bank memory
													//!< 1 -> Use bank memory
	UBM_L1_CFG_CTRL_PID_EBI2_MODE	=3,				//!< whether to read control PIDs via EBI2
													//!< 0 -> use TSIF to read PIDs
													//!< 1 -> use EBI2 to read PIDs
	UBM_L1_CFG_PAGE_MODE			=4,				//!< whether to use page mode for EBI2 comms to MBP.
													//!< 0 -> Don’t use page mode
													//!< 1 -> Use page mode
	UBM_L1_CFG_USE_MODE2_CPCE		=5,				//!< whether to use channel estimation for mode 2
													//!< 0 -> do not use channel estimation
													//!< 1 -> use channel estimation
	UBM_L1_CFG_DEBUG				=6,				//!< whether to turn on CFG debugging messages
													//!< 0 -> Turn off CFG messages
													//!< 1 -> Turn on CFG messages
	UBM_L1_HANDLE_L3_ACK			=7,				//!< whether to handle acks intended for L3 by L1
													//!< 0 -> Handle L3 acks
													//!< 1 -> Do not handle L3 acks
	UBM_L1_CFG_USE_ACQ_TIMEOUT		=8,				//!< whether to use the acquisition timeout
													//!< 0 -> Do not use acq timeout
													//!< 1 -> Use acq timeout
	UBM_L1_CFG_SNOOZE				=9,				//!< whether to allow snoozing
													//!< 0 -> Disable snooze
													//!< 1 -> Enable snooze
	UBM_L1_CFG_SLEEP				=10,			//!< whether to allow sleeping
													//!< 0 -> Disable sleep
													//!<  1 -> Enable sleep
	UBM_L1_CFG_BW					=11,			//!<  Set the channel bandwidth.
													//!<  0 -> UBM_L1_BW_5_MHZ
													//!<  1 -> UBM_L1_BW_6_MHZ
													//!<  2 -> UBM_L1_BW_7_MHZ
													//!<  3 -> UBM_L1_BW_8_MHZ
	UBM_L1_RX_FRONT_WARMUP			=12,			//!<  whether to do rx front warmup
													//!<  0 -> Don’t use RX front end warmup
													//!<  1 -> Do RX front end warmup
	UBM_L1_CHANGE_TCXO_PDM			=13,			//!<  Whether to do TCXO corrections
													//!<  0 -> do not change TCXO PDM
													//!<  1 -> change TCXO PDM
	UBM_L1_MAP_ADDR_12_18			=14,			//!<  Whether to map A12-18 on debug bus  no matter whether page mode is used
													//!<  0 -> do not map A12-18
													//!<  1 -> do map A12 -18
	UBM_L1_FAP_LAP_CORR_MASK		=15,		    //!<  Bit mask to enable/disable FAP/LAP correction for particular mode/guard combinations.
													//!<  To enable FAP/LAP correction for mode i, guard j, set bit number (i-1)*4+j to 1.
													//!<  Otherwise set it to 0. Default is 0x100, for mode 3 guard 0 only (8k, 1/4 guard).
	UBM_L1_SIGNAL_MONITOR			=16,			//!<  Signal Monitor.  Currently not used.  Default value is 0.
	UBM_L1_CFG_INPUT_MODE			=17,			//!<  Input mode type.
													//!< 0 -> RF mode
													//!< 1 -> IQ mode
													//!< 2 -> Y1Y2 mode
	UBM_L1_CFG_RDSP_CLK_SRC			=18				//!<  RDSP clock source
													//!<  0 -> Internal clock source (by PLL).
													//!< 1 -> External clock source.
} ubm_dvbh_configuration_items;
/******************************************************************************
						FTM - WLAN
*******************************************************************************/

////////////////////
// First are the Philips definitions
////////////////////

/** Maximum message size */
#define FTM_WLAN_Philips_MAX_MSG_SIZE 1600


/** Philips "source" field value */
#define FTM_WLAN_Philips_SOURCE 0xFF


/** The FTM_WLAN_Philips_PCTI_READ_STATISTICS  response size, in byte */
#define FTM_WLAN_Philips_PCTI_READ_STATISTICS_RESPONSE_SIZE 128

/**
	These are the module types which will be passed to the firmware
	for each specific module selection from the host.

	Used for SET_MODULE.
*/
typedef enum
{
	/* Washington modules (not supported) */
	FTM_WLAN_Philips_M1		= 0x00,
	FTM_WLAN_Philips_M2		= 0x01,
	FTM_WLAN_Philips_M3		= 0x02,
	FTM_WLAN_Philips_M4		= 0x03,
	FTM_WLAN_Philips_MWAmax	= 0x0F, /* end of Washington range */
	/* Georgia modules */
	FTM_WLAN_Philips_M_BGW211	= 0x10
} FTM_WLAN_Philips_enModuleType;

/* *
	The data rate which will be passed from the host
	This gets transferred to the rates required in the firmware

	Used for TX_BURST
*/
typedef enum
{
	/* 11b rates backwards compatible with washington */
	FTM_WLAN_Philips_RATE_1_MBPS	= 0x0,
	FTM_WLAN_Philips_RATE_2_MBPS	= 0x1,
	FTM_WLAN_Philips_RATE_5_MBPS	= 0x2,
	FTM_WLAN_Philips_RATE_11_MBPS	= 0x3,
	/* plcp rate codes used for OFDM rates */
	FTM_WLAN_Philips_RATE_6_MBPS	= 11,
	FTM_WLAN_Philips_RATE_9_MBPS	= 15,
	FTM_WLAN_Philips_RATE_12_MBPS	= 10,
	FTM_WLAN_Philips_RATE_18_MBPS	= 14,
	FTM_WLAN_Philips_RATE_24_MBPS	= 9,
	FTM_WLAN_Philips_RATE_36_MBPS	= 13,
	FTM_WLAN_Philips_RATE_48_MBPS	= 8,
	FTM_WLAN_Philips_RATE_54_MBPS	= 12
} FTM_WLAN_Philips_enDataRate;

/**
	Sleep Test Option defines, used for SLEEP_MODE
*/
typedef enum
{
	FTM_WLAN_Philips_PCTI_SLEEP_DOWN = 0x0,
	FTM_WLAN_Philips_PCTI_TIMED_SLEEP = 0x1
} FTM_WLAN_Philips_enSleepMode;

/**
	These are the results which will be passed from the firmware
	for each specific command from the host
*/
typedef enum
{
	FTM_WLAN_Philips_PCTI_SUCCESS			= 0x00,
	FTM_WLAN_Philips_PCTI_FAILURE			= 0x01,
	FTM_WLAN_Philips_PCTI_NOT_SUPPORTED		= 0x02,
	/* Georgia (PCTI) specific */
	FTM_WLAN_Philips_PCTI_INVALID_PARAMETER	= 0x03,
	FTM_WLAN_Philips_PCTI_MISSING_PARAMETER	= 0x04,
	FTM_WLAN_Philips_PCTI_TEST_RUNNING		= 0x05
} FTM_WLAN_Philips_enPctiStatus;


/**
	This corresponds to the data specific part of TX_CONTINUOUS test.
*/
typedef enum
{
	FTM_WLAN_Philips_SINGLETONE_100KHZ		= 1,
	FTM_WLAN_Philips_SINGLETONE_3MHZ		= 2,
	FTM_WLAN_Philips_SINGLETONE_5d5MHZ		= 3,
	FTM_WLAN_Philips_DUALTONE_100_300KHZ	= 4,
	FTM_WLAN_Philips_DUALTONE_3_3d1MHZ		= 5,
	FTM_WLAN_Philips_DUALTONE_5d5_5d6MHZ	= 6,
	FTM_WLAN_Philips_QPSK_0_1_SEQUENCE		= 7,
	FTM_WLAN_Philips_QPSK_PN_SEQUENCE		= 8
} FTM_WLAN_Philips_enSignalType;

/**
	Specific request type used for passing Diagnostic request to the host
*/
typedef enum
{
	FTM_WLAN_Philips_PCTI_START					= 0x01, //!< 'same as FTM_WLAN_Philips_PCTI_CONNECT_DUT
	FTM_WLAN_Philips_PCTI_SLEEP					= 0x02, //!< 'not supported by PCTI
	FTM_WLAN_Philips_PCTI_TX_CONTINUOUS			= 0x03,
	FTM_WLAN_Philips_PCTI_TX_BURST				= 0x04,
	FTM_WLAN_Philips_PCTI_TX_POWER_CONTROL		= 0x05,
	FTM_WLAN_Philips_PCTI_RX_CONTINUOUS			= 0x06,
	FTM_WLAN_Philips_PCTI_REG_READ				= 0x08, //!< 'not supported by PCTI
	FTM_WLAN_Philips_PCTI_REG_WRITE				= 0x09, //!< 'not supported by PCTI
	FTM_WLAN_Philips_PCTI_EEPROM_READ			= 0x0A,
	FTM_WLAN_Philips_PCTI_EEPROM_WRITE			= 0x0B,
	FTM_WLAN_Philips_PCTI_SDIO_LOGIC			= 0x0C, //!< 'not supported by PCTI
	FTM_WLAN_Philips_PCTI_GET_MAC_ADDRESS		= 0x0D,
	FTM_WLAN_Philips_PCTI_STOP					= 0x12,
	FTM_WLAN_Philips_PCTI_SET_TX_DATA			= 0x13,
	FTM_WLAN_Philips_PCTI_THREE_WIRE_READ		= 0x14, //!< 'not supported by PCTI
	FTM_WLAN_Philips_PCTI_THREE_WIRE_WRITE		= 0x15, //!< 'not supported by PCTI
	FTM_WLAN_Philips_PCTI_SET_MODULE			= 0x16,
	FTM_WLAN_Philips_PCTI_DISCONNECT_DUT		= 0x17,
	/* Georgia (PCTI) specific */
	FTM_WLAN_Philips_PCTI_GET_HWSW_VERSIONS		= 0x20,
	FTM_WLAN_Philips_PCTI_TX_POWER_SET_CALIBRATED	= 0x21,
	FTM_WLAN_Philips_PCTI_MEMORY_READ			= 0x22,
	FTM_WLAN_Philips_PCTI_MEMORY_WRITE			= 0x23,
	FTM_WLAN_Philips_PCTI_SB_REGISTER_READ		= 0x24,
	FTM_WLAN_Philips_PCTI_SB_REGISTER_WRITE		= 0x25,
	FTM_WLAN_Philips_PCTI_RF_REGISTER_READ		= 0x26,
	FTM_WLAN_Philips_PCTI_RF_REGISTER_WRITE		= 0x27,
	FTM_WLAN_Philips_PCTI_COEX_LINE_READ		= 0x28,
	FTM_WLAN_Philips_PCTI_COEX_LINE_WRITE		= 0x29,
	FTM_WLAN_Philips_PCTI_CHECK_EXT_32KHZ_CLOCK = 0x2A,
	FTM_WLAN_Philips_PCTI_READ_STATISTICS		= 0x2B,
	FTM_WLAN_Philips_PCTI_RESET_STATISTICS		= 0x2C,
	FTM_WLAN_Philips_PCTI_SET_MAC_ADDRESS		= 0x2D,
	FTM_WLAN_Philips_PCTI_SLEEP_MODE			= 0x2E,
	FTM_WLAN_Philips_PCTI_GET_TEMPERATURE		= 0x2F,
	FTM_WLAN_Philips_PCTI_GET_CALIBRATIONINFO	= 0x30,
	FTM_WLAN_Philips_PCTI_SET_CALIBRATIONINFO	= 0x31,
	FTM_WLAN_Philips_PCTI_LOOPBACK_MODE			= 0x32,
	FTM_WLAN_Philips_PCTI_DEBUG_LEVEL			= 0x33
} FTM_WLAN_Philips__enPctiType;

/**
	Redefinition for Washington Compatibility
*/
#define FTM_WLAN_Philips_PCTI_CONNECT_DUT FTM_WLAN_Philips_PCTI_START


////////////////////
// Second are the Atheros definitions
////////////////////
/** Atheros: Continuous Transmit */
#define FTM_WLAN_Atheros_CMD_CONT_TRANSMIT	0x0

/** Atheros: Continuous Receive */
#define FTM_WLAN_Atheros_CMD_CONT_RECEIVE	0x1

/** Atheros: Force wake/sleep */
#define FTM_WLAN_Atheros_CMD_WAKE_SLEEP		0x2

/** Atheros: Continuous Receive Command Action*/

/** Accept all incoming frames */
#define FTM_WLAN_Atheros_CMD_CONT_RECEIVE_ACT_PROMISCUOUS_MODE	0x0
/** Accept frames accept only frames with dest address equal specified mac address */
#define FTM_WLAN_Atheros_CMD_CONT_RECEIVE_ACT_FILTER_MODE		0x1
/** Disable Rx and get last Rx report */
#define FTM_WLAN_Atheros_CMD_CONT_RECEIVE_ACT_OFF_MODE			0x2
/** Set MAC Address */
#define FTM_WLAN_Atheros_CMD_CONT_RECEIVE_ACT_SET_MAC_ADDRESS	0x3

typedef enum
{
	/* 11b rates backwards compatible with washington */
	FTM_WLAN_Atheros_RATE_1_MBPS	= 0,
	FTM_WLAN_Atheros_RATE_2_MBPS	= 1,
	FTM_WLAN_Atheros_RATE_5_MBPS	= 2,
	FTM_WLAN_Atheros_RATE_11_MBPS	= 3,
	/* plcp rate codes used for OFDM rates */
	FTM_WLAN_Atheros_RATE_6_MBPS	= 4,
	FTM_WLAN_Atheros_RATE_9_MBPS	= 5,
	FTM_WLAN_Atheros_RATE_12_MBPS	= 6,
	FTM_WLAN_Atheros_RATE_18_MBPS	= 7,
	FTM_WLAN_Atheros_RATE_24_MBPS	= 8,
	FTM_WLAN_Atheros_RATE_36_MBPS	= 9,
	FTM_WLAN_Atheros_RATE_48_MBPS	= 10,
	FTM_WLAN_Atheros_RATE_54_MBPS	= 11,
} FTM_WLAN_Atheros_enDataRate;

typedef enum
{
	FTM_WLAN_Atheros_CMD_SYNC = 0,
	FTM_WLAN_Atheros_CMD_ASYNC = 1
} FTM_WLAN_Atheros_cmdType;


/////////////////////////////
///   Below this are Qualcomm specific definitions
/////////////////////////////



/**
	The list of commands in the FTM_WLAN mode_id
*/
typedef enum
{
	_FTM_WLAN_USER_CMD		 = 0		//!<' Send a user command buffer to any WLAN module
} FTM_WLAN_CmdEnum;


/**
	Defines the types of modules supported, to be used for the eModuleType parameter
	for FTM_WLAN operations in QLIB.H, and for the FTM_WLAN_USER_CMD.

*/
typedef enum
{
	FTM_WLAN_ModuleType_Philips	= 0x00,	//!< Philips WLAN module
	FTM_WLAN_ModuleType_Atheros	= 0x01,	//!< Atheros WLAN module

} FTM_WLAN_ModuleTypeEnum;

//! Size of WLAN MAC Address
#define FTM_WLAN_MAC_ADRESS_SIZE 6

/******************************************************************************
						FTM - QFUSE
*******************************************************************************/
typedef enum
{
	FTM_QFUSE_Read	= 0x00,		//!< QFUSE read request
	FTM_QFUSE_Write	= 0x01,		//!< QFUSE write request
} FTM_QFUSE_CmdEnum;

#define FTM_QFUSE_Configuration_Chain 0

/******************************************************************************
						QMSL - Calibration Data Manager
*******************************************************************************/
/**
	Definition of NV Modes, which are used to specify which series of NV items will be written to.

	For example, older cdma2000 targets use the "NV_CDMA_" and "NV_PCS" prefixes while newer targets
	use "NV_BC0_" and "NV_BC1_" prefixes.

	These prefixes map to different nv_mode_id_type values.

	"NV_CDMA" is NV_MODE_CDMA_800 and "NV_BCO" is NV_MODE_BC0

  IMPORTANT: Maintain the order of these elements the same as the order of the elements in nv_mode_id in QMSL_NVDefinition.xsd
*/

typedef enum {

	NV_MODE_CDMA_800=5,
	NV_MODE_CDMA_1900=6,
	NV_MODE_WCDMA_IMT=9,
	NV_MODE_WCDMA_1900A=15,
	NV_MODE_WCDMA_1900B=16,
	NV_MODE_WCDMA_800=22,
	NV_MODE_WCDMA_800A=23,
	NV_MODE_WCDMA_1800=25,
	NV_MODE_WCDMA_BC4=28,
	NV_MODE_WCDMA_BC8=29,

	NV_MODE_BC0 = 100,
	NV_MODE_BC1 = 101,
	NV_MODE_BC2 = 102,
	NV_MODE_BC3 = 103,
	NV_MODE_BC4 = 104,
	NV_MODE_BC5 = 105,
	NV_MODE_BC6 = 106,
	NV_MODE_BC7 = 107,
	NV_MODE_BC8 = 108,
	NV_MODE_BC9 = 109,
	NV_MODE_BC10 = 110,
	NV_MODE_BC11 = 111,
	NV_MODE_BC14 = 114,
	NV_MODE_BC15 = 115,
  NV_MODE_BCX_BLOCK = 150,
  NV_MODE_BCX_BLOCK1 = 151,
	// GSM NV mode id
	NV_MODE_GSM_850		= 18,
	NV_MODE_GSM_EGSM	= 10,
	NV_MODE_GSM_DCS		= 11,
	NV_MODE_GSM_1900	= 12,

	NV_MODE_MAX=255

} nv_mode_id_type;


/**
	Tx Freq Compensation NV type

	All 1x MSM targets with RF chipsets other than RTR6500 use use QMSL_TX_COMP to map NVs to NV_<band class>_TX_COMP
	All UMTS MSM targets use QMSL_TX_COMP to map NVs to NV_<band class>_TX_COMP
	All 1x MSM targets with RF chipset RTR6500 use QMSL_TX_PWR_COMP to map NVs to NV_<band class>_TX_PWR_COMP

*/
typedef enum
{
	QMSL_NV_TX_COMP = 0,
	QMSL_NV_TX_PWR_COMP = 1
} nv_tx_comp_res_type;

/**
	Tx Freq Compensatiion NV type
*/

typedef enum
{
	NV_TX_COMP_PRIM_ONLY = 0,	  // Used by all targets
	NV_TX_COMP_PRIM_AND_SEC = 1   // Used by RTR6285.  RTR6285 requires 2 Tx freq compenstation values
} nv_tx_freq_comp_type;
/*
	Array size defintions for QMSL RF Calibration Data Manager
*/


//! Maximum number of cdma/wcdma number of Rx LNA states
#define QMSL_CDMA_RX_LNA_STATE_MAX 5

//! Maximum number of cdma/wcdma number of Rx power modes
#define QMSL_CDMA_MAX_RX_POWER_MODES 4

//! Maximum number of cdma/wcdma receivers
#define QMSL_CDMA_MAX_RECEIVERS 2

//! Maximum number of cdma/wcdma calibration channels
#define QMSL_CDMA_MAX_CHANNELS 16

//! Maximum number of PDM's available in any cdma/wcdma mode
#define QMSL_CDMA_MAX_PDM_STEPS 512

//! Maximum number of Tx Gain States available in any cdma/wcdma mode
#define QMSL_CDMA_MAX_TX_GAIN_STATES 4

//! Maximum number of RGI's available in any GSM mode
#define QMSL_GSM_MAX_RGI_STEPS 32

//! Maximum number of phase/amplitude samples for predistortion cal
#define QMSL_GSM_MAX_PREDIST_SAMPLES 64000

//! Number of PreDistortion Calibration AM/PM NV items per channel
#define QMSL_NV_MANAGER_PREDIST_CAL_TABLE_NUM			2

//! Maximum number of Tx Gain States available in any gsm mode
#define QMSL_GSM_MAX_TX_GAIN_STATES 4

//! Maximum number of Tx Gain States available in any gsm mode
#define QMSL_GSM_MAX_TX_CAL_CHANNELS 3

//! Maximum number of measurements for Tx Lim vs Freq, at each frequency, for any cdma/wcdma mode
#define QMSL_CDMA_MAX_TX_LIM_MEASUREMENTS 20

#define QMSL_CDMA_INTELLICEIVER_VALUE_ARRAY 128
/**
	Defintion of one band of cdma2000/WCDMA Rx measurement structure, for use with the
	QMSL RF Calibration Data manager functions
*/

//! NV item name length
#define QMSL_NV_MANAGER_NV_ITEM_NAME_LENGTH		50

//! Maximum number of DVGA/LNA offset vs freq NV item size
#define QMSL_NV_MANAGER_MAX_DVGA_LNA_OFFSET_VS_FREQ_SIZE	48

//! CDMA Tx Linearizer table slope size
#define QMSL_NV_MANAGER_CDMA_TX_LIN_TABLE_SLOPE_SIZE	36

//! CDMA Tx Linearizer table slope size
#define QMSL_NV_MANAGER_CDMA_TX_LIN_TABLE_SIZE	37

//! WCDMA Tx Linearizer table size
#define QMSL_NV_MANAGER_WCDMA_TX_LIN_TABLE_SIZE		37

//! WCDMA Tx Linearizer(Beta scaling) table size
#define QMSL_NV_MANAGER_WCDMA_BETA_SCALING_NV_SIZE	32

//! WCDMA Enhanced Tx Linearizer(Beta scaling) table size
#define QMSL_NV_MANAGER_WCDMA_ENH_BETA_SCALING_NV_SIZE	64

//! CDMA Tx Linearizer(Beta scaling) table max size
#define QMSL_NV_MANAGER_CDMA_BETA_SCALING_NV_SIZE	64

//! CDMA Tx Linearizer (RTR6500) table msx size
#define QMSL_NV_MANAGER_CDMA_TX_LIN_RTR6500_TABLE_SIZE 64

//! Maximum number of Polar cal raw data point
#define QMSL_NV_MANAGER_POLAR_CAL_RAW_DATA_MAX_SIZE 300

//! Polar calibration AMAM table size
#define QMSL_NV_MANAGER_POLAR_CAL_AMAM_TABLE_SIZE	64

//! Polar calibration AMAM table size
#define QMSL_NV_MANAGER_PREDISTORTION_CAL_AMAM_AMPM_TABLE_SIZE	64

//! Polar calibration AMPM table size
#define QMSL_NV_MANAGER_POLAR_CAL_AMPM_TABLE_SIZE	32

//! Number of Polar Calibration AM/PM NV items per channel
#define QMSL_NV_MANAGER_POLAR_CAL_TABLE_NUM			8

#define QMSL_NV_MANAGER_HDET_VS_AGC_NV_SIZE			16

#define QMSL_NV_MANAGER_GSM_IM2_CAL_MAX_SIZE		3
/**
	Defintion of CDMA Rx Calibration Measurment (DVGA and LNAs) for use with the
	QMSL RF Calibration Data manager functions
*/
typedef struct
{
	//! Storage of LNA Offset measurements
	short aiLNA_Measured[ QMSL_CDMA_MAX_CHANNELS ][ QMSL_CDMA_RX_LNA_STATE_MAX ][ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! Storage of DVGA Offset measurements
	short aiDVGA_Measured[ QMSL_CDMA_MAX_CHANNELS ] [ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! An array of LNA NV index measured.  For 1x targets, the lowest LNA NV index is 0.  For UMTS targets, the lowest LNA NV index is 1.
	unsigned short aiLNA_States[ QMSL_CDMA_RX_LNA_STATE_MAX ];

	//! Number of Rx LNA NV indices measured
	unsigned long iNumMeasured_LNA_States;

	//! Number of Rx power modes measured
	unsigned long iNumMeasuredPowerModes;

	//! Secondary or primary--enumeration (Primary == 0, Secondary == 1);
	unsigned short iPath;

} QMSL_CDMA_RxFreq_Measurement_struct;

/**
	Definition of NV item generated by QMSL RF Calibration Data manager
*/
typedef struct
{
	//! NV item enumeration
	short			iNVenum;

	//! NV item name
	char			sNVName[QMSL_NV_MANAGER_NV_ITEM_NAME_LENGTH];

	//! The NV byte array
	unsigned char	aNVItemData[DIAG_NV_ITEM_DATA_SIZE];

	//! Number of byte in the array
	unsigned char   iNumOfBytes;
} QMSL_RFCAL_NV_Item;

/**
	Definition of DVGA offset generated by QMSL RF Calibration Data manager
*/

typedef struct
{
	//! DVGA offset
	short			iDVGAOffset;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

} QMSL_CDMA_Rx_DVGA_NV_Result;

/**
	Definition of DVGA offset vs Freq generated by QMSL RF Calibration Data manager
*/

typedef struct
{
	//! DVGA offset vs Freq
	char	iDVGAOffsetVsFreq[QMSL_NV_MANAGER_MAX_DVGA_LNA_OFFSET_VS_FREQ_SIZE];

	//! Storage of DVGA Offset vs freq
	short	iDVGAOffsetVsFreqItem[ QMSL_CDMA_MAX_CHANNELS ] [ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}  QMSL_CDMA_Rx_DVGA_VS_FREQ_NV_Result;

/**
	Definition of LNA offset generated by QMSL RF Calibration Data manager
*/

typedef struct
{
	//! LNA offset
	short			iLNAOffset;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}  QMSL_CDMA_Rx_LNA_NV_Result;

/**
	Definition of LNA offset vs freq generated by QMSL RF Calibration Data manager
*/

typedef struct
{
	//! LNA offset vs freq
	char	iLNAOffsetVsFreq[QMSL_NV_MANAGER_MAX_DVGA_LNA_OFFSET_VS_FREQ_SIZE];

	//! LNA offset vs freq
	char 	iLNAOffsetVsFreqItem[ QMSL_CDMA_MAX_CHANNELS ][ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}  QMSL_CDMA_Rx_LNA_VS_FREQ_NV_Result;

/**
	Definition of Rx calibration (DVGA and LNA) results generated by QMSL RF Calibration Data manager
*/
typedef struct
{
	//! DVGA offset
	QMSL_CDMA_Rx_DVGA_NV_Result aDVGA;

	//! DVGA offset vs freq
	QMSL_CDMA_Rx_DVGA_VS_FREQ_NV_Result aDVGAvsFreq;

	//! LNA offset
	QMSL_CDMA_Rx_LNA_NV_Result aLNA[QMSL_CDMA_RX_LNA_STATE_MAX];

	//! LNA offset vs freq
	QMSL_CDMA_Rx_LNA_VS_FREQ_NV_Result aLNAvsFreq[QMSL_CDMA_RX_LNA_STATE_MAX];

	//! Number of LNA states generated by NV Manager
	int iNumOfLNAstates;
} QMSL_CDMA_Rx_DVGA_LNA_Cal_NV_Result;

/**
	Definition of CDMA Rx IM2 measurments for use with QMSL RF Calibration Data manager
*/
typedef struct
{

	//! Storage of IM2 - I values
	unsigned short aiIM2_I[ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! Storage of IM2 - Q values
	unsigned short aiIM2_Q[ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! Storage of IM2 - G (transconductance) values
	unsigned short aiIM2_G[ QMSL_CDMA_MAX_RX_POWER_MODES ];

	//! Number of Rx power modes measured
	unsigned long iNumMeasuredPowerModes;

	//! Secondary or primary--enumeration (Primary == 0, Secondary == 1);
	unsigned short iPath;

} QMSL_CDMA_RxIM2_Measurement_struct;


//! Maximum number of PDMs and Pwrs for DC DC PDM calibration
#define QMSL_GSM_MAX_PDM_PCL_INDEX 16


//	Definition of GSM Tx DC DC PDM Calibration results generated by QMSL RF Calbration Data Manager

typedef struct
{
	//! DC DC PDM CAL TABLE table value
	unsigned long iDC_DC_PDM_CAL_TBL[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! NV item generated
	QMSL_RFCAL_NV_Item oNVitem;

}QMSL_GSM_DC_DC_PDM_Cal_NV_struct;


/**
	Definition of GSM Tx DC DC PDM Calibration measurements for use by QMSL RF Calbration Data Manager
*/
typedef struct
{
	//! Storage for size of NV array
	int iNVcount;
	//! Storage for the RfMode
	int iRfMode;
	//! Storage for Number of measurements made
	int iNumMeas;
	//! Storage for Number of measurements used
	int iNumMeasUsed;
	//! Storage for the coefts of the Quadratic Curve fit daX^2 + dbX + dc
	double da,db,dc;
	//! Storage for the lowest possible PDM value to be written to the NV
	double dPdmAbsMin;
	//! Storage for lowest power measured to be used for curve fitting
	double dPwrCurveFitMin;
	//! Storage for highest power measured to be used for curve fitting
	double dPwrCurveFitMax;
	//! Storage for the measured Tx Power
	double aSmpsTxPwr[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the set PDM
	double aSmpsPDM[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the measurements used for curve fitting
	double aSmpsTxPwrUsed[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the PDM values used for curve fitting
	double aSmpsPDMUsed[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the Head Room values to be added to NV PDM values.
	double dHeadRoomList[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the NV generation Mask, a 0 indicates VBATT instead of SMPS power supply.
	double dMaskNVGeneration[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the PDM values generated from the curve fit equation for each of the PCL powers.
	double dPdmListCurveFit[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the Pwr values corresponding to each PCL value.
	double dPwrListNV[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Storage for the final NV generated
	double dPdmListNV[QMSL_GSM_MAX_PDM_PCL_INDEX];

}QMSL_GSM_DC_DC_PDM_Measurement_struct;

//	Definition of GSM Tx DC DC PDM Calibration results generated by QMSL RF Calbration Data Manager
typedef struct
{
	//! DC DC PDM CAL TABLE table value
	unsigned long iPADAC_DC_OFFSET[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! NV item generated
	QMSL_RFCAL_NV_Item oNVitem;

}QMSL_GSM_PA_DAC_Cal_NV_struct;


/**
	Definition of GSM Tx DC DC PDM Calibration measurements for use by QMSL RF Calbration Data Manager
*/
typedef struct
{
		//! Storage for size of NV array
	int iNVcount;
	//! Storage for the RfMode
	int iNumMeas;
	//! Storage for the coefts of the Quadratic Curve fit daX^2 + dbX + dc
	double da,db,dc;
	//! DC Offset Step size
	int iDCOffsetStep;
	//! Tx Power Tolerance
	double dTxPowerTolerance;
	//! Maximum Target Tx Power
	double dTargetTxPwrMax;
	//! Target Tx Power List
	double dTargetTxPwrs[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Set SMPS PDM
	int iSmpsPdms[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Target Tx Power List
	double dMeasTxPwr[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Power Levels to be used in curve fit
	double dPwrListNV[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Computed DC Offset from curve fit
	double dDCOffsetCurveFit[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Set DC Offset to be used for curve fitting
	double dDCOffsetSet[QMSL_GSM_MAX_PDM_PCL_INDEX];
	//! Set computed DC Offset generated for NV
	double dDCOffsetNV[QMSL_GSM_MAX_PDM_PCL_INDEX];

}QMSL_GSM_PA_DAC_Measurement_struct;

/**
	Definition of CDMA Rx IM2 results generated by QMSL RF Calibration Data manager
*/
typedef struct
{
	//! Number of valid NV items generated in aNVitem
	unsigned short iNumOfNVitemsGenerated;

	//! NV items generated
	QMSL_RFCAL_NV_Item aNVitem[5];

} QMSL_CDMA_RxIM2_NV_struct;

// Need to add jammer cal

/**
	Defintion of structure for one band of cdma2000/WCDMA tx linearizer measurement, for
	use with the QMSL RF Calibration Data manager functions
*/
typedef struct
{
	//! Storage for Tx power (dBm) for each gain range
	double aiTxLinPower[ QMSL_CDMA_MAX_TX_GAIN_STATES ][ QMSL_CDMA_MAX_PDM_STEPS ];

	//! Storage for Tx PDMs (AGC) for each gain range
	unsigned short aiTxLinPDM[ QMSL_CDMA_MAX_TX_GAIN_STATES ][ QMSL_CDMA_MAX_PDM_STEPS ];

	//! Storage for HDETs which to be used to generate AGC vs HDET table
	unsigned short aiTxHDET[ QMSL_CDMA_MAX_PDM_STEPS ];

	//! Number of Tx gain indices measured
	unsigned long iNumMeasuredTxGainStates;

	//! Number of power vs pdm values measured for each gain state
	unsigned short iNumTxPowerMeasurementsPerRange[ QMSL_CDMA_MAX_TX_GAIN_STATES ];

	//! Tx Measurement to  NV Index mapping. The value is mapped to the index of NV_TX_LIN_<index> item.
	//For example, for a 3 gain range design, an array value of [0, 1, 3] is mapped to NV_TX_LIN_0, NV_TX_LIN_1 and NV_TX_LIN_3
	//! aiTxLinPower[0] and aiTxLinPower[0] generate NV_TX_LIN_MASTER_0
	//! aiTxLinPower[1] and aiTxLinPower[1] generate NV_TX_LIN_MASTER_1
	//! aiTxLinPower[2] and aiTxLinPower[2] generate NV_TX_LIN_MASTER_3
	//! any values in aiTxLinPower[3] and aiTxLinPower[3] are ignored
	unsigned short aiTxMeasToNVMapping[ QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Tx Measurement index used to generate AGC vs HDET table
	//! For example, a value of 2 means that aiTxLinPower[2] and aiTxHDET are used to generate HDET vs AGC table
	unsigned short iTxMeasIndexUsedForHDET;

	//! Desired power level for HDET Offset calculation (usually 18dBm)
	double HDET_SpanPower_dBm;

	//! Desired power level for HDET Span calculation (usually 26dBm)
	double HDET_OffsetPower_dBm;

} QMSL_CDMA_TxLin_struct;

/**
	Defintion of structure for one band of cdma2000/WCDMA tx linearizer measurement, for
	use with the QMSL RF Calibration Data manager functions
*/
typedef struct
{
	//! Storage for Tx power (dBm) for each Tx gain state
	double aiTxLinPower[ QMSL_CDMA_MAX_TX_GAIN_STATES ][ QMSL_CDMA_MAX_PDM_STEPS ];

	//! Storage for Tx PDMs for each gain state
	unsigned short aiTxLinPDM[ QMSL_CDMA_MAX_TX_GAIN_STATES ][ QMSL_CDMA_MAX_PDM_STEPS ];

	//! Max power to be achieved for each gain state
	double adMaxPower[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Min power to be achieved for each gain state
	double adMinPower[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Extrapolate maximum power or not, 1 to extrapolate, 0 not to extrapolate
	unsigned char aiExtrapolatedMaxPower[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Extraploate minimum power or not, 1 to extrapolate, 0 not to extrapolate
	unsigned char aiExtrapolatedMinPower[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Storage for HDETs which to be used for generate HDET vs AGC
	unsigned short aiTxHDET[ QMSL_CDMA_MAX_PDM_STEPS ];

	//! Number of Tx gain indices used
	unsigned long iNumMeasuredTxGainStates;

	//! Number of power vs pdm values measured for each gain state
	unsigned short iNumTxPowerMeasurementsPerGainState[ QMSL_CDMA_MAX_TX_GAIN_STATES ];

	//! Tx Measurement to  NV Index mapping. The value is mapped to the index of NV_TX_LIN_<index> item.
	//! For example, for a 3 gain range design, an array value of [0, 1, 3] is mapped to NV_TX_LIN_0, NV_TX_LIN_1 and NV_TX_LIN_3
	//! aiTxLinPower[0] and aiTxLinPower[0] generate NV_TX_LIN_MASTER_0
	//! aiTxLinPower[1] and aiTxLinPower[1] generate NV_TX_LIN_MASTER_1
	//! aiTxLinPower[2] and aiTxLinPower[2] generate NV_TX_LIN_MASTER_3
	//!  any values in aiTxLinPower[3] and aiTxLinPower[3] are ignored
	unsigned short aiTxMeasToNVMapping[ QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! The number of NV items
	unsigned short iTotalNVItems;

	//! Tx Measurement index used to generate AGC vs HDET table
	//! For example, a value of 2 means that aiTxLinPower[2] and aiTxHDET are used to generate HDET vs AGC table
	unsigned short iTxMeasIndexUsedForHDET;

	//! Desired power level for HDET Offset calculation (usually 18dBm)
	double HDET_SpanPower_dBm;

	//! Desired power level for HDET Span calculation (usually 26dBm)
	double HDET_OffsetPower_dBm;

	//! Tx sweep step size for each gain state
	unsigned short aiTxSweepStepSize[ QMSL_CDMA_MAX_TX_GAIN_STATES ];

	//! The head room PDM for each gain state
	unsigned short aiHeadRoomPDMCountList [QMSL_CDMA_MAX_TX_GAIN_STATES ];

	//! The PDM step size used for extrapolation in each gain state
	unsigned short aiExtrapolation_PDM_StepSize [QMSL_CDMA_MAX_TX_GAIN_STATES ];

	//! The max Tx AGC used to generate the UMTS linearizer table.  For RTR6275, the value is 383.  For RTR6285, the value is 255
	//! it is not used in 1x
	unsigned short iUMTS_TxAGC_PDM_Max;

	//! The Tx AGC PDM value corresponding to minimum tx gain.  Used to limit NV PDM index values at min during extrapolation.
	unsigned short i_TxAGC_PDM_MinGain;

	//!  10 for 1x target i.e 1/10 dB per count
	//!  100 for RTR6500 target i.e 1/100 dB per count
	//!  12 for UMTS target i.e 1/12 dB per count
	int	 iNV_Resolution;

	//!  Boolean to specify if Linearizer 0 is enhanced or not.
	unsigned char iEnh_Linearizer_0_NV;

	//!  Boolean to specify if Non Monotonic Items have to be Skipped
	unsigned char iSkip_NonMonotonic_Items;

	//!  Boolean to specify if Lower powers are to be extrapolated (or not)
	unsigned char iDo_Not_Extrapolate_Lower_Powers;

   //!  Boolean to specify whether HDET_V2 is used
	unsigned char iUse_HDET_V2;

  //! Max limit for HDET;
  int iHDET_MaxLim;

	//!  Sort NV by PDM or Power (0 = Sort NV by Power), (1 = Sort NV by PDM)
	int	  iSortNVTxLin;

	//!  Sorting order (0 = descending, 1 = ascending)
	int   iSortingOrder;

} QMSL_CDMA_TxLin_BetaScaling_struct;

/**
	Definition of a CDMA Tx Linearizer table
*/
typedef struct
{
	//! Tx Linearizer master offset
	short			iMasterOffset;

	//! Tx Linearizer slope table
	unsigned char	aiSlope[QMSL_NV_MANAGER_CDMA_TX_LIN_TABLE_SLOPE_SIZE];

	//! Tx Linearizer power levels used to generate NV item
	double			adTxLinPowerLevel [QMSL_NV_MANAGER_CDMA_TX_LIN_TABLE_SLOPE_SIZE + 1];

	//! PDMs correponding to power levels in adTxLinPowerLevel
	unsigned short	aiPDM [QMSL_NV_MANAGER_CDMA_TX_LIN_TABLE_SLOPE_SIZE + 1];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

	//! Indicate that this item has valid data
	int				iHasValidData;
} QMSL_CDMA_TxLin_NV_struct;

/**
	Definition of a WCDMA Tx Linearizer table (3U platform)
*/
typedef struct
{
	//! PDMs correponding to power levels in adTxLinPowerLevel
	unsigned short	aiPDM[QMSL_NV_MANAGER_WCDMA_TX_LIN_TABLE_SIZE];

	//! Tx Linearizer power levels used to generate NV item
	double	adTxLinPowerLevel[QMSL_NV_MANAGER_WCDMA_TX_LIN_TABLE_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

	//! Indicate that this item has valid data
	int				iHasValidData;
} QMSL_WCDMA_TxLin_NV_struct;


/**
	Definition of a Tx Linearizer table(UMTS)
*/
typedef struct
{
	//! Tx Lin PDM table
	unsigned short	aiPDM[QMSL_NV_MANAGER_WCDMA_ENH_BETA_SCALING_NV_SIZE];

	//! The PDM table NV item
	QMSL_RFCAL_NV_Item oPDMNVItem;

	//! Tx Lin power table
	unsigned short aiPower[QMSL_NV_MANAGER_WCDMA_ENH_BETA_SCALING_NV_SIZE];

	//! The power table NV item
	QMSL_RFCAL_NV_Item oPowerNVItem;

	//! Indicate that this item has valid data
	int				iHasValidData;

} QMSL_WCDMA_TxLin_BetaScaling_NV_struct;

/**
	Definition of a Tx Linearizer table (1x)
*/
typedef struct
{
	//! Tx Lin PDM table
	unsigned short	aiPDM[QMSL_NV_MANAGER_CDMA_BETA_SCALING_NV_SIZE];

	//! The PDM table NV item
	QMSL_RFCAL_NV_Item oPDMNVItem;

	//! Tx Lin power table
	short aiPower[QMSL_NV_MANAGER_CDMA_BETA_SCALING_NV_SIZE];

	//! The power table NV item
	QMSL_RFCAL_NV_Item oPowerNVItem;

	//! Indicate that this item has valid data
	int				iHasValidData;

} QMSL_CDMA_TxLin_BetaScaling_NV_struct;

/**
	Definition of a Tx Linearizer table
*/

typedef struct
{
	//! Tx Lin table
	unsigned short	aiPDM[QMSL_NV_MANAGER_CDMA_TX_LIN_RTR6500_TABLE_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oPDMNVItem;

	short aiPower[QMSL_NV_MANAGER_CDMA_TX_LIN_RTR6500_TABLE_SIZE];

	QMSL_RFCAL_NV_Item oPowerNVItem;

	//! Indicate that this item has valid data
	int				iHasValidData;
} QMSL_CDMA_TxLin_RTR6500_NV_struct;


/**
	Definition of HDET offset generated by QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! HDET offset
	unsigned short	iHDET_offset;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}  QMSL_CDMA_HDET_OFFSET_NV_struct;

/**
	Definition of HDET span generated by QMSL RF Calibration Data Manager
*/

typedef struct
{
	//! HDET offset
	unsigned short	iHDET_span;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}  QMSL_CDMA_HDET_SPAN_NV_struct;


/**
	Definition of HDET vs AGC table generated by QMSL RF Calibration Data Manager
*/

typedef struct
{
	//! HDET vs AGC table
	unsigned short			aHDET_vs_AGC[QMSL_NV_MANAGER_HDET_VS_AGC_NV_SIZE];

	//! Tx Power Level(dBm) used to generate HDET vs AGC
	double					adTxPowerLevel[QMSL_NV_MANAGER_HDET_VS_AGC_NV_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

} QMSL_CDMA_HDET_VS_AGC_NV_struct;

/**
	Definition of Tx Linearizer, HDET spand and offset, HDET vs AGC tables generated by QMSL RF Calibration Data Manager
*/
typedef struct
{

	//! 1x target Tx Linearizer
	QMSL_CDMA_TxLin_NV_struct	aTx_GainState[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! HDET vs AGC
	QMSL_CDMA_HDET_VS_AGC_NV_struct aHDET_vs_AGC;

	//! HDET offset
	QMSL_CDMA_HDET_OFFSET_NV_struct	aHDET_offset;

	//! HDET span
	QMSL_CDMA_HDET_SPAN_NV_struct	aHDET_span;

	//! UMTS target Tx Linearizer
	QMSL_WCDMA_TxLin_NV_struct	aWCDMA_Tx_GainState[QMSL_CDMA_MAX_TX_GAIN_STATES];

} QMSL_CDMA_TxLin_Cal_Result_struct;

/**
	Definition of Tx Linearizer, HDET spand and offset, HDET vs AGC tables generated by QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! 1x Linearizer table
	QMSL_CDMA_TxLin_BetaScaling_NV_struct aTx_GainState[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! UMTS Linearizer table
	QMSL_WCDMA_TxLin_BetaScaling_NV_struct	aWCDMA_TxLin[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! HDET vs Tx AGC
	QMSL_CDMA_HDET_VS_AGC_NV_struct aHDET_vs_AGC;

	//! HDET offset
	QMSL_CDMA_HDET_OFFSET_NV_struct	aHDET_offset;

	//! HDET span
	QMSL_CDMA_HDET_SPAN_NV_struct	aHDET_span;

} QMSL_CDMA_TxLin_BetaScaling_Result_struct;


/**
	Definition of Tx Linearizer (RTR6500), HDET spand and offset, HDET vs AGC tables generated by QMSL RF Calibration Data Manager
*/
typedef struct
{

	//! Tx Linearizer (RTR6500)
	QMSL_CDMA_TxLin_RTR6500_NV_struct	aTx_GainState[QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! HDET vs AGC
	QMSL_CDMA_HDET_VS_AGC_NV_struct aHDET_vs_AGC;

	//! HDET offset
	QMSL_CDMA_HDET_OFFSET_NV_struct	aHDET_offset;

	//! HDET span
	QMSL_CDMA_HDET_SPAN_NV_struct	aHDET_span;


} QMSL_CDMA_TxLin_RTR6500_Result_struct;





/**
	Defintion of one band of cdma2000/WCDMA Tx Linearizer vs Frequency measurement structure,
	for use with the QMSL RF Calibration Data manager functions
*/
typedef struct
{
	//!Storage of Lin vs Freq measurement data
	double aiTxMeasuredPower[ QMSL_CDMA_MAX_TX_GAIN_STATES][  QMSL_CDMA_MAX_CHANNELS ];

	//! Tx Measurement to  NV Index mapping. The value is mapped to the index of NV_TX_LIN_<index> item.
	//For example, for a 3 gain range design, an array value of [0, 1, 3] is mapped to NV_TX_COMP_0, NV_TX_COMP_1 and NV_TX_COMP_3
	//! aiTxLinPower[0] and aiTxLinPower[0] generate NV_TX_COMP_0
	//! aiTxLinPower[1] and aiTxLinPower[1] generate NV_TX_COMP_1
	//! aiTxLinPower[2] and aiTxLinPower[2] generate NV_TX_COMP_3
	//! any values in aiTxLinPower[3] and aiTxLinPower[3] are ignored
	unsigned short aiTxMeasToNVMapping[ QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Number of Tx gain states measured
	unsigned short iNumMeasuredTxGainStates;

	//! 0 = primary Tx comp, 1 = both primary and secondary Tx comp.  See WCDMA Secondary Tx Comp vs Freq for RTR6285/RTR6280 (80-V4341-84)
	unsigned short iTxCompType;

	//! Sec freq comp may or may not be performed in all PA Ranges. This item is them populated to indicate which PA Ranges have measurements.
	//! Permitted values are 0's or 1s. 1 = means there are sec freq comp measurements for the corresponding PA Range. For example [0, 1]
	//! Must be populated when iTxCompType is 1. If no measurements are available, then provide 0.
	unsigned short a2ndTxCompHasMeas[ QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! The PDM value used to generate secondary Tx comp table.  Used by WCDMA Secondary Tx Comp vs Freq for RTR6285/RTR6280 (80-V4341-84)
	unsigned short a2ndTxCompPDM[ QMSL_CDMA_MAX_TX_GAIN_STATES ];

	//!Storage of sec_comp_meas.  Used by WCDMA Secondary Tx Comp vs Freq for RTR6285/RTR6280 (80-V4341-84)
	double aiTx_sec_comp_meas [QMSL_CDMA_MAX_TX_GAIN_STATES ] [ QMSL_CDMA_MAX_CHANNELS ];

	//!Storage of sec_comp_meas_next.  Used by WCDMA Secondary Tx Comp vs Freq for RTR6285/RTR6280 (80-V4341-84)
	double aiTx_sec_comp_meas_next [QMSL_CDMA_MAX_TX_GAIN_STATES ] [ QMSL_CDMA_MAX_CHANNELS ];


} QMSL_CDMA_TxLinVsFreq_struct;

/**
	Definition of Tx COMP table generaed by QMSL RF Calibraiton Data Manager
*/
typedef struct
{
	//! Tx Comp Value offset
	char aiTxCompValue[QMSL_CDMA_MAX_CHANNELS];

	//! Indicate that this item has valid data
	int	iHasValidData;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;


} QMSL_CDMA_Tx_LinVsFreq_NV_struct;


/**
	Definition of Tx COMP tables generaed by QMSL RF Calibraiton Data Manager
*/
typedef struct
{
	//! Array of Tx Frequency compensation result
	QMSL_CDMA_Tx_LinVsFreq_NV_struct	aTx_FreqComp [QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! Array of Tx 2nd Frequency compensation result
	QMSL_CDMA_Tx_LinVsFreq_NV_struct	aTx_2ndFreqComp [QMSL_CDMA_MAX_TX_GAIN_STATES];

	//! NV_<band>_TX_COMP_VS_FREQ_SEC_PDM_LIST
	QMSL_RFCAL_NV_Item					oTx_Comp_Vs_Freq_Sec_PDM_List;
} QMSL_CDMA_Tx_LinVsFreq_Cal_NV_Result_struct;


/**
	Definition of the reference channel Tx power vs HDET measurement data.  The ref channel data is
	used by LIM vs FREQ calculation.

	Note that the data needs to be sorted from high to lower power

	i.e.  aiTxMeasuredPower[0] = higest power, aiTxMeasuredHDET[0] = HDET at the highest power, aiTxAGC[0] = Tx AGC at the highest power
		  aiTxMeasuredPower[0] = 2nd higest power, aiTxMeasuredHDET[0] = HDET at the 2nd highest power, aiTxAGC[0] = Tx AGC at the 2nd highest power
*/
typedef struct
{
	//! Measured HDET value
	int aiTxMeasuredHDET [QMSL_CDMA_MAX_PDM_STEPS];

	//! Measured Power in dBm
	double aiTxMeasuredPower [QMSL_CDMA_MAX_PDM_STEPS];

	//! Tx AGC used for the measurement
	double aiTxAGC [QMSL_CDMA_MAX_PDM_STEPS];

	//! Number of measurmentt.  It should be less than the QMSL_CDMA_MAX_PDM_STEPS
	int iNumMeasurement;
} QMSL_CDMA_TxLimVsFreq_RefChan_Measurement_struct;

/**
	Definition of one band of cdma2000/WCDMA Tx Limit vs Frequency measurement structure,
	for use with the QMSL RF Calibration Data manager functions
*/
typedef struct
{
	//! Tx AGC (PDM) values used during LIM vs FREQ calibration
	short	aiTxMeasuredPDM[ QMSL_CDMA_MAX_CHANNELS][ QMSL_CDMA_MAX_TX_LIM_MEASUREMENTS ];

	//! Tx powers measured during LIM vs FREQ calibration
	double aiTxMeasuredPower[ QMSL_CDMA_MAX_CHANNELS ][ QMSL_CDMA_MAX_TX_LIM_MEASUREMENTS ];

	//! HDET values measured during LIM vs FREQ calibration
	unsigned int  aiTxMeasuredHDET[ QMSL_CDMA_MAX_CHANNELS][ QMSL_CDMA_MAX_TX_LIM_MEASUREMENTS ];

	//! Number of Tx measurement (Tx AGC, Power, HDET) at each channel
	int iNumOfTxLimMeasurement [QMSL_CDMA_MAX_CHANNELS];

	QMSL_CDMA_TxLimVsFreq_RefChan_Measurement_struct oRefChMeas;
} QMSL_CDMA_TxLimVsFreq_struct;

/**
	Definition of CDMA Tx LIM vs FREQ data generated by QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! LIM vs FREQ NV item
	char			aiLimVsFreq[QMSL_CDMA_MAX_CHANNELS];

	//! The Tx output power at reference HDET at each channel.  The reference HDET is HDET measured at Max power at reference channel
	double			adTxMaxPower[QMSL_CDMA_MAX_CHANNELS];

	//! The expected Tx PDM (AGC) at desired max power
	short			aiExpectedTxAGCatMaxPower[QMSL_CDMA_MAX_CHANNELS];

	//! The expected HDET at desired max power
	short			aiExpectedHDETatMaxPower[QMSL_CDMA_MAX_CHANNELS];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

} QMSL_CDMA_Tx_LimVsFreq_NV_struct;

/**
	Defintion of one band of cdma2000/WCDMA tx or rx channels structure,
	for use with the QMSL RF Calibration Data manager functions
*/
typedef struct
{
	//! Channel numbers used for measurements
	unsigned short aiMeasuredChannels[ QMSL_CDMA_MAX_CHANNELS ];

	//! Number of channels measured
	unsigned long iNumMeasuredChannels;

	//! Reference channel
	unsigned short iRefChannel;

} QMSL_CDMA_MeasuredChannels_struct;


/**
	Definition of the list of channels to genreate NV
*/
typedef struct
{
	//! Channel numbers used for generating NV
	unsigned short aiNVChannels[ QMSL_CDMA_MAX_CHANNELS ];

} QMSL_CDMA_NVChannels_struct;

/**
	Definition of Intellieceiver calibration item
*/
typedef struct
{
	//! The array to hold intelliceiver calibration value
	unsigned char aiIntelliceiverValue [QMSL_CDMA_INTELLICEIVER_VALUE_ARRAY];

	//! actual size of aiIntelliceiverValue
	int			  iSizeOfIntelliceiverValue;
} QMSL_CDMA_Intelliceiver_Value;

/**
	Definition of GSM Tx RSB calibration result
*/
typedef struct
{
	//! RSB calibration optimal alpha value
	unsigned char A_value;

	//! RSB calibration optimal beta value
	unsigned char B_value;
}  QMSL_GSM_Tx_RSB_struct;

//! Max number of GSM Rx LNA gain range calibration
#define QMSL_GSM_RX_GAIN_RANGE 5

//! Max number of channels used for GSM Rx LNA gain range calibration.
//! For GSM 850/900, the number of calibration channel is 8.
//! For GSM 1800(DCS)/1900(PCS), the number of calibration channel is 16.
#define QMSL_GSM_RX_GAIN_RANGE_CHANNELS 16

/**
	Definition of GSM gain range calibration measurement data for use with QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! A two dimensional array to hold GSM gain range calibraion value.  The first index is index of gain range.  The second index is index of channel
	unsigned long aiRSSIValue[QMSL_GSM_RX_GAIN_RANGE][QMSL_GSM_RX_GAIN_RANGE_CHANNELS];

	// The calibration power levels (in dBm)
	double aiCalibrationPwr[QMSL_GSM_RX_GAIN_RANGE];

	//! Number of valid gain range in aiRSSIValue
	unsigned long iNumRxGainRange;

	//! Number of calibration channel
	//! For GSM 850/900, the number of calibration channel is 8.
	//! For GSM 1800(DCS)/1900(PCS), the number of calibration channel is 16.
	unsigned long iNumChannel;

}  QMSL_GSM_Rx_Gain_Range_struct;

/**
	Definition of GSM gain range calibration NV results generated by QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! Number of valid gain range in aiRSSIValue
	unsigned long iNumRxGainRange;

	//! The RSSI Value
	unsigned short aiRSSI_NV_Value[QMSL_GSM_RX_GAIN_RANGE][QMSL_GSM_RX_GAIN_RANGE_CHANNELS];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem[QMSL_GSM_RX_GAIN_RANGE];

}  QMSL_GSM_Rx_Gain_Range_NV_struct;
/**
	Definition of GSM IM2 calibration results for use with QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! An array to hold the calibration channels
	unsigned short aiChannel[QMSL_NV_MANAGER_GSM_IM2_CAL_MAX_SIZE];

	//! An array to hold IM2 I values across the calibration channels
	unsigned char aIM2_I[QMSL_NV_MANAGER_GSM_IM2_CAL_MAX_SIZE];

	//! An array to hold IM2 Q values across the calibration channels
	unsigned char aIM2_Q[QMSL_NV_MANAGER_GSM_IM2_CAL_MAX_SIZE];

	//! IM2 Transconductance values across the calibration channels
	unsigned char aIM2_Transconductance;

}  QMSL_GSM_IM2_struct;

/**
	A struct to hold individual polar calibration sweep record
*/
typedef struct
{
	//! The DAC value
	long	dac;

	//! Duration count
	long	duration_counts;

	//! Amplitude in dBm
	double	amp_dBm;

	//! Phase in Radian
	double	phase_rad;
} QMSL_GSM_Polar_Cal_SweepRecord;


/**
	Definition of GSM Tx Polar Calibration channel
*/
typedef enum QMSL_GSM_Polar_Cal_Frequency
{
	//! NV items generated will be mapped to F1
	QMSL_POLAR_CAL_F1_CHANNEL = 1,

	//! NV items generated will be mapped to F2
	QMSL_POLAR_CAL_F2_CHANNEL = 2,

	//! NV items generated will be mapped to F3
	QMSL_POLAR_CAL_F3_CHANNEL = 3
}QMSL_GSM_Polar_Cal_Frequency;

/**
	Definition of GSM Tx Polar Calibration for use with QMSL RF Calibration Data Manager
*/
typedef struct
{
	QMSL_GSM_Polar_Cal_SweepRecord aPolar_Cal_Sweep_Result[QMSL_NV_MANAGER_POLAR_CAL_RAW_DATA_MAX_SIZE];
	//! Number of valid polar calibration sweep record in aPolar_Cal_Sweep_Result
	int iNumOfPoloarCalSweepRecord;

}QMSL_GSM_Polar_Cal_Measurement;

/**
	Definition of GSM Tx Polar Calibration for use with QMSL RF Calibration Data Manager
*/
typedef struct
{
	//! Polar calibration sweep record
	QMSL_GSM_Polar_Cal_SweepRecord aPolar_Cal_Sweep_Result[QMSL_NV_MANAGER_POLAR_CAL_RAW_DATA_MAX_SIZE];

	//! Number of valid polar calibration sweep record in aPolar_Cal_Sweep_Result
	int iNumOfPoloarCalSweepRecord;

	//! Smoothing Window Size in dB
	double	dSmoothingWindowSize;

	//! Calibration channel number
	unsigned short	iChannel;

	//! The generated NV items mapping.  Enumeration of QMSL_GSM_Polar_Cal_Frequency.
	QMSL_GSM_Polar_Cal_Frequency iFreqnecyLocation;

    //!	The maximum power (in dBm)
	double			dNVMaxPower;

	//! The minimum power (in dBm)
	double			dNVMinPower;

}QMSL_GSM_Polar_Cal_Result;


typedef struct
{
	//! Calibration channel number
	unsigned short	iChannel[ QMSL_GSM_MAX_TX_CAL_CHANNELS ];

	//! The generated NV items mapping.  Enumeration of QMSL_GSM_Polar_Cal_Frequency.
	QMSL_GSM_Polar_Cal_Frequency iFreqMapping[ QMSL_GSM_MAX_TX_CAL_CHANNELS ];

	//! Storage for Tx power (dBm) for each gain range
	double aiGSMTxPowerMeas[ QMSL_GSM_MAX_TX_CAL_CHANNELS ][ QMSL_GSM_MAX_TX_GAIN_STATES ][ QMSL_GSM_MAX_RGI_STEPS ];

	//! Storage for Tx power (dBm) for each gain range
	double aiEDGETxPowerMeas[ QMSL_GSM_MAX_TX_CAL_CHANNELS ][ QMSL_GSM_MAX_TX_GAIN_STATES ][ QMSL_GSM_MAX_RGI_STEPS ];

	//! Storage for Tx RGI for each gain range
	unsigned short aiGSMTxRGI[ QMSL_GSM_MAX_TX_CAL_CHANNELS ][ QMSL_GSM_MAX_TX_GAIN_STATES ][ QMSL_GSM_MAX_RGI_STEPS ];

	//! Storage for Tx RGI for each gain range
	unsigned short aiEDGETxRGI[ QMSL_GSM_MAX_TX_CAL_CHANNELS ][ QMSL_GSM_MAX_TX_GAIN_STATES ][ QMSL_GSM_MAX_RGI_STEPS ];

	//! Number of channels measured
	unsigned long iNumChannels;

	//! Number of Tx gain indices measured
	unsigned long iNumMeasuredTxGainStates;

	//! Number of power vs rgi values measured for each gain state
	unsigned short iNumTxPowerMeasurementsPerRange;

	//! Tx Measurement to  NV Index mapping. The value is mapped to the gain (G) index of RGI and PMEAN items
	unsigned short aiTxMeasToNVMapping[ QMSL_GSM_MAX_TX_GAIN_STATES];

}QMSL_GSM_DA_Cal_Result;


/**
	Definition of a Tx RGI vs PMEAS
*/

typedef struct
{
	//! Tx Lin PDM table
	unsigned char	aiRGI[QMSL_GSM_MAX_RGI_STEPS];

	//! The PDM table NV item
	QMSL_RFCAL_NV_Item oRGINVItem;

	//! Tx Lin power table
	short aiPower[QMSL_GSM_MAX_RGI_STEPS];

	//! The power table NV item
	QMSL_RFCAL_NV_Item oPowerNVItem;
} QMSL_GSM_RGI_PMeas_NV_struct;

typedef struct
{
	//! RGI PMEAS Struct
	QMSL_GSM_RGI_PMeas_NV_struct aEDGERgiPmeasNV[QMSL_GSM_MAX_TX_GAIN_STATES];

	//! RGI PMEAS Struct
	QMSL_GSM_RGI_PMeas_NV_struct aGSMRgiPmeasNV[QMSL_GSM_MAX_TX_GAIN_STATES];

	//! DA Cal Channel value
	unsigned short	iChannel;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oChanNVitem;

} QMSL_GSM_DA_Cal_NV_struct;

typedef struct
{
	//! RGI PMEAS Struct
	QMSL_GSM_DA_Cal_NV_struct aDaCalChanNV[QMSL_GSM_MAX_TX_CAL_CHANNELS];

	//! Indicate that this item has valid data
	int				iHasValidData;
} QMSL_GSM_DA_Cal_NV_Chan_struct;


/**
	Definition of GSM Tx Polar Calibration AM table generated by QMSL RF Calbration Data Manager
*/
typedef struct
{
	//! AMAM table value
	unsigned short	AMAM[QMSL_NV_MANAGER_POLAR_CAL_AMAM_TABLE_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;


}QMSL_GSM_Polar_Cal_AMAM_NV_struct;

/**
	Definition of GSM Tx Polar Calibration PM table generated by QMSL RF Calbration Data Manager
*/
typedef struct
{
	//! AMPM table value
	unsigned int	AMPM[QMSL_NV_MANAGER_POLAR_CAL_AMPM_TABLE_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;


}QMSL_GSM_Polar_Cal_AMPM_NV_struct;

/**
	Definition of GSM Tx Polar Calibration channel generated by QMSL RF Calbration Data Manager
*/
typedef struct
{
	//! AMPM table value
	unsigned short	iChannel;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}QMSL_GSM_Polar_Cal_AMAM_CalChannel_NV_struct;

/**
	Definition of GSM Tx Polar Calibration dynamic range generated by QMSL RF Calbration Data Manager
*/
typedef struct
{
	//! Max power
	int				iMaxPower;

	//! Min power
	int				iMinPower;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}QMSL_GSM_Polar_Cal_AMAM_DynamicRange_NV_struct;



/**
	Definition of GSM Tx Polar Calibration results generated by QMSL RF Calbration Data Manager
*/
typedef struct
{
	//! Polar calibration smoothed result
	QMSL_GSM_Polar_Cal_SweepRecord aPolar_Cal_Sweep_Smoothed_Result[QMSL_NV_MANAGER_POLAR_CAL_RAW_DATA_MAX_SIZE];

	//! Number of valid polar calibration sweep record in aPolar_Cal_Sweep_Smoothed_Result
	int iNumOfPoloarCalSweepRecord;

	//! Polar Calibration AMAM NV items table
	QMSL_GSM_Polar_Cal_AMAM_NV_struct aAMAM_NV[QMSL_NV_MANAGER_POLAR_CAL_TABLE_NUM];

	//! Polar Calibration AMPM NV items table
	QMSL_GSM_Polar_Cal_AMPM_NV_struct aAMPM_NV[QMSL_NV_MANAGER_POLAR_CAL_TABLE_NUM];

	//! Polar Calibration AMAM ARFCN NV items table
	QMSL_GSM_Polar_Cal_AMAM_CalChannel_NV_struct aCalChannel;

	//! Polar Calibration Dynamic range NV items table
	QMSL_GSM_Polar_Cal_AMAM_DynamicRange_NV_struct aDyanmicRange;
}QMSL_GSM_Polar_Cal_NV_struct;

/**
	Definition of GSM Tx Predistion calibrated data
*/
typedef struct
{
   //! The generated NV items mapping.  Enumeration of QMSL_GSM_Polar_Cal_Frequency.
   QMSL_GSM_Polar_Cal_Frequency iFreqMapping[ QMSL_GSM_MAX_TX_CAL_CHANNELS ];

   //! Phase samples of predistortion calibration waveform
   float adTxPhaseArr[ QMSL_GSM_MAX_TX_CAL_CHANNELS ][QMSL_GSM_MAX_PREDIST_SAMPLES];

   //! Amplitude samples of predistortion calibration waveform
   float adTxAmpl[ QMSL_GSM_MAX_TX_CAL_CHANNELS ][QMSL_GSM_MAX_PREDIST_SAMPLES];

   //! Number of channels measured
   unsigned long iNumChannels;

   //! Number of samples. = iDcSamples+iEdgeSamples+iNoiseSamples+ more noise samples.
   unsigned short iNumPredistortionWaveformSamples;

   //Cal RGI
   unsigned short iCalRgi;

   //! The sampling rate
   unsigned long iSamplingRateHz;

   //! Number of DC samples in the calibration waveform
   unsigned long iDcSamples;

   //! Number of EDGE samples in the waveform
   unsigned long iEdgeSamples;

   //! Number of samples corresponding to trigger delay :
   unsigned long iNoiseSamples;

   //! Cal EDGE Tx Gain Param (Range 30 - 256)
   unsigned short iEDGETxGainParam;

   //! Cal EDGE Tx Calibration Waveform Param (Range > 4096)
   unsigned short iEDGETxCalScale;
}QMSL_GSM_PreDist_Cal_Result;

/**
	Definition of GSM Tx Predistortion AMAM
*/
typedef struct
{
	//! AMAM table value
	unsigned short	AMAM[QMSL_NV_MANAGER_PREDISTORTION_CAL_AMAM_AMPM_TABLE_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;


}QMSL_GSM_Predistortion_Cal_AMAM_NV_struct;

/**
	Definition of GSM Tx Predistortion AMPM
*/
typedef struct
{
	//! AMPM table value
	short	AMPM[QMSL_NV_MANAGER_PREDISTORTION_CAL_AMAM_AMPM_TABLE_SIZE];

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;


}QMSL_GSM_Predistortion_Cal_AMPM_NV_struct;


/**
	Definition of GSM Tx Predistortion Calibration RGI
*/
typedef struct
{
	//! Max power
	int				iPredistRGI;

	//! The actual NV item
	QMSL_RFCAL_NV_Item oNVitem;

}QMSL_GSM_Predistortion_Cal_RGI_NV_struct;

/**
	Definition of GSM Tx Predistortion calibration NV
        Reusing Dynamic Range items from Polar Calibration
*/
typedef struct
{
	//! Predistortion Calibration AMAM NV items table
	QMSL_GSM_Predistortion_Cal_AMAM_NV_struct aAMAM_NV[QMSL_NV_MANAGER_PREDIST_CAL_TABLE_NUM];

	//! Predistortion Calibration AMPM NV items table
	QMSL_GSM_Predistortion_Cal_AMPM_NV_struct aAMPM_NV[QMSL_NV_MANAGER_PREDIST_CAL_TABLE_NUM];
}QMSL_GSM_PreDist_Cal_AMAM_AMPM_NV_struct;



typedef struct
{
	//! Predistortion Calibration AMAM NV items table
	QMSL_GSM_PreDist_Cal_AMAM_AMPM_NV_struct aAMAM_AMPM_NV[QMSL_GSM_MAX_TX_CAL_CHANNELS];

	//! Predistortion Calibration Dynamic range NV items table
	QMSL_GSM_Polar_Cal_AMAM_DynamicRange_NV_struct aDyanmicRange;

	//! Predistortion Calibration
	QMSL_GSM_Predistortion_Cal_RGI_NV_struct aCalRGI;

	//! Indicate that this item has valid data
	int				iHasValidData;
}QMSL_GSM_PreDist_Cal_NV_Chan_struct;


/**
	Definition of Internal Thermistor calibration measurements for use with QMSL RF Calibration Manager
*/
typedef struct
{
	//! The calibration temperature (typically room temperature)
	char iCalTemp;
	//! The HKADC reading during calibration
	char iHKADCAtCalTemp;
	//! The ENH Therm Value at Room Temp
	unsigned long iEnhThermAtCalTemp;
	//! The slope.  The typical value is - 1/3
	double dSlope;
	//! Minimum temperature
	char iMinTemp;
	//! Maximum temperature
	char iMaxTemp;
	//! Indicates whether or not to populate NV_THERM_BINS_I
	unsigned char bFillNVThermBins;
	//! Indicates whether or not to use two slopes
	unsigned char bUseTwoSlopes;
	//! Indicates whether or not to use two slopes
	unsigned char bUseEnhTherm;
	//! Min Temp Slope
	double dMinTempSlope;
	//! Max Temp Slope
	double dMaxTempSlope;
}QMSL_Internal_Thermistor_Cal_Meas_struct;

/**
	Definition of Internal Thermistor calibration NVs for use with QMSL RF Calibration Manager
*/
typedef struct
{
	//! ADC at min temperature
	//int iADC_Min_Temp;

	//! ADC at max temperature
	//int iADC_Max_Temp;

	//! NV_THERM
	QMSL_RFCAL_NV_Item oNV_Therm;

	//! NV_THERM_MIN
	QMSL_RFCAL_NV_Item oNV_Therm_Min;

	//! NV_THERM_MAX
	QMSL_RFCAL_NV_Item oNV_Therm_Max;

	//! NV_THERM_BINS
	QMSL_RFCAL_NV_Item oNV_Therm_Bins;

}QMSL_Internal_Thermistor_Cal_NV_struct;

/**
	Definition of GPS RFIC IM2 calibration measurements for use with QMSL RF Calibration Manager
*/
typedef struct
{
	//! IM2 calibration result for I-channel IM2 DAC
	unsigned short iGPS_IM2_I_DAC;
	//! IM2 calibration result for Q-channel IM2 DAC
	unsigned short iGPS_IM2_Q_DAC;

}QMSL_GPS_IM2_Cal_Results_struct;

/**
	Definition of GPS RFIC IM2 calibration NVs for use with QMSL RF Calibration Manager
*/
typedef struct
{
	//! NV_GPS_I_IM2_DAC
	QMSL_RFCAL_NV_Item oNV_GPS_IM2_I_DAC;
	//! NV_GPS_I_IM2_DAC
	QMSL_RFCAL_NV_Item oNV_GPS_IM2_Q_DAC;

}QMSL_GPS_IM2_Cal_NV_struct;

/**
	Definition of Rx IQMismatch calibration NVs for use with QMSL RF Calibration Manager
*/
typedef struct
{

	//! NV_MISCOMP_A
	QMSL_RFCAL_NV_Item oNV_Mis_Comp_A_Coeff;

	//! NV_MISCOMP_B
	QMSL_RFCAL_NV_Item oNV_Mis_Comp_B_Coeff;


}QMSL_RxIQMismatch_Cal_NV_struct;

/**
	Definition of Rx IQMismatch calibration measurements for use with QMSL RF Calibration Manager
*/
typedef struct
{

	//! Mismatch Compensation A
	short mismatch_value_A;

	//! Mismatch Compensation B
	short mismatch_value_B;

	//! Chain information
	unsigned short rx_path;


}QMSL_RxIQMismatch_Cal_Meas_struct;

typedef struct
{

	//!INO data [input], ino_data[0]=I0, ino_data[1]=Q0, ino_data[2]=I1, ino_data[3]=Q1, ... ino_data[25598]=I12799, ino_data[25599]=Q12799.  These I/Q data are collected via asychoronus FTM log
   char ino_data[25600];

	//!Coefficients [output](the real and imaginary data are returned via QLIB_RFCAL_Rx_ICI_Calibration_Algorithm), user doesn't need to fill it
	long coefficients[32];

	//! Chain information [input], rx_path = 0 for primary Rx, rx_path = 1 fo diversity path
	unsigned short rx_path;

   //! SNR (dB) value obtained as a result of performing ICI calibration.
   double snr;
}QMSL_Rx_ICI_Cal_Meas_struct;

typedef struct
{
	//An array of 17 NV items
	QMSL_RFCAL_NV_Item oNV_ICI_CAL_Coefficients[17];

}QMSL_Rx_ICI_Cal_NV_struct;

/**
	Enumeration for writing calibration channel
*/
typedef enum
{
	QMSL_TX_CAL_CHANNEL = 0x0,	  //!< 'Tx calibration channel
	QMSL_RX_CAL_CHANNEL = 0x1,	  //!< 'Rx Primary Path calibration channel
	QMSL_RX1_CAL_CHANNEL = 0x2	  //!< 'Rx Secondary Path calibration channel

}QMSL_RFCAL_Channel_Type;

/**
	NV write item status
*/
typedef struct
{
	//! Enumeration
	unsigned short iNVenum;

	//! Status
	nv_stat_enum_type iStatus;


}QMSL_RFCal_Write_Status;

#define QMSL_NV_MANAGER_WRITE_TO_PHONE_MAX_ITEM 1024

/**
	Structure to hold the NV write status from QLIB_RFCAL_NV_Manager_WriteToPhone
*/
typedef struct
{
	QMSL_RFCal_Write_Status aNVItemStatus[QMSL_NV_MANAGER_WRITE_TO_PHONE_MAX_ITEM];

	unsigned int iNumOfItemsInCache;

}QMSL_RFCal_WriteToPhone_Status;

/******************************************************************************
						QMSL - NV Tool
*******************************************************************************/
/**
	The enumeration for the "source" parameter in NV Tool call back function

*/
typedef enum
{
	NV_Tool_LoadNVsFromSource = 0,
	NV_Tool_LoadNVsFromQCN    = 1,
	NV_Tool_LoadNVsFromMobile = 2,
	NV_Tool_WriteNVsToSource  = 3,
	NV_Tool_WriteNVsToQCN	  = 4,
	NV_Tool_WriteNVsToMobile  = 5
}
QMSL_NVTool_CallBack_Source_Enum;

/**
	The enumeration for the "event" parameter in NV Tool call back function
*/
typedef enum
{
	NVTool_NV_DONE_S			= 0,		//!<' Request completed okay
    NVTool_NV_BUSY_S            = 1,		//!<' Request is queued
    NVTool_NV_BADCMD_S			= 2,		//!<' Unrecognizable command field
    NVTool_NV_FULL_S            = 3,		//!<' The NVM is full
    NVTool_NV_FAIL_S            = 4,		//!<' Command failed, reason other than NVM was full
    NVTool_NV_NOTACTIVE_S		= 5,		//!<' Variable was not active
    NVTool_NV_BADPARM_S		    = 6,		//!<' Bad parameter in command block
    NVTool_NV_READONLY_S		= 7,		//!<' Parameter is write-protected and thus read only
    NVTool_NV_BADTG_S			= 8,		//!<' Item not valid for Target
    NVTool_NV_NOMEM_S			= 9,		//!<' free memory exhausted
    NVTool_NV_NOTALLOC_S		= 10,		//!<' address is not a valid allocation
	NVTool_NV_UNKOWN_S			= 11,		//!<' Unknown response from NV_READ or NV_WRITE command
	NVTool_NV_Written_To_SRC    = 12,       //!<' NV Item appended to NV source XML
	NVTool_NV_Written_To_QCN    = 13,       //!<' NV Item appended to QCN
	NVTool_NV_Read_From_SRC_SUCCESS   = 14,       //!<' NV Item read from NV source file and added to NV tool
	NVTool_NV_Read_From_QCN_SUCCESS   = 15,       //!<' NV Item read from QCN file and added to NV tool
	NVTool_NV_Read_From_SRC_FAIL      = 16,       //!<' NV Item read from NV source file, but failed to add the item to NV tool
	NVTool_NV_Read_From_QCN_FAIL      = 17,       //!<' NV Item read from QCN file, but failed to add the item to NV tool
	NVTool_NV_Read_From_SRC_SUCCESS_FILTERED_OUT   = 18,       //!<' NV Item read from NV source file. It is NOT added to NV tool because it is filtered out
	NVTool_NV_Read_From_QCN_SUCCESS_FILTERED_OUT   = 19,       //!<' NV Item read from QCN file. It is not added to NV tool because it is filtered out

}QMSL_NVTool_CallBack_Event_Enum;

/**
	The result code returned by NV (QCN/XML/Mobile NV management) tool function
*/
typedef enum
{
		ERROR_FREE,								//!<' No error
		MSXML_FAILED_TO_INSTANTIATE,			//!<' Check MSMXL6 is installed on target machine
		MSXML_FAILED_TO_LOAD_SRC,				//!<' Check that source file exists and is well formatted
		MSXML_FAILED_TO_SAVE_SRC,				//!<' Check that target path exists
		MSXML_FAILED_TO_LOAD_DEF,				//!<' Check that definition file exists and is well formatted
		SOURCE_XML_VALIDATION_FAILED,           //!<' Failed to validate source xml
		DEF_XML_VALIDATION_FAILED,              //!<' Failed to validate definition xml
		FAILED_TO_OPEN_QCN,                     //!<' Check that QCN exists or path is valid.  QCN file may be opened by other application
		INDEX_OUT_OF_BOUND,						//!<' Check index
		NV_ITEM_NOT_DEFINED,	                //!<' The NV item doesn't have an definition
		NV_ITEM_NO_VALUE,						//!<' The NV item doesn't not have any values
		NV_ITEM_FAILED_TO_SET_VALUE,			//!<' Failed to set the item value
		NV_ITEM_FAILED_TO_GET_VALUE,			//!<' Failed to get the item value
		NV_ITEM_REDEFINED,						//!<' A NV Item has been defined by a previous load.  The NV item definition is overwritten
		FAILED_TO_OPEN_SRC,						//!<' Failed to open source file
		FAILED_TO_OPEN_QDF,						//!<' Failed to open a file for writing QDF
		FAILED_TO_CONNECT_TO_MOBILE,			//!<' Failed to read/write NVs from/to mobile.  Check phone connection
		FAILED_TO_OPEN_HTML,					//!<' Failed to open HTML file for write access
		FAILED_TO_OPEN_QCN_STORAGE,				//!<' Failed to open a QCN file because QMSL can't get a valid storage
		NVTool_UNKNOWN						    //!<' Unknown
}QMSL_NVTool_ResultCode_Enum;




/******************************************************************************
						FTM - Definitions for multiple FTM modes
*******************************************************************************/

/**

*/
typedef enum QMSL_WaitForMetric_Types
{
	QMSL_WaitForMetric_cdma2000_AGC_C0,		//!<' cdma2000 AGC C0
	QMSL_WaitForMetric_cdma2000_AGC_C1,		//!<' cdma2000 AGC C1
	QMSL_WaitForMetric_evdo2000_AGC_C0,		//!<' 1xEV-DO AGC C0
	QMSL_WaitForMetric_evdo2000_AGC_C1,		//!<' 1xEV-DO AGC C1
	QMSL_WaitForMetric_evdo2000_C2I_C0,		//!<' 1xEV-DO C/I C0
	QMSL_WaitForMetric_evdo2000_C2I_C1,		//!<' 1xEV-DO C/I C1
	QMSL_WaitForMetric_evdo2000_C2I_Both_C0_C1,		//!<' 1xEV-DO C/I, both C0 and C1

	QMSL_WaitForMetric_Max					//!<' Maximum number
} temp_QMSL_WaitForMetric_Types;


/******************************************************************************
						QMSL - Text logging flags
*******************************************************************************/

//	---------------------------------------------------------
//	defines for phone logging settings
//
//	---------------------------------------------------------

#define LOG_NOTHING		0x0000	//!<' log nothing

#define LOG_C_HIGH_LEVEL_START	0x0200	//!<' High level C function start, indicates the begining of a high level C function, which
										//!<' calls other low level C functions internal to the library

#define LOG_C_HIGH_LEVEL_STOP	0x4000	//!<' High level C function stop

#define LOG_IO			0x0001	//!<' data IO (data bytes)
#define LOG_FN			0x0002	//!<' function calls with parameters
#define LOG_RET			0x0004	//!<' function return data

#define LOG_INF			0x0008	//!<' general information (nice to know)--do not use this one, as
								//!<' this space needs to be reserved for async messages

#define LOG_ASYNC		0x0008	//!<' asynchronous messages

#define LOG_ERR			0x0010	//!<' critical error information

#define LOG_IO_AHDLC	0x0020	//!<' HDLC IO tracing (data bytes)
#define LOG_FN_AHDLC	0x0040	//!<' HDLC layer function calls
#define LOG_RET_AHDLC	0x0080	//!<' HDLC function return data
#define LOG_INF_AHDLC	0x0100	//!<' HDLC general information
#define LOG_ERR_AHDLC	LOG_INF_AHDLC	//!<' HDLC Error info merged with LOG_INF_AHDLC, to free up the log bit


#define LOG_IO_DEV		0x0400	//!<' device IO tracing (data bytes)
#define LOG_FN_DEV		0x0800	//!<' device layer function calls
#define LOG_RET_DEV		0x1000	//!<' device function return data
#define LOG_INF_DEV		0x2000	//!<' device general information
#define LOG_ERR_DEV		LOG_INF_DEV		//!<' device error information, merged with LOG_INF_DEV to free up the log bit

#define LOG_DEFAULT		(LOG_C_HIGH_LEVEL_START|LOG_C_HIGH_LEVEL_STOP|LOG_FN|LOG_IO|LOG_RET|LOG_ERR|LOG_ASYNC) //!<'  default settings

#define LOG_ALL			0xFFFF	//!<' everything

/**
	Enumeration of extended text logging categories, to be used with
	QLIB_ExtendedTextMessage_SetCategoryEnable(),

	QMSL Developer note:  when this list is updated, please also update these functions:

*/
typedef enum
{
	QMSL_ExtTextMsgCat_MediaFLO_Parse         = 0,  //!< ' MediaFLO Layer one log parsing
	QMSL_ExtTextMsgCat_cdma2000_RDA_Parse     = 1,  //!< ' cdma2000 RDA log parsing for SER/FER
	QMSL_ExtTextMsgCat_NV_Manager_Debug		  = 2,  //!< ' NV Manager Debug Message
	QMSL_ExtTextMsgCat_NV_Tool_Debug		  = 3   //!< ' NV Tool Debug Message
} QMSL_Extended_Text_Message_Category_Enum;


//! Maximum number of characters in the category name string, to be used with QLIB_ExtendedTextMessage_GetCategoryListItem()
#define QMSL_EXTENDED_TEXT_MESSAGE_CATEGORY_NAME_SIZE 30

/******************************************************************************
QMSL - SE BER Structs and Enums
*******************************************************************************/
/**
	Enum containing the list of Loopback types.
*/
typedef enum
{
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_WCMDA_RMC122K = 0,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_WCMDA_RMC64K = 1,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_GSM_C = 2,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_EGPRS_SRB_5_6 = 3,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_EGPRS_SRB_7_9 = 4,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_GSM_A = 5,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_GSM_B = 6,
	LOG_FTM2_FTM_LOG_SE_BER_LB_TYPE_MAX				//!< Size of list, not a valid log ID
} log_FTM2_SE_BER_Loopback_Type_enum;

/**
	Response packet for WCDMA RMC 12.2Mbps/64Mbps, GSM Loop C  Types
*/
typedef struct
{
	/**
		Frame Number that this BER information corresponds to (GSM TDMA Frame
	     Number for GSM and EGPRS/CFN for WCDMA operation)
	*/
	dword	frame_number;
	/**
		Slot number that this data block BER info corresponds to.  This will be
	    useful for multislot operation, in order to collect BER statistics independently on different timeslots.
	*/
	byte	slot_num;


	/**
		PN state for the data block.  This will indicate if the local PN generator is in
		sync or out of sync for this particular data block.
		Sync detection is evaluated on a data block by data block basis.
		\code
			0 ?PN state unlocked (out of sync)
			1 ?PN state locked (in sync)
		\endcode
	*/
	byte	pn_state;

	//! Number of bits in this data block.
	word	num_bits;

	//! Number of errors in this data block.
	word	num_errors;
}FTM_SE_BER_TYPE_I_RESPONSE_Struct;

/**
	Response packet for GSM Loop A/B  Types
*/
typedef struct
{
	/**
		Frame Number that this BER information corresponds to (GSM TDMA Frame Number
		for GSM and EGPRS/CFN for WCDMA operation)
	*/
	dword	frame_number;

	/**
	*/
	byte	slot_num;		//!< Slot number that this data block BER info corresponds to.  This will be useful
	//!< for multislot operation, in order to collect BER statistics independently on different timeslots.

	/**
		PN state for the data block.  This will indicate if the local PN generator is in
		sync or out of sync for this particular data block.
		Sync detection is evaluated on a data block by data block basis. 0 ?PN state unlocked (out of sync)/
		1 ?PN state locked (in sync)
	*/
	byte	pn_state;

	/**
		0 ?Bad Frame = FALSE and 1 ?Bad Frame = TRUE
	*/
	byte	bad_frame_indicator;

	/**
		Number of class 1a bits in this data block.  For full rate speech this
		will be set to 50.
	*/
	word num_class1a_bits;

	/**
		Number of errors in the class 1a bits.
	*/
	word num_class1a_errors;

	/**
		Number of class 1b bits in this data block.  For full rate speech this
		will be set to 132.
	*/
	word num_class1b_bits;

	/**
		Number of errors in the class 1b bits.
	*/
	word num_class1b_errors;

	/**
		Number of class 2 bits in this data block.  For full rate speech this
		will be set to 78.
	*/
	word num_class2_bits;

	/**
		Number of errors in the class 2 bits.
	*/
	word num_class2_errors;
}FTM_SE_BER_TYPE_II_RESPONSE_Struct;

//! LOG STRUCT for WCDMA RMC 12.2kbps/64kbps, GSM Loop C  Types
typedef struct
{
	word ftm_log_id;
	byte se_ber_log_type;
	byte num_payloads;
	/**
		In the START SE BER LOG REQUEST sent prior to the start measurement, the number of frames per log or
		num payloads is configured which is also returned in the response packet.
		\code
			8 -?Send a BER log packet every 8 data blocks processed.
			16 -?Send a BER log packet every 16 data blocks processed.
			32 -- Send a BER log packet every 32 data blocks processed.
			64 -- Send a BER log packet every 64 data blocks processed.
		\endcode
	*/
	FTM_SE_BER_TYPE_I_RESPONSE_Struct resp_struct[64];
} FTM_SE_BER_TYPE_I_LOG_Struct;

//! LOG STRUCT for GSM Loop A/B  Types
typedef struct
{
	word ftm_log_id;
	byte se_ber_log_type;
	byte num_payloads;
	/**
		In the START SE BER LOG REQUEST sent prior to the start measurement, the number of frames per log or num
		payloads is configured which is also returned in the response packet.
		\code
           8 -?Send a BER log packet every 8 data blocks processed.
           16 -?Send a BER log packet every 16 data blocks processed.
           32 -- Send a BER log packet every 32 data blocks processed.
           64 -- Send a BER log packet every 64 data blocks processed.
		\endcode
	*/
	FTM_SE_BER_TYPE_II_RESPONSE_Struct resp_struct[64];
} FTM_SE_BER_TYPE_II_LOG_Struct;

//!  Statistics structure to hold the BER and good frame ratios.
typedef struct
{
	double BER;          //!< BER for LB WCDMA RMC 12.2Mbps/64Mbps, GSM Loop C types.
	double BER_1a;       //!< BER for LB 1a bits of GSM Loop A/B Types.
	double BER_1b;       //!< BER for LB 1b bits of GSM Loop A/B Types.
	double BER_2;        //!< BER for LB 2 bits of GSM Loop A/B Types.
 	double RBER_1a;      //!< Residual BER for LB 1a bits of GSM Loop A/B Types.
	double RBER_1b;      //!< Residual BER for LB 1b bits of GSM Loop A/B Types.
	double RBER_2;       //!< Residual  BER for LB 2 bits of GSM Loop A/B Types.
	double frame_erasure_ratio;   //!< Frame erasure ratio for GSM Loop A/B types.
}SE_BER_Statistics;

//!<  Measurement struct that holds the current status for SE BER measurements.
typedef struct
{
	dword frames_received;   //!< Number of frames recieved.
	dword frames_valid;      //!< Number of valid frames, decided from the pnstate state of each frame.
	dword frames_invalid;    //!< Number of invalid frames, decided from the pnstate state of each frame.
	dword bad_frame_indicator_accum; //!<	 BFI from each frame is summed up
	dword bits_counted;      //!<Total number of bits in this data block, applicable for RMC 12.2K,64K,GSM Loop C types.
	dword errors_counted;    //!<Total number of errors, applicable for RMC 12.2K,64K,GSM Loop C types.
	dword bits_counted_1a;   //!<Total number of class 1a bits in this data block.  For full rate speech this will be
	                         //!< set to 50 for each frame, applicable for GSM A/B types.
	dword errors_counted_1a; //!<Total number of errors in the class 1a bits, applicable for GSM A/B types.
	/**
		Total number of class 1b bits in this data block.  For full rate speech this will be
	    set to 132 for each frame, applicable for GSM A/B types.
	*/
	dword bits_counted_1b;

	dword errors_counted_1b;  //!<Total number of errors in the class 1b bits, applicable for GSM A/B types.

	/**
		Total number of class 2 bits in this data block.  For full rate speech this will be
		set to 78 for each frame, applicable for GSM A/B types.
	*/
 	dword bits_counted_2;
	dword errors_counted_2;             //!<Total number of errors in the class 2 bits, applicable for GSM A/B types.
	/**
		Total number of residual class 1a bits in this data block.  For full rate speech this will be
		set to 50 for each frame, applicable for GSM A/B types.
	*/
	dword residual_bits_counted_1a;

	/**
		Total number of errors in the residual class 1a bits, applicable for GSM A/B types.
	*/
	dword residual_errors_counted_1a;

	/**
		Total number of residual class 1b bits in this data block.  For full rate speech this will be
		set to 132 for each frame, applicable for GSM A/B types.
	*/
	dword residual_bits_counted_1b;

	/**
		Total number of errors in the residual class 1b bits, applicable for GSM A/B types.
	*/
	dword residual_errors_counted_1b;

	/**
		Total number of residual class 2 bits in this data block.  For full rate speech this will be
		set to 78 for each frame, applicable for GSM A/B types.
	*/
 	dword residual_bits_counted_2;

	/**
		Total number of errors in the residual class 2 bits, applicable for GSM A/B types.
	*/
	dword residual_errors_counted_2;

	/**
		BER and good frame ratio struct.
	*/
	SE_BER_Statistics ber_stats;

	/**
		contains the loopback type information.
	*/
	log_FTM2_SE_BER_Loopback_Type_enum eSE_BER_Loopback_Type;
}SE_BER_Status;

/******************************************************************************
						QMSL - Time Out defintions
*******************************************************************************/

/**
	Enumeration of time-out values that can be set / retrieved using
	QLIB_ConfigureTimeOut() and QLIB_GetTimeOut()

*/
typedef enum
{

	// Timeouts for Diag_FTM area
	QMSL_Timeout_General,			//!< 'General communications time out, used for SendSync()  (4,000ms default)
	QMSL_Timeout_IsPhoneConnected,	//!< 'Timeout when connecting IsPhoneConnected() command (80ms default)
	QMSL_Timeout_Connect,			//!< 'Timeout for connecting to a phone the first time (200ms default)

	QMSL_Timeout_Nonsignaling,		//!< 'Non-signaling timeout (1,000ms default)

	QMSL_Timeout_GSDI,				//!< 'Timeout to be used for GSDI commands (6,000ms default)
	QMSL_Timeout_CNV,				//!< 'Timeout to be used for preparation of CNV (default 10,000ms)

	QMSL_Timeout_CDMA_FreqSweep,	//!< 'Timeout to be used for CDMA Tx Rx Frequency Sweep (default 10,000ms)

	QMSL_Timeout_WriteData,			//!< 'Timeout to be used for writing data to a communications device,
									//!< ' does not include read back of response (default 900ms)
									//!< ' this cannot be set in QPST mode.

	QMSL_Timeout_ReadData,			//!< 'Timeout to be used for reading data from a communications device,
									//!< ' this timeout is only for the low level read operation and does not
									//!< ' consider the call context, such as whether a synchronous command is
									//!< ' currently being executed.  Default is 500ms

	QMSL_Timeout_GSDI_Event,		//!< 'Timeout to be used when waiting for a GSDI Diag event
									//!< ' Default is 3000ms

	QMSL_Timeout_CGPS_Event,		//!< 'Timeout to be used when waiting for events relatd to CGPS functions
									//!< ' Default is 3000ms

	/*
		Below this are configurable delays for the Diag_FTM area
	*/

	/**
		Delay when switching from ONLINE-FTM mode.  This is needed
		for some targets because they do not process FTM commands
		immediately after going into FTM mode.  A recommended value
		is 200ms, but the default value is 0ms for backwards compatability.

		This is used in the function QLIB_DIAG_CONTROL_F()
	*/
	QMSL_Timeout_Delay_ONLINE_FTM,

	/**
		Delay when switching to ONLINE mode.  This is needed
		because it often takes some time for the AMSS to change modes.
		Default is 0ms

		This is used in the function QLIB_DIAG_CONTROL_F()
	*/
	QMSL_Timeout_Delay_ONLINE,

	/**
		Delay when switching to OFFLINE mode.  This is needed
		because it often takes some time for the AMSS to change modes, for example
		a power down registration must be done for some systems.
		Default is 3000ms

		This is used in the function QLIB_ChangeFTM_BootMode() and
		in QLIB_DIAG_CONTROL_F().
	*/
	QMSL_Timeout_Delay_OFFLINE,

	/**
		This delay will be used when the mobile enters a GSM RF mode
	*/
	QMSL_Timeout_Delay_GSM_RF_Mode,


	/**
		This delay will be used by QLIB_MFLO_GetPER_Phy() and QLIB_MFLO_GetPER_PhyMAC(),
		to set the delay between checking FLO status.  Default value is 100ms

		QLIB_MFLO_GetPER_Phy() uses this value to set the timeout when waiting for
		a status log.

		QLIB_MFLO_GetPER_PhyMAC() uses the value as the duration between the calls
		to poll the phone for status.

	 /**
		This delay will be used by QLIB_MFLO_GetPER_Phy() and QLIB_MFLO_GetPER_PhyMAC(),
		to set the delay between checking FLO status.  Default value is 100ms.

		QLIB_MFLO_GetPER_Phy() uses this value as the duration between calls to get the
		stats when the stats are not updating.

		QLIB_MFLO_GetPER_PhyMAC() uses the value as the duration between the calls
		to get the next log packet from the queue when the queue is empty.
	*/
	QMSL_Timeout_Delay_MediaFLO_StatusCheck,

	/**
		Timeout in ms for collecting async IQ log from ICI calibration

		This timeout is used by QLIB_FTM_GET_ICI_CAL_DATA()
	*/
	QMSL_Timeout_ICI_IQ_Data,
	/**
		Timeout in ms for waiting for BT HCI response

		This timeout is used by QLIB_FTM_BT_HCI_USER_CMD_WithEventResponse
	*/
	QMSL_Timeout_BT_HCI_Response,
	// Note to QMSL developers, new delays and timeouts should be added here, before the
	// the SW Download timouts


	// Timeout for software download
	/**
		Timeout for the duration of the next softare download action.
		For example, a download that should take 7 minutes, the timeout can be set to 8 minutes.
		Default will be 10 minutes ( 10 minutes * 60seconds/minute * 1000milliseconds/secon = 600000ms
	*/
	QMSL_Timeout_SoftwareDownloadActivity,

	QMSL_Timeout_ListSizeMax		//!< 'Not a timeout, just used to determine timeout list size

} QMSL_TimeOutType_Enum;

#pragma pack()	// Restore default packeting

#endif // !defined(_QLIB_DEFINES)
