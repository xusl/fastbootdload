#ifndef SAHARACMD_H
#define SAHARACMD_H

/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2013-10-29  jie.li	   init first version

=============================================================================*/

#pragma once
#include "stdafx.h"
//#include "typedef.h"
#include "pkt.h"
#include "dlprg.h"

#define SAHARA_MODE_IMAGE_TX_PENDING 0x0		/*Image Transfer Pending mode*/
#define SAHARA_MODE_IMAGE_TX_COMPLETE 0x1		/*Image Transfer Complete mode*/
#define SAHARA_MODE_MEMORY_DEBUG 0x2			/*Memory Debug mode*/
#define SAHARA_MODE_COMMAND 0x3					/*Command mode*/


#define SAHARA_EXEC_CMD_GET_FLASH_TYPE 0x8
#define SAHARA_EXEC_CMD_GET_MOBILE_ID  0x2
#define SAHARA_EXEC_CMD_GET_FW_VERSION  0x7

enum {
    CMD_HELLO           = 0x00000001,
    CMD_HELLO_RSP       = 0x00000002,
    CMD_READ_DATA       = 0x00000003,
    CMD_END_TRANSFER    = 0x00000004,
    CMD_DONE            = 0x00000005,
    CMD_DONE_RSP        = 0x00000006,
    CMD_SAHARA_RESET    = 0x00000007,
    CMD_RESET_RSP       = 0x00000008,
    CMD_MEMORY_DEBUG    = 0x00000009,
    CMD_MEMORY_READ     = 0x0000000A,
    CMD_READY           = 0x0000000B,
    CMD_SWITCH_MODE     = 0x0000000C,
    CMD_EXECUTE         = 0x0000000D,
    CMD_EXECUTE_RSP     = 0x0000000E,
    CMD_EXECUTE_DADA    = 0x0000000F,

    CMD_SEND_DATA       = 0x00000FFF
};

enum
{
    SUCCESS                        = 0x00,
    EINVALID_COMMAND,
    EPROTOCOL_MISMATCH,
    EINVALID_TPROTOCOL_VERSION,
    EINVALID_HPROTOCOL_VERSION,
    EINVALID_PACKET_SIZE,
    EUNEXPECTED_IMAGE_ID,
    EINVALID_IMAGE_HEADER_SIZE,
    EINVALID_IMAGE_DATA_SIZE,
    EINVALID_IMAGE_TYPE,
    EINVALID_TRANSMISSION_LEN,
    EINVALID_RECEPTION_LEN,
    ERR_GENERAL_TRANSMISSION_OR_RECEPTION,
    ERR_TRANSMITTING_READ_DATA_PKT,
    ERR_RCV_SPECIFIED_PRG_HEADER,
    EINVALID_DATA_LEN_OR_PRG_HEADERS,
    ERR_MULTIPLE_SEGMENTS,
    ERR_UNINITIALIZED_PRG_HEADER_LOCATION,
    EINVALID_DESTINATION_ADDR,
    EINVALID_DATA_SIZE_IN_IMAGE_HEADER,
    EINVALID_ELF_HEADER,
    ERR_UNKNOWN_HELLO_RESP,
    ERR_RCV_DATA_TIMEOUT,
    ERR_TRANSMITTING_DATA_TIMEOUT,
    EINVALID_MODE_RCV_FR_HOST,
    EINVALID_MEMORY_READ_ACC,
    ERR_HOST_HANDLE_DADA,
    ERR_UNSUPPORT_MEMORY_DEBUG,
    EINVALID_MODE_SWITCH,
    ERR_EXECUTE_CMD,
    EINVALID_PARAMETER_CMD_EXECUTE,
    ERR_UNSUPPORT_CLIENT_CMD,
    EINVALID_CLIENT_CMD_RCV
};

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 versionNumber;		/*Version number of this protocol implementation*/
    int32 versionCompatible;	/*Lowest compatible version*/
    int32 commandPacketLength;	/*Maximum command packet length (in bytes) the protocol supports*/
    int32 mode;					/*Expected mode of target operation*/
    int32 reserved1;				/*Unused*/
    int32 reserved2;
    int32 reserved3;
    int32 reserved4;
    int32 reserved5;
    int32 reserved6;
} sahara_hello_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 versionNumber;		/*Version number of this protocol implementation*/
    int32 versionCompatible;	/*Lowest compatible version*/
    int32 status;				/*Success or error code*/
    int32 mode;					/*Mode of operation for target to execute*/
    int32 reserved1;				/*Unused*/
    int32 reserved2;
    int32 reserved3;
    int32 reserved4;
    int32 reserved5;
    int32 reserved6;
} sahara_hello_rsp_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 imageId;				/*ID of the image to be transferred*/
    int32 dataOffset;			/*Offset into the image file to start transferring data from*/
    int32 dataLength;			/*Number of bytes target wants to transfer from the image*/
} sahara_read_data_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 imageId;				/*ID of the image to be transferred*/
    int32 status;				/*Success or error code*/
} sahara_end_image_transger_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
} sahara_done_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 status;				/*Success or error code*/
} sahara_done_rsp_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
} sahara_reset_type;


typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 mode;
} sahara_switch_cmd_mode_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
} sahara_cmd_ready_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 ClientCMD;
} sahara_cmd_execute_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 ClientCMD;
    int32 rspLength;
} sahara_cmd_execute_rsp_pkt_type;

typedef struct{
    int32 command;				/*Command identifier code*/
    int32 length;				/*Length of packet (in bytes)*/
    int32 ClientCMD;
} sahara_cmd_execute_data_pkt_type;

typedef struct{
    int32 command;
    int32 length;
    int32 ClientCMD;
} sahara_cmd_execute_data_pkt_rsp_type;


class SAHARACmd
{
public:
    SAHARACmd(CPacket& packetDll);
    ~SAHARACmd();

public:
    //start download PRG
    TResult DownloadPrg(uint8* prgbuf, size_t len, int myPort,bool blDownLoadMode);
     TResult DownloadPrg_9X07(uint8* prgbuf, size_t len, int myPort,bool blDownLoadMode);

    //receive hello from device
    TResult GetHelloAckCmd();

    //send hello response to device
    TResult SaharaHelloRsp();

    //receive read data from device
    TResult GetReadDataAckCmd();
    TResult GetReadDataAckCmd_9X07_PRG();

    //receive end transfer from device
    TResult GetEndTransferAckCmd();

    //send done to device
    TResult SaharaDone(void);
    //receive done response from device
    TResult GetDoneRsp(void);

    TResult switchToCmdMode();

    TResult GetHelloAckCmdInDlMode();

    TResult GetCmdReadyRsp();

    TResult CmdExecute(int32 commandId);

    TResult CmdExecuteDataPkt(int32 commandId);

    TResult GetCmdExecuteRspPkt(int32* rspLength);

    TResult SendResetCmd();

    TResult saharaReceive(uint8* buf,int len);

    TResult saharaswitchMode(uint32 mode);

    TResult SwitchToDLoadMode(void);

private:
    TResult SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen);

    bool    PingDevice(void);

    TResult SendDoneCmd();
    TResult ReInitPacket(const char* info);

private:
    uint16   port;
    uint8*   imgBuf;
    uint32   imgLen;
    CPacket* m_packetDll;
    uint8    readDadaPkgCount;
    uint32   readDataOffSet;
    uint32   readDataLength;

    cmd_buffer_s_type cmd;
    rsp_buffer_s_type rsp;

};


#endif // SAHARACMD_H
