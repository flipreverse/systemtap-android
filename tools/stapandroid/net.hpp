#ifndef __NET_HPP__
#define __NET_HPP__

#include "SystemTapMessage.pb.h"

#define DEFAULT_PORT 4711
#define DEFAULT_BUFLEN 1024

using SystemTapMessage::MessageType;

/* The code for sending start, stop or delete is the same, except the concret message type.
 * Hence, a define for each case to a generic parameterized function will do.
 */
#define deleteModule(ip,port,name)	controlModule(ip,port,name,SystemTapMessage::DELETE_MODULE)
#define startModule(ip,port,name)	controlModule(ip,port,name,SystemTapMessage::START_MODULE)
#define stopModule(ip,port,name)	controlModule(ip,port,name,SystemTapMessage::STOP_MODULE)

int sendModule(char *pIP, int pPort, char *pFileName, bool pIgnoreModuleSizeRestriction);
int controlModule(char *pIP, int pPort, char *pName, MessageType pType);
int listModules(char *pIP, int pPort);

#endif
