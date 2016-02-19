// PortStateUI.cpp : 实现文件
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "PortStateUI.h"


// CPortStateUI 对话框

IMPLEMENT_DYNAMIC(CPortStateUI, CDialog)

CPortStateUI::CPortStateUI(CWnd* pParent /*=NULL*/)
	: CDialog(CPortStateUI::IDD, pParent)
{
	iID = -1;
}

CPortStateUI::~CPortStateUI()
{
}

void CPortStateUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CPortStateUI::OnInitDialog()
{
  CDialog::OnInitDialog();
  //m_Brush.CreateSolidBrush(RGB(0,0,0));
  //GetDlgItem(IDC_DL_INFO)->SetWindowText(_T("Yy......"));

  return TRUE;
}

void CPortStateUI::SetInfo(UI_INFO_TYPE infoType, CString strInfo)
{
	switch(infoType)
	{
    case PROMPT_TEXT:
		GetDlgItem(IDC_DL_INFO)->SetWindowText(strInfo.GetBuffer());
		break;
    case PROMPT_TITLE:
		GetDlgItem(IDC_DLINFO_TITLE)->SetWindowText(strInfo.GetBuffer());
		break;
	case FIRMWARE_VER:
		GetDlgItem(IDC_EDIT_FRM_VER)->SetWindowText(strInfo.GetBuffer());
		break;
	case QCN_VER:
		GetDlgItem(IDC_EDIT_QCN_VER)->SetWindowText(strInfo.GetBuffer());
		break;
	case LINUX_VER:
		GetDlgItem(IDC_EDIT_LINUX_VER)->SetWindowText(strInfo.GetBuffer());
        break;
	case SYSTEM_VER:
		GetDlgItem(IDC_EDIT_SYSTEM_VER)->SetWindowText(strInfo.GetBuffer());
        break;
	case USERDATA_VER:
		GetDlgItem(IDC_EDIT_USERDATA_VER)->SetWindowText(strInfo.GetBuffer());
		break;
    default:
        return;
	}
    strInfo.ReleaseBuffer();
}

void CPortStateUI::SetTitle(CString strInfo)
{
	GetDlgItem(IDC_GROUP)->SetWindowText(strInfo.GetBuffer());
    strInfo.ReleaseBuffer();
}

void CPortStateUI::SetProgress(int iPercent)
{
    ((CProgressCtrl *)GetDlgItem(IDC_PROGRESS1))->SetPos(iPercent);
	//::SendMessage(GetDlgItem(IDC_PROGRESS1)->m_hWnd, PBM_SETPOS, iPercent, 0);
    //GetDlgItem(IDC_PROGRESS1)->SendMessage(PBM_SETPOS, iPercent, 0);
    //Invalidate();
}

void CPortStateUI::Init(int iPortID)
{
	iID = iPortID;
	CString strTitle;
	strTitle.Format(_T("%s %d"), _T("Port"), iPortID);
	SetTitle(strTitle);
	ShowWindow(1);
}

BEGIN_MESSAGE_MAP(CPortStateUI, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPortStateUI 消息处理程序

void CPortStateUI::OnSize(UINT nType, int cx, int cy)
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


afx_msg HBRUSH CPortStateUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    if(nCtlColor == CTLCOLOR_STATIC){
      if(pWnd->GetDlgCtrlID()== IDC_DL_INFO) {
           pDC->SetTextColor(RGB(255,0,0)); //文字颜色
           //pDC->SetBkColor(RGB(251, 247, 200));
           pDC->SetBkMode(TRANSPARENT);//透明
           //return (HBRUSH)::GetStockObject(NULL_BRUSH);
           //return (HBRUSH) m_Brush.GetSafeHandle();
       }
      }

    return brush;
}

void CPortStateUI::Reset(void)
{
    LPCTSTR lpszString = L"";

	CString strTitle;
	strTitle.Format(_T("%s %d"), _T("Port"), iID);
	SetTitle(strTitle);
	GetDlgItem(IDC_DL_INFO)->SetWindowText(lpszString);
	GetDlgItem(IDC_DLINFO_TITLE)->SetWindowText(lpszString);
	GetDlgItem(IDC_EDIT_FRM_VER)->SetWindowText(lpszString);
	GetDlgItem(IDC_EDIT_QCN_VER)->SetWindowText(lpszString);
	GetDlgItem(IDC_EDIT_LINUX_VER)->SetWindowText(lpszString);
	GetDlgItem(IDC_EDIT_SYSTEM_VER)->SetWindowText(lpszString);
	GetDlgItem(IDC_EDIT_USERDATA_VER)->SetWindowText(lpszString);
     ((CProgressCtrl *)GetDlgItem(IDC_PROGRESS1))->SetPos(0);
}

