// ProgramDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CPEReliabilityTool.h"
#include "ProgramDialog.h"
#include "afxdialogex.h"
#include "log.h"
#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include <initguid.h>
#include <setupapi.h>
#include <device.h>
#include "jrdmmidiag.h"
#include "adb_dev_register.h"
#include "NicManager.h"
#include "QLib_Defines.h"

// CProgramDialog dialog

IMPLEMENT_DYNAMIC(CProgramDialog, CDialogEx)

CProgramDialog::CProgramDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProgramDialog::IDD, pParent)
{
    mDevSwitchEvt = ::CreateEvent(NULL,TRUE,FALSE,_T("CPEMMITEST"));

}

CProgramDialog::~CProgramDialog()
{ 
    if (mDevSwitchEvt != NULL) {
      ::CloseHandle(mDevSwitchEvt);
      mDevSwitchEvt = NULL;
    }
    WSACleanup();
}

BOOL CProgramDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_EditIMEI.SetLimitText (15);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProgramDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
    DDX_Control(pDX, IDC_EDIT_IMEI, m_EditIMEI);
}


BEGIN_MESSAGE_MAP(CProgramDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_DEVICECHANGE()
    ON_BN_CLICKED(IDC_BUTTON_WRITEIMEI, &CProgramDialog::OnBnClickedButtonWriteimei)
END_MESSAGE_MAP()


// CProgramDialog message handlers

void CProgramDialog::OnBnClickedButtonWriteimei()
{
    mNic.EnumNetCards();

    NetCardStruct nic = mNic.GetDefaultNic();
	
if (mNic.GetNicNum() <= 0) {
        AfxMessageBox(_T("There are no ethernet Network Interface Card"), MB_OK);
        return;
    }

    if (mNic.GetNicNum() > 1) {
        AfxMessageBox(_T("There are more than one ethernet Network Interface Card"), MB_OK);
        return;
    }

    AfxBeginThread(CProgramDialog::WriteIMEI, this);    
}

UINT CProgramDialog::WriteIMEI(LPVOID wParam) {
    CProgramDialog *testDialog;
    string result;
    CString text;

    testDialog = (CProgramDialog *)wParam;

    SOCKET sock = testDialog->GetTelnetSocket();
    if (INVALID_SOCKET == sock) {        
        return 1;
    }
    TelnetClient tn(sock, 1500, true);

    LOGE("start telnet negotiate");
    
    tn.send_command(NULL, result);
    tn.send_command(NULL, result);
   

    tn.send_command("send_data 254 0 0 8 1 0 0", result);
    tn.send_command("usb_switch_to PC", result);
	
    testDialog->DiagWork();
	
    tn.send_command("send_data 254 0 0 8 0 0 0", result);
    tn.send_command("usb_switch_to IPQ", result);

    tn.send_command("exit", result);
    closesocket(sock);

    return 0;
}

VOID CProgramDialog::WaitForDevice(long seconds) {
    ::WaitForSingleObject(mDevSwitchEvt, seconds * 1000);
}

BOOL CProgramDialog::DiagWork() {
    vector<CDevLabel> devicePath;
    vector<CDevLabel>::iterator iter;
    BOOL success = FALSE;
	int port = QLIB_COM_AUTO_DETECT;

    WaitForDevice(30);
	
	LOGE("try search %S", SRV_JRDUSBSER);
    GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_JRDUSBSER, devicePath, false);
	if (devicePath.empty()) {		
		LOGE("try search %S", SRV_ALCATELUSBSER);
	    GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_ALCATELUSBSER, devicePath, false);
	}
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, SRV_JRDUSBSER, devicePath, false);
    //for  COM1, GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //GetDevLabelByGUID(&GUID_DEVCLASS_PORTS , SRV_SERIAL, devicePath, false);

    if (!devicePath.empty()) {
        //AddTestResult(_T("USB"), FALSE, "no USB port found");
        //return FALSE;
	    CDevLabel diag = devicePath.front();
		port = diag.GetComPortNum();
    }

    if(ConnectMS(port)) {
        string result;		
		string imei;
		CString strEdit;
		
		m_EditIMEI.GetWindowText(strEdit);
		CStringToString(strEdit, imei);
        success = DIAG_IMEIWrite(imei, result);
		if (success) {
            AfxMessageBox(_T("Write IMEI successfully"), MB_OK);
		} else {
			strEdit.Format(_T("Failed to Write IMEI, %S"), result.c_str());
            AfxMessageBox(strEdit, MB_OK);
		}
       
    } else {
        if (port > 255 && port != QLIB_COM_AUTO_DETECT) {
            AfxMessageBox(_T("The Com port number is too large, please set it lower than 255."), MB_OK);
        } else {
            AfxMessageBox(_T("can not connect to device"), MB_OK);
        }
    }
    DisconnectMs();

    devicePath.clear();
    return success;
}

BOOL CProgramDialog::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
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
                //mDeviceManager.HandleComDevice();
                ::SetEvent(mDevSwitchEvt);
                break;
            }

        case DBT_DEVTYP_DEVICEINTERFACE:
            LOGI("device arrive, DBT_DEVTYP_DEVICEINTERFACE");
            //SetTimer(TIMER_EVT_USBADB, 2000, &CmdmfastbootDlg::DeviceEventTimerProc);
            //HandleDeviceArrived(pDevInf->dbcc_name);
            //mDeviceManager.EnumerateAdbDevice();

            break;
        }
    } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
        switch (phdr->dbch_devicetype) {
        case DBT_DEVTYP_DEVICEINTERFACE:
            {
                //mDeviceManager.HandleDeviceRemoved(pDevInf, dwData);
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


SOCKET CProgramDialog::GetTelnetSocket() {
    NetCardStruct nic = mNic.GetDefaultNic();
    mNic.UpdateNic(nic);
    //m_MmiDevInfo = nic.mConnectionName;

    if (nic.mGateway.IsEmpty()) {
        LOGE( "No device IP address.");
        return INVALID_SOCKET;
    }

    PCHAR ip_addr = WideStrToMultiStr(nic.mGateway.GetString());
    if (ip_addr == NULL) {
        LOGE( "can not get IP address.");
        return INVALID_SOCKET;
    }

    CString text;
    text.Format(_T("Connect device %s"), nic.mGateway.GetString());
    //LOGE(text);

    SOCKET socket = ConnectServer(ip_addr, TELNET_PORT);
    delete [] ip_addr;
    return socket;
}
