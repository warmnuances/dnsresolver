#ifndef DNS_H
#define DNS_H

#include "utils.h"
// typedef struct dnsResponse DNSResponse ;
// typedef struct dnsHeader DNSHeader;

typedef struct dnsRequest DNSRequest;
typedef struct dnsHeader DNSHeader;
typedef struct dnsFlags DNSFlags;

struct dnsRequest{
  unsigned char* bytes;
  int bytesLength;
  int queryType;
};

struct dnsHeader{
  //Transaction ID
  unsigned char* txId;
  //Query Response
  DNSFlags *flags;
  
};

struct dnsFlags{
  int qr;
  int opcode;
  //Authoritative ans
  int AA;
  //Truncated content
  int tr;
  //Recursion desired
  int rd;
  //Response code
  int rc;
};



DNSRequest* serializeRequest(Utils* util);
DNSHeader* serializeHeader(Utils* util, unsigned char* header);

#endif
