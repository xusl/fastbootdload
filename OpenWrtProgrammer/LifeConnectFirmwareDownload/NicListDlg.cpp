// NicListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "NicManager.h"
#include "NicListDlg.h"
#include "afxdialogex.h"

using namespace std;

enum E_NICFields {
    NICFD_DEVICENAME,
    NICFD_CONNECTION,
    NICFD_PHYSADDRESS,
};
// NicListDlg dialog

IMPLEMENT_DYNAMIC(NicListDlg, CDialogEx)

NicListDlg::NicListDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(NicListDlg::IDD, pParent)
{

}

NicListDlg::~NicListDlg()
{
}

void NicListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LIST_NIC, m_NicList);
}


BEGIN_MESSAGE_MAP(NicListDlg, CDialogEx)
END_MESSAGE_MAP()


// NicListDlg message handlers


BOOL NicListDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();


    if (m_pNicManager == NULL)
        return TRUE;

    m_NicList.InsertColumn(NICFD_CONNECTION,  _T("Ethernet adapter"), LVCFMT_LEFT, 180);
    m_NicList.InsertColumn(NICFD_DEVICENAME,  _T("Description"), LVCFMT_LEFT, 130);
    m_NicList.InsertColumn(NICFD_PHYSADDRESS,  _T("Physical Address"), LVCFMT_LEFT, 80);
    //m_NicList.InsertColumn(FD_PROGRESS,  _T("Progress"), LVCFMT_LEFT, 60);
    //m_NicList.InsertColumn(FD_BYTES,    _T("Bytes"), LVCFMT_LEFT, 100);
    //m_NicList.InsertColumn(FD_TOTAL,    _T("Total"), LVCFMT_LEFT, 100);
    //m_NicList.InsertColumn(FD_TIMEOUT,  _T("Timeout"), LVCFMT_LEFT, 80);


    LVITEM      LvItem;
    int         itemPos = 0;

    list<NetCardStruct>* nic = (list<NetCardStruct>*)m_pNicManager->GetNicList();
     list<NetCardStruct>::iterator it;
    BOOL result = TRUE;
    for (it = nic->begin(); it != nic->end(); ++it) {
        //LvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    LvItem.mask = LVIF_PARAM | LVIF_STATE;
    LvItem.state = 0;
    LvItem.stateMask = 0;
    LvItem.iItem = itemPos;
    LvItem.lParam = (LPARAM) it->Id;
    LvItem.iSubItem = 0;
    // LvItem.pszText = "";
    itemPos = m_NicList.InsertItem(&LvItem);
    CString physaddr ;
    physaddr.Format("%X-%X-%X-%X-%X-%X", it->mPhysAddr[0],  it->mPhysAddr[1],
        it->mPhysAddr[2], it->mPhysAddr[3], it->mPhysAddr[4], it->mPhysAddr[5]);
    m_NicList.SetItemText (itemPos, NICFD_CONNECTION, it->mConnectionName.c_str());
    m_NicList.SetItemText (itemPos, NICFD_DEVICENAME, it->DeviceDesc.c_str());
    m_NicList.SetItemText (itemPos, NICFD_PHYSADDRESS, (LPCTSTR)physaddr.GetString());
    itemPos++;
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL NicListDlg::SetNicManager(NicManager &manager) {
    m_pNicManager = & manager;
    return TRUE;
}