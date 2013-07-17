// PortStateUI.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "PortStateUI.h"


// CPortStateUI �Ի���

IMPLEMENT_DYNAMIC(CPortStateUI, CDialog)

CPortStateUI::CPortStateUI(CWnd* pParent /*=NULL*/)
	: CDialog(CPortStateUI::IDD, pParent)
{

}

CPortStateUI::~CPortStateUI()
{
}

void CPortStateUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CPortStateUI::SetInfo(CString strInfo)
{
	GetDlgItem(IDC_DL_INFO)->SetWindowText(strInfo.GetBuffer());	
}

void CPortStateUI::SetTitle(CString strInfo)
{
	GetDlgItem(IDC_GROUP)->SetWindowText(strInfo.GetBuffer());	
}

void CPortStateUI::SetProgress(int iPercent)
{
	::SendMessage(GetDlgItem(IDC_PROGRESS1)->m_hWnd, PBM_SETPOS, iPercent, 0);
}

BEGIN_MESSAGE_MAP(CPortStateUI, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CPortStateUI ��Ϣ�������

void CPortStateUI::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if(NULL == GetDlgItem(IDC_GROUP))
		return;
	RECT rect;
	int space = 20;
	GetDlgItem(IDC_GROUP)->SetWindowPos(0, 0, 0, cx, cy, 0);
	GetDlgItem(IDC_STATIC)->GetClientRect(&rect);
	GetDlgItem(IDC_STATIC)->SetWindowPos(0, space, cy-8*rect.bottom, cx-2*space, rect.bottom, 0);
	GetDlgItem(IDC_PROGRESS1)->GetClientRect(&rect);
	GetDlgItem(IDC_PROGRESS1)->SetWindowPos(0, space, cy-3*rect.bottom, cx-2*space, rect.bottom, 0);
	GetDlgItem(IDC_DL_INFO)->SetWindowPos(0, space, cy-1*rect.bottom, cx-2*space, rect.bottom, 0);
}
