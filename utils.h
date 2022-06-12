#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>


typedef unsigned char Byte;

#define TIME_BUFFER 26
#define TRUE 1
#define FALSE 0


int hexToDec(unsigned char* buffer);
char* getTimeStamp();
void printBytes(Byte* bytes, int length);
void createFile();
void writeTofile(char* append);
FILE *getFile();



#endif

