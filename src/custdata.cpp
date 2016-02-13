#include "stdafx.h"
#include "diagcmd.h"
#include "log.h"
#include "custdata.h"
#include "utils.h"
//#include "src/xml/ParserWebsConfig.h"



//-----------------------------------------------------------------------------

/*typedef struct {
  uint8 cmd_code;
  uint8 subsys_id;                // File descriptor
  uint16 subsys_cmd_code;
} diagpkt_subsys_header_type;*/

/*
 * Default window sizes. Set to large numbers because the target can support
 * essentially unlimited windowing without doing any extra work.
 */
#define FS_TARG_PKT_WINDOW_DEFAULT  0x100000
#define FS_TARG_BYTE_WINDOW_DEFAULT 0x100000
#define FS_HOST_PKT_WINDOW_DEFAULT  0x100000
#define FS_HOST_BYTE_WINDOW_DEFAULT 0x100000
#define FS_ITER_PKT_WINDOW_DEFAULT  0x100000
#define FS_ITER_BYTE_WINDOW_DEFAULT 0x100000
/*
 * Protocol version information.
 */
#define FS_DIAG_VERSION     0x0001
#define FS_DIAG_MIN_VERSION 0x0001
#define FS_DIAG_MAX_VERSION 0x0001

/*
 * Feature Bits.
 */
#define FS_FEATURE_BITS 0x00000000

//ProgressCallback CCustData::func = NULL;

bool check_backup_data_ok(cust_data_info_s_type* custdata_info)
{
	return true;
}

CCustData::CCustData( CPacket* packetDll,
                      TCustDataInfoType* pCustDataInfo)
{
    m_uTotalRatio = 0;
    m_uRatio = 0;
    m_uBaseRatio = 0;
    m_pDIAGCmd = NULL;
    m_Callback = NULL;
    m_CallbackData = NULL;
    this->m_pCustDataInfo = pCustDataInfo;
    this->m_pItems = NULL;
    this->m_uCount = 0;
    this->m_pDIAGCmd = new CDIAGCmd(packetDll);
    this->dlPort = packetDll->GetComPort();
    memset(&this->m_simlockData, 0, sizeof(TImgBufType));
    memset(&this->m_traceData, 0, sizeof(TImgBufType));
}

CCustData::~CCustData()
{
    RELEASE_ARRAY(this->m_pItems);
    RELEASE_ARRAY(this->m_simlockData.data);
    RELEASE_ARRAY(this->m_traceData.data);
    RELEASE(this->m_pDIAGCmd);
}

bool CCustData::ChangeOfflineMode(uint16 wMode)
{
    return this->m_pDIAGCmd->SwitchToOfflineMode(wMode);
}

TResult CCustData::ReadConfigXml(const char* filename,uint8* pdata, long& rlen)
{
    DBGD("COM%d: Read jrdcfg/config.xml ...",dlPort);
    uint8* data = NULL;

    if (FAILURE(ReadFile(false, filename, &data, rlen)))
    {
        ERR("COM%d: Read config.xml failure!",dlPort);
        return EFAILED;
    }

    memcpy(pdata, data, rlen);
    RELEASE_ARRAY(data);

    return EOK;
}

TResult CCustData::WritePersoTxt(const char* fileName, uint8* data, long len)
{
    TResult result = EOK;

    INFO("Restore SIMLOCK ...");

    result = WriteFile(false, fileName, data, len);
    if (FAILURE(result))
    {
        return ERESTORESIMLOCK;
    }

    return result;
}

TResult CCustData::WriteConfigXml(const char* fileName, uint8* data, long len)
{

    TResult result = EOK;

    DBGD("Write config.xml ...");

	result = WriteFile(false, fileName, data, len);
	if (FAILURE(result))
	{
        ERR("COM%d: Fail to write config.xml!!!",dlPort);
		return EFAILED;
	}

	return result;
}

TResult CCustData::WriteStaticQCN(uint8* data, long len)
{
    TResult result = EOK;

    INFO("Write static nv item ...");

    for (int i = 0; i < 10; i++)
    {
        result = this->m_pDIAGCmd->EfsOpHello();
        if (SUCCESS(result))
        {
                break;
        }

        SLEEP(500);
        WARN("RestoreNVItem, EfsOpHello failure!");
    }

    if (FAILURE(result))
    {
        ERR("RestoreNVItem, device might be disconnect!");
        return ERESTORENV;
    }

    NV_ITEM_PACKET_INFO *pTemp = (NV_ITEM_PACKET_INFO *)data;
    //uint32  count = len/sizeof(NV_ITEM_PACKET_INFO);
    //uint16  uitem = 0;
    uint16  ustatus = 0;
    bool    bOk = false;
    uint32  uFailTimes = 0;

    while(pTemp->packetLen == 0x88)
    {

        bOk = this->m_pDIAGCmd->NV_Write_Item( pTemp->nvItemID,
                                               pTemp->itemData,
                                               sizeof(NV_ITEM_PACKET_INFO),
                                               &ustatus);

        //qDebug() << pTemp->nvItemID;

        char buf[64] = {0};
        sprintf(buf, "Write nv item %d!", pTemp->nvItemID);
        //INFO( buf);
        if (bOk)
        {
            ++pTemp;
            uFailTimes = 0;
        }
        else
        {
            if (!bOk || ustatus != NV_DONE_S)
            {
                memset(buf, 0, 64);
                sprintf(buf, "Write nv item failure, item %d, ustatus %d", pTemp->nvItemID, ustatus);
                //WARN(buf);
                //tr("Write nv item failure, item %1, ustatus %2").arg(pTemp->nvItemID).arg(ustatus));

            }
            uFailTimes++;
        }

        if(uFailTimes > 3)
        {
            return EFAILED;
        }
    }

    return EOK;
}

void CCustData::SetRatioParams(uint8 ratio, uint8 base)
{
        this->m_uTotalRatio = ratio;
        this->m_uBaseRatio  = base;
}


TResult CCustData::DLoadDashboard(bool bWriteArm, const char* fileName,uint8* data, long len)
{
    TResult result = EOK;

    DBGD("DLoadDashboard ...");

    result = WriteFile(bWriteArm, fileName,data, len);
    if (FAILURE(result))
    {
        ERR("COM%d Fail to Download Dashboard!",dlPort);
        return EFAILED;
    }

    return EOK;
}

TResult CCustData::ReadFile(bool bReadArmEfs, const char* filename, uint8** ppdata, long& rlen)
{

    TResult result = EOK;

    if (filename == NULL || !strcmp(filename, "") || ppdata == NULL)
    {
        ERR("Invalid param, ppdata == NULL!");
        return EINVALIDPARAM;
    }
    *ppdata = NULL;

    if (this->m_pDIAGCmd == NULL)
    {
        ERR("this->m_pDIAGCmd == NULL!");
        return EFAILED;
    }

    INFO("COM%d: ReadFile %s", dlPort, filename);

    for (int i = 0; i < 5; ++i)
    {
        result = this->m_pDIAGCmd->EfsOpHello(bReadArmEfs);
        if (SUCCESS(result))
        {
            break;
        }
        SLEEP(1000);
	}

	if (FAILURE(result))
	{
        ERR("COM%d: ReadFile failure, can't send any hello packet!",dlPort);
        return EEFSOPHELLO;
	}

	int32 mode = 0;
	int32 size = 0;
	int32 oflag = 0;
	int32 fd = 0;


	result = this->m_pDIAGCmd->EfsOpStat(bReadArmEfs, filename, mode, size);
	if (FAILURE(result))
	{
        ERR("COM%d: EfsOpStat failure!",dlPort);
        return EEFSOPSTAT;
	}

	result = this->m_pDIAGCmd->EfsOpOpen(bReadArmEfs, filename, oflag, mode, fd);
	if (FAILURE(result))
	{
        ERR("COM%d: EfsOpOpen failure!",dlPort);
        return EEFSOPOPEN;
	}

	uint8* pdata = NULL;
	NEW_ARRAY(pdata, uint8, size);
	if (pdata == NULL)
	{
        CRITICAL("COM%d: No memory!!!", dlPort);
        return ENOMEMORY;
	}

	result = this->m_pDIAGCmd->EfsOpRead(bReadArmEfs, fd, size, pdata);
	if (FAILURE(result))
	{
        ERR("COM%d: EfsOpRead failure!", dlPort);
		RELEASE_ARRAY(pdata);
		return EEFSOPREAD;
	}

	result = this->m_pDIAGCmd->EfsOpClose(bReadArmEfs, fd);
	if (FAILURE(result))
	{
        ERR("COM%d: EfsOpClose failure!", dlPort);
		RELEASE_ARRAY(pdata);
		return EEFSOPCLOSE;
	}

	*ppdata = pdata;
	rlen = size;

	return EOK;
}

TResult CCustData::DeleteEFSFiles(bool bWriteArm, const char* fileName)
{
    TResult result = EOK;
    //uint16 port = ((TDLUserDataType*)this->m_pDLCbInfo->pUserData)->port;

    result = DeleteFile(bWriteArm, fileName);
    if (FAILURE(result))
    {
        ERR("COM%d: Fail to Write EFSFiles!!!",dlPort);
        return EFAILED;
    }

    return EOK;
}
TResult CCustData::DeleteFile(bool bWriteArmEfs, const char* fileName)
{
    TResult result = EOK;
    //uint16 port = ((TDLUserDataType*)(this->m_pDLCbInfo->pUserData))->port;
    //HWND   hWnd = this->m_pDLCbInfo->hWnd;

    //DBGD("COM%d: CCustData::DeleteFile",dlPort);

    if (fileName == NULL || !strcmp(fileName, ""))
    {
        ERR("COM%d: Invalid params!",dlPort);
        return EINVALIDPARAM;
    }
    if (this->m_pDIAGCmd == NULL)
    {
        ERR("COM%d: this->m_pDIAGCmd == NULL",dlPort);
        return EFAILED;
    }

    INFO("COM%d: DeleteFile %s",dlPort,fileName);

    /* Ping device before any operation */
    for (int i = 0; i < 5; ++i)
    {
        result = this->m_pDIAGCmd->EfsOpHello(bWriteArmEfs);
        if (SUCCESS(result))
        {
            break;
        }
        INFO("COM%d: EfsOpHello packet retry %d times!\n",dlPort, i);
        SLEEP(1000);
    }
    if (FAILURE(result))
    {
        ERR("COM%d: EfsOpHello packet failure!",dlPort);
        return EEFSOPHELLO;
    }

    result = this->m_pDIAGCmd->EfsDelFile(bWriteArmEfs, fileName);
    if (FAILURE(result))
    {
        ERR("COM%d: EfsDelFile packet failure!",dlPort);
        return EEFSOPOPEN;
    }
    return EEFSOPOPEN;
}

TResult CCustData::WriteFile
(
    bool   bWriteArmEfs,
    const char*  fileName,
    uint8* data,
    long   len,
    bool   bDelExistFile
)
{
    TResult result = EOK;

    //DBGD("CCustData::WriteFile %s",fileName);

    if (fileName == NULL || !strcmp(fileName, "") || data == NULL)
    {
        ERR("Invalid params! ----fileName = %s  data = %d len = %l",fileName,*data,len);
            return EINVALIDPARAM;
    }

    if (this->m_pDIAGCmd == NULL)
    {
        ERR("this->m_pDIAGCmd == NULL");
            return EFAILED;
    }

    INFO("COM%d: WriteFile %s", dlPort, fileName);

    /* Ping device before any operation */
    for (int i = 0; i < 5; ++i)
    {
        result = this->m_pDIAGCmd->EfsOpHello(bWriteArmEfs);
        if (SUCCESS(result))
        {
            break;
        }
        INFO("COM%d: EfsOpHello packet retry %d times!\n", dlPort, i);
        SLEEP(1000);
    }

    if (FAILURE(result))
    {
        ERR("COM%d: EfsOpHello packet failure!",dlPort);
        return EEFSOPHELLO;
    }

    int32 oflag = (O_WRONLY | O_CREAT);
    int32 mode = EFS_IOLBF;
    int32 fd = 0;

    if (bDelExistFile)
    {
            /*del Efs file */
        for(int i = 0; i < 10; i++)
        {
            result = this->m_pDIAGCmd->EfsDelFile(bWriteArmEfs, fileName);

            if (SUCCESS(result))
            {
                INFO("COM%d: EfsDelFile packet success!",dlPort);
                break;
            }
            SLEEP(1000);
        }

        if (FAILURE(result))
        {
            ERR("COM%d: EfsDelFile packet failure!",dlPort);
            return EEFSOPOPEN;
        }
    }

    /* Open Efs file */
    result = this->m_pDIAGCmd->EfsOpOpen(bWriteArmEfs, fileName, oflag, mode, fd);
    if (FAILURE(result))
    {
        //add by jie.li 2011-09-28 create the path when the path is not exist.
        //QString strFileName(fileName);
        char* strTemp = strdup(fileName);
        int len = strlen(fileName);
        int dirp = 0;

        if(strTemp == NULL) {
            LOGE("No memory for store string");
            return EFAILED;
        }
        memset(strTemp, 0, len);

        for (int i=0; i<len; i++)
        {
            if (strTemp[i] == '/')
            {
                strTemp[i] = '\0';
                //dirp = this->m_pDIAGCmd->EfsOpOpenDir(bWriteArmEfs, strTemp.GetBuffer(strTemp.GetLength()));
                dirp = this->m_pDIAGCmd->EfsOpOpenDir(bWriteArmEfs, strTemp);
                if (dirp <= 0)
                {
                    result = this->m_pDIAGCmd->EfsOpMKDir(bWriteArmEfs, strTemp);
                    if (FAILURE(result))
                    {
                        return EFAILED;
                    }
                }
                //modify by yanbin.wan 2013-02-25 for more than four dirs open,
                else
                {
                    result = this->m_pDIAGCmd->EfsOpCloseDir(bWriteArmEfs, strTemp, dirp);
                    if (FAILURE(result))
                    {
                        return EFAILED;
                    }

                }
                strTemp[i] = '/';
            }
        }
            //end add

        if (FAILURE(result))
        {
            return EEFSOPOPEN;
        }
        result = this->m_pDIAGCmd->EfsOpOpen(bWriteArmEfs, fileName, oflag, mode, fd);
        if (FAILURE(result))
        {
            ERR("COM%d: EfsOpOpen packet failure!",dlPort);
            return EEFSOPOPEN;
        }
    }

    uint32 total = len;
    /* bytes to write */
    uint32 nwrite = 0;
    /* bytes has been written */
    uint32 nbytes = 0;
    /* current pos of write ptr */
    uint32 offset = 0;
    /* pos of write ptr before write */
    uint32 start = 0;
    /* last done percent */
    //static uint8 lastdone = 0;
    /* current percent */
    uint8 percent = 0;

    /* Write file to Efs */
    for (;;)
    {
        nwrite = (len > DIAG_FS_BUFFER_SIZE) ? DIAG_FS_BUFFER_SIZE : len;
        start = offset;
        result = this->m_pDIAGCmd->EfsOpWrite(bWriteArmEfs, fd, data, offset, nwrite);
        if (FAILURE(result))
        {
            break;
        }
        if (offset > total)
        {
            //ERR("COM%d: WriteFile failure, offset (%d) > total (%d)!",
            //	port, offset, total);
            return EEFSOPWRITE;
        }

        nbytes = offset - start;

        if (len < nbytes)
        {
            //ERR("COM%d: WriteFile failure, len (%d) < nbytes (%d)!",
            //	  port, len, nbytes);
            return EEFSOPWRITE;
        }
        len -= nbytes;

        if (total < 100)
        {
            percent = this->m_uBaseRatio + ((total-len)*100/(total)) * this->m_uTotalRatio / 100;
        }
        else
        {
            percent = this->m_uBaseRatio + ((total-len)/(total/100)) * this->m_uTotalRatio / 100;
        }

        percent = (percent<=100) ? percent : 100;
        if (percent != lastdone)
        {
            lastdone = percent;

                //SendMessage(hWnd, WM_USER_PROGRESS, port, percent);
            if (m_Callback != NULL)
            m_Callback(m_CallbackData, dlPort,percent);
        }

        /* If done with written */
        if (len == 0)
        {
                break;
        }
    }

    if (FAILURE(result))
    {
        ERR("COM%d: EfsOpWrite packet failure!",dlPort);
        return EEFSOPWRITE;
    }

    /* Close Efs file */
    result = this->m_pDIAGCmd->EfsOpClose(bWriteArmEfs, fd);
    if (FAILURE(result))
    {
        ERR("COM%d: EfsOpClose packet failure!",dlPort);
        return EEFSOPCLOSE;
    }

    return EOK;
}


/* write band config items.
*  include band,mode,order and domain
*/
bool CCustData::WriteBandConfigItems(unsigned int dsat_syssel_val[4])
{
/* Data buffer for individual writes/reads issued internally. */
/* Command buffer for individual writes/reads issued internally. */
  //uint64 band_pref = (uint64)(0x40000000);
  uint16 nv_441_val = 0xFFFF;
  uint16 nv_946_val = 0xFFFF;
  uint32 nv_2954_val = 0x7FFFFFFF;
  cm_mode_pref_e_type mode_pref = CM_MODE_PREF_NO_CHANGE;
  uint16  nv_10_val = 0;
  cm_gw_acq_order_pref_e_type acq_order_pref = CM_GW_ACQ_ORDER_PREF_NO_CHANGE;
  cm_srv_domain_pref_e_type srv_domain_pref = CM_SRV_DOMAIN_PREF_NO_CHANGE;
  unsigned long long nv_65633_val = 0;   //add by jie.li 2012-05-08
  bool bLTEBand = false;
  bool bWriterBandInfo = true;

  /*
  ** ----------------------------------------------------------------------
  ** Set band
  ** ----------------------------------------------------------------------
  */
  switch (dsat_syssel_val[0])
  {
    /*
    ** Default: UMTS850/900/2100 GSM850/900/1800/1900
    */
    case 0:
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x04F8;
      nv_2954_val = 0x00000000;
    }
    break;

    case 1: /* EURO: GSM900/1800,WCDMA2100 */
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x0050;
      nv_2954_val = 0x00000000;
    }
    break;

    case 2: /* NORTH AMERICA: GSM850/1900,WCDMA850/1900 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x04A8;
      nv_2954_val = 0x00000000;
    }
    break;

    case 3: /* GSM ONLY: GSM850/900/1800/1900 */
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x0038;
      nv_2954_val = 0x00000000;
    }
    break;

    case 4: /* UTMS ONLY: WCDMA850/1900/2100 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x04C0;
      nv_2954_val = 0x00000000;
    }
    break;

    case 5: /* GSM850 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x0008;
      nv_2954_val = 0x00000000;
    }
    break;

    case 6: /* GSM900 */
    {
      nv_441_val = 0x0300;
      nv_946_val = 0x0010;
      nv_2954_val = 0x00000000;
    }
    break;

    case 7: /* DCS1800 */
    {
      nv_441_val = 0x0080;
      nv_946_val = 0x0000;
      nv_2954_val = 0x00000000;
    }
    break;

    case 8: /* PCS1900 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x0020;
      nv_2954_val = 0x00000000;
    }
    break;

    case 9: /* WCDMA850 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x0400;
      nv_2954_val = 0x00000000;
    }
    break;

    case 10: /* WCDMA1900 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x0080;
      nv_2954_val = 0x00000000;
    }
    break;

    case 11: /* WCDMA2100 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x0040;
      nv_2954_val = 0x00000000;
    }
    break;

    case 12: /* WCDMA VIII 900 */
    {
      nv_441_val = 0x0000;
      nv_946_val = 0x0000;
      nv_2954_val = 0x00020000;
    }
    break;

    case 13: /* GSM900/1800/850/1900, WCDMA900/2100 */
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x0078;
      nv_2954_val = 0x00020000;
    }
    break;

    case 14: /* GSM900/1800/850/1900, WCDMA2100 */
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x0078;
      nv_2954_val = 0x00000000;
    }
    break;

    case 15: /* GSM900/1800/850/1900, WCDMA850/1900 */
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x04B8;
      nv_2954_val = 0x00000000;
    }
    break;

    case 16: /*WCDMA1700\2100*/
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x0278;
      nv_2954_val = 0x00000000;
    }
    break;

    case 17: /*WCDMA900\1900*/
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x00B8;
      nv_2954_val = 0x00020000;
    }
    break;

    case 18: /*WCDMA850*/
    {
      nv_441_val = 0x0380;
      nv_946_val = 0x0438;
      nv_2954_val = 0x00000000;
    }
    break;

	case 19: /*UMTS900/1900/2100MHz+AWS*/
	{
		nv_441_val = 0x0380;
		nv_946_val = 0x02F8;
        //nv_2954_val = 0x00000000;
        //changed by jie.li 2011-04-13 to UMTS900
        nv_2954_val = 0x00020000;
        //end changed
	}
	break;

	case 20:/*UMTS850/2100MHz*/
	{
		nv_441_val = 0x0380;
		nv_946_val = 0x0478;
		nv_2954_val = 0x00000000;
	}
	break;

    //Add by jie.li 2011-05-04
    case 21: /*W850/1700/2100MHz*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0678;
        nv_2954_val = 0x00000000;
    }
    break;

    case 22: /*UMTS(900/1800/2100)*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0178;
        nv_2954_val = 0x00020000;
    }
    break;
    //end

    //add by jie.li 2011-11-29 for Y580
    case 23: /*UMTS(UMTS2100/1700/800MHz) GSM(4 bands)*/
    {
        nv_441_val = 0x0380;
        //nv_946_val = 0x0CF8;
        nv_946_val = 0x0878;   //changed by jie.li 2012-12-24 for Y580J VJC
        nv_2954_val = 0x40000;
    }
    break;
    //end add

    //modify by yanbin.wan20130624 NV946:0x0c68->0x0478
    case 24: /* WCDMA900/850/2100 GSM850/900/1800/1900 */
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0478;
        nv_2954_val = 0x20000;
    }
    break;

    case 25:	/*UMTS900/1900/2100MHz GSM850/900/1800/1900*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x00F8;
        nv_2954_val = 0x20000;
    }
    break;

    //add by jie.li 2011-11-29
    case 40: /*LTE*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0178;
        nv_2954_val = 0x00020000;
        nv_65633_val = 0x800C5;
        bLTEBand = true;
    }
    break;
    //end add

    case 41: /*LTE800/1800/2600 WCDMA 2100/1800/900 GSM850/900/1800/1900/*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0178;
        nv_2954_val = 0x00020000;
        nv_65633_val = 0x80044;
        bLTEBand = true;
    }
    break;

    case 42: /*LTE1800/2600 WCDMA 2100/1800/900 GSM850/900/1800/1900/*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0078;
        nv_2954_val = 0x00020000;
        nv_65633_val = 0x00044;
        bLTEBand = true;
    }
    break;

    //add by jie.li 20120830
    case 43: /*GSM850/900/1800/1900 MHz UMTS850/1900/2100 MHz LTE2100/2600 MHz*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x04F8;
        nv_2954_val = 0;
        nv_65633_val = 0x00041;
        bLTEBand = true;
    }
    break;

    case 44: /*GSM850/900/1800/1900 MHz  UMTS850/1900/2100 MHz LTE700MHz(B17)/AWS(B4,1700/2100MHz)*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x04F8;
        nv_2954_val = 0;
        nv_65633_val = 0x10008;
        bLTEBand = true;
    }
    break;
    //end add

	//add by yanbin.wan for MDM9x15
	case 45: /*LTE 800/900/1800/2600 WCDMA 2100/1800/900 GSM850/900/1800/1900/*/
	{
		nv_441_val = 0x0380;
		nv_946_val = 0x00178;
		nv_2954_val = 0x20000;
		nv_65633_val = 0x800C4;
		bLTEBand = TRUE;
	}
	break;
	//end add

    //add by yanbin.wan for MDM9x15 2013-01-25
    case 46: /*LTE700/850/AWS/1900/2100/2600 UMTS 850/AWS/1900/2100	GSM850/900/1800/1900*/
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x06E8;
        nv_2954_val = 0;
        nv_65633_val = 0x1005B;
        bLTEBand = TRUE;
    }
    break;
    //end add
    //add by yanbin.wan 20130725 for W800 for china Telecom
    //47“C“CLTE 850/1800/2100/2600 WCDMA 2100/1800/850 GSM850/900/1800/1900
    case 47:
    {
        nv_441_val = 0x0380;
        nv_946_val = 0x0578;
        nv_2954_val = 0;
        nv_65633_val = 0x00055;
        bLTEBand = TRUE;
    }
    break;
    //48每每LTE 1800/2100 WCDMA 2100/900 GSM850/900/1800/1900
      case 48:
      {
          nv_441_val = 0x0380;
          nv_946_val = 0x0078;
          nv_2954_val = 0x20000;
          nv_65633_val = 0x00005;
          bLTEBand = TRUE;
      }
      break;
      //49每每LTE 2600 WCDMA 1900/900 GSM850/900/1800/1900
      case 49:
      {
          nv_441_val = 0x0380;
          nv_946_val = 0x00B8;
          nv_2954_val = 0x20000;
          nv_65633_val = 0x00040;
          bLTEBand = TRUE;
      }
      break;
    //end add
    default:
    {
          bWriterBandInfo = FALSE;
          //return FALSE;
          break;
    }
  }

//  band_pref = (nv_2954_val<<32) + (nv_946_val<<16) + nv_441_val;

  /*
  ** ----------------------------------------------------------------------
  ** Set mode
  ** ----------------------------------------------------------------------
  */
  switch (dsat_syssel_val[1])
  {
    case 0:
    {
      mode_pref = CM_MODE_PREF_AUTOMATIC;
      nv_10_val = NV_MODE_AUTOMATIC;
    }
    break;

    case 1:
    {
      mode_pref = CM_MODE_PREF_GSM_ONLY;
      nv_10_val = NV_MODE_GSM_ONLY;
    }
    break;

    case 2:
    {
      mode_pref = CM_MODE_PREF_WCDMA_ONLY;
      nv_10_val = NV_MODE_WCDMA_ONLY;
    }
    break;

    case 3:
    {
      mode_pref = CM_MODE_PREF_GWL;
      nv_10_val = NV_MODE_GWL;
    }
    break;

    case 4:
    {
      mode_pref = CM_MODE_PREF_GSM_WCDMA_ONLY;
      nv_10_val = NV_MODE_GSM_WCDMA_ONLY;
    }
    break;

    case 5:
    {
      mode_pref = CM_MODE_PREF_LTE_ONLY;
      nv_10_val = NV_MODE_LTE_ONLY;
    }
    break;

    case 6:
    {
      mode_pref = CM_MODE_PREF_WCDMA_LTE_ONLY3G;
      nv_10_val = NV_MODE_WCDMA_LTE_ONLY;
    }
    break;

    default:
    {
      return false;
    }
  }

  /*
  ** ----------------------------------------------------------------------
  ** Set acquisition order
  ** ----------------------------------------------------------------------
  */
  switch (dsat_syssel_val[2])
  {
    case 0:
    {
      acq_order_pref = CM_GW_ACQ_ORDER_PREF_AUTOMATIC;
    }
    break;

    case 1:
    {
      acq_order_pref = CM_GW_ACQ_ORDER_PREF_WCDMA_GSM;
    }
    break;

    case 2:
    {
      acq_order_pref = CM_GW_ACQ_ORDER_PREF_GSM_WCDMA;
    }
    break;

    default:
    {
      return false;
    }
  }

  /*
  ** ----------------------------------------------------------------------
  ** Set service domain
  ** ----------------------------------------------------------------------
  */
  switch (dsat_syssel_val[3])
  {
    case 0:
    {
      srv_domain_pref = CM_SRV_DOMAIN_PREF_CS_PS;
    }
    break;

    case 1:
    {
      srv_domain_pref = CM_SRV_DOMAIN_PREF_CS_ONLY;
    }
    break;

    case 2:
    {
      srv_domain_pref = CM_SRV_DOMAIN_PREF_PS_ONLY;
    }
    break;

    default:
    {
      return false;
    }
  }

  /* Write NV items */
  uint16 uState = 0;
  uint16 uFailTimes = 0;
  bool bWriteOk = false;

  uint8 valBuf[128] = {0};
  memset(valBuf, 0, 128);
  memcpy(valBuf + 1, &nv_10_val, sizeof(uint16));

  bWriteOk = m_pDIAGCmd->NV_Write_Item( 10/*NV_PREF_MODE_I*/, (uint8 *)(valBuf),
										128, &uState);

  while (!bWriteOk || uState != NV_DONE_S)
  {
	 // WARN( "COM%d: Write nv item failure, item 10, ustatus %d",
	//	  port, uState);

     if (uFailTimes > 3)
	 {
		 return false;
	 }

	 bWriteOk = m_pDIAGCmd->NV_Write_Item( 10/*NV_PREF_MODE_I*/, (uint8 *)(valBuf),
											128, &uState);

	 uFailTimes++;
  }
  if(bWriterBandInfo)
  {
  uFailTimes = 0;
  memset(valBuf, 0, 128);
  memcpy(valBuf+1, &nv_441_val, sizeof(uint16));
  bWriteOk = m_pDIAGCmd->NV_Write_Item( 441/*NV_BAND_PREF_I*/, (uint8 *)(valBuf),
										128, &uState);

  while (!bWriteOk || uState != NV_DONE_S)
  {
	//  WARN( "COM%d: Write nv item failure, item 441, ustatus %d",
	//	  port, uState);

	  if (uFailTimes > 3)
	  {
		  return false;
	  }

	  bWriteOk = m_pDIAGCmd->NV_Write_Item( 441/*NV_BAND_PREF_I*/, (uint8 *)(valBuf),
											3, &uState);

	  uFailTimes++;
  }

  uFailTimes = 0;
  memset(valBuf, 0, 128);
  memcpy(valBuf+1, &nv_946_val, sizeof(uint16));
  bWriteOk = m_pDIAGCmd->NV_Write_Item( 946/*NV_BAND_PREF_16_31_I*/, (uint8 *)(valBuf),
										128, &uState);

  while (!bWriteOk || uState != NV_DONE_S)
  {
	//  WARN( "COM%d: Write nv item failure, item 946, ustatus %d",
	//	  port, uState);

	  if (uFailTimes > 3)
	  {
		  return false;
	  }

	  bWriteOk = m_pDIAGCmd->NV_Write_Item( 946/*NV_BAND_PREF_16_31_I*/, (uint8 *)(valBuf),
											128, &uState);

	  uFailTimes++;
  }

  uFailTimes = 0;
  memset(valBuf, 0, 128);
  memcpy(valBuf+1, &nv_2954_val, sizeof(uint32));
  bWriteOk = m_pDIAGCmd->NV_Write_Item( 2954/*NV_BAND_PREF_32_63_I*/, (uint8 *)(valBuf),
										128, &uState);

  while (!bWriteOk || uState != NV_DONE_S)
  {
	//  WARN( "COM%d: Write nv item failure, item 2954, ustatus %d",
	//	  port, uState);

	  if (uFailTimes > 3)
	  {
		  return false;
	  }

	  bWriteOk = m_pDIAGCmd->NV_Write_Item( 2954/*NV_BAND_PREF_32_63_I*/, (uint8 *)(valBuf),
											128, &uState);

          uFailTimes++;
      }
    }

  uFailTimes = 0;
  memset(valBuf, 0, 128);
  memcpy(valBuf+1, &acq_order_pref, sizeof(uint16));
  bWriteOk = m_pDIAGCmd->NV_Write_Item( 848/*NV_ACQ_ORDER_PREF_I*/, (uint8 *)(valBuf),
										128, &uState);

  while (!bWriteOk || uState != NV_DONE_S)
  {
	//  WARN( "COM%d: Write nv item failure, item 848, ustatus %d",
	//	  port, uState);

	  if (uFailTimes > 3)
	  {
		  return false;
	  }

	  bWriteOk = m_pDIAGCmd->NV_Write_Item( 848/*NV_ACQ_ORDER_PREF_I*/, (uint8 *)(valBuf),
											128, &uState);

	  uFailTimes++;
  }

  uFailTimes = 0;
  memset(valBuf, 0, 128);
  memcpy(valBuf+1, &srv_domain_pref, sizeof(uint16));

  bWriteOk = m_pDIAGCmd->NV_Write_Item( 850/*NV_SERVICE_DOMAIN_PREF_I*/, (uint8 *)(valBuf),
										128, &uState);

  while (!bWriteOk || uState != NV_DONE_S)
  {
	//  WARN( "COM%d: Write nv item failure, item 850, ustatus %d",
	//	  port, uState);

	  if (uFailTimes > 3)
	  {
		  return false;
	  }

	  bWriteOk = m_pDIAGCmd->NV_Write_Item( 850/*NV_SERVICE_DOMAIN_PREF_I*/, (uint8 *)(valBuf),
											128, &uState);

	  uFailTimes++;
  }

  if (bLTEBand&&bWriterBandInfo)
  {
      uFailTimes = 0;
      //QString strLTEbandPref = "nv//item_files//modem//mmode//lte_bandpref";
      //typeQByteArray b = strLTEbandPref.toLatin1();
      //char* cFileName = (char*)b.data();
      char* cFileName = "nv//item_files//modem//mmode//lte_bandpref";
      TResult result = EOK;
      result = WriteFile( false, cFileName, (uint8 *)(&nv_65633_val), 8, true);

      while (FAILURE(result))
      {
          if (uFailTimes > 3)
          {
              return FALSE;
          }

          result = WriteFile( false, cFileName, (uint8 *)(&nv_65633_val), 8, true);

          uFailTimes++;
      }
  }

  return true;
}

void CCustData::RegisterCallback(ProgressCallback callback, void *data)
{
    m_Callback = callback;
    m_CallbackData = data;
}

//add by jie.li 2012-06-27
TResult CCustData::DeleteDir(bool bWriteArm, const char*  fileName)
{
    TResult result = EOK;
    //uint16 port = ((TDLUserDataType*)this->m_pDLCbInfo->pUserData)->port;

    //if (fileName == NULL || fileName == "")
    if (fileName == NULL || strlen(fileName) == 0)
    {
        //ERR("COM%d: Invalid param, ppdata == NULL!", port);
        return EINVALIDPARAM;
    }

    if (this->m_pDIAGCmd == NULL)
    {
        //ERR("COM%d: this->m_pDIAGCmd == NULL!", port);
        return EFAILED;
    }

    //INFO("COM%d: ReadFile %s", port, fileName);

    for (int i = 0; i < 5; ++i)
    {
        result = m_pDIAGCmd->EfsOpHello(bWriteArm);
        if (SUCCESS(result))
        {
            break;
        }
        //WARN("COM%d: Read file,send hello package error,%d times!\n",
        //    port, i+1);
        SLEEP(1000);
    }

    if (FAILURE(result))
    {
        //ERR("COM%d: ReadFile failure, can't send any hello packet!",
        //    port);
        return EEFSOPHELLO;
    }

    int dirpNum = 0;
    dirpNum = m_pDIAGCmd->EfsOpReadDelDir(false, fileName);
    while (dirpNum > 0)
    {
        dirpNum = m_pDIAGCmd->EfsOpReadDelDir(false, fileName);
    }

    if (dirpNum < 0)
        return EFAILED;

    return	EOK;
}
//end add

//add by jie.li 20120830 for Y580 to parse the webs_config
TResult CCustData::ReadWebsConfigFile(WebsXMLInfo* pwebsxmlinfo)
{
    uint8 *pdata = NULL;//,*pTemp = NULL;
    long rlen = 0;

    if(FAILURE(ReadFile(false, EFS_FILE_WEBS_CONFIG, &pdata, rlen)))
    {
        //ERR("COM%d: Read config file failure!", port);
        return EFAILED;
    }
    if(!websconfigParse(pdata, pwebsxmlinfo, rlen))
    {
        return EFAILED;
    };
    RELEASE_ARRAY(pdata);
    return EOK;
}

bool CCustData::websconfigParse(uint8* pXmlBuf, WebsXMLInfo* pxmlinfo, int len)
{

    //ParserWebsConfig *websConfig = new ParserWebsConfig();
    //bool bResult = websConfig->parserFromBuffer(pXmlBuf,pxmlinfo);
    //return bResult;
     return true;
}
//end add

TResult CCustData::ReadSSIDConfigFile(WebsXMLInfo* pwebsxmlinfo)
{
    uint8 *pdata = NULL;//,*pTemp = NULL;
    long rlen = 0;

    if(FAILURE(ReadFile(false, EFS_FILE_SSID_CONFIG, &pdata, rlen)))
    {
        //ERR("COM%d: Read config file failure!", port);
        return EFAILED;
    }
    if(!websconfigParse(pdata, pwebsxmlinfo, rlen))
    {
        return EFAILED;
    };

    RELEASE_ARRAY(pdata);
    return EOK;
}
