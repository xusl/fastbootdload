// GetProfileDlg.h : 头文件
//

#pragma once
#include "adb_dev_register.h"

// CGetProfileDlg 对话框
class CGetProfileDlg : public CDialog
{
// 构造
public:
	CGetProfileDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GETPROFILE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
  VOID DoGetProfile(VOID);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void CGetProfileDlg::OnDestroy();
	DECLARE_MESSAGE_MAP()
};
