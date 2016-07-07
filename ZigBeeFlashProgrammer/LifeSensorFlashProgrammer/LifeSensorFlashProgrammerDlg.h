// LifeSensorFlashProgrammerDlg.h : header file
//

#if !defined(AFX_LIFESENSORFLASHPROGRAMMERDLG_H__8649422B_BC30_4A5F_9A3C_6D0F0805BC8A__INCLUDED_)
#define AFX_LIFESENSORFLASHPROGRAMMERDLG_H__8649422B_BC30_4A5F_9A3C_6D0F0805BC8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "device.h"
#include "MyButton.h"
#include "DownloadPortCtl.h"

/////////////////////////////////////////////////////////////////////////////
// LifeSensorFlashProgrammerDlg dialog
typedef enum ErrorCode
{
	FlashFileNotFound = 1,
	FlashVerificationError,
	FlashFileError,
	COMPortNotRespond,
	EepromEraseError,
	MACprogrammingError
}ErrorCode_t,*pErrorCode_t;

enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_PROGRAMMER,
};

enum
{
  TIMER_EVT_SCHEDULE = 0,
  TIMER_EVT_ALL
};

#define TIMER_ELAPSE   (20 * 1000)

typedef enum
{
  PRG_MSG_DEFAULT = 0,
  PRG_MSG_PROMPT = 1,
  PRG_MSG_ERROR = 2,
  PRG_MSG_RESULT = 3,
  PRG_MSG_CHIPDETAIL = 4
}PRG_MSG_TYPE;

typedef struct _PrgMsg_
{
	PRG_MSG_TYPE	eType;
	int				    iVal;
	CString			  sVal;
  PVOID         pData;

  _PrgMsg_() {
    eType = PRG_MSG_DEFAULT;
    sVal = "";
    iVal = -1;
    pData = NULL;
  }
}PrgMsg;



#define MAX_PORTS_ONCE 6

class LifeSensorFlashProgrammerDlg : public CDialog
{
// Construction
public:
	LifeSensorFlashProgrammerDlg(CWnd* pParent = NULL);	// standard constructor
  ~LifeSensorFlashProgrammerDlg();
  BOOL HandleComDevice(VOID);
  DWORD ScheduleProgrammer();
  void SetMacByte(CEdit &edit, unsigned char * pValue);
  BOOL CreateDownloadPortDialogs();
  void UpdateTimer();
// Dialog Data
	//{{AFX_DATA(LifeSensorFlashProgrammerDlg)
	enum { IDD = IDD_FLASHPROGRAMMER_DIALOG };

	CEdit	m_mac8;
	CEdit	m_mac7;
	CEdit	m_mac6;
	CEdit	m_mac5;
	CEdit	m_mac4;
	CEdit	m_mac3;
	CEdit	m_mac2;
	CEdit	m_mac1;
	CButton	m_mac_en;
	CButton	m_Comlist;
	CButton	m_Open;
	CButton	m_Verify;
	//CProgressCtrl	m_Progress;
	CComboBox	m_BaudRate;
	CButton	m_Erase;
  CButton	m_StartProgram;
  CStatic  m_ProgrammerCount;

	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LifeSensorFlashProgrammerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
  DownloadPortCtl *m_DownloadPort[MAX_PORTS_ONCE];
	CString m_FilePath;
  DeviceCoordinator mDevCoordinator;
	// Generated message map functions
	//{{AFX_MSG(LifeSensorFlashProgrammerDlg)
	virtual BOOL OnInitDialog();
  LRESULT OnProgrammerMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProgram();
	afx_msg void OnClose();
	afx_msg void OnOpen();
	afx_msg void OnComlist();
	afx_msg void OnMacEn();
  afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LifeSensorFLASHPROGRAMMERDLG_H__8649422B_BC30_4A5F_9A3C_6D0F0805BC8A__INCLUDED_)
