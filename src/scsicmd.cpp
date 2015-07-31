#include "stdAfx.h"
#include "utils.h"
#include "scsicmd.h"
#include "log.h"

CSCSICmd::CSCSICmd()
{
}

CSCSICmd::~CSCSICmd()
{
}

  //In debug mode, adb interface will enumerated.
BOOL CSCSICmd::SwitchToDebugDevice(const WCHAR* devname) {
  UCHAR cmdBuf[CDB6GENERIC_LENGTH] = {0x16, 0xf9, 0x0, 0x0, 0x0, 0x0};
  return Send(devname, cmdBuf, sizeof(cmdBuf));
}

  //switch TPST, USB TPST devcie will enumerated. So ADSU or TPST can update the device.
BOOL CSCSICmd::SwitchToTPSTDeivce(const WCHAR* devname) {
  UCHAR cmdBuf[CDB6GENERIC_LENGTH] = {0x16, 0xf5, 0x0, 0x0, 0x0, 0x0};  
  return Send(devname, cmdBuf, sizeof(cmdBuf));
}

BOOL CSCSICmd::Send(LPCWSTR devname, PUCHAR cmd, size_t cmdLen)
{
	BOOL result = FALSE;
	HANDLE handle = NULL;

	handle = CreateFile(devname,
                      GENERIC_WRITE | GENERIC_READ,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING, 0, NULL);

	if (handle == INVALID_HANDLE_VALUE) {
        ERROR("Open device %S failed.", devname);
		return result;
	}

	result = SendCmd(handle, cmd, cmdLen, 10);

	CloseHandle(handle);

	return result;
}

BOOL CSCSICmd::SendCmd(HANDLE handle, PUCHAR cmd, size_t len, ULONG timeout)
{  BOOL result = FALSE;
  SCSI_PASS_THROUGH_WITH_BUFFERS sptdwb;
  DWORD length = 0;
  DWORD returned = 0;

  ZeroMemory(&sptdwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
  sptdwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
  sptdwb.spt.PathId = 0;
  sptdwb.spt.TargetId = 1;
  sptdwb.spt.Lun = 0;
  sptdwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
  sptdwb.spt.DataTransferLength = 192;
  sptdwb.spt.TimeOutValue = timeout;
  sptdwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
  sptdwb.spt.SenseInfoLength = SPT_SENSE_LENGTH;
  sptdwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
  sptdwb.spt.CdbLength = CDB6GENERIC_LENGTH;
  memcpy(sptdwb.spt.Cdb, cmd, len);
  length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf)
    + sptdwb.spt.DataTransferLength;

  result = DeviceIoControl(handle,
                        IOCTL_SCSI_PASS_THROUGH,
                        &sptdwb,
                        sizeof(SCSI_PASS_THROUGH),
                        &sptdwb,
                        length,
                        &returned,
                        NULL);
  if (result) {
    ERROR("##DeviceIoControl OK!");
  } else {
    ERROR("**DeviceIoControl fails!");
  }

  return result;
}
