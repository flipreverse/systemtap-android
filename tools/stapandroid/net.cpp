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

using SystemTapMessage::MessageType;
using SystemTapMessage::ModuleList;
using SystemTapMessage::ModuleInfo;
using SystemTapMessage::SendModule;
using SystemTapMessage::Ack;
using std::string;
using std::fstream;

static int connectToDevice(char *pIP, int pPort);
static int sendToDevice(int pFD,int pMsgType, char *pData, int pDataSize);
static char* receiveFromDevice(int pFD, int *pMsgType, int *pDataSize);

int controlModule(char *pIP, int pPort, char *pName, SystemTapMessage::ModuleStatus pStatus) {
	int fd = 0, msgType = 0, msgSize = 0;
	char *msgBuffer = NULL;
	ModuleInfo moduleInfo;
	Ack ack;
	
	moduleInfo.Clear();
	// Set the module name, which we want to control
	moduleInfo.set_name(pName);
	// Set the modules desired status
	moduleInfo.set_status(pStatus);
	// Serialize the module information to a byte array
	msgBuffer = (char*)malloc(moduleInfo.ByteSize() * sizeof(char));
	if (!moduleInfo.SerializeToArray(msgBuffer,moduleInfo.ByteSize())) {
		cerr << "Can't serialize SystemTapMessageObject to byte array" << endl;
		return -1;
	}
	
	if ((fd = connectToDevice(pIP,pPort)) == -1) {
		return -1;
	}

	if (sendToDevice(fd,SystemTapMessage::CONTROL_MODULE,msgBuffer,moduleInfo.ByteSize()) < 0) {
		close(fd);
		free(msgBuffer);
		return -1;
	}
	free(msgBuffer);
	// An answer is expected
	if ((msgBuffer = receiveFromDevice(fd,&msgType,&msgSize)) == NULL) {
		close(fd);
		return -1;
	} else {
		if (msgType == SystemTapMessage::ACK) {
			ack.Clear();
			// Parse the response
			if (!ack.ParseFromArray(msgBuffer,msgSize)) {
				printf("Can't parse ack. buffer size: %d\n",msgSize);
				free(msgBuffer);
				return -1;
			}
			// Did we get an ack for the right message type?
			if (ack.ackedtype() != SystemTapMessage::CONTROL_MODULE) {
				cerr << "Device acked wrong message!" << endl;
				return -1;
			}
			// An ack just tells us that the device got the instruction to start, stop or delete it.
			// It does *not* say anything about the result of this operation.
			if (pStatus == SystemTapMessage::RUNNING) {
				cout << "Instructed device to start module " << pName << endl;
			} else if (pStatus == SystemTapMessage::STOPPED) {
				cout << "Instructed device to stop module " << pName << endl;
			} else if (pStatus == SystemTapMessage::DELETED) {
				cout << "Instructed device to delete module " << pName << endl;
			}
		}
	}
	free(msgBuffer);
	
	return 0;
}

int sendModule(char *pIP, int pPort, char *pFileName, bool pIgnoreModuleSizeRestriction) {
	int fd = 0, msgType = 0, msgSize = 0;
	char *moduleFileContent = NULL, *msgBuffer = NULL;
	struct stat moduleFileInfo;
	SendModule sendModule;
	Ack ack;
	string  moduleName;
	fstream *in = NULL;

	// To allocate an appropriate buffer the particular file size is need. Hence, stat the file.
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
	sendModule.Clear();
	// Set module name and content 
	sendModule.set_name(moduleName);
	sendModule.set_data(moduleFileContent,moduleFileInfo.st_size);
	// Serialize message to an array
	msgBuffer = (char*)malloc(sendModule.ByteSize() * sizeof(char));
	if (!sendModule.SerializeToArray(msgBuffer,sendModule.ByteSize())) {
		cerr << "Can't serialize SendModule" << endl;
		free(moduleFileContent);
		return -1;
	}

	cout << "Sending module " << pFileName << " ..." << endl;
	if ((fd = connectToDevice(pIP,pPort)) == -1) {
		free(moduleFileContent);
		return -1;
	}

	if (sendToDevice(fd,SystemTapMessage::SEND_MODULE,msgBuffer,sendModule.ByteSize()) < 0) {
		close(fd);
		free(moduleFileContent);
		free(msgBuffer);
		return -1;
	}
	free(moduleFileContent);
	free(msgBuffer);
	// Expecting an acknowledge
	if ((msgBuffer = receiveFromDevice(fd,&msgType,&msgSize)) == NULL) {
		close(fd);
		return -1;
	} else {	
		if (msgType == SystemTapMessage::ACK) {
			// Parse the response
			if (!ack.ParseFromArray(msgBuffer,msgSize)) {
				cerr << "Can't parse AckPayload" << endl;
				free(msgBuffer);
				return -1;
			}
			// Need an ack for SEND_MODULE
			if (ack.ackedtype() != SystemTapMessage::SEND_MODULE) {
				cerr << "Device acked wrong message!" << endl;
				free(msgBuffer);
				return -1;
			}
		}
	}
	free(msgBuffer);
	
	return 0;
}

int listModules(char *pIP, int pPort) {
	int fd = 0, msgType = 0, msgSize = 0;
	char *msgBuffer = NULL;
	ModuleInfo moduleInfo;
	ModuleList moduleList;
	
	if ((fd = connectToDevice(pIP,pPort)) == -1) {
		exit(EXIT_FAILURE);
	}
	
	if (sendToDevice(fd,SystemTapMessage::LIST_MODULES,NULL,0) < 0) {
		close(fd);
	}
    // Expecting a response
	if ((msgBuffer = receiveFromDevice(fd,&msgType,&msgSize)) == NULL) {
		close(fd);
		return -1;
	} else {	
		if (msgType == SystemTapMessage::MODULE_LIST) {
			// Parse the message ...
			if (!moduleList.ParseFromArray(msgBuffer,msgSize)) {
				cerr << "Can't parse ModuleList from array" << endl;
				free(msgBuffer);
				return -1;
			}
			cout << "The following modules are currently installed:" << endl;
			if (moduleList.modules_size() == 0) {
				cout << "none" << endl;
			}
			for (int i = 0; i < moduleList.modules_size(); i++) {
				moduleInfo = moduleList.modules(i);
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
	free(msgBuffer);
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

static int sendToDevice(int pFD,int pMsgType, char *pMsg, int pMsgSize) {
	int sendBytesType = 0, sendBytesSize = 0, sendBytesObject;

	/* The concret byte protocol is structured as follows:
	 *  4 bytes: message type in big endian (network byte order)
	 *  4 bytes: message size in big endian (network byte order)
	 *  <message size> bytes: the serialized procotol object
	 * 
	 * the first eight bytes has to be in big endian order, because the java inputstream (readInt()) expects it.
	 * In network context a long value consists of four bytes.
	 */
	// Java expects integers in network byte order (see InputStream.readInt())
	pMsgType = htonl(pMsgType);
	sendBytesType = write(pFD,&pMsgType,sizeof(pMsgType));
    if (sendBytesType < 0) {
		perror("Cant't write to socket");
		return sendBytesType;
    }

	// Java expects integers in network byte order (see InputStream.readInt())
	pMsgSize = htonl(pMsgSize);
	sendBytesSize = write(pFD,&pMsgSize,sizeof(pMsgSize));
    if (sendBytesSize < 0) {
		perror("Cant't write to socket");
		return sendBytesSize;
    }
    // Convert it back to host order otherwise the following write-call will send much more bytes than it should do.
    pMsgSize = ntohl(pMsgSize);

    if (pMsgSize > 0) {
		sendBytesObject = write(pFD,pMsg,pMsgSize);
		if (sendBytesObject < 0) {
			perror("Cant't write to socket");
			return sendBytesObject;
		}
	}
    return sendBytesType + sendBytesSize + sendBytesObject;
}

static char* receiveFromDevice(int pFD, int *pMsgType, int *pMsgSize) {
	int recvBytes = 0, msgType = 0, msgSize = 0;
	char *msgBuffer = NULL;
	
	recvBytes = read(pFD,&msgType,sizeof(msgType));
    if (recvBytes < 0) {
		perror("Can't read message type from socket");
		return NULL;
	}
	*pMsgType = ntohl(msgType);

	recvBytes = read(pFD,&msgSize,sizeof(msgSize));
    if (recvBytes < 0) {
		perror("Can't read object size from socket");
		return NULL;
	}
	*pMsgSize = ntohl(msgSize);

	msgBuffer = (char*)malloc(msgSize * sizeof(char));
	if (msgBuffer == NULL) {
		perror("Can't allocate receive buffer");
		return NULL;
	}
	recvBytes = read(pFD,msgBuffer,msgSize);
    if (recvBytes < 0) {
		perror("Can't read from socket");
		return NULL;
	}
	return msgBuffer;
}
