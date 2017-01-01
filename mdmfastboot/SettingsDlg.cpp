// SettingsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "SettingsDlg.h"
#include "mdmfastbootDlg.h"


// CSettingsDlg 对话框

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSettingsDlg::IDD, pParent),
	m_pSchedule(NULL),
  m_pFlashdirect(NULL),
  m_pForceupdate(NULL)
{

	EnableAutomation();

}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
  if (m_pSchedule != NULL) {
    DDX_Check(pDX, IDC_SCHE_REMOVE, *m_pSchedule);
  }
  if (m_pFlashdirect != NULL){
    DDX_Check(pDX, IDC_FASTBOOT_ONLY, *m_pFlashdirect);
  }
  if (m_pForceupdate != NULL){
    DDX_Check(pDX, IDC_CHECK_FORCEUPDATE, *m_pForceupdate);
  }
}

BOOL CSettingsDlg::SetScheduleData(int *pData) {
  if (pData == NULL) {
    return FALSE;
  }
  m_pSchedule = pData;
  return TRUE;
}
BOOL CSettingsDlg::SetFlashDirectData(int *pData){
  if (pData == NULL) {
    return FALSE;
  }
  m_pFlashdirect = pData;
  return TRUE;
}
BOOL CSettingsDlg::SetForeUpdateData(int *pData){
  if (pData == NULL) {
    return FALSE;
  }
  m_pForceupdate = pData;
  return TRUE;
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,&CSettingsDlg::OnToolTipNotify)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_FORCEUPDATE, &CSettingsDlg::OnBnClickedCheckForceupdate)
	ON_BN_CLICKED(IDC_FASTBOOT_ONLY, &CSettingsDlg::OnBnClickedFastbootOnly)
	ON_BN_CLICKED(IDC_SCHE_REMOVE, &CSettingsDlg::OnBnClickedScheRemove)
END_MESSAGE_MAP()


 //Notification handler - add entry to class definition
 BOOL CSettingsDlg::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
 {
     TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
     int control_id = ::GetDlgCtrlID((HWND)pNMH->idFrom);
     WORD text_id = 0;
     switch(control_id)
     {
      case IDC_CHECK_FORCEUPDATE:
        text_id = IDS_TIP_FORCEUPDATE;
        break;
      case IDC_FASTBOOT_ONLY:
        text_id = IDS_TIP_FASTBOOT_ONLY;
        break;
      case IDC_SCHE_REMOVE:
        text_id = IDS_TIP_SCHE;
        break;
      default:
         return FALSE;
     }

    //TTM_GETTOOLINFO
    ::SendMessage(pNMH->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
     if (pText->uFlags & TTF_IDISHWND) {
         pText->lpszText = MAKEINTRESOURCE( text_id );
         pText->hinst = AfxGetInstanceHandle();
     }
     return TRUE;
 }

// CSettingsDlg 消息处理程序

BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

  //Why ? this will disable tip
  //CWnd * handle = GetDlgItem(IDC_STATIC_SETTING);
  //if (handle != NULL)
  // handle->SetWindowPos(0, 10, 10, 200 , 120, SWP_SHOWWINDOW);


  //EnableTrackingToolTips();
	EnableToolTips();
	return TRUE;  // return TRUE unless you set the focus to a control
}


void CSettingsDlg::OnBnClickedCheckForceupdate()
{
   CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_FORCEUPDATE);
   if (pBtn != NULL)
   {
	   *m_pForceupdate = pBtn->GetCheck();
//	   WritePrivateProfileString(L"app",L"forceupdate",*m_pForceupdate?L"1":L"0",m_pParent->m_ConfigPath.GetBuffer());
   }
}

void CSettingsDlg::OnBnClickedFastbootOnly()
{
    CButton* pBtn = (CButton*)GetDlgItem(IDC_FASTBOOT_ONLY);
   if (pBtn != NULL)
	 *m_pFlashdirect = pBtn->GetCheck();
}

void CSettingsDlg::OnBnClickedScheRemove()
{
   CButton* pBtn = (CButton*)GetDlgItem(IDC_SCHE_REMOVE);
   if (pBtn != NULL)
    *m_pSchedule = pBtn->GetCheck();
}

void CSettingsDlg::EnableSettings(BOOL enable) {
   CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_FORCEUPDATE);
   if (pBtn != NULL)
    pBtn->EnableWindow(enable);

   pBtn = (CButton*)GetDlgItem(IDC_FASTBOOT_ONLY);
   if (pBtn != NULL)
    pBtn->EnableWindow(enable);

   pBtn = (CButton*)GetDlgItem(IDC_SCHE_REMOVE);
   if (pBtn != NULL)
    pBtn->EnableWindow(enable);
}