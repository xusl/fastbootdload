#pragma once

#include "define.h"
#include <string>
#include <map>
#include "device.h"
#include <XmlParser.h>
#include <ConfigIni.h>
#include "adb_dev_register.h"

using namespace std;

#define VERTICAL_GAP    5
#define HORIZONAL_GAP   5

#define VERTICAL_BASE    15
#define HORIZONAL_BASE   15

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
    int GetDiagDlImgSize();
    int GetFbDlImgSize();
    BOOL ReadPackage();
    BOOL reset(BOOL free_only);

    int SetPartitionDownloadFlag(CString partition, boolean flag);

     bool AddFileBuffer(const wchar_t *partition, const wchar_t *pkgPath, const wchar_t *filName);
     map<string,FileBufStruct> GetFileBuffer() { return m_dlFileBuffer;};

  protected:
    //virtual int parse_pkg_sw(CString & node, CString & text);
    //virtual int parse_pkg_hw(CString & node, CString & text);

  private:
    int add_image(wchar_t *partition, const wchar_t *lpath, BOOL write =FALSE, const wchar_t* config = NULL);
    //void read_package_version(const wchar_t * package_conf);

  private:
    AppConfig *mAppConfig;
    FlashImageInfo *image_list;
    FlashImageInfo *image_last;
    unsigned int nv_num;
    char ** nv_buffer;
    char * nv_cmd;
    map<string,FileBufStruct>  m_dlFileBuffer;
    uint32          mDiagDlImgSize;
    uint32          mFbDlImgSize;
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
    UsbWorkData(int index, CWnd* pParentWnd,
      AppConfig *appConf, flash_image* package);
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
    BOOL SetInfo(UI_INFO_TYPE info_type, PCCH msg);
    BOOL SetInfo(UI_INFO_TYPE infoType, CString strInfo);
    UINT SetProgress(int progress);
    BOOL SetPromptMsg(PCCH msg) { return SetInfo(PROMPT_TEXT, msg);};
    const char *GetDevTag() { return mActiveDevIntf->GetDevTag();};
    float GetElapseSeconds();
    BOOL Log(const char * msg);
    int GetStatus() { return stat;};
    BOOL CheckValid() {
      return hWnd != NULL && mProjectPackage != NULL && mPAppConf != NULL ;
    }
    BOOL UpdateUsbHandle(BOOL force, BOOL flashdirect);
    AppConfig      * GetAppConfig() { return mPAppConf;}
    BOOL ShowSubWindow(BOOL show);

  private:
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
    AppConfig      *mPAppConf;
    flash_image    *mProjectPackage;
    short           partition_nr;
    BOOL            update_qcn;
} ;

class PSTManager {
public:
  PSTManager(AFX_THREADPROC pfnThreadProc);
  ~PSTManager();
  BOOL Initialize(CWnd *hWnd, BOOL showPort=TRUE);
  VOID SetWork(BOOL work, BOOL schedule=TRUE);
  BOOL IsWork() { return m_bWork; }
  BOOL IsAfterSaleMode() { return mAppConf.GetAfterSaleMode(); }
  BOOL IsSuperMode() { return mAppConf.GetFlashDirectFlag() || mAppConf.GetForceUpdateFlag(); }
  UINT GetPortRows();
  CPortStateUI* GetPortUI(UINT index);
  RECT GetPortRect(UINT index = 0);
  UsbWorkData *FindUsbWorkData(wchar_t *devPath);
  BOOL FlashDeviceDone(UsbWorkData * data);
  BOOL IsInit() { return m_bInit;}
  BOOL IsHaveUsbWork(void);
  BOOL ScheduleDeviceWork();
  BOOL Reset();

  BOOL RejectCDROM(VOID);
  BOOL HandleComDevice(BOOL schedule=TRUE);
  BOOL EnumerateAdbDevice(BOOL schedule=TRUE);
  BOOL HandleDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
  BOOL HandleDeviceArrived(wchar_t *devPath);

  int GetPortNum();
  int GetPortGridWidth() { return m_GridWidth;}
  int GetPortGridHeight() { return m_GridHeight;}
  BOOL SetPortDialogs(int x, int y);
  BOOL SetPortDialogs(int x, int y, int w, int h);
  AppConfig* GetAppConfig() { return &mAppConf;}
  flash_image* GetProjectPackage() { return m_image;}
  BOOL ChangePackage(const wchar_t * dir);
  const wchar_t * GetPackage() { return mAppConf.GetUpdateImgPkgDir();}
  BOOL SetDownload(CString partition, bool bDownload) {
    return m_image->set_download_flag(partition, bDownload);
  }

  void GetPackageHistory(list<CString> &history) {
    return mAppConf.GetPackageHistory(history);
  }

private:
  int               m_GridHeight;
  int               m_GridWidth;
  BOOL              m_bInit;
  AppConfig          mAppConf;
  flash_image       *m_image;
  UsbWorkData       *m_workdata[PORT_NUM_MAX];

  vector<CDevLabel>  m_WorkDev;
  DeviceCoordinator  mDevCoordinator;
  AFX_THREADPROC     mThreadProc;
  volatile BOOL      m_bWork;
};
