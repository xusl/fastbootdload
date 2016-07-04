#pragma once

#include "MyButton.h"
#include "ProgramThread.h"
// DownloadPortCtl 对话框

#define COLOR_RED 0x0000FF
#define COLOR_BLUE 0xFF0000

class DownloadPortCtl : public CDialog
{
	DECLARE_DYNAMIC(DownloadPortCtl)

public:
	DownloadPortCtl(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DownloadPortCtl();

// 对话框数据
	enum { IDD = IDD_DOWNLOADPORT };

	void SetProgress(int iPercent);
	void UpdateStatus(CString strInfo, COLORREF color=COLOR_BLUE);
	void Init(const char * port);
  BOOL IsInit();
  BOOL AttachDevice(const char *devicename, BOOL fixmap=TRUE);
  BOOL StartDownload();
  BOOL FinishDownload();
  void SetTitle(CString strInfo);
  void Reset(void);
  tsProgramThreadArgs * GetProgramArgs() { return m_progamArgs;};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
  BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
  BOOL SetProgramArgs(tsProgramThreadArgs * args);

public:
	CString mID;
  BOOL mIsDownload;
	CMyButton	m_Program;
  tsProgramThreadArgs *m_progamArgs;
  //CBrush m_Brush;
};
