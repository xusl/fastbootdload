// ziUsbDevice.cpp: implementation of the CziUsbDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "ziUsbDevice.h"

#ifdef Q_OS_WIN32

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CziUsbDevice::CziUsbDevice()
{
	m_hDevice = INVALID_HANDLE_VALUE;
}

void CziUsbDevice::CloseDeviceHandle()
{
	if(m_hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}
}

HANDLE CziUsbDevice::GetCommonHandle()
{
	return m_hDevice;
}

CziUsbDevice::~CziUsbDevice()
{
	CloseDeviceHandle();
}

#endif
