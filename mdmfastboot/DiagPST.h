#pragma once

#include "define.h"
#include "diagcmd.h"
#include "dlprg.h"
#include "hostdl.h"
#include "custdata.h"
#include "jrddiagcmd.h"
#include "saharacmd.h"
#include <map>
#include <string>
#include "XmlParser.h"

using namespace std;

class UsbWorkData;

typedef struct
{
    byte cmd_code;
    byte msm_hw_version_format;
    byte reserved[2]; /* for alignment / future use */
    uint32 msm_hw_version;
    uint32 mobile_model_id;
    /* The following character array contains 2 NULL terminated strings:
    'build_id' string, followed by 'model_string' */
    char ver_strings[32];
}get_version_rsp_type;

class DiagPST
{
public:
    DiagPST(UsbWorkData * worker, XmlParser *xmlParser, map<string,FileBufStruct> &filebuffer);
    ~DiagPST(void);


    bool EraseSimlock();
    bool checkIfFlashTypeMatchNormalMode();
    bool CompareVersions();
    bool CompareAllVersion(PCCH firmware_name);
    bool My_CompareDashboardVersion(PCCH Device_firmware,PCCH PC_firmware);
    bool Compare_EFS_Version(PCCH PC_firmware,PCCH Device_firmware);
    bool Compare_special_ver(PCCH firmware_name,PCCH Device_ver,PCCH PC_ver);

    virtual void initDLimgInfo();
    virtual void initCustDataInfo();
    virtual bool DownloadCheck();
    virtual bool RunTimeDiag();
    virtual bool DownloadImages(uint8 ratio, uint8 base);
    virtual bool DownloadCustomerInfo();
    virtual bool DownloadPrg(const wchar_t* config);
    virtual bool Calculate_length();

private:
    virtual bool SwitchOfflineMode();
    virtual bool RequestDeviceStatus();
    virtual TResult EnableDiagServer();
    virtual TResult DisableDiagServer();
    virtual bool checkIfPackageMatchNormalMode();
    virtual bool checkIfPackageMatchDlMode();
    virtual bool checkCusIdNormalMode();
    virtual bool checkIfPartitionMatchNormalMode();

    virtual bool GenerateFTFiles();
    virtual bool SetFuncFive();
    virtual bool StorePIC();

    bool RequestFirmwarVerAndMobileIdNormalMode();
    bool RequestExternalVersion();
    VOID SetPromptMsg(const char *fmt,  ...);
    BOOL StringSplit(char * content, PCHAR lineDelim, std::vector<string>& dataOut);



private:
    CPacket                         *m_DLLPacket;
    CCustData                       *m_pCustdata;
    CDLData                         *m_dlData;
    CDIAGCmd                        *m_DIAGCmd;
    CDLPrg                          *m_DLPrg;
    JRDdiagCmd                      *m_newDIAGCmd;
    SAHARACmd                       *m_sahara;
    UsbWorkData                     *m_Worker;
    map<string,FileBufStruct>        m_dlFileBuffer;
    uint32                           Software_size;
    TDLImgInfoType                  *m_pDLImgInfo;
    TCustDataInfoType               *m_pCustdataInfo;
    XmlParser                       *m_LocalConfigXml;

    bool                            m_blDownloadMode;
    bool                            m_bRestore;
    bool                            m_bForceMode;
    bool                            m_bEraseSimlock;
    bool                            m_isbDownload;
    bool                            m_bDownDashBrd;
    bool                            m_bDownFir;
    uint32                          m_iMobileId;
    string                          m_FirmwareVersion;
    uint16                          m_dlPort;
};
