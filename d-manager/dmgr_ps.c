#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "alloc.h"

#include "dmgr_global.h"
#include "dmgr_ps.h"












#define PS_READ			0x00
#define PS_WRITE		0x01





#define PIN_nSTATUS		0
#define PIN_DATA		1
#define PIN_nCONFIG		2
#define PIN_DCLK        3
#define PIN_CONF_DONE	4














static inline void Write_PIN_DATA(int fd, int data)
{ 
	int tmp = 0;
	
	tmp = (data << 16) | PIN_DATA;
	ioctl(fd, PS_WRITE, &tmp);
}














static inline int Read_PIN_nSTATUS(int fd)
{
	int tmp = 0;
	
	tmp = PIN_nSTATUS;
	ioctl(fd, PS_READ, &tmp);

	return tmp;
}














static inline void Write_PIN_nCONFIG(int fd, int data)
{ 
	int tmp = 0;
	
	tmp = (data << 16) | PIN_nCONFIG;
	ioctl(fd, PS_WRITE, &tmp);
}














static inline int Read_PIN_CONF_DONE(int fd)
{ 
	int tmp = 0;
	
	tmp = PIN_CONF_DONE;
	ioctl(fd, PS_READ, &tmp);

	return tmp;
}














static inline void Write_PIN_DCLK(int fd, int data)
{
	int tmp = 0;
	
	tmp = (data << 16) | PIN_DCLK;
	ioctl(fd, PS_WRITE, &tmp);
}














int Fpga_PS_Config(int fd, char *path)
{
	char i, byte;
	int count = 0;
	FILE *fp = NULL;

	fp = fopen(path , "r");
	if(NULL == fp)
	{
		return 0;
	}

	//When nCONFIG is pulled low, a reconfiguration cycle begins.
	Write_PIN_nCONFIG(fd, 0);
	Write_PIN_DCLK(fd, 0);
	usleep(5);

	//Before configuration, nSTATUS is low.
	if(1 == Read_PIN_nSTATUS(fd))
	{
		fclose(fp);
		return 0;
	}
	
	Write_PIN_nCONFIG(fd, 1);
	usleep(5);

	do
	{
		byte = fgetc(fp);
		for(i=0; i<8; i++)
		{
			Write_PIN_DATA(fd, byte&0x01);

			Write_PIN_DCLK(fd, 1);
			byte >>= 1;
			Write_PIN_DCLK(fd, 0);
		}

		//During configuration, nSTATUS is high.
		if(0 == Read_PIN_nSTATUS(fd))
		{
			fclose(fp);
			return 0;
		}

		count++;
	}while(count <= 0x122F6C);

	usleep(5);
	//After configuration, CONF_DONE is high.
	if(0 == Read_PIN_CONF_DONE(fd))
	{
		fclose(fp);
		return 0;
	}

	//Two cycles
	for(i=0; i<2; i++)
	{
		Write_PIN_DCLK(fd, 1);
		usleep(100);
		Write_PIN_DCLK(fd, 0);
		usleep(100);
	}

	//Do not leave the DATA pin floating after configuration.
	//Drive the DATA pin high or low.
	Write_PIN_DATA(fd, 0);
	
	fclose(fp);
	return 1;
}












