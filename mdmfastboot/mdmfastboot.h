// mdmfastboot.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

#define JRD_MDM_FASTBOOT_TOOL_APP L"JRD_MDM_FASTBOOT_TOOL"

// CmdmfastbootApp:
// �йش����ʵ�֣������ mdmfastboot.cpp
//

class CmdmfastbootApp : public CWinApp
{
public:
	CmdmfastbootApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CmdmfastbootApp theApp;