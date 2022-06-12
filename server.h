#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
#include <sys/stat.h>

#define BYTE_LENGTH 1
#define CHAR_BYTE 1
#define ANSWER_TYPE_OFFSET 33
#define MAX_BUF_SIZE 1024
#define MAX_LOG_SIZE 256
#define PACKET_LEN 2
#define GOTO_QUERY 14

#define AAAA_RECORD_TYPE 28

typedef unsigned char Byte;
typedef struct _dnsRequest DNSRequest;
typedef struct _dnsResponse DNSResponse;


struct _dnsResponse{
  unsigned char* bytes;
  int bytesLength;
  char* ipAddr;
};


struct _dnsRequest{
  unsigned char* bytes;
  int bytesLength;
  char* queryName;
  int recordType;
  int isValid;
};

//Request Response
DNSRequest* serializeRequest(Byte* buffer);
DNSRequest* handleRequest(int clientfd);

DNSResponse* handleResponse(int upstreamfd);
DNSResponse* serializeResponse(Byte* bytes);



//Server functions
int initServer(char* port);
int connectToUpstreamPersistent(char **argv);
int createClientSession(int serverSocket);

//log
int logResponse(DNSResponse* res);
int logRequest(DNSRequest* req);


//Send
void sendToUpstreamServer(DNSRequest* req, int upstreamfd);
void sendPacketToClient(int clientSockFd, DNSResponse *res);

//private
char* formatIPv6Addr(char* ipv6Addr);
int getIPAddressOffset(Byte *stream); 
char* readAddrByte(Byte byte1, Byte byte2);
char* getNameServer(DNSResponse* res);
void handleInvalidRequest(int clientSockFd, DNSRequest *req);
int getRecordType(DNSResponse* res);
DNSRequest* handleRequestStream(int clientfd);


#endif
