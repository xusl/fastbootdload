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
#include <atlutil.h>
#include <setupapi.h>

#include <header.h>
using namespace std;

//using std::vector;

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
    DEVICE_PLUGIN,
    DEVICE_CHECK,  //VERSION CHECK BY ADB OR DIAG
    DEVICE_PST,    //DOWNLOAD IMAGE THROUGH DIAG (PRG, SERIAL)
    DEVICE_FLASH,//DOWNLOAD IMAGE THROUGH FASTBOOT
    DEVICE_REBOOT,
    DEVICE_REMOVED,
    DEVICE_MAX
}usb_dev_t;

class DeviceInterfaces;

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
#define CLS_USB         _T("USB")
#define SRV_USBCCGP     _T("usbccgp")
#define SRV_FTDIBUS      _T("FTDIBUS")


//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\USBSTOR
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\Vid_0204&Pid_6025\1020500056180A10


//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\jrdusbser
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\Vid_1bbb&Pid_0196&MI_02\6&1e805b40&1&0002
//Class :Ports,
//ClassGUID: GUID_DEVCLASS_PORTS,  It is different fromGUID_CLASS_COMPORT
//Service:jrdusbser
//PortName: location in
//HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\USB\Vid_1bbb&Pid_0196&MI_02\6&1e805b40&1&0002\Device Parameters
#define SRV_JRDUSBSER    "jrdusbser"
#define SRV_SERIAL       "Serial"

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

    CDevLabel(const char * devPath, const char* usbBus = NULL,
              bool useBus = true, const char * name = NULL);
    CDevLabel(const CDevLabel & dev);
    ~CDevLabel();

    bool operator ==(const CDevLabel & ) const;
    bool Match(const CDevLabel * const &) const;
    bool MatchDevPath(const char * devPath);
    CDevLabel & operator =(const CDevLabel & );
    const char * GetDevPath() const {   return mDevPath;};
    const char * GetParentIdPrefix() const{ return mParentIdPrefix;};
    const char * GetDevId() const{ return mDevId;};
    const char * GetMatchId() const { return mMatchId;};
    bool SetMatchId(const char * matchId);
    bool SetDevId(const char * devId);
    bool SetServiceName(const char * name);
    const char * GetServiceName() const;
    bool SetEffectiveSnPort(long sn, long port);
    bool GetEffectiveSnPort(long *sn, long *port);
    bool SetComPort(const char *portName);
    int GetComPortNum() const{ return mPortNum;};
    VOID FreeBuffer();
    VOID Dump(const char *tag);

  private:
    void CopyDeviceDescPath(const char * devPath, const char* usbBus);

  public:
    char *   mDevPath;
    char *   mParentIdPrefix;

    bool        mUseParentIdPrefix;
    long         mEffectiveSn;
    long         mEffectivePort;

 private:
    char     mDevId[DEV_ID_LEN];
    char     mMatchId[DEV_MATCHID_LEN];
    char     mServiceName[DEV_SERVICE_LEN];
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
  bool MatchDevPath(const char * devPath) const;
  bool Match(const DeviceInterfaces * const & devIntf) const;
  DeviceInterfaces & operator =(const DeviceInterfaces &devIntf);
  CDevLabel* GetActiveIntf() const {  return mActiveIntf; };
  CDevLabel* GetDiagIntf() const {  return mDiag; };
  usb_dev_t GetDeviceStatus() const{ return mDeviceActive;};

  bool GetAttachStatus() const { return mAttachUiPort;};
  VOID SetAttachStatus(bool attached)  { mAttachUiPort = attached;};
  VOID SetDeviceStatus(usb_dev_t status);
  CDevLabel* SetDiagIntf(CDevLabel& intf);
  VOID SetActiveIntf(CDevLabel* intf) {  mActiveIntf = intf; };

  BOOL SetIntf(CDevLabel& dev, TDevType type, BOOL updateActiveIntf=TRUE);
  VOID DeleteMemory(VOID);
  int GetDevId();

  VOID UpdateDevTag();
  const char *GetDevTag() const{  return mTag;};
  //long long GetTimeElapse() { return mEndTimeStamp - mBeginTimeStamp;};
  VOID Dump(const char *tag);
  BOOL Reset();

  private:
    //long long     mBeginTimeStamp;
    //long long     mEndTimeStamp;
    bool       mAttachUiPort;
    char       mTag[DEV_TAG_LEN];


    usb_dev_t    mDeviceActive;  //Device is exactly exist. For when enable fix logic port,
                               //we do not remove DeviceInterfaces object.
    CDevLabel  *mActiveIntf;  //interface which now we operate
    CDevLabel  *mAdb;       //Adb interface, appear in debug configuration
    CDevLabel  *mDiag;      /*Diag interface, appear in TPST configuration, which only have TPST interface
                             * Diag interface, appear in debug configuration
                             * Diag interface, there are none image in flash, got Qualcomm 9008.
                            */


};


class DeviceCoordinator {
  public:
    DeviceCoordinator();
    ~DeviceCoordinator();
    DeviceInterfaces *GetValidDevice();
    BOOL GetDevice(const char *const devPath, DeviceInterfaces** outDevIntf);
    BOOL AddDevice(CDevLabel& dev, TDevType type, DeviceInterfaces** intfs);
    BOOL RemoveDevice(DeviceInterfaces*const & devIntf);
    BOOL IsEmpty();
    BOOL Reset();
    VOID Dump(VOID);

  private:
    list<DeviceInterfaces *>  mDevintfList;
};

BOOL GetDevLabelByGUID(CONST GUID *ClassGuid, PCSTR service, vector<CDevLabel>& labels, bool useParentIdPrefix);
long extract_serial_number(char * sn, char **ppstart =NULL, char **ppend= NULL);
long usb_host_sn(const char* dev_name, char** psn = NULL);
long usb_host_sn_port(const char* dev_name) ;

#endif //__DEVICE_H__
