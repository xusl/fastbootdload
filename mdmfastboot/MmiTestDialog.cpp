// MmiTestDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "MmiTestDialog.h"
#include "afxdialogex.h"
#include "telnet.h"
#include "log.h"
#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include <initguid.h>
#include <setupapi.h>
#include "jrdmmidiag.h"
// MmiTestDialog dialog

IMPLEMENT_DYNAMIC(MmiTestDialog, CDialogEx)

MmiTestDialog::MmiTestDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(MmiTestDialog::IDD, pParent),
    mDeviceManager(MmiTestDialog::RunMmiTest)
{
	EnableAutomation();
}

MmiTestDialog::~MmiTestDialog()
{
}

void MmiTestDialog::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialogEx::OnFinalRelease();
}

void MmiTestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_MMI_STATUS, m_MmiStatus);
    DDX_Text(pDX, IDC_MMI_DEVINFO, m_MmiDevInfo);
    DDX_Control(pDX, IDC_LIST_MMI, m_MmiItemList);
    DDX_Control(pDX, IDOK, m_StartButton);
    DDX_Control(pDX, IDCANCEL, m_ExitButton);
}


BEGIN_MESSAGE_MAP(MmiTestDialog, CDialogEx)
	ON_WM_DEVICECHANGE()
    ON_BN_CLICKED(IDOK, &MmiTestDialog::OnBnClickedStart)
    ON_BN_CLICKED(IDCANCEL, &MmiTestDialog::OnBnClickedExit)
	ON_MESSAGE(UI_MESSAGE_DEVICE_INFO, &MmiTestDialog::OnDeviceInfo)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(MmiTestDialog, CDialogEx)
END_DISPATCH_MAP()

// Note: we add support for IID_IMmiTestDialog to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the
//  dispinterface in the .IDL file.

// {0391DA2E-326F-4B44-94CD-66EBA49C0C92}
static const IID IID_IMmiTestDialog =
{ 0x391DA2E, 0x326F, 0x4B44, { 0x94, 0xCD, 0x66, 0xEB, 0xA4, 0x9C, 0xC, 0x92 } };

BEGIN_INTERFACE_MAP(MmiTestDialog, CDialogEx)
	INTERFACE_PART(MmiTestDialog, IID_IMmiTestDialog, Dispatch)
END_INTERFACE_MAP()


// MmiTestDialog message handlers


BOOL MmiTestDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    mDeviceManager.Initialize(this, FALSE);

    m_MmiItemList.InsertColumn(0, _T("Item"),LVCFMT_LEFT, 90);
    m_MmiItemList.InsertColumn(1, _T("File Name"),LVCFMT_LEFT, 280);
    m_MmiItemList.InsertColumn(1, _T("Result"),LVCFMT_LEFT, 180);
    //m_MmiItemList.SetExtendedStyle(m_MmiItemList.GetExtendedStyle());//设置控件有勾选功能

    m_MmiItemList.InsertItem(0, _T("MAC"));
    m_MmiItemList.SetItemText(0,1, _T("sdf"));
    m_MmiItemList.SetItemText(0,2,_T("Ok"));

    mNic.EnumNetCards();

    NetCardStruct nic = mNic.GetDefaultNic();
    m_MmiDevInfo = nic.mConnectionName;
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL MmiTestDialog::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
    if (dwData == 0)
    {
        //LOGD("OnDeviceChange, dwData == 0 .EventType: 0x%x", nEventType);
        return FALSE;
    }

    DEV_BROADCAST_HDR* phdr = (DEV_BROADCAST_HDR*)dwData;
    PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)phdr;

    //DEBUG("OnDeviceChange, EventType: 0x%x, DeviceType 0x%x", nEventType, phdr->dbch_devicetype);

    if (nEventType == DBT_DEVICEARRIVAL) {
        switch( phdr->dbch_devicetype ) {
        case DBT_DEVTYP_DEVNODE:
            WARN("OnDeviceChange, get DBT_DEVTYP_DEVNODE");
            break;
        case DBT_DEVTYP_VOLUME:
            {
                /* enumerate devices and shiftdevice
                 * When device, twice this message , one is mass storage (composite device),
                 * another is cd-rom. use timer to delay, by reset timer, we only need enumerate
                 * one time actually.
                 */
                LOGI("OnDeviceChange, get DBT_DEVTYP_VOLUME");
//                SetTimer(TIMER_EVT_REJECTCDROM, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
                break;
            }
        case DBT_DEVTYP_PORT:
            {
                LOGI("device arrive, DBT_DEVTYP_PORT");
                //SetTimer(TIMER_EVT_COMPORT, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
                //HandleDeviceArrived(pDevInf->dbcc_name);
                mDeviceManager.HandleComDevice();
                break;
            }

        case DBT_DEVTYP_DEVICEINTERFACE:
            LOGI("device arrive, DBT_DEVTYP_DEVICEINTERFACE");
            //SetTimer(TIMER_EVT_USBADB, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
            //HandleDeviceArrived(pDevInf->dbcc_name);
            mDeviceManager.EnumerateAdbDevice();

            break;
        }
    } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
        switch (phdr->dbch_devicetype) {
        case DBT_DEVTYP_DEVICEINTERFACE:
            {
                mDeviceManager.HandleDeviceRemoved(pDevInf, dwData);
                break;
            }
        case DBT_DEVTYP_VOLUME:
            {
                /* enumerate devices and shiftdevice
                */
                LOGI("OnDeviceChange, get DBT_DEVTYP_VOLUME");
                break;
            }
        case DBT_DEVTYP_PORT:
            {
                LOGI("device removed, DBT_DEVTYP_PORT");
                break;
            }
        }
    }

    return TRUE;
}

UINT MmiTestDialog::RunMmiTest(LPVOID wParam) {
    UsbWorkData* data = (UsbWorkData*)wParam;
    DeviceInterfaces *dev;
    dev = data->mActiveDevIntf;
    CDevLabel* diag = dev->GetDiagIntf();
    ConnectMS_NV( diag->GetComPortNum());
    return 0;
}

void MmiTestDialog::OnBnClickedStart()
{
    mDeviceManager.SetWork(TRUE, FALSE);
    mDeviceManager.HandleComDevice(TRUE);
    //CDialogEx::OnOK();
}


void MmiTestDialog::OnBnClickedExit()
{
    // TODO: Add your control notification handler code here
    CDialogEx::OnCancel();
}


LRESULT MmiTestDialog::OnDeviceInfo(WPARAM wParam, LPARAM lParam)
{
    UsbWorkData* data = (UsbWorkData*)lParam;
    UIInfo* uiInfo = (UIInfo*)wParam;

    if (uiInfo == NULL) {
        ERROR("Invalid wParam");
        return -1;
    }

    if (data == NULL ) {
        ERROR("Invalid lParam");
        delete uiInfo;
        return -1;
    }

    switch(uiInfo->infoType ) {
    case TITLE:
      //  data->pCtl->SetTitle(uiInfo->sVal);
        break;

    case PROGRESS_VAL:
        //data->pCtl->SetProgress(uiInfo->iVal);
        break;

    default:
        //data->pCtl->SetInfo(uiInfo->infoType, uiInfo->sVal);
        break;
    }

    delete uiInfo;
    return 0;
}

