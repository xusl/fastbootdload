#pragma once

#include "define.h"
#include <string>
#include <map>
#include "device.h"
#include <XmlParser.h>
#include <ConfigIni.h>
#include "adb_dev_register.h"

using namespace std;


enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_UPDATE_PROGRESS_INFO,
	UI_MESSAGE_UPDATE_PACKAGE_INFO,
	UI_MESSAGE_DEVICE_INFO,
};

enum
{
  USB_STAT_IDLE,
  USB_STAT_WORKING,
  USB_STAT_SWITCH,
  USB_STAT_SWITCHED,
  USB_STAT_FINISH,
  USB_STAT_ERROR,
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
    flash_image(const wchar_t* config_file);
    ~flash_image();
    const FlashImageInfo* get_partition_info(wchar_t *partition, void **ppdata, unsigned *psize);
    const FlashImageInfo* image_enum_init (void) ;
    const FlashImageInfo* image_enum_next (const FlashImageInfo* img);
    const wchar_t * get_package_dir(void);
    //config.xml file path
    const wchar_t * get_package_config(void);
    //static qcn file path
    const wchar_t * get_package_qcn_path(void);

    BOOL qcn_cmds_enum_init (char *cmd);
    const char* qcn_cmds_enum_next (unsigned int index);

    BOOL set_package_dir(const wchar_t * dir);
    BOOL get_pkg_a5sw_sys_ver(CString &version);
    BOOL get_pkg_a5sw_usr_ver(CString &version);
    BOOL get_pkg_a5sw_kern_ver(CString &version);
    BOOL get_pkg_fw_ver(CString &version);
    BOOL get_pkg_qcn_ver(CString &version);
	  BOOL set_download_flag(CString strPartitionName, bool bDownload);
    int read_fastboot_config(const wchar_t* config);
    int read_diagpst_config(const wchar_t* config);
    int GetDiagDlImgSize();
    int GetFbDlImgSize();
    BOOL ReadPackage();

    int SetPartitionDownloadFlag(CString partition, boolean flag);

     bool AddFileBuffer(const wchar_t *partition, const wchar_t *pkgPath, const wchar_t *filName);
     map<string,FileBufStruct> GetFileBuffer() { return m_dlFileBuffer;};

  protected:
    virtual int parse_pkg_sw(CString & node, CString & text);
    virtual int parse_pkg_hw(CString & node, CString & text);

  private:
    int add_image(wchar_t *partition, const wchar_t *lpath, BOOL write =FALSE, const wchar_t* config = NULL);
    void read_package_version(const wchar_t * package_conf);
    BOOL reset(BOOL free_only);

  private:
    CString  mAppConfigFile;  //todo , will change to project config, for app will support many projects
    FlashImageInfo *image_list;
    FlashImageInfo *image_last;
    wchar_t pkg_dir[MAX_PATH];
    wchar_t pkg_conf_file[MAX_PATH];
    wchar_t pkg_qcn_file[MAX_PATH];
    unsigned int nv_num;
    char ** nv_buffer;
    char * nv_cmd;
    map<string,FileBufStruct>  m_dlFileBuffer;
    uint32          mDiagDlImgSize;
    uint32          mFbDlImgSize;
    CString a5sw_kern_ver;
    CString a5sw_usr_ver;
    CString a5sw_sys_ver;
    CString qcn_ver;
    CString fw_ver;
};

typedef enum
{
	FIRMWARE_VER,
	QCN_VER,
	LINUX_VER,
	SYSTEM_VER,
	USERDATA_VER,
	PTS_VER,
	TITLE,
	PROGRESS_VAL,
	PROMPT_TITLE,
	PROMPT_TEXT,
	ADB_CHK_ABORT,
	REBOOT_DEVICE,
	FLASH_DONE,
	UI_DEFAULT,
}UI_INFO_TYPE;

typedef struct _UIInfo_
{
	UI_INFO_TYPE	infoType;
	int				    iVal;
	CString			  sVal;

  _UIInfo_() {
    infoType = UI_DEFAULT;
    sVal = "";
    iVal = -1;
  }
}UIInfo;


class CmdmfastbootDlg;
class CPortStateUI;
#define WORK_NAME_LEN 16

class UsbWorkData{
  public:
    UsbWorkData(int index, CWnd* pParentWnd, DeviceCoordinator *coordinator,
      ConfigIni *appConf, XmlParser *xmlParser, flash_image* package);
    ~UsbWorkData();
    BOOL Clean(BOOL noCleanUI=TRUE);
    BOOL IsIdle();
    BOOL Reset(VOID);
    BOOL Abort(VOID);
    BOOL Start(DeviceInterfaces* devIntf, AFX_THREADPROC pfnThreadProc, UINT nElapse, BOOL flashdirect);
    BOOL Finish(VOID);
    BOOL SwitchDev(UINT nElapse);
    BOOL SetSwitchedStatus();
    DWORD  WaitForDevSwitchEvt(DWORD dwMilliseconds = INFINITE);
    DWORD  SetDevSwitchEvt(BOOL flashdirect);
    UINT ui_text_msg(UI_INFO_TYPE info_type, PCCH msg);
    UINT SetProgress(int progress);
    UINT SetPromptMsg(PCCH msg) { return ui_text_msg(PROMPT_TEXT, msg);};
    const char *GetDevTag() { return mActiveDevIntf->GetDevTag();};
    float GetElapseSeconds();
    BOOL SetInfo(UI_INFO_TYPE infoType, CString strInfo);
    BOOL Log(const char * msg);
    int GetStatus() { return stat;};
    XmlParser *GetXmlParser() { return mPLocalConfigXml;};

  private:
    DeviceCoordinator *pCoordinator;
    long long       start_time_tick;
    HANDLE          mDevSwitchEvt;
    wchar_t         mName[WORK_NAME_LEN];
    int             stat;

  public:
    CWnd/*CmdmfastbootDlg*/  *hWnd;
    CPortStateUI     *pCtl;
    CWinThread       *work;
    usb_handle       *usb;
    //this is the serial number for logical ui.
    DeviceInterfaces*  mActiveDevIntf;
    DeviceInterfaces*  mMapDevIntf;
    FlashImageInfo const *flash_partition[PARTITION_NUM_MAX];
    ConfigIni      *mPAppConf;
    XmlParser      *mPLocalConfigXml;
    flash_image    *mProjectPackage;
    short           partition_nr;
    BOOL            update_qcn;
} ;

#if 1
class PSTManager {
public:
  PSTManager(AFX_THREADPROC pfnThreadProc);
  ~PSTManager();
  BOOL Initialize(CWnd *hWnd);
  BOOL IsWork() { return m_bWork; }
  VOID SetWork(BOOL work) { m_bWork = work; }
  BOOL IsAfterSaleMode() { return mAppConf.GetAfterSaleMode(); }
  BOOL IsSuperMode() { return mAppConf.GetFlashDirectFlag() || mAppConf.GetForceUpdateFlag(); }
  UINT GetPortRows() { return mAppConf.GetUiPortRowCount(); }
  CPortStateUI* GetPortUI(UINT index);
  UsbWorkData *FindUsbWorkData(wchar_t *devPath);
  BOOL FlashDeviceDone(UsbWorkData * data);
  BOOL IsHaveUsbWork(void);
  BOOL ScheduleDeviceWork();
  BOOL Reset();

  BOOL RejectCDROM(VOID);
  BOOL HandleComDevice(VOID);
  BOOL EnumerateAdbDevice(VOID);
  BOOL HandleDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
  BOOL HandleDeviceArrived(wchar_t *devPath);

  int GetPortNum();
  flash_image* GetProjectPackage() { return m_image;}
  BOOL ChangePackage(const wchar_t * dir);
  const wchar_t * GetPackage() { return m_image->get_package_dir();}
  BOOL SetDownload(CString partition, bool bDownload) {
    return m_image->set_download_flag(partition, bDownload);
  }

private:

private:
  ConfigIni          mAppConf;
  XmlParser          m_LocalConfigXml;
  flash_image       *m_image;
  UsbWorkData       *m_workdata[PORT_NUM_MAX];

  vector<CDevLabel>  m_WorkDev;
  DeviceCoordinator  mDevCoordinator;
  AFX_THREADPROC     mThreadProc;
  volatile BOOL      m_bWork;
};
#endif
