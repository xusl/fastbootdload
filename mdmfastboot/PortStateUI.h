#pragma once

typedef enum
{
	PORT_UI_ID_INVALID = 0,
	PORT_UI_ID_FIRST,
	PORT_UI_ID_SECOND,
	PORT_UI_ID_THIRD,
	PORT_UI_ID_FOURTH,
}PORT_ID;

typedef enum
{
	FIRMWARE_VER,
	QCN_VER,
	LINUX_VER,
	PROGRESS_VAL,
	PROGRESS_STR,
}UI_INFO_TYPE;

struct UIInfo
{
	UI_INFO_TYPE	infoType;
	int				iVal;
	CString			sVal;
};

// CPortStateUI 对话框

class CPortStateUI : public CDialog
{
	DECLARE_DYNAMIC(CPortStateUI)

public:
	CPortStateUI(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPortStateUI();

// 对话框数据
	enum { IDD = IDD_PORT_STATE };

	void SetProgress(int iPercent);
	void SetInfo(UI_INFO_TYPE infoType, CString strInfo);
	void Init(PORT_ID iPortID);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	void SetTitle(CString strInfo);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

public:
	PORT_ID iID;
	UIInfo m_PortFrmVer;
	UIInfo m_PortQCNVer;
	UIInfo m_PortLinuxVer;
	UIInfo m_PortProgressInfo;
	UIInfo m_PortProgressValue;
};
