#include "../utils/utils.h"
#include "../scsi/scsicmd.h"
#include "shiftdevice.h"

#ifdef Q_OS_WIN32

CShiftDevice::CShiftDevice()
{
	
}

CShiftDevice::~CShiftDevice()
{
	
}

bool CShiftDevice::Shift(const WCHAR* devname)
{
	return m_pScsiCmd.Send(devname);
}


#endif //Q_OS_WIN32
