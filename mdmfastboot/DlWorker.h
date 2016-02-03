#pragma once

//#include "mdmfastbootDlg.h"
class UsbWorkData;
typedef UINT (*WORKFN)(UsbWorkData * usbdata, flash_image  *image);

class CDownload
{
public:
  WORKFN work;
  UsbWorkData * data;
  flash_image  *image;

public:
  CDownload(): work(NULL){
  };

  CDownload(WORKFN wf, UsbWorkData * wParam, flash_image  *img):
    work(wf), data(wParam), image(img)
  {
  };

  void DoDownload()
  {
    if (NULL==work)
    {
      return;
    }
    work(data, image);
  }
};

class CDlWorker
{
public:
  typedef DWORD_PTR RequestType;

  CDlWorker() {
  }

  virtual BOOL Initialize(void *pvParam) {
    return TRUE;
  }

  virtual void Terminate(void* pvParam) {
  }

  void Execute(RequestType dw, void *pvParam, OVERLAPPED* pOverlapped) throw() {
    CDownload* pTask = (CDownload*)(DWORD_PTR)dw;
    pTask->DoDownload();
    delete pTask;
  }
};
