
// DownloadDlg.h : header file
//

#pragma once

#include "GetIp.h"
#include "device.h"
#include "TextProgressCtrl.h"


#define CONFIG_FILE_PATH         _T("Config.ini")
#define WS_LABEL_DIR             TEXT(".\\Label\\")
//TCP port 23 is reserved for Telnet client and server communication.
#define TELNET_PORT      IPPORT_TELNET
#define DOWNLOAD_SERVER_PORT     80
#define DOWNLOAD_SERVER_IP       "172.19.42.1"

enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_WORKTHREADS,
};

enum
{
  TIMER_EVT_SCHEDULE = 0,
  TIMER_EVT_ALL
};

#define TIMER_ELAPSE   (3 * 1000)


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
  static DWORD WINAPI NetworkSniffer(LPVOID lpPARAM);

// Implementation
protected:
	HICON m_hIcon;
	struct mg_server *server;
  string mHostIPAddr;
	HANDLE m_NetworkSnifferThreadHandle;
	HANDLE Server_Listen_Thread;
	HANDLE Send_Comand_Thread;
	DWORD   Server_Listen_Thread_ID;
	DWORD   Send_Comand_Thread_ID;
  DWORD   m_NetworkSnifferThreadID;
	CString mRomPath;
  CString mModulePath;
  BOOL  mWSAInitialized;
  SOCKET CreateSocket(const char *ip_addr,  u_short port = TELNET_PORT);
	void OnSend_Comand(SOCKET sockClient, const char * cmd);
	void server_listen(u_short port =DOWNLOAD_SERVER_PORT);
  void SniffNetwork();
  void GetHostIpAddr();
  void UpdateMessage(CString msg);
  void ClearMessage(void);
  void HandleDownloadException(CString msg, SOCKET &sock);
  void HandleServerException(CString msg, SOCKET sockConn, SOCKET sockSrv, const char ** ppContent);
  char const* BuildHttpServerResponse(const char *path, size_t  *contentLength);
  BOOL BuildUpdateCommand(CString file, CString &cmd);
  DWORD Schedule();
  DeviceCoordinator * GetDeviceCoodinator() { return m_pCoordinator;};
  void ReleaseThreadSyncSemaphore();

	// Generated message map functions
	virtual BOOL OnInitDialog();
  LRESULT OnMessageArrive(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	DECLARE_MESSAGE_MAP()
private:
	CString error_message;
	int Progress_range;
	bool server_state;
	bool is_downloading;
	bool downloading_successfull;
	bool b_download;
	CEdit* Line_edit;
  CEdit m_CUEdit;
  CEdit m_ORDEREdit;
  CStatic m_RomPathStaticText;	
	char s_PTS_new[4];
	//char s_PCBNo[16];
	char s_MMIFlag[2];
  char m_NetworkSegment[IPADDR_BUFFER_LEN];
	DWORD dwBeginTime;
	char s_Trace[50];
	CTextProgressCtrl m_progMac2;
  DeviceCoordinator *m_pCoordinator;
  HANDLE m_SyncSemaphore;
};
