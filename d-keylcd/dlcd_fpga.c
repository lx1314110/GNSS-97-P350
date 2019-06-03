
#include <stdio.h>

#include "dlcd_global.h"
#include "dlcd_fpga.h"


#define F_PATH "/root/output_file.rbf"

#define GPIO_WRITE	0x01
#define GPIO_READ	0x00


#define PS_STATUS      0
#define PS_DATA        1
#define PS_CONFIG      2
#define PS_DCLK        3
#define PS_CONFIG_DONE 4

#if 0


/**************************************************
author:   Tim
date:      2013.08.15
function: cpu download config to fpga
retrun:  1 ok  ; 0 error
modify:  
***************************************************/
int Fpga_DownLoad(void)
{ 
	int CountNum = 0;
	char FpgaBuffer, i;

	FILE *fp = NULL;
	fp = fopen(F_PATH , "rb");
	if(fp == NULL)
	{
		printf("fopen error\n");
		return 0;
	}

	Set_nCONFIG(0);               
	Set_DCLK(0); 
	usleep(5);             
	if (Read_nSTATUS() == 1)
	{
		printf("status error\n");
		return 0;
	}
	Set_nCONFIG(1);
	usleep(5);

	do
	{
		FpgaBuffer = fgetc(fp);
		for (i=0; i<8; i++)
		{ 
			Set_Data0(FpgaBuffer&0x01);       // DCLK="0", put data（LSB first）
			Set_DCLK(1);           
			FpgaBuffer >>= 1;              
			Set_DCLK(0);
		}

		if (Read_nSTATUS() == 0)
		{
			printf("status error during config	 %x\n",CountNum);
			return 0;
		}

		CountNum++;
	}while(CountNum <= 0x122F6C);

	printf("send ok\n");

	usleep(5);
	if (Read_nCONF_Done() == 0)
	{ 
		
		printf("config done error\n");
		return 0;
	}




	for(i=0; i<2; i++) // FPGA初始化：need two clk falling
	{
		Set_DCLK(1);
		usleep(100);
		Set_DCLK(0);
		usleep(100);
	}
	
	Set_Data0(0);
	
	fclose(fp);
	return 1;
}


/**************************************************
author:   Tim
date:      2013.08.15
function: Data0输出
retrun:  NULL
modify:  
***************************************************/
void Set_Data0(char setting)
{ 
	int tmp = 0;
	tmp = (setting << 16) | PS_DATA;
	ioctl(gLcdCtx.ps_fd, GPIO_WRITE, &tmp);

}

/**************************************************
author:   Tim
date:      2013.08.15
function: 读nSTATUS状态
retrun:  NULL
modify:  
***************************************************/
int Read_nSTATUS(void)
{
	int tmp = 0;
	tmp = PS_STATUS;
	ioctl(gLcdCtx.ps_fd, GPIO_READ, &tmp);

	return tmp;
}

/**************************************************
author:   Tim
date:      2013.08.15
function: 设置nCONFIG电平
retrun:  NULL
modify:  
***************************************************/
void Set_nCONFIG(char setting)
{ 
	int tmp = 0;
	tmp = (setting << 16) | PS_CONFIG;
	ioctl(gLcdCtx.ps_fd, GPIO_WRITE, &tmp);
}

/**************************************************
author:   Tim
date:      2013.08.15
function: 读nCONF_Done状态
retrun:  NULL
modify:  
***************************************************/
int Read_nCONF_Done(void)
{ 
	int tmp = 0;
	tmp = PS_CONFIG_DONE;
	ioctl(gLcdCtx.ps_fd, GPIO_READ, &tmp);

	return tmp;

}

/**************************************************
author:   Tim
date:      2013.08.15
function: 输出DCLK
retrun:  NULL
modify:  
***************************************************/
void Set_DCLK(char setting)
{
	int tmp = 0;
	tmp = (setting << 16) | PS_DCLK;
	ioctl(gLcdCtx.ps_fd, GPIO_WRITE, &tmp);
}



#endif




