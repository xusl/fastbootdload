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

unsigned char MAC_ADDR[8] = {0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char MAC_UI[8] = {0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
typedef struct
{
    const char*     pcFirmwareFile;
    const char*     pcDumpFlashFile;
    teEepromErase   eEepromErase;
    const char*     pcLoadEEPROMFile;
    const char*     pcDumpEEPROMFile;

    unsigned char*  pcMAC_Address;
    uint64_t        u64MAC_Address;

    int             iInitialSpeed;
    int             iProgramSpeed;
    int             iVerify;
    int             iVerbosity;

    int             iThreadNum;
    int             iThreadTotal;
    tsConnection    sConnection;
#if defined POSIX
    pthread_t       sThread;
#elif defined WIN32
    HANDLE          hThread;
#endif
    uint32_t thread_times;
} tsProgramThreadArgs;

static tsProgramThreadArgs sProgramThreadArgs = {
    "FLASH",
    NULL,
    E_ERASE_EEPROM_NONE, //E_ERASE_EEPROM_ALL,//erase eeprom   //modify by xusl 06.24
    NULL,
    NULL,
    NULL,
    0,
    38400,
    1000000,
    1,  //flash verify
    0,
	0,
	0,
    E_CONNECT_SERIAL,
	NULL,
	NULL,
	0,
	NULL
};

#if defined POSIX
static void *pvProgramThread(void* pvData);
#elif defined WIN32
static DWORD dwProgramThread(void* pvData);
#endif

char *FlashFilePath = "F:\\fastbootdload\\ZigBeeFlashProgrammer\\LifeSensorFlashProgrammer\\ZONE_JN5169_DR1199_CSW.bin";// NULL;
tsConnection    *asConnections     = NULL;
tsProgramThreadArgs *asThreads      = NULL;
uint32_t        u32NumConnections   = 0;
uint32_t        NumOfDevicesProgrammed = 0;

#define COLOR_RED 0x0000FF
#define COLOR_BLUE 0xFF0000

typedef enum ErrorCode
{
	FlashFileNotFound = 1,
	FlashVerificationError,
	FlashFileError,
	COMPortNotRespond,
	EepromEraseError,
	MACprogrammingError
}ErrorCode_t,*pErrorCode_t;

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
	ON_BN_CLICKED(IDC_PROGRAM, OnProgram)
	ON_WM_CLOSE()
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

	//Get COM Ports
	HandleComDevice();

	m_Program.SetForeColor(COLOR_BLUE);
	m_Program.SetTextFont(120,"Arial");
	m_Program.SetTextFont(120,"Verdana");

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


teStatus cbProgress(void *pvUser, const char *pcTitle, const char *pcText, int iNumSteps, int iProgress)
{
    tsProgramThreadArgs *psArgs = (tsProgramThreadArgs *)pvUser;

    if (psArgs->iVerbosity > 0) {
        printf("%15s: %s: %3d%%\n", psArgs->sConnection.pcName, pcText, (iProgress * 100) / iNumSteps);
    }
    return E_PRG_OK;
}

teStatus cbConfirm(void *pvUser, const char *pcTitle, const char *pcText)
{
    char c;
#if defined POSIX
    static struct termios sOldt, sNewt;

    tcgetattr( STDIN_FILENO, &sOldt);
    sNewt = sOldt;

    sNewt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &sNewt);
#endif /* POSIX */

    printf("--- %s ---\n", pcTitle);
    printf("%s\n\n", pcText);
    printf("Y/N\n");

#if defined POSIX
    c = getc(stdin);
    tcsetattr( STDIN_FILENO, TCSANOW, &sOldt);
#elif defined WIN32
    c = _getch();
#endif /* POSIX */

    if ((c == 'Y') || (c == 'y'))
    {
        return E_PRG_OK;
    }
    return E_PRG_ABORTED;
}
////////////////////////////////////////////////////////////////////////////////////////////////

void IncreaseMACAddr(unsigned char * mac)
{
	mac[7]++;
	if(mac[7] == 0x00)
	{
		mac[6] ++;
		if(mac[6] == 0x00)
		{
			mac[5] ++;
			if(mac[5] == 0x00)
			{
				mac[4] ++;
				if(mac[4] == 0x00)
				{
					mac[3] ++;
					if(mac[3] == 0x00)
					{
						mac[2] ++;
						if(mac[2] == 0x00)
						{
							mac[1] ++;
							if(mac[1] == 0x00)
							{
								mac[0] ++;
							}
						}
					}
				}
			}
		}
	}
}
BOOL StartApplication(CString cppAppName, CString cppCommandLine)
{
	BOOL is_ProcessCreated;
    STARTUPINFO S_Info;
	DWORD dwExitCode;
	PROCESS_INFORMATION P_Info;
	CString cppStr;
	memset( &S_Info, 0, sizeof(STARTUPINFO) );
	S_Info.cb = sizeof(STARTUPINFO);
	SYSTEMTIME start_time,stop_time;

	// set the initial directory
	//SetCurrentDirectory(m_cInitialDir);
	GetLocalTime(&start_time);

	// start the program
	is_ProcessCreated = CreateProcess (
		NULL,                // executable file name
		(LPTSTR)(LPCTSTR)cppCommandLine,	// pointer to command line string
		NULL,	// pointer to process security attributes
		NULL,	// pointer to thread security attributes
		FALSE,	// handle inheritance flag
		0,		// creation flags
		NULL,	// pointer to new environment block
		NULL,	// pointer to current directory name
		&S_Info,// pointer to STARTUPINFO
		&P_Info // pointer to PROCESS_INFORMATION
	);

	// test if it fails
	if (is_ProcessCreated == FALSE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			cppStr.Format ("application not implemented in this board", cppAppName);
			MessageBox(NULL,cppStr, "Application not found", MB_ICONERROR);
        } else {
            cppStr.Format ("Cannot start %s application", cppAppName);
			MessageBox(NULL,cppStr, "Application launching failed", MB_ICONERROR);
        }

        return FALSE;
	} else {
		CloseHandle(P_Info.hThread);
		WaitForSingleObject(P_Info.hProcess, INFINITE);
		GetExitCodeProcess(P_Info.hProcess, &dwExitCode);
		CloseHandle(P_Info.hProcess);
		if(dwExitCode != 0) {
			return FALSE;
		} else {
			GetLocalTime(&stop_time);
			//DWORD time_elaps = stop_time.wMinute*60*1000 + stop_time.wSecond*1000 + stop_time.wMilliseconds - start_time.wMinute*60*1000  - start_time.wSecond*1000  - start_time.wMilliseconds;
		    //if(time_elaps < 0x3400)
				//return FALSE;
		}
	}

    // wait for the application - mandatory for some applications
    WaitForInputIdle(P_Info.hProcess, 1000);

    return TRUE;
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
        
	Main_Entry(GetComPorts);
    return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////
DWORD LifeSensorFlashProgrammerDlg::Main_Entry(Operation_t Operation) {
    //printf("JennicModuleProgrammer Version: %s (libprogrammer version %s)\n", Version, pcPRG_Version);
    CString temp;
    CString temp2;

    if (Operation == GetComPorts) {
        tsPRG_Context       sContext;
        unsigned int         i;

        for (i = 0; i < u32NumConnections; i++)
            delete p_CheckBox[i];

        if (ePRG_Init(&sContext) != E_PRG_OK)
        {
            //fprintf(stderr, "Error initialising context\n");
            return -1;
        }

        if (ePRG_ConnectionListInit(&sContext, &u32NumConnections, &asConnections) != E_PRG_OK)
        {
            //printf("Error getting connection list: %s\n", pcPRG_GetLastStatusMessage(&sContext));
            return -1;
        }
        /*        printf("Available connections:\n");
                  for (i = 0; i < u32NumConnections; i++)
                  {
                  if(asConnections[i].portName[8] == 'V')
                  printf("%s %s\n", asConnections[i].pcName, asConnections[i].portName);
                  }*/
        tsConnection *asNewConnection = (tsConnection *)realloc(asConnections, sizeof(tsConnection) * (u32NumConnections + 1));
        asConnections = asNewConnection;
        /*if (ePRG_ConnectionListDestroy(&sContext, u32NumConnections, &asConnections) != E_PRG_OK)
          {
          printf("Error destroying connection list: %s\n", pcPRG_GetLastStatusMessage(&sContext));
          return -1;
          }

          if (ePRG_Destroy(&sContext) != E_PRG_OK)
          {
          fprintf(stderr, "Error destorying context\n");
          return -1;
          }
          */

        CFont  * f;
        f = new CFont;
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
            if(i<13) {
                rect.left = 30;
                rect.bottom = 40 + i*20;
                rect.top = 30 + i*20;
                rect.right = 200;
            } else if(i<26) {
                rect.left = 30 + 150;
                rect.bottom = 40 + (i-13)*20;
                rect.top = 30 + (i-13)*20;
                rect.right = 200 + 150;
            } else if(i<39) {
                rect.left = 30 + 300;
                rect.bottom = 40 + (i-26)*20;
                rect.top = 30 + (i-26)*20;
                rect.right = 200 + 300;
            }

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
    } else if(Operation == Program) {
        NumOfDevicesProgrammed = 0;
        m_Program.SetForeColor(COLOR_BLUE);
        m_Program.SetText("Start Programming ...");

            for (unsigned int i = 0; i < u32NumConnections; i++)
            {
                m_Progress.SetPos((i*100)/u32NumConnections);

                if(!p_CheckBox[i]->GetCheck())
                    continue;

                memcpy(&asThreads[i], &sProgramThreadArgs, sizeof(tsProgramThreadArgs));

                asThreads[i].sConnection = asConnections[i];
                asThreads[i].thread_times = 0;

                for(int k=0;k<8;k++)
                {
                    MAC_ADDR[k] = MAC_UI[k];
                }

                if(m_mac_en.GetCheck() == TRUE)
                    asThreads[i].pcMAC_Address = MAC_ADDR;
                else
                    asThreads[i].pcMAC_Address = NULL;

                //thread_begin:
                asThreads[i].hThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *))dwProgramThread, &asThreads[i], 0, NULL);
                if (!asThreads[i].hThread)
                {
                    continue;//printf("Error starting thread for device %s\n", asThreads[i].sConnection.pcName);
                }

                WaitForSingleObject(asThreads[i].hThread, INFINITE);

                unsigned long dwExitCode;
                GetExitCodeThread(asThreads[i].hThread, &dwExitCode);
                asThreads[i].thread_times ++;
                if(dwExitCode == 0)
                {
                    NumOfDevicesProgrammed ++;
                    CString temp;
                    temp.Format("Programming : %d ... ", NumOfDevicesProgrammed);
                    m_Program.SetForeColor(COLOR_BLUE);
                    m_Program.SetText(temp);
                    //printf("------   %s  : Programed OK  -------- \n", asThreads[i].sConnection.pcName);
                }
                //		else if(asThreads[i].thread_times <= 10)
                //		{
                //			printf("thread retry...\n");
                //			goto thread_begin;
                //		}
                else
                {
                    temp.Empty();
                    temp.Format("%s : ",asConnections[i].pcName);
                    switch(dwExitCode)
                    {
                    case FlashFileNotFound :
                        temp2 = temp +  "FlashFileNotFound";
                        break;
                    case FlashVerificationError :
                        temp2 = temp +  "FlashVerificationError";
                        break;
                    case FlashFileError :
                        temp2 = temp +  "FlashFileError";
                        break;
                    case COMPortNotRespond :
                        temp2 = temp +  "COMPortNotRespond";
                        break;
                    case EepromEraseError :
                        temp2 = temp +  "EepromEraseError";
                        break;
                    case MACprogrammingError :
                        temp2 = temp +  "MACprogrammingError";
                        break;
                    default :
                        break;
                    }
                    m_Program.SetForeColor(COLOR_RED);
                    m_Program.SetText(temp2);
                    UpdateData(FALSE);
                    p_CheckBox[i]->SetCheck(FALSE);
                }

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

            }
            //printf("Number of Devices Programmed : %d\n",NumOfDevicesProgrammed);
        }

return 0;
}


#if defined POSIX
static void *pvProgramThread(void* pvData)
#elif defined WIN32
static DWORD dwProgramThread(void *pvData)
#endif
{
    tsPRG_Context   sContext;
    tsProgramThreadArgs *psArgs = (tsProgramThreadArgs*)pvData;
	int error = 0;

    if (ePRG_Init(&sContext) != E_PRG_OK)
    {
		error = COMPortNotRespond;
        return error;
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
        return error;
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
            return error;
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
            return error;
    }
        if (ePRG_EepromErase(&sContext, psArgs->eEepromErase, cbProgress, psArgs) != E_PRG_OK)
        {
			error = EepromEraseError;
            return error;
        }
    }

	if (psArgs->pcMAC_Address!= NULL)
    {
        if (ePRG_MACAddressSet(&sContext, psArgs->pcMAC_Address, cbProgress,cbConfirm, psArgs) != E_PRG_OK)
        {
			error = MACprogrammingError;
            return error;
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
			return COMPortNotRespond;
        }
    }

    if (ePRG_ConnectionClose(&sContext) != E_PRG_OK)
    {
        return COMPortNotRespond;
    }

    if (ePRG_Destroy(&sContext) != E_PRG_OK)
    {
        return COMPortNotRespond;
    }

	return error;
}

CButton* LifeSensorFlashProgrammerDlg::NewCheckBox(int nID,CRect rect,int nStyle)
{
CString m_Caption;
m_Caption.LoadString(nID);
CButton *p_Button = new CButton();
ASSERT_VALID(p_Button);
p_Button->Create( m_Caption, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | nStyle, rect, this, nID );
return p_Button;
}

void CharToDigit(unsigned char* temp_char)
{
	if(*temp_char >= 65) // 'A'
		*temp_char -= 55;
	else
		*temp_char -= 48;
}

void LifeSensorFlashProgrammerDlg::OnProgram()
{
	// TODO: Add your control notification handler code here
	// Get the devicec need to be programmed
	unsigned int i;
	CString temp;
	unsigned char mac_temp;
	m_Program.EnableWindow(FALSE);
	m_Comlist.EnableWindow(FALSE);
	m_Open.EnableWindow(FALSE);
	m_Progress.SetPos(0);

    //get MAC ADDR
	if(m_mac_en.GetCheck() == TRUE)
	{
		////////////////////////////////////////////////////////
        m_mac1.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[7] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[7] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac2.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[6] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[6] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac3.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[5] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[5] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac4.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[4] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[4] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac5.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[3] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[3] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac6.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[2] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[2] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac7.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[1] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[1] |= mac_temp;

		///////////////////////////////////////////////////////
        m_mac8.GetWindowText(temp);

		mac_temp = temp.GetAt(0);
		CharToDigit(&mac_temp);
		mac_temp = (mac_temp<<4);

        MAC_UI[0] = mac_temp;

		mac_temp = temp.GetAt(1);
		CharToDigit(&mac_temp);
		MAC_UI[0] |= mac_temp;

	}

	if(FlashFilePath == NULL)
	{
		m_Program.SetForeColor(COLOR_RED);
		m_Program.SetText("ERR: No flash file");
		m_Comlist.EnableWindow(TRUE);
		m_Open.EnableWindow(TRUE);
		m_Program.EnableWindow(TRUE);
		return;
	}


	uint32_t NumOfDevicesNeedToBeProgrammed = 0;
	temp.Empty();

	for(i = 0; i< u32NumConnections ; i++)
		p_CheckBox[i]->EnableWindow(FALSE);


	for(i = 0; i< u32NumConnections ; i++)
	{
		if(p_CheckBox[i]->GetCheck())
			NumOfDevicesNeedToBeProgrammed ++;
	}

	if(NumOfDevicesNeedToBeProgrammed == 0)
	{
		m_Program.SetForeColor(COLOR_RED);
		temp.Format("WARNING : No device");
        m_Program.SetText(temp);
		m_Comlist.EnableWindow(TRUE);
		for(i = 0; i< u32NumConnections ; i++)
			p_CheckBox[i]->EnableWindow(TRUE);
		m_Open.EnableWindow(TRUE);
		m_Program.EnableWindow(TRUE);
		return;
	}

	Main_Entry(Program);
	m_Progress.SetPos(100);

	if(NumOfDevicesProgrammed == NumOfDevicesNeedToBeProgrammed)
	{
		temp.Format("OK : %2d devices programmed", NumOfDevicesProgrammed);
		m_Program.SetForeColor(COLOR_BLUE);
	}
	else
	{
		temp.Format("ERR : %2d devices programmed", NumOfDevicesProgrammed);
		m_Program.SetForeColor(COLOR_RED);
	}

	m_Program.SetText(temp);
	m_Comlist.EnableWindow(TRUE);
	m_Open.EnableWindow(TRUE);
	m_Program.EnableWindow(TRUE);

	for(i = 0; i< u32NumConnections ; i++)
		p_CheckBox[i]->EnableWindow(TRUE);

}

void LifeSensorFlashProgrammerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	free(asConnections);
	free(asThreads);
	for (int i = 0; i < u32NumConnections; i++)
        delete p_CheckBox[i];
	CDialog::OnClose();

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

