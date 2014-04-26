#include "MPU6050.h"

#define I2CDEV "/dev/i2c-1"
#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64

// CONFIGURATION
#define DYNAMIC_OFFSET FALSE
#define COMPLEMENTARY_FILTER_GYRO 0.95
#define COMPLEMENTARY_FILTER_ACCEL 0.05

#define ACCELOFFSET_X 802 
#define ACCELOFFSET_Y -368 
#define ACCELOFFSET_Z -1195 
#define GYROOFFSET_X -482
#define GYROOFFSET_Y 252
#define GYROOFFSET_Z -69

static float angleX,angleY,angleZ;

long timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
	return ((timeA_p->tv_sec*1000000000)+timeA_p->tv_nsec)-((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

int OpenMPU6050()
{
	int device;
	device=OpenI2Cdevice(I2CDEV,MPU6050_ADDRESS);
	return device;
}


int TestMPU6050(int device)
{
	int returnValue;
	returnValue = ReadRegister(device,MPU6050_RA_WHO_AM_I);

	if (returnValue==0x68)
	{
		printf("MPU6050: device test OK\n");
		return OK_VALUE;
	}
	else
	{
		perror("MPU6050: device test FAILED!\n");
		return ERROR_VALUE;
	}
}

int InitMPU6050(int device)
{
	int returnValue;
	returnValue=WriteRegister(device,MPU6050_RA_PWR_MGMT_1,0);
	if (returnValue==ERROR_VALUE)
	{
		perror("MPU6050: Error writing SLEEP register\n");
		return ERROR_VALUE;
	}
	
	returnValue = ReadRegister(device,MPU6050_RA_PWR_MGMT_1);
	if ((returnValue&BIT6)==0)
	{
		printf("MPU6050: device awaken\n");
		return OK_VALUE;
	}
	else
	{
		perror("MPU6050: Error initializing device\n");
		return ERROR_VALUE;
	}
}

int ReadGyroAccelValues (int device, MPU6050DataStruct* data)
{
	char outputs[14];
	
	ReadRegisters(device,MPU6050_RA_ACCEL_XOUT_H,14,outputs);
	
	data->aX = (outputs[0]<<8)|outputs[1];
	data->aY = (outputs[2]<<8)|outputs[3];
	data->aZ = (outputs[4]<<8)|outputs[5];
	// temp = (outputs[6]<<8)|outputs[7];
	data->gX = (outputs[8]<<8)|outputs[9];
	data->gY = (outputs[10]<<8)|outputs[11];
	data->gZ = (outputs[12]<<8)|outputs[13];
	
	return OK_VALUE;
}

int CalibrateAccel (int device, int samples, MPU6050DataStruct* offsets)
{
	char outputs[14];
	int cont;
	long sumX,sumY,sumZ;
	MPU6050DataStruct tempRawData;
	
	sumX=sumY=sumZ=0;
	
	for (cont=0;cont<200;cont++)
	{
		ReadGyroAccelValues(device,&tempRawData);
		sumX += tempRawData.aX;
		sumY += tempRawData.aY;
		sumZ += tempRawData.aZ-(-16384);
	}
	
	offsets->aX=sumX/cont;
	offsets->aY=sumY/cont;
	offsets->aZ=sumZ/cont;
	
	return OK_VALUE;
}

int CalibrateGyro (int device, int samples, MPU6050DataStruct* offsets)
{
	char outputs[14];
	int cont;
	long sumX,sumY,sumZ;
	
	sumX=sumY=sumZ=0;
	
	for (cont=0;cont<200;cont++)
	{
		ReadRegisters(device,MPU6050_RA_ACCEL_XOUT_H,14,outputs);
		sumX += (outputs[8]<<8)|outputs[9];
		sumY += (outputs[10]<<8)|outputs[11];
		sumZ += (outputs[12]<<8)|outputs[13];	
	}
	
	offsets->gX=sumX/cont;
	offsets->gY=sumY/cont;
	offsets->gZ=sumZ/cont;
	
	return OK_VALUE;
}

int AssignPreCalculatedOffsets (MPU6050DataStruct* offsets)
{
	offsets->aX=ACCELOFFSET_X;
	offsets->aY=ACCELOFFSET_Y;
	offsets->aZ=ACCELOFFSET_Z;
	offsets->gX=GYROOFFSET_X;
	offsets->gY=GYROOFFSET_Y;
	offsets->gZ=GYROOFFSET_Z;
	
	return OK_VALUE;
}

int CalculateAccelAngles (int device)
{
	MPU6050DataStruct MPU6050Data;
	MPU6050DataStruct MPU6050Offsets;
	
	float angleX,angleY,angleZ,temp;
	
	if (DYNAMIC_OFFSET==TRUE)
	{
		printf("MPU6050: Calibrating gyro... ");
		CalibrateGyro(device,1000,&MPU6050Offsets);
		printf("done\n");
	}
	else
	{
		printf("MPU6050: Assigning pre-calculated offset... ");
		AssignPreCalculatedOffsets(&MPU6050Offsets);
		printf("done\n");
	}
	
	for(;;)
	{
		ReadGyroAccelValues(device,&MPU6050Data);
	
		MPU6050Data.aX-=MPU6050Offsets.aX;
		MPU6050Data.aY-=MPU6050Offsets.aY;
		MPU6050Data.aZ-=MPU6050Offsets.aZ;

		angleX = (float)atan(MPU6050Data.aX/sqrt(pow(MPU6050Data.aY,2)+pow(MPU6050Data.aZ,2)));
		angleY = (float)atan(MPU6050Data.aY/sqrt(pow(MPU6050Data.aX,2)+pow(MPU6050Data.aZ,2)));
		angleZ = (float)atan(sqrt(pow(MPU6050Data.aX,2)+pow(MPU6050Data.aY,2))/MPU6050Data.aZ);	

		// FIX (?)
		temp = angleY;
		angleY = angleX;
		angleX = -temp;
		
		printf("%.6f\t%.6f\t%.6f\t\n",angleX*180/M_PI,angleY*180/M_PI,angleZ*180/M_PI);
	/*	
		printf("aX\t%i\t%i\n",MPU6050Data.aX,MPU6050Data.aX/16384);
		printf("aY\t%i\t%i\n",MPU6050Data.aY,MPU6050Data.aY/16384);
		printf("aZ\t%i\t%i\n",MPU6050Data.aZ,MPU6050Data.aZ/16384);
	
		printf("gX\t%i\t%i\n",MPU6050Data.gX,MPU6050Data.gX/131);
		printf("gY\t%i\t%i\n",MPU6050Data.gY,MPU6050Data.gY/131);
		printf("gZ\t%i\t%i\n",MPU6050Data.gZ,MPU6050Data.gZ/131);
		*/
	}
}



int CalculateGyroAngles (int device)
{
	MPU6050DataStruct MPU6050Data;
	MPU6050DataStruct MPU6050Offsets;
	//float angleX,angleY,angleZ;
	float incX,incY,incZ;

	timeControl loopControl;
	contLimit printContLimit;
	
	initCont(&printContLimit,5);
	
	if (DYNAMIC_OFFSET==TRUE)
	{
		printf("MPU6050: Calibrating gyro... ");
		CalibrateGyro(device,1000,&MPU6050Offsets);
		printf("done\n");
	}
	else
	{
		printf("MPU6050: Assigning pre-calculated offset... ");
		AssignPreCalculatedOffsets(&MPU6050Offsets);
		printf("done\n");
	}
	
	MPU6050Data.angleX=0;
	MPU6050Data.angleY=0;
	MPU6050Data.angleZ=0;
	ReadGyroAccelValues(device,&MPU6050Data);		
	beginTimeInterval(&loopControl);
	
	for(;;)
	{
		endTimeInterval(&loopControl);
		getTimeInterval(&loopControl);
		ReadGyroAccelValues(device,&MPU6050Data);		
		beginTimeInterval(&loopControl);
		usleep(15000); // INCTIME=0.02
		MPU6050Data.gX-=MPU6050Offsets.gX;
		MPU6050Data.gY-=MPU6050Offsets.gY;
		MPU6050Data.gZ-=MPU6050Offsets.gZ;
		
		
		
		incX=(float)MPU6050Data.gX/131*loopControl.lastSecondsIncrement;
		incY=(float)MPU6050Data.gY/131*loopControl.lastSecondsIncrement;
		incZ=(float)MPU6050Data.gZ/131*loopControl.lastSecondsIncrement;

		if (checkCont(&printContLimit))
		{
			printf("ANGLES: %.2f\t%.2f\t%.2f\t TIME=%.2f\n",(float)MPU6050Data.angleX,(float)MPU6050Data.angleY,(float)MPU6050Data.angleZ,loopControl.lastSecondsIncrement);
			//printf("RAW:  %i\t%i\t%i\n",MPU6050Data.gX,MPU6050Data.gY,MPU6050Data.gZ);
			printf("OFFSET: %i\t%i\t%i\n",MPU6050Offsets.gX,MPU6050Offsets.gY,MPU6050Offsets.gZ);
			printf("DEG/SG: %.2f\t%.2f\t%.2f\n",(float)MPU6050Data.gX/131,(float)MPU6050Data.gY/131,(float)MPU6050Data.gZ/131);
			printf("INCANG: %.2f\t%.2f\t%.2f\n",(float)incX,(float)incY,(float)incZ);
			//printf("ANGLES: %.2f\t%.2f\t%.2f\n",(float)MPU6050Data.angleX,(float)MPU6050Data.angleY,(float)MPU6050Data.angleZ);
			//printf("TIEMPO: %.4f, ANGULOX: %.4f, ANGULOY: %.4f, ANGULOZ: %.4f\n",(float)incTime,(float)MPU6050Data.gX/131,(float)MPU6050Data.gY/131,(float)MPU6050Data.gZ/131);
			//printf("\n");
		}

		printf("AAAAAAAAAA: %.6f\t %.6f\t %.6f\n",MPU6050Data.angleX,incX,MPU6050Data.angleX+incX);
		MPU6050Data.angleX=(float)MPU6050Data.angleX+incX;
		MPU6050Data.angleY=(float)MPU6050Data.angleY+incY;
		MPU6050Data.angleZ=(float)MPU6050Data.angleZ+incZ;

			


	}	
}



int CalculateAngles (int device,int interval)
{
	MPU6050DataStruct MPU6050Data;
	MPU6050DataStruct MPU6050Offsets;
	
	timeControl loopControl;
	contLimit printContLimit;

	float accelAngleX,accelAngleY,temp;
	float gyroAngleX,gyroAngleY,gyroAngleZ;
	float gyroDeltaX,gyroDeltaY,gyroDeltaZ;
	
	float gyroAngleTempX,gyroAngleTempY,gyroAngleTempZ;
	
	
	initCont(&printContLimit,100000/interval);	
	
	if (DYNAMIC_OFFSET==TRUE)
	{
		printf("MPU6050: Calibrating gyro... ");
		CalibrateGyro(device,1000,&MPU6050Offsets);
		printf("done\n");
	}
	else
	{
		printf("MPU6050: Assigning pre-calculated offset... ");
		AssignPreCalculatedOffsets(&MPU6050Offsets);
		printf("done\n");
	}
	
	// INITIAL ANGLE
	usleep(1000000);
	ReadGyroAccelValues(device,&MPU6050Data);
	beginTimeInterval(&loopControl);
	
	MPU6050Data.aX-=MPU6050Offsets.aX;
	MPU6050Data.aY-=MPU6050Offsets.aY;
	MPU6050Data.aZ-=MPU6050Offsets.aZ;
	
	MPU6050Data.angleX = (float)atan(MPU6050Data.aX/sqrt(pow(MPU6050Data.aY,2)+pow(MPU6050Data.aZ,2)));
	MPU6050Data.angleX = MPU6050Data.angleX*180/M_PI;
	MPU6050Data.angleY = (float)atan(MPU6050Data.aY/sqrt(pow(MPU6050Data.aX,2)+pow(MPU6050Data.aZ,2)));
	MPU6050Data.angleY = MPU6050Data.angleY*180/M_PI;
	MPU6050Data.angleZ = (float)0;

	gyroAngleTempX=MPU6050Data.angleX;
	gyroAngleTempY=MPU6050Data.angleY;
	gyroAngleTempZ=MPU6050Data.angleZ;
		
	for (;;)
	{
		//if (checkCont(&printContLimit))
		//	printf("%.2f\t%.2f %.2f ??.??\t%.2f %.2f %.2f \t%.2f %.2f %.2f\n",loopControl.lastSecondsIncrement,(float)accelAngleX,(float)accelAngleY,(float)gyroAngleTempX,(float)gyroAngleTempY,(float)gyroAngleTempZ,(float)MPU6050Data.angleX,(float)MPU6050Data.angleY,(float)MPU6050Data.angleZ);

		// printf("OFFSET: %i\t%i\t%i\n",MPU6050Offsets.gX,MPU6050Offsets.gY,MPU6050Offsets.gZ);
		// printf("RW-OF:  %i\t%i\t%i\n",MPU6050Data.gX,MPU6050Data.gY,MPU6050Data.gZ);
		// printf("DEG/SG: %.2f\t%.2f\t%.2f\n",(float)MPU6050Data.gX/131,(float)MPU6050Data.gY/131,(float)MPU6050Data.gZ/131);
		// printf("INCANG: %.2f\t%.2f\t%.2f\n",(float)gyroDeltaX,(float)gyroDeltaY,(float)gyroDeltaZ);
		// printf("ANGLES: %.2f\t%.2f\t%.2f\n",(float)MPU6050Data.angleX,(float)MPU6050Data.angleY,(float)MPU6050Data.angleZ);
		// printf("%.2f\t%.2f\t??.??\t\t%.2f\t%.2f\t%.2f\n",(float)accelAngleX,(float)accelAngleY,(float)gyroAngleX,(float)gyroAngleY,(float)gyroAngleZ);
			
		endTimeInterval(&loopControl);
		getTimeInterval(&loopControl);
		ReadGyroAccelValues(device,&MPU6050Data);		
		beginTimeInterval(&loopControl);
		
		MPU6050Data.aX-=MPU6050Offsets.aX;
		MPU6050Data.aY-=MPU6050Offsets.aY;
		MPU6050Data.aZ-=MPU6050Offsets.aZ;
		
		accelAngleX = (float)atan(MPU6050Data.aX/sqrt(pow(MPU6050Data.aY,2)+pow(MPU6050Data.aZ,2)));
		accelAngleY = (float)atan(MPU6050Data.aY/sqrt(pow(MPU6050Data.aX,2)+pow(MPU6050Data.aZ,2)));
		
		accelAngleX = accelAngleX*180/M_PI;
		accelAngleY = accelAngleY*180/M_PI;
		
		// FIX (?)
		temp = accelAngleY;
		accelAngleY = accelAngleX;
		accelAngleX = -temp;

		MPU6050Data.gX-=MPU6050Offsets.gX;
		MPU6050Data.gY-=MPU6050Offsets.gY;
		MPU6050Data.gZ-=MPU6050Offsets.gZ;
		
		gyroDeltaX=(float)MPU6050Data.gX/131*loopControl.lastSecondsIncrement;
		gyroDeltaY=(float)MPU6050Data.gY/131*loopControl.lastSecondsIncrement;
		gyroDeltaZ=(float)MPU6050Data.gZ/131*loopControl.lastSecondsIncrement;
		
		gyroAngleTempX+=(float)gyroDeltaX;
		gyroAngleTempY+=(float)gyroDeltaY;
		gyroAngleTempZ+=(float)gyroDeltaZ;

		MPU6050Data.angleX=COMPLEMENTARY_FILTER_GYRO*(MPU6050Data.angleX+gyroDeltaX)+COMPLEMENTARY_FILTER_ACCEL*accelAngleX;
		MPU6050Data.angleY=COMPLEMENTARY_FILTER_GYRO*(MPU6050Data.angleY+gyroDeltaY)+COMPLEMENTARY_FILTER_ACCEL*accelAngleY;
		MPU6050Data.angleZ=MPU6050Data.angleZ+gyroDeltaZ;
		
		angleX=MPU6050Data.angleX;
		angleY=MPU6050Data.angleY;
		angleZ=MPU6050Data.angleZ;

		usleep(interval);
	}
}

void* THREAD_Server()
{
    int serverSocket , clientSocket , c;
    struct sockaddr client;
    char serverMessage[200];
	char clientMessage[200];
	int numClients;
	
	memset(serverMessage,'\0', sizeof(serverMessage));
	memset(clientMessage,'\0', sizeof(clientMessage));
	
	BindServer(&serverSocket,8888);
	
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     		
	//accept connection from an incoming client
	clientSocket = accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&c);
	if (clientSocket < 0)
	{
		perror("accept failed");
		return;
	}

	printf("Connection accepted\n");
	sprintf(serverMessage,"%.2f %.2f %.2f",(float)angleX,(float)angleY,(float)angleZ);		

	while (1)
	{
		recv(clientSocket,clientMessage,2000,0);
		write(clientSocket,serverMessage,strlen(serverMessage));
	}

    return 0;
}
	

int BindServer(int* serverSocket, int port)
{
	struct sockaddr_in server;
	int option;
	
	*serverSocket = socket(AF_INET,SOCK_STREAM,0);
    if (*serverSocket == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
     
	if(setsockopt(*serverSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(option))< 0)
	{
		printf("setsockopt failed\n");
		return;
	}

	//Bind
    if (bind(*serverSocket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return;
    }
    puts("bind done");
    
    //Listen
    listen(*serverSocket,5);
}
     

int main()
{
        int status;
        pthread_t tid1,tid2;

        pthread_create(&tid1,NULL,THREAD_CalculateAngles,NULL);
        pthread_create(&tid2,NULL,THREAD_Server,NULL);
        pthread_join(tid1,NULL);
        pthread_join(tid2,NULL);
        return 0;
}


void* THREAD_CalculateAngles()
{
	int MPU6050;
	int returnValue;
	MPU6050DataStruct MPU6050RawData;
	MPU6050DataStruct MPU6050Offsets;
	
	MPU6050=OpenMPU6050();
	if (MPU6050!=ERROR_VALUE) printf("MPU6050: device correctly opened\n");
	else 
	{
		perror("MPU6050: Error opening MPU6050 device");
		return;
	}
	
	returnValue=TestMPU6050(MPU6050);
	if (returnValue!=ERROR_VALUE) printf("MPU6050: Test passed\n");
	else 
	{
		perror("MPU6050: Error testing device");
		return;
	}

	returnValue=InitMPU6050(MPU6050);
	if (returnValue!=ERROR_VALUE) printf("MPU6050: Initialization correct\n");
	else 
	{
		perror("MPU6050: Error initializing device");
		return;
	}	
	
	CalculateAngles(MPU6050,15000);
	//CalculateAccelAngles(MPU6050);
	//CalculateGyroAngles(MPU6050);
	
	/*
	int j=0;
	int aX,aY,aZ,gX,gY,gZ;
	aX=aY=aZ=gX=gY=gZ=0;
	for (j=0;j<20;j++)
	{
		CalibrateGyro(MPU6050,1000,&MPU6050Offsets);
		CalibrateAccel(MPU6050,1000,&MPU6050Offsets);
		aX+=MPU6050Offsets.aX;
		aY+=MPU6050Offsets.aY;
		aZ+=MPU6050Offsets.aZ;
		gX+=MPU6050Offsets.gX;
		gY+=MPU6050Offsets.gY;
		gZ+=MPU6050Offsets.gZ;
		//printf("%i %i %i %i %i %i\n",aX/(j+1),aY/(j+1),aZ/(j+1),gX/(j+1),gY/(j+1),gZ/(j+1));
	}
	
	printf("Average Offset: %i %i %i %i %i %i\n",aX/20,aY/20,aZ/20,gX/20,gY/20,gZ/20);
	
	
	CalibrateGyro(MPU6050,1000,&MPU6050Offsets);
	CalibrateAccel(MPU6050,1000,&MPU6050Offsets);
	
	ReadGyroAccelValues(MPU6050,&MPU6050RawData);
	
	printf("aX\t%i\t%i\n",MPU6050RawData.aX,MPU6050RawData.aX/16384);
	printf("aY\t%i\t%i\n",MPU6050RawData.aY,MPU6050RawData.aY/16384);
	printf("aZ\t%i\t%i\n",MPU6050RawData.aZ,MPU6050RawData.aZ/16384);

	printf("gX\t%i\t%i\n",MPU6050RawData.gX,MPU6050RawData.gX/131);
	printf("gY\t%i\t%i\n",MPU6050RawData.gY,MPU6050RawData.gY/131);
	printf("gZ\t%i\t%i\n",MPU6050RawData.gZ,MPU6050RawData.gZ/131);

	
	printf("OFFSETS\n");
	printf("G\t%i\t%i\t%i\n",MPU6050Offsets.gX,MPU6050Offsets.gY,MPU6050Offsets.gZ);
	printf("A\t%i\t%i\t%i\n",MPU6050Offsets.aX,MPU6050Offsets.aY,MPU6050Offsets.aZ);
	
	printf("aX\t%.2f\t%.2f\n",(float)MPU6050RawData.aX/16384,(float)(MPU6050RawData.aX-MPU6050Offsets.aX)/16384);
	printf("aY\t%.2f\t%.2f\n",(float)MPU6050RawData.aY/16384,(float)(MPU6050RawData.aY-MPU6050Offsets.aY)/16384);
	printf("aZ\t%.2f\t%.2f\n",(float)MPU6050RawData.aZ/16384,(float)(MPU6050RawData.aZ-MPU6050Offsets.aZ)/16384);
	
	printf("gX\t%i\t%.2f\n",MPU6050RawData.gX-MPU6050Offsets.gX,(float)(MPU6050RawData.gX-MPU6050Offsets.gX)/131);
	printf("gY\t%i\t%.2f\n",MPU6050RawData.gY-MPU6050Offsets.gY,(float)(MPU6050RawData.gY-MPU6050Offsets.gY)/131);
	printf("gZ\t%i\t%.2f\n",MPU6050RawData.gZ-MPU6050Offsets.gZ,(float)(MPU6050RawData.gZ-MPU6050Offsets.gZ)/131);
	*/
}
	

