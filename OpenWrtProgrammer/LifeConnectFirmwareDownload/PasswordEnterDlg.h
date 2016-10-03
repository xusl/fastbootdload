#pragma once


// PasswordEnterDlg dialog

class PasswordEnterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PasswordEnterDlg)

public:
	PasswordEnterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PasswordEnterDlg();

// Dialog Data
	enum { IDD = IDD_ENTER_PASSWORD };

private:
    CString m_passwordText;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
};
