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
#include <device.h>
#include "usb_vendors.h"

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
