#pragma once

#include <dbt.h>
#include <usb100.h>
#include <adb_api.h>
#include "usb_adb.h"
#include <initguid.h>
#include "devguid.h"
#include <atlutil.h>
#include <setupapi.h>

BOOL RegisterAdbDeviceNotification(HWND hWnd);
VOID SetUpAdbDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam);
BOOL StopAdbServer();
static const GUID usb_class_id[] = {
	//ANDROID_USB_CLASS_ID, adb and fastboot
	{0xf72fe0d4, 0xcbcb, 0x407d, {0x88, 0x14, 0x9e, 0xd6, 0x73, 0xd0, 0xdd, 0x6b}},
	//usb A5DCBF10-6530-11D2-901F-00C04FB951ED  GUID_DEVINTERFACE_USB_DEVICE
	//{0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}},
};