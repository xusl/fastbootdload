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
    m_CapsLock = FALSE;
}

PasswordEnterDlg::~PasswordEnterDlg()
{
}

BOOL PasswordEnterDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    //    NextDlgCtrl();
    GotoDlgCtrl(GetDlgItem(IDC_EDIT_PASSWORD));

    if (GetKeyState (VK_CAPITAL) & 1) {
        m_CapsLockPrompt.SetWindowText(_T("Caps Lock is on"));
        m_CapsLock = TRUE;
    }
    return FALSE;
}

BOOL PasswordEnterDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN)
    {
        m_CapsLock = (GetKeyState (VK_CAPITAL) & 1);

        if (m_CapsLock){
            m_CapsLockPrompt.SetWindowText(_T("Caps Lock is on"));
        } else {
            m_CapsLockPrompt.SetWindowText(_T(""));
        }
    }
    return CWnd::PreTranslateMessage(pMsg);
}

void PasswordEnterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_CAPSLOCK_PROMPT, m_CapsLockPrompt);
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
    if(password.Compare(_T("54321")) != 0) {
        ::MessageBox(NULL,_T("The enter password is wrong!"),_T("Error Passowrd"),
            MB_OK | MB_ICONINFORMATION);
        return;
    }
    CDialogEx::OnOK();
}


void PasswordEnterDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}
