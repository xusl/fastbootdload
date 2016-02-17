#include "StdAfx.h"
#include "DiagPST.h"
#include "device.h"
#include "resource.h"
#include "mdmfastbootDlg.h"
#include "PortStateUI.h"
#include <strstream>

void DiagPSTDownloadProgress(void *data, int port,uint16 percent) {
UsbWorkData * worker = (UsbWorkData * )data;
if(worker != NULL)
worker->SetProgress(percent);
}

void DiagPSTDownloadState(void *data, int port, string msg) {
UsbWorkData * worker = (UsbWorkData * )data;
if(worker != NULL)
worker->ui_text_msg(PROMPT_TEXT, msg.c_str());
}

DiagPST::DiagPST(UsbWorkData * worker, map<string,FileBufStruct> & filebuffer):
Software_size (0),
    m_iMobileId(0),
    m_FirmwareVersion(""),
    m_blDownloadMode(false),
    m_dlFileBuffer(filebuffer)
{
    DeviceInterfaces *dev =worker->devIntf;
    m_Worker = worker;
    m_DLLPacket = dev->GetPacket();
    m_DIAGCmd = new CDIAGCmd(m_DLLPacket);
    m_DLPrg = new CDLPrg(m_DLLPacket);
    m_sahara = new SAHARACmd(m_DLLPacket);
    m_newDIAGCmd = new JRDdiagCmd(m_DLLPacket);
    m_dlPort = m_DLLPacket->GetPort() ;
    m_dlData = new CDLData(m_DLLPacket,NULL,m_dlPort);
    m_pDLImgInfo = new TDLImgInfoType();
    m_pCustdataInfo = new TCustDataInfoType();
    memset(m_pDLImgInfo, 0, sizeof(TDLImgInfoType));
    memset(m_pCustdataInfo, 0, sizeof(TCustDataInfoType));
    m_pCustdata = new CCustData(m_DLLPacket, m_pCustdataInfo);
    m_pCustdata->RegisterCallback(DiagPSTDownloadProgress, (void *) worker);
    m_dlData->RegisterCallback(DiagPSTDownloadProgress, DiagPSTDownloadState, (void *) worker);

    if(0){
        JRDdiagCmd  DIAGCmd (m_DLLPacket);
        DIAGCmd.EnableDiagServer();
        char version[VERSION_LEN] ={0};
        for (int i = 0; i < 12; i++) {
            memset(version, 0, sizeof version);
            DIAGCmd.RequestVersion(i, (char *)(&version));
            m_Worker->ui_text_msg(FIRMWARE_VER, version);
            LOGE("index %d version %s", i, version);
        }
        //char pFlash_Type[20] = {0};
        //TResult result = DIAGCmd.RequestFlashType((char *)(&pFlash_Type));
        DIAGCmd.DisableDiagServer();
    }
}


DiagPST::~DiagPST(void){
    DELETE_IF(m_DIAGCmd);
    DELETE_IF(m_DLPrg);
    DELETE_IF(m_pDLImgInfo);
    DELETE_IF(m_pCustdataInfo);

    DELETE_IF(m_sahara);
    DELETE_IF(m_newDIAGCmd);
    DELETE_IF(m_dlData);
    DELETE_IF(m_pCustdata);

    if (m_DLLPacket != NULL) {
        m_DLLPacket->Uninit();
        delete m_DLLPacket;
    }
}

void DiagPST::initDLimgInfo() {
    //get the Dashboard version
    string strWinVer = "EE40_WKIT_00_00";//m_LocalConfigXml->get_XML_Value("WKIT");
    string strMacVer = "EE40_MKIT_00_00";//m_LocalConfigXml->get_XML_Value("MKIT");
    string strLinuxVer ="EE40_LKIT_00_00";//m_LocalConfigXml->get_XML_Value("LKIT");

    string strTotal = strWinVer + DASHBOARD_VER_SEG + strMacVer + DASHBOARD_VER_SEG + strLinuxVer;
    m_pDLImgInfo->lenDashboardVer = strTotal.length();
    memcpy(m_pDLImgInfo->dashboardVer,strTotal.c_str(), strTotal.length());
    m_dlData->setDlImgInfo(m_pDLImgInfo);
}


void DiagPST::initCustDataInfo()
{
    m_pCustdataInfo->configxmlData.data = m_dlFileBuffer.at(XML_NAME).strFileBuf;
    m_pCustdataInfo->configxmlData.count = m_dlFileBuffer.at(XML_NAME).uFileLens;
}

inline TResult DiagPST::EnableDiagServer()
{
    return m_newDIAGCmd->EnableDiagServer();
}

inline TResult DiagPST::DisableDiagServer()
{
    return m_newDIAGCmd->DisableDiagServer();
}

bool DiagPST::RequestDeviceStatus()
{
    TResult result = EOK;

    SetPromptMsg("GETTING DEVICE STATUS");

    bool requestNormalStatus = false;
    bool requestDLStatus = false;

    for(int i = 0; i < 3; ++i)
    {
        result = m_DIAGCmd->EfsOpHello();
        if (SUCCESS(result))
        {
            INFO("COM%d device not in Download mode",m_dlPort);
            requestNormalStatus = true;
            break;
        }
        result = m_DLPrg->SendNopCmd();
        if (SUCCESS(result))
        {
            INFO("COM%d device in Download mode",m_dlPort);
            requestDLStatus = true;
            break;
        }
    }

    if(!requestNormalStatus && !requestDLStatus)
    {
        m_blDownloadMode = true;
        INFO("COM%d request device status fail,in download mode",m_dlPort);

        for(int i = 0; i < 4;i++)
        {
            //wait to receive hello package
            result = m_sahara->GetHelloAckCmdInDlMode();

            if(SUCCESS(result))
            {
                INFO("COM%d in download mode,receive hello",m_dlPort);
                break;
            }
        }

        if(SUCCESS(result))
        {
            result = m_sahara->switchToCmdMode();

            if(SUCCESS(result))
            {
                for(int i = 0; i < 4; i++)
                {
                    result = m_sahara->GetCmdReadyRsp();
                    if(SUCCESS(result))
                    {
                        INFO("COM%d in download mode,receive Command ready",m_dlPort);
                        break;
                    }
                }
            }
        }
    }

    if (FAILURE(result))
    {
        SetPromptMsg("Request device mode failed!");

        return false;
    }

    if (m_blDownloadMode)
    {
        SetPromptMsg("E_PRG_DEVICE_IN_DL_MODE");
    }
    return true;
}

bool DiagPST::EraseSimlock()
{
   if (!m_blDownloadMode && m_bEraseSimlock)
    {
        SetPromptMsg("ERASE SIMLOCK");
        TResult result = m_newDIAGCmd->EraseSimLock();

        if (FAILURE(result))
        {
            SetPromptMsg("erase simlock failed!");
            SLEEP(6000);
            return false;
        }
    }

    return true;
}

bool DiagPST::checkIfFlashTypeMatchNormalMode()
{
    return true;
#if 0
    char pFlash_Type[20];
    memset(&pFlash_Type,0,20);
    TResult result = m_newDIAGCmd->RequestFlashType((char *)(&pFlash_Type));
    if (FAILURE(result) && (strlen(pFlash_Type) != 4))
    {
        SetPromptMsg("Request Flash Type error!");
        return false;
    }
    //compare the flash type
    QString pPcFlash_Type = m_LocalConfigXml->get_XML_Value( "Flash_Code");
    if(pPcFlash_Type.length() != 8)
    {
        pPcFlash_Type = "0" + pPcFlash_Type;
    }
    QString strFlash=QString::number(pFlash_Type[0], 16)+QString::number(pFlash_Type[1],16)+QString::number(pFlash_Type[2],16)
        +QString::number(pFlash_Type[3],16);

    INFO( "COM%d: PC Flash_type, Flash_type = %s",m_dlPort,pPcFlash_Type.toLatin1().data());
    INFO( "COM%d: device Flash_type, Flash_type = %s", m_dlPort,strFlash.toLatin1().data());

    if(((pPcFlash_Type.mid(0,2).toDouble()) != (pFlash_Type[0]))
       //||((pPcFlash_Type.mid(2,2).toDouble()) != (pFlash_Type[1]))
       ||((pPcFlash_Type.mid(4,2).toDouble()) != (pFlash_Type[2]))
       ||((pPcFlash_Type.mid(6,2).toDouble()) != (pFlash_Type[3])))
    {
        SetPromptMsg("Flash Type is not match!");
        return false;
    }
#endif

    return true;
}
bool DiagPST::CompareVersions()
{
    std::map<string,FileBufStruct>::iterator it;

    if(m_blDownloadMode)
        return true;

    if (!m_bForceMode) {
        RequestExternalVersion();

        for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++) {
            if(it->second.isDownload){
                if(CompareAllVersion(it->first.c_str()))
                    it->second.isDownload=false;
            }
        }
    } else {
           RequestExternalVersion();
           for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++) {
               if(it->second.isDownload) {
                   if(string(it->first)=="b.vhd")
                       CompareAllVersion(it->first.c_str());
               }
           }
       }

    return true;
}

bool DiagPST::GenerateFTFiles()
{
    TResult result = EOK;
    if(!m_blDownloadMode)
    {
        if(m_bForceMode)
        {
            result = m_newDIAGCmd->GenerateFTFilesNew(E_JRD_BACKUP_CREAT);
        }
        else
        {
            result = m_newDIAGCmd->GenerateFTFilesNew(E_JRD_BACKUP_UPDATE);
        }
        if (FAILURE(result))
        {
            SetPromptMsg(" Generate FT Files error!");
            //TODO::
            //return false;
            return true;
        }
    }
    return true;
}
bool DiagPST::StorePIC() {
    if (m_blDownloadMode)
        return true;

    std::map<string,FileBufStruct>::iterator it;
    for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++)
    {
        string strFileName = it->first;
        if (strFileName.find("poweron_logo.bmp") != std::string::npos){
            SetPromptMsg("Store poweron_logo.bmp ");
            it->second.isDownload=false;

            TResult result = m_newDIAGCmd->StorePIC(m_dlFileBuffer.at(strFileName).strFileBuf, m_dlFileBuffer.at(strFileName).uFileLens);

            INFO("send diag Store PIC !");
            if (FAILURE(result)) {
                SetPromptMsg("Store poweron_logo.bmp failed!");
                return false;
            }
        }
    }
    return true;
}

bool DiagPST::SetFuncFive()
{
    if(!m_blDownloadMode)
    {
        if (FAILURE(m_newDIAGCmd->SetFuncFive(0)))
        {
            SetPromptMsg("Set at+func=5,0 error!");
            return true;
            //TODO::
            //return false;
        }

        SLEEP(1000);
    }
    return true;
}


bool DiagPST::DownloadCustomerInfo()
{
   /*******************************
   *     Write custom_info.xml    *
   ********************************/

    if(m_blDownloadMode)
    {
        return true;
    }
        TResult result = EOK;

        SetPromptMsg("E_PRG_DOWNLOAD_CUSTOMERINFO");

        for(int i = 0; i < 3; i++){
            int32 offset = 0;
            uint32 remain_len = 0;
            char* content;

            map<string,FileBufStruct>::iterator it = m_dlFileBuffer.find("custom_info.xml");

            if(it == m_dlFileBuffer.end())
            {
                SetPromptMsg("can not find the file custom_info.xml!");
                return false;
            }
            it->second.isDownload=false;

            remain_len = m_dlFileBuffer.at("custom_info.xml").uFileLens;
            content = (char*)m_dlFileBuffer.at("custom_info.xml").strFileBuf;

            while(offset == 0 || remain_len > 0) {
                uint32 write_len = 0;
                if(remain_len > XML_FILE_LEN){
                    write_len = XML_FILE_LEN;
                } else {
                    write_len = remain_len;
                }

                result = m_newDIAGCmd->WriteConfigXml(offset,E_JRD_CUSTOM_INFO_XML,content + offset,write_len);
                if(FAILURE(result)) {
                    ERR("COM%d write custom_info.xml fail, i = ",m_dlPort,i);
                    break;
                }
                remain_len -= write_len;
                offset += write_len;
            }

            SLEEP(1000);
            if(SUCCESS(result))
            {
                INFO("COM%d: write custom_info.xml succes ...,len = %d", m_dlPort,m_dlFileBuffer.at("custom_info.xml").uFileLens);
                break;
            }
        }
        if (FAILURE(result))
        {
            SetPromptMsg("download custom_info.xml failed!");
            return false;
        }

    return true;
}

bool DiagPST::checkIfPackageMatchNormalMode()
{
    //if(m_imgVersion == IMG_VERSION_8)
        if (MDM9x30_MOBILE_ID == m_iMobileId)
        {
        SetPromptMsg("IMG package not match!");
        return true;
    }

    return false;
}
bool DiagPST::checkCusIdNormalMode()
{
    /******************** compare Customer_ID for ADSU************************/
    return true;
}

bool DiagPST::checkIfPartitionMatchNormalMode()
{
    TResult result = EOK;

    if(!m_bForceMode && !m_blDownloadMode)
    {
        char pPartitionVersion[VERSION_LEN] ;
        memset(&pPartitionVersion,0, sizeof pPartitionVersion);

        for(int i = 0; i < 25;i++)
        {
            result = m_newDIAGCmd->RequestVersion(E_JRD_PARTITION_VERSION,(char *)(&pPartitionVersion));
            INFO("COM%d: Request partition Version, version = %s",m_dlPort,pPartitionVersion);
            if(pPartitionVersion != NULL && '\0' != pPartitionVersion[0]) {
                break;
            }
            SLEEP(1000);
        }

        if (FAILURE(result) || pPartitionVersion == NULL || '\0' == pPartitionVersion[0])
        {
            SetPromptMsg("Request partition Version error!");
            return false;
        }

        //compare the partition version
        //TODO::
        string pPcPartition = "";//m_LocalConfigXml->get_XML_Value( "PARTITION");
        //INFO("COM%d: PC partition Version, version = %s",
        // m_dlPort,pPcPartition.c_str());
        string pFWPartitionVersion(pPartitionVersion);
        vector<string> list1;
        vector<string> list2;
        char pcPartitionVersion[VERSION_LEN]={0};
        StringSplit(pPartitionVersion,"_", list1);
        strcpy(pcPartitionVersion, pPcPartition.c_str());
        StringSplit(pcPartitionVersion, "_", list2);

        if(list1.size() > 3)
        {
            pFWPartitionVersion = list1.at(0);
            pFWPartitionVersion+= list1.at(3);
        }
        if(list2.size() > 3)
        {
            pPcPartition = list2.at(0) ;
            pPcPartition+= list2.at(3);
        }
        if (pFWPartitionVersion != pPcPartition)
        {
            SetPromptMsg("partition type not match!");
            return false;
        }
    }
    return true;
}

bool DiagPST::checkIfPackageMatchDlMode()
{
    TResult result = EOK;
    SetPromptMsg("REQUEST MOBILE ID");

    result = m_sahara->CmdExecute(SAHARA_EXEC_CMD_GET_MOBILE_ID);
    int32 len = 0;
    result = m_sahara->GetCmdExecuteRspPkt(&len);
    if(len > 0)
    {
        uint8 *buf = new uint8[len];
        char data[5] = {0};
        char *end = data + 4;
        m_sahara->CmdExecuteDataPkt(SAHARA_EXEC_CMD_GET_MOBILE_ID);
        result = m_sahara->saharaReceive(buf,len);

        //QString mobileID = QString("%1%2").arg(buf[1],0,16).arg(buf[0],0,16);
        //bool ok;
        //m_iMobileId = mobileID.toInt(&ok, 16);
        snprintf(data, 5, "%2x%2x", buf[1], buf[0]);
        m_iMobileId = strtol(data, &end, 16);

        delete []buf;
    }

    INFO("COM%d in download mode RequestMobileID = %d",m_dlPort,m_iMobileId);

    if (SUCCESS(result)){
    //if(m_imgVersion == IMG_VERSION_8)
        {
             if (m_iMobileId!=MDM9x30_MOBILE_ID)
              {
                SetPromptMsg("in download mode IMG package unmatched!");
                return false;
             }
         }

            SetPromptMsg("REQUEST FW VERSION");
            m_sahara->CmdExecute(SAHARA_EXEC_CMD_GET_FW_VERSION);
            int32 len = 0;
            string fwVer = "";
            m_sahara->GetCmdExecuteRspPkt(&len);
            if(len > 0) {
            uint8* buf = new uint8[len];
                m_sahara->CmdExecuteDataPkt(SAHARA_EXEC_CMD_GET_FW_VERSION);
                result = m_sahara->saharaReceive(buf,len);
                for(int i = 0; i < len; i++) {
                    strstream ss;
                    string s;
                    ss << buf[i];
                    ss >> s;
                    fwVer = fwVer.append(s);
                }
                delete buf;
            }

            m_sahara->saharaswitchMode(SAHARA_MODE_IMAGE_TX_PENDING);
    } else {
        SetPromptMsg("in download request mobile ID fail!");
        return false;
    }
    return true;
}

bool DiagPST::RequestFirmwarVerAndMobileIdNormalMode()
{
    get_version_rsp_type pVersion;
    memset(&pVersion, 0, sizeof(get_version_rsp_type));
    TResult result = m_DIAGCmd->RequestFirmwareVer_N((char *)(&pVersion));
    if (FAILURE(result))
    {
        INFO("COM%d RequestFirmwareVer failed",m_dlPort);
        /* if request firmware fails, notify GUI and return false */

        SetPromptMsg("READ FW VERSION failed");
        return false;
    }

    INFO("COM%d pVersion.ver_strings = %s",m_dlPort,pVersion.ver_strings);
    if (pVersion.ver_strings == NULL)
    {
        INFO("COM%d pVersion.ver_strings == NULL",m_dlPort);
        /* if firmware version length is NULL, notify GUI and return false */
        SetPromptMsg("E_RES_ERR_READ_FW_VERSION");

        return false;
    }

    m_FirmwareVersion = pVersion.ver_strings;
    m_iMobileId = pVersion.mobile_model_id;
    INFO("COM%d pVersion.mobile_model_id = %d",m_dlPort,pVersion.mobile_model_id);
    INFO("COM%d pVersion.ver_strings = %s",m_dlPort,pVersion.ver_strings);
    return true;
}

bool DiagPST::SwitchOfflineMode()
{
    if (m_blDownloadMode)
    {
        return true;
    }
        bool bOk = m_pCustdata->ChangeOfflineMode(MODE_CHANGE_OFFLINE_DIGITAL_MODE);
        if (bOk)
        {
            SetPromptMsg("Swtich offile mode ok!");
        }
        else
        {
            SetPromptMsg("Swtich offline mode fails!");
        }
    return bOk;

#if 0
     if (0) {
            CDIAGCmd DIAGCmd(m_DLLPacket);
            DIAGCmd.EnableDiagServer();
            //DIAGCmd.RestartDevice();
            DIAGCmd.SwitchToOfflineMode(MODE_CHANGE_OFFLINE_DIGITAL_MODE);
            DIAGCmd.DLoadMode();
            DIAGCmd.DisableDiagServer();
        }

        if (0){
            TCustDataInfoType *m_pCustdataInfo = new TCustDataInfoType;
            CCustData m_pCustdata (m_DLLPacket, m_pCustdataInfo);
            m_pCustdata.ChangeOfflineMode(MODE_CHANGE_OFFLINE_DIGITAL_MODE);
            delete m_pCustdataInfo;
        }
#endif
}

bool DiagPST::CompareAllVersion(PCCH firmware_name)
{
#if 0
    QSettings *configIniRead = new QSettings("option.ini", QSettings::IniFormat);
    QString Command_Key="/Comand_List/";
    QString XML_Key="/XML_KEY/";
    XML_Key+=firmware_name;
    Command_Key+=firmware_name;
    QString Command_value=configIniRead->value(Command_Key).toString();
    int temp_value=Command_value.toDouble(); ;
    uint8 which_Ver=temp_value;
    QString XML_Value=configIniRead->value(XML_Key).toString();
    QString PC_Version = m_LocalConfigXml->get_XML_Value( XML_Value);
    delete configIniRead;
#else
    //TODO::
    uint8 which_Ver = 0;
    string PC_Version = "";
#endif
    TResult result = EOK;
    char pDviceVersion[VERSION_LEN] ;
    memset(&pDviceVersion,0, VERSION_LEN);

    result = m_newDIAGCmd->RequestVersion(which_Ver,(char *)(&pDviceVersion));
    //INFO(FILE_LINE, "COM%d: Request %s Version, version = %s",
        //m_dlPort,XML_Value,pDviceVersion);
    string pFW_Device_Version(pDviceVersion);
    if(PC_Version==""||pFW_Device_Version=="")
        return false;


    if(firmware_name=="b.vhd")
    {
      return  My_CompareDashboardVersion(pFW_Device_Version.c_str(),PC_Version.c_str());
    }
    return Compare_special_ver(firmware_name,pFW_Device_Version.c_str(),PC_Version.c_str());

}

bool DiagPST::Compare_special_ver(PCCH firmware_name,PCCH Device_ver,PCCH PC_VER)
{
    return true;
}

bool DiagPST::My_CompareDashboardVersion(PCCH Device_firmware,PCCH PC_firmware)
{
    return true;
#if 0
    TResult result = EOK;

    //INFO(FILE_LINE, "COM%d: Request dashboard Version, version = %s",
    // m_dlPort,PC_firmware);
    if(NULL != Device_firmware && Device_firmware != "" )
    {
        vector<QString> list = StringSplit(Device_firmware,"######");
        if(list.size() >= 2)
        {
            QString strWinVer = list.at(0);
            QString strMacVer = list.at(1);
#ifdef FEATURE_TPST
            setVerCallBack(m_dlPort,strWinVer,WIN_VER);
            setVerCallBack(m_dlPort,strMacVer,MAC_VER);
#endif
            if(PC_firmware.indexOf(strWinVer, 0, Qt::CaseInsensitive)!=-1)
            {
                QString strPcMacVer = m_LocalConfigXml->get_XML_Value( "MKIT");
                if(strPcMacVer.indexOf(strMacVer, 0, Qt::CaseInsensitive)!=-1)
                {
                    return true;
                }
            }
        }
    }
    result = m_DIAGCmd->EfsOpHello();	//add this command to clear file of the handle of the port

    return false;
#endif
}

bool DiagPST::RequestExternalVersion()
{
    //-------------------request external version------------------//
    TResult result = EOK;
    char pExternal[VERSION_LEN] = {0};

    for(int i = 0; i < 20;i++)  {
        result = m_newDIAGCmd->RequestVersion(E_JRD_EXTERNAL_VERSION, pExternal);
        INFO("COM%d: Request external version, version = %s", m_dlPort,pExternal);
        if( '\0' != pExternal[0] && SUCCESS(result))
        {
           break;
        }
        SLEEP(1000);
    }

    m_Worker->ui_text_msg(FIRMWARE_VER, pExternal);
    m_Worker->ui_text_msg(LINUX_VER, "NA");
 #ifdef FEATURE_TPST
    setVerCallBack(m_dlPort,"NA",PTS_VER);
#endif
    return true;
}

bool DiagPST::DownloadCheck()
{
    bool result = true;
    initDLimgInfo();
     EnableDiagServer();
    result = RequestDeviceStatus();

    if(!m_blDownloadMode) {
        result = RequestFirmwarVerAndMobileIdNormalMode();
        //result = result && checkIfPackageMatchNormalMode();
        result = result && checkCusIdNormalMode();
        result = result && checkIfPartitionMatchNormalMode();
        result = result && checkIfFlashTypeMatchNormalMode();
    } else {
           result = checkIfPackageMatchDlMode();
    }
    if (!result)
        DisableDiagServer();
    return result;
}

bool DiagPST::RunTimeDiag() {
    if(!SwitchOfflineMode())
    {
        return false;
    }

    if(m_blDownloadMode)
    {
        SetPromptMsg("DEVICE in Download mode");
    }
    CompareVersions();

    if(!StorePIC())
    {
        return false;
    }
    if(!DownloadCustomerInfo())
    {
        return false;
    }

    if(!EraseSimlock())
    {
        return false;
    }
    if(!SetFuncFive())
    {
        return false;
    }
    if(!GenerateFTFiles())
    {
        return false;
    }
    return true;
}


bool DiagPST::DownloadPrg(const wchar_t* config) {
    /* download PRG */
    TImgBufType *pPrgImg = &m_pDLImgInfo->prg;
    const wchar_t  *prg = PST_NPRG;
    wchar_t     filename[MAX_PATH];
    TResult     result = EOK;

    if (m_blDownloadMode)
        prg = PST_ENPRG;

    int data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                           prg,
                                           NULL,
                                           filename,
                                           MAX_PATH,
                                           config);

    if (data_len == 0) {
        LOGE("Can not found prg file %S in configuration file %S.", prg, config);
        return false;
    }
    char * fn = WideStrToMultiStr(filename);
    if (fn == NULL)
        return false;

    map<string,FileBufStruct>::iterator iter=m_dlFileBuffer.find(fn);
    if(iter!=m_dlFileBuffer.end()) {
        pPrgImg->data = m_dlFileBuffer.at(fn).strFileBuf;
        pPrgImg->len = m_dlFileBuffer.at(fn).uFileLens;
    }

    if (pPrgImg->data == NULL) {
        SetPromptMsg("Can not found PRG image.");
        return false;
    }

    SetPromptMsg("download PRG");

    /*
       if device in debug mode, it will enter tpst mode, and the com port of diag is changed.
       */
    m_sahara->SwitchToDLoadMode();

    result = m_sahara->DownloadPrg_9X07(pPrgImg->data,pPrgImg->len,m_dlPort,m_blDownloadMode);
    if (FAILURE(result))
        LOGE("Download PRG %s failed", m_dlFileBuffer.at(fn).strFileName);
    return true;
}

bool DiagPST::Calculate_length() {
    std::map<string,FileBufStruct>::iterator it;
    for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++) {
        if(it->second.isDownload) {
            string Only_DownLoad("appsboot.mbn,tz.mbn,sbl1.mbn,rpm.mbn,appsboot_fastboot.mbn");
            string mode=it->first;
            if(Only_DownLoad.find(mode)!=-1) {
                Software_size+=it->second.uFileLens;
            } else {
               it->second.isDownload=false;
//               it->second.isDownload_fast=true;
           }
        }
    }
    return true;
}

bool DiagPST::DownloadImages() {
    uint32 base = 0;
    TResult result = EOK;

    if(Software_size > 0) {
        SetPromptMsg("begin downloading image .");
        m_dlData->SetRatioParams(100 - base, base);
        SLEEP(500);
        result = m_dlData->DLoad9X07ImagesUsePtn(m_dlFileBuffer, Software_size);
        //result = m_dlData->DLoad9X25ImagesUsePtn(m_dlFileBuffer, Software_size);

        if (FAILURE(result)) {
            return false;
        }
        SetPromptMsg("Close serial port");
        m_dlData->SendResetCmd();
    } else {
        m_DIAGCmd->RestartDevice();
        DisableDiagServer();
    }

    return true;
}


BOOL DiagPST::StringSplit(char * content,  PCHAR lineDelim, std::vector<string>& dataOut) {
    char *str1, *token;
    char *saveptr1;
    int j;
    char* chTemp;
    for (j = 1, str1 = content; ; j++, str1 = NULL) {
        token = strtok_s(str1, lineDelim, &saveptr1);
        if (token == NULL)
            break;

        //chTemp = new char[MAX_PATH];
        //memset(chTemp,0,MAX_PATH);
        //strcpy_s(chTemp,MAX_PATH,token);

        dataOut.push_back(token);
    }
    return TRUE;
}

VOID DiagPST::SetPromptMsg(PCCH msg) {
    INFO("%s: %s", m_Worker->GetDevTag(), msg);
    m_Worker->SetPromptMsg(msg);
}