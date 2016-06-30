#!/bin/bash
CUR_DIR=`pwd`
INSTALL_DIR=${CUR_DIR}"/installed"
SRC_DIR=${CUR_DIR}"/src"
TOOLS_DIR=${CUR_DIR}"/tools"

ELFUTILS_VERSION="0.166"
ELFUTILS_DIR="${CUR_DIR}/elfutils-"${ELFUTILS_VERSION}
ELFUTILS_TAR="elfutils-"${ELFUTILS_VERSION}".tar.bz2"

CONFIGURE_OUTPUT="${SRC_DIR}/configure_out.txt"
MAKE_OUTPUT="${SRC_DIR}/make_out.txt"
ANDROID_BUILD_OUTPUT="${SRC_DIR}/android_build_out.txt"
TOOLS_OUTPUT="${TOOLS_DIR}/make_out.txt"
NUM_CPUS=`grep -c ^processor /proc/cpuinfo`	
BUILDSCRIPT_ANDROID="./build_android.sh"

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
if [ -e "Makefile" ];
then
	echo "Workspace already configured. Cleaning it..."
	make distclean > /dev/null 2>&1
fi

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
make install >> ${MAKE_OUTPUT} 2>&1
if [ $? -gt 0 ];
then
	echo "Error installing SystemTap. For further information look at ${MAKE_OUTPUT}" >&2
	cd ${PWD}
	exit 1
fi

echo "Starting buildscript for Android..."
#${BUILDSCRIPT_ANDROID} > ${ANDROID_BUILD_OUTPUT} 2>&1
echo "Skipping build of stap* for Android, because the build process is broken"
if [ $? -gt 0 ];
then
	echo "Error compiling SystemTap for Android. For further information look at ${ANDROID_BUILD_OUTPUT}" >&2
	cd ${PWD}
	exit 1
fi

echo "Building bootimg tools..."
echo "make -C ${TOOLS_DIR}/bootimg" > ${TOOLS_OUTPUT}
make -C ${TOOLS_DIR}/bootimg >> ${TOOLS_OUTPUT} 2>&1
if [ $? -gt 0 ];
then
	echo "Error compiling bootimg. For further information look at ${TOOLS_OUTPUT}" >&2
	cd ${PWD}
	exit 1
fi

echo "Building stapandroid tool..."
echo "make -C ${TOOLS_DIR}/stapandroid" >> ${TOOLS_OUTPUT}
make -C ${TOOLS_DIR}/stapandroid >> ${TOOLS_OUTPUT} 2>&1
if [ $? -gt 0 ];
then
	echo "Error compiling stapandroid control tool. For further information look at ${TOOLS_OUTPUT}" >&2
	cd ${PWD}
	exit 1
fi

echo "Information about every step during the build process is located at ${SRC_DIR}:"
echo "configure:	${CONFIGURE_OUTPUT}"
echo "make:		${MAKE_OUTPUT}"
echo "Android binaries:	${ANDROID_BUILD_OUTPUT}"
echo "tools:	${TOOLS_OUT}"

cd ${PWD}
