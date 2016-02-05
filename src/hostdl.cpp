
#include "stdafx.h"
#include "hostdl.h"
//#include "feature.h"
#include "utils.h"
#include "log.h"
#include "dlprg.h"

//-----------------------------------------------------------------------------

/* Command/Rsp codes
*/
#define HELLO_CMD					 0x01
#define HELLO_RSP					 0x02
#define READ_CMD					 0x03
#define READ_RSP					 0x04
#define WRITE_CMD					 0x05
#define WRITE_RSP					 0x06
#define STREAM_WRITE_CMD                                 0x07
#define STREAM_WRITE_RSP                                 0x08
#define NOP_CMD 					 0x09
#define NOP_RSP 					 0x0A
#define RESET_CMD					 0x0B
#define RESET_RSP					 0x0C
#define ERROR_RSP					 0x0D
#define CMD_LOG 					 0x0E
#define UNLOCK_CMD					 0x0F
#define UNLOCK_RSP					 0x10
#define PWRDOWN_CMD                                      0x11
#define PWRDOWN_RSP                                      0x12
#define OPEN_CMD					 0x13
#define OPEN_RSP					 0x14
#define CLOSE_CMD					 0x15
#define CLOSE_RSP					 0x16
#define SECURITY_MODE_CMD                                0x17
#define SECURITY_MODE_RSP                                0x18
#define PARTITION_TABLE_CMD                              0x19
#define PARTITION_TABLE_RSP                              0x1A

#define OPEN_MULTI_RSP                                   0x1C

#define FINALIZE_KBYRE_RSP                               0x7F

#define MI_BOOT_SEC_MODE_NON_TRUSTED                     0x00
#define MI_BOOT_SEC_MODE_TRUSTED                         0x01

#define WRITE_DASHBOARD_VER                              0x20
#define HANDLE_DASHBOARD_VER_RSP                         0x21

//add for MDM9x15
#define CMD_WRITE_DASHBOARD_VER                          0xFA
//end add



//add by jie.li for MDM9x15
typedef enum {
    MODE_NONE                   = 0x00,
    MODE_SBL1                   = 0x01,
    MODE_SBL2                   = 0x02,
    MODE_RPM                    = 0x03,
    MODE_DSP1                   = 0x04,
    MODE_DSP2                   = 0x05,
    MODE_DSP3                   = 0x06,
    MODE_BOOTOE                 = 0x07,
    MODE_APPSBOOT               = 0x08,
    MODE_BOOTIMG                = 0x09,
    MODE_USERIMG                = 0x10,
    MODE_EFS                    = 0x11,
    MODE_OEM                    = 0x12,
    MODE_CDROM                  = 0x13,
    MODE_FOTA                   = 0x14,
    //add by minghui.zhang 2013-11-05
    MODE_MBA                    = 0x15,
    MODE_QDSP6SW                = 0x16,
    MODE_SDI                    = 0x17,
    MODE_TZ                     = 0x18,

    MODE_9X25RESOURCE           = 0x19,
    MODE_9X25USRIMG             = 0x20,
    MODE_9X25BOOTIMG            = 0x21,
    MODE_9X25IMGMDM             = 0x22,
    MODE_M850DSP2               = 0x23,

    MODE_9X25RECBOOTIMG         = 0x24,
    MODE_9X25RECMDMIMG          = 0x25,
    MODE_9X30_NON_HLOS          = 0x26,
    MODE_9X30_RESOURCE          = 0x27,
    MODE_9X30_USRFS             = 0x28,
    MODE_9X30_SYSFS             = 0x29,
    MODE_9X35RECOVERY_IMAGE     = 0x30,
    MODE_9635_BOOT              = 0x31,                 //mdm9635-boot.img
    MODE_9X15RESOURCE           = 0x32,       //add by zhang hao for YOTA 20150312
    MODE_9635_Boot_Second       = 0x33,   //add by zhang hao for 9X35 mdm9635-boot.img 20150330
    //end add

}TOPENMode;
//end add

/* Status Code Enumeration
   This lists the status result codes passed around in the program.

   This enum is used to index a table of response packets, so these
   values map exactly to possible responses. */
enum {
    ACK,					/* Success. Send an acknowledgement.		   */
    NAK_INVALID_FCS,		/* Failure: invalid frame check sequence.	   */
    NAK_INVALID_DEST,		/* Failure: destination address is invalid.    */
    NAK_INVALID_LEN,		/* Failure: operation length is invalid.	   */
    NAK_EARLY_END,			/* Failure: packet was too short for this cmd. */
    NAK_TOO_LARGE,			/* Failure: packet was too long for my buffer. */
    NAK_INVALID_CMD,		/* Failure: packet command code was unknown.   */
    NAK_FAILED, 			/* Failure: operation did not succeed.		   */
    NAK_WRONG_IID,			/* Failure: intelligent ID code was wrong.	   */
    NAK_BAD_VPP,			/* Failure: programming voltage out of spec    */
    NAK_VERIFY_FAILED,		/* Failure: readback verify did not match	   */
    NAK_NO_SEC_CODE,		/* Failure: not permitted without unlock	   */
    NAK_BAD_SEC_CODE,		/* Failure: invalid security code			   */
    NAK_CRC
};


/* Partition table override flag */
#define FLASH_PARTI_NO_OVERRRIDE      (0x0)
#define FLASH_PARTI_OVERRRIDE         (0x1)


/* List to include all the return types for partition table command */
enum {
    PARTI_TBL_ACCEPTED,
    PARTI_TBL_DIFFERS,
    PARTI_TBL_BAD_FORMAT,
    PARTI_TBL_ERASE_FAIL,
    PARTI_TBL_UNKNOWN_ERROR,
};


//-----------------------------------------------------------------------------

#define MAX_RESEND                   (5)    //5 max repeat send
#define MAX_DUMMY_PACKET_NUM         (10) //
#define MAX_HELLO_PACKET_NUM         (5)  //

#define SEND_DUMMY_INTERVAL          (3000)        //3000
#define SEND_HELLO_INTERVAL          (1000)

#define HOSTDL_LONG_TIMEOUT          (10000) // 60s
#define HOSTDL_SHORT_TIMEOUT         (5000)  // 5s

#ifdef FEATURE_FAST_DLOAD
#define SEND_HOSTDL_PACKET_LEN       (8*KB)
#else
#define SEND_HOSTDL_PACKET_LEN       (1*KB)
#endif

/*
*/

#define DECLARE_CMD_PTR(cmd_ptr) \
cmd_buffer_s_type* cmd_ptr = &this->cmd

#define DECLARE_RSP_PTR(rsp_ptr) \
                             rsp_buffer_s_type* rsp_ptr = &this->rsp

#define START_BUILDING_CMD(cmd_code) \
start_building_hostdl_cmd(cmd_ptr, cmd_code)

#define START_BUILDING_RSP(rsp_ptr) \
start_building_hostdl_rsp(rsp_ptr)

static void start_building_hostdl_cmd
(
cmd_buffer_s_type* cmd_ptr,
uint8 cmd_code
)
{
    memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
    cmd_ptr->type   = CMD_TYPE_HOSTDL;
    cmd_ptr->broken = false;
    cmd_ptr->buf[0] = cmd_code;
    cmd_ptr->length = 1;
}

static void start_building_hostdl_rsp(rsp_buffer_s_type* rsp_ptr)
{
    memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
    rsp_ptr->type   = CMD_TYPE_HOSTDL;
    rsp_ptr->broken = false;
    rsp_ptr->length = MAX_RSP_BUFFER_LEN;
}

static bool check_hostdl_cmd_ok(cmd_buffer_s_type* cmd_ptr)
{
    if (cmd_ptr->broken)
    {
        return false;
    }

    if (cmd_ptr->length > MAX_CMD_BUFFER_LEN)
    {
        return false;
    }

    if (cmd_ptr->type >= CMD_TYPE_LAST)
    {
        return false;
    }

    return true;
}

static bool NeedResend(TResult result)
{
    return (result == EDATACRC || result == EPACKETRECEIVECRC);
}

//-----------------------------------------------------------------------------
ProgressCallback CDLData::func = NULL;

CDLData::CDLData
        (
                CPacket* packetDll,
                TDLImgInfoType* pDLImgInfo,
                int adlPort
                )
{
    m_packetDll = packetDll;

    m_packetDll->SetPacketType(PKT_TYPE_HDLC);
    this->m_pDLImgInfo = pDLImgInfo;

    this->m_uBaseRatio = 0;
    this->m_uTotalRatio = 100;
    this->m_uRatio = 0;
    this->dlPort = adlPort;
    this->m_totalCount = 0;
    is9X25 = false;
    lastdone = 0;
}

CDLData::~CDLData(void)
{

}

//add by jie.li for MDM9x15
TResult CDLData::DownloadDataUsePrtn
        (
                uint8* data,
                uint32 len,
                uint8 * mode
                )
{
    TResult result = EOK;
    //TDLMode DlMode = OPEN_MULTI_MODE_NONE;

    //if (data == NULL || len == 0 || mode == MODE_NONE)
        if (data == NULL || len == 0 )
    {
        ERR("COM%d: DownloadData, invalid data=%p, len=%d, mode=OPEN_MULTI_MODE_NONE",
              this->m_port, data, len);
        return EINVALIDPARAM;
    }

    INFO("COM%d: DownloadData, mode=%d",dlPort, mode);

    m_packetDll->SetTimeout(HOSTDL_LONG_TIMEOUT);
    //result = SendOpenPacketUsePrtn((TOPENMode)mode);
    result = SendOpenPacketUsePrtn(mode);

    if (FAILURE(result))
    {
        return EHOSTDLOPEN;
    }

    result = SendWritePacket(data, len);
    if (FAILURE(result))
    {
        return EHOSTDLWRITE;
    }

    result = SendClosePacket();
    if (FAILURE(result))
    {
        return EHOSTDLCLOSE;
    }

    SLEEP(2000);

    return result;
}

//end add

/*===========================================================================
DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/

TResult CDLData::SendSecMode(void)
{
    TResult result = EOK;
    uint32  rlen = 0;
    int32   AckCode, ErrCode;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(SECURITY_MODE_CMD);

    cmd_ptr->buf[cmd_ptr->length++] = MI_BOOT_SEC_MODE_TRUSTED;

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = m_packetDll->Send(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            ERR("COM%d: Send SECMODE packet failed!", dlPort);
            return result;
        }

        char* ErrMsg = NULL;
        result = GetAck(AckCode, ErrCode, &ErrMsg);
        if (AckCode == CMD_LOG)
        {
            INFO("COM%d: SendSecMode rsp CMD_LOG, %s", dlPort, ErrMsg);
        }
        if (FAILURE(result))
        {
            ERR("COM%d: Get Ack of SECMODE packet failed!", dlPort);
            return result;
        }

        if (AckCode != SECURITY_MODE_RSP)
        {
            result = this->CheckAckError(AckCode, ErrCode);
            INFO("COM%d: Error Rsp for SECMODE packet, AckCode %d, ErrCode %d!",dlPort, AckCode, ErrCode);
        }

        if (!NeedResend(result))
        {
            break;
        }
    }

    return result;
}

/*===========================================================================
DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/

TResult CDLData::SendHelloPacket(void)
{
    TResult result = EOK;
    uint32  rlen = 0;
    int AckCode, ErrCode;
    DECLARE_CMD_PTR(cmd_ptr);

    //LOG("COM%d: Send HELLO packet ...", this->m_port);

    START_BUILDING_CMD(HELLO_CMD);

    const char* hello = "QCOM fast download protocol host";
    strcpy((char*)&cmd_ptr->buf[cmd_ptr->length], hello);
    cmd_ptr->length += strlen(hello);
    cmd_ptr->buf[cmd_ptr->length++] = 0x02; //magic number 1
    cmd_ptr->buf[cmd_ptr->length++] = 0x02; //magic number 2
    cmd_ptr->buf[cmd_ptr->length++] = 0x03;

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = m_packetDll->Send(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            WARN("COM%d: Send HELLO packet failed!", dlPort);
            return result;
        }

        char* ErrMsg = NULL;
        result = this->GetAck(AckCode, ErrCode, &ErrMsg);
        if (AckCode == CMD_LOG)
        {
            INFO("COM%d: SendHelloPacket rsp CMD_LOG, %s", dlPort, ErrMsg);
        }
        if (FAILURE(result))
        {
            WARN("COM%d: Get Ack of HELLO packet failed!", dlPort);
            return result;
        }

        if (AckCode != HELLO_RSP)
        {
            result = this->CheckAckError(AckCode, ErrCode);
            WARN("COM%d: Error Rsp for HELLO packet!", dlPort);
        }

        if (!NeedResend(result))
        {
            break;
        }
    }

    return result;
}

/*===========================================================================
DESCRIPTION
	Send open packet to the device to prepare for the following write packet.

DEPENDENCIES
	The security mode packet and the partition table have been sent successfully
	before a open packet has been sent.

RETURN VALUE
	TResult

SIDE EFFECTS

===========================================================================*/

TResult CDLData::SendOpenPacket
        (
                uint8 mode    /* open mode, i.e. which partition to open */
                )
{
    TResult result = EDATACRC;
    uint32  rlen = 0;
    int AckCode, ErrCode;
    DECLARE_CMD_PTR(cmd_ptr);

    //LOG("COM%d: Send OPEN packet ...",
    //	this->m_port);

    START_BUILDING_CMD(OPEN_MULTI_CMD);

    cmd_ptr->buf[cmd_ptr->length++] = mode;

    /* The OPEN command for AMSS and OEMSBL should contain header data.
	 */
    if (mode == OPEN_MULTI_MODE_AMSS)
    {
        if (m_pDLImgInfo->amsshd.len > 0)
        {
            memcpy(&cmd_ptr->buf[cmd_ptr->length],
                   this->m_pDLImgInfo->amsshd.data,
                   this->m_pDLImgInfo->amsshd.len);
            cmd_ptr->length += this->m_pDLImgInfo->amsshd.len;
        }
    }
    else if (mode == OPEN_MULTI_MODE_OEMSBL)
    {
        memcpy(&cmd_ptr->buf[cmd_ptr->length],
               this->m_pDLImgInfo->oemsblhd.data,
               this->m_pDLImgInfo->oemsblhd.len);
        cmd_ptr->length += this->m_pDLImgInfo->oemsblhd.len;
    }
    /* Begin: add by jie.li 2011-07-04 .MDM9200 compatible.*/
    else if (mode == OPEN_MULTI_MODE_CUSTOM)
    {
        cmd_ptr->buf[2] = '0';
        cmd_ptr->buf[3] = ':';
        cmd_ptr->buf[4] = 'F';
        cmd_ptr->buf[5] = 'T';
        cmd_ptr->buf[6] = 'L';
        cmd_ptr->buf[7] = '\0';
        cmd_ptr->length = 8;

    }
    /* End.*/

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = m_packetDll->Send(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            //ERR("COM%d: Send OPEN packet failed!",
            //	this->m_port);
            return result;
        }

        /* SLEEP 0.1 second before get ack */
        SLEEP(100);

        char* ErrMsg = NULL;
        result = this->GetAck(AckCode, ErrCode, &ErrMsg);
        if (AckCode == CMD_LOG)
        {
            //  INFO("COM%d: SendOpenPacket rsp CMD_LOG, %s",
            //		this->m_port, ErrMsg);
        }
        if (FAILURE(result))
        {
            //ERR("COM%d: Get Ack of OPEN packet failed!",
            //	this->m_port);
            return result;
        }

        if (AckCode != OPEN_MULTI_RSP)
        {
            result = this->CheckAckError(AckCode, ErrCode);
            //LOG("COM%d: Error Rsp for OPEN packet!",
            //	this->m_port);
        }

        if (!NeedResend(result))
        {
            break;
        }
    }

    return result;
}

//add by jie.li for MDM9x15
TResult CDLData::SendOpenPacketUsePrtn(uint8 *mode)
{
    TResult result = EDATACRC;
    uint32  rlen = 0;
    int AckCode, ErrCode;
    DECLARE_CMD_PTR(cmd_ptr);

    INFO("COM%d: Send OPEN packet ...", dlPort);
    memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
    cmd_ptr->type   = CMD_TYPE_HOSTDL;
    cmd_ptr->broken = false;
    int i=0;
    for(;mode[i]!='\0';i++)
        cmd_ptr->buf[i]=mode[i];
    cmd_ptr->length = i+1;
    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = m_packetDll->Send(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            ERR("COM%d: Send OPEN packet failed!", dlPort);
            return result;
        }

        /* SLEEP 0.1 second before get ack */
        SLEEP(100);

        char* ErrMsg = NULL;
        result = this->GetAck(AckCode, ErrCode, &ErrMsg);
        if (AckCode == CMD_LOG)
        {
            INFO("COM%d: SendOpenPacket rsp CMD_LOG, %s", dlPort, ErrMsg);
        }
        if (FAILURE(result))
        {
            ERR("COM%d: Get Ack of OPEN packet failed!", dlPort);
            return result;
        }

        if (AckCode != OPEN_MULTI_RSP)
        {
            result = this->CheckAckError(AckCode, ErrCode);
            INFO("COM%d: Error Rsp for OPEN packet!", dlPort);
        }

        if (!NeedResend(result))
        {
            break;
        }
    }

    return result;
}
//end add

/*===========================================================================
DESCRIPTION
	Send close packet to the device after all the write packets have been
	sent to the device successfully and force the device to write the buffer
	into flash.

DEPENDENCIES
	The images has been successfully written into the device.

RETURN VALUE
	TResult

SIDE EFFECTS
	Write packet can no long be sent when the close packet has been sent.

===========================================================================*/

TResult CDLData::SendClosePacket(void)
{
    TResult result = EOK;
    uint32  rlen = 0;
    int AckCode, ErrCode;
    DECLARE_CMD_PTR(cmd_ptr);

    //LOG("COM%d: Send CLOSE packet ...",
    //	this->m_port);

    START_BUILDING_CMD(CLOSE_CMD);

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = m_packetDll->Send(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            //ERR("COM%d: Send CLOSE packet failed!",
            //	this->m_port);
            return result;
        }

        char* ErrMsg = NULL;
        result = this->GetAck(AckCode, ErrCode, &ErrMsg);
        if (AckCode == CMD_LOG)
        {
            //INFO("COM%d: SendClosePacket rsp CMD_LOG, %s",
            //	this->m_port, ErrMsg);
        }
        if (FAILURE(result))
        {
            //ERR("COM%d: Get Ack of CLOSE packet failed!",
            //	this->m_port);
            return result;
        }

        if (AckCode != CLOSE_RSP)
        {
            //result = this->CheckAckError(AckCode, ErrCode);

            //remove by zhang hao for 9x15 boot-oe-msm9615.imag downlaod failed 20151020
            //LOG("COM%d: Error Rsp for CLOSE packet!",
            //	this->m_port);
        }

        if (!NeedResend(result))
        {
            break;
        }
    }

    return result;
}

/*===========================================================================
DESCRIPTION
	Send write packet to the device for images data written into the flash.

DEPENDENCIES
	The images data has been read into the buffer.

RETURN VALUE

SIDE EFFECTS

===========================================================================*/

TResult CDLData::SendWritePacket
        (   uint8* pData,    // data buffer to be written
            int len       // length of the data
        )
{
    TResult result = EOK;
    uint32  rlen = 0;
    int AckCode, ErrCode;
    uint32  addr = 0;
    uint32  datalen = 0;
    uint8   percent = 0;
    uint32  total = len;

    DECLARE_CMD_PTR(cmd_ptr);

    //INFO("COM%d: Send WRITE packet ...", this->m_port);

    while (len > 0)
    {
        datalen = MIN(SEND_HOSTDL_PACKET_LEN, len);

        START_BUILDING_CMD(STREAM_WRITE_CMD);

        /* CAUTION: Address byte sequence is different from downloading PRG!!!*/
        cmd_ptr->buf[cmd_ptr->length++] = B_PTR(addr)[0];
        cmd_ptr->buf[cmd_ptr->length++] = B_PTR(addr)[1];
        cmd_ptr->buf[cmd_ptr->length++] = B_PTR(addr)[2];
        cmd_ptr->buf[cmd_ptr->length++] = B_PTR(addr)[3];

        memcpy(&cmd_ptr->buf[cmd_ptr->length], pData+addr, datalen);
        cmd_ptr->length += datalen;

        if (!check_hostdl_cmd_ok(cmd_ptr))
        {
            //ERR("COM%d: SendWritePacket, hostdl cmd is broken!",
            //	this->m_port);
            return EBUFFERBROKEN;
        }

        for (int i=0; i<MAX_RESEND; ++i)
        {
            result = this->SendCmd(cmd_ptr, &rlen);
            if (FAILURE(result))
            {
                //WARN("COM%d: Send WRITE packet failed!",
                //	this->m_port);
                /* In case the device is removed because of unstability of the driver,
                         * we give one more chance to wait for the device being found.
                         */
                //INFO("COM%d: Wait 5 second for the device to be found again!!!",
                //	this->m_port);
                //this->UpdatePortState(PS_WAIT);
                //WaitForSingleObject(*((TDLUserDataType*)this->m_pCbInfo->pUserData)->pEvent, 5000);
                //this->UpdatePortState(PS_BUSY);
                //INFO("COM%d: Give another try to send write packet.",
                //	this->m_port);
                continue;
            }

            char* ErrMsg = NULL;
            result = this->GetAck(AckCode, ErrCode, &ErrMsg);
            if (AckCode == CMD_LOG)
            {
                //INFO("COM%d: SendWritePacket rsp CMD_LOG, %s",
                //	this->m_port, ErrMsg);
            }
            if (FAILURE(result))
            {
                //WARN("COM%d: Get Ack of WRITE packet failed!",
                //	this->m_port);
                //break;
                continue;
            }

            if (AckCode != STREAM_WRITE_RSP)
            {
                result = this->CheckAckError(AckCode, ErrCode);
                ////WARN("COM%d: Error Rsp for WRITE packet!",
                //	this->m_port);
            }

            if (addr != ErrCode)
            {
                //ERR("COM%d: SendWritePacket failure, cmd_addr %d, rsp_addr %d!!!",
                //	this->m_port, addr, ErrCode);
                return EFAILED;
            }

            if (!NeedResend(result))
            {
                break;
            }
        }

        if (FAILURE(result))
        {
            //ERR("COM%d: SendWritePacket failure!", this->m_port);
            return result;
        }

        //add to reslove the bug of progressbar
        if (total < 100)
        {
            percent = (((addr+datalen)*100)/total * this->m_uRatio) /100;
            //iRatio = (pImgLen->uDashboardImgLen*100/total);
        }
        else
        {
            percent = (((addr+datalen))/(total/100) * this->m_uRatio) /100;
            //iRatio = (pImgLen->uDashboardImgLen/(total/100));
        }
        //end add
        //percent = (((addr+datalen)*100)/total * this->m_uRatio) /100;
        percent = this->m_uBaseRatio + (percent * this->m_uTotalRatio) /100;

        if(is9X25 && percent < 1)
        {
            percent = 1;
            //INFO("COM%d is9X25 && percent < 1",dlPort);
        }

        this->UpdateProgress(percent);

        addr += datalen;
        len  -= datalen;

        /* Update progress here */
    }

    return result;
}

/*===========================================================================
DESCRIPTION
	Send reset command to the device to require for a reboot.

DEPENDENCIES

RETURN VALUE
	TResult

SIDE EFFECTS
	The device which receives and response the command correctly will reboot.

===========================================================================*/

TResult CDLData::SendResetCmd(void)
{
    INFO("COM%d: Send Reset command",dlPort);
    TResult result = EDATACRC;
    uint32  rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);

    START_BUILDING_CMD(RESET_CMD);

    result = m_packetDll->Send(cmd_ptr, &rlen);

    return result;
}


/*===========================================================================
DESCRIPTION
	Get Ack code and Error code from the response buffer.

DEPENDENCIES

RETURN VALUE
	TResult

SIDE EFFECTS

===========================================================================*/

TResult CDLData::GetAck
        (
                int& AckCode,
                int& ErrCode,
                char** ErrMsg
                )
{
    TResult result = EOK;
    uint32  rlen = 0;
    DECLARE_RSP_PTR(rsp_ptr);

    START_BUILDING_RSP(rsp_ptr);

    if (ErrMsg != NULL)
    {
        *ErrMsg = NULL;
    }
    ErrCode = ACK;

    /* only to retrieve 5 bytes */
    //rsp_ptr->length = 5;
    rsp_ptr->length = 256;

    /* SLEEP 0.1 second before receive data */
    //SLEEP(100);

    // receive a ACK packet
    result = m_packetDll->Receive(rsp_ptr, &rlen);
    if (FAILURE(result))
    {
        return result;
    }

    AckCode = rsp_ptr->buf[0];

    uint32 addr = 0;

    switch (AckCode)
    {
    case HELLO_RSP:
        //uint32 p = 44 + rsp_ptr->buf[43];
        //uint32 wsize = rsp_ptr->buf[p++];
        //wsize += rsp_ptr->buf[p++];
        break;

    case STREAM_WRITE_RSP:
        addr = rsp_ptr->buf[1] | (rsp_ptr->buf[2] << 8) | (rsp_ptr->buf[3] << 16) | (rsp_ptr->buf[4] << 24);
        ErrCode = (int)addr;
        break;

    case ERROR_RSP:
        ErrCode = rsp_ptr->buf[4];
        ErrCode <<= 8;
        ErrCode |= rsp_ptr->buf[3];
        ErrCode <<= 8;
        ErrCode |= rsp_ptr->buf[2];
        ErrCode <<= 8;
        ErrCode |= rsp_ptr->buf[1];
        if (ErrMsg != NULL)
        {
            //	int i = 0;
            *ErrMsg = (char*)&rsp_ptr->buf[5];
            //ERR("COM%d: ERROR //LOG from device <%s>",
            //	this->m_port, *ErrMsg);
        }
        result = EFAILED;
        break;

	case CMD_LOG:
        if (ErrMsg != NULL)
        {
            int i = 0;
            *ErrMsg = (char*)&(rsp_ptr->buf[1]);
            for (i=1; i<MAX_RSP_BUFFER_LEN; ++i)
            {
                if (rsp_ptr->buf[i] == '\n')
                {
                    rsp_ptr->buf[i] = 0;
                    break;
                }
            }
            if (i == MAX_RSP_BUFFER_LEN)
            {
                rsp_ptr->buf[MAX_RSP_BUFFER_LEN-1] = 0;
            }
            //WARN("COM%d: CMD LOG from device <%s>",
            //	this->m_port, *ErrMsg);
        }
        if (memcmp(&rsp_ptr->buf[1], "ERR: CRC invalid", 10) == 0)
        {
            AckCode = ERROR_RSP;
            ErrCode = NAK_CRC;
            result = EDATACRC;
        }
        else if (memcmp(&rsp_ptr->buf[1], "Close failed", 10) == 0)
        {
            AckCode = ERROR_RSP;
            ErrCode = NAK_FAILED;
            //result = EACKFAILED;  //remove by zhang hao for 9x15 boot-oe-msm9615.imag downlaod failed 20151020
        }
        break;

	case PARTITION_TABLE_RSP:
        /* As for PARTITION_TABLE_RSP, second byte of the buffer
		 * contains partition_status (PARTI_TBL_ACCEPTED/
		 * PARTI_TBL_DIFFERS/...)
		 */
        ErrCode = rsp_ptr->buf[1];
        break;

	default:
        break;
    }

    return result;
}

/*===========================================================================
DESCRIPTION
	Check ACK error according to the 'AckCode' and 'ErrCode'.

DEPENDENCIES

RETURN VALUE
	TResult

SIDE EFFECTS

===========================================================================*/

TResult CDLData::CheckAckError
        (
                int AckCode,
                int ErrCode
                )
{
    if (AckCode == ERROR_RSP && ErrCode == NAK_CRC)
    {
        return EDATACRC;
    }
    else
    {
        return EDATARSP;
    }
}


/*===========================================================================
DESCRIPTION
	Send dummy packet to the device to ping if the device is ready for
	communication when it's in PRG running state.

DEPENDENCIES
	The PRG has been downloaded into the device and has been run up.

RETURN VALUE
	TResult

SIDE EFFECTS

===========================================================================*/

TResult CDLData::SendDummyData(void)
{
    TResult result = EOK;
    uint32  rlen = 0;
    //int AckCode, ErrCode;
    DECLARE_CMD_PTR(cmd_ptr);

    INFO("COM%d: Send DUMMY packet ...", dlPort);

    START_BUILDING_CMD(HELLO_CMD);

    const char* hello = "QCOM fast download protocol host";

    strcpy((char*)&cmd_ptr->buf[cmd_ptr->length], hello);
    cmd_ptr->length += strlen(hello);

    cmd_ptr->buf[cmd_ptr->length++] = 0x02; // magic number 1
    cmd_ptr->buf[cmd_ptr->length++] = 0x02; // magic number 2
    cmd_ptr->buf[cmd_ptr->length++] = 0x03;

    result = m_packetDll->Send(cmd_ptr, &rlen);
    if (FAILURE(result)) {
        WARN("COM%d: Send DUMMY packet failed!", dlPort);
    }

    return result;
}

/*===========================================================================
DESCRIPTION
	Send partition table to the device first to check if the partition table
	is matched with the device's one.

DEPENDENCIES
	Partition table image buffer has been read into the memory.

RETURN VALUE
	TResult

SIDE EFFECTS
	If the partition table sent is different from the one in device, the
	whole partition of the flash needs to be erased first before downloading
	any images.

===========================================================================*/

TResult CDLData::SendPrtnTbl
        (
                uint8* pData,         // partition table data buffer
                uint32 len,           // length of partition table data
                bool   bOverride      // override flag
                )
{
    TResult result = EOK;
    uint32  rlen = 0;
    int AckCode, ErrCode;
    cmd_buffer_s_type* cmd_ptr = &this->cmd;

    //start_building_hostdl_cmd(cmd_ptr, PARTITION_TABLE_CMD);
    START_BUILDING_CMD(PARTITION_TABLE_CMD);

    if (bOverride)
    {
        cmd_ptr->buf[cmd_ptr->length++] = FLASH_PARTI_OVERRRIDE;
    }
    else
    {
        cmd_ptr->buf[cmd_ptr->length++] = FLASH_PARTI_NO_OVERRRIDE;
    }

    memcpy(&cmd_ptr->buf[cmd_ptr->length], pData, len);
    cmd_ptr->length += len;

    result = m_packetDll->Send(cmd_ptr, &rlen);
    if (FAILURE(result))
    {
        ERR("COM%d: Send PRTNTBL packet failed!",dlPort);
        return result;
    }

    char* ErrMsg = NULL;
    result = this->GetAck(AckCode, ErrCode, &ErrMsg);
    if (AckCode == CMD_LOG)
    {
        INFO("COM%d: SendPrtnTbl rsp CMD_LOG, %s",dlPort, ErrMsg);
    }
    if (AckCode != PARTITION_TABLE_RSP)
    {
        result = this->CheckAckError(AckCode, ErrCode);
        ERR("COM%d: Error Rsp for PRTNTBL packet!",dlPort);
    }
    else
    {
        if (ErrCode == PARTI_TBL_DIFFERS)
        {
            /* Partition table unmatched, return error or
* take special action.*/
            ERR("COM%d: difference partion version!",dlPort);
            return EHOSTDLPRTNTBLDIFF;
        }
    }

    if (FAILURE(result))
    {
        ERR("COM%d: Ack for PRTNTBL packet failed!",dlPort);
        return result;
    }

    return result;
}

/*===========================================================================
DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/

TResult CDLData::SendCmd
        (cmd_buffer_s_type* cmd_ptr,  // command buffer to be sent
        uint32* rlen                 // bytes of command buffer that being sent
        )
{
    if (cmd_ptr == NULL || rlen == NULL)
    {
        return EINVALIDPARAM;
    }

    return m_packetDll->Send(cmd_ptr, rlen);
}
//add by minghui.zhang for MDM9x25
TResult CDLData::DLoad9X25ImagesUsePtn(map<string,FileBufStruct> &FileBufMap,  uint32 Software_size)
{
    TResult result = EOK;
    is9X25 = true;
    uint8*  pdata = NULL;
    uint8* mode=NULL;
    uint32  len = 0;
    int     i;
    //QString comPort = QString("COM%1").arg(dlPort);
    for (i = 0; i < MAX_DUMMY_PACKET_NUM; ++i)
    {
        DBGD("COM%d: Send DUMMY packet %d ...",dlPort, i);
        result = this->SendDummyData();
        if (SUCCESS(result))
        {
            break;
        }
        SLEEP(SEND_DUMMY_INTERVAL);
    }
    if (FAILURE(result))
    {
        return EHOSTDLDUMMY;
    }

    SLEEP(SEND_DUMMY_INTERVAL);
    /* Keep send hello packet until reach max retry */
    for (i = 0; i < MAX_HELLO_PACKET_NUM; ++i)
    {
        DBGD("COM%d: Send HELLO packet %d ...",
              dlPort, i);
        result = this->SendHelloPacket();
        if (SUCCESS(result))
        {
            break;
        }
        SLEEP(SEND_HELLO_INTERVAL);
    }
    if (FAILURE(result))
    {
        return EHOSTDLHELLO;
    }
    SLEEP(1000);
    /* send security mode*/
    result = this->SendSecMode();
    if (FAILURE(result))
    {
        DBGD("COM%d: Send SECMODE packet failed!",
              dlPort);
        return EHOSTDLSECMODE;
    }
    /* send partition table*/
   // m_pDLImgInfo->prtn.data = m_dlFileBuffer.at(PARTITION_NAME).strFileBuf;
   // m_pDLImgInfo->prtn.len = m_dlFileBuffer.at(PARTITION_NAME).uFileLens;
    pdata = FileBufMap.at(PARTITION_NAME).strFileBuf;
    len = FileBufMap.at(PARTITION_NAME).uFileLens;
    result = this->SendPrtnTbl(pdata, len, FALSE);
    /* If send partition table returns unmatched, check
     * if all the images needed are present. If YES, resend
     * partition table then send qcsbl_cfgdata->qcsbl->
     * oemsbl->amss->efs; if NOT, report the error and return.
    */
    if (FAILURE(result))
    {
#if 0
        if (result == EHOSTDLPRTNTBLDIFF)
        {
            WARN("COM%d: Partition table unmatched!",
                 dlPort);
#ifdef FEATURE_PRTNTBL_DIFF_DLOAD
            /* check if all the images needed are present */
            if (mask & IMAGE_TYPE_ALL)
            {
                INFO("COM%d: Re-send partition table with override TRUE.",
                     dlPort);
                result = this->SendPrtnTbl(pdata, len, TRUE);
            }
            else
            {
                ERR("COM%d: Not all images are available, mask = %d!",
                      dlPort, mask);
            }
#endif
        }
#endif
        if (FAILURE(result))
        {
            ERR("COM%d: Send PRTNTBL packet failed!",
                  dlPort);
            return EHOSTDLPRTNTBL;
        }
    }
    /* calculate all images buffer length, hd files not included */
    uint32 total = Software_size;

    //end add
    uint8 base = this->m_uBaseRatio;
    std::map<string,FileBufStruct>::iterator it;
    for (it = FileBufMap.begin(); it != FileBufMap.end(); it++)
    {
        if(it->second.isDownload)
        {

            pdata=it->second.strFileBuf;
            len=it->second.uFileLens;
            /* Start downloading images buffer into device*/
            if (pdata == NULL || len == 0)
            {
                INFO("COM%d: DLoadImages, invalid pdata=%p or len=%d",
                     dlPort, pdata, len);
                // should we break to reset or return here???
                return EINVALIDPARAM;
            }
            mode=it->second.Area;
            //INFO(FILE_LINE, "COM%d: %s ...,len = %d", dlPort,QString::fromStdString(it->first),len);

            //TODO
            //m_mainApp->SlotUpdateStatus(comPort,"Download "+QString::fromStdString(it->first)+"...");

            if (total < 100)
            {
                this->m_uRatio = (len*100)/total;
            }
            else
            {
                this->m_uRatio = (len)/(total/100);
            }
            result = this->DownloadDataUsePrtn(pdata, len,  mode);
            if (FAILURE(result))
            {
                ERR("COM%d: DownloadData failed, return here!!!",
                      dlPort);
                return result;
            }
            this->m_uBaseRatio += (this->m_uRatio * this->m_uTotalRatio) / 100;
					if ("b.vhd" == it->first)
            {
                INFO("COM%d: WriteDashboard Version, version = %s",
                     dlPort,(char*)m_pDLImgInfo->dashboardVer);
                result = this->WriteDashboardVer((char*)m_pDLImgInfo->dashboardVer);
                if (FAILURE(result))
                {
                    ERR("COM%d: WriteDashboard Version failed,continue!!!",
                     dlPort);
                }
            }
            #if 0
            if("mdm9635-boot.img"==it->first)
            {
                QString AREA="0:recovery";
                for(int i=0;i<AREA.length();i++)
                {
                    char*  ch;
                    QByteArray ba = AREA.toLatin1();
                    ch=ba.data();
                    it->second.Area[i+2]=(unsigned char)ch[i];
                }
                mode=it->second.Area;
                LOG("COM%d: mdm9635-boot.img second download...,len = %d", dlPort,len);
                result = this->DownloadDataUsePrtn(pdata, len, mode);
                if (FAILURE(result))
                {
                    ERR("COM%d: mdm9635-boot.img second download failed, return here!!!",
                        dlPort);
                    return result;
                }

            }
            #endif
        }
    }
    this->m_uBaseRatio = base;
    this->UpdateProgress(this->m_uBaseRatio + this->m_uTotalRatio);
    return result ;
}
TResult CDLData::DLoad9X07ImagesUsePtn(map<string,FileBufStruct> &FileBufMap,  uint32 Software_size)
{
    TResult result = EOK;
    is9X25 = true;
    uint8*  pdata = NULL;
    uint8* mode=NULL;
    uint32  len = 0;
    int     i;
    //QString comPort = QString("COM%1").arg(dlPort);
    for (i = 0; i < MAX_DUMMY_PACKET_NUM; ++i)
    {
        DBGD("COM%d: Send DUMMY packet %d ...",dlPort, i);
        result = this->SendDummyData();
        if (SUCCESS(result))
        {
            break;
        }
        SLEEP(SEND_DUMMY_INTERVAL);
    }
    if (FAILURE(result))
    {
        return EHOSTDLDUMMY;
    }

    SLEEP(SEND_DUMMY_INTERVAL);
    /* Keep send hello packet until reach max retry */
    for (i = 0; i < MAX_HELLO_PACKET_NUM; ++i)
    {
        DBGD("COM%d: Send HELLO packet %d ...",dlPort, i);
        result = this->SendHelloPacket();
        if (SUCCESS(result))
        {
            break;
        }
        SLEEP(SEND_HELLO_INTERVAL);
    }
    if (FAILURE(result))
    {
        return EHOSTDLHELLO;
    }
    SLEEP(1000);
    /* send security mode*/
    result = this->SendSecMode();
    if (FAILURE(result))
    {
        DBGD("COM%d: Send SECMODE packet failed!",dlPort);
        return EHOSTDLSECMODE;
    }
    /* send partition table*/
   // m_pDLImgInfo->prtn.data = m_dlFileBuffer.at(PARTITION_NAME).strFileBuf;
   // m_pDLImgInfo->prtn.len = m_dlFileBuffer.at(PARTITION_NAME).uFileLens;
    pdata = FileBufMap.at(PARTITION_NAME).strFileBuf;
    len = FileBufMap.at(PARTITION_NAME).uFileLens;
    result = this->SendPrtnTbl(pdata, len, FALSE);
    /* If send partition table returns unmatched, check
     * if all the images needed are present. If YES, resend
     * partition table then send qcsbl_cfgdata->qcsbl->
     * oemsbl->amss->efs; if NOT, report the error and return.
    */
    if (FAILURE(result))
    {
#if 0
        if (result == EHOSTDLPRTNTBLDIFF)
        {
            WARN("COM%d: Partition table unmatched!",
                 dlPort);
#ifdef FEATURE_PRTNTBL_DIFF_DLOAD
            /* check if all the images needed are present */
            if (mask & IMAGE_TYPE_ALL)
            {
                INFO("COM%d: Re-send partition table with override TRUE.",
                     dlPort);
                result = this->SendPrtnTbl(pdata, len, TRUE);
            }
            else
            {
                ERR("COM%d: Not all images are available, mask = %d!",
                      dlPort, mask);
            }
#endif
        }
#endif
        if (FAILURE(result))
        {
            ERR("COM%d: Send PRTNTBL packet failed!", dlPort);
            return EHOSTDLPRTNTBL;
        }
    }
    /* calculate all images buffer length, hd files not included */
    uint32 total = Software_size;

    //end add
    uint8 base = this->m_uBaseRatio;
    std::map<string,FileBufStruct>::iterator it;
    for (it = FileBufMap.begin(); it != FileBufMap.end(); it++)
    {
        if(it->second.isDownload)
        {
            pdata=it->second.strFileBuf;
            len=it->second.uFileLens;
            /* Start downloading images buffer into device*/
            if (pdata == NULL || len == 0)
            {
                INFO("COM%d: DLoadImages, invalid pdata=%p or len=%d", dlPort, pdata, len);
                // should we break to reset or return here???
                return EINVALIDPARAM;
            }
            mode=it->second.Area;

            //INFO(FILE_LINE, "COM%d: %s ...,len = %d", dlPort,QString::fromStdString(it->first),len);
#ifdef FEATURE_TPST
           // m_mainApp->SlotUpdateStatus(comPort,"Download "+QString::fromStdString(it->first)+"...");
#endif
            if (total < 100)
            {
                this->m_uRatio = (len*100)/total;
            }
            else
            {
                this->m_uRatio = (len)/(total/100);
            }
            result = this->DownloadDataUsePrtn(pdata, len, mode);
            if (FAILURE(result))
            {
                ERR("COM%d: DownloadData failed, return here!!!",dlPort);
                return result;
            }
            this->m_uBaseRatio += (this->m_uRatio * this->m_uTotalRatio) / 100;
            if ("b.vhd" == it->first)
            {
                INFO("COM%d: WriteDashboard Version, version = %s",
                     dlPort,(char*)m_pDLImgInfo->dashboardVer);
                result = this->WriteDashboardVer((char*)m_pDLImgInfo->dashboardVer);
                if (FAILURE(result))
                {
                    ERR("COM%d: WriteDashboard Version failed,continue!!!",
                     dlPort);
                }
            }

        }
    }
    this->m_uBaseRatio = base;
    this->UpdateProgress(this->m_uBaseRatio + this->m_uTotalRatio);
    return result ;
}


//end add
/*===========================================================================
DESCRIPTION
	Set progress ratio and base for UI displaying.

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/

void CDLData::SetRatioParams(uint8 ratio, uint8 base)
{
    this->m_uTotalRatio = ratio;
    this->m_uBaseRatio = base;
}


/*===========================================================================
DESCRIPTION
	Send messages to update GUI progress bar, which indicating the
	downloading process is going on.

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/

void CDLData::UpdateProgress(uint8 percent)
{


    /* avoid too many update painting */
    if (percent == lastdone)
    {
        return;
    }

    percent = (percent < 100) ? percent : 100;

    lastdone = percent;

    if (func != NULL)
        func(dlPort,percent);
}


/*===========================================================================
DESCRIPTION
	Send messages to update com port state and GUI displaying.

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/


void CDLData::RegisterCallback(ProgressCallback callback)
{
    func = callback;
}

TResult CDLData::WriteDashboardVer(char* Ver)
{
    TResult result = EOK;
    uint32  rlen = 0;
    int32   AckCode, ErrCode;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(WRITE_DASHBOARD_VER);

    strcpy((char*)&cmd_ptr->buf[cmd_ptr->length], Ver);
    cmd_ptr->length += strlen(Ver);

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = m_packetDll->Send(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            ERR("COM%d: Send WriteDashboardVer failed!", this->m_port);
            return result;
        }

        char* ErrMsg = NULL;
        result = GetAck(AckCode, ErrCode, &ErrMsg);
        if (AckCode == CMD_LOG)
        {
            INFO("COM%d: WriteDashboardVer rsp CMD_LOG, %s", dlPort, ErrMsg);
        }
        if (FAILURE(result))
        {
            ERR("COM%d: Get Ack of WriteDashboardVer failed!", dlPort);
            return result;
        }

        if (AckCode != HANDLE_DASHBOARD_VER_RSP)
        {
            result = this->CheckAckError(AckCode, ErrCode);
            INFO("COM%d: Error Rsp for WriteDashboardVer, AckCode %d, ErrCode %d!",
                dlPort, AckCode, ErrCode);
        }

        if (!NeedResend(result))
        {
            break;
        }
    }

    return result;
}

void CDLData::setDlImgInfo(TDLImgInfoType*   pDLImgInfo)
{
    this->m_pDLImgInfo = pDLImgInfo;
}


