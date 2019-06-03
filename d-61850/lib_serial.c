

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include"pthread.h"

#include "lib_serial.h"
#include "lib_dbg.h"
#include "alloc.h"


struct termios options;
struct termios oldoptions;

struct serial_config serialread;

static int serial_fd;

int speed_arr[] = {B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,
		   B38400, B19200, B9600, B4800, B2400, B1200, B300};

int name_arr[] = {230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300,
		  38400, 19200, 9600, 4800, 2400, 1200, 300};

//-----------------------------------------------
//打印配置文件tq2440_serial.cfg的内容
//-----------------------------------------------
void print_serialread()
{
	print(DBG_INFORMATIONAL, "serialread.dev is %s\n",serialread.serial_dev);
	print(DBG_INFORMATIONAL, "serialread.speed is %d\n",serialread.serial_speed);
	print(DBG_INFORMATIONAL, "serialread.databits is %d\n",serialread.databits);
	print(DBG_INFORMATIONAL, "serialread.stopbits is %d\n",serialread.stopbits);
	print(DBG_INFORMATIONAL, "serialread.parity is %c\n",serialread.parity);
}

//-----------------------------------------------
//读取tq2440_serial.cfg文件并进行配置
//-----------------------------------------------
void readserialcfg()
{
	FILE *serial_fp;
	char j[10];
	
	print(DBG_INFORMATIONAL, "readserailcfg\n");

	serial_fp = fopen("/etc/tq2440_serial.cfg","r");
	if(NULL == serial_fp)
	{
		print(DBG_ERROR, "can't open /etc/tq2440_serial.cfg");
	}
	else
	{
		fscanf(serial_fp, "DEV=%s\n", serialread.serial_dev);

		fscanf(serial_fp, "SPEED=%s\n", j);
		serialread.serial_speed = atoi(j);

		fscanf(serial_fp, "DATABITS=%s\n", j);
		serialread.databits = atoi(j);

		fscanf(serial_fp, "STOPBITS=%s\n", j);
		serialread.stopbits = atoi(j);

		fscanf(serial_fp, "PARITY=%s\n", j);
		serialread.parity = j[0];
	}

	fclose(serial_fp);
}

//-----------------------------------------------
//设置波特率
//-----------------------------------------------
void set_speed(int fd)
{
	int i;
	int status;

	tcgetattr(fd, &oldoptions);
//	printf("serialread.speed is %d\n",serialread.serial_speed);
	for( i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
	{
		if(serialread.serial_speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &options);
			if(status != 0)
			{
				perror("tcsetattr fd1");
				return;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
}


//-----------------------------------------------
//设置其他参数
//-----------------------------------------------
int set_Parity(int fd)
{
	if(tcgetattr(fd, &oldoptions) != 0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}

	options.c_cflag |= (CLOCAL|CREAD);
	options.c_cflag &=~CSIZE;
//	printf("serialread.databits is %d\n",serialread.databits);
	switch(serialread.databits)
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			options.c_cflag |= CS8;
			fprintf(stderr, "Unsupported data size\n");
			return(FALSE);
	}
//	printf("serialread.parity is %c\n",serialread.parity);
	switch(serialread.parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		default:
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			fprintf(stderr, "Unsupported parity\n");
			return(FALSE);
	}
//	printf("serialread.stopbits is %d\n",serialread.stopbits);
	switch(serialread.stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			options.c_cflag &= ~CSTOPB;
			fprintf(stderr, "Unsupported stop bits\n");
			return(FALSE);
	}
	if(serialread.parity != 'n')
		options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 0;	//150;			//15 seconds
	options.c_cc[VMIN] = 0;
#if 1
	options.c_iflag |= IGNPAR|ICRNL;
	options.c_oflag |= OPOST; 
	options.c_iflag &= ~(IXON|IXOFF|IXANY);                     
#endif
	tcflush(fd, TCIFLUSH);
	if(tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("SetupSerial 3");
		return(FALSE);
	}
	return(TRUE);
}

//-----------------------------------------------
//打开串口设备
//-----------------------------------------------
int OpenDev(char *Dev)
{
	//int fd = open(Dev, O_RDWR | O_NOCTTY | O_NDELAY);
	int fd = open(Dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(-1 == fd)
	{
		perror("Can't Open Serial Port");
		return -1;
	}
	else
		return fd;
}

//--------------------------------------------------
//串口初始化
//--------------------------------------------------
int serial_init(void)
{
	char *Dev;


	readserialcfg();
	print_serialread();

	Dev = (char *) serialread.serial_dev;
	//打开串口设备
	serial_fd = OpenDev(Dev);

	if(serial_fd > 0)
		set_speed(serial_fd);		//设置波特率
	else
	{
		print(DBG_ERROR, "Can't Open Serial Port!\n");
		exit(-1);
	}
	//恢复串口未阻塞状态
	if (fcntl(serial_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		print(DBG_ERROR, "fcntl failed!\n");
		exit(-1);
	}

	//检查是否是终端设备
#if 1	//如果屏蔽下面这段代码，在串口输入时不会有回显的情况，调用下面这段代码时会出现回显现象。
	if(isatty(STDIN_FILENO)==0)
	{
		print(DBG_INFORMATIONAL, "standard input is not a terminal device\n");
	}
	else
		print(DBG_INFORMATIONAL, "isatty success!\n");
#endif
	//设置串口参数
	if(set_Parity(serial_fd) == FALSE)
	{
		print(DBG_ERROR, "Set parity Error\n");
		exit(-1);
	}
	return serial_fd;
}

void serial_rw()
{
//	int i;
	unsigned char buff[512];
	unsigned char buff2[] = "hello,TQ2440!\n";
	int nread,nwrite;

	nwrite = write(serial_fd,buff2,sizeof(buff2));
	printf("nwrite=%d\n",nwrite);
	while(1)
	{
		if((nread = read(serial_fd,buff,512))>0)
		{ 
			buff[nread] = '\0';
#if 0	//调用这段代码可以实现回显，如果配合上面的回显，就会出现回显两次的情况。
			write(serial_fd,buff,nread);
#endif
			printf("\nrecv:%d\n",nread);
#if 0
			for(i=0;i<nread;i++)
			{
				printf("%c",buff[i]);
			}
			printf("\n");
#else
			printf("%s",buff);
			printf("\n");
#endif
		}
	}

	close(serial_fd);

}


#if 0
int main(int argc, char **argv)
{
	serial_init();

	serial_rw();	
	return 0;
//	close(serialread.serial_dev);
//	serial_test(telnum, telmoney);
}
#endif

