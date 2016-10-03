// PasswordEnterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "resource.h"
#include "PasswordEnterDlg.h"
// PasswordEnterDlg dialog

IMPLEMENT_DYNAMIC(PasswordEnterDlg, CDialogEx)

PasswordEnterDlg::PasswordEnterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(PasswordEnterDlg::IDD, pParent)
{

}

PasswordEnterDlg::~PasswordEnterDlg()
{
}

void PasswordEnterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PasswordEnterDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &PasswordEnterDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &PasswordEnterDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// PasswordEnterDlg message handlers


void PasswordEnterDlg::OnBnClickedOk()
{
    CEdit *passwordEdit=(CEdit*)GetDlgItem(IDC_EDIT_PASSWORD);
    CString password;
    passwordEdit->GetWindowText(password);
    if(password.Compare(_T("1plus1>2")) != 0) {
        ::MessageBox(NULL,_T("The enter password is wrong!"),_T("Error Passowrd"),MB_OK);
        return;
    }
    CDialogEx::OnOK();
}


void PasswordEnterDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}
