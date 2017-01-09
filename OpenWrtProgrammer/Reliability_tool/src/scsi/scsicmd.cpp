#include "../define/stdAfx.h"
#include "../utils/utils.h"
#include "scsicmd.h"

#ifdef Q_OS_WIN32

#define SPT_SENSE_LENGTH          (32)
#define SPTWB_DATA_LENGTH         (512)
#define CDB6GENERIC_LENGTH        (6)
#define SCSIOP_MODE_SENSE         (0x1A)
#define MODE_SENSE_RETURN_ALL     (0x3f)


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

typedef struct {
    SCSI_PASS_THROUGH spt;
    ULONG             Filler;      // realign buffers to double word boundary
    uint8             ucSenseBuf[SPT_SENSE_LENGTH];
    uint8             ucDataBuf[SPTWB_DATA_LENGTH];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

CSCSICmd::CSCSICmd()
{
}

CSCSICmd::~CSCSICmd()
{
}

bool CSCSICmd::Send(const WCHAR* devname)
{
    bool bOk = false;
    DWORD accessMode = 0, shareMode = 0;
    HANDLE handle = NULL;
    ULONG length = 0;
    ULONG returned = 0;

    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;  // default
    accessMode = GENERIC_WRITE | GENERIC_READ;		 // default
    handle = CreateFile(    devname,
                            accessMode,
                            shareMode,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL
                            );

    if (handle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(handle);
        return false;
    }

    uint8 cmd5[] = {
        0x16,
        0xF9,
        0x04,
        0x02,
        0x52,
        0x70

    };

    bOk = this->SendCmd(handle, cmd5, COUNTOF(cmd5), 10);

    CloseHandle(handle);

    return bOk;
}

bool CSCSICmd::SendCmd(HANDLE handle, uint8* cmd, uint32 len, uint64 timeout)
{
    bool bOk = false;
    SCSI_PASS_THROUGH_WITH_BUFFERS sptdwb;
    uint64 length = 0;
    uint64 returned = 0;

    memset(&sptdwb, 0, sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
    sptdwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptdwb.spt.PathId = 0;
    sptdwb.spt.TargetId = 1;
    sptdwb.spt.Lun = 0;
    sptdwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptdwb.spt.SenseInfoLength = 32;
    sptdwb.spt.DataIn = 1;
    sptdwb.spt.DataTransferLength = 192;
    sptdwb.spt.TimeOutValue = timeout;
    sptdwb.spt.DataBufferOffset =
            offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    sptdwb.spt.SenseInfoOffset =
            offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);

    for (uint32 i=0; i<len; ++i)
    {
        memcpy(sptdwb.spt.Cdb, cmd, len);
    }
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf)
             + sptdwb.spt.DataTransferLength;

    bOk = DeviceIoControl(  handle,
                            IOCTL_SCSI_PASS_THROUGH,
                            &sptdwb,
                            sizeof(SCSI_PASS_THROUGH),
                            &sptdwb,
                            length,
                            &returned,
                            false
                            );

    return bOk;
}

#endif //Q_OS_WIN32
