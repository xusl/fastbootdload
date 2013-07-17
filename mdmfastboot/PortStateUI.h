#pragma once


// CPortStateUI �Ի���

class CPortStateUI : public CDialog
{
	DECLARE_DYNAMIC(CPortStateUI)

public:
	CPortStateUI(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPortStateUI();

// �Ի�������
	enum { IDD = IDD_PORT_STATE };

	void SetInfo(CString strInfo);
	void SetTitle(CString strInfo);
	void SetProgress(int iPercent);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
