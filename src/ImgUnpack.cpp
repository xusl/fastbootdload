#include "StdAfx.h"
#include "utils.h"
#include "device.h"
#include "resource.h"

#include <ImgUnpack.h>


ImgUnpack::ImgUnpack():
    m_filesName(NULL),
    pImgData(NULL),
    packageDataBuf(NULL),
    staticQcnBuf(NULL),
    dynamicNvData(NULL),
    qcnDataBuffer(NULL),
    imgVersion(0),
    qcnLens(0),
    bParsePacketOk(false)
{
    m_downloadFileBuffer.clear();
    versionMap.clear();
    m_efsFileName.clear();
    nvItemsIgnore.clear();
}

ImgUnpack::~ImgUnpack() {
    ;
}


BOOL ImgUnpack::UnpackDownloadImg(const wchar_t *lpath, const wchar_t* config_file) {
    uint32 size;
    BYTE* pImgData = (BYTE*)load_file(lpath, &size);
    char headVerBuf[VERSION_HEAD_LEN] = {0};
    memcpy(headVerBuf, pImgData, VERSION_HEAD_LEN);
    IMGVerS* pImgVer = (IMGVerS *)headVerBuf;
    imgVersion = pImgVer->imgVer;
    if (IMG_VERSION > imgVersion || imgVersion > IMG_VERSION_9) //changed by minghui.zhang 2013-12-30
    {
        ERR("Img version unmatched !");
        return false;
    }

    m_downloadFileBuffer.clear();
    m_efsFileName.clear();
    if (pImgVer->imgVer >= IMG_VERSION_4) {
        PCHAR pConfig = WideStrToMultiStr(config_file);
        char filesInfoBuf[FILEINFO_HEAD_LEN_NEW]= {0};
        memcpy(filesInfoBuf, pImgData + VERSION_HEAD_LEN, FILEINFO_HEAD_LEN_NEW);

        FilePosInfoNewS* fileInfo = (FilePosInfoNewS *)filesInfoBuf;
        string fileName = fileInfo->fileName;
        int k = 0;

        while(fileName.size() >= 1) {
            char profileBuf[32] = {0};
            char* partition = profileBuf;
            FileBufStruct afBuf;
            m_efsFileName.push_back(fileName);

            afBuf.strFileName = (uint8*)m_efsFileName.at(k).data();
            afBuf.uFileLens = fileInfo->fileLen;
            afBuf.strFileBuf = pImgData + fileInfo->beginPos;

            GetPrivateProfileStringA("Area_List",
                                     fileInfo->fileName,
                                     NULL,
                                     profileBuf,
                                     32,
                                     pConfig);


            if(fileName=="sbl1.mbn"&&(imgVersion==IMG_VERSION_6||imgVersion==IMG_VERSION_9))
                partition="0:SBL";
            if(fileName=="appsboot.mbn"&&(imgVersion==IMG_VERSION_8||imgVersion==IMG_VERSION_9))
                partition="0:aboot";
            if(strlen(partition) == 0)
                afBuf.isDownload=false;
            strcpy((char *)(afBuf.partition + 2), partition);
            m_downloadFileBuffer.insert(std::pair<string,FileBufStruct>(fileName,afBuf));

            k++;
            fileInfo++;
            fileName = fileInfo->fileName;
        }
        delete []pConfig;
    } else {
        char filesInfoBuf[FILEINFO_HEAD_LEN] = {0};
        memcpy(filesInfoBuf, pImgData + VERSION_HEAD_LEN, FILEINFO_HEAD_LEN);

        FilePosInfoS* fileInfo = (FilePosInfoS *)filesInfoBuf;
        string fileName = fileInfo->fileName;

        int k = 0;
        while(fileName.size() > 1)
        {
            if (!fileName.compare(""))
                break;

            FileBufStruct afBuf;
            m_efsFileName.push_back(fileName);

            afBuf.strFileName = (uint8*)m_efsFileName.at(k).data();
            afBuf.uFileLens = fileInfo->fileLen;
            afBuf.strFileBuf = pImgData + fileInfo->beginPos;

            m_downloadFileBuffer.insert(std::pair<string,FileBufStruct>(fileName,afBuf));
            k++;
            fileInfo++;
            fileName = fileInfo->fileName;
        }
    }

    if(!ReadVersionInfo())
    {
        return false;
    }

    if (pImgVer->imgVer != IMG_VERSION_6 &&
        pImgVer->imgVer != IMG_VERSION_7 &&
        pImgVer->imgVer != IMG_VERSION_8 &&
        pImgVer->imgVer != IMG_VERSION_9 &&
        !ParseQCN())
    {
        bParsePacketOk = false;
        return false;
    }

    return TRUE;
}


bool ImgUnpack::ReadVersionInfo() {
    map<string,FileBufStruct>::iterator it = m_downloadFileBuffer.find(XML_NAME);
    if(it != m_downloadFileBuffer.end()) {
        uint8* pXmlBuf = m_downloadFileBuffer.at(XML_NAME).strFileBuf;
        XmlParser *m_LocalConfig = new XmlParser();
        m_LocalConfig->Parse((PCCH)pXmlBuf);

        versionMap.clear();
        string value=m_LocalConfig->get_XML_Value("External_Ver");
        if(value!="") {
            versionMap[XML_KEY_FIRMWARE_EX_VER]=value;
            versionMap[XML_KEY_WINDOWS_VER] =m_LocalConfig->get_XML_Value("WKIT");
            versionMap[XML_KEY_MAC_VER] =m_LocalConfig->get_XML_Value("MKIT");
            versionMap[XML_KEY_QCN_VER] =m_LocalConfig->get_XML_Value("EFS2");
            versionMap[XML_KEY_LINUX_VER] = "NA";
            versionMap[XML_KEY_WEBUI_VER] = "NA";
            versionMap[XML_KEY_PTS_VER] = "NA";
        } else {
            versionMap[XML_KEY_FIRMWARE_EX_VER] = m_LocalConfig->get_XML_Value("Firmware_External_Ver");
            versionMap[XML_KEY_WINDOWS_VER] = m_LocalConfig->get_XML_Value("WINDOWS");
            versionMap[XML_KEY_MAC_VER] =m_LocalConfig->get_XML_Value("MAC");
            versionMap[XML_KEY_LINUX_VER] =m_LocalConfig->get_XML_Value("LINUX");
            versionMap[XML_KEY_WEBUI_VER] =m_LocalConfig->get_XML_Value("WEBUI");
            versionMap[XML_KEY_QCN_VER] =m_LocalConfig->get_XML_Value("QCN");
            versionMap[XML_KEY_PTS_VER] = m_LocalConfig->get_XML_Value("PTS_Number");
        }
        delete m_LocalConfig;
    }
    return true;
}


bool ImgUnpack::ParseQCN() {
    if (qcnDataBuffer) {
        RELEASE_ARRAY(qcnDataBuffer);
    }

    map<string,FileBufStruct>::iterator it = m_downloadFileBuffer.find(QCN_NAME);

    if(it != m_downloadFileBuffer.end()) {
        uint32 dwFileLens = m_downloadFileBuffer.at(QCN_NAME).uFileLens;
        uint8* pQcnBufTemp = m_downloadFileBuffer.at(QCN_NAME).strFileBuf;

        NEW_ARRAY(qcnDataBuffer, uint8, dwFileLens);
        memcpy(qcnDataBuffer, pQcnBufTemp, dwFileLens);
        qcnLens = dwFileLens;
    } else {
        return false;
    }

    NEW_ARRAY(staticQcnBuf, uint8, qcnLens);

    NV_ITEM_PACKET_INFO* itemPacket = (NV_ITEM_PACKET_INFO *)qcnDataBuffer;
    bool bFind = false;
    uint32 uTimes = 0;
    uint32 itemSize = qcnLens/sizeof(NV_ITEM_PACKET_INFO);
    while(itemSize) {
        bFind = false;
        if (imgVersion != IMG_VERSION_5) {
            uint16 lens = nvItemsIgnore.size();
            for (uint16 i = 0; i < lens; i++) {
                if (itemPacket->nvItemID == atoi(nvItemsIgnore[i].c_str()))//TODO::::
                {
                    bFind = true;
                    break;
                }
            }
        }
        if (!bFind) {
            memcpy( staticQcnBuf + uTimes*sizeof(NV_ITEM_PACKET_INFO),
                   itemPacket,sizeof(NV_ITEM_PACKET_INFO));

            uTimes++;
        }
        itemPacket++;
        itemSize--;
    }

    qcnLens = uTimes*sizeof(NV_ITEM_PACKET_INFO);

    it = m_downloadFileBuffer.find(QCN_NAME);
    if(it != m_downloadFileBuffer.end()) {
        m_downloadFileBuffer.at(QCN_NAME).uFileLens = qcnLens;
        m_downloadFileBuffer.at(QCN_NAME).strFileBuf = staticQcnBuf;
    }

    if (qcnDataBuffer) {
        RELEASE_ARRAY(qcnDataBuffer);
    }

    return true;
}
