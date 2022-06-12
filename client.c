
#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef unsigned char Byte;

typedef struct _byteStream ByteStream;

struct _byteStream{
  Byte* stream;
  int length;
};


void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

// void delay(int number_of_seconds)
// {
//     // Converting time into milli_seconds
//     int milli_seconds = 1000 * number_of_seconds;
  
//     // Storing start time
//     clock_t start_time = clock();
  
//     // looping till required time is not achieved
//     while (clock() < start_time + milli_seconds)
//         ;
// }

ByteStream* readFile(){
  int i;
  FILE *fileptr;
  char *buffer;
  long filelen;

  fileptr = fopen("packets/1.comp30023.req.raw", "rb");  // Open the file in binary mode
  fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
  filelen = ftell(fileptr);             // Get the current byte offset in the file
  rewind(fileptr);                      // Jump back to the beginning of the file

  buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
  fread(buffer, filelen * sizeof(char), 1, fileptr); // Read in the entire file

  
  
  ByteStream* bs = (ByteStream*)malloc(sizeof(ByteStream));
  bs->stream = (Byte*)malloc(sizeof(Byte)*filelen);
  Byte *bytes = (Byte*)malloc(sizeof(Byte)*filelen);

  for(i = 0 ; i< filelen; i++){
    bytes[i] = (Byte)buffer[i];  
  }

  bytes[i-1] = '\n';

  bs->stream = bytes;
  bs->length = filelen;


  fclose(fileptr); // Close the file


  return bs;

}



int main(int argc, char** argv) {
  ByteStream* bs = readFile();

	int sockfd, n, s;
	struct addrinfo hints, *servinfo, *rp;

	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Create address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Get addrinfo of server. From man page:
	// The getaddrinfo() function combines the functionality provided by the
	// gethostbyname(3) and getservbyname(3) functions into a single interface
	s = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// Connect to first valid result
	// Why are there multiple results? see man page (search 'several reasons')
	// How to search? enter /, then text to search for, press n/N to navigate
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



	printf("Length: %d \nReq ", bs->length);
	for(int i =0 ; i< bs->length ; i++){
		printf("%d ", bs->stream[i]);
	}
	
	ByteStream* stream1 = (ByteStream*)malloc(sizeof(Byte)* 24);
	ByteStream* stream2 = (ByteStream*)malloc(sizeof(Byte)* (bs->length - 24));

	memcpy(stream1, bs->stream, 24);
	memcpy(stream2, bs->stream + 24, (bs->length - 24));


	// Send message to server
	n = write(sockfd, stream1, 24 );
	if (n < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	waitFor(5);

	n = write(sockfd, stream2, (bs->length - 24));
	if (n < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}







	unsigned char wbuffer[1024];

	//Get the request back
	n = read(sockfd, wbuffer, 1024);
	// if (n == 0) {
	// 	break;
	// }
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	printf("\nRESP: %d \n ",n);
	// Null-terminate string
	wbuffer[n] = '\0';
	for(int i =0 ; i<n  ; i++){
		printf("%d ", wbuffer[i]);
	}



	fflush(stdin);
	fflush(stdout);

	close(sockfd);


	return 0;
}
