#pragma once
#include "afxdialogex.h"

// CSettingsDlg 对话框

//class CmdmfastbootDlg;

class CSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSettingsDlg();

  BOOL SetScheduleData(int *pData);
  BOOL SetFlashDirectData(int *pData);
  BOOL SetForeUpdateData(int *pData);
  void EnableSettings(BOOL enable);

// 对话框数据
	enum { IDD = IDD_SETTINGS };

	//CmdmfastbootDlg* m_pParent;
private:
  int *m_pSchedule;
  int *m_pFlashdirect;
  int *m_pForceupdate;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
  BOOL OnToolTipNotify(UINT id, NMHDR *pNMH,  LRESULT *pResult);

  afx_msg void OnBnClickedCheckForceupdate();
  afx_msg void OnBnClickedFastbootOnly();
  afx_msg void OnBnClickedScheRemove();
};
