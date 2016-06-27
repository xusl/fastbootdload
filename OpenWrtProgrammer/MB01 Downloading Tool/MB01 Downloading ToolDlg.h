
// MB01 Downloading ToolDlg.h : header file
//

#pragma once
#include "Wifi_Connect.h"
#include "GetIp.h"
#include "serialport.h"
#include "TextProgressCtrl.h"
#include "afxcmn.h"

// CMB01DownloadingToolDlg dialog
class CMB01DownloadingToolDlg : public CDialogEx
{
// Construction
public:
	CMB01DownloadingToolDlg(CWnd* pParent = NULL);	// standard constructor
	~CMB01DownloadingToolDlg(void);

// Dialog Data
	enum { IDD = IDD_MB01DOWNLOADINGTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	

	static DWORD WINAPI Thread_Check_Wifi(LPVOID lpPARAM);
	static DWORD WINAPI Thread_Server_Listen(LPVOID lpPARAM);
	static  DWORD WINAPI Thread_Send_Comand(LPVOID lpPARAM);


// Implementation
protected:
	struct mg_server *server;
	HICON m_hIcon;
	CString strFile ; 
	void OnSend_Comand();
	void OnSend_Resset_Comand();
	//CMacProgressCtrl	m_progMac2;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	void server_listen();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg  void OnClose();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	HANDLE WIFI_NAME_Thread;
	HANDLE Server_Listen_Thread;
	HANDLE Send_Comand_Thread;
	DWORD   WIFI_NAME_Thread_ID; 
	DWORD   Server_Listen_Thread_ID; 
	DWORD   Send_Comand_Thread_ID; 
public:
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
private:
	char file_name[260];
	CString error_message;
	int Progress_range;
	bool server_state;
	void Check_Wifi();
	bool is_downloading;
	bool downloading_successfull;
	CEdit* Line_edit;
	char s_CommercialRef[14];
	char s_PTS_new[4];
	char s_PCBNo[16];
	char s_MMIFlag[2];
	char s_WIFI[13];
	char s_SSID_Prefix[21];
	DWORD dwBeginTime;
	Wifi_Connect wifi_connecting;
	void get_Wifi_connect();
	WCHAR Original_Wifi_name [256];
	char s_Trace[50];
	bool exitSocket;
	
public:
	CTextProgressCtrl m_progMac2;
};
