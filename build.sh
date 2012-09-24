#!/bin/bash
CUR_DIR=`pwd`
INSTALL_DIR=${CUR_DIR}"/installed"
SRC_DIR=${CUR_DIR}"/src"

ELFUTILS_VERSION="0.155"
ELFUTILS_DIR="${CUR_DIR}/elfutils-"${ELFUTILS_VERSION}
ELFUTILS_TAR="elfutils-"${ELFUTILS_VERSION}".tar.bz2"

CONFIGURE_OUTPUT="${SRC_DIR}/configure_out.txt"
MAKE_OUTPUT="${SRC_DIR}/make_out.txt"
NUM_CPUS=`grep -c ^processor /proc/cpuinfo`	

if [ ! -d ${ELFUTILS_DIR} ];
then
	if [ ! -f ${ELFUTILS_TAR} ];
	then
		echo "No elfutils tarball (${ELFUTILS_TAR}) found" >&2
		exit 1
	else
		tar -xjvf ${ELFUTILS_TAR} > /dev/null
		if [ $? -eq 0 ];
		then
			echo "Extracted ${ELFUTILS_TAR}"
		else
			echo "Could not extract ${ELFUTILS_TAR}" >&2
			exit 1
		fi
	fi
fi

cd ${SRC_DIR}
CONFIGURE_CMD="./configure --prefix=${INSTALL_DIR} --with-elfutils=${ELFUTILS_DIR}"
echo "Executing ${CONFIGURE_CMD}"
${CONFIGURE_CMD} > ${CONFIGURE_OUTPUT} 2>&1

if [ $? -gt 0 ];
then
	echo "Error configuring systemtap. For further information look at ${CONFIGURE_OUTPUT}" >&2
	cd ${PWD}
	exit 1
fi

echo "Start compiling with -j${NUM_CPUS} ..."
nice make -j${NUM_CPUS} > ${MAKE_OUTPUT} 2>&1

if [ $? -gt 0 ];
then
	echo "Error compiling systemtap. For further information look at ${MAKE_OUTPUT}" >&2
	cd ${PWD}
	exit 1
fi

if [ -d ${INSTALL_DIR} ];
then
	echo "${INSTALL_DIR} exists. Cleaning it."
	rm -rf ${INSTALL_DIR}/*
fi

echo "Installing systemtap in ${INSTALL_DIR}..."
make install > /dev/null

cd ${PWD}
