Requirements
------------

* A rooted android device
* An Android-SDK [5]
* A running android debugging server ("adb devices" oder "adb start-server" - both need to be executed as root)

ATTENTION
----------

If you have a custom recovery manager installed on your device, you *should* create a backup!
If you have none, have a look at Clockworkmod. ;-)


Build your own kernel for your android device
---------------------------------------------

1. You need the kernelsources matching the currently running kernel. For information how do determine your kernel version have a look at `find-kernelsource.md`.
	How do you get the sources for a official google device is described in [3].	

2. You need the config for currently running kernel
	a. Custom images like CyanogenMod have enabled config export through the proc-filesystem. You could obtain it by:
	
		`adb pull /proc/config.gz .`
	
		`gunzip config.gz`
	
		`cp config /path/to/your/kernel/.config`
	
	b. If you run a stock image, have a look at `documentation/kernel-tree.txt`. It lists a set of Android device their kernel trees and the matching default config.

3. Set the `CROSS_COMPILE` variable.

	export CROSS_COMPILE=/path/to/your/toolchain/
	
For the Android prebuilt toolchain this step may look like:

	export CROSS_COMPILE=/<path>/prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/bin/arm-eabi-

4. First, clearify your configuration is up-to-date

	make ARCH=arm oldconfig
	
Set all needed options for SystemTap descriped in [7]

	make ARCH=arm menuconfig
	
5. Don't forget to build the kernel itself ;-)

	make ARCH=arm [-j X]

The compiled kernel image is located in `arch/arm/boot/`.

6. Now you need the current boot image, because it contains the initialramdisk which we will recylce. First, determine the boot partition.
 Newer devices place symlinks with  human readable names in `/dev/`, e.g. the `Samsung Galaxy Nexus` `/dev/block/platform/omap/omap_hsmmc.0/by-name/boot`.
Otherwise you have to determine the kind of flashdriver used and print the partitiontable. Two examples are given below:

	* HTC Dream (aka T-Mobile G1): cat /proc/mtd
		#cat /proc/mtd
		dev:    size   erasesize  name
		mtd0: 00040000 00020000 "misc"
		mtd1: 00500000 00020000 "recovery"
		mtd2: 00280000 00020000 "boot"
		mtd3: 04380000 00020000 "system"
		mtd4: 04380000 00020000 "cache"
		mtd5: 04ac0000 00020000 "userdata"
	
	* HTC Desire Z: cat /proc/emmc
		# cat /proc/emmc
		dev:        size     erasesize name
		mmcblk0p17: 00040000 00000200 "misc"
		mmcblk0p21: 0087f400 00000200 "recovery"
		mmcblk0p22: 00400000 00000200 "boot"
		mmcblk0p25: 22dffe00 00000200 "system"
		mmcblk0p27: 12bffe00 00000200 "cache"
		mmcblk0p26: 442ffe00 00000200 "userdata"
		mmcblk0p28: 014bfe00 00000200 "devlog"
		mmcblk0p29: 00040000 00000200 "pdata"
		
If you have determined the correct boot partition, dump it and copy the dump to your computer:

	adb shell dd if=/dev/<path> of=/sdcard/boot.img
	
	adb pull /sdcard/boot.img

7. Unpack the boot image with the tool `unpackbootimg`. Its sourcecode is located in `tools/bootimg`.

	unpackbootimg boot.img
	
The output may look like this:

$ unpackbootimg ./boot.img
Magic value:ANDROID!
Kernel:         size:3691628            addr:0x80008000
Ramdisk:        size:223281             addr:0x81000000
Second:         size:0                  addr:0x80f00000
Page size:              2048
Name:
Cmd:
Calc file size:3919872          stat size:3919872
Extracting kernel (3691628 bytes)...
Extracting ramdisk (223281 bytes)...
No secondary image found.

Pay attention on the kernel loadaddress. Here it is 0x80008000, which means the baseaddress is 0x80000000. Keep that in mind!
You have to copy the displayed commandline arguments as well.

8. To create the new boot image just run one command and push it to the device:

	mkbootimg --kernel /path/to/kernel/arch/arm/boot/zImage --ramdisk ramdisk.img --cmdline "<look at output of unpackbootimg>" -o new_boot.img

	adb push new_boot.img /sdcard/
	
9. Finally, you have to flash the new boot image. On some firmware images the tool `flash_kernel`, not on aosp images, is available. I prefer to use this tool. Otherwise you must do it with dd.
 Someone may argue to reset the flash with zeros before flashing it:
 
	dd if=/dev/zero of=/dev/<path>


	flash_kernel boot /sdcard/new_boot.img
	
	or
	
	dd if=/sdcard/new_boot.img of=/dev/<path>


Sources
=======

[1] http://wiki.cyanogenmod.org/wiki/Howto:_Install_Kernels
[2] http://android-dls.com/wiki/index.php?title=HOWTO:_Unpack%2C_Edit%2C_and_Re-Pack_Boot_Images
[3] http://source.android.com/source/downloading.html
[4] http://sourceware.org/systemtap/wiki/SystemTapWithSelfBuiltKernel
[5] http://developer.android.com/sdk/index.html
[6] http://wiki.cyanogenmod.org/wiki/Building_Kernel_from_source
