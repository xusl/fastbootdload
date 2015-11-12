@echo off
set PATH=bin;bin\usb_driver;%PATH%

:switch
set errorlevel=0
@echo Please connect AT^&T Modio LTE Case to computer via USB cable.

pause
deviceswitch.exe
if %errorlevel% equ 1 goto switch

@echo NOTE: You may be prompted to install device drivers, please accept these prompts.
install.exe

@echo Update /usr/sbin/fatlabel
adb push data\fatlabel  /usr/sbin/fatlabel
adb shell chmod 755 /usr/sbin/fatlabel

@echo Update /usr/oem/formatsdcard.sh
adb push data\formatsdcard.sh  /usr/oem/formatsdcard.sh
adb shell chmod 755 /usr/oem/formatsdcard.sh

@echo Update /usr/oem/umount.sh
adb push data\umount.sh  /usr/oem/umount.sh
adb shell chmod 755 /usr/oem/umount.sh

@echo Update /usr/oem/restartusb.sh
adb push data\restartusb.sh  /usr/oem/restartusb.sh
adb shell chmod 755 /usr/oem/restartusb.sh

@echo Update /usr/oem/webs
adb push data\webs  /usr/oem/webs
adb shell chmod 755 /usr/oem/webs

@echo Update /usr/oem/core_app
adb push data\core_app  /usr/oem/core_app
adb shell chmod 755 /usr/oem/core_app

if %errorlevel% equ 0 echo Patch successfully applied!
pause

goto :eof
