// mdmfastbootDlg.h : ͷ�ļ�
//

#pragma once

#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include <initguid.h>
#include <setupapi.h>
#include "PortStateUI.h"

enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_UPDATE_PROGRESS_INFO,
	UI_MESSAGE_UPDATE_PACKAGE_INFO,
};

class CmdmfastbootDlg;
struct TranseInfo
{
	CmdmfastbootDlg*	dlgMain;
	CPortStateUI*		portUI;
};

struct UIInfo
{
	UI_INFO_TYPE	infoType;
	int				iVal;
	CString			sVal;
};

// CmdmfastbootDlg �Ի���
class CmdmfastbootDlg : public CDialog
{
// ����
public:
	CmdmfastbootDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MDMFASTBOOT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


	LRESULT OnUpdateProgressInfo(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdatePackageInfo(WPARAM wParam, LPARAM lParam);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	DECLARE_MESSAGE_MAP()

	void UpdatePortUI(CPortStateUI& portUI, UIInfo* uiInfo);
public:
	afx_msg void OnBnClickedButtonStop();

	//port UI
	CPortStateUI PortStateUI1;
	CPortStateUI PortStateUI2;
	CPortStateUI PortStateUI3;
	CPortStateUI PortStateUI4;
	TranseInfo TranseInfo1;
	TranseInfo TranseInfo2;
	TranseInfo TranseInfo3;
	TranseInfo TranseInfo4;
	CString m_strFrmVer;
	CString m_strQCNVer;
	CString m_strLinuxVer;
	CString m_strPackagePath;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL RegisterAdbDeviceNotification(void);
	void UpdateDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
	afx_msg void OnBnClickedBtnBrowse();
};
