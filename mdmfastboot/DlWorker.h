#pragma once

typedef UINT (*WORKFN)(LPVOID wParam);

class CDownload
{
public:
	WORKFN fn;
	LPVOID wParam;
public:
	CDownload(){ fn = NULL; };
	void DoDownload()
	{
		if (NULL==fn)
		{
			return;
		}
		fn(wParam);
	}
};

class CDlWorker
{
public:
    typedef DWORD_PTR RequestType;

	CDlWorker()
	{
	}

    virtual BOOL Initialize(void *pvParam)
    {
		return TRUE;
    }

    virtual void Terminate(void* pvParam)
    {
	}

	void Execute(RequestType dw, void *pvParam, OVERLAPPED* pOverlapped) throw()
    {
		CDownload* pTask = (CDownload*)(DWORD_PTR)dw;
		pTask->DoDownload();
		delete pTask;
	}
};
