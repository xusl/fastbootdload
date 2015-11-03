#include "stdafx.h"
#include <windows.h>
#include <strsafe.h>
#include <setupapi.h>
#include "scsicmd.h"

#if 0
#define SPT_SENSE_LENGTH          (32)
#define SPTWB_DATA_LENGTH         (512)
#define CDB6GENERIC_LENGTH        (6)
#define SCSIOP_MODE_SENSE         (0x1A)
#define MODE_SENSE_RETURN_ALL     (0x3f)

#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
	((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
	)

typedef struct {
    SCSI_PASS_THROUGH spt;
    ULONG             Filler;      // realign buffers to double word boundary
    uint8             ucSenseBuf[SPT_SENSE_LENGTH];
    uint8             ucDataBuf[SPTWB_DATA_LENGTH];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

void ErrorExit(LPTSTR lpszFunction)
{

	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,
					0, NULL );

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	printf("Error:%s\n",(LPCTSTR)lpDisplayBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	//ExitProcess(dw);

}

bool Send(const char* devname,const uint8* cmd,uint32 cmdLen)
{
	BOOL bOk = FALSE;
	DWORD accessMode = 0, shareMode = 0;
	HANDLE handle = NULL;
	ULONG length = 0;
	ULONG returned = 0;

	shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;  // default
	accessMode = GENERIC_WRITE | GENERIC_READ;		 // default
	handle = CreateFile(
					devname,
					accessMode,
					shareMode,
					NULL,
					OPEN_EXISTING,
					0,
					NULL
					);

	if (handle == INVALID_HANDLE_VALUE)
	{
		ErrorExit(TEXT("CreateFile"));
		CloseHandle(handle);
		return FALSE;
	}

	bOk = SendCmd(handle, cmd, cmdLen, 10);

	CloseHandle(handle);

	return TRUE;
}

bool SendCmd(HANDLE handle, const uint8* cmd, uint32 len, uint64 timeout)
{
	BOOL bOk = FALSE;
	SCSI_PASS_THROUGH_WITH_BUFFERS sptdwb;
	uint64 length = 0;
	uint64 returned = 0;

	ZeroMemory(&sptdwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptdwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptdwb.spt.PathId = 0;
	sptdwb.spt.TargetId = 1;
	sptdwb.spt.Lun = 0;
	sptdwb.spt.CdbLength = CDB6GENERIC_LENGTH;
	sptdwb.spt.SenseInfoLength = 32;
	sptdwb.spt.DataIn = 1;
	sptdwb.spt.DataTransferLength = 192;
	sptdwb.spt.TimeOutValue = timeout;
	sptdwb.spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
	sptdwb.spt.SenseInfoOffset =
			offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
	for (uint32 i=0; i<len; ++i) {
		memcpy(sptdwb.spt.Cdb, cmd, len);
	}
	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf)
			+ sptdwb.spt.DataTransferLength;

	bOk = DeviceIoControl(
				handle,
				IOCTL_SCSI_PASS_THROUGH,
				&sptdwb,
				sizeof(SCSI_PASS_THROUGH),
				&sptdwb,
				length,
				&returned,
				FALSE
				);
	if (!bOk)
	{
		printf("**DeviceIoControl fails!\n");
	}
	else
	{
		printf("##DeviceIoControl OK!\n");
	}

	return bOk;
}

void EnumCDROM(std::vector<string>& m_Cdroms)
{
	m_Cdroms.clear();
	char szDevDesc[256] = {0};
	GUID guidDev = {0x53f56308L, 0xb6bf, 0x11d0,{0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

	hDevInfo = SetupDiGetClassDevs(&guidDev,
									NULL,
									NULL,
									DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
									);

	if ( INVALID_HANDLE_VALUE == hDevInfo)
		return;

	SP_DEVINFO_DATA devInfoElem;
	devInfoElem.cbSize = sizeof(SP_DEVINFO_DATA);
	SP_DEVICE_INTERFACE_DATA ifcData;
	ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	DWORD dwDetDataSize = 0;

	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guidDev, i, &ifcData); ++ i)
	{
		SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

		// Get buffer size first
		SetupDiGetDeviceInterfaceDetail(
			hDevInfo, &ifcData, NULL, 0, &dwDetDataSize, NULL);

		if (dwDetDataSize != 0)
		{
			pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new char[dwDetDataSize]);
			pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		}

		BOOL bOk = SetupDiGetDeviceInterfaceDetail(
			hDevInfo, &ifcData, pDetData, dwDetDataSize, NULL, &devdata);
		if(bOk)
		{
			char buf[512] = {0};
			memcpy(buf, pDetData->DevicePath, strlen(pDetData->DevicePath));
			m_Cdroms.push_back(buf);
			delete pDetData;
			pDetData = NULL;
		}
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

}
#endif

//\\?\ide#cdromhp_dvd-rom_ts-h353c_____________________h430____#4&32dd3bc3&0&0.1.0#{53f56308-b6bf-11d0-94f2-00a0c91efb8b}
//\\?\ide#diskhitachi_hds721050cla362_________________jp2oa3gh#4&32dd3bc3&0&0.0.0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
//\\?\usbstor#disk&ven_onetouch&prod_mobilebroadband&rev_2.31#7&15b65f8e&0&0bdffe1f22df&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
/* if ClassGuid is NULL, will enumerate also devices. */
BOOL GetDeviceByGUID(std::vector<string>& devicePaths, const GUID *ClassGuid) {
  HDEVINFO hDevInfo = SetupDiGetClassDevs(ClassGuid,
                                          NULL,
                                          NULL,
                                          DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfo == INVALID_HANDLE_VALUE)  {
    printf("SetupDiGetClassDevs return invalid handle.");
    return FALSE;
  }

  devicePaths.clear();

  SP_DEVICE_INTERFACE_DATA ifcData;
  ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, ClassGuid, i, &ifcData); ++ i) {
    DWORD dwDetDataSize = 0;
    SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

    // Get buffer size first
    SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, &dwDetDataSize, NULL);

    SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;
    if (dwDetDataSize == 0) {
      printf("SetupDiGetDeviceInterfaceDetail Get empty data.");
      continue;
    }
    pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new BYTE[dwDetDataSize]);
    memset(pDetData, 0, dwDetDataSize*sizeof(char));
    pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    if(SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData,
                                       dwDetDataSize, NULL, &devdata)) {
      devicePaths.push_back(pDetData->DevicePath);
    }
    delete pDetData;
    pDetData = NULL;
  }

  SetupDiDestroyDeviceInfoList(hDevInfo);
  return TRUE;
}



CSCSICmd::CSCSICmd()
{
}

CSCSICmd::~CSCSICmd()
{
}

  //In debug mode, adb interface will enumerated.
BOOL CSCSICmd::SwitchToDebugDevice(const CHAR* devname) {
  UCHAR cmdBuf[CDB6GENERIC_LENGTH] = {0x16, 0xf9, 0x0, 0x0, 0x0, 0x0};
  return Send(devname, cmdBuf, sizeof(cmdBuf));
}

  //switch TPST, USB TPST devcie will enumerated. So ADSU or TPST can update the device.
BOOL CSCSICmd::SwitchToTPSTDeivce(const CHAR* devname) {
  UCHAR cmdBuf[CDB6GENERIC_LENGTH] = {0x16, 0xf5, 0x0, 0x0, 0x0, 0x0};
  return Send(devname, cmdBuf, sizeof(cmdBuf));
}

BOOL CSCSICmd::Send(LPCSTR devname, PUCHAR cmd, size_t cmdLen)
{
	BOOL result = FALSE;
	HANDLE handle = NULL;

	handle = CreateFile(devname,
                      GENERIC_WRITE | GENERIC_READ,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING, 0, NULL);

	if (handle == INVALID_HANDLE_VALUE) {
        printf("Open device %S failed.", devname);
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
    printf("##DeviceIoControl OK!");
  } else {
    printf("**DeviceIoControl fails!");
  }

  return result;
}
