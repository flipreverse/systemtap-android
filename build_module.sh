#!/bin/bash
CONF_DIR="config"
CONF_EXT=".conf"
MODULE_DIR="modules"
SCRIPT_DIR="scripts"

PREFIX=`pwd`
STAP=$PREFIX"/installed/bin/stap"
TAPSETS=$PREFIX"/installed/share/systemtap/tapset/"
STAP_RUNTIME=$PREFIX"/installed/share/systemtap/runtime/"

COMPILER_PREFIX_LIST=("arm-eabi-" "arm-none-linux-gnueabi-" "arm-none-eabi-")

function listSupportedDevices()
{
	echo -n "devices: "
	ls ${CONF_DIR}/*${CONF_EXT} >/dev/null 2>&1
	if [ $? -gt 0 ];
	then
		echo "no devices found"
		return
	fi
	for device in `ls ${CONF_DIR}/*${CONF_EXT}`;
	do
		device=`basename ${device}`
		echo -n ${device%${CONF_EXT}}","
	done;
	echo "."
}

function checkCompiler()
{
	for cur_compiler_prefix in ${COMPILER_PREFIX_LIST[@]}
	do
		CUR_GCC="${cur_compiler_prefix}gcc"
		RET=`${CUR_GCC} --version 2>&1`
		if [ $? -eq 0 ];
		then
			#COMPILER_PREFIX=`dirname \`which ${CUR_GCC}\``"/${cur_compiler_prefix}"
			COMPILER_PREFIX=${cur_compiler_prefix}
			return
		fi
	done;

	echo "No suitable compiler found. Check your PATH" >&2
	exit 1
}

function getKernelInfos()
{
	if [ ! -d ${CONF_DIR} ];
	then
		echo "conf directory does not exist" >&2
		exit 1
	fi
	DEVICE=$1
	CONF_FILE=${CONF_DIR}/${DEVICE}${CONF_EXT}

	if [ -f ${CONF_FILE} ];
	then
		KERNEL_SRC=`cat ${CONF_FILE}`
		if [ ! -d ${KERNEL_SRC} ];
		then
			echo "kernel source tree (${KERNEL_SRC}) does not exit. Check the corresponding config file: ${CONF_FILE}" >&2
			exit 1
		fi
	else
		echo "config file (${CONF_FILE}) does not exist." >&2
		exit 1
	fi
}

function usage()
{
	echo "usage: $1 <skriptname> <device>"
	listSupportedDevices
	exit 1
}

if [ $# -lt 2 ];
then
	usage $0
fi

checkCompiler

getKernelInfos $2

SCRIPT=$1
if [ ! -f ${SCRIPT_DIR}/${SCRIPT}".stp" ];
then
	echo "The specifid script (${SCRIPT}.stp) does not exist" >&2	
	exit 1
fi

MODULE_NAME=${SCRIPT//./}


if [ ! -d ${MODULE_DIR} ]; then
	echo "module directory does not exist. Creating it..."
	mkdir ${MODULE_DIR}
fi

if [ ! -d ${MODULE_DIR}"/"${DEVICE} ]; then
	mkdir ${MODULE_DIR}"/"${DEVICE}
fi

STAP_CMD="${STAP} -p 4 -v ${3} -a arm -B CROSS_COMPILE=${COMPILER_PREFIX} -r ${KERNEL_SRC} -j ${TAPSETS} -R ${STAP_RUNTIME} -t -g -m ${MODULE_NAME} ${SCRIPT_DIR}/${SCRIPT}.stp"
echo "Executing: ${STAP_CMD}"
${STAP_CMD}

if [ -f ${MODULE_NAME}".ko" ]; then
	mv ${MODULE_NAME}".ko" ${MODULE_DIR}"/"${DEVICE}"/"${MODULE_NAME}".ko"
fi;
