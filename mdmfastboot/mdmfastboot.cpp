// mdmfastboot.cpp : 定义应用程序的类行为。
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


// CmdmfastbootApp 构造

CmdmfastbootApp::CmdmfastbootApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CmdmfastbootApp 对象

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

// CmdmfastbootApp 初始化

BOOL CmdmfastbootApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("TCL MBB MODULE FASTBOOT"));

	StartLogging();//LogFile();
	//adb_init() ;
	adb_sysdeps_init();

	CmdmfastbootDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
		usb_cleanup(); //退出usb poll thread
		StopLogging();
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

