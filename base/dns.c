#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


#include "dns.h"
#define NO_OF_BITS 8
#define RESPONSE 1
#define REQUEST 0
#define HEADER_BYTES 12
#define MASK 1

DNSFlags* serializeFlags(Utils* util, unsigned char* flagBytes){
  DNSFlags* flag = (DNSFlags*)malloc(sizeof(DNSFlags*));
  

  // & 0x80 which is 1000000 & x--------
  flag->qr = flagBytes[0] & util->conversion->getByteFromIndex(7);
  util->conversion->printData(flagBytes, 2);

  // 4 bit flag 

  return flag;
}


DNSHeader* serializeHeader(Utils* util, unsigned char* header){
  DNSHeader* head = (DNSHeader*)malloc(sizeof(DNSHeader*));
  // DNSFlags* flag = (DNSFlags*)malloc(sizeof(DNSFlags*));

  unsigned char txId[2] = {header[0], header[1]};
  head->txId = txId;
  // util->conversion->printData(txId, 2);


  // DNSFlags* flag = (DNSFlags*)malloc(sizeof(DNSFlags*));
  unsigned char flagByte[2] = {header[2], header[3]};
  serializeFlags(util, flagByte);





  return head;

}

// Semi Serialize not full serialization
DNSRequest* serializeRequest(Utils* util){
  DNSRequest* req = (DNSRequest*)malloc(sizeof(DNSRequest*));


  // Read first 2 bytes, it tells us about the length
  unsigned char buffer[2];
  if(read(STDIN_FILENO, &buffer, 2) > 0){
    req->bytesLength = util->conversion->hexToDec(buffer);
  }

  // Read whatever bytes that comes.
  unsigned char data[req->bytesLength];

  if(read(STDIN_FILENO, &data, req->bytesLength) > 0){
    req->bytes = data;
  }

  //Populate header
  unsigned char header[12];
  memcpy(header,data,12);
  serializeHeader(util, header);



  return req;

}


int main(int argc, char const *argv[])
{
  Utils* utils = initUtils();
  DNSRequest *req = serializeRequest(utils);

  
  printf("Byteslength: %d \n", req->bytesLength);


  free(req);

  return 0;
}

