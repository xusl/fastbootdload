/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-02-06  dawang.xu  init first version

=============================================================================*/
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <vector>
#include <string>
#include <usb100.h>
#include <adb_api.h>
#include "define.h"
using namespace std;

/* Currently we support 16 devices maxinumly */
const uint16 MAX_DEVICES = /*9*/12;//v3.9.0
/* Each device has a cdrom */
const uint16 MAX_CDROMS  = (MAX_DEVICES);
/* Each device has two port (diag & nmea) */
const uint16 MAX_PORTS   = (MAX_DEVICES * 2);

/* Port Type */
typedef enum {
	PORTTYPE_DIAG = 0,
	PORTTYPE_NMEA,
	// ...
	PORTTYPE_DIAG_NMEA,
	PORT_MAX = 0xFF,
} TPortEnumType;

typedef struct {
	TPortEnumType   type;              // port type
	BOOL            occupied;          // if the port is being occupied
	BOOL            used;              // if the port is in use
	uint16		    comid;             // port number
	char            fname[_MAX_PATH];  // friendly name
} TComPortInfoType;

typedef struct {
	string          devpath;           // device path
	string          fname;             // friendly name
	string          devdesc;           // device descriptor
	string          hwid;              // hardware id
} TCdromInfoType;

/* Device Type */
typedef enum {
	DEVTYPE_NONE  = 0,
	DEVTYPE_CDROM,
	DEVTYPE_PORT,
	DEVTYPE_DISK,
	// ...
	DEVTYPE_ALL,
	DEVTYPE_UNKNOWN,
	DEVTYPE_MAX = 0xFF,
} TDeviceEnumType;

typedef enum {
	DEVEVT_UNKNOWN = 0,
	DEVEVT_ARRIVAL,
	DEVEVT_REMOVECOMPLETE,
	//...
	DEVEVT_MAX = 0xFF, // pack to 1 bytes
}TDevChangeEvtEnumType;

typedef struct {
	TDeviceEnumType type;			   // device type
	string			devpath;		   // device path
	string			fname;			   // friendly name
	string			devdesc;		   // device descriptor
	string			hwid;			   // hardware id
} TDevInfoType;

typedef struct {
	uint16 port;
	BOOL   used;
} TDiagPortInfoType;

typedef struct {
	uint16 port;
	BOOL   used;
} TNmeaPortInfoType;

typedef struct {
	uint16          diag;
	uint16          nmea;
} TCompositeType;

typedef enum {
    DEVICE_UNKNOW = 0,
    //DEVICE_PLUGIN,
    DEVICE_CHECK,//ADB,
    //DEVICE_SWITCH,
    DEVICE_FLASH,//FASTBOOT,
    DEVICE_CONFIGURE,
    DEVICE_REMOVE,
    DEVICE_MAX
}usb_dev_t;

/** Structure usb_handle describes our connection to the usb device via
  AdbWinApi.dll. This structure is returned from usb_open() routine and
  is expected in each subsequent call that is accessing the device.
*/
struct usb_handle {
  /// Previous entry in the list of opened usb handles
  usb_handle *prev;

  /// Next entry in the list of opened usb handles
  usb_handle *next;

  /// Handle to USB interface
  ADBAPIHANDLE  adb_interface;

  /// Handle to USB read pipe (endpoint)
  ADBAPIHANDLE  adb_read_pipe;

  /// Handle to USB write pipe (endpoint)
  ADBAPIHANDLE  adb_write_pipe;

  /// Interface name
  wchar_t*         interface_name;

  int interface_protocol;
  usb_dev_t status;
  BOOL work;
  long usb_sn;
  long usb_sn_port;
  long dummy_sn;

  /// Mask for determining when to use zero length packets
  unsigned zero_mask;
};

typedef struct adb_device_t {
   adb_device_t *prev;
   adb_device_t *next;
   long adb_sn;  // adb device serial number that allocate by Windows.
   long cd_sn;    // serial number of composite device which adb device in.
   long cd_sn_port;
}adb_device;

typedef struct dev_switch_t {
  dev_switch_t *prev;
  dev_switch_t *next;
  long        usb_sn;
  usb_dev_t   status;
  long        time;
} dev_switch_t;


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
	uint16 GetCompositeList(vector<TCompositeType>& vCompositeList);

	/* Get CDROM device list */
	uint16 GetCdromList(vector<TDevInfoType>& cdromList);

	/* Get friendly name of the com port */
	BOOL   GetComPortFName(uint16 comid, char fname[_MAX_PATH]);

//protected:
	/* Clear device list according to type */
	void   Clear(TDeviceEnumType type);

	/* Enumerate device list according to type */
	BOOL   Enumerate(TDeviceEnumType type/*, vector<TDevInfoType> &devicelist*/);

private:
	uint16 ExtractComId(const char* fname);
	string ExtractCompositeId(string hwid);
	BOOL   DeviceTypeMatch(TDeviceEnumType type, string hwid);
	BOOL   PortTypeMatch(TPortEnumType type, string hwid);

public: // public attrib for test
	void   AddPortComposite(TCompositeType);
	void   RemovePortCompsite(TCompositeType);
	BOOL   SrchPortComposite(uint16 comId, TCompositeType& composite);

private:
	/* Store enumerated devices information */
	vector<TDevInfoType>    m_DeviceList;

	/* Store valid DIAG<->NMEA composites */
	vector<TCompositeType>  m_CompositeList;
}; // end class CDeviceList

#endif //__DEVICE_H__
