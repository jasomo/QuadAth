#ifndef I2C_INCLUDED
#define I2C_INCLUDED

#define I2C_OK 0
#define I2C_ERROR -1

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int OpenI2Cdevice(char *devFile, int devAddress);
int ReadRegister (int device, char command);
int WriteRegister (int device, char command, int value);
int ReadWord(int device, char command);
int ReadRegisters(int device, char firstCommand, int numRegisters, void *block);


#endif
