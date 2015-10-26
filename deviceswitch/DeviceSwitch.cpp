// SENDSCSI.cpp :

#include "stdafx.h"
#include "scsicmd.h"
#include <Windows.h>
#include <string>
#include <algorithm>

static std::vector<string> cdromList;

size_t EnumDisk(char* str)
{
	size_t   szAllDriveStrings   =   GetLogicalDriveStrings(0,NULL);
	char   *pDriveStrings   =   new   char[szAllDriveStrings   +   sizeof(_T(""))];
	GetLogicalDriveStrings(szAllDriveStrings,pDriveStrings);
	memcpy(str,pDriveStrings,szAllDriveStrings   +   sizeof(_T("")));
	delete[]   pDriveStrings;
	return szAllDriveStrings;
}


LRESULT  OnInitDevice(VOID) {
  std::vector<string> devicePath;
  GetDeviceByGUID(devicePath, &GUID_DEVINTERFACE_DISK);
  size_t cdromSize = devicePath.size();

  for(size_t i = 0; i < cdromSize; i++)
  {
    string path = devicePath[i];
    if (path.find(_T("\\\\?\\usbstor#")) == -1) {
      //printf("Fix DISK %S:", path);
    } else {
      transform(path.begin(), path.end(), path.begin(), ::toupper);
      //path.MakeUpper();
      if (path.find(_T("ONETOUCH")) == -1 && path.find(_T("ALCATEL")) == -1 &&
        path.find(_T("VEN_AT&T&PROD_MODIO")) == -1) {
        //printf("USB Stor %S is not target device.",path);
      } else {
        CSCSICmd scsi = CSCSICmd();
        printf("AT&T Modio LTE Case detected.");
        //SetTimer(TIMER_INSTALL_ADB_DRIVER, 1000, NULL);
        scsi.SwitchToDebugDevice(path.c_str());
        //m_hDevchangeTips->SetWindowText(_T("Switch Modio USB Ports"));

        //scsi.SwitchToDebugDevice(_T("\\\\?\\H:"));
        return 0;
      }
    }
  }
  devicePath.clear();
  printf("Please connect AT&T Modio LTE Case to computer via USB cable.");

  return 1;
}



int _tmain(int argc, _TCHAR* argv[])
{
  return OnInitDevice();
#if 0
	if (argc < 3)
	{
		if (argc == 1)
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
			char volumeText[26] = {0};
			int j = 0;

			for (int i = 0; i < volumeNum; i++)
			{
				memcpy(&volumeLabel, strOrgDisk+(i*4), sizeof(volumeLabel));
				if (GetDriveType(volumeLabel) == DRIVE_CDROM)
				{
					/* Add by jianwen.he 2011.05.04*/
					//Bug resolved: when the CD-ROM is empty, the error message pops up
					HANDLE hDevice = NULL;
					char szBuf[20] = {0};

					sprintf(szBuf, "\\\\?\\%C:", volumeLabel[0]);

					hDevice = ::CreateFileA(szBuf,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  NULL,
                                  NULL);

					if (hDevice != INVALID_HANDLE_VALUE)
					{
						CloseHandle(hDevice);
					}
					else
					{
						continue;
					}
					/* End 2011.05.04 */

					totalNumberOfBytes.QuadPart = 0;
					GetDiskFreeSpaceEx((LPCTSTR)volumeLabel,
                             &freeBytesAvailable,
                             &totalNumberOfBytes,
                             &totalNumberOfFreeBytes);
					if (totalNumberOfBytes.QuadPart > 1 && totalNumberOfBytes.QuadPart < 1024*1024*100)
					{
						FILE* readFile = NULL;
						char fileName[32] = {0};
						sprintf_s(fileName, "%s\\ModemShow.ini", volumeLabel);
						fopen_s(&readFile,fileName,"r");
						if (readFile == NULL)
							return 0;
						else
							fclose(readFile);

						volumeText[j] = volumeLabel[0];
						j++;
					}
				}
			}

			if (j > 0)
			{
				char cmdBuf[128] = {0};
				sprintf_s(cmdBuf, "open %c: type cdAudio alias cd wait shareable", volumeText[j-1]);
				DWORD dwRet = mciSendString(cmdBuf, NULL, 0, NULL);
				dwRet = mciSendString("set cd door open wait", NULL, 0, NULL);
				return dwRet;
			}

		}
		else if (argc == 2)
		{
			if ((lstrcmp(argv[1], "-h") == 0) || (lstrcmp(argv[1], "-H") == 0))
			{
				return 1;
			}

			if ((lstrcmp(argv[1], "?") == 0) || (lstrcmp(argv[1], "-?") == 0))
			{
				//printf("SWITCH Version: SWW0010000\n");
				printf("SWITCH Version: SWW0010100\n");
				printf("SWITCH usage:\n");
				printf("  SWITCH.exe [param]\n");
				printf("   [param:]\n");
				printf("	 Default: Switch CD-ROM to  Multi-port and Mass storage.\n");
				printf("     -NORMAL: Switch CD-ROM to Multi-port and Mass storage.\n");
				printf("     -DIAG: Switch CD-ROM to Download-port and Mass storage,at the same time the Diag port is enable.\n");
				printf("     -ALL: Switch CD-ROM to Multi-port and Mass storage and CD-ROM, at the same time the Diag port is enable.\n");

				return 1;
			}

			if ((lstrcmp(argv[1], "-normal") == 0) || (lstrcmp(argv[1], "-NORMAL") == 0))
			{
				cdromList.clear();
				EnumCDROM(cdromList);

				uint8 cmdBuf[2] = {0};
				bool bOk = false;
				cmdBuf[0] = 0x06;
				cmdBuf[1] = 0xf5;
				int cdromSize = cdromList.size();
				for(int i = 0; i < cdromSize; i++)
				{
					bOk = Send(cdromList[i].c_str(), cmdBuf, sizeof(cmdBuf));
				}
				cdromList.clear();

				return 1;
			}
			else if ((lstrcmp(argv[1], "-diag") == 0) || (lstrcmp(argv[1], "-DIAG") == 0))
			{
				cdromList.clear();
				EnumCDROM(cdromList);

				uint8 cmdBuf[2] = {0};
				bool bOk = false;
				cmdBuf[0] = 0x16;
				cmdBuf[1] = 0xf5;
				int cdromSize = cdromList.size();
				for(int i = 0; i < cdromSize; i++)
				{
					bOk = Send(cdromList[i].c_str(), cmdBuf, sizeof(cmdBuf));
				}
				cdromList.clear();
				return 1;
			}
			else if ((lstrcmp(argv[1], "-all") == 0) || (lstrcmp(argv[1], "-ALL") == 0))
			{

				cdromList.clear();
				EnumCDROM(cdromList);

				uint8 cmdBuf[2] = {0};
				bool bOk = false;
				cmdBuf[0] = 0x16;
				cmdBuf[1] = 0xf6;
				int cdromSize = cdromList.size();
				for(int i = 0; i < cdromSize; i++)
				{
					bOk = Send(cdromList[i].c_str(), cmdBuf, sizeof(cmdBuf));
				}
				cdromList.clear();
				return 1;
			}
			else
			{
				char cmdBuf[128] = {0};
				sprintf_s(cmdBuf, "open %s type cdAudio alias cd wait shareable", argv[1]);
				DWORD dwRet = mciSendString(cmdBuf, NULL, 0, NULL);
				dwRet = mciSendString("set cd door open wait",NULL,0,NULL);
				//dwRet = mciSendString("set cd door closed wait",NULL,0,NULL);
				return dwRet;
			}
		}

		return 1;
	}
#endif
}

/*	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);
	printf("OS Ver size:%d OS string:%s\n",osvi.dwMajorVersion, osvi.szCSDVersion);
*/


