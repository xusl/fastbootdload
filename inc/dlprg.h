#ifndef __DLPRG_H__
#define __DLPRG_H__

#include "stdafx.h"
#include "pkt.h"

//-----------------------------------------------------------------------------
typedef enum
{
	ACK_OK = 0,
	ACK_INVALID_FCS,
	ACK_INVALID_DEST,
	ACK_INVALID_LEN,
	ACK_EARLY_END,
	ACK_TOO_LARGE,
	ACK_INVALID_CMD,
	ACK_FAILED,
	ACK_WRONG_IID,
	ACK_BAD_VPP,
	ACK_VERIFY_FAILED,
	ACK_NO_SEC_CODE,
	ACK_BAD_SEC_CODE,
	ACK_REV1,
	ACK_OP_NOT_PERMITTED,
	ACK_INVALID_ADDR,
	ACK_END,
	ACK_MAX = 0xFF // packet to 1 byte
}TAckCode;


typedef enum {
	EDLOADOK = 0,      // no error
	EDLOADNOP,         // send nop cmd error
	EDLOADGO,          // send go cmd error
	EDLOADWRITE,       // send write cmd error
	EDLOADRESET,       // send reset cmd error
	//...              // more error add here
	EDLOADMAX = 0xFF   // max error code (1 byte)
}TDLErrorEnumType;



//-----------------------------------------------------------------------------

class CDLPrg
{
public:
    CDLPrg(CPacket* packetDll);
	~CDLPrg();
    //TResult DownloadPrg(uint8* prgbuf, long len, bool bDL8200 = false);  //modify by huangzhongping 2011-03-24
    TResult DownloadPrg(uint8* prgbuf, size_t len, int addrType/*bool bDL8200 = false*/);  //changed by jie.li for MDM9x15
    TResult MySwitchToDLoadMode(void);  //changed by yanbin.wan 20130814 for switch device  port changed first time.
    TResult MyDownloadPrg(uint8* prgbuf, size_t len,int addrType,int myPort);
    //TResult GetDeviceInfo(TDLDeviceInfo* pDevInfo);
	TResult SendNopCmd(void);
	TResult SwitchToDLoadMode(void);
	bool    PingDevice(void);
    TResult RequestFlashType_D(uint8& type,uint8& Flash_Size ,bool bTypeDload = false); //changed for MDM9x15

	/*Add by jianwen.he 2010-06-25*/
	TResult RequestMobileID(uint32& mobileID,bool bTypeDload = false); //changed for MDM9x15
	TResult RequestBooVer(uint8& bootVer);
	/*end*/

private:
	TResult SendVerReq(char** version);
	TResult SendGoCmd(int addrType/*bool bGo8200 = false*/); //changed by jie.li for MDM9x15
	TResult SendResetCmd(void);
	TResult SendWrite32Cmd(uint8* prgbuf, long len, int addrType/*bool bWrite8200 = false*/); //changed by jie.li for MDM9x15

private:
	TResult GetAckCmd(TAckCode* pAckCode);
	TResult SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen);

	uint16   port;
	CPacket* m_packetDll;

	cmd_buffer_s_type cmd;
	rsp_buffer_s_type rsp;
};

//-----------------------------------------------------------------------------

#endif //_DL_PRG_H_
