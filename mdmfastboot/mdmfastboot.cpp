// mdmfastboot.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "mdmfastbootDlg.h"
#include "MmiTestDialog.h"
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmdmfastbootApp

BEGIN_MESSAGE_MAP(CmdmfastbootApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CmdmfastbootApp ����

CmdmfastbootApp::CmdmfastbootApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CmdmfastbootApp ����

CmdmfastbootApp theApp;


// CmdmfastbootApp ��ʼ��

BOOL CmdmfastbootApp::InitInstance()
{
	//for single instance
	SetLastError(0);
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, JRD_MDM_FASTBOOT_TOOL_APP);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hSem);
		// Ѱ����ǰʵ����������
		HWND hWndPrevious = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
		while (::IsWindow(hWndPrevious))
		{
			// ��鴰���Ƿ���Ԥ��ı��?
			// �У���������Ѱ�ҵ�����
			if (::GetProp(hWndPrevious, JRD_MDM_FASTBOOT_TOOL_APP))
			{
				// ����������С������ָ����С
				if (::IsIconic(hWndPrevious))
					::ShowWindow(hWndPrevious, SW_RESTORE);
				// ����������
				::SetForegroundWindow(hWndPrevious);
				// �������ĶԻ��򼤻�
				::SetForegroundWindow(::GetLastActivePopup(hWndPrevious));
				// �˳���ʵ��
				return FALSE;
			}

			// ����Ѱ����һ������
			hWndPrevious = ::GetWindow(hWndPrevious, GW_HWNDNEXT);
		}
		return FALSE;
	}
	//for single instance end

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

/*
    �����MFC��CSocket������Ҫ����AfxSocketInit()��ʼ����
    һ�㶼�Ƿ���App���InitInstance()�����У�
    ������ô�SOCKET��API������
    ����Ҫ����WSAStartup()����ȥ��ʼ�����绷����

    һ����˵ ,�ڵ����κ�winsock api֮ǰ,�������wsastartup()���г�ʼ��,������WSACleanup()����������

        MFC�еĺ��� AfxSocketInit() ��װ�˺��� WSAStartup(), ��֧��WinSock��Ӧ�ó���ĳ�ʼ������IninInstance()�е���AfxSocketInit()���г�ʼ��, �����򲻱ص���WSACleanUp().

    AfxSocketTerm
    */
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("TCL MBB MODULE FASTBOOT"));

//#define MMI_TOOL
#ifdef MMI_TOOL
	MmiTestDialog dlg;
#else
	CmdmfastbootDlg dlg;
#endif
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���

	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

