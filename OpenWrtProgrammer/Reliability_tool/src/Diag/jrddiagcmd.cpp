#include "jrddiagcmd.h"

#define MAX_RESEND (2)

#define DECLARE_CMD_PTR(cmd_ptr) \
        cmd_buffer_s_type* cmd_ptr = &this->cmd

#define DECLARE_RSP_PTR(rsp_ptr) \
        rsp_buffer_s_type* rsp_ptr = &this->rsp

#define INITIATING_CMD(cmd_ptr) \
        initiating_diag_cmd(cmd_ptr)

#define START_BUILDING_CMD(cmd_code) \
        start_building_diag_cmd(cmd_ptr, cmd_code)

#define START_BUILDING_SUBSYS_CMD(cmd_code, subsys_id, subsys_cmd_code) \
        start_building_extend_diag_cmd(cmd_ptr, cmd_code, subsys_id, subsys_cmd_code)

#define START_BUILDING_RSP() \
        start_building_diag_rsp(rsp_ptr)

static void initiating_diag_cmd
(
      cmd_buffer_s_type* cmd_ptr
)
{
    //ASSERT(cmd_ptr != NULL);
    memset(cmd_ptr->buf, 0, sizeof(cmd_ptr->buf));
    cmd_ptr->type   = CMD_TYPE_DIAG;
    cmd_ptr->broken = false;
    cmd_ptr->length = 0;
}

static void start_building_diag_cmd
(
      cmd_buffer_s_type* cmd_ptr,
     uint8 cmd_code
)
{
    //ASSERT(cmd_ptr != NULL);
    memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
    cmd_ptr->type   = CMD_TYPE_DIAG;
    cmd_ptr->broken = false;
    cmd_ptr->buf[0] = cmd_code;
    cmd_ptr->length = 1;
}


static void start_building_diag_rsp
(
      rsp_buffer_s_type* rsp_ptr
)
{
    memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
    rsp_ptr->type   = CMD_TYPE_DIAG;
    rsp_ptr->broken = false;
    /* length might need to be reset according to specific cmd */
    rsp_ptr->length = MAX_RSP_BUFFER_LEN;
}

static bool check_diag_cmd_ok
(
    cmd_buffer_s_type* cmd_ptr
)
{
    if (cmd_ptr == NULL)
    {
        return false;
    }

    if (cmd_ptr->broken == true)
    {
        ERROR(FILE_LINE, "check_diag_cmd_ok, cmd_ptr is broken!");
        return false;
    }

    if (cmd_ptr->length >= MAX_CMD_BUFFER_LEN)
    {
        ERROR(FILE_LINE, "check_diag_cmd_ok, cmd_ptr is broken!");
        cmd_ptr->broken = true;
        return false;
    }

    return true;
}

static bool check_diag_rsp_ok
(
     cmd_buffer_s_type* cmd_ptr,
     rsp_buffer_s_type* rsp_ptr
)
{
    if ((cmd_ptr->buf[0] != rsp_ptr->buf[0]) || (cmd_ptr->buf[1] != rsp_ptr->buf[1]) ||
        (cmd_ptr->buf[2] != rsp_ptr->buf[2]) || (cmd_ptr->buf[3] != rsp_ptr->buf[3]))
    {
        ERROR(FILE_LINE, "check_diag_rsp_ok failure, cmd_code = %d %d %d %d, rsp_code = %d %d %d %d",cmd_ptr->buf[0],cmd_ptr->buf[1],cmd_ptr->buf[2],cmd_ptr->buf[3],rsp_ptr->buf[0],rsp_ptr->buf[1],rsp_ptr->buf[2],rsp_ptr->buf[3]);

        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

JRDdiagCmd::JRDdiagCmd(CPacket& packetDll)
{
    m_packetDll = &packetDll;
}


TResult JRDdiagCmd::EnableDiagServer()
{
    TResult result = EOK;

    jrd_diag_sys_port_lock_req_type	req;
    jrd_diag_sys_port_lock_rsp_type rsp;

    memset(&rsp, 0, sizeof(jrd_diag_sys_port_lock_rsp_type));
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_PORT_LOCK;

    req.hdr = header;
    req.lock = E_JRD_UNLOCK;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Enable diag server, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Enable diag server failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Enable diag server failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "Enable diag server failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "Enable diag server check_diag_rsp_ok failure!");
         return EFAILED;
        //return EOK;                                          //only for test, if normal return EFAILED20140915
    }

    memcpy(&rsp,rsp_ptr->buf,sizeof(rsp));
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
         return EFAILED;
    }

    return EOK;
}

TResult JRDdiagCmd::DisableDiagServer()
{
    TResult result = EOK;

    jrd_diag_sys_port_lock_req_type	req;
    jrd_diag_sys_port_lock_rsp_type rsp;

    memset(&rsp, 0, sizeof(jrd_diag_sys_port_lock_rsp_type));
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_PORT_LOCK;

    req.hdr = header;
    req.lock = E_JRD_LOCK;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Disable diag server, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Disable diag server failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Disable diag server failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "Disable diag server failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "Disable diag server check_diag_rsp_ok failure!");
        return EFAILED;
    }

    memcpy(&rsp,rsp_ptr->buf,sizeof(rsp));
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
         return EFAILED;
    }

    return EOK;
}

#ifdef FEATHER_CUSTOM_IDEA
TResult JRDdiagCmd::ReadIMEI(QString & strImei)
{

}
TResult JRDdiagCmd::NewEnableDiagServer(QString strCode)
{

}
TResult JRDdiagCmd::NewDisableDiagServer()
{

}
#endif

TResult JRDdiagCmd::RequestVersion(uint8 whichVer,char* fdata)
{

    TResult result = EOK;

    jrd_diag_get_version_req_type	req;
    jrd_diag_get_version_rsp_type   rsp;

    memset(&rsp, 0, sizeof(jrd_diag_sys_port_lock_rsp_type));
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_GET_VER;

    req.hdr = header;
    req.which_version = whichVer;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        //if(E_JRD_LINUXKER_VERSION==whichVer)

        ERROR(FILE_LINE, "RequestVersion, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "RequestVersion failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result)||rsp_ptr->buf[0]==0)
        {
            LOG(FILE_LINE, "RequestVersion receive failure, result: %d!", result);

            continue;
        }

        break;
    }
    if (FAILURE(result))
    {
        //if(E_JRD_LINUXKER_VERSION==whichVer)

        ERROR(FILE_LINE, "----RequestVersion failure----, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "check  RequestVersion response failure!");
        if(E_JRD_LINUXKER_VERSION==whichVer)

        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(jrd_diag_get_version_rsp_type));
    memcpy(fdata,rsp.version,sizeof(rsp.version));



    return EOK;
}
QString JRDdiagCmd::Get_Hardversion(char *HardWare)
{
    QString hardware_version;
    if(HardWare[20]=='\0')
        hardware_version="NULL";
    else
        if(HardWare[20]=='A')
            hardware_version="01";
    else
        if(HardWare[20]=='B')
            hardware_version="02";
    else
        if(HardWare[20]=='C')
            hardware_version="03";
    else
        if(HardWare[20]=='D')
            hardware_version="04";
    else
        if(HardWare[20]=='E'||HardWare[20]=='F'||HardWare[20]=='G')
            hardware_version="05";
    else
        if(HardWare[20]=='H'||HardWare[20]=='I'||HardWare[20]=='J')
            hardware_version="06";
    else
        if(HardWare[20]=='K'||HardWare[20]=='L'||HardWare[20]=='M')
            hardware_version="07";
    else
        if(HardWare[20]=='N'||HardWare[20]=='O'||HardWare[20]=='P')
            hardware_version="08";
    else
        if(HardWare[20]=='Q'||HardWare[20]=='R'||HardWare[20]=='S')
            hardware_version="09";
    else
        if(HardWare[20]=='T'||HardWare[20]=='U'||HardWare[20]=='V')
            hardware_version="10";
    else
        if(HardWare[20]=='W'||HardWare[20]=='X'||HardWare[20]=='Y'||HardWare[20]=='Z')
            hardware_version="11";
    else
        hardware_version="NULL";
    return hardware_version;

}

TResult JRDdiagCmd::RequestHardwareVersion(char *HardWare)
{
    TResult result = EOK;

    jrd_diag_sys_get_hardware_ver_req_type   req;
    jrd_diag_sys_get_hardware_ver_req_type   rsp;

    memset(&rsp, 0, sizeof(jrd_diag_sys_get_hardware_ver_req_type));
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_TRACE_INFO;

    req.hdr = header;
    req.event_type=E_JRD_LEDS_EXIT_MSG_ID;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "RequestVersion, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "RequestVersion failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "RequestVersion receive failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "----RequestVersion failure----, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "check  RequestVersion response failure!");
        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(jrd_diag_sys_get_hardware_ver_req_type));
    memcpy(HardWare,rsp.traceinfo,sizeof(rsp.traceinfo));
    return EOK;

}




TResult JRDdiagCmd::RequestFlashType_N(uint8& type)
{
    TResult result = EOK;

    jrd_diag_sys_get_flash_type_req_type	req;
    jrd_diag_sys_get_flash_type_rsp_type    rsp;

    memset(&rsp, 0, sizeof(jrd_diag_sys_get_flash_type_rsp_type));
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_GET_FLASH_TYPE_INFO;

    req.hdr = header;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Request flash type, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Request flash type failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Request flash type receive failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "----Request flash type failure----, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "check  Request flash type response failure!");
        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(jrd_diag_sys_get_flash_type_rsp_type));
    if (rsp.diag_errno == E_JRD_FAILURE)
    {
        return EFAILED;
        ERROR(FILE_LINE, "Request flash type response failure");
    }
    type = rsp.flash_type;
    return EOK;
}

TResult JRDdiagCmd::RequestFlashType_9X25(char *Flash_Type)
{
    TResult result = EOK;

    jrd_diag_sys_get_flash_type_req_type	req;
    jrd_diag_sys_get_flash_type_9X25_rsp_type    rsp;

    memset(&rsp, 0, sizeof(jrd_diag_sys_get_flash_type_9X25_rsp_type));
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_GET_FLASH_TYPE_INFO;

    req.hdr = header;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Request flash type, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Request flash type failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Request flash type receive failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "----Request flash type failure----, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "check  Request flash type response failure!");
        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(jrd_diag_sys_get_flash_type_9X25_rsp_type));
    if (rsp.diag_errno == E_JRD_FAILURE)
    {
        return EFAILED;
        ERROR(FILE_LINE, "Request flash type response failure");
    }
    memcpy(Flash_Type,rsp.flash_type,sizeof(rsp.flash_type));
    return EOK;
}
TResult JRDdiagCmd::StorePIC(uint8* data, uint32 len)
{
    TResult result = EOK;
    jrd_diag_sys_store_boot_image_req_type req = {0};
    jrd_diag_sys_store_boot_image_rsp_type rsp = {0};

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_STORE_BOOT_IMAGE;

    uint32 rlen = 0;

    req.hdr = header;
    req.pic_len = len;
    memcpy(&req.pic, data, len);

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, sizeof(req));
    cmd_ptr->length += sizeof(req);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Store pic, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Store pic failure, result: %d!", result);
            continue;
        }

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Store pic failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "Store pic failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "Store pic check_diag_rsp_ok failure!");
        return EFAILED;
    }

    memcpy(&rsp,rsp_ptr->buf,sizeof(rsp));
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
         return EFAILED;
    }

    return EOK;
}

TResult JRDdiagCmd::ReadMacAddress(QString & strMacAddress, int iType)
{

}

TResult JRDdiagCmd::ReadWifiPwdFlag(int &retFlagVal)
{

}

TResult JRDdiagCmd::SetFuncFive(byte valueFuncFive)
{
    TResult result = EOK;

    jrd_diag_sys_usb_switch_req_type	req;
    jrd_diag_sys_usb_switch_rsp_type    rsp;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_USB_SWITCH;

    req.hdr = header;
    req.value = valueFuncFive;

    uint32 len = sizeof(req);
    uint32  rlen = 0;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += sizeof(req);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "SetFuncFive, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "SetFuncFive command send fail, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "SetFuncFive receive failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "SetFuncFive failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "SetFuncFive check_diag_rsp_ok failure!");
        return EFAILED;
    }

    rsp = *(jrd_diag_sys_usb_switch_rsp_type *)(rsp_ptr->buf);
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
        ERROR(FILE_LINE, "SetFuncFive E_JRD_SUCCESS != rsp.diag_errno");
        return EFAILED;
    }

    LOG(FILE_LINE, "SetFuncFive succssefull, result: %d!", result);

    return EOK;

}

TResult JRDdiagCmd::GenerateFTFilesNew(int32 value)
{
    TResult result = EOK;
    jrd_diag_sys_backup_handle_req_type 	req = {0};
    jrd_diag_sys_backup_handle_rsp_type 	rsp = {0};

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_SYS_BACKUP_HANDLE;

    req.hdr = header;
    req.type = value;

    uint32  rlen = 0;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, sizeof(req));
    cmd_ptr->length += sizeof(req);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "GenerateFTFilesNew, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "GenerateFTFilesNew failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "GenerateFTFilesNew failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "GenerateFTFilesNew failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "GenerateFTFilesNew check_diag_rsp_ok failure!");
        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(rsp));
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
         return EFAILED;
    }

    LOG(FILE_LINE, "GenerateFTFilesNew successfully, result: %d!", result);

    return EOK;
}

TResult JRDdiagCmd::GetConfigXml(int32 offset,uint8 whichXml,char* content,int32* cur_len, int32* remain_len)
{

    TResult result = EOK;
    jrd_diag_sys_read_write_config_xml_req_type 	req = {0};
    jrd_diag_sys_read_write_config_xml_rsp_type 	rsp = {0};

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_READ_WRITE_CONFIG_XML;

    req.hdr = header;
    req.rw_flag = E_JRD_READ;
    req.which_xml = whichXml;
    req.offset = offset;

    uint32  rlen = 0;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, sizeof(req));
    cmd_ptr->length += sizeof(req);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "GetConfigXml, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "GetConfigXml failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "GetConfigXml failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "GetConfigXml failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "GetConfigXml check_diag_rsp_ok failure!");
        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(rsp));
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
         return EFAILED;
    }

    memcpy(content,&rsp.content,sizeof(rsp.content));
    memcpy(cur_len,&rsp.cur_len,sizeof(rsp.cur_len));
    memcpy(remain_len,&rsp.remain_len,sizeof(rsp.remain_len));

    return EOK;
}

TResult JRDdiagCmd::WriteConfigXml(int32 offset,uint8 whichXml,char* content,int32 len)
{
    TResult result = EOK;
    jrd_diag_sys_read_write_config_xml_req_type 	req = {0};
    jrd_diag_sys_read_write_config_xml_rsp_type 	rsp = {0};

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_SYS;
    header.cmd_code = E_JRD_DIAG_READ_WRITE_CONFIG_XML;

    req.hdr = header;
    req.rw_flag = E_JRD_WRITE;
    req.which_xml = whichXml;
    req.offset = offset;
    req.len = len;
    memcpy(req.content,content,len);

    uint32  rlen = 0;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, sizeof(req));
    cmd_ptr->length += sizeof(req);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "write ConfigXml, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < 10; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "write ConfigXml failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "write ConfigXml failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "write ConfigXml failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "write ConfigXml check_diag_rsp_ok failure!");
        return EFAILED;
    }
    memcpy(&rsp,rsp_ptr->buf,sizeof(rsp));
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
         return EFAILED;
    }

   // memcpy(content,&rsp.content,sizeof(rsp.content));

    return EOK;
}

TResult JRDdiagCmd::EraseSimLock()
{
    TResult result = EOK;

    jrd_diag_sim_lock_req_type	req;
    jrd_diag_sim_lock_rsp_type  rsp;

    jrd_diag_hdr_type header;
    header.cmd_entry = JRD_DIAG_CMD_F;
    header.class_code = E_JRD_DIAG_PERIPHERAL;
    header.cmd_code = E_JRD_DIAG_PERI_SIM_LOCK;

    req.hdr = header;
    req.op_type = E_JRD_ERASE;

    uint32 len = sizeof(req);
    uint32  rlen = 0;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += sizeof(req);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Erase simlock, cmd is not ok!");
        return EFAILED;
    }

    for (int i = 0; i < MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Erase simlock, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Erase simlock receive failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "Erase simlock failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "Erase simlock check_diag_rsp_ok failure!");
        return EFAILED;
    }

    rsp = *(jrd_diag_sim_lock_rsp_type *)(rsp_ptr->buf);
    if (E_JRD_SUCCESS != rsp.diag_errno)
    {
        ERROR(FILE_LINE, "Erase simlock,E_JRD_SUCCESS != rsp.diag_errno!");
        return EFAILED;
    }

    return EOK;
}

TResult JRDdiagCmd::SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen)
{
    if (cmd_ptr == NULL || rlen == NULL)
    {
        return EINVALIDPARAM;
    }

    return m_packetDll->Send(cmd_ptr, rlen);
}

JRDdiagCmd::~JRDdiagCmd()
{

}

