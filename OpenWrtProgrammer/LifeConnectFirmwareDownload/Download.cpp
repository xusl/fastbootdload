
// Download.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Download.h"
#include "DownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDownloadApp

BEGIN_MESSAGE_MAP(CDownloadApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDownloadApp construction

CDownloadApp::CDownloadApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDownloadApp object

CDownloadApp theApp;


// CDownloadApp initialization

BOOL CDownloadApp::InitInstance()
{
	m_hMutex = ::CreateMutex(NULL, TRUE, _T("LIFECONNECTFWDL_Mutex"));    
	if (GetLastError() == ERROR_ALREADY_EXISTS)                                        //程序已经运行    
	{        
		HWND   oldHWnd = NULL;        
		oldHWnd = ::FindWindow(NULL, _T("HDT.exe"));                   //查找已经运行的程序     
		if (oldHWnd)          
		{               
			::ShowWindow(oldHWnd, SW_SHOWNORMAL);                                        //激活显示找到的已运行的程序        
			::SetForegroundWindow(oldHWnd);                                              //将已运行的程序设置为当前窗口      
		}         
		CloseHandle(m_hMutex);     
		m_hMutex = NULL;        
		return FALSE;    
	}
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CDownloadDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

