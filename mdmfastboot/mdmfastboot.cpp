// mdmfastboot.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "mdmfastbootDlg.h"
#include "log.h"
#include "sysdeps.h"
#include "adb_client.h"
#include "usb_vendors.h"

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

UINT fbeventServerThread(LPVOID pParam) {
#if 1
	char local_name[30];
	int server_port = DEFAULT_ADB_PORT;
		//start adb host server
//	 adb_trace_init();
    adb_sysdeps_init();

	//adb_set_transport(kTransportUsb, NULL);
    //adb_set_tcp_specifics(DEFAULT_ADB_PORT);

	// init_transport_registration(); // INIT fbevent.
	//adb_main(is_daemon, server_port);

	usb_vendors_init();
    //usb_init();
    //local_init(DEFAULT_ADB_LOCAL_TRANSPORT_PORT);

   // build_local_name(local_name, sizeof(local_name), server_port);
   // if(install_listener(local_name, "*smartsocket*", NULL)) {
   //     exit(1);
   // }

	 //start_logging();

    //fdevent_loop();

    usb_cleanup();
#endif
	return TRUE;
}

UINT adb_init() {

	//adb_connect("host:start-server");
	AfxBeginThread(fbeventServerThread, (void*)NULL);
	return TRUE;
}

// CmdmfastbootApp ��ʼ��

BOOL CmdmfastbootApp::InitInstance()
{
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

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("TCL MBB MODULE FASTBOOT"));

	StartLogging();//LogFile();
	//adb_init() ;
	adb_sysdeps_init();

	CmdmfastbootDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
		usb_cleanup(); //�˳�usb poll thread
		StopLogging();
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

