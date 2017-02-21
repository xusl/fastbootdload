#pragma once

#include "telnet.h"
#include "NicManager.h"

// CProgramDialog dialog

class CProgramDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CProgramDialog)

public:
	CProgramDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgramDialog();

// Dialog Data
	enum { IDD = IDD_PROGRAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	virtual BOOL OnInitDialog();
	//afx_msg void OnPaint();
	//afx_msg HCURSOR OnQueryDragIcon();
  
	DECLARE_MESSAGE_MAP()

private:
  CEdit m_EditIMEI;
  NicManager mNic;
  HANDLE  mDevSwitchEvt;
  
public:
    afx_msg void OnBnClickedButtonWriteimei();
    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

    SOCKET CProgramDialog::GetTelnetSocket();
    BOOL CProgramDialog::DiagWork() ;
    VOID CProgramDialog::WaitForDevice(long seconds);
    static UINT WriteIMEI(LPVOID wParam);
};
