/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-02-06  dawang.xu  init first version

=============================================================================*/
#include "StdAfx.h"
#include <objbase.h>
#include <initguid.h>
#include <setupapi.h>
#include <algorithm>
using namespace std;

#include "device.h"
#include "utils.h"
#include "log.h"

#pragma 	  comment(lib,"setupapi.lib")

#define INVALID_PORT_ID (-1)

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

#ifndef GUID_DEVINTERFACE_DISK
DEFINE_GUID(GUID_DEVINTERFACE_DISK, 0x53f56307, 0xb6bf, 0x11d0,
			0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
//DEFINE_GUID(GUID_DEVINTERFACE_DISK, 0x2c7089aa, 0x2e0e, 0x11d1, 0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4);
#endif

char* DEVPATH_CDROM = "USBSTOR\\CDROMUSBMODEMMMC_STORAGE_____2.31";
//char* DEVPATH_APPLICATION1 = "USB\\VID_1BBB&PID_0000&REV_0000&MI_00";
char* DEVPATH_APPLICATION1 = "OPTIONBUS\\GT72V_DBG";

char* DEVPATH_APPLICATION2 = "USB\\VID_1BBB&PID_0000&REV_0000&MI_01";

char* HWID_DIAGNOSTIC[] = {
	"USB\\VID_1BBB&PID_0000&REV_0000",
	"USB\\VID_1BBB&PID_0017&REV_0000",
	"USB\\VID_1BBB&PID_0052&REV_0000",
	"USB\\VID_0B3C&PID_C003&REV_0000",
	"USB\\VID_0B3C&PID_C004&REV_0000",
	"USB\\VID_0B3C&PID_C005&REV_0000",
	"USB\\VID_1BBB&PID_007A&REV_0000",
	"USB\\VID_1BBB&PID_900E&REV_0000",
	"USB\\VID_1BBB&PID_00B6&REV_0000",
	"USB\\VID_1BBB&PID_00B7&REV_0000",
	"OPTIONBUS\\GT72V_DBG",
	"USB\\VID_1BBB&PID_007A&REV_0228",//add by-lsh for MDM9x15
	"USB\\VID_1BBB&PID_007A&REV_0232",
	"USB\\VID_1BBB&PID_011E&REV_0000",
	"USB\\VID_1BBB&PID_007A",//add by-lsh 通用匹配
	"USB\\VID_05C6&PID_9025"//add by-lsh MDM9x25
};

char* HWID_NEMANOSTIC[] = {
	"USB\\VID_1BBB&PID_0000&REV_0000&MI_01",
	"USB\\VID_1BBB&PID_0017&REV_0000&MI_03"
};

char* HWID_CDROM[] = {
    "USBSTOR\\CDROMUSBMODEMMMC_STORAGE_____2.31",
    "USBSTOR\\CDROMALCATEL_MASS_STORAGE____2.31",
	"USBSTOR\\CDROMONETOUCHMOBILEBROADBAND_2.31",//add by-lsh for MDM9x15
	"USBSTOR\\DISKONETOUCHMOBILEBROADBAND_2.31",
	"USBSTOR\\CDROMONETOUCHKINGFISHER______2.31",//add by-lsh for MDM9X15 new devices
	"USBSTOR\\DISKONETOUCHKINGFISHER______2.31",
	"USBSTOR\\CDROMONETOUCHRAINBOW_________2.31",
	"USBSTOR\\DISKONETOUCHRAINBOW_________2.31",
	"USBSTOR\\CDROMONETOUCHSUZUKA__________2.31",
	"USBSTOR\\DISKONETOUCHSUZUKA__________2.31",
	//Y850
	"USBSTOR\\CDROMONETOUCHCONCERT_________2.31",
	"USBSTOR\\DISKONETOUCHCONCERT_________2.31",
	//L850
	"USBSTOR\\CDROMONETOUCHESTORIL_________2.31",
	"USBSTOR\\DISKONETOUCHESTORIL_________2.31",
	//add for Y855
    "USBSTOR\\CDROMONETOUCHMORPHO__________2.31",
	"USBSTOR\\DISKONETOUCHMORPHO__________2.31",
	"USBSTOR\\CDROMALCATEL_Y855____________2.31",
	"USBSTOR\\DISKALCATEL_Y855____________2.31",
	//add for Y858
	"USBSTOR\\CDROMONETOUCHLINK4___________2.31",
	"USBSTOR\\DISKONETOUCHLINK4___________2.31",
	//Y860 AT&T
	"USBSTOR\\DISKAT&T____MODIO___________2.31",
	//add for Y859
	"USBSTOR\\CDROMONETOUCHLINK4_II________2.31",
	"USBSTOR\\DISKONETOUCHLINK4_II________2.31",
	//add for Y856UB
	"USBSTOR\\CDROMONETOUCHRIDE-FI_________2.31",
	"USBSTOR\\DISKONETOUCHRIDE-FI_________2.31"
};//本数组的添加必须是全大写，否则无法匹配到

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
	this->m_DeviceList.reserve(MAX_PORTS);
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
	IN  TPortEnumType type,
	OUT uint16* pComIdList
)
{
	if (pComIdList == NULL)
    {
		return 0;
	}

	/* If there's no device at all, we assume the devices haven't been
	 * enumerated yet, so we'd try a enumeration first.
	 */
	if (this->m_DeviceList.empty())
        {
		this->Enumerate(DEVTYPE_PORT);
	}
	uint16 i = 0;
	for (vector<TDevInfoType>::iterator iter = this->m_DeviceList.begin();
		 iter != this->m_DeviceList.end();
		 ++ iter)
	{
		//if (iter->type == DEVTYPE_PORT) {
		if (!this->PortTypeMatch(type, iter->hwid))
        {
			continue;
		}
		pComIdList[i] = this->ExtractComId(iter->fname.c_str());
		++ i;
		//}
	}
	return i;
}

/*=============================================================================
DESCRIPTION
	Search all the DIAG and NMEA ports, then find the matched composite
	from them.

=============================================================================*/
uint16 CDeviceList::GetCompositeList(vector<TCompositeType>& vCompositeList)
{
	uint16 diagCnt = 0;
	uint16 count = 0;
	uint16 diagPortList[MAX_DEVICES];
	TCompositeType composite;

	/* clear the list first */
	vCompositeList.clear();

	diagCnt = this->GetComPortList(PORTTYPE_DIAG, diagPortList);
	for (int i = 0; i < diagCnt; ++i)
	{
		if (diagPortList[i] == 0)
		{
			continue;
		}
		composite.diag = 0;
		composite.nmea = 0;
		if (this->SrchPortComposite(diagPortList[i], composite))
                {
			vCompositeList.push_back(composite);
			++ count;
		}
	}

	return count;
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

	if (this->m_DeviceList.empty())
        {
		this->Enumerate(DEVTYPE_CDROM);
	}

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

	if (this->m_DeviceList.empty())
        {
		this->Enumerate(DEVTYPE_DISK);//add for MDM9X15//标记
	}

	for (vector<TDevInfoType>::iterator iter2 = this->m_DeviceList.begin();
		 iter2 != this->m_DeviceList.end();
		 ++ iter2)
	{
		if (iter2->type == DEVTYPE_DISK )
                {
			if (!this->DeviceTypeMatch(iter2->type, iter2->hwid))
                        {
				continue;
			}
			cdromList.push_back(*iter2);
			++ i;
		}
	}
	return i;
}

/*=============================================================================
DESCRIPTION
	Get friendly name from the device which has the com id.

DEPENDENCIES

RETURN VALUE
	TRUE:  If ComPortInfo with comid exists
	FALSE: If ComPortInfo with comid not exist

SIDE EFFECT

=============================================================================*/
BOOL CDeviceList::GetComPortFName
(
	IN  uint16 comid,
	OUT char fname[_MAX_PATH]
)
{
	for (vector<TDevInfoType>::iterator iter = this->m_DeviceList.begin();
		 iter != this->m_DeviceList.end();
		 ++ iter)
	{
		if (iter->type != DEVTYPE_PORT)
                {
			continue;
		}

		if (this->ExtractComId(iter->fname.c_str()) == comid)
                {
			memcpy(&fname[0], &iter->fname[0], _MAX_PATH);
			return TRUE;
		}
	}
	return FALSE;
}

/*=============================================================================
DESCRIPTION
	Remove devices from the list according to type.

DEPENDENCIES

RETURN VALUE
	None

SIDE EFFECT
	Device list will be updated.

=============================================================================*/
void CDeviceList::Clear(TDeviceEnumType type)
{
	vector<TDevInfoType>& v = this->m_DeviceList;

	if (v.empty())
	{
		//this->Enumerate(type);
		return;
	}

	switch (type)
	{
	case DEVTYPE_CDROM:
		v.erase(remove_if(v.begin(), v.end(), IsCdrom), v.end());
		break;

	case DEVTYPE_PORT:
		v.erase(remove_if(v.begin(), v.end(), IsPort), v.end());
		break;

	case DEVTYPE_ALL:
		this->m_DeviceList.clear();
		break;

	default:
		break;
	}
}

/*=============================================================================
DESCRIPTION
	Enumerate devices according to requested type and update device list.

DEPENDENCIES

RETURN VALUE
	TRUE:  The devices have been enumerated successfully.
	FALSE: Failed to enumerate the devices.

SIDE EFFECT
	Device list will be updated. But if there's something wrong during
	the enumeration, device list will be cleared.

=============================================================================*/
BOOL CDeviceList::Enumerate
(
	IN  TDeviceEnumType type
	//,OUT vector<TDevInfoType> &devicelist
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
		guidDev = (LPGUID)&CDROM_GUID;
		break;

	case DEVTYPE_PORT:
		//guidDev = (LPGUID)&GUID_DEVCLASS_PORTS;
		guidDev = (LPGUID)&GUID_CLASS_COMPORT;
		break;

	case DEVTYPE_DISK://add for MDM9X15
		guidDev = (LPGUID)&GUID_DEVINTERFACE_DISK;
		break;

	case DEVTYPE_ALL:
	default:
		break;
	}

	/* Request device type is not supported! */
	if (guidDev == NULL)
    {
		return FALSE;
	}

	hDevInfo = SetupDiGetClassDevs(
					guidDev,
					NULL,
					NULL,
					DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
					);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	BOOL bOk = FALSE;
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
			pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)new char[dwDetDataSize];
			pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		}

		bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData, dwDetDataSize, NULL, &devdata);

		if (bOk)
		{
			// Got a path to the device. Try to get some more info.
			wstring strDevPath(pDetData->DevicePath);
			char fname[_MAX_PATH];
			char desc[_MAX_PATH];
			char hwid[_MAX_PATH];

			BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
				hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
				(PBYTE)fname, sizeof(fname), NULL);
			bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
				hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
				(PBYTE)desc, sizeof(desc), NULL);
			bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
				hDevInfo, &devdata, SPDRP_HARDWAREID, NULL,
				(PBYTE)hwid, sizeof(hwid), NULL);

			if (bSuccess)
            {
				if (type == DEVTYPE_PORT || type == DEVTYPE_CDROM || type == DEVTYPE_DISK)
                {
					if (!this->DeviceTypeMatch(type, hwid))
                    {
						RELEASE_ARRAY((void**)&pDetData);
						continue;
					}
				}
				TDevInfoType info;
				info.type = type;
				//info.devpath = strDevPath;
				info.fname = fname;
				info.devdesc = desc;
				info.hwid = hwid;
				this->m_DeviceList.push_back(info);
			}
		}
        else
        {
			this->m_DeviceList.clear();
			return FALSE;
		}

		RELEASE_ARRAY((void**)&pDetData);
	}

	if (hDevInfo != INVALID_HANDLE_VALUE)
        {
		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

    return TRUE;
}

/*=============================================================================
DESCRIPTION
	Extract the com port id from friendly name.

DEPENDENCIES

RETURN VALUE
	Com port id or INVALID_PORT_ID

SIDE EFFECT

=============================================================================*/
uint16 CDeviceList::ExtractComId(const char* fname)
{
#define D_COM_KEYWORD "(COM"
#define D_COM_KEYWORD_SIZE (sizeof(D_COM_KEYWORD)-1)

	int Len = strlen(fname);
	int i;

	for (i = Len-1; i >= 0 && fname[i] != '('; i--); // find '('

	if (strncmp(fname + i, D_COM_KEYWORD, D_COM_KEYWORD_SIZE) == 0) {
		return atoi(fname + i + D_COM_KEYWORD_SIZE);
	}
	return INVALID_PORT_ID;
}

/*=============================================================================
DESCRIPTION
	Extract composite id from the device path.
	For example:
		If the device path is:
		"\\?\usb#vid_1bbb&pid_0000&mi_00#6&2b3bc58e&0&0000#{86e0d1e0-8089-11d0-9ce4-08003e301f73}"
		                                   --------
		                                    8 chars
		This function returns "2b3bc58e"

DEPENDENCIES

RETURN VALUE
	TRUE:  Composite is found
	FALSE: Composite is not found

SIDE EFFECT

=============================================================================*/
string CDeviceList::ExtractCompositeId(string devpath)
{
	char* s = "&0&0000#{86e0d1e0-8089-11d0-9ce4-08003e301f73}";
	string tmp = devpath.substr(0, devpath.length() - strlen(s));
	return tmp.substr(tmp.length()-8);
}

/*=============================================================================
DESCRIPTION
	Check if the hardware id string match the device type.
	If multi-platforms are to be supported in future, more VID&PID should be
	checked here!!!

DEPENDENCIES
	What platforms to support

RETURN VALUE
	TRUE:  Input hardwareid matches the port type
	FALSE: Input hardwareid not matches the port type

SIDE EFFECT

=============================================================================*/
BOOL CDeviceList::DeviceTypeMatch
(
	IN TDeviceEnumType type,
	IN string hwid
)
{
	int i = 0;
	int j = 0;
	transform(hwid.begin(), hwid.end(), hwid.begin(), toupper);
	switch (type)
        {
	case DEVTYPE_CDROM:
		for (i=0; i<COUNTOF(HWID_CDROM); ++i)
		{
            if (hwid.find(HWID_CDROM[i]) != string::npos)
			{
                return TRUE;
            }
        }
		break;

	case DEVTYPE_PORT:
		for (i=0; i<COUNTOF(HWID_DIAGNOSTIC); ++i)
        {
			if (hwid.find(HWID_DIAGNOSTIC[i]) != string::npos)
            {
				return TRUE;
			}
		}
		for (i=0; i<COUNTOF(HWID_NEMANOSTIC); ++i)
        {
			if (hwid.find(HWID_NEMANOSTIC[i]) != string::npos)
            {
				return TRUE;
			}
		}
		break;

	case DEVTYPE_DISK:
	for(j=0; j<2; j++)
	{
		for (i=0; i<COUNTOF(HWID_CDROM); ++i)
		{
            if (hwid.find(HWID_CDROM[i]) != string::npos)
			{
                return TRUE;
            }
        }
	}
		break;

	default:
		break;
	}

	return FALSE;
}

/*=============================================================================
DESCRIPTION
	Check if the hardware id string match the port type. If more platforms need
	to be supported, more VID&PID should be checked here!!!

DEPENDENCIES
	What platforms to support

RETURN VALUE
	TRUE:  Input hardwareid matches the port type
	FALSE: Input hardwareid not matches the port type

SIDE EFFECT

=============================================================================*/
BOOL CDeviceList::PortTypeMatch
(
	IN  TPortEnumType type,
	IN  string hwid
)
{
	int i = 0;
	/* If the hardware id contains a '\', another '\' should be appended to
	 * the first one, since the first '\' will be considered as escape char.
	 */
	transform(hwid.begin(), hwid.end(), hwid.begin(), toupper);
	switch (type) {
	case PORTTYPE_DIAG:
		for (i=0; i<COUNTOF(HWID_DIAGNOSTIC); ++i)
                {
			if (hwid.find(HWID_DIAGNOSTIC[i]) != string::npos)
                        {
			        return TRUE;
		        }
		}
		break;

	case PORTTYPE_NMEA:
		for (i=0; i<COUNTOF(HWID_NEMANOSTIC); ++i)
                {
			if (hwid.find(HWID_NEMANOSTIC[i]) != string::npos)
                        {
				return TRUE;
			}
		}
		break;

	case PORTTYPE_DIAG_NMEA:
		for (i=0; i<COUNTOF(HWID_DIAGNOSTIC); ++i)
                {
			if (hwid.find(HWID_DIAGNOSTIC[i]) != string::npos)
                        {
				return TRUE;
			}
		}
		for (i=0; i<COUNTOF(HWID_NEMANOSTIC); ++i)
                {
			if (hwid.find(HWID_NEMANOSTIC[i]) != string::npos)
                        {
				return TRUE;
			}
		}
		break;

	default:
		break;
	}

	return FALSE;
}

/*=============================================================================
DESCRIPTION
	Add a composite to the list if it doesn't exist, or update NMEA port if
	it has a different value (DIAG same but NMEA different).

DEPENDENCIES

RETURN VALUE
	None

SIDE EFFECT
	Composite list will be updated.

=============================================================================*/
void CDeviceList::AddPortComposite
(
	IN TCompositeType composite
)
{
	BOOL bCompFound = FALSE;

	for (vector<TCompositeType>::iterator iter = this->m_CompositeList.begin();
		 iter != this->m_CompositeList.end();
		 ++ iter)
	{
		/* If the composite has existed, update it if necessary */
		if (iter->diag == composite.diag)
                {
			bCompFound = TRUE;
			if (iter->nmea != composite.nmea)
                        {
				iter->nmea = composite.nmea;
			}
		}
	}

	/* If the composite doesn't exists, add it to the composite list */
	if (!bCompFound)
        {
		this->m_CompositeList.push_back(composite);
	}
}

/*=============================================================================
DESCRIPTION
	Remove the composite from the list.

DEPENDENCIES

RETURN VALUE
	None

SIDE EFFECT
	Composite list will be updated.

=============================================================================*/
void CDeviceList::RemovePortCompsite
(
	IN TCompositeType composite
)
{
	for (vector<TCompositeType>::iterator iter = this->m_CompositeList.begin();
		 iter != this->m_CompositeList.end();
		 ++ iter)
	{
		if (iter->diag == composite.diag && iter->nmea == composite.nmea)
		{
			this->m_CompositeList.erase(iter);
			break;
		}
	}
}

/*=============================================================================
DESCRIPTION
	Search for a composite (DIAG&NMEA) according to the specific com id.

DEPENDENCIES

RETURN VALUE
	TRUE:  If the composite is found
	FALSE: If the composite is not found

SIDE EFFECT

=============================================================================*/
BOOL CDeviceList::SrchPortComposite
(
	IN  uint16 comId,
	OUT TCompositeType& composite // Found DIAG & NMEA composite
)
{
	/* Com Id should not be 0 */
	if (comId == 0)
	{
		composite.diag = 0;
		composite.nmea = 0;
		return FALSE;
	}

	/* Indicate if we find the DIAG/NMEA port */
	BOOL bDiagFound = FALSE;
	BOOL bNmeaFound = FALSE;

	/* Indicate if we find the composite Id */
	BOOL bCompIdFound = FALSE;

	/* Store composite Id of the device with specific com id */
	string compositeId("");

	vector<TDevInfoType>& v = this->m_DeviceList;
	vector<TDevInfoType>::iterator iter;

	/* Search for the compositeId string according to the com id */
	for (iter = v.begin(); iter != v.end(); ++ iter)
    {
		if (this->ExtractComId(iter->fname.c_str()) == comId) {
			compositeId = this->ExtractCompositeId(iter->devpath);
			bCompIdFound = TRUE;
			break;
		}
	}

	/* If no composite Id is found according to the com id, something
	 * must be wrong. This is probably not gonna happen.
	 */
	if (!bCompIdFound)
    {
		composite.diag = 0;
		composite.nmea = 0;
		return FALSE;
	}

	/* We found a composite Id, now we search for DIAG/NMEA ports
	 * according to this hwid.
	 */
	for (iter = v.begin(); iter != v.end(); ++ iter)
    {
		string tmp = this->ExtractCompositeId(iter->devpath);
		if (tmp.compare(compositeId) != 0) { // composite id not match
			continue;
		}
		if (this->PortTypeMatch(PORTTYPE_DIAG, iter->hwid))
        {
			composite.diag = this->ExtractComId(iter->fname.c_str());
			bDiagFound = TRUE;
		}
                else
                {
			composite.nmea = this->ExtractComId(iter->fname.c_str());
			bNmeaFound = TRUE;
		}
		if (bDiagFound && bNmeaFound)
        {
			return TRUE;
		}
	}

	return (bDiagFound && bNmeaFound);
}


