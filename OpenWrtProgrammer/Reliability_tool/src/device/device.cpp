/*=============================================================================
					   *DEVICE MANAGER IMPLEMENTATION*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-08-30  jianwen.he      Init first version

=============================================================================*/

#include "device.h"

#ifdef Q_OS_WIN32

#include <objbase.h>
//#include "atlbase.h"
#include <initguid.h>
#include <setupapi.h>
#include <algorithm>
#include "../utils/utils.h"


using namespace std;

#pragma 	  comment(lib,"setupapi.lib")

#define INVALID_PORT_ID (-1)
// begin_wioctlguids

DEFINE_GUID(GUID_DEVINTERFACE_CDROM, 0x53f56308L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
DEFINE_GUID(GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, 0x4D36E978L, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);
DEFINE_GUID(GUID_DEVINTERFACE_MODEM, 0x2c7089aa, 0x2e0e, 0x11d1, 0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4);
DEFINE_GUID(GUID_DEVINTERFACE_DISK, 0x53f56307, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);


#if 0
/* The following define is from ntddser.h in the DDK.
 * GUID_DEVCLASS_CDROM & GUID_DEVCLASS_PORTS which are defined in devguid.h
 * seem don't work here, I'm not sure why. So we use the definition from
 * ntddser.h, as the following shows:
*/
#ifndef CDROM_GUID
DEFINE_GUID(CDROM_GUID, 0x53f56308L, 0xb6bf, 0x11d0,
			0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#endif

#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0,
			0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif
#endif

//char* HWID_DIAG[] = {
//	"USB\\VID_1BBB&PID_0017&REV_0000"	
//};

char* HWID_NMEA[] = {
        "USB\\VID_1BBB&PID_0000&REV_0000&MI_01"
};

char* HWID_MODEM[] = {
        "USB\\VID_1BBB&PID_0000&REV_0000&MI_03",
        "USB\\VID_1BBB&PID_0017&REV_0000&MI_05"
};

char* HWID_CDROM[] = {
        "USBSTOR\\CDROMUSBMODEMMMC_STORAGE_____2.31",
        "USBSTOR\\CDROMALCATEL_MASS_STORAGE____2.31",
        //add by jie.li for MDM9x15
        "USBSTOR\\CDROMONETOUCHMOBILEBROADBAND_2.31",
        "USBSTOR\\DISKONETOUCHMOBILEBROADBAND_2.31",
        //end add
        //add by yanbin.wan 20121219 for X600
        "USBSTOR\\CDROMONETOUCHQUICKSTART______2.31",
        //end add
        "USBSTOR\\CDROMONETOUCHKINGFISHER______2.31",
        "USBSTOR\\DISKONETOUCHKINGFISHER______2.31",
        "USBSTOR\\CDROMONETOUCHRAINBOW_________2.31",
        "USBSTOR\\DISKONETOUCHRAINBOW_________2.31",
        "USBSTOR\\CDROMONETOUCHSUZUKA__________2.31",
        "USBSTOR\\DISKONETOUCHSUZUKA__________2.31",
        "USBSTOR\\CDROMONETOUCHCONCERT_________2.31",
        "USBSTOR\\CDROMONETOUCHESTORIL_________2.31"
};

/*=============================================================================
*								F u n c t o r s                               *
*                                                                             *
* These functions are mainly for STL containers and algorithms use.           *
*																			  *
=============================================================================*/
bool IsCdrom(TDevInfoType info)
{
	if (info.type == DEVTYPE_CDROM) 
	{
		return true;
	} 
	else 
	{
		return false;
	}
}

bool IsPort(TDevInfoType info)
{
	if (info.type == DEVTYPE_PORT) 
	{
		return true;
	} 
	else 
	{
		return false;
	}
}


/*=============================================================================
DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECT

=============================================================================*/
CDeviceList::CDeviceList()
{
	m_supportDevice.clear();	
        m_supportDevice.push_back("USB\\VID_1BBB&Pid_0196&Rev_0310&MI_02");
        m_supportDevice.push_back("USB\\VID_1BBB&Pid_0196&MI_02");

	m_supportDevice.push_back("USB\\VID_1BBB&PID_0017&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_0000&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_0B3C&PID_C003&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_0052&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_05C6&PID_9000&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_05C6&PID_9002&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_0017&REV_0000");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_0000&REV_0000");
	m_supportDevice.push_back("USB\\VID_0B3C&PID_C003&REV_0000");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_0052&REV_0000");
	m_supportDevice.push_back("USB\\VID_05C6&PID_9000&REV_0000");
	m_supportDevice.push_back("USB\\VID_05C6&PID_9002&REV_0000");
	m_supportDevice.push_back("USB\\VID_05C6&PID_900E&REV_0000");
	m_supportDevice.push_back("USB\\VID_0B3C&PID_C004&REV_0000");
	m_supportDevice.push_back("USB\\VID_0B3C&PID_C004&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_0B3C&PID_C005&REV_0000");
	m_supportDevice.push_back("USB\\VID_0B3C&PID_C005&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_05C6&PID_9001&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_900E&REV_0000");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_00B6&REV_0000&MI_00");
	m_supportDevice.push_back("USB\\VID_1BBB&PID_00B7&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_0B3C&PID_C007&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_011E&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_1052&PID_E000&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_0B3C&PID_C009&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_0B3C&PID_C00A&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_1E89&PID_2916&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_05C6&PID_9025&REV_0228&MI_00");
   // m_supportDevice.push_back("USB\\VID_1BBB&PID_007A&REV_0228");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0197&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_0B3C&PID_C00B&REV_0000&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_007A&REV_0232");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0197&REV_0232&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_007A&REV_0000");

    //add by jie.li for x715
    m_supportDevice.push_back("USB\\VID_1BBB&PID_01AA&REV_0232");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_01AA&REV_0232&MI_03");
	//end add
    //add by yanbin.wan for M800
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0192&REV_0232");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0192&REV_0232&MI_00");

    m_supportDevice.push_back("USB\\VID_1BBB&PID_0196&REV_0228&MI_02");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0196&REV_0242&MI_02");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0196&MI_02");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0197&REV_????&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0197&MI_00");

    m_supportDevice.push_back("USB\\VID_1BBB&PID_0197&REV_0242&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0197&REV_0332&MI_00");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_007A&REV_0242");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_007A&REV_0332");


    m_supportDevice.push_back("USB\\VID_1BBB&PID_007A&REV_????");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_007A");
    m_supportDevice.push_back("USB\\VID_1BBB&PID_0196&REV_0232&MI_02");

    //end add
    m_iSupportNums = 51;//m_supportDevice.;

}

/*=============================================================================
DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECT

=============================================================================*/
CDeviceList::~CDeviceList()
{
	
}


/*=============================================================================
DESCRIPTION
	Get all devices' com port.

DEPENDENCIES
	Enumerate has been called before.

RETURN VALUE
	Number of the com port (type)

SIDE EFFECT

=============================================================================*/
uint16 CDeviceList::GetComPortList
(
    TPortEnumType type,
    uint16* pComIdList
)
{	
	if (pComIdList == NULL) 
	{
		return 0;
	}

	uint16 i = 0;
	

	this->Enumerate(DEVTYPE_PORT);	

	for (vector<TDevInfoType>::iterator iter = this->m_DeviceList.begin();
		 iter != this->m_DeviceList.end();
		 ++ iter)
	{
		if (!this->PortTypeMatch(type, iter->hwid)) 
		{
			continue;
		}
		pComIdList[i] = this->ExtractPortId(iter->pname.c_str());
		++ i;
	}

	return i;
}

/*=============================================================================
DESCRIPTION
	Extract port id from port name.

DEPENDENCIES

RETURN VALUE
	Port id or INVALID_PORT_ID

SIDE EFFECT

=============================================================================*/

uint16 CDeviceList::ExtractPortId(const wchar_t* pname)
{	
	uint32 port = INVALID_PORT_ID;  
    QString strPort = "";
    strPort = strPort.fromStdWString(pname);
    strPort = strPort.toUpper();
    strPort.remove(0, 3);
    port = strPort.toInt();

	return port;
}


/*=============================================================================
DESCRIPTION
	Get all devices' com port.

DEPENDENCIES
	Enumerate has been called before.

RETURN VALUE
	Number of the cdrom devices

SIDE EFFECT

=============================================================================*/
uint16 CDeviceList::GetCdromList(vector<TDevInfoType>& cdromList)
{
	uint16 i = 0;

	this->Enumerate(DEVTYPE_CDROM);

	for (vector<TDevInfoType>::iterator iter = this->m_DeviceList.begin();
		 iter != this->m_DeviceList.end();
		 ++ iter)
	{
		if (iter->type == DEVTYPE_CDROM)
		{
			if (!this->DeviceTypeMatch(iter->type, iter->hwid)) 
			{
				continue;
			}
			cdromList.push_back(*iter);
			++ i;
		}
	}
	return i;
}

//add by jie.li for MDM9x15
uint16 CDeviceList::GetDiskList(vector<TDevInfoType>& cdromList)
{
	uint16 i = 0;

	this->Enumerate(DEVTYPE_DISK);

	for (vector<TDevInfoType>::iterator iter = this->m_DeviceList.begin();
		iter != this->m_DeviceList.end();
		++ iter)
	{
		if (iter->type == DEVTYPE_DISK)
		{
			if (!this->DeviceTypeMatch(iter->type, iter->hwid)) 
			{
				continue;
			}
			cdromList.push_back(*iter);
			++ i;
		}
	}
	return i;
}
//end add
/*=============================================================================
DESCRIPTION
	Enumerate devices according to requested type and update device list.

DEPENDENCIES

RETURN VALUE
	true:  The devices have been enumerated successfully.
	false: Failed to enumerate the devices.

SIDE EFFECT
	Device list will be updated. But if there's something wrong during
	the enumeration, device list will be cleared.

=============================================================================*/
bool CDeviceList::Enumerate
(
	  TDeviceEnumType type
	//, vector<TDevInfoType> &devicelist
)
{
	GUID* guidDev = NULL;
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	this->m_DeviceList.clear();
	
	switch (type) 
	{
	case DEVTYPE_CDROM:
		//guidDev = (LPGUID)&GUID_DEVCLASS_CDROM;
		guidDev = (LPGUID)&GUID_DEVINTERFACE_CDROM;
		break;

	case DEVTYPE_PORT:
		//guidDev = (LPGUID)&GUID_DEVCLASS_PORTS;
		guidDev = (LPGUID)&GUID_DEVINTERFACE_COMPORT;
		break;

	case DEVTYPE_MODEM:
		guidDev = (LPGUID)&GUID_DEVINTERFACE_MODEM;
		break;

	//add by jie.li for MDM9x15
	case DEVTYPE_DISK:
		guidDev = (LPGUID)&GUID_DEVINTERFACE_DISK;
		break;
	//end add

	case DEVTYPE_ALL:
	default:
		break;
	}

	/* Request device type is not supported! */
	if (guidDev == NULL) 
	{
		return false;
	}

    hDevInfo = SetupDiGetClassDevs( guidDev,
                                    NULL,
                                    NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                                    );
	if (hDevInfo == INVALID_HANDLE_VALUE) 
	{
		return false;
	}
	

	bool bOk = false;
	SP_DEVICE_INTERFACE_DATA ifcData;
	ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	DWORD dwDetDataSize = 0;

	for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, i, &ifcData); ++ i) 
	{
		// Got a device. Get the details.
		SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

		// Get buffer size first
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, &dwDetDataSize, NULL);

		if (dwDetDataSize != 0)
		{
			pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new char[dwDetDataSize]);
			if ((pDetData) != NULL)
			{
				memset(pDetData, 0, dwDetDataSize*sizeof(char));
			}

			//NEW_ARRAY_CAST(pDetData, char, dwDetDataSize, SP_DEVICE_INTERFACE_DETAIL_DATA);
			pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		}

		bOk = SetupDiGetDeviceInterfaceDetail( hDevInfo, 
                                               &ifcData,
                                               pDetData,
                                               dwDetDataSize,
                                               NULL,
                                               &devdata);

		if (bOk) 
		{
			// Got a path to the device. Try to get some more info.           
            wstring deviePath = pDetData->DevicePath;
			
            wchar_t fname[_MAX_PATH];
            wchar_t desc[_MAX_PATH];
            wchar_t hwid[_MAX_PATH];
            memset(fname, 0, sizeof(wchar_t)*_MAX_PATH);
            memset(desc, 0, sizeof(wchar_t)*_MAX_PATH);
            memset(hwid, 0, sizeof(wchar_t)*_MAX_PATH);

			bool bSuccess = SetupDiGetDeviceRegistryProperty( hDevInfo, 
                                                              &devdata,
                                                              SPDRP_FRIENDLYNAME,
                                                              NULL,
                                                              (PBYTE)fname,
                                                              sizeof(fname),
                                                              NULL);

			bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty( hDevInfo, 
                                                                     &devdata,
                                                                     SPDRP_DEVICEDESC,
                                                                     NULL,
                                                                     (PBYTE)desc,
                                                                     sizeof(desc),
                                                                     NULL);

			bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty( hDevInfo, 
                                                                     &devdata,
                                                                     SPDRP_HARDWAREID,
                                                                     NULL,
                                                                     (PBYTE)hwid,
                                                                     sizeof(hwid),
                                                                     NULL);

			if (bSuccess) 
			{
				if (type == DEVTYPE_PORT 
					|| type == DEVTYPE_CDROM
					|| type == DEVTYPE_MODEM
					|| type == DEVTYPE_DISK)	//add by jie.li for MDM9x15 
				{
					if (!this->DeviceTypeMatch(type, hwid)) 
					{						
						RELEASE_ARRAY(&pDetData);						
						continue;
					}
				}

				TDevInfoType* info = new TDevInfoType;
				memset(info, 0, sizeof(info));
				info->type = type;
				info->devpath = deviePath;
				info->fname = fname;
				info->devdesc = desc;
                info->hwid = hwid;

                //QString strTemp = QString::fromWCharArray(&deviePath);
//                strTemp.fromWCharArray(deviePath);
                //qDebug()<<QString(strTemp);
				
				if (type == DEVTYPE_PORT || type == DEVTYPE_MODEM) 
				{
					HKEY DevKey = SetupDiOpenDevRegKey(	hDevInfo, 
                                                        &devdata,
                                                        DICS_FLAG_GLOBAL,
                                                        0,
                                                        DIREG_DEV,
                                                        KEY_READ);
					if (DevKey == INVALID_HANDLE_VALUE) 
					{						
						RELEASE_ARRAY(&pDetData);
						RegCloseKey(DevKey);
						continue;
					}

                    wchar_t chPortName[MAX_PATH];
                    memset(chPortName, 0, sizeof(wchar_t)*MAX_PATH);
					DWORD lType = 0, lpcbData = MAX_PATH;			
                    char szValue[256];
                    memset(szValue, 0, sizeof(szValue));
                    LONG lRes = RegQueryValueExA(DevKey, "PortName", NULL, &lType,
                                                 (LPBYTE)&szValue, &lpcbData);

                    if (lRes == ERROR_SUCCESS)
                    {
                        QString str = szValue;
                        info->pname = str.toStdWString();
					}					
					RegCloseKey(DevKey);					
					
				}
				this->m_DeviceList.push_back(*info);
				delete info;
				info = NULL;
				
			}
		} 
		else 
		{
			this->m_DeviceList.clear();
			return false;
		}
		
		RELEASE_ARRAY(&pDetData);
	}

	if (hDevInfo != INVALID_HANDLE_VALUE) 
	{
		bool bOk = SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	
    return true;
}

/*=============================================================================
DESCRIPTION
	Check if the hardware id string match the port type. If more platforms need
	to be supported, more VID&PID should be checked here!!!

DEPENDENCIES
	What platforms to support

RETURN VALUE
	true:  Input hardwareid matches the port type
	false: Input hardwareid not matches the port type

SIDE EFFECT

=============================================================================*/
bool CDeviceList::PortTypeMatch
(
	  TPortEnumType type,
	  wstring hwid
)
{
	int i = 0;
    QString strHwid = "";
    strHwid = strHwid.fromStdWString(hwid);
    strHwid = strHwid.toUpper();
    /* uppercase hardware id */
    //transform(hwid.begin(), hwid.end(), hwid.begin(), toupper);
    QString strTemp = "";
    switch (type)
    {
	case PORTTYPE_DIAG:
		for (i=0; i<m_iSupportNums; ++i) 
		{
            if (strHwid.compare(m_supportDevice[i]) == 0)
				return true;			
		}
		break;

	case PORTTYPE_NMEA:
		for (i=0; i<COUNTOF(HWID_NMEA); ++i) 
        {
            char* str = HWID_NMEA[i];
            if (strcmp(str, strHwid.toLatin1().data()) == 0)
            {
                return true;
            }
		}
		break;

	case PORTTYPE_ALL:
		for (i=0; i<m_iSupportNums; ++i) 
		{
            strTemp = m_supportDevice[i];
            if (strHwid.compare(strTemp) == 0)
			{
				return true;
			}
		}
		for (i=0; i<COUNTOF(HWID_NMEA); ++i) 
		{
            char* str = HWID_NMEA[i];
            if (strcmp(str, strHwid.toLatin1().data()) == 0)
            {
                return true;
            }

		}
		break;

	default:
		break;
	}

	return false;
}

/*=============================================================================
DESCRIPTION
	Check if the hardware id string match the device type.
	If more devices are to be supported in future, more device type should be
	checked here!!!

DEPENDENCIES
	What devices to support

RETURN VALUE
	true:  Input hardwareid matches the port type
	false: Input hardwareid not matches the port type

SIDE EFFECT

=============================================================================*/
bool CDeviceList::DeviceTypeMatch
(
	 TDeviceEnumType type,
	 wstring hwid
)
{
	int i = 0;
	
	/* uppercase hardware id */
    QString strHwid = "";
    strHwid = strHwid.fromStdWString(hwid);
    strHwid = strHwid.toUpper();

    QString strTemp = "";
	switch (type) 
	{
	case DEVTYPE_CDROM:
		for (i=0; i<COUNTOF(HWID_CDROM); ++i) 
        {
            int tt = strcmp(HWID_CDROM[i], strHwid.toLatin1().data());
            if (strcmp(HWID_CDROM[i], strHwid.toLatin1().data()) == 0)
            {
                return true;
            }
		}
		break;

	case DEVTYPE_PORT:		
		for (i=0; i<m_iSupportNums; ++i) 
        {
            strTemp = m_supportDevice[i];
            if (strHwid.compare(strTemp) == 0)
			{
				return true;
			}
		}
		for (i=0; i<COUNTOF(HWID_NMEA); ++i) 
		{
            char* str = HWID_NMEA[i];
            if (strcmp(str, strHwid.toLatin1().data()) == 0)
            {
                return true;
            }
		}
		break;

	case DEVTYPE_MODEM:
		for (i=0; i<COUNTOF(HWID_MODEM); ++i) 
		{
            char* str = HWID_MODEM[i];
            if (strcmp(str, strHwid.toLatin1().data()) == 0)
            {
                return true;
            }
		}

    //add by jie.li for MDM9x15
    case DEVTYPE_DISK:
        for (i=0; i<COUNTOF(HWID_CDROM); ++i)
        {
            if (strcmp(HWID_CDROM[i], strHwid.toLatin1().data()) == 0)
            {
                return true;
            }
        }
        break;
    //end add

	default:
		break;
	}

	return false;
}

#endif //Q_OS_WIN32
