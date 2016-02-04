#ifndef __DLDATA_H__
#define __DLDATA_H__

#include "stdafx.h"
#include "pkt.h"
#include "utils.h"
//#include "WinMsgProcessApp.h"

//class WinMsgProApp;
//extern WinMsgProApp *mainApp;

class CPacket;

   typedef void (*ProgressCallback)(int port,uint16 percent);
   typedef void (*updateDLstate)(int port,string msg);

class CDLData
{
public:
        CDLData(CPacket& packetDll, TDLImgInfoType* pDLImgInfo,int dlPort);
	~CDLData(void);

	void    SetRatioParams(uint8 ratio, uint8 base);
	TResult SendHelloPacket(void);
	TResult SendResetCmd(void);
        static  ProgressCallback func;
        void    RegisterCallback(ProgressCallback callback);

        //add by minghui.zhang 2013-11-05   delete
        TResult DLoad9X25ImagesUsePtn(map<string,FileBufStruct> &FileBufMap,  uint32 Software_size);
        TResult DLoad9X07ImagesUsePtn(map<string,FileBufStruct> &FileBufMap,  uint32 Software_size);
        //end add


        void setDlImgInfo(TDLImgInfoType*   pDLImgInfo);    //add by minghui.zhang 2014/09/17

private:
        //TResult DownloadData(uint8* pData, long Len, int Mode);
        TResult DownloadDataUsePrtn(uint8* pData, uint32 Len, uint8 * Mode);  //add by jie.li for MDM9x15

	/* Packet functions */
	TResult SendDummyData(void);
	TResult SendSecMode(void);
	TResult SendPrtnTbl(uint8* pData, uint32 len, bool bOverride=false);

	TResult SendOpenPacket(uint8 mode);
        TResult SendWritePacket(uint8* pData, int Len);
	TResult SendClosePacket(void);
	//TResult SendResetCmd(void);
        TResult SendOpenPacketUsePrtn(uint8 *mode);			//add by jie.li for MDM9x15

        TResult WriteDashboardVer(char* ver);

	/* Helper functions */
	TResult SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen);
	TResult GetAck(int& AckCode, int& ErrCode, char** ErrMsg);
	TResult CheckAckError(int AckCode, int ErrCode);

	/* UI displaying functions */
	void UpdateProgress(uint8 percent);


        CPacket*             m_packetDll;

        TDLImgInfoType*      m_pDLImgInfo;


	/* ratio of progress for each image downloading
	 */
	uint8                m_uRatio;
	/* progress ratio base of current image downloading
	 */
	uint8                m_uBaseRatio;
	/* Total ratio of the images downloading in the
	 * whole process.
	 */
	uint8                m_uTotalRatio;
	uint16               m_port;
	cmd_buffer_s_type    cmd;
	rsp_buffer_s_type    rsp;
        uint8 lastdone;

        int dlPort;
        //WinMsgProApp         *m_mainApp;
        uint32               m_totalCount;  //add by minghui.zhang calculate download file total size
        bool                 is9X25;         //add by minghui.zhang fix progress bar issue
};

#endif //_DL_DATA_H_
