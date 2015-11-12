#! /bin/sh

rm /cache/format_status
rm /cache/format_progress
echo "Formatting" > /cache/format_status
echo "1" > /cache/format_progress
sddev=`df | grep '/dev/mmcblk0'| awk '{ if (NR == 1) print $1}'`
if [ -n "$sddev" ]; then
	/usr/oem/umount.sh
else
	sddev=`ls /dev/mmcblk0p[1-9]|awk '{ if (NR == 1) print $1}'`
	if [ -z "$sddev" ]; then
		sddev="/dev/mmcblk0"
	fi
fi
echo "8" > /cache/format_progress
name=`fatlabel $sddev | grep "LABEL=" | awk -F '=' '{ print $2 }' | sed 's/ *$//g'`
echo "22" > /cache/format_progress
mkfs.vfat $sddev
echo "76" > /cache/format_progress
if [ -n "$name" ]; then
	fatlabel $sddev "$name"
fi

echo "92" > /cache/format_progress
mount -o rw,relatime,fmask=0000,dmask=0000,iocharset=utf8 $sddev /ftproot/sdcard
mount -o rw,relatime,fmask=0000,dmask=0000,iocharset=utf8 $sddev /media/card
usleep 200000
echo "98" > /cache/format_progress
item=`mount | grep '/ftproot/sdcard' | grep -v grep`
if [ -z "$item" ]; then
	echo "Error" > /cache/format_status
else
	echo "Formatted" > /cache/format_status
	echo "100" > /cache/format_progress
	/etc/init.d/vsftp restart > /dev/null 2>&1                                              
        /etc/init.d/minidlna restart > /dev/null 2>&1  
	id=`ps | grep '/sbin/watch_dlna.sh'|grep -v 'grep'`
        [ -z "$id" ] && /sbin/watch_dlna.sh & 

	echo 0 > /sys/class/android_usb/android0/enable
	echo 1 > /sys/class/android_usb/android0/enable
fi

