#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>


#include "server.h"
#include "utils.h"


DNSRequest* serializeRequest(Byte* buffer){



  DNSRequest *req = (DNSRequest*)malloc(sizeof(DNSRequest));

	Byte tcpHeader[PACKET_LEN];
	memcpy(tcpHeader, buffer, PACKET_LEN);
	req->bytesLength = hexToDec(tcpHeader) + PACKET_LEN;



	// Initialise Memory at first.
  req->bytes = (Byte*)malloc(sizeof(Byte)* (req->bytesLength + PACKET_LEN));
	memcpy(req->bytes, buffer, sizeof(Byte)* (req->bytesLength + PACKET_LEN));

  return req;
}

// Returns FALSE if request is invalid, returns true if request is valid
int logRequest(DNSRequest* req){
  int memLength = req->bytesLength - GOTO_QUERY;
  Byte *bytes = req->bytes;

  //Get the rest of the DNS request. Extract the queries out
  Byte* rest = (Byte*)malloc(sizeof(Byte)*req->bytesLength);
  memcpy(rest, bytes + (CHAR_BYTE*GOTO_QUERY), memLength);

  // Get the size initially, DNS Query Name terminated by 00000000 = 0
  // DNS Query Structure: [variable][2 byte- type][2 byte -class]
  int count = 0;
  while(rest[count] != 0){
    count++;
  }
  // Null terminator + Type + Class
  count += 5;

  // Since we can expect the query name to be < count;
  // use char becaus ascii runs from 0 - 127
  char* queryName = (char*)malloc(sizeof(char)*count);

  // Byte that contains the query
  Byte* query = (Byte*)malloc(sizeof(Byte)*count);
  memcpy(query, bytes + (CHAR_BYTE*14), count);

  int index =0;
  int queryIndex=0;

  while(query[index] != 0){
    int length = query[index];
    index+=1;

    for(int i=0; i<length; i++){
      queryName[queryIndex] = query[index];
      index+=1;queryIndex += 1;
    }
    queryName[queryIndex++] = '.';
  }
  // Undo the last .
  queryName[queryIndex-1] = '\0';


  //RecordType
  Byte recordType[2];
  memcpy(recordType, query+(count-4), 2);

  req->queryName = queryName;
  req->recordType = hexToDec(recordType);

  //Assuming it will not exceed 256 characters
  char log[MAX_LOG_SIZE];
  memset(log, 0, MAX_LOG_SIZE);

  if(req->recordType != AAAA_RECORD_TYPE){
    FILE* fptr = getFile();
    fprintf(fptr, "%s requested %s\n", getTimeStamp(), req->queryName);
    fprintf(fptr, "%s unimplemented request\n", getTimeStamp());
    fclose(fptr);

    return FALSE;
  }else{
    FILE* fptr = getFile();
    
    fprintf(fptr, "%s requested %s\n", getTimeStamp(), req->queryName);
    fclose(fptr);

    return TRUE;
  }

}


void sendToUpstreamServer(DNSRequest* req, int upstreamfd){
  req->bytes[5] = req->bytes[5] |0x80;

  int n;

	// Send message to server
	n = write(upstreamfd, req->bytes, req->bytesLength);
	if (n < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
}


int initServer(char* port){
	int sockfd, re , s;
	struct addrinfo hints, *res;

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res
	s = getaddrinfo(NULL, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Reuse port if possible
	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);

	// Listen on socket - means we're ready to accept connections,
	// incoming connection requests will be queued, man 3 listen
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

int connectToUpstreamPersistent(char **argv){
	int sockfd, s;
	struct addrinfo hints, *servinfo, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break; // success

		close(sockfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(servinfo);


	return sockfd;
}


int createClientSession(int serverSocket){
	int newsockfd;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	client_addr_size = sizeof client_addr;
	newsockfd =
		accept(serverSocket, (struct sockaddr*)&client_addr, &client_addr_size);
	if (newsockfd < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	
	return newsockfd;
}

DNSRequest* handleRequest(int clientfd){
  Byte clientBuffer[MAX_BUF_SIZE];
  int n; // n is number of characters read

  n = read(clientfd, clientBuffer,MAX_BUF_SIZE); 
	if (n < 0) {
		perror("ERROR reading from socket");
		exit(EXIT_FAILURE);
	}
	clientBuffer[n] = '\0';

  DNSRequest* request = serializeRequest(clientBuffer);

  return request;
}


DNSResponse* handleResponse(int upstreamfd){  
  Byte upstreamResponseBuf[MAX_BUF_SIZE];
  int n; // n is number of characters read

  n = read(upstreamfd, upstreamResponseBuf,MAX_BUF_SIZE); 
	if (n < 0) {
		perror("ERROR reading from socket");
		exit(EXIT_FAILURE);
	}
	upstreamResponseBuf[n] = '\0';

  fflush(stdout);

  DNSResponse* response = serializeResponse(upstreamResponseBuf);

  return response;
}

DNSResponse* serializeResponse(Byte* bytes){
  DNSResponse *res = (DNSResponse*)malloc(sizeof(DNSResponse));
  Byte tcpHeader[PACKET_LEN];
	memcpy(tcpHeader, bytes, PACKET_LEN);
  res->bytesLength = hexToDec(tcpHeader);

  res->bytes = (Byte*)malloc(sizeof(Byte)* (res->bytesLength + PACKET_LEN));
	memcpy(res->bytes, bytes, sizeof(Byte)*(res->bytesLength + PACKET_LEN));

  return res;
}

int logResponse(DNSResponse* res){

  Byte* stream = res->bytes;

  int recordType = getRecordType(res);

  if(recordType != 28){
    return FALSE;
  }

  int offset = getIPAddressOffset(stream);
  char* strAddr = (char*)calloc(INET6_ADDRSTRLEN, sizeof(char));
  
  int idx =0;
  for(int i = offset; i< offset+ 16; i = i + 2){
    char* temp = readAddrByte(stream[i], stream[i+1]);
    if(i == (offset + 14)){
      temp[4] = 0;
    }

    for(int j =0; j < 5 ; j++){
      strAddr[idx+j] = temp[j]; 
    }
    idx+=5;
  }

  char* formattedIp6 = formatIPv6Addr(strAddr);
  FILE* fptr = getFile();

  fprintf(fptr, "%s %s is at %s\n", getTimeStamp(), getNameServer(res),formattedIp6 );
  fclose(fptr);

  return TRUE;
}

char* getNameServer(DNSResponse* res){
  int memLength = res->bytesLength - GOTO_QUERY;
  Byte *bytes = res->bytes;

  //Get the rest of the DNS request. Extract the queries out
  Byte* rest = (Byte*)malloc(sizeof(Byte)*res->bytesLength);
  memcpy(rest, bytes + (CHAR_BYTE*GOTO_QUERY), memLength);

  // Get the size initially, DNS Query Name terminated by 00000000 = 0
  // DNS Query Structure: [variable][2 byte- type][2 byte -class]
  int count = 0;
  while(rest[count] != 0){
    count++;
  }
  // Null terminator + Type + Class
  count += 5;

  // Since we can expect the query name to be < count;
  // use char becaus ascii runs from 0 - 127
  char* queryName = (char*)malloc(sizeof(char)*count);

  // Byte that contains the query
  Byte* query = (Byte*)malloc(sizeof(Byte)*count);
  memcpy(query, bytes + (CHAR_BYTE*14), count);

  int index =0;
  int queryIndex=0;

  while(query[index] != 0){
    int length = query[index];
    index+=1;

    for(int i=0; i<length; i++){
      queryName[queryIndex] = query[index];
      index+=1;queryIndex += 1;
    }
    queryName[queryIndex++] = '.';
  }
  // Undo the last .
  queryName[queryIndex-1] = '\0';


  return queryName;
}

int getRecordType(DNSResponse* res){
  Byte* stream = res->bytes;

  int packetLen = PACKET_LEN;
  int header = 12;
  int queries = 5; //1: Null Terminator , 2 types , 2 class
  int offset = packetLen + header;
  int nameLen = 2;
  while(stream[offset] != 0){
    offset++;
  }
  offset += (queries + nameLen);

  Byte recordType[2];
  memcpy(recordType, res->bytes + offset ,2 );

  return hexToDec(recordType);
}

char* formatIPv6Addr(char* ipv6Addr){
  int rval;
  struct in6_addr in6addr;
  char *buf6 = (char*)calloc(INET6_ADDRSTRLEN,sizeof(char));

  if ((rval = inet_pton(AF_INET6, ipv6Addr, &in6addr)) == 0) {
      printf("Invalid address: %s\n", ipv6Addr);
   } else if (rval == -1) {
      perror("inet_pton");
   }
   
   if (inet_ntop(AF_INET6, &in6addr, buf6, sizeof(char)*INET6_ADDRSTRLEN) != NULL){
   }
   else {
      perror("inet_ntop");
   }

  return buf6;

}


int getIPAddressOffset(Byte *stream){
  int packetLen = PACKET_LEN;
  int header = 12;

  int queries = 5; //1: Null Terminator , 2 types , 2 class
  
  int offset = packetLen + header;
  while(stream[offset] != 0){
    offset++;
  }
  offset += queries;

  //Start of Answer 
  int nameLen = 2, recordTypeLen = 2, classLen = 2, ttlLen = 4, dateLen = 2;
  offset += (nameLen + recordTypeLen + classLen + ttlLen + dateLen);

  return offset;
}


void handleInvalidRequest(int clientSockFd, DNSRequest *req){
  int n;
  
  // Set QR to Response
  req->bytes[4] = req->bytes[4] | 0x80; 

  //Set RCODE = 4 NOT IMPLEMENTED AND RECURSION AVAILABLE TO 4
  req->bytes[5] = req->bytes[5] |0x84;

  n = write(clientSockFd, req->bytes, req->bytesLength + PACKET_LEN);
  if (n < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
}


void sendPacketToClient(int clientSockFd, DNSResponse *res){
  int n;
  n = write(clientSockFd, res->bytes, res->bytesLength+2);
  if (n < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
}



// Read a segment of Ipv6 --> 0xff:0xff = only for 0xff which is 2 byte
char* readAddrByte(Byte byte1, Byte byte2){
  char *acc  = (char*)malloc(sizeof(char)*5);
  acc[0] = (byte1>>4) + '0' ;
  acc[1] = (byte1 & 0x0F) + '0' ;
  acc[2] = (byte2>>4) + '0' ;
  acc[3] = (byte2 & 0x0F) + '0' ;
  acc[4] = ':';

  return acc;
}
