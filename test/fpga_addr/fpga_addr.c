#include <stdio.h>
#include "alloc.h"
#include <pthread.h>
#include <lib_fpga.h>
#include <unistd.h>
int fpga_fd;
int err;
int loop;
static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{
		loop = 0;
		printf("fpga test error is %d \n",err);
	}
}

static int init_exit_signal(void)
{
	
	//installs new signal handler for specific signal.
	//--------------------------------------------------------------------------------------------------------
	if ( SIG_ERR == signal( EXIT_SIGNAL1, exitHandler ) )
	{
		printf(" EXIT_SIGNAL1 err \n");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )
	{
		printf(" EXIT_SIGNAL2 err \n");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )
	{
		printf(" EXIT_SIGNAL3 err \n");
		return(-1);
	}
	
	return 0;
}
unsigned short addr_group[]={0x00fe,0x01fe,0x02fe,0x03fe,0x04fe,0x10fe,0x11fe,0x12fe,0x13fe};

int main(int argc, char * argv[])
{

	int i;
	unsigned short value;

	loop = 1;
	err = 0;
	fpga_fd = FpgaOpen(FPGA_DEV);


	if (-1 == init_exit_signal())
	{
		printf("init signal error \n");
		return 0;
	}
	while(loop)
	{
		for (i = 0; i < sizeof(addr_group)/sizeof(unsigned short);i++){

			FpgaWrite(fpga_fd, addr_group[i], 0xaaaa);
			FpgaRead(fpga_fd, addr_group[i], &value);
			if (value != 0x5555)
			{
				err++;
			//	printf("err is %d \n",err);
			}
		}
	}
	return 0;
}