// GetProfileDlg.h : 头文件
//

#pragma once
#include "adb_dev_register.h"
#include "scsicmd.h"
#include <vector>
#include "afxwin.h"

enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_INIT_DEVICE,
};

enum
{
  TIMER_SWITCH_DISK = 0,
  TIMER_PROFILE_LIST,
};

// CGetProfileDlg 对话框
class CGetProfileDlg : public CDialog
{
// 构造
public:
	CGetProfileDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CGetProfileDlg();

// 对话框数据
	enum { IDD = IDD_GETPROFILE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
  int FilterProfiles(std::vector<PCCH> src, PCHAR filterWords, std::vector<PCCH> &outData); 
  BOOL ParseContent(char *content , PCHAR lineDelim,  std::vector<PCCH>& dataOut);
  BOOL ParseProfilesList(char * content , PCHAR lineDelim, PCHAR recordDelim);
  VOID GetProfilesList(BOOL trySwitchDisk);
  VOID DoGetProfilesList(usb_handle* handle);
  BOOL DoPokeProfile(usb_handle* handle, PCHAR profileName, PCHAR *data);
  BOOL CheckDeviceProfilePath(usb_handle* handle);
  VOID ShowSpecficProfile(CString profile);
  usb_handle* GetUsbHandle();

// 实现
protected:
  HICON m_hIcon;
  usb_handle* m_hUSBHandle;
  //adbhost m_AdbHost;
  CString m_filterWords;
  CListCtrl *m_hProfileList;
  CListBox *m_hProfileDataList;
  CStatic *m_hProfileName;
  CStatic *m_hDevchangeTips;
  std::vector<PCCH> m_pProfiles;
  std::vector<PCCH> m_pFilterProfiles; // 装载过滤后的profiles
  CStringA m_DeviceProfilePath;
  BOOL m_bSwitchDisk;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	LRESULT OnInitDevice(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void CGetProfileDlg::OnDestroy();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedListProfile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListProfile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnAbout();
	afx_msg void OnLbnSelchangeListProfileData();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeFilterwordsEdit();
};
