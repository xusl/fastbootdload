#! /bin/sh
                                         
/etc/init.d/vsftp stop > /dev/null 2>&1                                                 
/etc/init.d/minidlna stop > /dev/null 2>&1
sddev=`df | grep '/dev/mmcblk0'| awk '{ if (NR == 1) print $1}'`
local count=0  
while true
do
	mounted=`mount | grep "/ftproot/sdcard" | wc -l`
	if [ $mounted -ge 1 ]; then
		umount -f /ftproot/sdcard
		umount -f /media/card
	else
		break;
	fi
	let count=$count+1
	if [ $count -ge 10 ]; then
		break;
	fi
	usleep 200000
done                

if [ "$1" == "remount" -a -n "$sddev" ]; then
	mount -o rw,relatime,fmask=0000,dmask=0000,iocharset=utf8 $sddev /ftproot/sdcard
	mount -o rw,relatime,fmask=0000,dmask=0000,iocharset=utf8 $sddev /media/card
	usleep 200000
	/etc/init.d/vsftp restart > /dev/null 2>&1
        /etc/init.d/minidlna restart > /dev/null 2>&1
	id=`ps | grep '/sbin/watch_dlna.sh'|grep -v 'grep'`
        [ -z "$id" ] && /sbin/watch_dlna.sh &
fi
