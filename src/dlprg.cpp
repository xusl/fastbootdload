#include "stdafx.h"
#include "dlprg.h"
#include "utils.h"
#include "diagcmd.h"
#include "log.h"


//-----------------------------------------------------------------------------

#define DLOAD_BASE        (0x800000)
#define DLOAD_8200_BASE   (0x100000)
#define DLOAD_9X15_BASE   (0x41700000)

#define MAX_NOP_CMD_NUM   (10)         //

#define SEND_NOP_INTERVAL (3000)       //3000

// DO NOT change this !!!
#define SEND_PRG_PACKET_LEN (1*KB)

//add by jie.li for MDM9x15
typedef enum{
	TYPE_ADDR_NONE	= 0,
	TYPE_ADDR_ONE	= 1,	// <8200
	TYPE_ADDR_TWO	= 2,	// 8200 || 9200
	TYPE_ADDR_THREE	= 3,	// 9x15
}TypeAddr;
//end add

//-----------------------------------------------------------------------------

#define DECLARE_CMD_PTR(cmd_ptr) \
		cmd_buffer_s_type* cmd_ptr = &this->cmd

#define DECLARE_RSP_PTR(rsp_ptr) \
		rsp_buffer_s_type* rsp_ptr = &this->rsp

#define START_BUILDING_CMD(cmd_code) \
		start_building_dload_cmd(cmd_ptr, cmd_code)

#define START_BUILDING_RSP() \
		start_building_dload_rsp(rsp_ptr)

#define SEND_CMD() \
		fSend(cmd_ptr, &rlen);


static void start_building_dload_cmd(cmd_buffer_s_type* cmd_ptr, uint8 cmd_code)
{
	memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
	cmd_ptr->type   = CMD_TYPE_DLOAD;
	cmd_ptr->broken = false;
	cmd_ptr->buf[0] = cmd_code;
	cmd_ptr->length = 1;
}

static void start_building_dload_rsp(rsp_buffer_s_type* rsp_ptr)
{
	memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
	rsp_ptr->type   = CMD_TYPE_DLOAD;
	rsp_ptr->broken = false;
	rsp_ptr->length = MAX_RSP_BUFFER_LEN;
}

CDLPrg::CDLPrg(CPacket* packetDll)
{
	m_packetDll = packetDll;
	m_packetDll->SetPacketType(PKT_TYPE_HDLC);
	this->port = m_packetDll->GetComPort();
}

CDLPrg::~CDLPrg()
{

}

TResult CDLPrg::MySwitchToDLoadMode(void)
{
    INFO("COM%d: MySwitchToDLoadMode",port);
    TResult result = EOK;

    for (int i = 0; i <= 10; i++)
    {
       if (i == 10)
            return EFAILED;

            bool bOk = this->PingDevice();
       if (bOk)
       {
                break;
       }
       SLEEP(1000);
    }

    // Switch device to download mode first
    result = this->SwitchToDLoadMode();
    if (FAILURE(result))
    {
        SLEEP(1000);
        result = this->SwitchToDLoadMode();
    }

    // The device takes some time to enter download mode
    SLEEP(10000);

    return result;
}

TResult CDLPrg::MyDownloadPrg(uint8* prgbuf, size_t len, int addrType,int myPort)
{
    TResult result = EOK;

    if (prgbuf == NULL || len == 0)
    {
        return EINVALIDPARAM;
    }

    /* After send out the switch command, keep pinging
     * the device, if the device is in download mode,
     * it will ack the NOP command.
     */

        //reopen the device


    /*QMessageBox msgBox;
    QString str = QString::number(myPort);
    msgBox.setInformativeText(str);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();*/
        SLEEP(3000);
        for (int i=0; i<3; i++)
        {
            result = m_packetDll->Init(myPort);
            if (SUCCESS(result))
            {
                break;
            }

            SLEEP(3000);
        }

        /*int port = m_packetDll->GetComPort();
        str = QString::number(port);
        msgBox.setInformativeText(str);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton(QMessageBox::Yes);
        ret = msgBox.exec();*/

    //for (int i = 0; i < 5; i++) {
    for (int i = 0; i < MAX_NOP_CMD_NUM; i++)
    {
        //WARN(FILE_LINE, tr("COM%d: Send NOP Cmd %1 ,index:%2...").arg(this->port).arg(i));
        result = SendNopCmd();
        if (SUCCESS(result))
        {
            break;
        }
        if (FAILURE(m_packetDll->Init(myPort)))
        {
            return EFAILED;
        }

        SLEEP(SEND_NOP_INTERVAL);
    }
    if (FAILURE(result))
    {
        ERR("COM%d: Send NOP Cmd failure!");
        return result;
    }

#if 1
    char* pVersion = NULL;
    //this->m_pPacket->SetTimeout(20000);

    for (int i=0; i<10; ++i)
    {
        result = this->SendVerReq(&pVersion);
        if (SUCCESS(result))
        {
            break;
        }
    }
    if (FAILURE(result))
    {
        ERR("COM%d: Send version request failure!",port);
    }

    RELEASE_ARRAY(pVersion);

#endif

    result = SendWrite32Cmd(prgbuf, len, addrType);
    if (FAILURE(result))
    {
        return result;
    }

    result = SendGoCmd(addrType);

    return result;
}

TResult CDLPrg::DownloadPrg(uint8* prgbuf, size_t len, int addrType)
{
    TResult result = EOK;

	if (prgbuf == NULL || len == 0)
	{
		return EINVALIDPARAM;
        }

    for (int i = 0; i <= 10; i++)
    {
       if (i == 10)
            return EFAILED;

            bool bOk = this->PingDevice();
       if (bOk)
       {
                break;
       }
       SLEEP(1000);
    }

	/* Switch device to download mode first */
	result = this->SwitchToDLoadMode();
	if (FAILURE(result))
	{
		SLEEP(1000);
		result = this->SwitchToDLoadMode();
	}

	/* The device takes some time to enter download mode */
	SLEEP(3000);

	/* After send out the switch command, keep pinging
	 * the device, if the device is in download mode,
	 * it will ack the NOP command.
	 */

        //reopen the device

	//for (int i = 0; i < 5; i++) {
	for (int i = 0; i < MAX_NOP_CMD_NUM; i++)
	{
		//WARN(FILE_LINE, tr("COM%d: Send NOP Cmd %1 ,index:%2...").arg(this->port).arg(i));
		result = SendNopCmd();
		if (SUCCESS(result))
		{
			break;
		}

		SLEEP(SEND_NOP_INTERVAL);
	}
	if (FAILURE(result))
	{
		ERR("COM%d: Send NOP Cmd failure!");
		return result;
	}

#if 1
	char* pVersion = NULL;
	//this->m_pPacket->SetTimeout(20000);

	for (int i=0; i<10; ++i)
	{
		result = this->SendVerReq(&pVersion);
		if (SUCCESS(result))
		{
			break;
		}
	}
	if (FAILURE(result))
	{
        ERR("COM%d: Send version request failure!",port);
	}

	RELEASE_ARRAY(pVersion);

#endif

    result = SendWrite32Cmd(prgbuf, len, addrType);
	if (FAILURE(result))
	{
		return result;
	}

    result = SendGoCmd(addrType);

	return result;
}

TResult CDLPrg::SendNopCmd(void)
{
	TResult result = EOK;
	uint32   rlen = 0;
	TAckCode ackcode;
	DECLARE_CMD_PTR(cmd_ptr);

	START_BUILDING_CMD(CMD_NOP);

	//LOG("COM%d: Send NOP Cmd", this->port);
	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
            ERR("COM%d: Send NOP Cmd failed!",port);
            return result;
	}

	result = GetAckCmd(&ackcode);
	if (FAILURE(result))
	{
        WARN("COM%d: Get Ack for NOP failed!",port);
		return result;
	}

	if (ackcode != ACK_OK)
	{
        WARN("COM%d: Ack for NOP error!",port);
		result = EACKFAILED;
	}

	return result;
}

TResult CDLPrg::RequestMobileID(uint32& mobileID, bool bTypeDload)
{
	TResult result = EOK;
	uint32   rlen = 0;
	TAckCode ackcode = ACK_END;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_CMD(CMD_MODEL_ID_REQ);

	//LOG("COM%d: Send NOP Cmd", this->port);
	result = SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
        ERR("COM%d: Send request Mobile ID failure!",port);
		return result;
	}

	//START_BUILDING_RSP();
	memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
	//changed for MDM9x15
	if (bTypeDload)
	{
		rsp_ptr->type   = CMD_TYPE_DLOAD;
	}
	else
		rsp_ptr->type   = CMD_TYPE_DIAG;
	//end changed
	rsp_ptr->broken = false;
	rsp_ptr->length = MAX_RSP_BUFFER_LEN;

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result))
	{
		return result;
	}

	if (rsp_ptr->buf[0] == CMD_MODEL_ID_RSP)
	{
		ackcode = ACK_OK;
	}
	else
	{
		result = EACKFAILED;
	}

	if (ackcode == ACK_OK)
	{
		memcpy(&mobileID, &rsp_ptr->buf[1], sizeof(mobileID));
		uint16 wH = (mobileID&0xFFFF0000)>>16;
		uint8  bH = (wH&0xFF00)>>8;
		uint8  bL = wH&0x00FF;
		wH = 0;
		wH = ((wH|bL)<<8)|bH;

		uint16 wL = (mobileID&0x0000FFFF)>>16;
		bH = (wL&0xFF00)>>8;
		bL = wL&0x00FF;
		wL = 0;
		wL = ((wL|bL)<<8)|bH;

		mobileID = 0;
		mobileID = ((mobileID|wL)<<16)|wH;
	}

	return result;
}

TResult CDLPrg::RequestBooVer(uint8& bootVer)
{
	TResult result = EOK;
	uint32   rlen = 0;
	TAckCode ackcode = ACK_END;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_CMD(CMD_BOOT_VER_REQ);

	//LOG("COM%d: Send NOP Cmd", this->port);
	result = SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
        ERR("COM%d: Send request Mobile ID failure!",port);
		return result;
	}

	//START_BUILDING_RSP();
	memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
	rsp_ptr->type   = CMD_TYPE_DIAG;
	rsp_ptr->broken = false;
	rsp_ptr->length = MAX_RSP_BUFFER_LEN;

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result))
	{
		return result;
	}

	if (rsp_ptr->buf[0] == CMD_BOOT_VER_RSP)
	{
		ackcode = ACK_OK;
	}
	else
	{
		result = EACKFAILED;
	}

	if (ackcode == ACK_OK)
	{
		bootVer = rsp_ptr->buf[1];
	}

	return result;
}

TResult CDLPrg::RequestFlashType_D(uint8& type,uint8& flash_size, bool bTypeDload)
{
	TResult result = EOK;
	uint32   rlen = 0;
	TAckCode ackcode = ACK_END;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_CMD(CMD_REQ_FLASH_TYPE);

	//LOG("COM%d: Send NOP Cmd", this->port);
	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
		ERR("COM%d: Send request flash type failure!");
		return result;
	}

	//START_BUILDING_RSP();
	memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
	//changed for MDM9x15
	if (bTypeDload)
	{
		rsp_ptr->type = CMD_TYPE_DLOAD;
	}
	else
		rsp_ptr->type = CMD_TYPE_DIAG;
	//end changed
	rsp_ptr->broken = false;
	rsp_ptr->length = MAX_RSP_BUFFER_LEN;

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result))
	{
		return result;
	}

	if (rsp_ptr->buf[0] == CMD_RSP_FLASH_TYPE)
	{
		ackcode = ACK_OK;
	}
	else
	{
		result = EACKFAILED;
	}

	if (ackcode == ACK_OK)
	{
		type = rsp_ptr->buf[1];
                flash_size=rsp_ptr->buf[5];
	}

	return result;
}

TResult CDLPrg::SendVerReq(char** version)
{
	TResult result = EOK;
	uint32   rlen = 0;
	TAckCode ackcode = ACK_END;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	if (version == NULL)
	{
		ERR("COM%d: SendVerReq failure, EINVALIDPARAM!");
		return EINVALIDPARAM;
	}

	START_BUILDING_CMD(CMD_VERREQ);

	//LOG("COM%d: Send NOP Cmd", this->port);
	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
		ERR("COM%d: Send VERREQ failure!");
		return result;
	}

	//START_BUILDING_RSP();
	memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
	rsp_ptr->type   = CMD_TYPE_DIAG;
	rsp_ptr->broken = false;
	rsp_ptr->length = MAX_RSP_BUFFER_LEN;

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result))
	{
		return result;
	}

	if (rsp_ptr->buf[0] == CMD_VERRSP)
	{
		ackcode = ACK_OK;
	}
	else
	{
		result = EACKFAILED;
	}

	if (ackcode == ACK_OK)
	{
		uint8 verlen = rsp_ptr->buf[1];
		uint8* pVer = new uint8[verlen + 1];

		if (pVer == NULL)
		{
			CRITICAL("COM%d: SendVerReq, out of memory!!!");
			return ENOMEMORY;
		}
		memcpy(pVer, &rsp_ptr->buf[2], verlen);
		*version = (char*)pVer;
	}

	return result;
}

#if 0
TResult CDLPrg::GetDeviceInfo(TDLDeviceInfo* pDevInfo)
{
#ifdef DEVICE_IMAGE_INFO_REQ_SUPPORT
#error code not present
#else
	return EOK;
#endif
}
#endif

TResult CDLPrg::SendGoCmd(int addrType)
{
	TResult result = EOK;
	uint32 rlen = 0;
	TAckCode ackcode;
	DECLARE_CMD_PTR(cmd_ptr);

        INFO("COM%d: Send GO Cmd",port);

	START_BUILDING_CMD(CMD_GO);

	//changed by jie.li for MDM9x15
	TypeAddr typeAddr = (TypeAddr)addrType;
	switch(typeAddr)
	{
	case TYPE_ADDR_NONE:
		break;

	case TYPE_ADDR_ONE:
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		cmd_ptr->buf[cmd_ptr->length++] = 0x80;
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		break;

	case TYPE_ADDR_TWO:
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		cmd_ptr->buf[cmd_ptr->length++] = 0x10;
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		break;

	case TYPE_ADDR_THREE:
		cmd_ptr->buf[cmd_ptr->length++] = 0x41;
		cmd_ptr->buf[cmd_ptr->length++] = 0x70;
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		cmd_ptr->buf[cmd_ptr->length++] = 0x00;
		break;

	default:
		break;
	}
	//end changed

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
        ERR("COM%d: Send GO Cmd failed!",port);
		return result;
	}

	result = GetAckCmd(&ackcode);
	if (FAILURE(result))
	{
        ERR("COM%d: Get Ack for GO failed!",port);
		return result;
	}

	if (ackcode != ACK_OK)
	{
        ERR("COM%d: Ack for GO error!",port);
		return EACKFAILED;
	}

	return result;
}


TResult CDLPrg::SendResetCmd(void)
{
	TResult result = EOK;
	uint32  rlen = 0;
	TAckCode ackcode;
	DECLARE_CMD_PTR(cmd_ptr);

	START_BUILDING_CMD(CMD_RESET);

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
		return result;
	}

	result = this->GetAckCmd(&ackcode);
	if (FAILURE(result))
	{
		return result;
	}

	if (ackcode != ACK_OK)
	{
		result = EACKFAILED;
	}

	return result;
}

TResult CDLPrg::SendWrite32Cmd(uint8* prgbuf, long len, int addrType)
{
	TResult  result = EOK;
	uint32   rlen = 0;
	uint32   addr = 0;
	uint32   dstaddr = 0;
	TAckCode ackcode;
	DECLARE_CMD_PTR(cmd_ptr);

        INFO("COM%d: Send WRITE32 Cmd",port);

	for (; len>SEND_PRG_PACKET_LEN; len-=SEND_PRG_PACKET_LEN, addr+=SEND_PRG_PACKET_LEN)
	{
		START_BUILDING_CMD(CMD_WRITE_32BIT);

		//add by jie.li for MDM9x15
		TypeAddr typeAddr = (TypeAddr)addrType;
		if (typeAddr == TYPE_ADDR_ONE)
		{
			dstaddr = addr + DLOAD_BASE;
		}
		else if (typeAddr == TYPE_ADDR_TWO)
		{
			dstaddr = addr + DLOAD_8200_BASE;
		}
		else if (typeAddr == TYPE_ADDR_THREE)
		{
			dstaddr = addr + DLOAD_9X15_BASE;
		}
		else
		{
			return EFAILED;
		}
		//end add

		/*
		if (!bWrite8200)
			dstaddr = addr + DLOAD_BASE;
		else
			//dstaddr = addr + DLOAD_8200_BASE;
			dstaddr = addr + DLOAD_9X15_BASE;  //add by jie.li for MDM9x15
		*/

		// address
		cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[3];
		cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[2];
		cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[1];
		cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[0];

		// len
		cmd_ptr->buf[cmd_ptr->length++] = SEND_PRG_PACKET_LEN >> 8;
		cmd_ptr->buf[cmd_ptr->length++] = SEND_PRG_PACKET_LEN & 0xFF;

		memcpy(&cmd_ptr->buf[cmd_ptr->length], prgbuf+addr, SEND_PRG_PACKET_LEN);
		cmd_ptr->length += SEND_PRG_PACKET_LEN;

		result = this->SendCmd(cmd_ptr, &rlen);

		if (FAILURE(result))
		{
            ERR("COM%d: Send WRITE32 Cmd failed!",port);
			return result;
		}
		if (rlen == 0)
		{
			return EPACKETSEND;
		}

		result = this->GetAckCmd(&ackcode);
		if (FAILURE(result))
		{
            ERR("COM%d: Get Ack for WRITE32 failed!",port);
			return result;
		}

		if (ackcode != ACK_OK)
		{
            ERR("COM%d: Ack for WRITE32 error!",port);
			return EACKFAILED;
		}
	}

	/* Send last packet with rest of the data */
	//add by jie.li for MDM9x15
	TypeAddr typeAddr = (TypeAddr)addrType;
	if (typeAddr == TYPE_ADDR_ONE)
	{
		dstaddr = addr + DLOAD_BASE;
	}
	else if (typeAddr == TYPE_ADDR_TWO)
	{
		dstaddr = addr + DLOAD_8200_BASE;
	}
	else if (typeAddr == TYPE_ADDR_THREE)
	{
		dstaddr = addr + DLOAD_9X15_BASE;
	}
	else
	{
		return EFAILED;
	}
	//end add

	/*
	if (!bWrite8200)
		dstaddr = addr + DLOAD_BASE;
	else
		//dstaddr = addr + DLOAD_8200_BASE;
		dstaddr = addr + DLOAD_9X15_BASE;  //add by jie.li for MDM9x15
	*/

	START_BUILDING_CMD(CMD_WRITE_32BIT);

	// address
	cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[3];
	cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[2];
	cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[1];
	cmd_ptr->buf[cmd_ptr->length++] = B_PTR(dstaddr)[0];

	// len
	cmd_ptr->buf[cmd_ptr->length++] = len >> 8;
	cmd_ptr->buf[cmd_ptr->length++] = len & 0xFF;

	memcpy(&cmd_ptr->buf[cmd_ptr->length], prgbuf+addr, len);
	cmd_ptr->length += len;

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result))
	{
        ERR("COM%d: Send WRITE32 Cmd failed!",port);
		return result;
	}
	if (rlen == 0)
	{
		return EPACKETSEND;
	}

	// check ACK_CMD
	result = GetAckCmd(&ackcode);
	if (FAILURE(result))
	{
        ERR("Get Ack for WRITE32 failed!",port);
		return result;
	}

	if (ackcode != ACK_OK)
	{
        ERR("Ack for WRITE32 error!",port);
		return EACKFAILED;
	}

	return result;
}

/*=============================================================================
DESCRIPTION
	Switch to download mode, preparing for downloading PRG.

DEPENDENCIES
	The device is in normal mode and can process the DIAG command.

RETURN VALUE

SIDE EFFECT

=============================================================================*/
TResult CDLPrg::SwitchToDLoadMode(void)
{
	bool result = true;
//	uint32 rlen = 0;
	CDIAGCmd diaginst(m_packetDll);

	result = diaginst.DLoadMode();

	return (result ? EOK : EFAILED);
}

bool CDLPrg::PingDevice(void)
{
	bool result = EOK;
//	uint32  rlen = 0;
	CDIAGCmd diaginst(m_packetDll);

	result = diaginst.WcdmaStatus();
	return result;
}


TResult CDLPrg::GetAckCmd(TAckCode* pAckCode)
{
	TResult result = EOK;
	uint32  rlen = 0;
	DECLARE_RSP_PTR(rsp_ptr);

	if (pAckCode == NULL)
	{
		return EINVALIDPARAM;
	}

    *pAckCode = ACK_END;  //invalid

	START_BUILDING_RSP();
	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result))
	{
		return result;
	}

	if (rsp_ptr->buf[0] == CMD_ACK)
	{
		*pAckCode = ACK_OK;
	}
	else if (rsp_ptr->buf[0] == CMD_NAK)
	{
		*pAckCode = (TAckCode)rsp_ptr->buf[2];
	}
	else
	{
		result = EACKPACKET;
	}

	return result;
}

TResult CDLPrg::SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen)
{
	if (cmd_ptr == NULL || rlen == NULL)
	{
		return EINVALIDPARAM;
	}

	return m_packetDll->Send(cmd_ptr, rlen);
}
