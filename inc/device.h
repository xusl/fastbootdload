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
#include <list>
#include <algorithm>
#include <string>
#include <usb100.h>
#include <adb_api.h>
#include "define.h"
using namespace std;

//using std::vector;
//using namespace std;


/* Currently we support 16 devices maxinumly */
const uint16 MAX_DEVICES = /*9*/12;//v3.9.0
/* Each device has a cdrom */
const uint16 MAX_CDROMS  = (MAX_DEVICES);
/* Each device has two port (diag & nmea) */
const uint16 MAX_PORTS   = (MAX_DEVICES * 2);

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

  /// Mask for determining when to use zero length packets
  unsigned zero_mask;
};


//device Class GUID
//location:
//HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\USB\Vid_1bbb&Pid_0196&MI_03\6&1e805b40&1&0003
//device Interface GUID
//location:
//HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\USB\Vid_1bbb&Pid_0196&MI_03\6&1e805b40&1&0003\Device Parameters
//item name: DeviceInterfaceGUIDs
//if device has device interface guid same as class guid , this item is not exist.
//{3F966BD9-FA04-4EC5-991C-D326973B5128} classguid, correct to service "WinUSB"
//iterface guid ANDROID_USB_CLASS_ID
#define CLS_ADB         _T("AndroidUsbDeviceClass")
#define SRV_WINUSB      _T("WinUSB")

//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Cdrom
#define SRV_CDROM        _T("cdrom")

//GUID_DEVINTERFACE_DISK
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Disk
#define SRV_DISK        _T("disk")

//HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\USB\Vid_1bbb&Pid_0196\5&10cd67f3&0&3
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\usbccgp
//GUID_DEVCLASS_USB {36FC9E60-C465-11CF-8056-444553540000} , name "USB",
//device interface : GUID_DEVINTERFACE_USB_DEVICE
#define CLS_USB         L"USB"
#define SRV_USBCCGP     L"usbccgp"


//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\USBSTOR
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\Vid_0204&Pid_6025\1020500056180A10


//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\jrdusbser
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\Vid_1bbb&Pid_0196&MI_02\6&1e805b40&1&0002
//Class :Ports,
//ClassGUID: GUID_DEVCLASS_PORTS,  It is different fromGUID_CLASS_COMPORT
//Service:jrdusbser
//PortName: location in
//HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\USB\Vid_1bbb&Pid_0196&MI_02\6&1e805b40&1&0002\Device Parameters
#define SRV_JRDUSBSER    L"jrdusbser"
#define SRV_SERIAL       L"Serial"

typedef enum {
	//DEVTYPE_NONE  = 0,
	//DEVTYPE_CDROM,
	//DEVTYPE_DISK
	DEVTYPE_DIAGPORT,
	DEVTYPE_FASTBOOT,
	DEVTYPE_ADB,
	//DEVTYPE_MAX = 0xFF,
} TDevType;

/*
a usb device path in various mode:
              interface                        composite device
fastboot      5&10cd67f3&0&3                       N/A

debug mode
(adb device)  6&1e805b40&0&4                    5&10cd67f3&0&3
(mass storage)    N/A                           6&1e805b40&1&0005
(diagnostic port) 6&1e805b40&0&4                    N/A

cd-rom        6&21c8898b&0123456789abcdef&1      5&10cd67f3&0&3
*/
#define DEV_ID_LEN        64
#define DEV_TAG_LEN       64
#define DEV_SERVICE_LEN   64
#define DEV_MATCHID_LEN   64
class CDevLabel {
  public:

    CDevLabel(const wchar_t * devPath, const wchar_t* usbBus = NULL,
              bool useBus = true, const wchar_t * name = NULL);
    CDevLabel(const CDevLabel & dev);
    ~CDevLabel();

    bool operator ==(const CDevLabel & ) const;
    bool Match(const CDevLabel * const &) const;
    bool MatchDevPath(const wchar_t * devPath);
    CDevLabel & operator =(const CDevLabel & );
    const wchar_t * GetDevPath() const;
    const wchar_t * GetParentIdPrefix() const;
    const wchar_t * GetDevId() const;
    const wchar_t * GetMatchId() const;
    bool SetMatchId(const wchar_t * matchId);
    bool SetDevId(const wchar_t * devId);
    bool SetServiceName(const wchar_t * name);
    const wchar_t * GetServiceName() const;
    bool SetEffectiveSnPort(long sn, long port);
    bool GetEffectiveSnPort(long *sn, long *port);
    bool SetComPort(const wchar_t *portName);
    int GetComPortNum() const;
    VOID FreeBuffer();

  private:
    void CopyDeviceDescPath(const wchar_t * devPath, const wchar_t* usbBus);

  public:
    wchar_t *   mDevPath;
    wchar_t *   mParentIdPrefix;

    bool        mUseParentIdPrefix;
    long         mEffectiveSn;
    long         mEffectivePort;

 private:
    wchar_t     mDevId[DEV_ID_LEN];
    wchar_t     mMatchId[DEV_MATCHID_LEN];
    wchar_t     mServiceName[DEV_SERVICE_LEN];
    int          mPortNum;
};

//CD-ROM->Diag TPST -> Fastboot
//CD-ROM->Diag Debug -> adb -> Fastboot
//Diag 9008 -> Fastboot
//We do not associate the CD-ROM interface.
class DeviceInterfaces {
  public:
  DeviceInterfaces();
  DeviceInterfaces(const DeviceInterfaces & devIntf);
  ~DeviceInterfaces();

  bool operator ==(const DeviceInterfaces * const & devIntf) const;
  bool MatchDevPath(const wchar_t * devPath) const;
  DeviceInterfaces & operator =(const DeviceInterfaces &devIntf);
  CDevLabel* GetActiveIntf() const;
  CDevLabel* GetAdbIntf() const;
  CDevLabel* GetDiagIntf() const;
  CDevLabel* GetFastbootIntf() const;
  BOOL GetDeviceStatus() const;
  VOID SetDeviceStatus(BOOL status);
  CDevLabel* SetFastbootIntf(CDevLabel& intf);
  CDevLabel* SetDiagIntf(CDevLabel& intf);
  CDevLabel* SetAdbIntf(CDevLabel& intf);
  VOID SetActiveIntf(CDevLabel* intf);
  BOOL SetIntf(CDevLabel& dev, TDevType type, BOOL updateActiveIntf=TRUE);
  VOID DeleteMemory(VOID);
  int GetDevId();
  VOID UpdateDevTag();
  const char *GetDevTag() const;

  private:
    char       mTag[DEV_TAG_LEN];
    BOOL       mDeviceActive;  //Device is exactly exist. For when enable fix logic port,
                               //we do not remove DeviceInterfaces object.
    CDevLabel  *mActiveIntf;  //interface which now we operate
    CDevLabel  *mAdb;       //Adb interface, appear in debug configuration
    CDevLabel  *mDiag;      /*Diag interface, appear in TPST configuration, which only have TPST interface
                             * Diag interface, appear in debug configuration
                             * Diag interface, there are none image in flash, got Qualcomm 9008.
                            */

    CDevLabel   *mFastboot;  //Fastboot interface, though it uses adb driver, it alway have a different PID/VID from adb interface
};


class DeviceCoordinator {
  public:
    DeviceCoordinator();
    ~DeviceCoordinator();
    BOOL GetDevice(const wchar_t *const devPath, DeviceInterfaces** outDevIntf);
    BOOL CreateDevice(CDevLabel& dev, TDevType type, DeviceInterfaces** outDevIntf);
    BOOL AddDevice( DeviceInterfaces* const &devIntf) ;
    BOOL RemoveDevice( DeviceInterfaces*const & devIntf) ;

  private:
    list<DeviceInterfaces *>  mDevintfList;
};


long extract_serial_number(wchar_t * sn, wchar_t **ppstart =NULL, wchar_t **ppend= NULL);
long usb_host_sn(const wchar_t* dev_name, wchar_t** psn = NULL);
long usb_host_sn_port(const wchar_t* dev_name) ;

#endif //__DEVICE_H__
