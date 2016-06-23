// JN516x Flash Programmer.h : main header file for the JN516X FLASH PROGRAMMER application
//

#if !defined(AFX_LIFESENSORFLASHPROGRAMMER_H__4339449D_0FC6_499D_B60D_FA76C9C46242__INCLUDED_)
#define AFX_LIFESENSORFLASHPROGRAMMER_H__4339449D_0FC6_499D_B60D_FA76C9C46242__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJN516xFlashProgrammerApp:
// See JN516x Flash Programmer.cpp for the implementation of this class
//

class CJN516xFlashProgrammerApp : public CWinApp
{
public:
	CJN516xFlashProgrammerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJN516xFlashProgrammerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJN516xFlashProgrammerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JN516XFLASHPROGRAMMER_H__4339449D_0FC6_499D_B60D_FA76C9C46242__INCLUDED_)
