#include "utils.h"


int hexToDec(unsigned char* buffer){
  return (int)buffer[0]*256 + (int)buffer[1];
}

char* getTimeStamp(){
  time_t t ;
  struct tm *tmp ;
  char *buf = (char*)malloc(sizeof(char)*TIME_BUFFER);
  time(&t);
  tmp = localtime(&t);

  strftime(buf, sizeof(char)*TIME_BUFFER, "%FT%T%z", tmp);

  return buf;
}


void createFile(){
  char* filePath = "dns_svr.log";
  FILE *fp;

  //Check if file exists, delete it
  // if(access( filePath, F_OK ) == 0 ) {
  //   remove(filePath);
  // }

  // Create a new file
  fp = fopen(filePath, "w+");
  fclose(fp);

}
// Created file will have full wrx
void writeTofile(char* append){
  char* filePath = "dns_svr.log";
  FILE *fPtr;

  fPtr = fopen(filePath, "a");

  if (fPtr == NULL)
  {
      /* Unable to open file hence exit */
      printf("\nUnable to open '%s' file.\n", filePath);
      printf("Please check whether file exists and you have write privilege.\n");
  }

  // To clear extra white space characters in stdin
  fputs(append, fPtr);

  fclose(fPtr);
}


FILE *getFile(){
  char* filePath = "dns_svr.log";
  FILE *fPtr;
  fPtr = fopen(filePath, "a");

  if (fPtr == NULL)
  {
      /* Unable to open file hence exit */
      printf("\nUnable to open '%s' file.\n", filePath);
      printf("Please check whether file exists and you have write privilege.\n");
  }

  return fPtr;
}

void printBytes(Byte* bytes, int length){
  printf("Length: %d\n", length);
  for(int i=0; i< length; i++){
    printf("%d ", bytes[i]);
  }
  printf("\n");
}
