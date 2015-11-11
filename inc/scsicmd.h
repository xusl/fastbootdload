#ifndef __SCSICMD_H__
#define __SCSICMD_H__
#include <windows.h>

#include <devioctl.h>
#include <ntdddisk.h>
#include <ntddscsi.h>
#define _NTSCSI_USER_MODE_
#include <scsi.h>

#define IOCTL_SCSI_BASE					  FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_PASS_THROUGH		CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long      uint64;

#define SPT_CDB_LENGTH 32
#define SPT_SENSE_LENGTH          (32)
#define SPTWB_DATA_LENGTH         (512)

/*
typedef struct _SCSI_PASS_THROUGH {
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    ULONG_PTR DataBufferOffset;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
}SCSI_PASS_THROUGH;
*/
typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH spt;
    ULONG             Filler;      // realign buffers to double word boundary
    UCHAR             ucSenseBuf[SPT_SENSE_LENGTH];
    UCHAR             ucDataBuf[SPTWB_DATA_LENGTH];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;


typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
    SCSI_PASS_THROUGH_DIRECT sptd;
    ULONG             Filler;      // realign buffer to double word boundary
    UCHAR             ucSenseBuf[SPT_SENSE_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;



class CSCSICmd
{
public:
	CSCSICmd();
	~CSCSICmd();

public:
  //In debug mode, adb interface will enumerated.
  //devname : such as "\\\\?\\H:" or "\\\\.\\PhysicalDrive1"
  //refer to DeviceIoControl Function
  BOOL SwitchToDebugDevice(const WCHAR* devname);
  //switch TPST, USB TPST devcie will enumerated. So ADSU or TPST can update the device.
  BOOL SwitchToTPSTDeivce(const WCHAR* devname);
  BOOL SwitchToAdbOnly(const WCHAR* devname);

private:
	BOOL SendCmd(HANDLE handle, PUCHAR cmd, size_t len, ULONG timeout);
	BOOL Send(LPCWSTR devname, PUCHAR cmd, size_t cmdLen);
};

#endif //__SCSICMD_H__

