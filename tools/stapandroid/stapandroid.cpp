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

#include "common.hpp"
#include "net.hpp"
#include <unistd.h>
#include <string.h>

#define OPTIONS "-p:"

static Commands parseCommand(char *pCmd, int pLength, char * const pArgv[]);

int main(int argc, char * const argv[]) {

	char option = -1, *cmd = NULL, *ip = NULL;
	int port = DEFAULT_PORT, ipIDX = 0, cmdIDX = 0, ret = 0;
	bool ignoreModuleSizeRestriction = false;
	Commands command = UNKNOWN;

	// At least a devices ip address and a command are needed
	if (argc < 3) {
		printHelp(argv[0]);
		return EXIT_FAILURE;
	}
	/* Parse the optional commandline argument. Parsing will stop, if the first unknown option occurs,
	 * which should be the devices ip address
	 */
	while ((option = getopt(argc,argv,"+p:i")) != -1) {
		switch (option) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'i':
				ignoreModuleSizeRestriction = true;
				break;
			case '?':
			default:
				break;
		}
	}
	// No arguments left after parsing the optional ones --> invalid usage!
	if (optind + 1 == argc) {
		printUsage(argv[0]);
		return EXIT_FAILURE;
	}
	/* Read the devices ip address and the command:
	 * optind (man 3 getopt) is the index of the next command line argument to be parsed.
	 */
	ipIDX = optind;
	ip = argv[ipIDX];
	// The above if statement avoids segmentations fault, when accessing optind + 1.
	cmdIDX = optind + 1;
	cmd = argv[cmdIDX];
	// Evaluate the supplied command. Concerning to the parsed command check for further arguments.
	if ((command = parseCommand(cmd,argc - cmdIDX - 1,&argv[cmdIDX + 1])) == UNKNOWN) {
		printHelp(argv[0]);
		return EXIT_FAILURE;
	}

	switch (command) {
		case LIST_MODULES:
			ret = listModules(ip,port);
			break;
		case SEND_MODULE:
			ret = sendModule(ip,port,argv[cmdIDX + 1],ignoreModuleSizeRestriction);
			break;
		case DELETE_MODULE:
			ret = deleteModule(ip,port,argv[cmdIDX + 1]);
			break;
		case START_MODULE:
			ret = startModule(ip,port,argv[cmdIDX + 1]);
			break;
		case STOP_MODULE:
			ret = stopModule(ip,port,argv[cmdIDX + 1]);
			break;
	}
	if (ret < 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static Commands parseCommand(char *pCmd, int pLength, char * const pArgv[]) {
	if (strcmp(pCmd,"list") == 0) {
		return LIST_MODULES;
	} else if (strcmp(pCmd,"send") == 0) {
		if (pLength == 0) {
			return UNKNOWN;
		}
		return SEND_MODULE;
	} else if (strcmp(pCmd,"delete") == 0) {
		if (pLength == 0) {
			return UNKNOWN;
		}
		return DELETE_MODULE;
	} else if (strcmp(pCmd,"start") == 0) {
		if (pLength == 0) {
			return UNKNOWN;
		}
		return START_MODULE;
	} else if (strcmp(pCmd,"stop") == 0) {
		if (pLength == 0) {
			return UNKNOWN;
		}
		return STOP_MODULE;
	}
	return UNKNOWN;
}
