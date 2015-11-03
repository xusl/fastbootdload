#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <shellapi.h>

#include <mmsystem.h>
#include <atlstr.h>

BOOL IsWow64(void);

//////////////////////////////////////////////////////////////////////////

BOOL Is64bitSystem()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);

    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
        si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR   lpCmdLine,
					 int   nCmdShow)
{
  TCHAR strModulePath[MAX_PATH] = {0};
  TCHAR strInstallPath[MAX_PATH] = {0};
  TCHAR exe[MAX_PATH] = {0};

  GetModuleFileName(NULL, strModulePath, MAX_PATH);
  *_tcsrchr(strModulePath,'\\') = 0;
  _tcscpy(strInstallPath, strModulePath);

  SHELLEXECUTEINFO ShExecInfo = {0};
  ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  ShExecInfo.hwnd = NULL;
  ShExecInfo.lpVerb = NULL;
  if (IsWow64()) {
    _tcscat(strInstallPath, _T("\\ModioLTECase-x64\\"));
  } else {
    _tcscat(strInstallPath, _T("\\ModioLTECase-win32\\"));
  }
  _tcscpy(exe, strInstallPath);
  _tcscat(exe, "ModioLTECase_microSDPatch.exe");
  //ShExecInfo.lpDirectory = NULL;
  ShExecInfo.lpDirectory = strInstallPath;
  ShExecInfo.lpFile = exe;
  ShExecInfo.lpParameters = _T("");
  ShExecInfo.nShow = SW_SHOW;
  ShExecInfo.hInstApp = NULL;
  ShellExecuteEx(&ShExecInfo);
  WaitForSingleObject(ShExecInfo.hProcess,INFINITE);

  return 0;
}

/*
 * Wow64 means Windows-On-Windows64.
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms684139%28v=vs.85%29.aspx
 * WOW64 is the x86 emulator that allows 32-bit Windows-based applications to
 * run seamlessly on 64-bit Windows. WOW64 is provided with the operating
 * system and does not have to be explicitly enabled.
 *
 * So, build this tools under X86 (or win32), do not use X64 build.
 *
 */
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}
#if 0
BOOL IsWow64(void)
{
	UINT unResult = 0;
	int nResult = 0;
	TCHAR szWinSysDir[MAX_PATH+1] ={0};
	TCHAR szKernel32File[MAX_PATH+1+14]={0};
	HINSTANCE hLibKernel32 = NULL;
	BOOL bIsWow64Process = FALSE;

	BOOL (WINAPI *lpIsWow64Process)(HANDLE,PBOOL) = NULL;

	unResult = GetSystemDirectory(szWinSysDir, sizeof(szWinSysDir)/sizeof(TCHAR));
	if (unResult > 0)
	{
		nResult = _stprintf(szKernel32File,_T("%s\\kernel32.dll"),szWinSysDir);
		if(nResult > 0)
		{
			hLibKernel32 = LoadLibrary(szKernel32File);
		}
	}

	if(NULL == hLibKernel32)
	{
		hLibKernel32 = LoadLibrary(_T("kernel32.dll"));
	}

	// Get the Address of Win32 API -- IsWow64Process()
	if(NULL != hLibKernel32)
	{
		lpIsWow64Process = (BOOL (WINAPI *)(HANDLE,PBOOL))
		GetProcAddress(hLibKernel32,("IsWow64Process"));
	}

	if(NULL != lpIsWow64Process )
	{
		// Check whether the 32-bit program is running under WOW64 environment.
		if (!lpIsWow64Process(GetCurrentProcess(),&bIsWow64Process))
		{
			FreeLibrary(hLibKernel32);
			return FALSE;
		}
	}

	if(NULL != hLibKernel32)
	{
		FreeLibrary(hLibKernel32);
	}

	return bIsWow64Process;
}
#endif
