/*=============================================================================
                        *Communication Mode*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-09-13  jianwen.he      Init first version

=============================================================================*/

#ifndef __PKT_H__
#define __PKT_H__

#include "stdafx.h"
#include <stdio.h>
#include "serialport.h"

/* Each packet sent or received in this download protocol begins with a
   one-byte command code chosen from the following list.  Each packet
   type is either received by the phone or transmitted by the phone, but
   not both. */

/*lint -e749  Some values in this local enum are not used in this program. */
/*lint -esym(751,cmd_code_type) and in fact this enum itself is not used. */
enum {
	CMD_WRITE  = 0x01,	 /* Write a block of data to memory (received)	  */
	CMD_ACK    = 0x02,	 /* Acknowledge receiving a packet	(transmitted) */
	CMD_NAK    = 0x03,	 /* Acknowledge a bad packet		(transmitted) */
	CMD_ERASE  = 0x04,	 /* Erase a block of memory 		(received)	  */
	CMD_GO	   = 0x05,	 /* Begin execution at an address	(received)	  */
	CMD_NOP    = 0x06,	 /* No operation, for debug 		(received)	  */
	CMD_PREQ   = 0x07,	 /* Request implementation info 	(received)	  */
	CMD_PARAMS = 0x08,	 /* Provide implementation info 	(transmitted) */
	CMD_DUMP   = 0x09,	 /* Debug: dump a block of memory	(received)	  */
	CMD_RESET  = 0x0A,	 /* Reset the phone 				(received)	  */
	CMD_UNLOCK = 0x0B,	 /* Unlock access to secured ops	(received)	  */
	CMD_VERREQ = 0x0C,	 /* Request software version info	(received)	  */
	CMD_VERRSP = 0x0D,	 /* Provide software version info	(transmitted) */
	CMD_PWROFF = 0x0E,	 /* Turn phone power off			(received)	  */
	CMD_WRITE_32BIT = 0x0F,  /* Write a block of data to 32-bit memory
						   address (received)						 */
	CMD_MEM_DEBUG_QUERY = 0x10, /* Memory debug query		(received)	  */
	CMD_MEM_DEBUG_INFO	= 0x11, /* Memory debug info		(transmitted) */
	CMD_MEM_READ_REQ	= 0x12, /* Memory read request		(received)	  */
	CMD_MEM_READ_RESP	= 0x13, /* Memory read response 	(transmitted) */
	CMD_DLOAD_SWITCH	= 0x3A,
	CMD_REQ_FLASH_TYPE  = 0xF4,
	CMD_RSP_FLASH_TYPE	= 0xF5,
	CMD_MODEL_ID_REQ	= 0xF6,
	CMD_MODEL_ID_RSP	= 0xF7,
	CMD_BOOT_VER_REQ	= 0xF8,
	CMD_BOOT_VER_RSP	= 0xF9,
    /* add by jie.li 2011-07-04 for MDM9200 dashboard partition flag.*/
    CMD_GET_DASHBOARD_FLAG_REQ = 0xA0,
    CMD_GET_DASHBOARD_FLAG_RSP = 0xA1,
    CMD_SET_DASHBOARD_FLAG_REQ = 0xA2,
    CMD_SET_DASHBOARD_FLAG_RSP = 0xA3
    /* End.*/
};

/* Add by jianwen.he 2010-06-25*/
/*1. Get model id (4 bytes)
Refer to CUST_MOB_MODEL_EXT in mobile.c
2. Get boot version (1 byte)
Currently return 1
Command definition:
CMD_MODEL_ID_REQ    = 0xF6,
CMD_MODEL_ID_RSP    = 0xF7,
CMD_BOOT_VER_REQ    = 0xF8,
CMD_BOOT_VER_RSP    = 0xF9,
*/

/*end*/
/* MAX (HDLC) packet size has 4 bytes more than MAX_CMD_BUFFER_LEN.
 * 2 bytes for CRC, 2 bytes for leading and ending 0x7E. */



typedef struct {
	int16		type;
	int16		length;
	bool		broken;
	uint8		buf[4*1024];
} cmd_buffer_s_type, rsp_buffer_s_type;

typedef struct
{
	int16		length; 				/* Length of packet so far */
	bool		broken; 				/* Set if the packet can't be built */
	uint8		buf[4*1024];	/* The packet under construction */
} pkt_buffer_s_type;

typedef enum {
	PKT_TYPE_NONE = 0,
	PKT_TYPE_HDLC,
	PKT_TYPE_DIAG,
	PKT_TYPE_AT,
    //add by minghui.zhang 2013-11-04
	PKT_TYPE_SAHARA,
    //end add
	PKT_TYPE_MAX = 0xFF,
}TPktTypeEnumType;

/* Command buffer type
*/
typedef enum {
	CMD_TYPE_DIAG = 0,
	CMD_TYPE_DLOAD,  // download prg, refer to service/dload
	CMD_TYPE_HOSTDL, // download data, refer to tools/hostdl
	CMD_TYPE_AT,
	CMD_TYPE_SCSI,
    //add by minghui.zhang 2013-11-04
	CMD_TYPE_SAHARA,
    //end add
	CMD_TYPE_LAST,
	//...
	CMD_TYPE_MAX = 0xFF,
} cmd_type_e_type;


#ifndef byte
typedef unsigned char byte;
#endif


#define PORT_TIMEOUT  5000			// timeout each operation default

/***************************************************************************/
//-----------------------------------------------------------------------------

class  CPacket
{
public:
	CPacket(TPktTypeEnumType type = PKT_TYPE_NONE);
	CPacket(const char* devpath);
	~CPacket(void);

public:
	/* used to close current port communication */
	TResult Uninit(void);

	/* provide service of sending commands */
	TResult Send(cmd_buffer_s_type* cmd_ptr, uint32* rlen);
	TResult Receive(rsp_buffer_s_type* rsp_ptr, uint32* rlen);
	TResult SetTimeout(unsigned int ms=0);

  /* used to get ready for port communication */
  TResult Init(unsigned short port);
	TResult SetTimeout(COMMTIMEOUTS to);

	uint32  GetTimeout(void);
	uint16  GetComPort(void);
	void    SetPacketType(TPktTypeEnumType type);
	char*   GetDevice();
  unsigned short GetPort() { return m_Port; };
    //add by minghui.zhang 2013-11-06
    TResult SendData(uint8 buf[], uint32 len);
    TResult SaharaDLReceive(rsp_buffer_s_type* rsp_ptr, uint32 rlen);

private:
	/* Internal use only, sending packets */
	TResult SendDIAGPkt(uint8* pbuf, uint32 len, uint32* rlen);
	TResult SendHDLCPkt(uint8* pbuf, uint32 len, uint32* rlen);
	TResult SendATPkt(uint8* pbuf, uint32 len, uint32* rlen);
	TResult SendSCSIPkt(uint8* pbuf, uint32 len, uint32* rlen);
    //add by minghui.zhang 2013-11-04
    TResult SendSAHARAPkt(unsigned char* pbuf, unsigned int len, unsigned int* rlen);
    //end add

	TResult RcvDIAGPkt(uint8 *buf, uint32* rlen);
	TResult RcvHDLCPkt(uint8 *buf, uint32* rlen);
	TResult RcvATPkt(uint8 *buf, uint32 len, uint32* rlen);
	TResult RcvSCSIPkt(uint8 *buf, uint32* rlen);
    //add by minghui.zhang 2013-11-04
    TResult RcvSAHARAPkt(unsigned char *buf, unsigned int len, unsigned int* rlen);
    //end add

	TResult SendPacket(pkt_buffer_s_type* pkt, uint32* rlen);

private:
        CSerialPort         m_SerialPort;
        pkt_buffer_s_type   m_Packet;
        TPktTypeEnumType    m_PktType;
        unsigned short      m_Port;
};

//-----------------------------------------------------------------------------

#endif //__PKT_H__


