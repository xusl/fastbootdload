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
#include "log.h"
#include <initguid.h>
#include <winioctl.h>
#include <devguid.h>
#include "usbiodef.h"

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

//    int             iThreadNum;
//    int             iThreadTotal;

unsigned char MAC_ADDR[8] = {0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char MAC_UI[8] = {0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
typedef struct
{
    const char*     pcFirmwareFile;
    teEepromErase   eEepromErase;
    const char*     pcLoadEEPROMFile;

    unsigned char*  pcMAC_Address;
  //  uint64_t        u64MAC_Address;

    int             iInitialSpeed;
    int             iProgramSpeed;
    int             iVerify;
    int             iVerbosity;
    tsConnection    sConnection;
    HANDLE          hThread;
    LifeSensorFlashProgrammerDlg *hProgrammerDlg;
    int            bWriteMACAddress;
} tsProgramThreadArgs;

static tsProgramThreadArgs sProgramThreadArgs = {
    "FLASH",
    E_ERASE_EEPROM_NONE, //E_ERASE_EEPROM_ALL,//erase eeprom   //modify by xusl 06.24
    NULL,
    NULL,  //pcMAC_Address
    38400,
    1000000,
    1,  //flash verify
    0,
	E_CONNECT_SERIAL, //sConnection
	NULL,
	NULL,    //hProgrammerDlg
	TRUE
};

static DWORD dwProgramThread(void* pvData);
void AddProgrammedDevicesCount();

char *FlashFilePath = "F:\\fastbootdload\\ZigBeeFlashProgrammer\\LifeSensorFlashProgrammer\\ZONE_JN5169_DR1199_CSW.bin";// NULL;
tsConnection    *asConnections     = NULL;
tsProgramThreadArgs *asThreads      = NULL;
uint32_t        u32NumConnections   = 0;
uint32_t        NumOfDevicesProgrammed = 0;
CRITICAL_SECTION m_ProgrammerCountSection;
CRITICAL_SECTION gMacAddressSection;

#define COLOR_RED 0x0000FF
#define COLOR_BLUE 0xFF0000

/** Import binary data from FlashProgrammerExtension_JN5168.bin */
int _binary_FlashProgrammerExtension_JN5168_bin_start;
int _binary_FlashProgrammerExtension_JN5168_bin_size;
int _binary_FlashProgrammerExtension_JN5169_bin_start;
int _binary_FlashProgrammerExtension_JN5169_bin_size;
int _binary_FlashProgrammerExtension_JN5179_bin_start;
int _binary_FlashProgrammerExtension_JN5179_bin_size;

char * flashExtension = NULL;

static int importExtension( char * file, int * start, int * size ) {
    size_t bytestoread = 0;

    FILE* fp = NULL;
    int bytesread;
    if ( ( fp = fopen(file,"r") ) <= 0 ) {
        LOGE("open %s failed", file);
        return 0;
    }

    fseek( fp, 0L, SEEK_END );
    bytestoread =ftell(fp);
    fseek( fp, 0L, SEEK_SET );

    if ( ( flashExtension = (char *)malloc(bytestoread + 100 ) ) == NULL ) {
        perror("malloc");
        return 0;
    }

    char * pbuf = flashExtension;
    while ( !feof(fp)) {
        if ( ( bytesread = fread( pbuf, bytestoread, 1, fp) ) < 0 ) {
            break;
        }
        //bytestoread -= bytesread;
        pbuf += bytesread;
        }
    fclose(fp);
        *start = (int)flashExtension;
        *size  = bytestoread;
        printf( "Loaded binary of %d bytes\n", *size );
        return 1;

}

//#define IOT_EXTENSION_PATH "/usr/share/iot"
#define IOT_EXTENSION_PATH "."
static teStatus ePRG_ImportExtension(tsPRG_Context *psContext)
{
    int ret = 0;
    switch (CHIP_ID_PART(psContext->sChipDetails.u32ChipId))
    {
        case (CHIP_ID_PART(CHIP_ID_JN5168)):
            ret = importExtension( IOT_EXTENSION_PATH "/FlashProgrammerExtension_JN5168.bin",
                &_binary_FlashProgrammerExtension_JN5168_bin_start,
                &_binary_FlashProgrammerExtension_JN5168_bin_size );
            psContext->pu8FlashProgrammerExtensionStart    = (uint8_t *)_binary_FlashProgrammerExtension_JN5168_bin_start;
            psContext->u32FlashProgrammerExtensionLength   = (uint32_t)_binary_FlashProgrammerExtension_JN5168_bin_size;
            break;
        case (CHIP_ID_PART(CHIP_ID_JN5169)):
            ret = importExtension( IOT_EXTENSION_PATH "/FlashProgrammerExtension_JN5169.bin",
                &_binary_FlashProgrammerExtension_JN5169_bin_start,
                &_binary_FlashProgrammerExtension_JN5169_bin_size );
            psContext->pu8FlashProgrammerExtensionStart    = (uint8_t *)_binary_FlashProgrammerExtension_JN5169_bin_start;
            psContext->u32FlashProgrammerExtensionLength   = (uint32_t)_binary_FlashProgrammerExtension_JN5169_bin_size;
            break;
        case (CHIP_ID_PART(CHIP_ID_JN5179)):
            ret = importExtension( IOT_EXTENSION_PATH "/FlashProgrammerExtension_JN5179.bin",
                &_binary_FlashProgrammerExtension_JN5179_bin_start,
                &_binary_FlashProgrammerExtension_JN5179_bin_size );
            psContext->pu8FlashProgrammerExtensionStart    = (uint8_t *)_binary_FlashProgrammerExtension_JN5179_bin_start;
            psContext->u32FlashProgrammerExtensionLength   = (uint32_t)_binary_FlashProgrammerExtension_JN5179_bin_size;
            break;
    }
    if ( ret ) {
        return E_PRG_OK;
    }

    return E_PRG_ERROR;
}

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
	DDX_Control(pDX, IDC_PROGRAM, m_Program);
	DDX_Control(pDX, IDC_COMLIST, m_Comlist);
	DDX_Control(pDX, IDC_OPEN, m_Open);
	DDX_Control(pDX, IDC_VERIFY, m_Verify);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_BAUDRATE, m_BaudRate);
	DDX_Control(pDX, IDC_ERASE, m_Erase);
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
	ON_BN_CLICKED(IDC_PROGRAM, OnProgram)
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

	m_Progress.SetRange(0,100);

    m_mac_en.EnableWindow(TRUE);

	//Get COM Ports
	HandleComDevice();

	m_Program.SetForeColor(COLOR_BLUE);
	m_Program.SetTextFont(120,"Arial");
	m_Program.SetTextFont(120,"Verdana");

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

teStatus HandleProgrammerError(tsProgramThreadArgs *psArgs , int error)
{
    PrgMsg* info = new PrgMsg;

  info->eType = PRG_MSG_ERROR;
  info->sVal = "";
  info->iVal = error;
  psArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                          (WPARAM)info,
                          (LPARAM)psArgs);
    return E_PRG_OK;
}


teStatus cbProgress(void *pvUser, const char *pcTitle, const char *pcText, int iNumSteps, int iProgress)
{
    tsProgramThreadArgs *psArgs = (tsProgramThreadArgs *)pvUser;

    PrgMsg* info = new PrgMsg;

  info->eType = PRG_MSG_PROMPT;
  info->sVal.Format("%15s: %s: %3d%%", psArgs->sConnection.pcName, pcText, (iProgress * 100) / iNumSteps);
  info->iVal = 0;
  psArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                          (WPARAM)info,
                          (LPARAM)psArgs);

    if (psArgs->iVerbosity > 0) {
        LOGD("%15s: %s: %3d%%\n", psArgs->sConnection.pcName, pcText, (iProgress * 100) / iNumSteps);
    }
    return E_PRG_OK;
}

teStatus cbConfirm(void *pvUser, const char *pcTitle, const char *pcText)
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
////////////////////////////////////////////////////////////////////////////////////////////////
void AddProgrammedDevicesCount() {
            EnterCriticalSection(&m_ProgrammerCountSection);
            NumOfDevicesProgrammed ++;
            LeaveCriticalSection(&m_ProgrammerCountSection);
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

    tsPRG_Context       sContext;
    unsigned int         i;
    uint32_t             orgin_connection = u32NumConnections;

    for (i = 0; i < u32NumConnections; i++)
        delete p_CheckBox[i];

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

    for (i = 0; i < u32NumConnections; i++) {
        CRect rect;
        rect.left = 30 + 150 * floor(float(i / 13));
        rect.bottom = 40 + (i % 13)*20;
        rect.top = rect.bottom - 20;
        rect.right = rect.left + 170;

        p_CheckBox[i] = new CButton();
        p_CheckBox[i]->Create(_T(asConnections[i].pcName),
                              WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                              rect, this, 1100 + i );
        p_CheckBox[i]->SetCheck(TRUE);
    }

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

    return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD LifeSensorFlashProgrammerDlg::ScheduleProgrammer() {
    bool haveThread;
    LOGD("Programmer Version: %s (libprogrammer version %s)", Version, pcPRG_Version);

    //NumOfDevicesProgrammed = 0;
    m_Program.SetForeColor(COLOR_BLUE);
    m_Program.SetText("Start Programming ...");

    for (unsigned int i = 0; i < u32NumConnections; i++)
    {
        //m_Progress.SetPos((i*100)/u32NumConnections);

        if(!p_CheckBox[i]->GetCheck() || asThreads[i].hThread != NULL)
            continue;

        memcpy(&asThreads[i], &sProgramThreadArgs, sizeof(tsProgramThreadArgs));

        asThreads[i].sConnection = asConnections[i];
        asThreads[i].hProgrammerDlg = this;

        for(int k=0;k<8;k++)
        {
            MAC_ADDR[k] = MAC_UI[k];
        }

        if(m_mac_en.GetCheck() == TRUE)
            asThreads[i].pcMAC_Address = MAC_ADDR;
        else
            asThreads[i].pcMAC_Address = NULL;

        asThreads[i].bWriteMACAddress = m_mac_en.GetCheck();

        //thread_begin:
        asThreads[i].hThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))dwProgramThread, &asThreads[i], 0, NULL);
        if (!asThreads[i].hThread)
        {
            LOGE("Error starting thread for device %s", asThreads[i].sConnection.pcName);
            continue;
        }

        //WaitForSingleObject(asThreads[i].hThread, INFINITE);

        //unsigned long dwExitCode;
        //GetExitCodeThread(asThreads[i].hThread, &dwExitCode);
    }
    //printf("Number of Devices Programmed : %d\n",NumOfDevicesProgrammed);

    SetTimer(TIMER_EVT_SCHEDULE, TIMER_ELAPSE, NULL);
    return 0;
}


void LifeSensorFlashProgrammerDlg::OnTimer(UINT_PTR nIDEvent) {
    KillTimer(nIDEvent);
    ScheduleProgrammer();
}

static DWORD dwProgramThread(void *pvData)
{
    tsPRG_Context   sContext;
    tsProgramThreadArgs *psArgs = (tsProgramThreadArgs*)pvData;
	int error = 0;

    if (ePRG_Init(&sContext) != E_PRG_OK)
    {
		error = COMPortNotRespond;
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

    if (ePRG_ConnectionOpen(&sContext, &psArgs->sConnection) != E_PRG_OK)
    {
		error = COMPortNotRespond;
        ePRG_Destroy(&sContext);
        goto ERROR;
    }

	if (ePRG_ChipGetDetails(&sContext) != E_PRG_OK)
    {
		error = COMPortNotRespond;
    	goto done;
	}
    if (psArgs->iInitialSpeed != psArgs->iProgramSpeed)
    {
        psArgs->sConnection.uDetails.sSerial.u32BaudRate = psArgs->iProgramSpeed;

        if (ePRG_ConnectionUpdate(&sContext, &psArgs->sConnection) != E_PRG_OK)
        {
			error = COMPortNotRespond;
            goto ERROR;
        }
    }

    if (psArgs->pcFirmwareFile)
    {
        /* Have file to program */
		//if (ePRG_FwOpen(&sContext, "C:\\flashfile.bin") != E_PRG_OK)
        //if (ePRG_FwOpen(&sContext, (char *)psArgs->pcFirmwareFile) != E_PRG_OK)
		if (ePRG_FwOpen(&sContext, FlashFilePath) != E_PRG_OK)
        {
			error = FlashFileError;
            goto done;
        }

        if (ePRG_FlashProgram(&sContext, cbProgress, cbConfirm, psArgs) != E_PRG_OK)
        {
			error = FlashFileError;
            goto done;
        }

        if (psArgs->iVerify)
        {
            printf("Verifying\n");
            if (ePRG_FlashVerify(&sContext, cbProgress, psArgs) != E_PRG_OK)
            {
				error = FlashVerificationError;
                goto done;
            }
        }
    }
	else
	{
		error = FlashFileNotFound;
        goto done;
	}


    if (psArgs->eEepromErase != E_ERASE_EEPROM_NONE)
    {
    if ( ePRG_ImportExtension(&sContext) != E_PRG_OK ) {
			error = EepromEraseError;
            goto ERROR;
    }
        if (ePRG_EepromErase(&sContext, psArgs->eEepromErase, cbProgress, psArgs) != E_PRG_OK)
        {
			error = EepromEraseError;
            goto ERROR;
        }
    }

	if (psArgs->pcMAC_Address!= NULL)
    {
        if (ePRG_MACAddressSet(&sContext, psArgs->pcMAC_Address, cbProgress,cbConfirm, psArgs) != E_PRG_OK)
        {
			error = MACprogrammingError;
            goto ERROR;
        }
    }

done:
    //if (psArgs->iInitialSpeed != psArgs->iProgramSpeed)
    {
        if (psArgs->iVerbosity > 1)
        {
            printf("Setting baudrate: %d\n", psArgs->iInitialSpeed);
        }

        psArgs->sConnection.uDetails.sSerial.u32BaudRate = psArgs->iInitialSpeed;

        if (ePRG_ConnectionUpdate(&sContext, &psArgs->sConnection) != E_PRG_OK)
        {
        //  printf("Error: %s\n", pcPRG_GetLastStatusMessage(&sContext));
        //  printf("Error setting baudrate - check cabling and power\n");
            ePRG_ConnectionClose(&sContext);
            ePRG_Destroy(&sContext);
			goto ERROR;
        }
    }

    if (ePRG_ConnectionClose(&sContext) != E_PRG_OK)
    {
        goto ERROR;
    }
    ePRG_FwClose(&sContext);
    if (ePRG_Destroy(&sContext) != E_PRG_OK)
    {
        goto ERROR;
    }

    if (error == 0) {
        AddProgrammedDevicesCount();
          PrgMsg* info = new PrgMsg;

  info->eType = PRG_MSG_RESULT;
  info->sVal = "";
  info->iVal = error;
  psArgs->hProgrammerDlg->PostMessage(UI_MESSAGE_PROGRAMMER,
                          (WPARAM)info,
                          (LPARAM)psArgs);
    } else {
    HandleProgrammerError(psArgs, error);
    }

ERROR:
    if (error != 0){
    HandleProgrammerError(psArgs, error);
    }

    psArgs->hThread = NULL;
	return error;
}

LRESULT LifeSensorFlashProgrammerDlg::OnProgrammerMessage(WPARAM wParam, LPARAM lParam) {
    PrgMsg* info = (PrgMsg*)wParam;
    tsProgramThreadArgs * data = (tsProgramThreadArgs*)lParam;

    CString temp;
    switch(info->eType) {
    case PRG_MSG_RESULT:
            temp.Format("Programming : %d ... ", NumOfDevicesProgrammed);
            m_Program.SetForeColor(COLOR_BLUE);
            m_Program.SetText(temp);
            LOGD("------   %s  : Programed OK  --------", data->sConnection.pcName);


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
            temp.Format("%s : ", data->sConnection.pcName);
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
            m_Program.SetForeColor(COLOR_RED);
            m_Program.SetText(temp);
            UpdateData(FALSE);
           // p_CheckBox[i]->SetCheck(FALSE);
        }
        break;

    case PRG_MSG_PROMPT:
            m_Program.SetForeColor(COLOR_BLUE);
            m_Program.SetText(info->sVal);
        break;
    }
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
	unsigned int i;
	CString temp;

	if(FlashFilePath == NULL)
	{
		m_Program.SetForeColor(COLOR_RED);
		m_Program.SetText("ERR: No flash file");
        return;
	}

	m_Program.EnableWindow(FALSE);
	m_Comlist.EnableWindow(FALSE);
	m_Open.EnableWindow(FALSE);

	for(i = 0; i< u32NumConnections ; i++)
		p_CheckBox[i]->EnableWindow(FALSE);
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
	m_Program.EnableWindow(TRUE);

	for(i = 0; i< u32NumConnections ; i++)
		p_CheckBox[i]->EnableWindow(TRUE);

}

void LifeSensorFlashProgrammerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	for (int i = 0; i < u32NumConnections; i++) {
        delete p_CheckBox[i];
     //   asThreads[i].hThread->PostThreadMessage( WM_QUIT, NULL, NULL );
	}
	CDialog::OnClose();
	free(asConnections);
	free(asThreads);
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
		 m_Progress.SetPos(0);
		 UpdateData(FALSE);

		 m_Program.SetForeColor(COLOR_BLUE);
		 m_Program.SetText("Program");
     }
}

void LifeSensorFlashProgrammerDlg::OnComlist()
{
	m_Progress.SetPos(0);
    HandleComDevice();
	m_Program.SetForeColor(COLOR_BLUE);
	m_Program.SetText("Program");
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

