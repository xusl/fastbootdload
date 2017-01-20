// MmiTestDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "MmiTestDialog.h"
#include "afxdialogex.h"
#include "log.h"
#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include <initguid.h>
#include <setupapi.h>
#include "jrdmmidiag.h"
// MmiTestDialog dialog

//IMPLEMENT_DYNAMIC(MmiTestDialog, CDialogEx)

MmiTestDialog::MmiTestDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(MmiTestDialog::IDD, pParent),
    mDeviceManager(MmiTestDialog::RunMmiTest),
    m_Work(FALSE)
{
    mDevSwitchEvt = ::CreateEvent(NULL,TRUE,FALSE,_T("CPEMMITEST"));
}

MmiTestDialog::~MmiTestDialog()
{
    if (mDevSwitchEvt != NULL) {
      ::CloseHandle(mDevSwitchEvt);
      mDevSwitchEvt = NULL;
    }
    WSACleanup();
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

// MmiTestDialog message handlers
BOOL MmiTestDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    mDeviceManager.Initialize(this, FALSE);

    m_MmiItemList.InsertColumn(0, _T("Item"),LVCFMT_LEFT, 200);
    m_MmiItemList.InsertColumn(1, _T("Result"),LVCFMT_LEFT, 100);
    m_MmiItemList.InsertColumn(2, _T("Description"),LVCFMT_LEFT, 600);
    //m_MmiItemList.SetExtendedStyle(m_MmiItemList.GetExtendedStyle());//设置控件有勾选功能

    mNic.EnumNetCards();

    NetCardStruct nic = mNic.GetDefaultNic();
    m_MmiDevInfo = nic.mConnectionName;
    GetDlgItem(IDC_MMI_DEVINFO)->SetWindowText(nic.mConnectionName);
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


#define WIFI_GET_2DOT4G_MAC      0
#define WIFI_SET_2DOT4G_MAC  1
#define WIFI_GET_5G_MAC  2
#define WIFI_SET_5G_MAC  3
#define LED_ALL_OFF  0
#define LED_POWER_BIT 1
#define LED_WIFI_BIT  2
#define LED_WPS_ON  3
#define TEST_KEY "send_data 254 0 2 4 1"
#define TEST_LED "send_data 254 0 2 3 0"
#define TEST_RJ45 "send_data 254 0 2 5 1"
#define TEST_RJ11 "send_data 254 0 2 5 0"
#define TEST_WIFI_MAC  "send_data 254 0 3 1 "
#define TEST_PASS    _T("Pass")
#define TEST_FAILED  _T("Failed")


BOOL MmiTestDialog::AddTestResult(CString item, BOOL pass, string description) {
    int nItem = m_MmiItemList.GetItemCount();
    m_MmiItemList.InsertItem(nItem, item);
    m_MmiItemList.SetItemText(nItem, 1, pass ? TEST_PASS : TEST_FAILED);
    m_MmiItemList.SetItemText(nItem, 2, CString(description.c_str()));
    nItem = m_MmiItemList.GetItemCount();
    if (nItem > 0)
        m_MmiItemList.EnsureVisible(nItem - 1, FALSE);
    return TRUE;
}

SOCKET MmiTestDialog::GetTelnetSocket() {
    NetCardStruct nic = mNic.GetDefaultNic();
    m_MmiDevInfo = nic.mConnectionName;

    PCHAR ip_addr = WideStrToMultiStr(nic.mGateway.GetString());
    if (ip_addr == NULL) {
        return INVALID_SOCKET;
    }

    SOCKET socket = ConnectServer(ip_addr, TELNET_PORT);
    delete [] ip_addr;
    return socket;
}

UINT MmiTestDialog::RunMmiTest(LPVOID wParam) {
    MmiTestDialog *testDialog;
    string result;
    CString text;
    BOOL  pass;

    testDialog = (MmiTestDialog *)wParam;

    testDialog->SetStatus( _T("Begin MMI test."));

    SOCKET sock = testDialog->GetTelnetSocket();
    if (INVALID_SOCKET == sock) {
        testDialog->AddTestResult(_T("RJ45 1"), FALSE, "Can not connect device.");
        testDialog->SetWork(FALSE);
        return 1;
    }

    testDialog->AddTestResult(_T("RJ45 1"), TRUE, "Ok.");
    telnet tn(sock, 3000, true);

    LOGE("start telnet negotiate");
    testDialog->SetStatus(_T("start telnet negotiate"));
    tn.send_command(NULL, result);
    tn.send_command(NULL, result);

//    tn.send_command("send_data 254 0 2 5 1", result);

    pass = testDialog->TestItem(tn, _T("RJ45 2"), "send_data 254 0 2 5 1", "254 0 2 5 0 0 0", "254 0 2 5 0 1 0");
    pass = testDialog->TestItem(tn, _T("RJ11"), "send_data 254 0 2 5 0", "254 0 2 5 0 0 0", "254 0 2 5 0 1 0");

#ifdef USE_SENDDATA
    string command = TEST_WIFI_MAC;
    command += "1";
    pass = testDialog->TestWiFi(tn, _T("WiFi 5G MAC"), command.c_str());
    command = TEST_WIFI_MAC;
    command += "0";
    pass = testDialog->TestWiFi(tn, _T("WiFi 2.4G MAC"), command.c_str());
#else
    pass = testDialog->TestWiFi(tn, _T("WiFi 2.4G MAC"), "uci show wireless.wifi0.macaddr");
    pass = testDialog->TestWiFi(tn, _T("WiFi 2.4G SSID"), "uci show wireless.@wifi-iface[0].ssid");
    pass = testDialog->TestWiFi(tn, _T("WiFi 5G MAC"), "uci show wireless.wifi1.macaddr");
    pass = testDialog->TestWiFi(tn, _T("WiFi 5G SSID"), "uci show wireless.@wifi-iface[1].ssid");
#endif

   // pass = testDialog->TestItem(tn, _T("WiFi scan AP"), "send_data 254 0 3 0");

#if 0
    tn.send_command(TEST_LED, result);
    testDialog->SetStatus(CString(result.c_str()));
    testDialog->AddTestResult(_T("LED"), FALSE, result.c_str());
#endif

    tn.send_command("send_data 254 0 0 8 0 0 0", result);
    tn.send_command("usb_switch_to IPQ", result);

    tn.set_receive_timeout(1000);
    pass = testDialog->TestLed(tn, _T("ALL LED OFF"), 0x00000000);
    pass = testDialog->TestLed(tn, _T("Power LED"), 0x00000001);
    pass = testDialog->TestLed(tn, _T("WIFI LED"), 0x00000002);
    pass = testDialog->TestLed(tn, _T("TEL LED"), 0x00000004);
    pass = testDialog->TestLed(tn, _T("Net1-green LED"), 0x00000008);
    pass = testDialog->TestLed(tn, _T("Net2-blue LED"), 0x00000010);
    pass = testDialog->TestLed(tn, _T("Net3-red LED"), 0x00000020);
    //pass = testDialog->TestLed(tn, _T("Signal1 LED"), 0x00000040);
    //pass = testDialog->TestLed(tn, _T("Signal2 LED"), 0x00000080);
    //pass = testDialog->TestLed(tn, _T("Signal3 LED"), 0x00000100);
    //pass = testDialog->TestLed(tn, _T("Zigbee LED"), 0x00000200);
    pass = testDialog->TestLed(tn, _T("WPS LED"), 0x00000400);
    pass = testDialog->TestLed(tn, _T("ALL LED ON"), 0x000007ff);

    tn.set_receive_timeout(5000);
    pass = testDialog->TestKey(tn, _T("Power Key"),  "254 0 2 4 1 1 0", 1, 5);
    pass = testDialog->TestKey(tn, _T("WPS Key"),  "254 0 2 4 1 2 0", 2, 5);
    pass = testDialog->TestKey(tn, _T("Reset Key"),  "254 0 2 4 1 4 0", 4, 5);
    tn.set_receive_timeout(3000);

    tn.send_command("send_data 254 0 0 8 1 0 0", result);
    tn.send_command("usb_switch_to PC", result);
    testDialog->DiagTest();
    tn.send_command("send_data 254 0 0 8 0 0 0", result);
    tn.send_command("usb_switch_to IPQ", result);

    testDialog->SetWork(FALSE);
    LOGE("Test finish");

    tn.send_command("exit", result);
    closesocket(sock);

    return 0;
}

//error code :  "254 0 2 4 1 0 0"
BOOL MmiTestDialog::TestKey(telnet &client, CString item, const string &ok, int key, int elapse) {
    string data;
    char command[64] = {0};
    CString text;
    BOOL pass;

    text.Format(_T("Press '%s' Key within %d seconds after click 'OK'"), item.GetString(), elapse);

    int iRet = AfxMessageBox(text, MB_OK);

    snprintf(command, sizeof command, "send_data 254 0 2 4 1 %d 0", key);
    TestItem(client, item, command, data);

    size_t found = data.rfind("\r\n");
    if (found != string::npos)
        data = data.substr(found + 2);

    pass = (data == ok);
    AddTestResult(item, pass, data.c_str());
    return pass;
}

BOOL MmiTestDialog::TestLed(telnet &client, CString item,  int value) {
    string data;
    char command[64] = {0};
    string ok = "254 0 2 3 0 0 0";
    BOOL pass;

    //snprintf(ok, sizeof ok, "254 0 2 3 0 %d 0", value);
    snprintf(command, sizeof command, "send_data 254 0 2 3 0 %d 0", value);
    TestItem(client, item, command, data);

    size_t found = data.rfind("\r\n");
    if (found != string::npos)
        data = data.substr(found + 2);

    pass = (data == ok);
    AddTestResult(item, pass, data.c_str());
    return pass;
}

BOOL MmiTestDialog::TestItem(telnet &client, CString item, PCCH command, const string &ok, const string &error) {
    string data;
    BOOL pass;

    TestItem(client, item, command, data);

//   LOGE("data is '%s'", data.c_str());
//    LOGE("ok is '%s'",  ok.c_str());

    pass = (data == ok);
    AddTestResult(item, pass, data.c_str());
    return pass;
}

BOOL MmiTestDialog::TestItem(telnet &client, CString item, PCCH command, string &data) {
    CString status = _T("Test item \n\t");
    status += item;
    SetStatus(status);
    client.send_command(command, data);
    return TRUE;
}

BOOL MmiTestDialog::TestWiFi(telnet &client, CString item, PCCH command) {
    string data;
    string result;
    CString text;
    BOOL pass;

    TestItem(client, item, command, data);

#if 0
    text = "Test Result:\n";
    text += data.c_str();
    text += "\n\n Click 'Ok' to confirm pass test";
    int iRet = AfxMessageBox(text, MB_YESNO);
    pass = (IDYES == iRet);
#endif

    size_t found = data.rfind("\r\n");
    if (found != string::npos)
        result = data.substr(found + 2);
    else
        result = data;

     found = result.rfind("=");
    if (found != string::npos) {
        result = result.substr(found + 1);
        pass = !result.empty();
    } else {
        pass = FALSE;
    }
    AddTestResult(item, pass, data.c_str());

    return pass;
}

BOOL MmiTestDialog::DiagTest() {
    vector<CDevLabel> devicePath;
    vector<CDevLabel>::iterator iter;
    BOOL success = FALSE;

    SetStatus(_T("Wait for USB device"));
    WaitForDevice(15);
    GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_JRDUSBSER, devicePath, false);
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, SRV_JRDUSBSER, devicePath, false);
    //for  COM1, GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //GetDevLabelByGUID(&GUID_DEVCLASS_PORTS , SRV_SERIAL, devicePath, false);

    if (devicePath.empty()) {
        AddTestResult(_T("USB"), FALSE, "no USB port found");
        return FALSE;
    }

    CDevLabel diag = devicePath.front();
    if(ConnectMS( diag.GetComPortNum())) {
        string result;
        BOOL pass = DIAG_CheckSIM_Card(result);
        AddTestResult(_T("SIM card"), pass, result);
    } else {
        AddTestResult(_T("USB"), FALSE, "can not connect to device");
    }
    DisconnectMs();

#if 0
    for (iter = devicePath.begin(); iter != devicePath.end();++iter) {
        iter->Dump(__FUNCTION__);
    }
#endif

    devicePath.clear();
    return success;
}

VOID MmiTestDialog::WaitForDevice(long seconds) {
    ::WaitForSingleObject(mDevSwitchEvt, seconds * 1000);
}

BOOL MmiTestDialog::SetWork(BOOL work) {
    if(work)
        m_MmiItemList.DeleteAllItems();
    GetDlgItem(IDOK)->EnableWindow(!work);
    GetDlgItem(IDCANCEL)->EnableWindow(!work);
    return TRUE;
}

BOOL MmiTestDialog::SetStatus(CString text) {
    GetDlgItem(IDC_MMI_STATUS)->SetWindowText(text);
    return TRUE;
}

void MmiTestDialog::OnBnClickedStart()
{
    //mDeviceManager.SetWork(TRUE, FALSE);
    //mDeviceManager.HandleComDevice(FALSE);
    //mDeviceManager.ScheduleDeviceWork();
    AfxBeginThread(MmiTestDialog::RunMmiTest, this);
    SetWork(TRUE);
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
    case UI_DEFAULT:
        m_MmiStatus = uiInfo->sVal;
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

