// GetProfileDlg.h : ͷ�ļ�
//

#pragma once
#include "adb_dev_register.h"

// CGetProfileDlg �Ի���
class CGetProfileDlg : public CDialog
{
// ����
public:
	CGetProfileDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GETPROFILE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
  VOID DoGetProfile(VOID);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void CGetProfileDlg::OnDestroy();
	DECLARE_MESSAGE_MAP()
};
