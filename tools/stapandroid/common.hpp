#ifndef __COMMON_HPP__
#define __COMMON_HPP__

/*
 * Copyright 2013 Alexander Lochmann, Michael Lenz
 *
 * This file is part of SystemTap4Android.
 *
 * SystemTap4Android is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SystemTap4Android is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SystemTap4Android.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <iostream>

#define MAX_MODULE_FILE_SIZE 1024 * 1024 // 1MiBi

using std::cout;
using std::endl;
using std::cerr;

enum Commands {
	UNKNOWN			= 0,
	LIST_MODULES	= 1,
	SEND_MODULE		= 2,
	DELETE_MODULE	= 3,
	START_MODULE	= 4,
	STOP_MODULE		= 5
};

void exitOnError(const char *pMsg);
void printUsage(const char *pBinary);
void printHelp(const char *pBinary);

#endif
