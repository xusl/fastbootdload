#pragma once

enum {
SETTING_FORCEUPDATE = 0,
SETTING_REMOVE,
};
// CSettingsDlg �Ի���

class CSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSettingsDlg();

  BOOL SetScheduleData(int *pData);
  BOOL SetFlashDirectData(int *pData);
  BOOL SetForeUpdateData(int *pData);

// �Ի�������
	enum { IDD = IDD_SETTINGS };

private:
  int *m_pSchedule;
  int *m_pFlashdirect;
  int *m_pForceupdate;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
  BOOL OnToolTipNotify(UINT id, NMHDR *pNMH,  LRESULT *pResult);

  afx_msg void OnBnClickedCheckForceupdate();
  afx_msg void OnBnClickedFastbootOnly();
  afx_msg void OnBnClickedScheRemove();
	};
