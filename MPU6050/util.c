#include "util.h"

void initCont(contLimit *contLimit, int limit)
{
	contLimit->cont=0;
	contLimit->limit=limit;
}

int checkCont(contLimit *contLimit)
{
	if (contLimit->cont==contLimit->limit)
	{
		contLimit->cont=0;
		return TRUE;
	}
	contLimit->cont=contLimit->cont+1;
	return FALSE;
}

void beginTimeInterval(timeControl* tc)
{
	struct timespec start;
	clock_gettime(CLOCK_REALTIME, &start);
	
	tc->start.tv_sec=start.tv_sec;
	tc->start.tv_nsec=start.tv_nsec;
}

void newTimeInterval(timeControl* tc)
{
	struct timespec stop;
	struct timespec start;
	long temp1,temp2;
	
	clock_gettime(CLOCK_REALTIME, &stop);
	
	tc->end.tv_sec=stop.tv_sec;
	tc->end.tv_nsec=stop.tv_nsec;
	
	temp1=(tc->start.tv_sec*1000000000)+tc->start.tv_nsec;
	temp2=(tc->end.tv_sec*1000000000)+tc->end.tv_nsec;
	
	tc->lastSecondsIncrement=(float)(temp2-temp1)/1000000000;
	
	clock_gettime(CLOCK_REALTIME, &start);
	
	tc->start.tv_sec=start.tv_sec;
	tc->start.tv_nsec=start.tv_nsec;
}

void endTimeInterval(timeControl* tc)
{
	struct timespec stop;
	long temp1,temp2;
	
	clock_gettime(CLOCK_REALTIME, &stop);
	
	tc->end.tv_sec=stop.tv_sec;
	tc->end.tv_nsec=stop.tv_nsec;
	
	temp1=(tc->start.tv_sec*1000000000)+tc->start.tv_nsec;
	temp2=(tc->end.tv_sec*1000000000)+tc->end.tv_nsec;
	
}

void getTimeInterval(timeControl* tc)
{
	long temp1;
	long temp2;
	
	temp1=(tc->start.tv_sec*1000000000)+tc->start.tv_nsec;
	temp2=(tc->end.tv_sec*1000000000)+tc->end.tv_nsec;
	
	tc->lastSecondsIncrement=(float)(temp2-temp1)/1000000000;
}

int BindSocket(int* serverSocket, int port)
{
	struct sockaddr_in server;
	
	*serverSocket = socket(AF_INET,SOCK_STREAM,0);
	if (*serverSocket==ERROR_VALUE)
	{
		printf("UTIL: Could not create socket");
		return ERROR_VALUE;
	}
	printf("UTIL: Socket created\n");
	
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(8888);
	
	if(bind(*serverSocket,(struct sockaddr *)&server,sizeof(server))<0)
	{
		perror("UTIL: bind failed. Error");
		return ERROR_VALUE;
	}
	printf("UTIL: Bind done on port %i\n",port);
}

/*
	int client_sock , c , read_size;
	char client_message[2000];
	struct sockaddr_in client;

	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	puts("Connection accepted");
	
	//Receive a message from client
	while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
	{
		//Send the message back to client
		write(client_sock , client_message , strlen(client_message));
	}
	
	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
	
	return 0;
}
*/

int connectClientSocket(int *clientSocket,char *IP,int port)
{
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    *clientSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (*clientSocket == -1)
    {
        //perror("UTIL: Could not create socket");
		return ERROR_VALUE;
    }
    //puts("UTIL: Socket created");
     
    server.sin_addr.s_addr = inet_addr(IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
 
    //Connect to remote server
    if (connect(*clientSocket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //perror("UTIL: Error connecting socket");
        return ERROR_VALUE;
    }
     
    printf("UTIL: Connected to %s:%i\n",IP,port);
}

int endClientSocket(int *clientSocket)
{
	close(*clientSocket);
	return 0;
}

int clientSendReceive (int clientSocket,char *message)
{
	//Send some data
	if(send(clientSocket,message,strlen(message),0)<0)
    {
        //perror("UTIL: Error sending data from client");
        return ERROR_VALUE;
    }

/*	
	//Receive a reply from the server
	if( recv(sock , server_reply , 2000 , 0) < 0)
	{
		puts("recv failed");
		break;
	}
         
	puts("Server reply :");
	puts(server_reply);
*/
}