// mdmfastboot.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "mdmfastboot.h"
#include "mdmfastbootDlg.h"
#include "log.h"

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


// CmdmfastbootApp 初始化

BOOL CmdmfastbootApp::InitInstance()
{
	//for single instance
	SetLastError(0);
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, JRD_MDM_FASTBOOT_TOOL_APP);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hSem);
		// 寻找先前实例的主窗口
		HWND hWndPrevious = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
		while (::IsWindow(hWndPrevious))
		{
			// 检查窗口是否有预设的标记?
			// 有，则是我们寻找的主窗
			if (::GetProp(hWndPrevious, JRD_MDM_FASTBOOT_TOOL_APP))
			{
				// 主窗口已最小化，则恢复其大小
				if (::IsIconic(hWndPrevious))
					::ShowWindow(hWndPrevious, SW_RESTORE);
				// 将主窗激活
				::SetForegroundWindow(hWndPrevious);
				// 将主窗的对话框激活
				::SetForegroundWindow(::GetLastActivePopup(hWndPrevious));
				// 退出本实例
				return FALSE;
			}

			// 继续寻找下一个窗口
			hWndPrevious = ::GetWindow(hWndPrevious, GW_HWNDNEXT);
		}
		return FALSE;
	}
	//for single instance end

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

	StartLogging();

	CmdmfastbootDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码

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

