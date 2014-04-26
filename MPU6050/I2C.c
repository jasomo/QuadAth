#include "I2C.h"

int OpenI2Cdevice(char *devFile, int devAddress)
{
	int devDescriptor=open(devFile, O_RDWR);
	if (devDescriptor==-1)
	{
		perror("I2C: Error opening device file");
		return I2C_ERROR;
	}

	if (ioctl(devDescriptor, I2C_SLAVE, devAddress) < 0)
	{
		perror("I2C: Failed to acquire bus access and/or talk to slave");
		return I2C_ERROR;
	}
	return devDescriptor;
}


int ReadRegister (int device, char command)
{
	struct i2c_smbus_ioctl_data blk;
	union i2c_smbus_data i2cdata;
	int returnValue;
		
	blk.read_write=1;
	blk.command=command;
	blk.size=I2C_SMBUS_BYTE_DATA;
	blk.data=&i2cdata;

	returnValue=ioctl(device,I2C_SMBUS,&blk);
	if (returnValue==I2C_ERROR)
	{
		perror("I2C: Error reading I2C register");
		return I2C_ERROR;
	}
		
		
	return (int)i2cdata.byte;
}

int WriteRegister (int device, char command, int value)
{
	struct i2c_smbus_ioctl_data blk;
	union i2c_smbus_data i2cdata;
	int returnValue;

	i2cdata.byte=value;
	blk.read_write=0;
	blk.command=command;
	blk.size=I2C_SMBUS_BYTE_DATA;
	blk.data=&i2cdata;

	returnValue=ioctl(device,I2C_SMBUS,&blk);
	if (returnValue==I2C_ERROR)
	{
		perror("I2C: Error writing I2C register");
		return I2C_ERROR;
	}
	return I2C_OK;
}

int ReadWord(int device, char command)
{
	struct i2c_smbus_ioctl_data blk;
	union i2c_smbus_data i2cdata;
	int returnValue;

	blk.read_write=1;
	blk.command=command;
	blk.size=I2C_SMBUS_WORD_DATA;
	blk.data=&i2cdata;

	returnValue=ioctl(device,I2C_SMBUS,&blk);
	if (returnValue==I2C_ERROR)
	{
		perror("I2C: Error reading I2C word");
		return I2C_ERROR;
	}
	return (int) i2cdata.word;
}

int ReadRegisters(int device, char firstCommand, int numRegisters, void *block)
{
	struct i2c_smbus_ioctl_data blk;
	union i2c_smbus_data i2cdata;
	int returnValue;

	blk.read_write=1;
	blk.command=firstCommand;
	blk.size= I2C_SMBUS_I2C_BLOCK_DATA;
	blk.data=&i2cdata;
	i2cdata.block[0]=numRegisters;

	returnValue=ioctl(device,I2C_SMBUS,&blk);
	if (returnValue==I2C_ERROR)
	{
		perror("I2C: Error reading I2C registers");
		return I2C_ERROR;
	}
	memcpy(block,&i2cdata.block[1],numRegisters);
	return I2C_OK;
}
