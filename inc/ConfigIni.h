#pragma once
#include "stdafx.h"
#include "define.h"
#include <XmlParser.h>
#include <map>
#include <string>
#include <list>
#include <usb_vendors.h>

using namespace std;

static const int PORT_NUM_MAX = 9;

enum PlatformType {
  PLATFORM_CPE = 0,
  PLATFORM_MIFI,
  PLATFORM_MAX
};

class ProjectConfig {
  public:
    ProjectConfig(CString configFile = _T("\\."));
    ~ProjectConfig();
    BOOL  ReadConfig(list<CString> &codes);

    VOID SetProjectCode(CString code) { mCode = code;}
    VOID SetPlatform(CString platform) { mPlatform = platform; }
    VOID SetVersion(int version) { mVersion = version; }
    VOID SetValid(BOOL valid) { mIsValidConfig = valid; }
    VOID SetVid(int vid) { mVid = vid; }
    VOID SetPid(int pid) { mPid = pid; }

    BOOL  IsValid() { return mIsValidConfig; }
    CString GetConfigPath() { return mProjectConfigPath; }
    CString GetProjectCode() { return mCode; }
    CString GetPlatform() { return mPlatform; }
    CString GetCPEFlagFile() { return mCPEFlagFile; }
    CString GetModemSubDir() { return mModemSubDir; }
    int   GetVersion() { return mVersion; }
    BOOL  GetDiagPSTNandPrg(wchar_t *filename, int size, BOOL emergency);
    BOOL  IsUseAdbShell() { return mUseAdbShell;}
    int   GetVid() { return mVid; }
    int   GetPid() { return mPid; }
    PlatformType GetPlatformType() { return mPlatformType; }

private:
    VOID  SetPlatformType(CString type);

private:
    CString mProjectConfigPath;
    CString mCode;
    CString mPlatform;
    CString mCPEFlagFile;
    CString mModemSubDir;
    int    mVersion;
    BOOL   mIsValidConfig;
    BOOL   mUseAdbShell;
    PlatformType mPlatformType;
    int    mVid;
    int    mPid;
};

class PackageConfig {
  public:
    PackageConfig();
    ~PackageConfig();
    BOOL  Set(CString path);
    CString GetProjectCode() { return GetConfig("Project_Code");}
    CString GetCURef() { return GetConfig("CURef"); }
    CString GetVersion() { return GetConfig("External_Ver");}
    CString GetCustomerCode() { return GetConfig("Customer_Code");}
    CString GetFlashType() { return GetConfig("Flash_Code");}
    string GetPartition() { return m_ConfigXmlParser.get_XML_Value("PARTITION");}
    CString GetConfig(PCCH name) {
      return CString(m_ConfigXmlParser.get_XML_Value(name).c_str());
    }
    const wchar_t *GetPkgDlImgPath(void) {return pkg_dlimg_file;};
    const wchar_t *GetPkgConfXmlPath(void) {return pkg_conf_file;};
    const wchar_t *GetPkgQcnPath(void) {return pkg_qcn_file;};
private:
    XmlParser               m_ConfigXmlParser;
    wchar_t                 pkg_conf_file[MAX_PATH];
    wchar_t                 pkg_dlimg_file[MAX_PATH];
    wchar_t                 pkg_qcn_file[MAX_PATH];
};


static const int PARTITION_NUM_MAX = 32;
static const int PARTITION_NAME_LEN = 32;
static const int PARTITION_TBL_LEN = PARTITION_NUM_MAX * PARTITION_NAME_LEN;

typedef struct FlashImageInfo {
    struct FlashImageInfo *next;
    wchar_t *partition;
    char *partition_str;
    wchar_t *lpath;
	  void *data;
	  unsigned size;
	  bool need_download;
}FlashImageInfo;

class flash_image{
  public:
    friend class AppConfig;
    flash_image(AppConfig *appConfig);
    ~flash_image();
    const FlashImageInfo* get_partition_info(wchar_t *partition, void **ppdata, unsigned *psize);
    const FlashImageInfo* image_enum_init (void) ;
    const FlashImageInfo* image_enum_next (const FlashImageInfo* img);

    BOOL qcn_cmds_enum_init (char *cmd);
    const char* qcn_cmds_enum_next (unsigned int index);
	  BOOL set_download_flag(CString strPartitionName, bool bDownload);
    int read_fastboot_config(const wchar_t* config, const wchar_t* pkg_dir);
    int read_diagpst_config(const wchar_t* config, const wchar_t* pkg_dir);
    int read_openwrt_config(const wchar_t* config, const wchar_t* pkg_dir);
    int GetDiagDlImgSize();
    int GetFbDlImgSize();
    BOOL ReadPackage();
    BOOL reset(BOOL free_only);

    int SetPartitionDownloadFlag(CString partition, boolean flag);

    bool AddFileBuffer(const wchar_t *partition, const wchar_t *pkgPath, const wchar_t *filName);
    map<string, FileBufStruct> GetFileBuffer() { return m_dlFileBuffer; }
    map<string, CString> GetOpenWrtFiles() { return m_OpenWrtFiles; }
    BOOL IsCPEPackage() { return m_OpenWrtFiles.size() > 0; }
    CString GetOpenWrtFilePath(string& filename) { return m_OpenWrtFiles.at(filename);}

  protected:
    //virtual int parse_pkg_sw(CString & node, CString & text);
    //virtual int parse_pkg_hw(CString & node, CString & text);

  private:
    int add_image(wchar_t *partition, const wchar_t *lpath, BOOL write =FALSE, const wchar_t* config = NULL);
    //void read_package_version(const wchar_t * package_conf);

  private:
    AppConfig *mAppConfig;
    FlashImageInfo *image_list; //for image download by FASTBOOT
    FlashImageInfo *image_last;
    map<string, FileBufStruct> m_dlFileBuffer;//for image download by DIAG
    map<string, CString> m_OpenWrtFiles; // for image download by ETHERNET (HTTP or TFTP)
    unsigned int nv_num;
    char ** nv_buffer;
    char * nv_cmd;
    uint32          mDiagDlImgSize;
    uint32          mFbDlImgSize;
};

class AppConfig{
public:
    AppConfig();
    ~AppConfig();
    BOOL         ReadConfigIni(const wchar_t * ini = L"mdmconfig.ini");
    const wchar_t *GetAppConfIniPath(void) { return m_ConfigPath.GetString();};
    const wchar_t *GetPkgDir(void) { return pkg_dir;}
    const wchar_t *GetModemPackagePath(void) { return m_ModemPackagePath.GetString();}
    const wchar_t *GetCPEPackagePath(void) { return m_CPEPackagePath.GetString();}
    const wchar_t *GetLogFilePath(void) {return log_file;};
    const char  *GetLogTag(void) { return log_tag;};
    const char  *GetLogLevel(void) { return log_level;};
    BOOL         GetFlashDirectFlag(void) { return m_flashdirect;};
    BOOL         GetPortDevFixedFlag(void) { return m_fix_port_map;};
    BOOL         GetForceUpdateFlag(void) { return m_forceupdate;};
    BOOL         GetAutoWorkFlag(void) { return m_bWork; };
    BOOL         GetAfterSaleMode(void) { return m_pack_img;};
    int          GetUiPortTotalCount(void) { return m_nPort;};
    int          GetPSTWorkTimeout(void) { return work_timeout;};
    BOOL         GetDiagPSTNandPrg(string &prgName, BOOL emergency);
    /*
    this is indicate that use debug mode and use adb command reboot-bootloader to enter fastboot.
    */
    BOOL         IsUseAdbShell() { return mProjectConfig.IsUseAdbShell(); }
    PlatformType GetPlatformType() { return mProjectConfig.GetPlatformType(); }

    BOOL         SetProjectCode(CString &projectCode);
    BOOL         GetProjectConfig(ProjectConfig& config) {
      config = mProjectConfig;
      return TRUE;
    }
    BOOL         SetPackageDir(const wchar_t * dir, CString& errMsg);
    VOID         GetPackageHistory(list<CString> & history) {
       history = m_PackageDirs;};
     PackageConfig *GetPackageConfig() {
      return &mPackageConfig;
    }
private:
    void         ParseProjectConfig(CString &projectCofig);
    void         SetupPackageInformation();
    void         ReadPackageHistory();
    void         WritePackageHistory();

private:
    CString                 m_ConfigPath;
    BOOL                    m_pack_img;
    BOOL                    m_fix_port_map;
    BOOL                    m_flashdirect;  //if device is in fastboot when attach, whether can flash image
    BOOL                    m_forceupdate; // do not check version, if not exist config.xml or version rule is not match
    volatile BOOL           m_bWork;
    int                     m_nPort;
    int                     switch_timeout;
    int                     work_timeout;
    wchar_t                 *log_file;
    char                    *log_tag;
    char                    *log_level;
    wchar_t                 pkg_dir[MAX_PATH];
    CString                 m_ModemPackagePath;
    CString                 m_CPEPackagePath;
    list<CString>           m_PackageDirs;
    map<CString, ProjectConfig> m_SupportProject;
    ProjectConfig            mProjectConfig;
    PackageConfig            mPackageConfig;
    CString m_strModuleName;
};
