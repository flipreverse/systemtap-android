#include "net.hpp"
#include "common.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <fstream>

using SystemTapMessage::SystemTapMessageObject;
using SystemTapMessage::MessageType;
using SystemTapMessage::ModuleListPayload;
using SystemTapMessage::ModuleInfo;
using SystemTapMessage::SendModulePayload;
using SystemTapMessage::AckPayload;
using SystemTapMessage::ModulePayload;
using std::string;
using std::fstream;

static SystemTapMessageObject msgObject;

static int connectToDevice(char *pIP, int pPort);
static int sendToDevice(int pFD,SystemTapMessageObject *pMsgObject );
static SystemTapMessageObject* receiveFromDevice(int pFD);

int controlModule(char *pIP, int pPort, char *pName, MessageType pType) {
	int fd = 0;
	string modulePayloadRAW;
	ModulePayload modulePayload;
	AckPayload ackPayload;
	string ackPayloadRAW;
	SystemTapMessageObject *rcvMsgObject = NULL;
	
	modulePayload.Clear();
	// Set the module name, which we want to control
	modulePayload.set_name(pName);
	// Serialize the module information to a string
	if (!modulePayload.SerializeToString(&modulePayloadRAW)) {
		cerr << "Can't serialize ModulePayload" << endl;
		return -1;
	}
	
	msgObject.Clear();
	// Set the message type and its payload
	msgObject.set_type(pType);
	msgObject.set_payload(modulePayloadRAW);	
	
	if ((fd = connectToDevice(pIP,pPort)) == -1) {
		return -1;
	}

	if (sendToDevice(fd,&msgObject) < 0) {
		close(fd);
		return -1;
	}
	// An answer is expected
	if ((rcvMsgObject = receiveFromDevice(fd)) == NULL) {
		close(fd);
		return -1;
	} else {	
		if (rcvMsgObject->type() == SystemTapMessage::ACK) {
			// Parse the messages payload
			ackPayloadRAW = rcvMsgObject->payload();
			if (!ackPayload.ParseFromString(ackPayloadRAW)) {
				cerr << "Can't parse AckPayload" << endl;
				delete rcvMsgObject;
				return -1;
			}
			// Did we get an ack for the right message type?
			if (ackPayload.ackedtype() != pType) {
				cerr << "Device acked wrong message!" << endl;
				delete rcvMsgObject;
				return -1;
			}
			// An ack just tells us that the device got the instruction to start, stop or delete it.
			// It does *not* say anything about the result of this operation.
			if (pType == SystemTapMessage::START_MODULE) {
				cout << "Instructed device to start module " << pName << endl;
			} else if (pType == SystemTapMessage::STOP_MODULE) {
				cout << "Instructed device to stop module " << pName << endl;
			} else if (pType == SystemTapMessage::DELETE_MODULE) {
				cout << "Instructed device to delete module " << pName << endl;
			}
		}
	}
	delete rcvMsgObject;
	
	return 0;
}

int sendModule(char *pIP, int pPort, char *pFileName, bool pIgnoreModuleSizeRestriction) {
	int fd = 0;
	char *moduleFileContent = NULL;
	struct stat moduleFileInfo;
	SystemTapMessageObject *rcvMsgObject = NULL;
	SendModulePayload sendModulePayload;
	AckPayload ackPayload;
	string ackPayloadRAW,sendModulePayloadRAW, moduleName;
	fstream *in = NULL;

	// To allocate an appropriate buffer the concret file size is need. Hence, stat the file.
	if (stat(pFileName,&moduleFileInfo) != 0) {
		perror("Can't stat module file");
		return -1;
	}
	/* Normally systemtap kernel modules are smaller than 1 MiByte
	 * To avoid memory bombs refuse reading files with a size larger than MAX_MODULE_FILE_SIZE.
	 */
	if (moduleFileInfo.st_size > MAX_MODULE_FILE_SIZE && !pIgnoreModuleSizeRestriction) {
		cerr << "Module file is too large: " << moduleFileInfo.st_size << endl;
		cerr << "Only modules with a size of " << MAX_MODULE_FILE_SIZE << " bytes are allowed. For further information use --help." << endl;
		return -1;
	}
	// Allocate a buffer for the module content
	moduleFileContent = (char*)malloc(moduleFileInfo.st_size * sizeof(char));
	if (moduleFileContent == NULL) {
		cerr << "Can't allocate buffer" << endl;
		return -1;
	}
	
	in = new fstream();
	in->open(pFileName,std::fstream::in |std::fstream::binary);
	if (!in->is_open()) {
		cerr << "Can't open file: " << pFileName << endl;
		delete in;
		free(moduleFileContent);
		return -1;
	}
	in->read(moduleFileContent,moduleFileInfo.st_size);
	if (in->fail()) {
		cerr << "Can't read module content from file: " << pFileName << endl;
		in->close();
		delete in;
		free(moduleFileContent);
		return -1;
	}
	in->close();
	delete in;
	
	// Strip off the path and the module extension to retrieve the module name
	moduleName = basename(pFileName);
	moduleName = moduleName.substr(0,moduleName.find_first_of("."));
	sendModulePayload.Clear();
	// Set module name and content 
	sendModulePayload.set_name(moduleName);
	sendModulePayload.set_data(moduleFileContent,moduleFileInfo.st_size);
	// Serialize message payload to string
	if (!sendModulePayload.SerializeToString(&sendModulePayloadRAW)) {
		cerr << "Can't serialize SendModulePayload" << endl;
		free(moduleFileContent);
		return -1;
	}
	
	msgObject.Clear();
	// Assign message type and payload to message object
	msgObject.set_type(SystemTapMessage::SEND_MODULE);
	msgObject.set_payload(sendModulePayloadRAW);	
	
	cout << "Sending module " << pFileName << " ..." << endl;
	if ((fd = connectToDevice(pIP,pPort)) == -1) {
		free(moduleFileContent);
		return -1;
	}

	if (sendToDevice(fd,&msgObject) < 0) {
		close(fd);
		free(moduleFileContent);
		return -1;
	}
	free(moduleFileContent);
	// Expecting an acknowledge
	if ((rcvMsgObject = receiveFromDevice(fd)) == NULL) {
		close(fd);
		return -1;
	} else {	
		if (rcvMsgObject->type() == SystemTapMessage::ACK) {
			// Get and parse the message payload...
			ackPayloadRAW = rcvMsgObject->payload();
			if (!ackPayload.ParseFromString(ackPayloadRAW)) {
				cerr << "Can't parse AckPayload" << endl;
				delete rcvMsgObject;
				return -1;
			}
			// Need an ack for SEND_MODULE
			if (ackPayload.ackedtype() != SystemTapMessage::SEND_MODULE) {
				cerr << "Device acked wrong message!" << endl;
				delete rcvMsgObject;
				return -1;
			}
		}
	}
	delete rcvMsgObject;
	
	return 0;
}

int listModules(char *pIP, int pPort) {
	int fd = 0;
	SystemTapMessageObject *rcvMsgObject = NULL;
	string payload;
	ModuleListPayload moduleListPayload;
	ModuleInfo moduleInfo;
	
	if ((fd = connectToDevice(pIP,pPort)) == -1) {
		exit(EXIT_FAILURE);
	}
	
	msgObject.Clear();
	msgObject.set_type(SystemTapMessage::LIST_MODULES);
	
	if (sendToDevice(fd,&msgObject) < 0) {
		close(fd);
	}
    // Expecting an acknowledge
	if ((rcvMsgObject = receiveFromDevice(fd)) == NULL) {
		close(fd);
		return -1;
	} else {	
		if (rcvMsgObject->type() == SystemTapMessage::MODULE_LIST) {
			// Get and parse the message payload...
			payload = rcvMsgObject->payload();
			if (!moduleListPayload.ParseFromString(payload)) {
				cerr << "Can't parse ModuleListPayload from string" << endl;
				delete rcvMsgObject;
				return -1;
			}
			cout << "The following modules are currently installed:" << endl;
			if (moduleListPayload.modules_size() == 0) {
				cout << "none" << endl;
			}
			for (int i = 0; i < moduleListPayload.modules_size(); i++) {
				moduleInfo = moduleListPayload.modules(i);
				cout << moduleInfo.name() << " --> ";
				switch (moduleInfo.status()) {
					case SystemTapMessage::RUNNING:
						cout << "RUNNING" << endl;break;
					case SystemTapMessage::STOPPED:
						cout << "STOPPED" << endl;break;
					case SystemTapMessage::CRASHED:
						cout << "CRASHED" << endl;break;
				}
			}
		} else {
			cerr << "Got unexpected response to LIST_MODULES" << endl;
		}
	}
	delete rcvMsgObject;
	return 0;
}

static int connectToDevice(char *pHost, int pPort) {
	int socketFD = 0;
    struct sockaddr_in serverAddr;
    struct hostent *hostnameInfo;
    char *hostIP = NULL;
	
	memset(&serverAddr,0,sizeof(serverAddr));

	if ((hostnameInfo = gethostbyname(pHost)) == NULL) {
		fprintf(stderr,"Can't resolve hostname: %s\n",hstrerror(h_errno));
		return -1;
	}
	hostIP = hostnameInfo->h_addr_list[0];

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Can't create socket");
		return -1;
	}
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = *((in_addr_t*)hostIP);
    serverAddr.sin_port = htons(pPort);
    
    if (connect(socketFD,(struct sockaddr *) &serverAddr,sizeof(serverAddr)) < 0) {
		perror("Can't connect to device");
		return -1;
	}
	
	return socketFD;
}

static int sendToDevice(int pFD,SystemTapMessageObject *pMsgObject ) {
	int msgObjectSize = 0, sendBytesSize = 0, sendBytesObject;
	char *buffer = NULL;

	buffer = (char*)malloc(pMsgObject->ByteSize() * sizeof(char));
	if (!pMsgObject->SerializeToArray(buffer,pMsgObject->ByteSize())) {
		cerr << "Can't serialize SystemTapMessageObject to byte array" << endl;
		return -1;
	}

	msgObjectSize = pMsgObject->ByteSize();
	/* The concret byte protocol is structured as follows:
	 *  4 bytes: message size in big endian (network byte order)
	 *  <message size> bytes: the serialized procotol object
	 * 
	 * the first four bytes has to be in big endian order, because the java inputstream (readInt()) expects it.
	 * In network context a long value consists of four bytes.
	 */
	msgObjectSize = htonl(msgObjectSize);
	sendBytesSize = write(pFD,&msgObjectSize,sizeof(msgObjectSize));
    if (sendBytesSize < 0) {
		perror("Cant't write to socket");
		return sendBytesSize;
    }
    sendBytesObject = write(pFD,buffer,pMsgObject->ByteSize());
    if (sendBytesObject < 0) {
		perror("Cant't write to socket");
		return sendBytesObject;
    }
    return sendBytesSize + sendBytesObject;
}

static SystemTapMessageObject* receiveFromDevice(int pFD) {
	int recvBytes = 0, objectSize = 0;
	char *buffer = NULL;
	SystemTapMessageObject *retMsgObj = NULL;
	
	recvBytes = read(pFD,&objectSize,sizeof(objectSize));
    if (recvBytes < 0) {
		perror("Can't read protocol object size from socket");
		return NULL;
	}
	buffer = (char*)malloc(objectSize * sizeof(char));
	if (buffer == NULL) {
		perror("Can't allocate receive buffer");
		return NULL;
	}
	recvBytes = read(pFD,buffer,objectSize);
    if (recvBytes < 0) {
		perror("Can't read from socket");
		return NULL;
	} else if (recvBytes > 0) {
		retMsgObj = new SystemTapMessageObject();
		retMsgObj->Clear();
		if (!retMsgObj->ParseFromArray(buffer,recvBytes)) {
			printf("Can't parse protobuf. array length: %d\n",recvBytes);
			return NULL;
		}
	}
	free(buffer);
	return retMsgObj;
}
