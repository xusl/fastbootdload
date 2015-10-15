
// FreeportLiveDeployDlg.h : header file
//

#pragma once
#include "adb_dev_register.h"
#include "scsicmd.h"
#include "adbhost.h"
enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_INIT_DEVICE,
};

enum
{
  TIMER_SWITCH_DISK = 0,
  TIMER_PUSH_FILES,
  TIMER_INSTALL_ADB_DRIVER,
};
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

  VOID LiveDeploy(BOOL trySwitchDisk);
  usb_handle* GetUsbHandle();

// Implementation
protected:
	HICON m_hIcon;
  usb_handle* m_hUSBHandle;
  BOOL m_bSwitchDisk;
  HDEVNOTIFY hDeviceNotify;
  
	// Generated message map functions
	virtual BOOL OnInitDialog();
	LRESULT OnInitDevice(WPARAM wParam, LPARAM lParam);
	LRESULT PushFile(adbhost & adb, const char *lpath, const char *rpath);    
	LRESULT InstallAdbDriver(void);
    afx_msg void OnPaint();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

public:    
  CStatic *m_hDevchangeTips;
};
