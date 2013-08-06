// mdmfastbootDlg.h : ͷ�ļ�
//

#pragma once

#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include <initguid.h>
#include <setupapi.h>
#include "PortStateUI.h"
#include "usb_adb.h"

enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_UPDATE_PROGRESS_INFO,
	UI_MESSAGE_UPDATE_PACKAGE_INFO,
	UI_MESSAGE_DEVICE_INFO,
};

class CmdmfastbootDlg;

typedef struct {
    CWnd* hWnd;
    CPortStateUI  ctl;
    usb_handle * usb;
    int usb_sn;
} UsbWorkData;


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
  BOOL m_bInit;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	DECLARE_MESSAGE_MAP()

	void OnHelp();
	void OnAbout();
public:
	afx_msg void OnBnClickedButtonStop();

  UsbWorkData data[4];

	//port UI

	CString m_strFrmVer;
	CString m_strQCNVer;
	CString m_strLinuxVer;
	CString m_strPackagePath;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL RegisterAdbDeviceNotification(void);
  BOOL AdbUsbHandler(BOOL update_device);
  BOOL SetPortDialogs(UINT nType, int x, int y, int w, int h);
  LRESULT OnDeviceInfo(WPARAM wParam, LPARAM lParam);
	void UpdateDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnDestroy();

  private:
    UsbWorkData * GetUsbWorkData(usb_handle* handle);
    BOOL InitUsbWorkData(void);
    BOOL CleanUsbWorkData(UsbWorkData *data);
    BOOL SwitchUsbWorkData(UsbWorkData *data);
};
