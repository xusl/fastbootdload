#include "mainapp.h"
#include "src/log/log.h"
#include <dbt.h>
#include "src/device/device.h"
#include "src/shiftdevice/shiftdevice.h"
#include "src/device/ziUsbDisk.h"

#ifdef Q_OS_WIN32
static BOOL IsWin8(void)
{
    BOOL blRet = FALSE;
    OSVERSIONINFOEX osvi;

    memset(&osvi, 0, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (GetVersionEx((OSVERSIONINFO *)&osvi))
    {
        //fix bug on win 8.1 do not open the browser add by yin.xiao 2013.10.14 begin
        /*if (osvi.dwMajorVersion == 6L && osvi.dwMinorVersion == 2L &&
            osvi.wProductType == VER_NT_WORKSTATION)
        {
            blRet = TRUE;
        }
        */
        if (osvi.dwMajorVersion == 6L && osvi.dwMinorVersion >= 2L &&
            osvi.wProductType == VER_NT_WORKSTATION)
        {
            INFO(FILE_LINE, "------is win 8----------");
            blRet = TRUE;
        }
        //fix bug on win 8.1 do not open the browser add by yin.xiao 2013.10.14 end
    }

    return blRet;
}

#endif

size_t EnumDisk(char* str)
{
    size_t   szAllDriveStrings   =   GetLogicalDriveStrings(0,NULL);
    char*pDriveStrings   =   new   char[szAllDriveStrings + sizeof("")];

    GetLogicalDriveStringsA(szAllDriveStrings,pDriveStrings);

    memcpy(str,pDriveStrings,szAllDriveStrings   +   sizeof(""));
    delete[]   pDriveStrings;
    return szAllDriveStrings;
}

MainApp::MainApp(int &argc, char **argv):
    QApplication(argc,argv)

{
     QByteArray date = QDateTime::currentDateTime().toString("yyyyMMdd").toLatin1();
     char* logName = "ReliabilityTool_log.txt";//date.data();
    // logName = strcat("ReliabilityTool_",logName);

     StartLogging(logName);

     testPort = 0;

     SwitchCDROM();
}

MainApp::~MainApp()
{

}


bool MainApp::winEventFilter(MSG* msg, long* result)
{
    if (msg->message == WM_DEVICECHANGE)
    {
        DWORD nEventType = msg->wParam;
        DWORD dwDataHdr = msg->lParam;

        if (dwDataHdr == 0&& !IsWin8())
        {
            return false;
        }

        DEV_BROADCAST_HDR* phdr = (DEV_BROADCAST_HDR*)dwDataHdr;
        bool bSrchDevice = false;
        uint16 devtype = DEVTYPE_NONE;
        uint16 devevt  = DEVEVT_UNKNOWN;

       // DEBUG(FILE_LINE,
           // tr("OnDeviceChange, EventType: %1, DeviceType %2").arg(nEventType).arg(phdr->dbch_devicetype).toLocal8Bit());

        if (nEventType == DBT_DEVICEARRIVAL)
        {
            devevt = DEVEVT_ARRIVAL;

            if (phdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                devtype = DEVTYPE_CDROM;
            }
            else if (phdr->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                devtype = DEVTYPE_PORT;
            }

        }
        else if (nEventType == DBT_DEVICEREMOVECOMPLETE)
        {
            devevt = DEVEVT_REMOVECOMPLETE;
            if (phdr->dbch_devicetype == DBT_DEVTYP_PORT)
            {
                devtype = DEVTYPE_PORT;
            }
        }
        else if(IsWin8())
        {
            if(nEventType == DBT_DEVNODES_CHANGED)
            {
                devevt = DEVEVT_REMOVECOMPLETE;
                INFO(FILE_LINE, "in IsWin8,phdr->dbch_devicetype == DBT_DEVTYP_PORT");
                devtype = DEVTYPE_PORT;
                bSrchDevice = true;
            }
        }
        else
        {
            devtype = DEVTYPE_UNKNOWN;
            //emit SignalUpdateStatus(LoadStringEx(IDS_NO_DEVICE));
        }

        #ifdef Q_OS_WIN32
        //INFO(FILE_LINE, tr("devevt:%1, devtype:%2").arg(devevt).arg(devtype).toLocal8Bit());
        if (devtype == DEVTYPE_PORT)
        {
            if (devevt == DEVEVT_ARRIVAL)
            {
                INFO(FILE_LINE, "in winEventFilter,DEVEVT_ARRIVAL");
                EnumDiagPort();
            }
            else
            {
                INFO(FILE_LINE, "in winEventFilter,DEVEVT_REMOVE");
                emit signalDeviceRemove();
            }
        }
        else if (devtype == DEVTYPE_CDROM)
        {
            INFO(FILE_LINE, "in winEventFilter,DEVTYPE_CDROM");
            SwitchCDROM();
        }

        #endif //Q_OS_WIN32

        return true;
    }

    return false;
}

void MainApp::EnumDiagPort()
{
    uint16 ComId[32] = {0};
    uint16 count = 0;
    CDeviceList devlist;
    count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
    testPort = ComId[0];

    if (testPort != 0)
    {
        emit signalDeviceArrive(testPort);
    }
}


void MainApp::SwitchCDROM()
{
    CDeviceList devlist;
    vector<TDevInfoType> vCdromList;
    devlist.GetCdromList(vCdromList);

    // Send cmd to shiftdevice
    vector<TDevInfoType>::iterator iter;
    CShiftDevice shiftdevice;

    for (iter = vCdromList.begin(); iter != vCdromList.end(); ++iter)
    {
        /* switch device mode from CD-ROM to port*/

        if (!shiftdevice.Shift(iter->devpath.c_str()))
        {
            WARN(FILE_LINE, "Fail to send SCSI cmd!");
        }
        else
        {
            DEBUG(FILE_LINE, "Send SCSI cmd successfully!");
        }

        /* switch the first CD-ROM in vector.*/
        break;
    }

    vCdromList.clear();
    //add for MDM9x15
    //send the SCSI cmd to SD card
    SwitchDisk();
}

void MainApp::SwitchDisk()
{
    char strOrgDisk[100];
    memset(strOrgDisk,0,100);
    int sizeOrg = EnumDisk(strOrgDisk);
    int volumeNum = sizeOrg/4;

    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes ;
    ZeroMemory(&freeBytesAvailable, sizeof(ULARGE_INTEGER));
    ZeroMemory(&totalNumberOfBytes, sizeof(ULARGE_INTEGER));
    ZeroMemory(&totalNumberOfFreeBytes, sizeof(ULARGE_INTEGER));

    char volumeLabel[4] = {0};


    for (int i = 0; i < volumeNum; i++)
    {
        memcpy(&volumeLabel, strOrgDisk+(i*4), sizeof(volumeLabel));
        if (GetDriveTypeA(volumeLabel) == DRIVE_REMOVABLE||GetDriveTypeA(volumeLabel) ==DRIVE_CDROM )
        {
            char cUdiskName = volumeLabel[0];

            #ifdef Q_OS_WIN32

            CziUsbDisk usbDisk(cUdiskName);


            BYTE pbCBD[16] = {'\0'};
            BYTE pbBuff[192] = {0};
            memset(pbCBD, 0, 16);

            pbCBD[0] = 0x16;
            //pbCBD[1] = 0xF5;
            pbCBD[1] = 0xF9;

            usbDisk.UDiskIO(pbCBD,12,192,pbBuff,SCSI_IOCTL_DATA_IN);

            #endif

        }
     }
}


