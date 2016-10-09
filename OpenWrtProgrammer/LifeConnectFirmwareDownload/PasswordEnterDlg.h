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
    CStatic m_CapsLockPrompt;
    BOOL m_CapsLock;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
};
