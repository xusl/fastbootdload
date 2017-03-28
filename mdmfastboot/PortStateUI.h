#pragma once

#include "PST.h"

#define PORTSTATE_WIDTH   240
#define PORTSTATE_HEIGHT  130

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
  BOOL AddDevInfo(CString name, CString value);
	void Init(int iPortID);
  void SetTitle(CString strInfo);
  void Reset(void);
  void SetProgressVisible(BOOL show);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
  BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:
	int iID;
  CListCtrl m_DevInfoList;
	UIInfo m_PortFrmVer;
	UIInfo m_PortQCNVer;
	UIInfo m_PortLinuxVer;
	UIInfo m_PortProgressInfo;
	UIInfo m_PortProgressValue;
  //CBrush m_Brush;
};
