#pragma once

#include "define.h"
#include <string>
#include <map>
#include "device.h"
#include <XmlParser.h>
#include <ConfigIni.h>
#include "adb_dev_register.h"
#include "NicManager.h"
#include "CMiniHttpDownloadServer.h"

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
  USB_STAT_WAITING_CLOSE,
};


typedef enum
{
	TITLE,
	PROGRESS_VAL,
	PROMPT_TITLE,
	PROMPT_TEXT,
	ADB_CHK_ABORT,
	FLASH_DONE,
	PORTUI_DEVINFO,
	OPENWRT_UPDATED,
	UI_DEFAULT,
}UI_INFO_TYPE;

typedef struct _UIInfo_
{
	UI_INFO_TYPE	infoType;
	int				    iVal;
	CString			  sVal;
  CString       mInfoName;

  _UIInfo_(UI_INFO_TYPE type, CString text) {
    infoType = type;
    sVal = text;
    iVal = -1;
    mInfoName = "";
  }

  _UIInfo_(UI_INFO_TYPE type) {
    infoType = type;
    sVal = "";
    iVal = -1;
    mInfoName = "";
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
    BOOL SetDevice(DeviceInterfaces* pDevIntf, BOOL flashdirect);
    BOOL Start(AFX_THREADPROC pfnThreadProc);
    BOOL Finish(VOID);
    BOOL SwitchDev(UINT nElapse);
    DWORD  WaitForDevSwitchEvt(BOOL changeStatus, DWORD dwMilliseconds = INFINITE);
    DWORD  SetDevSwitchEvt(BOOL flashdirect);
    BOOL SetInfo(UI_INFO_TYPE info_type, PCCH msg);
    BOOL AddDevInfo(CString name, CString value);
    BOOL SetProgress(int progress);
    BOOL SetPromptMsg(PCCH msg, UI_INFO_TYPE info_type = PROMPT_TEXT);
    const char *GetDevTag() { return mActiveDevIntf->GetDevTag();};
    float GetElapseSeconds();
    BOOL RebootFastboot();
    int GetStatus() { return stat;};
    VOID SetStatus(int status) { stat = status;};
    VOID SetParallelMode(BOOL on) {m_ParallelMode = on;}
    BOOL CheckValid() {
      return hWnd != NULL && mProjectPackage != NULL && mPAppConf != NULL ;
    }
    BOOL UpdateUsbHandle(BOOL force, BOOL flashdirect);
    AppConfig      * GetAppConfig() { return mPAppConf;}
    BOOL ShowSubWindow(BOOL show);
    int GetIndex() { return mIndex;}
  private:
    long long       start_time_tick;
    HANDLE          mDevSwitchEvt;
    wchar_t         mName[WORK_NAME_LEN];
    int             stat;
    int             mIndex;

  public:
    CWnd/*CmdmfastbootDlg*/  *hWnd;
    CPortStateUI     *pCtl;
    CWinThread       *work;
    int               threadId;
    usb_handle       *usb;
    //this is the serial number for logical ui.
    DeviceInterfaces*  mActiveDevIntf;
    //for multiple port download, this is the tag. 
    //it set after the first device is updated.
    DeviceInterfaces*  mMapDevIntf;
    FlashImageInfo const *flash_partition[PARTITION_NUM_MAX];
    AppConfig      *mPAppConf;
    flash_image    *mProjectPackage;
    short           partition_nr;
    BOOL            update_qcn;
    BOOL            m_ParallelMode;
} ;

class PSTManager {
public:
  PSTManager(AFX_THREADPROC pfnThreadProc);
  ~PSTManager();

 public:
  BOOL Initialize(CWnd *hWnd, BOOL showPort=TRUE);
  VOID SetWork(BOOL work, BOOL schedule=TRUE);
  BOOL IsWork() { return m_bWork; }
  BOOL IsAfterSaleMode() { return mAppConf.GetAfterSaleMode(); }
  BOOL IsSuperMode() { return mAppConf.GetFlashDirectFlag() || mAppConf.GetForceUpdateFlag(); }

  UsbWorkData *GetWorkData(UINT index = 0);
  UsbWorkData *FindUsbWorkData(wchar_t *devPath);
  BOOL FlashDeviceDone();
  BOOL IsInit() { return m_bInit;}
  BOOL IsHaveUsbWork(void);
  BOOL ScheduleDeviceWork();
  BOOL Reset();

  BOOL RejectCDROM(VOID);
  BOOL HandleComDevice(BOOL schedule=TRUE);
  BOOL EnumerateAdbDevice(BOOL schedule=TRUE);
  BOOL HandleDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
  BOOL HandleDeviceArrived(wchar_t *devPath);

  int GetPortGridWidth() { return m_GridWidth;}
  int GetPortGridHeight() { return m_GridHeight;}
  BOOL SetPortDialogs(int x, int y);
  BOOL SetPortDialogs(int x, int y, int w, int h);
  UINT GetPortNum();
  UINT GetPortRows();
  CPortStateUI* GetPortUI(UINT index);
  RECT GetPortRect(UINT index = 0);

  NicManager * GetNicManager() { return &mNicManager;}

  AppConfig* GetAppConfig() { return &mAppConf;}
  flash_image* GetProjectPackage() { return m_image;}
  BOOL ChangePackage(const wchar_t * dir);
  const wchar_t * GetPackage() { return mAppConf.GetPkgDir();}
  BOOL SetDownload(CString partition, bool bDownload) {
    return m_image->set_download_flag(partition, bDownload);
  }

  void GetPackageHistory(list<CString> &history) {
    return mAppConf.GetPackageHistory(history);
  }

  UINT CPEModemPST(UsbWorkData *workData);

  static BOOL HttpServerGetFileCB (PVOID data, string filename, CString& filePath);
  static VOID HttpServerMessageCB (PVOID data, int uiPort, string message);
private:
    static UINT RunMiFiPST(LPVOID wParam);
    static UINT RunTelnetServer(LPVOID wParam);

private:
  int                   m_GridHeight;
  int                   m_GridWidth;
  BOOL                  m_bInit;
  AppConfig             mAppConf;
  flash_image           *m_image;
  UsbWorkData           *m_workdata[PORT_NUM_MAX];
  vector<CDevLabel>     m_WorkDev;
  DeviceCoordinator     mDevCoordinator;
  NicManager            mNicManager;
  CMiniHttpDownloadServer mHttpServer;  
  CWinThread            *mTelnetClientThread;
  HANDLE                mScheduleEvt;
  AFX_THREADPROC        mThreadProc;
  volatile BOOL         m_bWork;
};
