// LifeSensorFlashProgrammerDlg.cpp : implementation file
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setupapi.h>
#include <dbt.h>
#include <conio.h>
#include <Windows.h>
#include "LifeSensorFlashProgrammer.h"
#include "LifeSensorFlashProgrammerDlg.h"
#include "ChipID.h"
#include "programmer.h"
#include "DeviceConfig.h"
#include "log.h"
#include <initguid.h>
#include <winioctl.h>
#include <devguid.h>
#include "usbiodef.h"
#include "ProgramThread.h"

//#define __STDC__ TRUE
#define vDelay(a) usleep(a * 1000)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef VERSION
#error Version is not defined!
#else
const char *Version = "1.0 (r" VERSION ")";
#endif

#define VERBOSITY_DEFAULT 1

unsigned char MAC_ADDR[8] = {0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char MAC_UI[8] = {0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


static tsProgramThreadArgs sProgramThreadArgs = {
    "FLASH",
    NULL,
    E_ERASE_EEPROM_NONE, //E_ERASE_EEPROM_ALL,//erase eeprom   //modify by xusl 06.24
    NULL,
    NULL,  //au8MacAddress
    38400,
    1000000,
    1,  //flash verify
    0,
	{E_CONNECT_SERIAL}, //sConnection
	NULL,
	NULL,    //hProgrammerDlg
	TRUE
};

void SetDefaultProgramArgs() {
    sProgramThreadArgs.eStatus = E_PRG_INIT;
    sProgramThreadArgs.pcFirmwareFile     = NULL;
    sProgramThreadArgs.eEepromErase       = E_ERASE_EEPROM_ALL;
    sProgramThreadArgs.iInitialSpeed      = 38400;
    sProgramThreadArgs.iProgramSpeed      = 1000000;
    sProgramThreadArgs.iVerify            = 0;
    sProgramThreadArgs.iVerbosity         = VERBOSITY_DEFAULT;
    //    sProgramThreadArgs.iForce             = 0;
    sProgramThreadArgs.u32FlashIndex      = 0;
    sProgramThreadArgs.psDeviceConfigGet = (tsDeviceConfig*)1;
    sProgramThreadArgs.pu32AesKeyGet = (uint32_t*)1;
    sProgramThreadArgs.apu32UserDataGet[0] = (uint32_t*)1;
}

static DWORD dwProgramThread(void* pvData);
void AddProgrammedDevicesCount();

char *FlashFilePath = "ZONE_JN5169_DR1199_CSW.bin";// NULL;
tsConnection    *asConnections     = NULL;
//tsProgramThreadArgs *asThreads      = NULL;
uint32_t        u32NumConnections   = 0;
uint32_t        NumOfDevicesProgrammed = 0;
CRITICAL_SECTION m_ProgrammerCountSection;
CRITICAL_SECTION gMacAddressSection;
map<uint32_t, bool, greater<uint32_t>> mProgramRecords;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LifeSensorFlashProgrammerDlg dialog

LifeSensorFlashProgrammerDlg::LifeSensorFlashProgrammerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LifeSensorFlashProgrammerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(LifeSensorFlashProgrammerDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ANIMAL);
    for (int i = 0; i < MAX_PORTS_ONCE; i++) {
        m_DownloadPort[i] = NULL;
    }
}

LifeSensorFlashProgrammerDlg::~LifeSensorFlashProgrammerDlg() {
    for (int i = 0; i < MAX_PORTS_ONCE; i++) {
        if (m_DownloadPort[i] != NULL) {
           delete m_DownloadPort[i];
           m_DownloadPort[i] = NULL;
        }
  }
}
void LifeSensorFlashProgrammerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LifeSensorFlashProgrammerDlg)

	DDX_Control(pDX, IDC_MAC8, m_mac8);
	DDX_Control(pDX, IDC_MAC7, m_mac7);
	DDX_Control(pDX, IDC_MAC6, m_mac6);
	DDX_Control(pDX, IDC_MAC5, m_mac5);
	DDX_Control(pDX, IDC_MAC4, m_mac4);
	DDX_Control(pDX, IDC_MAC3, m_mac3);
	DDX_Control(pDX, IDC_MAC2, m_mac2);
	DDX_Control(pDX, IDC_MAC1, m_mac1);
	DDX_Control(pDX, IDC_MAC_EN, m_mac_en);
	DDX_Control(pDX, IDC_COMLIST, m_Comlist);
	DDX_Control(pDX, IDC_OPEN, m_Open);
	DDX_Control(pDX, IDC_VERIFY, m_Verify);
	DDX_Control(pDX, IDC_BAUDRATE, m_BaudRate);
	DDX_Control(pDX, IDC_ERASE, m_Erase);
    DDX_Control(pDX, IDC_START_PROGRAM, m_StartProgram);
    DDX_Control(pDX, IDC_PROGRAMMER_COUNTER, m_ProgrammerCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(LifeSensorFlashProgrammerDlg, CDialog)
	//{{AFX_MSG_MAP(LifeSensorFlashProgrammerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_DEVICECHANGE()
    ON_WM_TIMER()
	ON_WM_CLOSE()
    ON_MESSAGE(UI_MESSAGE_PROGRAMMER, &LifeSensorFlashProgrammerDlg::OnProgrammerMessage)
	ON_BN_CLICKED(IDC_START_PROGRAM, OnProgram)
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_COMLIST, OnComlist)
	ON_BN_CLICKED(IDC_MAC_EN, OnMacEn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LifeSensorFlashProgrammerDlg message handlers

BOOL LifeSensorFlashProgrammerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    InitializeCriticalSection(&m_ProgrammerCountSection);
    InitializeCriticalSection(&gMacAddressSection);

    //Init UI
	m_BaudRate.InsertString(0,"1000000");
	m_BaudRate.InsertString(1,"38400");
	m_BaudRate.SetCurSel(0);
	m_BaudRate.EnableWindow(FALSE);

	m_Erase.SetCheck(TRUE);
	m_Erase.EnableWindow(FALSE);

	m_Verify.SetCheck(TRUE);
	m_Verify.EnableWindow(FALSE);

//	m_Progress.SetRange(0,100);

    CFont* ptf=m_ProgrammerCount.GetFont();
    LOGFONT lf;
    ptf->GetLogFont(&lf);
    lf.lfHeight = 20;
    CFont font;
    font.CreateFontIndirect(&lf);
    m_ProgrammerCount.SetFont(&font);

    m_mac_en.EnableWindow(TRUE);

	//Get COM Ports
	HandleComDevice();

    if (FlashFilePath != NULL)
    m_Open.SetWindowText(basename(FlashFilePath));

	m_mac1.LimitText(2);
	m_mac2.LimitText(2);
	m_mac3.LimitText(2);
	m_mac4.LimitText(2);
	m_mac5.LimitText(2);
	m_mac6.LimitText(2);
	m_mac7.LimitText(2);
	m_mac8.LimitText(2);
    CreateDownloadPortDialogs();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void LifeSensorFlashProgrammerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void LifeSensorFlashProgrammerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR LifeSensorFlashProgrammerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

teStatus HandleProgrammerError(DownloadPortCtl *pDownloadPortCtl , int error)
{
    PrgMsg* info = new PrgMsg;

    tsProgramThreadArgs *psArgs = pDownloadPortCtl->GetProgramArgs();
  info->eType = PRG_MSG_ERROR;
  info->sVal = "";
  info->iVal = error;
  psArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                          (WPARAM)info,
                          (LPARAM)pDownloadPortCtl);
    return E_PRG_OK;
}


teStatus cbUI_Progress(void *pvUser, const char *pcTitle, const char *pcText, int iNumSteps, int iProgress)
{
    DownloadPortCtl *pDownloadPortCtl = (DownloadPortCtl *)pvUser;
    tsProgramThreadArgs *psArgs = pDownloadPortCtl->GetProgramArgs();

    PrgMsg* info = new PrgMsg;

  info->eType = PRG_MSG_PROMPT;
  //info->sVal.Format("%15s: %s: %3d%%", psArgs->sConnection.pcName, pcText, (iProgress * 100) / iNumSteps);
  info->sVal.Format("%s: %3d%%", pcText, (iProgress * 100) / iNumSteps);
  info->iVal = 0;
  psArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                          (WPARAM)info,
                          (LPARAM)pDownloadPortCtl);

    if (psArgs->iVerbosity > 0) {
        LOGD("%15s: %s: %3d%%\n", psArgs->sConnection.pcName, pcText, (iProgress * 100) / iNumSteps);
    }
    return E_PRG_OK;
}

teStatus cbUI_Confirm(void *pvUser, const char *pcTitle, const char *pcText)
{
    CString msg = pcTitle;
    msg += "\n";
    msg += pcText;

    int iRet = AfxMessageBox(msg,  MB_YESNO|MB_DEFBUTTON2);
    if (IDYES ==iRet)
    {
      return E_PRG_OK;
    }

    return E_PRG_ABORTED;
}

void vUI_UpdateStatus(tsProgramThreadArgs *psThreadArgs, const char *pcFmt, ...)
{
    char buffer[512] = {0};

    va_list vl;

    va_start(vl, pcFmt);
    vsnprintf(buffer, sizeof(buffer), pcFmt, vl);
      LOGE("%s", buffer);

if (psThreadArgs->hPortCtl != NULL && psThreadArgs->hProgrammerDlg != NULL) {
        PrgMsg* info = new PrgMsg;
        info->eType = PRG_MSG_PROMPT;
        info->sVal = buffer;
        info->iVal = 0;

        psThreadArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                                            (WPARAM)info,
                                            (LPARAM)psThreadArgs->hPortCtl);
}

    va_end(vl);
}

void vUI_UpdateDeviceInfo(tsProgramThreadArgs *psThreadArgs, tsChipDetails *psChipDetails)
{
if (psThreadArgs->hPortCtl == NULL || psThreadArgs->hProgrammerDlg == NULL || psChipDetails == NULL) {
    return;
}
        PrgMsg* info = new PrgMsg;
 char acMacAddress[24];
        sprintf(acMacAddress, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                psChipDetails->au8MacAddress[0] & 0xFF, psChipDetails->au8MacAddress[1] & 0xFF,
                psChipDetails->au8MacAddress[2] & 0xFF, psChipDetails->au8MacAddress[3] & 0xFF,
                psChipDetails->au8MacAddress[4] & 0xFF, psChipDetails->au8MacAddress[5] & 0xFF,
                psChipDetails->au8MacAddress[6] & 0xFF, psChipDetails->au8MacAddress[7] & 0xFF);



        info->eType = PRG_MSG_CHIPDETAIL;
        info->sVal.Format("Mac: %s\n Bootloader Version:%d\n Chip Id:%d", acMacAddress, psChipDetails->u32BootloaderVersion,psChipDetails->u32ChipId);;
        info->iVal = 0;

        psThreadArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                                            (WPARAM)info,
                                            (LPARAM)psThreadArgs->hPortCtl);
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void AddProgrammedDevicesCount(uint32_t chipId) {
    EnterCriticalSection(&m_ProgrammerCountSection);
    NumOfDevicesProgrammed ++;
    mProgramRecords.insert(pair<uint32_t, bool>(chipId, true));
    LeaveCriticalSection(&m_ProgrammerCountSection);
}

BOOL CheckChipProgrammed(uint32_t chipId) {
    BOOL result = FALSE;
    EnterCriticalSection(&m_ProgrammerCountSection);
    map <uint32_t, bool, greater<uint32_t>>::iterator it = mProgramRecords.find(chipId);
    if ( it != mProgramRecords.end() && mProgramRecords[chipId]) {
      result = TRUE;
    }
    LeaveCriticalSection(&m_ProgrammerCountSection);
    return result;
}
void IncreaseMACAddr(unsigned char * mac)
{
    for (signed int i = 7; i >= 0; i--) {
        mac[i]++;
        if(mac[i] > 0x00)
            break;
    }
}

BOOL LifeSensorFlashProgrammerDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
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
            LOGW("OnDeviceChange, get DBT_DEVTYP_DEVNODE");
            break;

        case DBT_DEVTYP_PORT:
            {
                LOGI("device arrive, DBT_DEVTYP_PORT");
                HandleComDevice();
                //    ScheduleDeviceWork(m_flashdirect);
                break;
            }
        }
    } else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
        switch (phdr->dbch_devicetype) {
        case DBT_DEVTYP_PORT:
            {
                LOGI("device removed, DBT_DEVTYP_PORT");
                HandleComDevice();
                break;
            }
        }
    }

    return TRUE;
}

BOOL LifeSensorFlashProgrammerDlg::HandleComDevice(VOID) {
#if 0
    vector<CDevLabel> devicePath;
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_JRDUSBSER, devicePath, false);
    GetDevLabelByGUID(&GUID_DEVINTERFACE_USB_DEVICE, SRV_FTDIBUS, devicePath, false);
    //for  COM1, GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //GetDevLabelByGUID(&GUID_DEVCLASS_PORTS , SRV_SERIAL, devicePath, false);
    vector<CDevLabel>::iterator iter;
    DeviceInterfaces* devintf;
    BOOL success = FALSE;

    for (iter = devicePath.begin(); iter != devicePath.end();++iter) {
        iter->Dump(__FUNCTION__);
        if(mDevCoordinator.AddDevice(*iter, DEVTYPE_DIAGPORT, NULL))
            success = TRUE;
    }
    devicePath.clear();
#endif
    tsPRG_Context       sContext;
    uint32_t             orgin_connection = u32NumConnections;

    if (ePRG_Init(&sContext) != E_PRG_OK)
    {
        LOGE("Error initialising context\n");
        return -1;
    }

    if (ePRG_ConnectionListInit(&sContext, &u32NumConnections, &asConnections) != E_PRG_OK)
    {
        LOGE("Error getting connection list: %s\n", pcPRG_GetLastStatusMessage(&sContext));
        return -1;
    }

    tsConnection *asNewConnection = (tsConnection *)realloc(asConnections, sizeof(tsConnection) * (u32NumConnections + 1));
    asConnections = asNewConnection;


#if 0
    if(asThreads == NULL)
        asThreads = (tsProgramThreadArgs *)malloc(sizeof(tsProgramThreadArgs) * u32NumConnections);
    else
        asThreads = (tsProgramThreadArgs *)realloc(asThreads,sizeof(tsProgramThreadArgs) * u32NumConnections);


    if (!asThreads)
    {
        //printf("Memory allocation failure\n");
        return -1;
    }
    if (orgin_connection < u32NumConnections)
        memset(asThreads + orgin_connection, 0, sizeof(tsProgramThreadArgs) * (u32NumConnections - orgin_connection));
#endif

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD LifeSensorFlashProgrammerDlg::ScheduleProgrammer() {
    LOGD("Programmer Version: %s (libprogrammer version %s)", Version, pcPRG_Version);

    //NumOfDevicesProgrammed = 0;
    for (unsigned int i = 0; i < u32NumConnections; i++)
    {
        BOOL downloading = FALSE;
        for (unsigned int j = 0; j < MAX_PORTS_ONCE; j++) {
            if(m_DownloadPort[j]->IsDownload(_T(asConnections[i].pcName))) {
                downloading = TRUE;
                break;
            }
        }
        if(downloading)
            continue;

        for (unsigned int j = 0; j < MAX_PORTS_ONCE; j++) {
        //m_Progress.SetPos((i*100)/u32NumConnections);
        if(!m_DownloadPort[j]->AttachDevice(_T(asConnections[i].pcName))) {
            continue;
        }
        tsProgramThreadArgs *args = m_DownloadPort[j]->GetProgramArgs();
        memcpy(args, &sProgramThreadArgs, sizeof(tsProgramThreadArgs));
        memcpy(&args->sConnection, &asConnections[i], sizeof(args->sConnection));
        args->hProgrammerDlg = this;
        args->hPortCtl = m_DownloadPort[j];

        for(int k=0;k<8;k++)
        {
            MAC_ADDR[k] = MAC_UI[k];
        }

        if(m_mac_en.GetCheck() == TRUE)
            args->au8MacAddress = MAC_ADDR;
        else
            args->au8MacAddress = NULL;

        args->bWriteMACAddress = m_mac_en.GetCheck();

        //thread_begin:
        m_DownloadPort[j]->UpdateStatus("Start Programming ...");
        args->hThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))dwProgramThread, m_DownloadPort[j], 0, NULL);
        if (!args->hThread)
        {
            LOGE("Error starting thread for device %s", args->sConnection.pcName);
            //continue;
        }
        break;

        //WaitForSingleObject(asThreads[i].hThread, INFINITE);

        //unsigned long dwExitCode;
        //GetExitCodeThread(asThreads[i].hThread, &dwExitCode);
    }
    }
    //printf("Number of Devices Programmed : %d\n",NumOfDevicesProgrammed);

    SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    return 0;
}


void LifeSensorFlashProgrammerDlg::OnTimer(UINT_PTR nIDEvent) {
    KillTimer(nIDEvent);
    ScheduleProgrammer();
}
void LifeSensorFlashProgrammerDlg::UpdateTimer() {
    EnterCriticalSection(&m_ProgrammerCountSection);

    KillTimer(TIMER_EVT_SCHEDULE);
    SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);

    LeaveCriticalSection(&m_ProgrammerCountSection);
}

static DWORD dwProgramThread(void *pvData) {
    tsPRG_Context   sContext;
    DownloadPortCtl * pDownloadPortCtl = (DownloadPortCtl *) pvData;
    tsProgramThreadArgs *psArgs = pDownloadPortCtl->GetProgramArgs();
    int error = 0;

    pDownloadPortCtl->StartDownload();

    if (ePRG_Init(&sContext) != E_PRG_OK)
    {
        error = COMPortNotRespond;
        LOGE("Error initialising context");
        goto ERROR;
    }

    switch (psArgs->sConnection.eType)
    {
    case (E_CONNECT_SERIAL):
        psArgs->sConnection.uDetails.sSerial.u32BaudRate = psArgs->iInitialSpeed;
        break;

    default:
        break;
    }

    /* Set the offsets for flash/eeprom operations */
    sContext.u32FlashOffset     = psArgs->u32FlashOffset;
    sContext.u32EepromOffset    = psArgs->u32EepromOffset;

    if ((psArgs->eStatus = ePRG_ConnectionOpen(&sContext, &psArgs->sConnection)) != E_PRG_OK)
    {
        vUI_UpdateStatus(psArgs, "Error opening connection: %s", pcPRG_GetLastStatusMessage(&sContext));
        error = COMPortNotRespond;
        ePRG_Destroy(&sContext);
        goto ERROR;
    }

    /* Read module details at initial baud rate */
    if ((psArgs->eStatus = ePRG_ChipGetDetails(&sContext)) != E_PRG_OK)
    {
        vUI_UpdateStatus(psArgs, "Error: %s - check cabling and power", pcPRG_GetLastStatusMessage(&sContext));
        error = COMPortNotRespond;
        goto ERROR;
    }

    vUI_UpdateDeviceInfo(psArgs, &sContext.sChipDetails);

    if (CheckChipProgrammed(sContext.sChipDetails.u32ChipId)) {
        vUI_UpdateStatus(psArgs, "Chip is already Programmed, remove it!");
        goto ERROR;
    }

    if (psArgs->iInitialSpeed != psArgs->iProgramSpeed)
    {
        if (psArgs->iVerbosity > 2)
        {
            vUI_UpdateStatus(psArgs, "Setting baudrate: %d", psArgs->iProgramSpeed);
        }

        psArgs->sConnection.uDetails.sSerial.u32BaudRate = psArgs->iProgramSpeed;

        if ((psArgs->eStatus = ePRG_ConnectionUpdate(&sContext, &psArgs->sConnection)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto ERROR;
        }
    }

    if (psArgs->u32FlashIndex)
    {
        /* Select non-default flash index */
        if ((psArgs->eStatus = ePRG_FlashSelectDevice(&sContext, psArgs->u32FlashIndex)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "Selected flash: %s", sContext.sChipDetails.asFlashes[psArgs->u32FlashIndex].pcFlashName);
    }

    if (psArgs->au8MacAddress)
    {
        /* Set new MAC Address - On 4x devices just set it in the app header. On 6x, write to flash index sector */
        if ((psArgs->eStatus = ePRG_MACAddressSet(&sContext, psArgs->au8MacAddress, cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "MAC address updated");
    }

    if (psArgs->pu32AesKeySet)
    {
        if ((psArgs->eStatus = ePRG_AesKeySet(&sContext, psArgs->pu32AesKeySet, cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "AES Key updated");
    }

    if (psArgs->pu32AesKeyGet)
    {
        uint32_t au32AesKeyGet[4];
        if ((psArgs->eStatus = ePRG_AesKeyGet(&sContext, au32AesKeyGet, cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "AES Key: 0x%08X%08X%08X%08X",
                         au32AesKeyGet[0], au32AesKeyGet[1], au32AesKeyGet[2], au32AesKeyGet[3]);
    }

    {
        int i;
        for (i = 0; i < 3; i++)
        {
            uint32_t au32UserDataGet[4];

            if (psArgs->apu32UserDataSet[i])
            {
                if ((psArgs->eStatus = ePRG_UserDataSet(&sContext, i, psArgs->apu32UserDataSet[i], cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
                {
                    vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
                    goto done;
                }
                vUI_UpdateStatus(psArgs, "User data word %d updated", i);
            }

            if (psArgs->apu32UserDataGet[i])
            {
                if ((psArgs->eStatus = ePRG_UserDataGet(&sContext, i, au32UserDataGet, cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
                {
                    vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
                    goto done;
                }
                vUI_UpdateStatus(psArgs, "User data word %d: 0x%08X%08X%08X%08X", i,
                                 au32UserDataGet[0], au32UserDataGet[1], au32UserDataGet[2], au32UserDataGet[3]);
            }
        }
    }

    if (psArgs->psDeviceConfigSet)
    {
        vUI_UpdateStatus(psArgs, "Setting device configuration");

        if ((psArgs->eStatus = ePRG_DeviceConfigSet(&sContext, psArgs->psDeviceConfigSet, cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }

        if (psArgs->iVerbosity > 0)
        {
            char *pcBuffer = NULL;
            if ((psArgs->eStatus = eDeviceConfigToString(&pcBuffer, &sContext.sDeviceConfig)) != E_PRG_OK)
            {
                goto done;
            }
            vUI_UpdateStatus(psArgs, "Device configuration set to: %s", pcBuffer);
            free(pcBuffer);
        }
        else
        {
            vUI_UpdateStatus(psArgs, "Device configured successfully");
        }
    }

    if (psArgs->psDeviceConfigGet)
    {
        /* Device config is always read by ePRG_ChipGetDetails */
        char *pcBuffer = NULL;
        if (eDeviceConfigToString(&pcBuffer, &sContext.sDeviceConfig) != E_PRG_OK)
        {
            goto done;
        }
        vUI_UpdateStatus(psArgs, "Device configuration: %s", pcBuffer);
        free(pcBuffer);
    }

    if (psArgs->pcFirmwareFile)
    {
        /* Have file to program */
        //if (ePRG_FwOpen(&sContext, (char *)psArgs->pcFirmwareFile) != E_PRG_OK)
        if (ePRG_FwOpen(&sContext, FlashFilePath) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }

        vUI_UpdateStatus(psArgs, "Programming %s", sContext.sChipDetails.asFlashes[psArgs->u32FlashIndex].pcFlashName);
        if ((psArgs->eStatus = ePRG_FlashProgram(&sContext, cbUI_Progress, cbUI_Confirm, pDownloadPortCtl)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "Flash programmed successfully");

        if (psArgs->iVerify)
        {
            vUI_UpdateStatus(psArgs, "Verifying %s", sContext.sChipDetails.asFlashes[psArgs->u32FlashIndex].pcFlashName);
            if ((psArgs->eStatus = ePRG_FlashVerify(&sContext, cbUI_Progress, pDownloadPortCtl)) != E_PRG_OK)
            {
                vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
                goto done;
            }
            vUI_UpdateStatus(psArgs, "Flash verified successfully");
        }
    }
    else
    {
        error = FlashFileNotFound;
        goto done;
    }

    if (psArgs->pcDumpFlashFile)
    {
        const char *pcExt = NULL;
        char acDumpFlashFile[1024] = "";

        pcExt = strrchr(psArgs->pcDumpFlashFile, '.');
        if (!pcExt)
        {
            vUI_UpdateStatus(psArgs, "Dump file has no extenstion (%s)", psArgs->pcDumpFlashFile);
            goto done;
        }
        else
        {
            int iFileNameLength = pcExt - psArgs->pcDumpFlashFile;
            memcpy(acDumpFlashFile, psArgs->pcDumpFlashFile, iFileNameLength);
            sprintf(&acDumpFlashFile[iFileNameLength], "_%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X%s",
                    sContext.sChipDetails.au8MacAddress[0] & 0xFF, sContext.sChipDetails.au8MacAddress[1] & 0xFF,
                    sContext.sChipDetails.au8MacAddress[2] & 0xFF, sContext.sChipDetails.au8MacAddress[3] & 0xFF,
                    sContext.sChipDetails.au8MacAddress[4] & 0xFF, sContext.sChipDetails.au8MacAddress[5] & 0xFF,
                    sContext.sChipDetails.au8MacAddress[6] & 0xFF, sContext.sChipDetails.au8MacAddress[7] & 0xFF,
                    pcExt);

            vUI_UpdateStatus(psArgs, "Dumping %s into file %s", sContext.sChipDetails.asFlashes[psArgs->u32FlashIndex].pcFlashName, acDumpFlashFile);

            if ((psArgs->eStatus = ePRG_FlashDump(&sContext, acDumpFlashFile, cbUI_Progress, psArgs)) != E_PRG_OK)
            {
                vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
                goto done;
            }
            vUI_UpdateStatus(psArgs, "Flash dumped successfully");
        }
    }

    if (psArgs->eEepromErase != E_ERASE_EEPROM_NONE)
    {
        if ((psArgs->eStatus = ePRG_EepromErase(&sContext, psArgs->eEepromErase, cbUI_Progress, psArgs)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error erasing EEPROM: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "EEPROM erased successfully");
    }

    if (psArgs->pcLoadEEPROMFile)
    {
        if ((psArgs->eStatus = ePRG_EepromProgram(&sContext, (char*)psArgs->pcLoadEEPROMFile, cbUI_Progress, psArgs)) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error loading EEPROM: %s", pcPRG_GetLastStatusMessage(&sContext));
            goto done;
        }
        vUI_UpdateStatus(psArgs, "EEPROM loaded successfully");
    }

    if (psArgs->au8MacAddress!= NULL)
    {
        if (ePRG_MACAddressSet(&sContext, psArgs->au8MacAddress, cbUI_Progress,cbUI_Confirm, psArgs) != E_PRG_OK)
        {
            error = MACprogrammingError;
            goto ERROR;
        }
    }

done:
    if (psArgs->iInitialSpeed != psArgs->iProgramSpeed)
    {
        if (psArgs->iVerbosity > 2)
        {
            vUI_UpdateStatus(psArgs, "Setting baudrate: %d", psArgs->iInitialSpeed);
        }

        psArgs->sConnection.uDetails.sSerial.u32BaudRate = psArgs->iInitialSpeed;

        if (ePRG_ConnectionUpdate(&sContext, &psArgs->sConnection) != E_PRG_OK)
        {
            vUI_UpdateStatus(psArgs, "Error: %s", pcPRG_GetLastStatusMessage(&sContext));
            vUI_UpdateStatus(psArgs, "Error setting baudrate - check cabling and power");
            ePRG_ConnectionClose(&sContext);
            ePRG_Destroy(&sContext);
            goto ERROR;
        }
    }

    if (ePRG_ConnectionClose(&sContext) != E_PRG_OK)
    {
        goto ERROR;
    }

    if (error == 0) {
        AddProgrammedDevicesCount(sContext.sChipDetails.u32ChipId);
        PrgMsg* info = new PrgMsg;

        info->eType = PRG_MSG_RESULT;
        info->sVal = "";
        info->iVal = error;
        psArgs->hProgrammerDlg->UpdateTimer();
        psArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                                            (WPARAM)info,
                                            (LPARAM)pDownloadPortCtl);
    } else {
        HandleProgrammerError(pDownloadPortCtl, error);
    }

ERROR:
    ePRG_Destroy(&sContext);
    if (error != 0){
        HandleProgrammerError(pDownloadPortCtl, error);
    }

    psArgs->hThread = NULL;
    pDownloadPortCtl->FinishDownload();
    return error;
}

LRESULT LifeSensorFlashProgrammerDlg::OnProgrammerMessage(WPARAM wParam, LPARAM lParam) {
    PrgMsg* info = (PrgMsg*)wParam;
    DownloadPortCtl * downloadPortCtl= (DownloadPortCtl*)lParam;
    tsProgramThreadArgs *args;
    if (info == NULL || downloadPortCtl == NULL) {
    LOGE("Bad parameter, null pointer");
        return 1;
        }
    args = downloadPortCtl->GetProgramArgs();
    if (args == NULL) {
    LOGE("Can not get invalid tsProgramThreadArgs");
        return 2;
    }
    CString temp;
    switch(info->eType) {
    case PRG_MSG_RESULT:
            temp.Format("Programming : %d ", NumOfDevicesProgrammed);
            m_ProgrammerCount.SetWindowText(temp);
            LOGD("------   %s  : Programed OK  --------", args->sConnection.pcName);
        //advance MAC address
        if(m_mac_en.GetCheck() == TRUE)
        {
            IncreaseMACAddr(MAC_UI);

            temp.Format("%.2x",MAC_UI[0]);
            m_mac8.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[1]);
            m_mac7.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[2]);
            m_mac6.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[3]);
            m_mac5.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[4]);
            m_mac4.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[5]);
            m_mac3.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[6]);
            m_mac2.SetWindowText(temp);

            temp.Format("%.2x",MAC_UI[7]);
            m_mac1.SetWindowText(temp);
        }

        break;
    case PRG_MSG_ERROR:
        {
            temp.Empty();
            temp.Format("%s : ", args->sConnection.pcName);
            switch(info->iVal)
            {
            case FlashFileNotFound :
                temp +=  "FlashFileNotFound";
                break;
            case FlashVerificationError :
                temp +=  "FlashVerificationError";
                break;
            case FlashFileError :
                temp +=  "FlashFileError";
                break;
            case COMPortNotRespond :
                 temp +=  "COMPortNotRespond";
                break;
            case EepromEraseError :
                temp +=  "EepromEraseError";
                break;
            case MACprogrammingError :
                 temp +=  "MACprogrammingError";
                break;
            default :
                break;
            }
            downloadPortCtl->UpdateStatus(temp);
            UpdateData(FALSE);
           // p_CheckBox[i]->SetCheck(FALSE);
        }
        break;

    case PRG_MSG_PROMPT:
        downloadPortCtl->UpdateStatus(info->sVal);
        break;

    case PRG_MSG_CHIPDETAIL:
        downloadPortCtl->SetChipDetail(info->sVal);
        break;
    }
    delete info;
    return 0;
}

void LifeSensorFlashProgrammerDlg::SetMacByte(CEdit &edit,  unsigned char * pValue) {
    CString temp;
 //   unsigned char mac_temp;
//    unsigned char buf[8] = {0};
    const char *pTemp;

    edit.GetWindowText(temp);
    pTemp = temp.GetString();
  //  sscanf(pTemp, "%x", buf);

    *pValue = (unsigned char)strtol(pTemp, NULL, 16);//atoi

/*
    mac_temp = temp.GetAt(0);
    if(mac_temp >= 65) // 'A'
        mac_temp -= 55;
    else
        mac_temp -= 48;

    mac_temp = (mac_temp<<4);

    *pValue = mac_temp;

    mac_temp = temp.GetAt(1);
    if(mac_temp >= 65) // 'A'
        mac_temp -= 55;
    else
        mac_temp -= 48;

    *pValue |= mac_temp;
*/
}

void LifeSensorFlashProgrammerDlg::OnProgram()
{
	// TODO: Add your control notification handler code here
	// Get the devicec need to be programmed
	CString temp;

	if(FlashFilePath == NULL)
	{
//		m_Program.SetForeColor(COLOR_RED);
//		m_Program.SetText("ERR: No flash file");
        return;
	}

//	m_Program.EnableWindow(FALSE);
	m_Comlist.EnableWindow(FALSE);
	m_Open.EnableWindow(FALSE);

	//for(i = 0; i< u32NumConnections ; i++)
	//	p_CheckBox[i]->EnableWindow(FALSE);
	//m_Progress.SetPos(0);

    //get MAC ADDR
	if(m_mac_en.GetCheck() == TRUE)
	{
		////////////////////////////////////////////////////////
		SetMacByte(m_mac1, MAC_UI + 7);
        SetMacByte(m_mac2, MAC_UI + 6);
        SetMacByte(m_mac3, MAC_UI + 5);
        SetMacByte(m_mac4, MAC_UI + 4);
        SetMacByte(m_mac5, MAC_UI + 3);
        SetMacByte(m_mac6, MAC_UI + 2);
        SetMacByte(m_mac7, MAC_UI + 1);
        SetMacByte(m_mac8, MAC_UI);
	}

	ScheduleProgrammer();
	//m_Progress.SetPos(100);

	m_Comlist.EnableWindow(TRUE);
	m_Open.EnableWindow(TRUE);
//	m_Program.EnableWindow(TRUE);

	//for(i = 0; i< u32NumConnections ; i++)
	//	p_CheckBox[i]->EnableWindow(TRUE);

}

void LifeSensorFlashProgrammerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//for (int i = 0; i < u32NumConnections; i++) {
    //    delete p_CheckBox[i];
     //   asThreads[i].hThread->PostThreadMessage( WM_QUIT, NULL, NULL );
	//}
	CDialog::OnClose();
	free(asConnections);
	//free(asThreads);
    DeleteCriticalSection(&m_ProgrammerCountSection);
    DeleteCriticalSection(&gMacAddressSection);
}

void LifeSensorFlashProgrammerDlg::OnOpen()
{
	// TODO: Add your control notification handler code here
	 CFileDialog dlg(TRUE, "bin", "*.bin", NULL, "LifeSensor flash file(*.bin)",NULL);
	 if(dlg.DoModal()==IDOK)
     {
		 m_FilePath = dlg.GetPathName();
		 int total = m_FilePath.GetLength();
		 int current = 0;
		 int index = 0;

		 m_FilePath.Replace("\\","\\\\");

		 FlashFilePath = m_FilePath.GetBuffer(m_FilePath.GetLength());

		 m_Open.SetWindowText(dlg.GetFileName());
//		 m_Progress.SetPos(0);
		 UpdateData(FALSE);
     }
}

void LifeSensorFlashProgrammerDlg::OnComlist()
{
//	m_Progress.SetPos(0);
    HandleComDevice();
}

void LifeSensorFlashProgrammerDlg::OnMacEn()
{
    if(m_mac_en.GetCheck() == TRUE)
    {
        m_mac1.EnableWindow(TRUE);
        m_mac2.EnableWindow(TRUE);
        m_mac3.EnableWindow(TRUE);
        m_mac4.EnableWindow(TRUE);

        m_mac5.EnableWindow(TRUE);
        m_mac6.EnableWindow(TRUE);
        m_mac7.EnableWindow(TRUE);
        m_mac8.EnableWindow(TRUE);
    }
    else
    {
        m_mac1.EnableWindow(FALSE);
        m_mac2.EnableWindow(FALSE);
        m_mac3.EnableWindow(FALSE);
        m_mac4.EnableWindow(FALSE);

        m_mac5.EnableWindow(FALSE);
        m_mac6.EnableWindow(FALSE);
        m_mac7.EnableWindow(FALSE);
        m_mac8.EnableWindow(FALSE);
    }
}

BOOL LifeSensorFlashProgrammerDlg::CreateDownloadPortDialogs()
{
    int i, j;
    const int bx = 6, by = 10, pw = 300, ph = 180;
    int x, y;
    const int ROWS = 2;
#if 0
    CFont  * f = new CFont;
    f -> CreateFont(18,  //  nHeight
                    0 ,  //  nWidth
                    0 ,  //  nEscapement
                    0 ,  //  nOrientation
                    FW_BOLD,  //  nWeight
                    TRUE,  //  bItalic
                    FALSE,  //  bUnderline
                    0 ,  //  cStrikeOut
                    ANSI_CHARSET,  //  nCharSet
                    OUT_DEFAULT_PRECIS,  //  nOutPrecision
                    CLIP_DEFAULT_PRECIS,  //  nClipPrecision
                    DEFAULT_QUALITY,  //  nQuality
                    DEFAULT_PITCH  |  FF_SWISS,  //  nPitchAndFamily
                    _T("MS Sans Serif" ));  //  lpszFac
    CRect rect;
    rect.left = LONG(30 + 150 * floor(float(i / COLUMNS)));
    rect.bottom = 40 + (i % COLUMNS)*20;
    rect.top = rect.bottom - 100;
    rect.right = rect.left + 170;
    p_CheckBox[i] = new CButton();
    p_CheckBox[i]->Create(_T(asConnections[i].pcName),
                           WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                           rect, this, 1100 + i );
    p_CheckBox[i]->SetCheck(TRUE);
#endif
    for (j = 0; j < MAX_PORTS_ONCE; j++) {
        if (m_DownloadPort[j] != NULL) {
            continue;
        }
        m_DownloadPort[j] = new DownloadPortCtl;
        m_DownloadPort[j]->Create(IDD_DOWNLOADPORT, this);
        x = bx + LONG(floor(float(j / ROWS))) * (3 + pw);
        y = by + (j % ROWS) * (3+ph);
        m_DownloadPort[j]->SetWindowPos(0, x, y, pw, ph, SWP_SHOWWINDOW);
    }
    Invalidate();

    return true;
}
