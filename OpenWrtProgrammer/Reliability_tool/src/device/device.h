/*=============================================================================
					   *DEVICE MANAGER DEFINE*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-08-30  jianwen.he      Init first version

=============================================================================*/

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "../define/stdafx.h"

#ifdef Q_OS_WIN32

#include <vector>
#include <string>

using namespace std;

/* Currently we support 4 devices maxinumly */
const uint16 MAX_DEVICES = 4;
/* Each device has a cdrom */
const uint16 MAX_CDROMS  = (MAX_DEVICES);
/* Each device has two port (diag & nmea) */
const uint16 MAX_PORTS   = (MAX_DEVICES * 2);

/* Port Type */
typedef enum 
{
	PORTTYPE_DIAG = 0,
	PORTTYPE_NMEA,
	// ...
	PORTTYPE_ALL,
	PORT_MAX = 0xFF,
} TPortEnumType;

typedef struct 
{
	TPortEnumType   type;              // port type
	bool            occupied;          // if the port is being occupied
	bool            used;              // if the port is in use
        uint16		comid;             // port number
	char            fname[_MAX_PATH];  // friendly name
} TComPortInfoType;

typedef struct 
{
	string          devpath;           // device path
	string          fname;             // friendly name
	string          devdesc;           // device descriptor
	string          hwid;              // hardware id
} TCdromInfoType;

/* Device Type */
typedef enum 
{
	DEVTYPE_NONE  = 0,
	DEVTYPE_CDROM,
	DEVTYPE_PORT,
	DEVTYPE_MODEM,
	DEVTYPE_DISK,		//add by jie.li for MDM9x15
	// ...
	DEVTYPE_ALL,
	DEVTYPE_UNKNOWN,
	DEVTYPE_MAX = 0xFF,
} TDeviceEnumType;

typedef enum 
{
	DEVEVT_UNKNOWN = 0,
	DEVEVT_ARRIVAL,
	DEVEVT_REMOVECOMPLETE,
	//...
	DEVEVT_MAX = 0xFF, // pack to 1 bytes
}TDevChangeEvtEnumType;

typedef struct 
{
	TDeviceEnumType type;			   // device type
	wstring			devpath;		   // device path
	wstring			fname;			   // friendly name
	wstring			devdesc;		   // device descriptor
    wstring			hwid;			   // hardware id
    wstring         pname;             // port name
} TDevInfoType;

typedef struct 
{
	uint16          diag;              // diag port
	uint16          nmea;              // nmea port
} TCompositeType;

/*=============================================================================
	Class provides functions of enumerating devices, including cdrom,
	serial port and so on.
=============================================================================*/
class CDeviceList
{
public:
	CDeviceList();
	virtual ~CDeviceList();

public:
	/* Get valid com port */
	uint16 GetComPortList(TPortEnumType type, uint16* pComIdList);

	/* Get CDROM device list */
	uint16 GetCdromList(vector<TDevInfoType>& cdromList);

    //add by jie.li for MDM9x15
    /* Get DISK device list */
    uint16 GetDiskList(vector<TDevInfoType>& cdromList);
    //end add

    uint16 ExtractPortId(const wchar_t* pname);

	bool   DeviceTypeMatch(TDeviceEnumType type, wstring hwid);

	bool   PortTypeMatch(TPortEnumType type, wstring hwid);

protected:	
	/* Enumerate device list according to type */
	bool   Enumerate(TDeviceEnumType type/*, vector<TDevInfoType> &devicelist*/);
	

private:
	/* Store enumerated devices information */
	vector<TDevInfoType>    m_DeviceList;

	/* Store valid DIAG<->NMEA composites */
	vector<TCompositeType>  m_CompositeList;

	/*Support device table*/
    vector<QString>         m_supportDevice;

	int						m_iSupportNums;	
}; // end class CDeviceList

#endif //__DEVICE_H__

#endif //Q_OS_WIN32
