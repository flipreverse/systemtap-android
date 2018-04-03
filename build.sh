#!/bin/bash

# Copyright 2012 Alexander Lochmann
#
# This file is part of SystemTap4Android.
#
# SystemTap4Android is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SystemTap4Android is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SystemTap4Android.  If not, see <http://www.gnu.org/licenses/>.

CUR_DIR=`pwd`
TOOLS_DIR=${CUR_DIR}"/tools"

ANDROID_BUILD_OUTPUT="${SRC_DIR}/android_build_out.txt"
TOOLS_OUTPUT="${TOOLS_DIR}/make_out.txt"
BUILDSCRIPT_ANDROID="./build_android.sh"


cd ${SRC_DIR}
if [ -e "Makefile" ];
then
	echo "Workspace already configured. Cleaning it..."
	make distclean > /dev/null 2>&1
fi

echo "Starting buildscript for Android..."
${BUILDSCRIPT_ANDROID} > ${ANDROID_BUILD_OUTPUT} 2>&1

if [ $? -gt 0 ];
then
	echo "Error compiling SystemTap for Android. For further information look at ${ANDROID_BUILD_OUTPUT}" >&2
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
echo "Android binaries:	${ANDROID_BUILD_OUTPUT}"
echo "tools:	${TOOLS_OUT}"

cd ${PWD}
