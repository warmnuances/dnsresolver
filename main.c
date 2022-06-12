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
#include <time.h>

#include "server.h"


int main(int argc, char** argv) {

  if(argc < 3){
		fprintf(stderr, "ERROR, Not enough Argument supplied\n");
		exit(EXIT_FAILURE);
	}

  // Create a file
  createFile();

  int sockfd;
  sockfd = initServer("8053");	

  // Connect to upstream server

  //Weird Bug fix. Basically, when calloc/malloc, the memory starts with 0.
  //Somehow calling localtime() will probably initialise memory to some number
  getTimeStamp();


  // Read characters from the connection, then process
	while (1) {

	  int clientSockfd = createClientSession(sockfd);
    int upstreamfd = connectToUpstreamPersistent(argv);


    Byte lenByte[2];
    int nlen = read(clientSockfd, lenByte,2); 
    if (nlen < 0) {
      perror("ERROR reading from socket");
      exit(EXIT_FAILURE);
    }
    int length = hexToDec(lenByte);

    
    int index = 0;
    Byte *reqStream = (Byte*)calloc(length+2,sizeof(Byte));
    for(int i = 0; i < 2 ; i++){
      reqStream[i] = lenByte[i];
    }
    index += 2;
    int toRead = length;

    while(toRead > 0){
      Byte buf[1];
      int hasRead = read(clientSockfd, buf, 1);
    
      if(hasRead != 0){
        toRead = toRead - 1;
        reqStream[index]  = buf[0];
        index += 1;
      }
    }
    reqStream[index] = '\0';  


    DNSRequest* request = serializeRequest(reqStream);
    int isValidRequest = logRequest(request);

    if(isValidRequest){
      // Connecting to upstream server
      // sendToUpstreamServer(request, upstreamfd);
      int n;
      n = write(upstreamfd,reqStream, length + 2);
      if (n < 0) {
        perror("socket");
      }


      DNSResponse *response =  handleResponse(upstreamfd);
      int isValidResponse = logResponse(response);
      if(isValidResponse){
        sendPacketToClient(clientSockfd, response);
      }else{
        response->bytes[5] = response->bytes[5] | 0x84;
        sendPacketToClient(clientSockfd, response);
      }
   

    }
    else{
       handleInvalidRequest(clientSockfd,request);
    }

    close(upstreamfd);
		close(clientSockfd);
	}

	close(sockfd);
	return 0;
}


