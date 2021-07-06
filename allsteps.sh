#!/bin/bash
#sh compile_kernel.sh
echo $#
if [ "$#" -gt "0" ];then
	for i in "$*"; do

		if [ "$i" -eq "1" ];then
			echo "copy Image.gz-dtb to target directory"
			cp ../out/arch/arm64/boot/Image.gz ../boot/ ../boot/ 		
		fi
		
		if [ "$i" -eq "2" ];then
			cd ../boot/
			echo "remake boot.img"
			./mkbootimg --base 0 --pagesize 2048 --kernel_offset 0x00480000 --ramdisk_offset 0x08000000 --second_offset 0x01300000 --tags_offset 0x07a00000 --cmdline 'loglevel=4 page_tracker=on slub_min_objects=12 unmovable_isolate1=2:192M,3:224M,4:256M androidboot.selinux=enforcing' --kernel Image.gz --ramdisk ramdisk.cpio.gz -o newboot.img	
		fi

		if [ "$i" -eq "3" ];then
			cd ../boot/
			echo "reboot to bootloader"
			adb reboot bootloader	
			echo "wait for bootloader status"
			sleep 15	
		fi

		if [ "$i" -eq "4" ];then
			cd ../boot/
			echo "flash newboot.img into device"
			fastboot flash boot newboot.img
			fastboot reboot		
		fi

	done

else
	echo "copy Image.gz-dtb to target directory"
	cp ../out/arch/arm64/boot/Image.gz ../boot/ 
	cd ../boot/
	echo "remake boot.img"
	./mkbootimg --base 0 --pagesize 2048 --kernel_offset 0x00480000 --ramdisk_offset 0x08000000 --second_offset 0x01300000 --tags_offset 0x07a00000 --cmdline 'loglevel=4 page_tracker=on androidboot.selinux=enforcing' --kernel Image.gz --ramdisk ramdisk.cpio.gz -o newboot.img
	echo "reboot to bootloader"
	adb reboot bootloader
	echo "wait for bootloader status"
	sleep 15
	echo "flash newboot.img into device"
	fastboot flash boot newboot.img
	fastboot reboot
fi
