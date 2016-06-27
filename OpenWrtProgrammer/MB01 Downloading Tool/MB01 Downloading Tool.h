
// MB01 Downloading Tool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMB01DownloadingToolApp:
// See MB01 Downloading Tool.cpp for the implementation of this class
//

class CMB01DownloadingToolApp : public CWinApp
{
public:
	CMB01DownloadingToolApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	HANDLE m_hMutex;
};

extern CMB01DownloadingToolApp theApp;