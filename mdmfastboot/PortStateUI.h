#pragma once


// CPortStateUI 对话框

class CPortStateUI : public CDialog
{
	DECLARE_DYNAMIC(CPortStateUI)

public:
	CPortStateUI(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPortStateUI();

// 对话框数据
	enum { IDD = IDD_PORT_STATE };

	void SetInfo(CString strInfo);
	void SetTitle(CString strInfo);
	void SetProgress(int iPercent);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
