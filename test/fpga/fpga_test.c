#include <stdio.h>
#include "alloc.h"
#include <pthread.h>
#include <lib_fpga.h>
#include <unistd.h>
int fpga_fd;
int read_cnt;
int write_cnt;
int rw_err;

#define FPGA_TEST_ADDR 	0x0332

void * write1_thread(void * arg)
{
	unsigned short value;

	while(1)
	{
		value = 0x55;
		if (!FpgaWrite(fpga_fd, FPGA_TEST_ADDR, value)){
			printf("fpga write1 err \n");
		}
		write_cnt++;
	}
	
	return 0;
}

void * write2_thread(void * arg)
{
	unsigned short value;

	while(1){
		value = 0xaa;
		if (!FpgaWrite(fpga_fd, FPGA_TEST_ADDR, value)){
			printf("fpga write2 err \n");
		}
		write_cnt++;
	}
	return 0;
}

void * read_thread(void * arg)
{
	unsigned  short value;

	while(1){
				if (!FpgaRead(fpga_fd, FPGA_TEST_ADDR, &value)){
					printf("fpga read err \n");
				}


		read_cnt++;
		if ((value != 0x55)&&(value != 0xaa) ){
			printf("err value is %x \n",value);
			rw_err++;
		}	
	}

	return 0;
}

int main(int argc, char * argv[])
{

	int res;
	pthread_t write1,write2,read;

	fpga_fd = FpgaOpen(FPGA_DEV);

	res = pthread_create(&write1, NULL,write1_thread, NULL);
	if (res != 0){
		printf("create write1_thread err \n");
	}

	res = pthread_create(&write2, NULL,write2_thread, NULL);
	if (res != 0){
		printf("create write1_thread err \n");
	}

	res = pthread_create(&read, NULL,read_thread, NULL);
	if (res != 0){
		printf("create write1_thread err \n");
	}


	while(1)
	{
		sleep(10);
		printf("write %d, read %d,err %d\n",write_cnt,read_cnt,rw_err);
	}
	
}