#!/bin/bash

if [ $# -lt 2 ]; then
	echo "usage: $0 <skriptname> <device>"
	echo "devices: g1, n1, desirez,qemu,p920,omap"
	exit 1
fi

DEVICE=$2
SCRIPT=$1
MODULE_NAME=${SCRIPT//./}
MODULE_DIR="modules"
SCRIPT_DIR="scripts"

PREFIX="/fs/students/sfb876-a1/android/"
STAP=$PREFIX"/systemtap/installed/bin/stap"
TOOLCHAIN=$PREFIX"/roms/android_origin/prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/bin/arm-eabi-"
TAPSETS=$PREFIX"/systemtap/installed/share/systemtap/tapset/"
STAP_RUNTIME=$PREFIX"/systemtap/installed/share/systemtap/runtime/"

case $DEVICE in
"g1")
    KERNEL_SRC=$PREFIX"/kernel/msm_g1";;
"n1")
    KERNEL_SRC=$PREFIX"/kernel/cyanogenmod_generic";;
"n1-2")
    KERNEL_SRC=$PREFIX"/kernel/msm_n1";;
"desirez")
    KERNEL_SRC=$PREFIX"/kernel/cyanogenmod_desirez";;
"qemu")
    KERNEL_SRC=$PREFIX"/kernel/qemu";;
"p920")
    KERNEL_SRC=$PREFIX"/kernel/p920";;
"omap")
    KERNEL_SRC=$PREFIX"/kernel/omap";;
*)
  echo "Unknown device specified!"
  exit 1;;
esac

if [ ! -d $MODULE_DIR ]; then
	mkdir output
fi

if [ ! -d $MODULE_DIR"/"$DEVICE ]; then
	mkdir $MODULE_DIR"/"$DEVICE
fi

echo "$STAP -p 4 -v $3 -a arm -B CROSS_COMPILE=$TOOLCHAIN -r $KERNEL_SRC -j $TAPSETS -R $STAP_RUNTIME -t -g -m $MODULE_NAME $SCRIPT_DIR/$SCRIPT.stp"
$STAP -p 4 -v $3 -a arm -B CROSS_COMPILE=$TOOLCHAIN -r $KERNEL_SRC -j $TAPSETS -R $STAP_RUNTIME -t -g -m $MODULE_NAME $SCRIPT_DIR"/"$SCRIPT.stp
if [ -f $MODULE_NAME".ko" ]; then
	mv $MODULE_NAME".ko" $MODULE_DIR"/"$DEVICE"/"$MODULE_NAME".ko"
fi;
