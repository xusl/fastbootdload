#! /bin/sh
                                         
echo 0 > /sys/class/android_usb/android0/enable
echo 3 > /proc/sys/vm/drop_caches
sync
echo 1 > /sys/class/android_usb/android0/enable
