#ifndef __SCSICMD_H__
#define __SCSICMD_H__

#include "../define/stdafx.h"

#ifdef Q_OS_WIN32

#if (WINVER < 0x500)
	typedef unsigned long ULONG_PTR;
#endif

#include <windows.h>


#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE(0x00000004, 0x0401, 0, 0x0001 | 0x0002)

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long      uint64;

class CSCSICmd
{
public:
	CSCSICmd();
	~CSCSICmd();

public:
        bool Send(const WCHAR* devname);

private:
	bool SendCmd(HANDLE handle, uint8* cmd, uint32 len, uint64 timeout);
};

#endif //__SCSICMD_H__

#endif // Q_OS_WIN32
