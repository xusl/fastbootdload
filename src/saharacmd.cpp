
/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2013-10-29  jie.li	   init first version

=============================================================================*/


#include "stdafx.h"
#include "saharacmd.h"
#include "diagcmd.h"
#include "log.h"

#define DECLARE_CMD_PTR(cmd_ptr) \
    cmd_buffer_s_type* cmd_ptr = &this->cmd

#define DECLARE_RSP_PTR(rsp_ptr) \
    rsp_buffer_s_type* rsp_ptr = &this->rsp

#define START_BUILDING_CMD(cmd_code) \
    start_building_dload_cmd(cmd_ptr, cmd_code)

#define START_BUILDING_RSP() \
    start_building_dload_rsp(rsp_ptr)


static void start_building_dload_cmd(cmd_buffer_s_type* cmd_ptr, uint8 cmd_code)
{
    memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
    cmd_ptr->type   = CMD_TYPE_SAHARA;
    cmd_ptr->broken = FALSE;
    cmd_ptr->length = 0;
}

static void start_building_dload_rsp(rsp_buffer_s_type* rsp_ptr)
{
    memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
    rsp_ptr->type   = CMD_TYPE_SAHARA;
    rsp_ptr->broken = FALSE;
    //rsp_ptr->length = MAX_RSP_BUFFER_LEN;
    rsp_ptr->length = 50;
}

SAHARACmd::SAHARACmd(CPacket* packetDll)
{
    m_packetDll = packetDll;
    m_packetDll->SetPacketType(PKT_TYPE_SAHARA);
    this->port = m_packetDll->GetComPort();

    readDadaPkgCount = 0;
    readDataOffSet = 0;
    readDataLength = 0;
}

SAHARACmd::~SAHARACmd()
{

}

TResult SAHARACmd::SwitchToDLoadMode(void)
{
    bool result = TRUE;
    CDIAGCmd diaginst(m_packetDll);

    result = diaginst.DLoadMode();
    LOGW("COM%d SwitchToDLoadMode", port);

    return (result ? EOK : EFAILED);
}
TResult SAHARACmd::DownloadPrg_9X07(uint8* prgbuf, size_t len,int myPort, bool blDownLoadMode)
{
    TResult result = EOK;
    imgBuf = prgbuf;

    imgLen = len;


#ifdef Q_OS_WIN32
    Sleep(3000);
#else
    SLEEP(3000);
#endif

    for(int i = 0; i < 5;i++)
    {
        //wait to receive hello package
        if(blDownLoadMode)
        {
            result = GetHelloAckCmdInDlMode();
        }
        else
        {
            result = GetHelloAckCmd();
        }
        if(SUCCESS(result))
        {
            break;
        }
        if(i == 4)
        {
            // return EFAILED;
        }
    }
    //send hello response package
    SaharaHelloRsp();
//#ifdef download_9X07_PRG
    //result = GetReadDataAckCmd_9X07_PRG();
//#else


        result = GetReadDataAckCmd_9X07_PRG();

    SaharaDone();
    return EOK;

    for(int i = 0; i < 5;i++)
    {
        result = GetDoneRsp();

        if(SUCCESS(result))
        {
            break;
        }
        if(i == 4)
        {
            return EFAILED;
        }
    }

    return result;
}

TResult SAHARACmd::DownloadPrg(uint8* prgbuf, size_t len,int myPort, bool blDownLoadMode)
{
    TResult result = EOK;
    imgBuf = prgbuf;
    imgLen = len;

    LOGW("COM%d DownloadPrg", port);
    /* Switch device to download mode first */

   /* if(!blDownLoadMode)
    {
        result = this->SwitchToDLoadMode();
    }*/
    /* The device takes some time to enter download mode */

    SLEEP(3000);

    for(int i = 0; i < 5;i++)
    {
        //wait to receive hello package
        if(blDownLoadMode)
        {
            result = GetHelloAckCmdInDlMode();
        }
        else
        {
           result = GetHelloAckCmd();
        }
        if(SUCCESS(result))
        {
            break;
        }
        if(i == 4)
        {
           // return EFAILED;
        }
    }

    //send hello response package
    SaharaHelloRsp();

    for(int i = 0; i < 2; i++)
    {
        //receive read data package
        result = GetReadDataAckCmd();
    }
    for(int i = 0; i < 5; i++)
    {
        result = GetEndTransferAckCmd();
        if(SUCCESS(result))
        {
            break;
        }
    }

    SaharaDone();

    for(int i = 0; i < 5;i++)
    {
        result = GetDoneRsp();

        if(SUCCESS(result))
        {
            break;
        }
        if(i == 4)
        {
            return EFAILED;
        }
    }

    return result;
}

TResult SAHARACmd::GetHelloAckCmd()
{
    TResult result = EOK;
    uint32  rlen = 0;
    sahara_hello_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();
    result = m_packetDll->Receive(rsp_ptr, &rlen);
    if (FAILURE(result))
    {
        ReInitPacket("GetHelloAckCmd:fail to receive Hello");
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);
    if (rsp.command != CMD_HELLO)
    {
        ReInitPacket("GetHelloAckCmd:receive not Hello");
        result = EFAILED;
    }
    return result;
}


TResult SAHARACmd::GetHelloAckCmdInDlMode()
{
    TResult result = EOK;
    uint32  rlen = 48;
    sahara_hello_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();

    result = m_packetDll->SaharaDLReceive(rsp_ptr,rlen);
    if (FAILURE(result))
    {
        ReInitPacket("Download mode,GetHelloAckCmd:fail to receive Hello");
        m_packetDll->Uninit();
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);

    if (rsp.command != CMD_HELLO)
    {
        ReInitPacket("Download mode,GetHelloAckCmd:receive not Hello");
        result = EFAILED;
    }
    return result;
}

TResult SAHARACmd::GetCmdReadyRsp()
{
    TResult result = EOK;
    uint32  rlen = 8;
    sahara_cmd_ready_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();

    result = m_packetDll->SaharaDLReceive(rsp_ptr,rlen);
    if (FAILURE(result))
    {
        ReInitPacket("Download mode,GetCmdReadyRsp:fail to receive command ready");
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);

    if (rsp.command != CMD_READY)
    {
        ReInitPacket("Download mode,GetCmdReadyRsp:receive not command ready");
        result = EFAILED;
    }
    return result;

}

TResult SAHARACmd::CmdExecute(int32 commandId)
{

    TResult result = EOK;
    uint32 rlen = 0;
    sahara_cmd_execute_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_EXECUTE;
    req.length = 0x0000000C;
    req.ClientCMD = commandId;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_EXECUTE);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    if (FAILURE(result))
    {
        ERR("COM%d Send CmdExecute failed",port);
        return result;
    }

    return result;
}

TResult SAHARACmd::CmdExecuteDataPkt(int32 commandId)
{

    TResult result = EOK;
    uint32 rlen = 0;
    sahara_cmd_execute_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_EXECUTE_DADA;
    req.length = 0x0000000C;
    req.ClientCMD = commandId;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_EXECUTE_DADA);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    if (FAILURE(result))
    {
        ERR("COM%d Send CmdExecuteDataPkt failed",port);
        return result;
    }

    return result;

}

TResult SAHARACmd::GetCmdExecuteRspPkt(int32* rspLength)
{
    TResult result = EOK;
    uint32  rlen = 16;
    sahara_cmd_execute_rsp_pkt_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();

    result = m_packetDll->SaharaDLReceive(rsp_ptr,rlen);
    if (FAILURE(result))
    {
        ReInitPacket("Download mode,GetCmdExecuteRspPkt:fail to receive ExecuteRspPkt");
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);

    if (rsp.command != CMD_EXECUTE_RSP)
    {
        ReInitPacket("Download mode,GetCmdExecuteRspPkt:receive not ExecuteRspPkt");
        result = EFAILED;
    }

    *rspLength = rsp.rspLength;
    return result;

}

TResult SAHARACmd::saharaReceive(uint8* buf,int len)
{
    TResult result = EOK;
    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();

    result = m_packetDll->SaharaDLReceive(rsp_ptr,len);
    if (FAILURE(result))
    {
        ReInitPacket("Download mode,GetCmdExecuteRspPkt:fail to receive ExecuteRspPkt");
        return result;
    }
    memcpy(buf, rsp_ptr->buf, len);

    return result;
}

TResult SAHARACmd::SaharaHelloRsp()
{
    TResult result = EOK;
    uint32 rlen = 0;
    sahara_hello_rsp_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_HELLO_RSP;
    req.length = 0x00000030;
    req.versionNumber = 0x00000002;
    req.versionCompatible = 0x00000001;
    req.mode = 0;
   // req.mode = mode;
    //req.status = 0;
    req.status = SUCCESS;


    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_HELLO_RSP);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    INFO("COM%d SaharaHelloRsp:send %04X",port,*(cmd_ptr->buf));
    if (FAILURE(result))
    {
        ERR("COM%d Send Sahara hello respose Cmd failed",port);
        return result;
    }

    return result;
}

TResult SAHARACmd::GetDoneRsp(void)
{
    TResult result = EOK;
    uint32  rlen = 0;
    sahara_done_rsp_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();
    result = m_packetDll->Receive(rsp_ptr, &rlen);
    if (FAILURE(result))
    {
        ERR("COM%d GetDoneRsp:fail to receive data",port);
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);
    if (rsp.command != CMD_DONE_RSP)
    {
        ERR("COM%d GetDoneRsp:receive not done respone",port);
        result = EFAILED;
    }
    return result;
}
TResult SAHARACmd::GetReadDataAckCmd_9X07_PRG()
{
    TResult result = EOK;
    uint32  rlen = 0;
    sahara_read_data_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();
    while(true)
    {
        result = m_packetDll->Receive(rsp_ptr, &rlen);
        INFO("COM%d GetReadDataAckCmd:receive %04X",port,*(rsp_ptr->buf));
        if (FAILURE(result))
        {
            ERR("COM%d GetReadDataAckCmd:fail to get data ",port);
            return result;
        }
        memcpy(&rsp, rsp_ptr->buf, rlen);
        if (rsp.command != CMD_READ_DATA)
        {
            ERR("COM%d GetReadDataAckCmd:not CMD_READ_DATA",port);
            return EFAILED;
        }


        if(readDataOffSet != rsp.dataOffset || readDataLength != rsp.dataLength)
        {
            readDataOffSet = rsp.dataOffset;
            readDataLength = rsp.dataLength;

            //uint8 buf[rsp.dataLength];
        //uint8 buf[rsp.dataLength];
        uint8 *buf = (uint8 *)malloc(sizeof uint8 * rsp.dataLength);

        if(buf == NULL){
            LOGE("No memory, malloc failed.");
            return EFAILED;
        }

            memcpy(buf, imgBuf + rsp.dataOffset, rsp.dataLength);
            //send PRG
            m_packetDll->SendData(buf,rsp.dataLength);
            free(buf);
        }
        else
            break;
    }

    return result;
}

TResult SAHARACmd::GetReadDataAckCmd()
{
    TResult result = EOK;
    uint32  rlen = 0;
    sahara_read_data_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();
    result = m_packetDll->Receive(rsp_ptr, &rlen);
    INFO("COM%d GetReadDataAckCmd:receive %04X",port,*(rsp_ptr->buf));
    if (FAILURE(result))
    {
        ERR("COM%d GetReadDataAckCmd:fail to get data ",port);
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);
    if (rsp.command != CMD_READ_DATA)
    {
        ERR("COM%d GetReadDataAckCmd:not CMD_READ_DATA",port);
        return EFAILED;
    }

    if(readDataOffSet != rsp.dataOffset || readDataLength != rsp.dataLength)
    {
        readDataOffSet = rsp.dataOffset;
        readDataLength = rsp.dataLength;

        //uint8 buf[rsp.dataLength];
        uint8 *buf = (uint8 *)malloc(sizeof uint8 * rsp.dataLength);

if(buf == NULL){
    LOGE("No memory, malloc failed.");
    return EFAILED;
}
        memcpy(buf, imgBuf + rsp.dataOffset, rsp.dataLength);
        //send PRG
        m_packetDll->SendData(buf,rsp.dataLength);
        free(buf);
    }

    return result;
}

TResult SAHARACmd::GetEndTransferAckCmd()
{
    TResult result = EOK;
    uint32  rlen = 0;
    sahara_end_image_transger_type rsp = {0};

    DECLARE_RSP_PTR(rsp_ptr);
    START_BUILDING_RSP();
    result = m_packetDll->Receive(rsp_ptr, &rlen);
    if (FAILURE(result))
    {
        ERR("COM%d fail receive end transfer command",port);
        return result;
    }
    memcpy(&rsp, rsp_ptr->buf, rlen);
    if (rsp.command != CMD_END_TRANSFER)
    {
        ERR("COM%d receive not end transfer",port);
        result = EFAILED;
    }
    return result;
}

TResult SAHARACmd::SaharaDone(void)
{
    TResult result = EOK;
    uint32 rlen = 0;
    sahara_done_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_DONE;
    req.length = 0x00000008;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_DONE);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    if (FAILURE(result))
    {
        ERR("COM%d Sahara Send Done failed",port);
        return result;
    }

    return result;
}


TResult SAHARACmd::SendDoneCmd()
{
    TResult result = EOK;
    uint32 rlen = 0;
    sahara_done_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_DONE;
    req.length = 0x00000008;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_SAHARA_RESET);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    if (FAILURE(result))
    {
        INFO("COM%d: Send Sahara done Cmd failed! errcode = ", this->port,result);
        return result;
    }

    SLEEP(100);

   // GetResponseData();
   return result;
}

TResult SAHARACmd::SendResetCmd()
{
    TResult result = EOK;
    uint32 rlen = 0;
    sahara_hello_rsp_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_SAHARA_RESET;
    req.length = 0x00000008;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_HELLO_RSP);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    if (FAILURE(result))
    {
        INFO("COM%d: Send Sahara reset Cmd failed! errcode = ", this->port,result);
        return result;
    }

    SLEEP(100);

    //GetResponseData();
    return result;
}

TResult SAHARACmd::saharaswitchMode(uint32 mode)
{
    TResult result = EOK;
    uint32 rlen = 0;
    sahara_switch_cmd_mode_type req = {0};
    uint32 len  = sizeof(req);

    req.command = 0x0000000C;
    req.length = 0x0000000C;
    req.mode = mode;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_SWITCH_MODE);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);

    if (FAILURE(result))
    {
        ERR("COM%d Send saharaswitchMode fail",port);
        return result;
    }
    return result;
}

TResult SAHARACmd::switchToCmdMode()
{
    TResult result = EOK;
    uint32 rlen = 0;
    sahara_hello_rsp_type req = {0};
    uint32 len  = sizeof(req);

    req.command = CMD_HELLO_RSP;
    req.length = 0x00000030;
    req.versionNumber = 0x00000002;
    req.versionCompatible = 0x00000001;
    req.mode = SAHARA_MODE_COMMAND;

    req.status = SUCCESS;

    DECLARE_CMD_PTR(cmd_ptr);
    START_BUILDING_CMD(CMD_HELLO_RSP);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    result = this->SendCmd(cmd_ptr, &rlen);
    INFO("COM%d SaharaHelloRsp,switch to cmd mode",port,*(cmd_ptr->buf));
    if (FAILURE(result))
    {
        ERR("COM%d Send Sahara hello respose Cmd,switch to cmd mode failed",port);
        return result;
    }

   // Sleep(1000);
    return result;
}


TResult SAHARACmd::SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen)
{
    if (cmd_ptr == NULL || rlen == NULL)
    {
        return EINVALIDPARAM;
    }

    return m_packetDll->Send(cmd_ptr, rlen);
}


TResult SAHARACmd::ReInitPacket(const char* info)
{
    LOGE("COM%d '%s'", port, info);
    m_packetDll->Uninit();
    m_packetDll->Init((unsigned short)port);

    return EOK;
}