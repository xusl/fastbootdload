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

#include "SettingsDlg.h"
#include "qcnlib/QcnParser.h"

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

class CmdmfastbootDlg;

typedef struct UsbWorkData{
    CmdmfastbootDlg  *hWnd;
    CPortStateUI      ctl;
    CWinThread       *work;
    //flash_image  *img;
    usb_handle       *usb;
    int              usb_sn;
    int              usb_sn_port;
    int              stat;
    FlashImageInfo const * flash_partition[PARTITION_NUM_MAX];
    short           partition_nr;
    BOOL            update_qcn;
} UsbWorkData;

#define THREADPOOL_SIZE	4
static const int PORT_NUM_MAX = 9;


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
  unsigned int m_updated_number;

 friend CSettingsDlg;
  //configuration
  BOOL m_pack_img;
  BOOL m_fix_port_map;
  BOOL m_flashdirect;
  BOOL m_forceupdate; // do not check version, if not exist config.xml or version rule is not match
  int m_nPort;
  int m_nPortRow;
  int switch_timeout;
  int work_timeout;
  CString m_strModuleName;

	//CThreadPool<CDlWorker> m_dlWorkerPool;
  CString m_ConfigPath;
  flash_image *m_image;
  UsbWorkData m_workdata[PORT_NUM_MAX];
  CListCtrl  *m_imglist;
  //CListCtrl  *m_port;
  CSettingsDlg m_SetDlg;

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

	static MODULE_NAME m_module_name;
	CString m_FwVer;
	CString m_QCNVer;
	CString m_LinuxVer;
	CString m_PackagePath;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL RegisterAdbDeviceNotification(void);
  BOOL AdbUsbHandler(BOOL update_device);
  BOOL SetPortDialogs(int x, int y, int w, int h);
  BOOL SetDlgItemPos(UINT nID, int x, int y);
  BOOL UpdatePackageInfo(void);
	void SetUpAdbDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
  LRESULT OnDeviceInfo(WPARAM wParam, LPARAM lParam);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedCancel();
  afx_msg void OnBnClickedSetting();
	afx_msg void OnClose();
	afx_msg void OnDestroy();

private:
    static UINT usb_work(LPVOID wParam);
    static UINT adb_hw_check(adbhost& adb, UsbWorkData* data);
    static UINT adb_sw_version_cmp(adbhost& adb, UsbWorkData* data);
    static UINT sw_version_parse(UsbWorkData* data,PCCH key, PCCH value);
    static UINT adb_shell_command(adbhost& adb, UsbWorkData* data, PCCH command,
                                UI_INFO_TYPE info = UI_DEFAULT);
    static UINT adb_write_IMEI(adbhost& adb, UsbWorkData* data);
    static UINT adb_update_NV(adbhost& adb, UsbWorkData* data,  flash_image  *const image);
    static UINT ui_text_msg(UsbWorkData* data, UI_INFO_TYPE info_type, PCCH msg);

private:
    BOOL InitSettingDlg(void);
    BOOL InitUsbWorkData(void);
    UsbWorkData * GetUsbWorkData(long usb_sn, long usb_sn_port, BOOL fix_map);
    UsbWorkData * FindUsbWorkData(long usb_sn, long usb_sn_port);
    BOOL SetUsbWorkData(UsbWorkData *data, usb_handle * usb);
    BOOL CleanUsbWorkData(UsbWorkData *data, BOOL schedule = TRUE);
    BOOL SwitchUsbWorkData(UsbWorkData *data);
    BOOL FinishUsbWorkData(UsbWorkData *data);
    BOOL AbortUsbWorkData(UsbWorkData *data);
    BOOL ResetUsbWorkData(void);
    BOOL IsHaveUsbWork(void);
    UINT UsbWorkStat(UsbWorkData *data);
    BOOL SetWorkStatus(BOOL bwork, BOOL bforce);
    BOOL InitSettingConfig(void);
	DWORD FindProcess(wchar_t *strProcessName, CString &AppPath);
	BOOL UnableAdb();
	void UpdatePackage();
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileM850();
	afx_msg void OnFileM801();
};
