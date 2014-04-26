#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include <stdint.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#define TRUE 1
#define FALSE 0

#define OK_VALUE 0
#define ERROR_VALUE -1

typedef struct 
{
	struct timespec start;
	struct timespec end;
	float lastSecondsIncrement;
} timeControl;

typedef struct
{
	int cont;
	int limit;
} contLimit;

long timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);
void beginTimeInterval(timeControl* tc);
void endTimeInterval(timeControl* tc);
void newTimeInterval(timeControl* tc);
void getTimeInterval(timeControl* tc);

void initCont(contLimit *contLimit, int limit);
int checkCont(contLimit *contLimit);

int connectClientSocket(int *clientSocket,char *IP,int port);
int clientSendReceive (int clientSocket,char *message);

int BindSocket(int* serverSocket, int port);

#endif