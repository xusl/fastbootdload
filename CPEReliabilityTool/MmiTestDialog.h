
// MmiTestDialog.h : header file
//

#pragma once

#include "telnet.h"
#include "NicManager.h"
// MmiTestDialog dialog
class MmiTestDialog : public CDialogEx
{
// Construction
public:
	MmiTestDialog(CWnd* pParent = NULL);	// standard constructor
	virtual ~MmiTestDialog();

// Dialog Data
	enum { IDD = IDD_MMITEST };

private:
  BOOL    m_Work;
  CListCtrl m_MmiItemList;
  CString m_MmiStatus;
  CString m_MmiDevInfo;
  NicManager mNic;
  CButton m_ExitButton;
  CButton m_StartButton;

  HANDLE          mDevSwitchEvt;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
    static UINT RunMmiTest(LPVOID wParam);
    BOOL TestWiFi(TelnetClient &client, CString item, PCCH command);
    BOOL TestItem(TelnetClient &client, CString item, PCCH command, string &data);
    BOOL TestItem(TelnetClient &client, CString item, PCCH command, const string &ok, const string &error);
    BOOL TestKey(TelnetClient &client, CString item,  const string &ok, int key, int elapse);
    BOOL TestLed(TelnetClient &client, CString item,  int value);
    BOOL DiagTest();
    BOOL AddTestResult(CString item, BOOL pass, string description);
    SOCKET GetTelnetSocket();
    BOOL SetWork(BOOL work);
    BOOL SetStatus(CString text);
    VOID WaitForDevice(long seconds);
    LRESULT OnDeviceInfo(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedExit();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
