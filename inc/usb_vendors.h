/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __USB_VENDORS_H
#define __USB_VENDORS_H
#ifdef __cplusplus
extern "C" {
#endif

// Google's USB Vendor ID
#define VENDOR_ID_GOOGLE        0x18d1
// Alcatel's USB Vendor ID
#define VENDOR_ID_ALCATEL       0x1bbb
// HTC's USB Vendor ID
#define VENDOR_ID_HTC           0x0bb4
// Samsung's USB Vendor ID
#define VENDOR_ID_SAMSUNG       0x04e8
// Motorola's USB Vendor ID
#define VENDOR_ID_MOTOROLA      0x22b8
// LG's USB Vendor ID
#define VENDOR_ID_LGE           0x1004
// Huawei's USB Vendor ID
#define VENDOR_ID_HUAWEI        0x12D1
// Acer's USB Vendor ID
#define VENDOR_ID_ACER          0x0502
// Sony Ericsson's USB Vendor ID
#define VENDOR_ID_SONY_ERICSSON 0x0FCE
// Foxconn's USB Vendor ID
#define VENDOR_ID_FOXCONN       0x0489
// Dell's USB Vendor ID
#define VENDOR_ID_DELL          0x413c
// Nvidia's USB Vendor ID
#define VENDOR_ID_NVIDIA        0x0955
// Garmin-Asus's USB Vendor ID
#define VENDOR_ID_GARMIN_ASUS   0x091E
// Sharp's USB Vendor ID
#define VENDOR_ID_SHARP         0x04dd
// ZTE's USB Vendor ID
#define VENDOR_ID_ZTE           0x19D2
// Kyocera's USB Vendor ID
#define VENDOR_ID_KYOCERA       0x0482
// Pantech's USB Vendor ID
#define VENDOR_ID_PANTECH       0x10A9
// Qualcomm's USB Vendor ID
#define VENDOR_ID_QUALCOMM      0x05c6
// NEC's USB Vendor ID
#define VENDOR_ID_NEC           0x0409
// Panasonic Mobile Communication's USB Vendor ID
#define VENDOR_ID_PMC           0x04DA
// Toshiba's USB Vendor ID
#define VENDOR_ID_TOSHIBA       0x0930
// SK Telesys's USB Vendor ID
#define VENDOR_ID_SK_TELESYS    0x1F53
// KT Tech's USB Vendor ID
#define VENDOR_ID_KT_TECH       0x2116
// Asus's USB Vendor ID
#define VENDOR_ID_ASUS          0x0b05
// Philips's USB Vendor ID
#define VENDOR_ID_PHILIPS       0x0471


// HTC's USB Vendor ID
#define VENDOR_ID_HTC           0x0bb4
// QUALCOMM's USB Vendor ID
#define VENDOR_ID_QUALCOMM      0x05c6

// products for VENDOR_ID_GOOGLE
#define PRODUCT_ID_SOONER       0xd00d  // Sooner bootloader
#define PRODUCT_ID_SOONER_COMP  0xdeed  // Sooner composite device

// products for VENDOR_ID_HTC
#define PRODUCT_ID_DREAM        0x0c01  // Dream bootloader
#define PRODUCT_ID_DREAM_COMP   0x0c02  // Dream composite device

// products for VENDOR_ID_QUALCOMM
#define PRODUCT_ID_QUALCOMM     0x9018  // Qualcomm bootloader

#define ANDROID_PATH            ".android"
#define ANDROID_ADB_INI         "adb_usb.ini"

typedef struct usbid_t {
    int vid;
    int pid;
} usbid_t;

extern usbid_t   gUSBIds[];
extern unsigned  gUSBIdCount;

void usb_vendors_init(void);
#ifdef __cplusplus
}
#endif
#endif