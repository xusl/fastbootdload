
// DownloadDlg.h : header file
//

#pragma once

#include "GetIp.h"
#include "TextProgressCtrl.h"

// CDownloadDlg dialog
class CDownloadDlg : public CDialogEx
{
// Construction
public:
	CDownloadDlg(CWnd* pParent = NULL);	// standard constructor
	~CDownloadDlg();

// Dialog Data
	enum { IDD = IDD_LIFECONNECTFIRMWAREDOWNLOAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	static DWORD WINAPI Thread_Server_Listen(LPVOID lpPARAM);
	static DWORD WINAPI Thread_Send_Comand(LPVOID lpPARAM);

// Implementation
protected:
	HICON m_hIcon;
	struct mg_server *server;
	HANDLE Server_Listen_Thread;
	HANDLE Send_Comand_Thread;
	DWORD   Server_Listen_Thread_ID;
	DWORD   Send_Comand_Thread_ID;
	CString mRomPath;
  CString mModulePath;
	void OnSend_Comand();
	void OnSend_Resset_Comand();
	void server_listen();
  void UpdateMessage(CString msg);
  void ClearMessage(void);
  void HandleDownloadException(CString msg, SOCKET &sock, bool cleanWSA=TRUE);
  void HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent);
  char const* BuildHttpServerResponse(const char *path, size_t  *contentLength);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	DECLARE_MESSAGE_MAP()
private:
	CString error_message;
	int Progress_range;
	bool server_state;
	bool is_downloading;
	bool downloading_successfull;
	CEdit* Line_edit;
  CEdit m_CUEdit;
	char s_CommercialRef[14];
	char s_PTS_new[4];
	char s_PCBNo[16];
	char s_MMIFlag[2];
	DWORD dwBeginTime;
	char s_Trace[50];
	bool exitSocket;
	CTextProgressCtrl m_progMac2;
};
