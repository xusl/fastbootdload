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
#include "PST.h"
#include "DlWorker.h"

#include "adbhost.h"
#include "SettingsDlg.h"
#include "diagcmd.h"
#include "jrddiagcmd.h"
#include <ConfigIni.h>

//#define INLINE_SETTING


#define THREADPOOL_SIZE	4

enum
{
  TIMER_EVT_ADBKILLED = 0,
  TIMER_EVT_REJECTCDROM,
  TIMER_EVT_COMPORT,
  TIMER_EVT_USBADB,
  TIMER_EVT_ALL
};

// CmdmfastbootDlg 对话框
class CmdmfastbootDlg : public CDialog
{
 friend CSettingsDlg;
// 构造
public:
	CmdmfastbootDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CmdmfastbootDlg();

// 对话框数据
	enum { IDD = IDD_MDMFASTBOOT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	LRESULT OnUpdateProgressInfo(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdatePackageInfo(WPARAM wParam, LPARAM lParam);
// 实现
protected:
	HICON m_hIcon;
  BOOL m_UpdateDownloadFlag;
  unsigned int m_updated_number;
  CString m_strModuleName;
  PSTManager mPSTManager;
  CListCtrl   *m_imglist;
  CComboBox m_PackageHistory;
  CComboBox m_NicComboBox;
  int        m_PackageSelIdx;

//#ifdef INLINE_SETTING
//  CSettingsDlg m_SetDlg;
//#endif
  //CListCtrl  *m_port;
	//CThreadPool<CDlWorker> m_dlWorkerPool;

 public:
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
	CString m_PackagePath;

	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  BOOL SetDlgItemPos(UINT nID, int x, int y);
  BOOL UpdatePackageInfo(BOOL update = TRUE);
  BOOL SetupDevice(int evt);
  static void CALLBACK DeviceEventTimerProc(HWND hWnd,  UINT nMsg,  UINT_PTR nIDEvent,  DWORD dwTime);
  LRESULT OnDeviceInfo(WPARAM wParam, LPARAM lParam);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedCancel();
  afx_msg void OnBnClickedSetting();
	afx_msg void OnClose();
	afx_msg void OnDestroy();

private:
#ifdef INLINE_SETTING
  BOOL InitSettingDlg(void);
#endif
  BOOL SetWorkStatus(BOOL bwork, BOOL bforce);
  VOID LayoutControl();
  VOID SetDialogSize();
  BOOL StopConfirm();
  VOID SetupNicList();

public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnSelchangeCbPackagePath();
  afx_msg void OnCbnSelchangeComboNic();
  afx_msg void OnSettingsOptions();
	afx_msg void OnFileM850();
	afx_msg void OnFileM801();
};
