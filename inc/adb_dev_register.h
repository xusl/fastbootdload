#pragma once
/*
* handle window register about usb device.
*/
#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include "usb_adb.h"
#include <initguid.h>
#include "devguid.h"
#include "usbiodef.h"
#include <atlutil.h>
#include <setupapi.h>
#include <vector>
#include <list>
#include <device.h>
#include "usb_vendors.h"
#include <algorithm>

using std::vector;
using namespace std;

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
#define DEV_SERVICE_LEN   64
#define DEV_MATCHID_LEN   64
class CDevLabel {
  public:
    CDevLabel();
    CDevLabel(const wchar_t * name, const wchar_t * devPath, const wchar_t* usbBus, bool useBus);
    CDevLabel(const wchar_t * devPath);
    CDevLabel(const CDevLabel & dev);
    ~CDevLabel();

    bool operator ==(const CDevLabel & ) const;
    bool operator ==(const wchar_t * devPath);
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

  bool operator ==(const DeviceInterfaces & devIntf) const;
  DeviceInterfaces & operator =(const DeviceInterfaces &devIntf);
  CDevLabel* GetActiveIntf() const;
  const CDevLabel& GetAdbIntf() const;
  const CDevLabel& GetDiagIntf() const;
  const CDevLabel& GetFastbootIntf() const;
  VOID SetFastbootIntf(CDevLabel& intf);
  VOID SetDiagIntf(CDevLabel& intf);
  VOID SetAdbIntf(CDevLabel& intf);
  VOID SetActiveIntf(CDevLabel* intf);

  private:
    CDevLabel  *mActiveIntf;  //interface which now we operate
    CDevLabel   mAdb;       //Adb interface, appear in debug configuration
    CDevLabel   mDiag;      /*Diag interface, appear in TPST configuration, which only have TPST interface
                             * Diag interface, appear in debug configuration
                             * Diag interface, there are none image in flash, got Qualcomm 9008.
                            */

    CDevLabel   mFastboot;  //Fastboot interface, though it uses adb driver, it alway have a different PID/VID from adb interface
};

class DeviceCoordinator {
  public:
    DeviceCoordinator();
    ~DeviceCoordinator();
    BOOL GetDevice(const wchar_t *devPath, DeviceInterfaces& outDevIntf);
    BOOL GetDevice(const CDevLabel& dev, DeviceInterfaces& outDevIntf);
    BOOL AddDevice(const DeviceInterfaces& devIntf) ;
    BOOL RemoveDevice(const DeviceInterfaces& devIntf) ;

  private:
    list<DeviceInterfaces>  mDevintfList;
};

void check_regedit_usbflags(usbid_t USBIds[], unsigned count);
BOOL RegisterAdbDeviceNotification(IN HWND hWnd, OUT HDEVNOTIFY *phDeviceNotify = NULL);
BOOL GetDeviceByGUID(std::vector<CString>& devicePaths, const GUID *ClassGuid);
BOOL GetDevLabelByGUID(CONST GUID *ClassGuid, PCWSTR service, vector<CDevLabel>& labels, bool useParentIdPrefix);
VOID SetUpAdbDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);

static const GUID usb_class_id[] = {
	ANDROID_USB_CLASS_ID, //adb and fastboot, Class ID assigned to the device by androidusb.sys

	//{0xf72fe0d4, 0xcbcb, 0x407d, 0x88, 0x14, 0x9e, 0xd6, 0x73, 0xd0, 0xdd, 0x6b},
	//usb A5DCBF10-6530-11D2-901F-00C04FB951ED  GUID_DEVINTERFACE_USB_DEVICE
	//{0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}},
};
DEFINE_GUID(GUID_DEVINTERFACE_ADB, 0xf72fe0d4, 0xcbcb, 0x407d, 0x88, 0x14, 0x9e, 0xd6, 0x73, 0xd0, 0xdd, 0x6b);
//DEFINE_GUID(GUID_DEVINTERFACE_CDROM, 0x53f56308L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
//DEFINE_GUID(GUID_DEVINTERFACE_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
//DEFINE_GUID(GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, 0x4D36E978L, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);
DEFINE_GUID(GUID_DEVINTERFACE_MODEM, 0x2c7089aa, 0x2e0e, 0x11d1, 0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4);
//DEFINE_GUID(GUID_DEVINTERFACE_DISK, 0x53f56307, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);

long get_adb_composite_device_sn(long adb_sn, long *cd_sn, long *cd_sn_port);
int add_adb_device(wchar_t *ccgp, wchar_t *parentId);
void dump_adb_device(void);
void build_port_map(CListCtrl *  port_list) ;

long extract_serial_number(wchar_t * sn, wchar_t **ppstart =NULL, wchar_t **ppend= NULL);
long usb_host_sn(const wchar_t* dev_name, wchar_t** psn = NULL);
long usb_host_sn_port(const wchar_t* dev_name) ;