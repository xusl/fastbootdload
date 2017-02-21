#pragma once
#include "MmiTestDialog.h"
#include "ProgramDialog.h"
// CReliabilityDialog dialog

class CReliabilityDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CReliabilityDialog)

public:
	CReliabilityDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CReliabilityDialog();

// Dialog Data
	enum { IDD = IDD_RELIABILITY };
private:  
  CTabCtrl m_TabCtrl;
  MmiTestDialog m_MMITestDlg;
  CProgramDialog m_ProgramDlg;
  
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
  
	DECLARE_MESSAGE_MAP()
};
