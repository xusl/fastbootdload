// mdmfastbootDlg.h : 头文件
//

#pragma once

#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include <initguid.h>
#include <setupapi.h>
#include "PortStateUI.h"
#include "usb_adb.h"
#include <atlutil.h>
#include "fastbootflash.h"
#include "DlWorker.h"

#include "adbhost.h"

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
  USB_STAT_FINISH
};

class CmdmfastbootDlg;

typedef struct UsbWorkData{
    CWnd         *hWnd;
    CPortStateUI  ctl;
    CWinThread   *work;
    flash_image  *img;
    usb_handle   *usb;
    int           usb_sn;
    int           stat;
} UsbWorkData;

#define THREADPOOL_SIZE	4
static const int PORT_NUM = 9;
static const int PORT_LAYOUT_ROW = 3;
static const int PARTITION_NAME_LEN = 32;

// CmdmfastbootDlg 对话框
class CmdmfastbootDlg : public CDialog
{
// 构造
public:
	CmdmfastbootDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MDMFASTBOOT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	LRESULT OnUpdateProgressInfo(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdatePackageInfo(WPARAM wParam, LPARAM lParam);
// 实现
protected:
	HICON m_hIcon;
  BOOL m_bInit;
  volatile BOOL m_bWork;
  int m_nPort;
  int m_nPortRow;
	CThreadPool<CDlWorker> m_dlWorkerPool;
  CString m_ConfigPath;
  flash_image *m_image;
  UsbWorkData m_workdata[PORT_NUM];

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	DECLARE_MESSAGE_MAP()

	void OnHelp();
	void OnAbout();
public:
	//port UI

	CString m_FwVer;
	CString m_QCNVer;
	CString m_LinuxVer;
	CString m_PackagePath;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL RegisterAdbDeviceNotification(void);
  BOOL AdbUsbHandler(BOOL update_device);
  BOOL SetPortDialogs(UINT nType, int x, int y, int w, int h);
  LRESULT OnDeviceInfo(WPARAM wParam, LPARAM lParam);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
	void SetUpAdbDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	afx_msg void OnDestroy();

private:
    BOOL InitUsbWorkData(void);
    UsbWorkData * GetUsbWorkData(long usb_sn );
    UsbWorkData * FindUsbWorkData(long usb_sn);
    BOOL SetUsbWorkData(UsbWorkData *data, usb_handle * usb);
    BOOL CleanUsbWorkData(UsbWorkData *data);
    BOOL SwitchUsbWorkData(UsbWorkData *data);
    BOOL FinishUsbWorkData(UsbWorkData *data);
    BOOL IsHaveUsbWork(void);
    UINT UsbWorkStat(UsbWorkData *data);
    BOOL SetWorkStatus(BOOL bwork, BOOL bforce);
    BOOL InitSettingConfig(void);
public:
	afx_msg void OnBnClickedSetting();
	};
