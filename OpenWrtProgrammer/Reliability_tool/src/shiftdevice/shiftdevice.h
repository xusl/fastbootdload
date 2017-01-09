#ifndef __SHIFTDEVICE_H__
#define __SHIFTDEVICE_H__

#include "../define/stdafx.h"

#ifdef Q_OS_WIN32

#include "../scsi/scsicmd.h"

class CShiftDevice
{
public:
	CShiftDevice();
	~CShiftDevice();

public:
	bool Shift(const WCHAR* devname);

private:
	CSCSICmd m_pScsiCmd;
};

#endif //__SHIFTDEVICE_H__

#endif //Q_OS_WIN32
