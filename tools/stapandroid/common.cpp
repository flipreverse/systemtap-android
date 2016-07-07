/*
 * Copyright 2013 Alexander Lochmann
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

#include "common.hpp"
#include <stdlib.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

void exitOnError(const char *pMsg) {
	cerr << pMsg << endl;
	exit(EXIT_FAILURE);
}

void printUsage(const char *pBinary) {
	cerr << pBinary << " [-p <port>|-i] <ip> <command> [command depended options]" << endl;
}

void printHelp(const char *pBinary) {
	cout << pBinary << " [options] <ip> <command> [command depended options]" << endl;
	cout << "-p <port>\t the port used by the android app to receive commands" << endl;
	cout << "-i\t\t ignore the module size limit of " << MAX_MODULE_FILE_SIZE << " bytes" << endl;
	cout << "commands:" << endl;
	cout << "\t list\t get a list of all installed modules and their status" << endl;
	cout << "\t send\t send and install a module on the device identified by <ip>" << endl;
	cout << "\t delete\t deletes a module installed on the device identified by <ip> (may fail, if module is running)" << endl;
	cout << "\t start\t start a module installed on the device identified by <ip>" << endl;
	cout << "\t stop\t stop a module installed on the device identified by <ip>" << endl;
}
