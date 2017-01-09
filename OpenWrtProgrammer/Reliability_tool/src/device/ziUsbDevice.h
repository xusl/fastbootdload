// ziUsbDevice.h: interface for the CziUsbDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIUSBDEVICE_H__696BF7DA_A123_4C19_86F6_E190FFFAC97A__INCLUDED_)
#define AFX_ZIUSBDEVICE_H__696BF7DA_A123_4C19_86F6_E190FFFAC97A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "ziBasic.h"

#ifdef Q_OS_WIN32

class CziUsbDevice : public CziBasic  
{
public:
	//¹¹Ôìº¯Êý
	CziUsbDevice();
	virtual ~CziUsbDevice();
	
	virtual void CloseDeviceHandle();
	HANDLE GetCommonHandle();
	
protected:
	HANDLE m_hDevice;
	virtual BOOL GetDeviceHandle() = 0;
};

#endif // !defined(AFX_ZIUSBDEVICE_H__696BF7DA_A123_4C19_86F6_E190FFFAC97A__INCLUDED_)

#endif
