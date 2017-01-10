#pragma once

#include "PST.h"
#include "NicManager.h"
// MmiTestDialog dialog

class MmiTestDialog : public CDialogEx
{
	DECLARE_DYNAMIC(MmiTestDialog)

public:
	MmiTestDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~MmiTestDialog();

	virtual void OnFinalRelease();

// Dialog Data
	enum { IDD = IDD_MMITEST };

private:
  CListCtrl m_MmiItemList;
  CString m_MmiStatus;
  CString m_MmiDevInfo;
  NicManager mNic;
  PSTManager mDeviceManager;
  CButton m_ExitButton;
  CButton m_StartButton;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
    static UINT RunMmiTest(LPVOID wParam);
    LRESULT OnDeviceInfo(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedExit();
};
