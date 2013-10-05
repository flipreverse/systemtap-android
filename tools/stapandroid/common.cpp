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
