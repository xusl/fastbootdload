#pragma once


// NicListDlg dialog

class NicListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(NicListDlg)

public:
	NicListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~NicListDlg();
  BOOL SetNicManager(NicManager &manager);

// Dialog Data
	enum { IDD = IDD_DIALOG_NIC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
  CListCtrl m_NicList;
  NicManager *m_pNicManager;
  virtual void OnOK();
public:
    afx_msg void OnNMDblclkListNic(NMHDR *pNMHDR, LRESULT *pResult);
};
