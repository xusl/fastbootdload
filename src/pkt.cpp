#include "stdafx.h"
//#include "utils.h"
//#include "comdef.h"

#include "pkt.h"
#include "log.h"

#define MAX_PACKET_LEN 4*1024

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

//-----------------------------------------------------------------------------

/*-------------------------------------------------------------------------*/

/* Mask for CRC-16 polynomial:
**
**      x^16 + x^12 + x^5 + 1
**
** This is more commonly referred to as CCITT-16.
** Note:  the x^16 tap is left off, it's implicit.
*/
#define CRC_16_L_POLYNOMIAL     0x8408

/* Seed value for CRC register.  The all ones seed is part of CCITT-16, as
** well as allows detection of an entire data stream of zeroes.
*/
#define CRC_16_L_SEED           0xFFFF

/* Residual CRC value to compare against return value of a CRC_16_L_STEP().
** Use CRC_16_L_STEP() to calculate a 16 bit CRC, complement the result,
** and append it to the buffer.  When CRC_16_L_STEP() is applied to the
** unchanged entire buffer, and complemented, it returns CRC_16_L_OK.
** That is, it returns CRC_16_L_OK_NEG.
*/
#define CRC_16_L_OK             0x0F47
#define CRC_16_L_OK_NEG         0xF0B8

/*===========================================================================

MACRO CRC_16_L_STEP

DESCRIPTION
  This macro calculates one byte step of an LSB-first 16-bit CRC.
  It can be used to produce a CRC and to check a CRC.

PARAMETERS
  xx_crc  Current value of the CRC calculation, 16-bits
  xx_c    New byte to figure into the CRC, 8-bits

DEPENDENCIES
  None

RETURN VALUE
  The new CRC value, 16-bits.  If this macro is being used to check a
  CRC, and is run over a range of bytes, the return value will be equal
  to CRC_16_L_OK_NEG if the CRC checks correctly according to the DMSS
  Async Download Protocol Spec.

SIDE EFFECTS
  xx_crc is evaluated twice within this macro.

===========================================================================*/


/* CRC table for 16 bit CRC, with generator polynomial 0x8408,
** calculated 8 bits at a time, LSB first.  This table is used
** from a macro in sio.c.
*/
const uint16 crc_16_l_table[] = {
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
  0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
  0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
  0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
  0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
  0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
  0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
  0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
  0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
  0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
  0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
  0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
  0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
  0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
  0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
  0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
  0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
  0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
  0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
  0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
  0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
  0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#define CRC_16_L_STEP(xx_crc,xx_c) \
  (((xx_crc) >> 8) ^ crc_16_l_table[((xx_crc) ^ (xx_c)) & 0x00ff])

#define  ADD_CRC_TO_PACKET(pkt,val)         \
  add_byte_to_packet(&pkt, (const byte)(val & 0xFF));        /* low  byte */ \
  add_byte_to_packet(&pkt, (const byte)((val >> 8) & 0xFF))  /* high byte */

/*-------------------------------------------------------------------------*/

/* Async HDLC achieves data transparency at the byte level by using
   two special values. The first is a flag value which begins and ends
   every packet: */
#define  ASYNC_HDLC_FLAG      0x7e

/* The flag value might appear in the data.  If it does, it is sent as
   a two-byte sequence consisting of a special escape value followed by
   the flag value XORed with 0x20.  This gives a special meaning to the
   escape character, so if it appears in the data it is itself escaped
   in the same way. */
#define  ASYNC_HDLC_ESC       0x7d
#define  ASYNC_HDLC_ESC_MASK  0x20

/*-------------------------------------------------------------------------*/

/* For the received packets, we need the minimum length of a valid packet
   of that type (except where the packet is only a command code). Note,
   size is in bytes */

#define  WRITE_SIZ   8  /* Minimum size of the Write packet */
#define  ERASE_SIZ   9  /* Total size of the Erase packet   */
#define  GO_SIZ      7  /* Total size of the Go packet      */
#define  UNLOCK_SIZ  9  /* Total size of the unlock packet  */
#define  WRITE_32BIT_SIZ   9  /* Minimum size of the Write_32bit packet */

/*-------------------------------------------------------------------------*/

/* Other packets must be built up on the fly.  This data type and the
   associated macros support dynamic construction of a packet.  */

/* According to the protocol spec, a version string can be at most 20 bytes */
#define  MAX_VERSTRING_LEN    (20)

#define  ROOM_FOR_CRC   (4)         /* Allow this much room for the CRC,
                                       including worst-case expansion due
                                       to byte-stuffing */
#define  ROOM_FOR_FLAG  (1)         /* Allow this much room for trailing flag */
   /* Maximum size of a received packet. */

   /* Minimum size of a received packet. */
#define  MIN_PACKET_LEN    3        /* 2 bytes CRC + 1 byte command code */


/*===========================================================================

MACRO START_BUILDING_HDLC_PKT

DESCRIPTION
  This macro initializes the process of dynamically building a HDLC packet.

PARAMETERS
  pkt	  A pkt_buffer_s_type struct in which the packet will be built.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  pkt is evaluated twice within this macro.
  This macro is not an expression, nor is it a single statement.  It
  must be called with a trailing semicolon.

===========================================================================*/
#define  START_BUILDING_HDLC_PKT(pkt)					\
			{										    \
				(pkt)->length = 1;					    \
				(pkt)->broken = false;				    \
				(pkt)->buf[0] = ASYNC_HDLC_FLAG;	    \
			}

#define START_BUILDING_DIAG_PKT(pkt)					\
			{											\
				(pkt)->length = 0;						\
				(pkt)->broken = false;					\
			}

/* Nothing to do, just to keep the code pretty */
#define START_BUILDING_AT_PKT(pkt)						\
			{											\
				(pkt)->length = 0;						\
				(pkt)->broken = false;					\
			}

#define FINISH_BUILDING_HDLC_PKT(pkt)  finish_building_hdlc_packet(pkt)

#define FINISH_BUILDING_DIAG_PKT(pkt)  finish_building_diag_packet(pkt)

#define FINISH_BUILDING_AT_PKT(pkt)    finish_building_at_packet(pkt)

/*===========================================================================

FUNCTION add_byte_to_packet

DESCRIPTION
  This function adds a single byte to a packet that is being built
  dynamically.	It takes care of byte stuffing and checks for buffer
  overflow.

  This function is a helper function for the packet building macros
  and should not be called directly.

DEPENDENCIES
  The START_BUILDING_PACKET() macro should have been called on the
  packet buffer before calling this function.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
static void add_byte_to_packet(pkt_buffer_s_type *pkt, const uint8 val)
{
	if (pkt->broken != false)	/* If the packet is broken already, */
		return; 				 /* Don't do anything. */

	/* Check if the byte needs escaping for transparency. */
	if (val == ASYNC_HDLC_FLAG || val == ASYNC_HDLC_ESC) {
		/* Check for an impending overflow. */
		if (pkt->length+2+ROOM_FOR_CRC+ROOM_FOR_FLAG >= MAX_PACKET_LEN) {
			pkt->broken = true; 	/* Overflow.  Mark this packet broken. */
			return;
		}

		/* No overflow.  Escape the byte into the buffer. */
		pkt->buf[pkt->length++] = ASYNC_HDLC_ESC;
		pkt->buf[pkt->length++] = val ^ (byte)ASYNC_HDLC_ESC_MASK;
	} else {	/* Byte doesn't need escaping. */
		/* Check for an impending overflow. */
		if (pkt->length+1+ROOM_FOR_CRC+ROOM_FOR_FLAG >= MAX_PACKET_LEN) {
			pkt->broken = true; 	/* Overflow.  Mark this packet broken. */
			return;
		}

		/* No overflow.  Place the byte into the buffer. */
		pkt->buf[pkt->length++] = val;
	}

}/* add_byte_to_packet() */



/*===========================================================================

FUNCTION finish_building_hdlc_packet

DESCRIPTION
  This function completes the process of building a packet dynamically.
  If all is well, it adds the CRC and a trailing flag to the buffer.
  If an error has been encountered in building the packet, it substitutes
  a NAK packet for whatever has been built.

  This function is a helper function for the packet building macros
  and should not be called directly.

DEPENDENCIES
  The START_BUILDING_PACKET() macro should have been called on the
  packet buffer before calling this function.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/

static void finish_building_hdlc_packet(pkt_buffer_s_type	*pkt)
{
	word  crc;
	/* Cyclic Redundancy Check for the packet we've built. */

	word  i;
	/* Index for scanning through the packet, computing the CRC. */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	if (pkt->broken == false) {
		/* Compute the CRC for all the bytes in the packet. */
		crc = CRC_16_L_SEED;
		for (i=1; i < pkt->length; i++) {
			/* According to the DMSS Download Protocol ICD, the CRC should only
			* be run over the raw data, not the escaped data, so since we
			* escaped the data as we built it, we have to back out any escapes
			* and uncomplement the escaped value back to its original value */
			if (pkt->buf[i] != ASYNC_HDLC_ESC) {
				crc = CRC_16_L_STEP(crc, (word) pkt->buf[i]);
			} else {
				i++;
				crc = CRC_16_L_STEP(crc,
				(word)(pkt->buf[i] ^ (byte)ASYNC_HDLC_ESC_MASK));
			}
		}
		crc ^= CRC_16_L_SEED;

		ADD_CRC_TO_PACKET(*pkt,crc);			 /* Add the CRC to the packet */

		pkt->buf[pkt->length++] = ASYNC_HDLC_FLAG;	/* Add a flag to the packet.*/
											  /* This can't use the regular
												 add_byte_to_packet() function
												 because it's a flag. */
	}
}/* finish_building_hdlc_packet() */


/*===========================================================================

FUNCTION finish_building_diag_packet

DESCRIPTION
  This function completes the process of building a diag packet dynamically.
  If all is well, it adds the CRC and a trailing flag to the buffer.
  If an error has been encountered in building the packet, it substitutes
  a NAK packet for whatever has been built.

  This function is a helper function for the packet building macros
  and should not be called directly.

DEPENDENCIES
  The START_BUILDING_PACKET() macro should have been called on the
  packet buffer before calling this function.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/

static void finish_building_diag_packet
(
  pkt_buffer_s_type  *pkt
	/* Structure containing the packet being built */
)
{
	/* Cyclic Redundancy Check for the packet we've built. */
	word	crc;

	/* Index for scanning through the packet, computing the CRC. */
	word	i;

	if (pkt->broken == false) {
	/* Compute the CRC for all the bytes in the packet. */
	crc = CRC_16_L_SEED;
	for (i=0; i < pkt->length; i++) {
	   /* According to the DMSS Download Protocol ICD, the CRC should only
		* be run over the raw data, not the escaped data, so since we
		* escaped the data as we built it, we have to back out any escapes
		* and uncomplement the escaped value back to its original value */
		if (pkt->buf[i] != ASYNC_HDLC_ESC) {
			crc = CRC_16_L_STEP(crc, (word) pkt->buf[i]);
		} else {
			i++;
			crc = CRC_16_L_STEP(crc, (word)(pkt->buf[i]^(byte)ASYNC_HDLC_ESC_MASK));
		}
	}
	crc ^= CRC_16_L_SEED;

	ADD_CRC_TO_PACKET(*pkt,crc);			 /* Add the CRC to the packet */

	pkt->buf[pkt->length++] = ASYNC_HDLC_FLAG;	/* Add a flag to the packet.*/
											  /* This can't use the regular
												 add_byte_to_packet() function
												 because it's a flag. */
	}
}/* finish_building_hdlc_packet() */

/*===========================================================================

FUNCTION finish_building_at_packet

DESCRIPTION
  This function completes the process of building a at packet dynamically.
  If all is well, it adds '\r\0' to the end of the buffer.
  If an error has been encountered in building the packet, it substitutes
  a NAK packet for whatever has been built.

DEPENDENCIES

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/

static void finish_building_at_packet
(
  pkt_buffer_s_type  *pkt
	/* Structure containing the packet being built */
)
{
	uint8* pbuf = pkt->buf;
	uint32 len  = pkt->length;
	uint32 nlen = len;
	uint8* nbuf = NULL;
	uint32 i = 0;

	//ASSERT(pkt != NULL);

	if (len < 2) {
		pkt->broken = true;
		return;
	}

	uint8  last_byte = pbuf[len-1];
	uint8  last_second_byte = pbuf[len-2];

	/* check last 2 bytes */
	if (last_byte == '\0') {
		if (last_second_byte != '\r') {
			nlen = len + 1; // 1 more byte for '\r'
		}
	} else if (last_byte == '\r') {
		nlen = len + 1; // 1 more byte for '\0'
	} else { // last_byte != '\0' && last_byte != '\r'
		nlen = len + 2; // 2 more bytes for '\r' and '\0'
	}

	if (len >= 4*1024) {
		pkt->broken = true;
		return;
	}

	/* original buffer is a fine AT cmd */
	if (nlen == len)
	{
		return;
	}

	NEW_ARRAY(nbuf, uint8, nlen);

	//ASSERT(nbuf != NULL);

	for (i=0; i<len; i++) {
		if (pbuf[i] == '\r' || pbuf[i] == '\0') {
			break;
		}
		nbuf[i] = pbuf[i];
	}

	nbuf[i++] = '\r';
	nbuf[i++] = '\0';

	if (i > nlen)
	{
		RELEASE_ARRAY(nbuf);

		pkt->broken = true;
		return;
	}

	memcpy(pbuf, nbuf, i);
	pkt->length = i;

	RELEASE_ARRAY(nbuf);
}


//-----------------------------------------------------------------------------
CPacket::CPacket(TPktTypeEnumType type)
{
	if (type != PKT_TYPE_NONE)
	{
		this->m_PktType = type;
	}
	else
	{
		this->m_PktType = PKT_TYPE_HDLC;
	}
    m_Port = 0;
}

CPacket::~CPacket(void)
{

}

CPacket::CPacket(const char* devpath)
{
    m_Port = 0;
}

/*===========================================================================
DESCRIPTION
	Initialize the packet instance, and open the internal serial port
	communication instance for packet sending/receiving.

DEPENDENCIES
	Serial port communication with specific port is ready for opening.

RETURN VALUE
  TResult

SIDE EFFECTS
	Internal serial port comm instance is opened.

===========================================================================*/
TResult CPacket::Init(uint16 port)
{
    if (port == 0)
    {
        return EINVALIDPARAM;
    }

    if (FAILURE(this->m_SerialPort.Open(port)))
    {
        return EFAILED;
    }

    COMMTIMEOUTS to;
    switch (this->m_PktType)
    {
    case PKT_TYPE_AT:
        to.ReadIntervalTimeout = 0;
        to.ReadTotalTimeoutMultiplier = 0;
        to.ReadTotalTimeoutConstant = PORT_TIMEOUT;
        to.WriteTotalTimeoutMultiplier = 0;
        to.WriteTotalTimeoutConstant = PORT_TIMEOUT;
        break;

    case PKT_TYPE_DIAG:
    case PKT_TYPE_HDLC:
    default:
        to.ReadIntervalTimeout = 0;
        to.ReadTotalTimeoutMultiplier = 1;
        to.ReadTotalTimeoutConstant = PORT_TIMEOUT;
        to.WriteTotalTimeoutMultiplier = 1;
        to.WriteTotalTimeoutConstant = PORT_TIMEOUT;
        break;
    }
    m_Port = port;
	return this->m_SerialPort.SetTimeOut(to);
}

/*===========================================================================
DESCRIPTION
	Uninitialize the packet, and close the internal serial port comm.

DEPENDENCIES

RETURN VALUE
  None.

SIDE EFFECTS
	Internal serial port comm will be closed.

===========================================================================*/

TResult CPacket::Uninit(void)
{
	return this->m_SerialPort.Close();
}

/*===========================================================================
DESCRIPTION
	Dispatch the command to speficic handler according to command type
	and get sent buffer length.

DEPENDENCIES

RETURN VALUE
  TResult

SIDE EFFECTS

===========================================================================*/

TResult CPacket::Send
(
	cmd_buffer_s_type* cmd_ptr, // command to be sent
	uint32* rlen                // sent buffer length
)
{
	TResult result = EOK;

	if (cmd_ptr == NULL || rlen == NULL) {
		return EINVALIDPARAM;
	}

	switch (cmd_ptr->type) {
	case CMD_TYPE_DIAG:
		result = this->SendDIAGPkt(cmd_ptr->buf, cmd_ptr->length, rlen);
		break;

	/* Both DLOAD and HOSTDL packet are HDLC frames */
	case CMD_TYPE_DLOAD:
	case CMD_TYPE_HOSTDL:
		result = this->SendHDLCPkt(cmd_ptr->buf, cmd_ptr->length, rlen);
		break;

	case CMD_TYPE_AT:
		result = this->SendATPkt(cmd_ptr->buf, cmd_ptr->length, rlen);
		break;

	case CMD_TYPE_SCSI:
		result = this->SendSCSIPkt(cmd_ptr->buf, cmd_ptr->length, rlen);
		break;

	case CMD_TYPE_SAHARA:
		result = this->SendSAHARAPkt(cmd_ptr->buf, cmd_ptr->length, rlen);
		break;

	default:
		result = EFAILED;
		break;
	}

	return result;
}

/*===========================================================================
DESCRIPTION
	Dispatch the received buffer to specific handler according to the
	expected response.

DEPENDENCIES

RETURN VALUE
	TResult

SIDE EFFECTS
	Response buffer will be updated.

===========================================================================*/

TResult CPacket::Receive(rsp_buffer_s_type* rsp_ptr, uint32* rlen)
{
	TResult result = EOK;

	if (rsp_ptr == NULL || rlen == NULL)
	{
		return EINVALIDPARAM;
	}

	uint32 len  = rsp_ptr->length;
	uint8* pbuf = rsp_ptr->buf;

	/* we expect to receive some data, so return EFAILED in case the
	 * input param is wrong.
	 */
	if (len == 0) {
		return EFAILED;
	}

	*rlen = 0;

	/* Dispatch command to specific handler according to type */
	switch (rsp_ptr->type) {
	case CMD_TYPE_DIAG:
		result = this->RcvDIAGPkt(m_Packet.buf, rlen);
		break;

	/* Both DLOAD and HOSTDL data are HDLC frames */
	case CMD_TYPE_DLOAD:
	case CMD_TYPE_HOSTDL:
		result = this->RcvHDLCPkt(m_Packet.buf, rlen);
		break;

	case CMD_TYPE_AT:
		result = this->RcvATPkt(m_Packet.buf, rsp_ptr->length, rlen);
		break;

	case CMD_TYPE_SCSI:
		result = this->RcvSCSIPkt(m_Packet.buf, rlen);
		break;

	case CMD_TYPE_SAHARA:
		result = this->RcvSAHARAPkt(m_Packet.buf, rsp_ptr->length, rlen);
		break;

	default:
		result = EPACKETUNKNOWN;
		break;
	}

	if (FAILURE(result)) {
		return result;
	}

	/* Copy rsp buffer size no more than requested size */
	*rlen = MIN(*rlen, len);
	memcpy(pbuf, m_Packet.buf, *rlen);
	rsp_ptr->length = *rlen;

	return result;
}

/*===========================================================================
DESCRIPTION
	Set packet sending/receiving operation timeout.

DEPENDENCIES

RETURN VALUE
  TResult

SIDE EFFECTS
	Internal serial port comm timeout will be updated.

===========================================================================*/

TResult CPacket::SetTimeout(uint32 ms)
{
        if (ms == 0)
        {
		return this->m_SerialPort.SetTimeOut();
        }
        else
        {
		return this->m_SerialPort.SetTimeOut(ms);
	}
}

TResult CPacket::SetTimeout(COMMTIMEOUTS to)
{
	return this->m_SerialPort.SetTimeOut(to);
}

/*===========================================================================
DESCRIPTION
	Get current packet sending/receiving operation timeout.

DEPENDENCIES

RETURN VALUE
	Internal serial port comm timeout.

SIDE EFFECTS
	None

===========================================================================*/

uint32 CPacket::GetTimeout(void)
{
	return this->m_SerialPort.GetTimeOut();
}


uint16 CPacket::GetComPort(void)
{
	return this->m_SerialPort.GetComPort();
}

void CPacket::SetPacketType(TPktTypeEnumType type)
{
	this->m_PktType = type;
}

char* CPacket::GetDevice()
{
	return this->m_SerialPort.GetDevice();
}
/*===========================================================================
DESCRIPTION
	Packing command buffer into DIAG frame format and sent it through
	internal serial port comm.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	TResult

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::SendDIAGPkt(uint8* pbuf, uint32 len, uint32* rlen)
{
	TResult result = EOK;
	pkt_buffer_s_type* pkt_ptr = &this->m_Packet;

	/* We expect to send some data, so return EFAILED in case the
	 * input param is wrong.
	 */
	if (pbuf == NULL || rlen == NULL || len == 0)
	{
		return EINVALIDPARAM;
	}

	*rlen = 0;

	START_BUILDING_DIAG_PKT(pkt_ptr);
	for (uint32 i=0; i<len; i++)
	{
		add_byte_to_packet(pkt_ptr, pbuf[i]);
	}
	FINISH_BUILDING_DIAG_PKT(pkt_ptr);

	if (pkt_ptr->broken)
	{
		return EPACKETBROKEN;
	}

	result = this->SendPacket(pkt_ptr, rlen);

	return result;
}

/*===========================================================================
DESCRIPTION
	Packing command buffer into HDLC frame format and sent it through
	internal serial port comm.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	TResult

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::SendHDLCPkt(uint8 * pbuf, uint32 len, uint32 * rlen)
{
	TResult result = EOK;
	pkt_buffer_s_type* pkt_ptr = &this->m_Packet;

	/* we expect to send some data, so return EFAILED in case the
	 * input param is wrong.
	 */
	if (len == 0)
	{
		return EFAILED;
	}

	if (pbuf == NULL || rlen == NULL)
	{
		return EINVALIDPARAM;
	}

	*rlen = 0;

	START_BUILDING_HDLC_PKT(pkt_ptr);
	for (uint16 i=0; i < len; i++)
		add_byte_to_packet(pkt_ptr, pbuf[i]);
	FINISH_BUILDING_HDLC_PKT(pkt_ptr);

	if (pkt_ptr->broken)
	{
		return EPACKETBROKEN;
	}

	result = this->SendPacket(pkt_ptr, rlen);
	if (result == EPORTSEND)
	{
		result = EPACKETSEND;
	}
	else if (FAILURE(result))
	{
		result = EFAILED;
	}

	return result;
}

/*===========================================================================
DESCRIPTION
	Packing command buffer into HDLC frame format and sent it through
	internal serial port comm.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	None

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::SendATPkt(uint8 * pbuf, uint32 len, uint32 * rlen)
{
	TResult result = EOK;
	pkt_buffer_s_type* pkt_ptr = &this->m_Packet;

	if (pbuf == NULL || rlen == NULL)
	{
		return EINVALIDPARAM;
	}

	/* we expect to send some data, so return EFAILED in case the
	 * input param is wrong.
	 */
	if (len == 0)
	{
		return EFAILED;
	}

	*rlen = 0;

	START_BUILDING_AT_PKT(pkt_ptr);
	memcpy(pkt_ptr->buf, pbuf, len);
	pkt_ptr->length += len;
	FINISH_BUILDING_AT_PKT(pkt_ptr);

	if (pkt_ptr->broken)
	{
		return EPACKETBROKEN;
	}

	result = this->SendPacket(pkt_ptr, rlen);

	return result;
}

/*===========================================================================
DESCRIPTION
	Packing command buffer into SCSI frame format and sent it through cdrom.

DEPENDENCIES

RETURN VALUE
	TResult

SIDE EFFECTS

===========================================================================*/

TResult CPacket::SendSCSIPkt(uint8 * pbuf, uint32 len, uint32 * rlen)
{
	TResult result = EOK;

	return result;
}

/*===========================================================================
DESCRIPTION
===========================================================================*/
//add by minghui.zhang 2013-11-04
TResult CPacket::SendSAHARAPkt(uint8 * pbuf, uint32 len, uint32 * rlen)
{
	TResult result = EOK;
	pkt_buffer_s_type* pkt_ptr = &this->m_Packet;

	if (pbuf == NULL || rlen == NULL) {
		return EINVALIDPARAM;
	}

	/* we expect to send some data, so return EFAILED in case the
	 * input param is wrong.
	 */
	if (len == 0) {
		return EFAILED;
	}

	*rlen = 0;

	START_BUILDING_AT_PKT(pkt_ptr);
	memcpy(pkt_ptr->buf, pbuf, len);
	pkt_ptr->length += len;

	if (pkt_ptr->broken) {
		return EPACKETBROKEN;
	}

	result = this->SendPacket(pkt_ptr, rlen);

	return result;
}
//end add
/*===========================================================================
	Receive DIAG frame from serial port comm and unpack it to a DIAG rsp.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	TResult

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::RcvDIAGPkt(uint8 *buf, uint32* rlen)
{
#define BUFFER_HEADER_LEN (64)

	TResult result;
	uint8   chr;
	uint32  count = 0;
	uint32  len = 0;
	uint16 port = this->m_SerialPort.GetComPort();

	if (buf == NULL || rlen == NULL)
	{
		return EINVALIDPARAM;
	}

	uint16 i = 0;
	uint8 buffer_header[BUFFER_HEADER_LEN] = {0};

	for (;;)
	{
		result = this->m_SerialPort.Receive(&chr, 1, &count);
		/* Print first 5 bytes of the received buffer */
		if (i < BUFFER_HEADER_LEN)
		{
			buffer_header[i] = chr;
			++ i;
		}
		if (FAILURE(result))
		{
			goto RcvDIAGPkt_Error;
		}
		else if (count == 0)
		{
			break;
		}

		/* If we reach the end of the packet, stop gethering more buffer */
		if (chr == ASYNC_HDLC_FLAG)
		{
			buf[len++] = chr;
			break;
		}

		if (chr == ASYNC_HDLC_ESC)
		{
			result = this->m_SerialPort.Receive(&chr, 1, &count);
			if (FAILURE(result))
			{
				goto RcvDIAGPkt_Error;
			}
			if (count == 0)
			{
				break;
			}
			chr ^= ASYNC_HDLC_ESC_MASK;
		}

		if (len >= MAX_PACKET_LEN)
		{			  /* Make sure there's room */

			result = EPACKETRECEIVETOOLARGE;
			goto RcvDIAGPkt_Error;
		}

		buf[len++] = chr;
	}

	*rlen = len;

	return EOK;

RcvDIAGPkt_Error:
    LOGD("COM%d: RcvDIAGPkt failure, first %d bytes:", port, MIN(i, BUFFER_HEADER_LEN));

	return result;
}

/*===========================================================================
DESCRIPTION
	Receive HDLC frame from serial port comm and unpack it to a
	DLOAD/HOSTDL rsp.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	TResult

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::RcvHDLCPkt(uint8 *buf, uint32* rlen)
{
#define BUFFER_HEADER_LEN (64)
        int i = 0;
	uint8 buffer_header[BUFFER_HEADER_LEN] = {0};

	/* State variable for decoding async HDLC */
	enum {
		HDLC_HUNT_FOR_FLAG, 	/* Waiting for a flag to start a packet 	  */
		HDLC_GOT_FLAG,			/* Have a flag, expecting the packet to start */
		HDLC_GATHER,			/* In the middle of a packet				  */
		HDLC_PACKET_RCVD		/* Now have received a complete packet		  */
	} state;

	/* Current character being received */
	byte   chr;

	/* Length of packet collected so far */
	int  len=0;

	/* Cyclic Redundancy Check, computed as we go. */
	int  crc=0;

	TResult result = EOK;
	uint32     count = 0;
	uint16  port = this->m_SerialPort.GetComPort();

	if (buf == NULL || rlen == NULL)
	{
		result = EINVALIDPARAM;
		goto RcvHDLCPkt_Error;
		//return EINVALIDPARAM;
	}

	*rlen = 0;

	/* Look at characters and try to find a valid async-HDLC packet of
	** length at least MIN_PACKET_LEN with a valid CRC.
	** Keep looking until we find one. */

	for (state = HDLC_HUNT_FOR_FLAG; state != HDLC_PACKET_RCVD; /* nil */)
	{
		result = m_SerialPort.Receive(&chr, 1, &count); /* Get next character (wait for it) */
		/* Print first 5 bytes of the received buffer */
		if (i < BUFFER_HEADER_LEN)
		{
			//DBGD("COM%d: RcvHDLCPkt, byte %d = %d", port, i, chr);
			buffer_header[i] = chr;
			++ i;
		}
		if (FAILURE(result))
		{
			goto RcvHDLCPkt_Error;
			//return result;
		} else if (count == 0)
		{

			result = EPORTRECEIVE;
			goto RcvHDLCPkt_Error;
			//return EPORTRECEIVE;
		}

		switch(state)
		{
		/*lint -esym(788,HDLC_PACKET_RCVD)	No need to deal with HDLC_PACKET_RCVD
		since we're in a loop that guarantees we're not in that state. */
		case HDLC_HUNT_FOR_FLAG:			 /* We're looking for a flag ... */
			if (chr == ASYNC_HDLC_FLAG)
			{	 /*   and we got one ...		 */
				state = HDLC_GOT_FLAG;		 /*   so go on to the next step. */
			}
			break;

		case HDLC_GOT_FLAG: 				 /* Had a flag, now expect a packet */
			if (chr == ASYNC_HDLC_FLAG)
			{	 /* Oops, another flag.  No change. */
				break;
			}
			else
			{								 /* Ah, we can really begin a packet */
				len = 0;					 /* The packet starts out empty 	 */
				crc = CRC_16_L_SEED;		 /* and the CRC in its initial state */
				state = HDLC_GATHER;		 /* and we begin to gather a packet  */
				/* Fall through */			 /*   (starting with this byte) 	 */
			}

		case HDLC_GATHER:					 /* We're gathering a packet	  */
			if (chr == ASYNC_HDLC_FLAG)
			{								/* We've reached the end		  */
				if (len < MIN_PACKET_LEN)
				{							/* Reject any too-short packets  */

					result = EPACKETRECEIVETOOSHORT;
					goto RcvHDLCPkt_Error;
					//return EPACKETRECEIVETOOSHORT;
				}
				else if (crc != CRC_16_L_OK_NEG)
				{							/* Reject any with bad CRC */

					result = EPACKETRECEIVECRC;
					goto RcvHDLCPkt_Error;
					//return EPACKETRECEIVECRC;
				}
				else
				{                             /* Yay, it's a good packet! */
					state = HDLC_PACKET_RCVD;                  /* Done for now   */
				}
				break;						/* However it turned out, this packet is over.  */
			}

			/* It wasn't a flag, so we're still inside the packet. */
			if (chr == ASYNC_HDLC_ESC)
			{								 /* If it was an ESC		*/
				result = m_SerialPort.Receive(&chr,1, &count); /* Get next character (wait for it) */
				if (FAILURE(result))
				{    /* If it's an error ... */
					goto RcvHDLCPkt_Error;
					//return result;
				}
				else if (count == 0)
				{
					result = EPORTRECEIVE;
					goto RcvHDLCPkt_Error;
					//return EPORTRECEIVE;
				}

				chr ^= ASYNC_HDLC_ESC_MASK; 		   /* Otherwise, de-mask it  */
				/* No break; process the de-masked byte normally */
			}

			if (len >= MAX_PACKET_LEN)
			{	/* Make sure there's room */
				result = EPACKETRECEIVETOOLARGE;
				goto RcvHDLCPkt_Error;
				//return EPACKETRECEIVETOOLARGE;
			}
			else
			{
				buf[len++] = (byte) chr;				   /* Add byte to buffer */
				crc = CRC_16_L_STEP(crc, (word) chr);	   /* Update the CRC	 */
			}

			break;

		default:	   /* Shouldn't happen with an enum, but for safety ...  */
			state = HDLC_HUNT_FOR_FLAG; 				 /* Start over		   */
			break;
		}/* switch on state */
	}/* for (packet not found) */

	*rlen = len;

	return EOK;

RcvHDLCPkt_Error:
	//TRACE2("COM%d: RcvHDLCPkt failure, first %d bytes: ", port, MIN(i, BUFFER_HEADER_LEN));
    LOGD("COM%d: RcvHDLCPkt failure, first %d bytes: ", port, MIN(i, BUFFER_HEADER_LEN));

	return result;
}


/*===========================================================================
DESCRIPTION
	Receive AT frame from serial port comm and unpack it to a AT rsp.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	TResult

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::RcvATPkt(uint8 *buf, uint32 len, uint32* rlen)
{
	TResult result = EOK;
	uint32  count = 0;

	result = this->m_SerialPort.Receive(buf, len, &count);
	if (FAILURE(result))
	{
		return result;
	}

	*rlen = count;

	return result;
}

/*===========================================================================
DESCRIPTION
	Receive SCSI frame from serial port comm and unpack it to a SCSI rsp.

DEPENDENCIES
	Packet instance has been inited.

RETURN VALUE
	TResult

SIDE EFFECTS
	If rsp is correct, the device received the command will switch the
	composite to display serial port comm and hide cdrom.

===========================================================================*/

TResult CPacket::RcvSCSIPkt(uint8 *buf, uint32* rlen)
{
	TResult result = EOK;

	return result;
}

/*===========================================================================
DESCRIPTION
===========================================================================*/
//add by minghui.zhang 2013-11-04
TResult CPacket::RcvSAHARAPkt(uint8 *buf, uint32 len, uint32* rlen)
{
	TResult result = EOK;
	uint32  count = 0;

	result = this->m_SerialPort.Receive(buf, len, &count);
	if (FAILURE(result)) {
		return result;
	}

	*rlen = count;

	return result;
}
//end add
/*===========================================================================
	Send packet frame through internal serial port comm.

DEPENDENCIES
	Internal serial port comm has been opened.

RETURN VALUE
	TResult

SIDE EFFECTS
	None

===========================================================================*/

TResult CPacket::SendPacket(pkt_buffer_s_type* pkt, uint32* rlen)
{
	if (pkt == NULL || rlen == NULL)
	{
		return EINVALIDPARAM;
	}
	return this->m_SerialPort.Send(pkt->buf, pkt->length, rlen);
}

//add by minghui.zhang send data directly 2013-11-06
TResult CPacket::SendData(uint8 buf[], uint32 length)
{
    uint32  rlen = 0;
    if (buf == NULL || length == 0)
    {
        return EINVALIDPARAM;
    }
    return this->m_SerialPort.Send(buf, length, &rlen);
}

TResult CPacket::SaharaDLReceive(rsp_buffer_s_type* rsp_ptr, uint32 rlen)
{

    TResult result = EOK;
    uint32  count = 0;
    uint32 len  = rsp_ptr->length;
    uint8* pbuf = rsp_ptr->buf;

   // result = this->m_SerialPort.SaharaDLReceive(m_Packet.buf, len, &count);
    result = this->m_SerialPort.Receive(m_Packet.buf, len, &count);
    if (FAILURE(result))
    {
        return result;
    }

    memcpy(pbuf, m_Packet.buf, rlen);

    return result;

}

