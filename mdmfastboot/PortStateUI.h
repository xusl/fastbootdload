#pragma once


typedef enum
{
	FIRMWARE_VER,
	QCN_VER,
	LINUX_VER,
	SYSTEM_VER,
	USERDATA_VER,
	TITLE,
	PROGRESS_VAL,
	PROGRESS_STR,
	PROMPT_TITLE,
	PROMPT_TEXT,
	REBOOT_DEVICE,
}UI_INFO_TYPE;

struct UIInfo
{
	UI_INFO_TYPE	infoType;
	int				iVal;
	CString			sVal;
};

// CPortStateUI �Ի���

class CPortStateUI : public CDialog
{
	DECLARE_DYNAMIC(CPortStateUI)

public:
	CPortStateUI(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPortStateUI();

// �Ի�������
	enum { IDD = IDD_PORT_STATE };

	void SetProgress(int iPercent);
	void SetInfo(UI_INFO_TYPE infoType, CString strInfo);
	void Init(int iPortID);
  void SetTitle(CString strInfo);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

public:
	int iID;
	UIInfo m_PortFrmVer;
	UIInfo m_PortQCNVer;
	UIInfo m_PortLinuxVer;
	UIInfo m_PortProgressInfo;
	UIInfo m_PortProgressValue;
};
