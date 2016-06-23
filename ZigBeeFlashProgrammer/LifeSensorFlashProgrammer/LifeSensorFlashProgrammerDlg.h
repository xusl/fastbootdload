// JN516x Flash ProgrammerDlg.h : header file
//

#if !defined(AFX_LIFESENSORFLASHPROGRAMMERDLG_H__8649422B_BC30_4A5F_9A3C_6D0F0805BC8A__INCLUDED_)
#define AFX_LIFESENSORFLASHPROGRAMMERDLG_H__8649422B_BC30_4A5F_9A3C_6D0F0805BC8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyButton.h"

/////////////////////////////////////////////////////////////////////////////
// LifeSensorFlashProgrammerDlg dialog
typedef enum Operation
{
	GetComPorts = 0,
	Program
}Operation_t, *pOperation_t;

#define MAX 100

class LifeSensorFlashProgrammerDlg : public CDialog
{
// Construction
public:
	LifeSensorFlashProgrammerDlg(CWnd* pParent = NULL);	// standard constructor
    DWORD Main_Entry(Operation_t Operation);
	CButton* NewCheckBox(int nID,CRect rect,int nStyle);
// Dialog Data
	//{{AFX_DATA(LifeSensorFlashProgrammerDlg)
	enum { IDD = IDD_JN516XFLASHPROGRAMMER_DIALOG };
	CButton	m_cli;
	CEdit	m_mac8;
	CEdit	m_mac7;
	CEdit	m_mac6;
	CEdit	m_mac5;
	CEdit	m_mac4;
	CEdit	m_mac3;
	CEdit	m_mac2;
	CEdit	m_mac1;
	CButton	m_mac_en;
	CMyButton	m_Program;
	CButton	m_Comlist;
	CButton	m_Open;
	CButton	m_Verify;
	CProgressCtrl	m_Progress;
	CComboBox	m_BaudRate;
	CButton	m_Erase;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LifeSensorFlashProgrammerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CButton *p_CheckBox[MAX];
	CString m_FilePath;

	// Generated message map functions
	//{{AFX_MSG(LifeSensorFlashProgrammerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProgram();
	afx_msg void OnClose();
	afx_msg void OnOpen();
	afx_msg void OnComlist();
	afx_msg void OnMacEn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JN516XFLASHPROGRAMMERDLG_H__8649422B_BC30_4A5F_9A3C_6D0F0805BC8A__INCLUDED_)
