#ifndef __COMMON_HPP__
#define __COMMON_HPP__

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
