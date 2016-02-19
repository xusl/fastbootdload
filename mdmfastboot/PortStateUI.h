#pragma once


typedef enum
{
	FIRMWARE_VER,
	QCN_VER,
	LINUX_VER,
	SYSTEM_VER,
	USERDATA_VER,
	PTS_VER,
	TITLE,
	PROGRESS_VAL,
	PROMPT_TITLE,
	PROMPT_TEXT,
	ADB_CHK_ABORT,
	REBOOT_DEVICE,
	FLASH_DONE,
	UI_DEFAULT,
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
	void Init(int iPortID);
  void SetTitle(CString strInfo);
  void Reset(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
  BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:
	int iID;
	UIInfo m_PortFrmVer;
	UIInfo m_PortQCNVer;
	UIInfo m_PortLinuxVer;
	UIInfo m_PortProgressInfo;
	UIInfo m_PortProgressValue;
  //CBrush m_Brush;
};
