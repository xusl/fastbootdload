// PortStateUI.cpp : 实现文件
//

#include "stdafx.h"
#include "assert.h"
#include "resource.h"
#include "DownloadPortCtl.h"

// DownloadPortCtl 对话框

IMPLEMENT_DYNAMIC(DownloadPortCtl, CDialog)

DownloadPortCtl::DownloadPortCtl(CWnd* pParent /*=NULL*/)
	: CDialog(DownloadPortCtl::IDD, pParent),
    mIsDownload(FALSE),
    m_progamArgs(NULL)
{
	mID.Empty();
    m_progamArgs = (tsProgramThreadArgs *)malloc(sizeof(tsProgramThreadArgs));
}

DownloadPortCtl::~DownloadPortCtl()
{
    mID.Empty();
    if(m_progamArgs) {
        free(m_progamArgs);
        m_progamArgs = NULL;
    }
}

void DownloadPortCtl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRAM, m_Program);
}

BOOL DownloadPortCtl::OnInitDialog()
{
  CDialog::OnInitDialog();
  //m_Brush.CreateSolidBrush(RGB(0,0,0));
  //GetDlgItem(IDC_DL_INFO)->SetWindowText(_T("Yy......"));
	m_Program.SetForeColor(COLOR_BLUE);
	m_Program.SetTextFont(120,"Arial");
	m_Program.SetTextFont(120,"Verdana");
  return TRUE;
}

void DownloadPortCtl::UpdateStatus(CString strInfo, COLORREF color)
{
	 m_Program.SetForeColor(color);
	 m_Program.SetText(strInfo);
}

void DownloadPortCtl::SetTitle(CString strInfo)
{
	GetDlgItem(IDC_DOWNLOADPORT_GROUP)->SetWindowText(strInfo.GetBuffer());
    strInfo.ReleaseBuffer();
}

void DownloadPortCtl::SetProgress(int iPercent)
{
    //((CProgressCtrl *)GetDlgItem(IDC_PROGRESS1))->SetPos(iPercent);
	//::SendMessage(GetDlgItem(IDC_PROGRESS1)->m_hWnd, PBM_SETPOS, iPercent, 0);
    //GetDlgItem(IDC_PROGRESS1)->SendMessage(PBM_SETPOS, iPercent, 0);
    //Invalidate();
}

void DownloadPortCtl::Init(const char * port)
{
	mID = port;
	//CString strTitle;
	//strTitle.Format(_T("%s %d"), _T("Port"), iPortID);
	ASSERT(port != NULL);
	SetTitle(port);
	ShowWindow(1);
}

BOOL DownloadPortCtl::AttachDevice(const char *devicename, BOOL fixmap) {
    if(devicename == NULL || mIsDownload)
        return FALSE;

    if(mID.IsEmpty() || fixmap == FALSE) {
        Init(devicename);
        return TRUE;
    }

    return mID == devicename;
}

BOOL DownloadPortCtl::IsInit() {
    return !mID.IsEmpty();
}

BEGIN_MESSAGE_MAP(DownloadPortCtl, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// DownloadPortCtl 消息处理程序

void DownloadPortCtl::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

#if 0
	if(NULL == GetDlgItem(IDC_GROUP))
		return;
	RECT rect;
	int space = 20;
	GetDlgItem(IDC_GROUP)->SetWindowPos(0, 0, 0, cx, cy, 0);
	GetDlgItem(IDC_STATIC)->GetClientRect(&rect);
	GetDlgItem(IDC_STATIC)->SetWindowPos(0, space, cy-8*rect.bottom, cx-2*space, rect.bottom, 0);
	GetDlgItem(IDC_PROGRESS1)->GetClientRect(&rect);
	GetDlgItem(IDC_PROGRESS1)->SetWindowPos(0, space, cy-3*rect.bottom, cx-2*space, rect.bottom, 0);
	GetDlgItem(IDC_DL_INFO)->SetWindowPos(0, space, cy-1*rect.bottom, cx-2*space, rect.bottom-10, 0);
#endif
}


afx_msg HBRUSH DownloadPortCtl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    if(nCtlColor == CTLCOLOR_STATIC){
      if(pWnd->GetDlgCtrlID()== IDC_DOWNLOADPORT_GROUP) {
           pDC->SetTextColor(RGB(255,0,0)); //文字颜色
           //pDC->SetBkColor(RGB(251, 247, 200));
           pDC->SetBkMode(TRANSPARENT);//透明
           //return (HBRUSH)::GetStockObject(NULL_BRUSH);
           //return (HBRUSH) m_Brush.GetSafeHandle();
       }
      }

    return brush;
}

void DownloadPortCtl::Reset(void)
{
    LPCTSTR lpszString = "";

//	CString strTitle;
//	strTitle.Format(_T("%s %d"), _T("Port"), iID);
    mID.Empty();
	SetTitle(mID);
    UpdateStatus(lpszString);
    mIsDownload = FALSE;
    m_progamArgs = NULL;
	//GetDlgItem(IDC_DL_INFO)->SetWindowText(lpszString);
	//GetDlgItem(IDC_DLINFO_TITLE)->SetWindowText(lpszString);
    //((CProgressCtrl *)GetDlgItem(IDC_PROGRESS1))->SetPos(0);
}

BOOL DownloadPortCtl::SetProgramArgs(tsProgramThreadArgs * args) {
    if (args == NULL)
        return FALSE;
    m_progamArgs = args;
    return TRUE;
}

BOOL DownloadPortCtl::StartDownload() {
    mIsDownload = TRUE;
    return TRUE;
 }

BOOL DownloadPortCtl::FinishDownload() {
    mIsDownload = FALSE;
    return TRUE;
}