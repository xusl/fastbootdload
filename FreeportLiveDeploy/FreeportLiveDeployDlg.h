
// FreeportLiveDeployDlg.h : header file
//

#pragma once
#include "adb_dev_register.h"
#include "scsicmd.h"
#include "adbhost.h"
#include "list.h"
#include <map>
#include <string>
using std::map;
using std::string;
using std::greater;
using std::hash;
enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_INIT_DEVICE,
    UI_MESSAGE_UPDATE_PROMPT,
    UI_MESSAGE_PATCH,
};

enum
{
  TIMER_SWITCH_DISK = 0,
  TIMER_PUSH_FILES,
  TIMER_INSTALL_ADB_DRIVER,
  TIMER_ADB_HANDLER,
};

enum
{
  DEPLOY_STAT_OK = 0,
  DEPLOY_STAT_DOING = -1,
  DEPLOY_STAT_NONEDEVICE = -2,
};

//#define PATCH_CONF
typedef struct PatchFile {
  struct listnode node;
  char *local;
  char *remote;
}PatchFile;

class CFreeportLiveDeployDlg;
typedef struct WorkThreadParam{
    CFreeportLiveDeployDlg  *hDlg;
    CWinThread       *hWorkThread;
} WorkThreadParam;
// CFreeportLiveDeployDlg dialog

class CFreeportLiveDeployDlg : public CDialogEx
{
// Construction
public:
	CFreeportLiveDeployDlg(CWnd* pParent = NULL);	// standard constructor
    ~CFreeportLiveDeployDlg();
// Dialog Data
	enum { IDD = IDD_FREEPORTLIVEDEPLOY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

  LRESULT LiveDeploy(BOOL bPrompt);
  BOOL GetConfig(LPCSTR lpFileName);
  usb_handle* GetUsbHandle();

// Implementation
protected:
  HICON m_hIcon;
  usb_handle* m_hUSBHandle;
  BOOL m_bSwitchDisk;
  BOOL m_bAdbAlready;  //adb is present if device attach, do not need switch.
  BOOL m_bInstallDriver;
  BOOL m_bDoDeploy;
  int  m_GetAdbHandleTicks;
  HDEVNOTIFY hDeviceNotify;
#ifdef PATCH_CONF
#ifdef USE_CPP_MAP
  map<string, string, greater<string>> m_PatchCmd;
#else
  struct listnode key_list;
#endif
#endif
  WorkThreadParam mThreadparam;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	LRESULT OnInitDevice(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdatePrompt(WPARAM wParam, LPARAM lParam);
	LRESULT OnPatch(WPARAM wParam, LPARAM lParam);
	LRESULT PushFile(adbhost & adb, const char *lpath, const char *rpath);
	static LRESULT InstallAdbDriver(CFreeportLiveDeployDlg   *hWnd);
    LRESULT RefreshDevice(VOID);
    LRESULT InstallAndDeploy(VOID);
    VOID SendSetTipsMsg(LPCTSTR lpszString);
    VOID ConfirmMessage(VOID);
    BOOL ToggleConfirmWindow(BOOL show);
    static UINT WorkThread(LPVOID wParam);
    afx_msg void OnPaint();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedOk();

public:
    CStatic *m_hDevchangeTips;
};