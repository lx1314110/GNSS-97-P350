#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "lib_fpga.h"
#include "lib_md5.h"

#include "addr.h"
#include "din_p50.h"
#include "lib_dbg.h"
#include "alloc.h"







/*为ptp in 国外模块*/


/*
  1		成功
  0		失败
*/
int p50_write(int fpga_fd, int slot, u8_t *buf, int len)
{
	u16_t i;
	//printf("---%s\n",buf);
	//print(DBG_INFORMATIONAL, "--S%d--%s\n", slot, buf);
	for(i=0; i<len; i++)
	{
		
		if(!FpgaWrite(fpga_fd, FPGA_IN_PTP_WADDR(slot), i))
		{
			return 0;
		}
		if(!FpgaWrite(fpga_fd, FPGA_IN_PTP_WVALUE(slot), buf[i]))
		{
			return 0;
		}
		if(!FpgaWrite(fpga_fd, FPGA_IN_PTP_WEN(slot), 0x0000))
		{
			return 0;
		}
		if(!FpgaWrite(fpga_fd, FPGA_IN_PTP_WEN(slot), 0x0001))
		{
			return 0;
		}
	}

	if(!FpgaWrite(fpga_fd, FPGA_IN_PTP_WREADY(slot), 0x0001))
	{
		return 0;
	}

	if(!FpgaWrite(fpga_fd, FPGA_IN_PTP_WREADY(slot), 0x0000))
	{
		return 0;
	}

	return 1;
}

#if 0
//以下函数根据锁相环的状态来进行设置

u8_t const num2hex[] = {
		0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x61,0x62,0x63,0x64,0x65,0x66
};

void ptp_set_ssm( int fpga_fd,u8_t ssm_level)
{
	u8_t buffer[20] = "ptp2 s ssm ";

	buffer[11] = ssm_level + 0x30;
	buffer[12] = '\0';
	p50_write(fpga_fd, 11, buffer, strlen((char *)buffer)+1);
	
	usleep(100000);
}


//set the clock property
void ptp_set_clkproperty( int fpga_fd,u8_t clkclass,u8_t accuracy,u16_t range)
{
	u8_t buffer[20] = "ptp2 c q ";

	buffer[9]  = num2hex[clkclass/16];
	buffer[10] = num2hex[clkclass%16];
	buffer[11] = ' ';
	buffer[12] = num2hex[accuracy/16];
	buffer[13] = num2hex[accuracy%16];
	buffer[14] = ' ';
	buffer[15] = num2hex[range/4096];
	buffer[16] = num2hex[(range%4096)/256];
	buffer[17] = num2hex[(range%256)/16];
	buffer[18] = num2hex[range%16];
	buffer[19] = '\0';

	p50_write(fpga_fd, 11, buffer, strlen((char *)buffer)+1);
	usleep(100000);
	//Uart_2_send_str(buffer); //观察发送的串
}


//set freq_trace
void ptp_set_freqtrace( int fpga_fd,u8_t value) //normal
{
    u8_t buffer[15] = "ptp2 p f ";

	buffer[9] = value + 0x30;
	buffer[10] = '\0';

	p50_write(fpga_fd, 11, buffer, strlen((char *)buffer)+1);
	usleep(100000);
}

//set time_trace
void ptp_set_timetrace( int fpga_fd,u8_t value) //normal
{
    u8_t buffer[15] = "ptp2 p t ";

	buffer[9] = value + 0x30;
	buffer[10] = '\0';

	p50_write(fpga_fd, 11, buffer, strlen((char *)buffer)+1);
	usleep(100000);
}

//set time source
void ptp_set_timesource( int fpga_fd,u8_t value)  //normal
{
    u8_t buffer[15] = "ptp2 p s ";

	buffer[9] = num2hex[value/16];
	buffer[10] = num2hex[value%16];
	buffer[11] = '\0';

	p50_write(fpga_fd, 11, buffer, strlen((char *)buffer)+1);
	usleep(100000);
}
#endif



#if 0
void ptp_set_mac(char *buffer,u8_t *macSrc)
{	
	u8_t mac[6]={0},c=0,v=0,m=0,i=0,n=0;
	
	
	for(;i<=17;i++)
	{
		if( n == 2 )
		{
			mac[m++]=c;
			n=0;
			c=0;
			continue;
		}
		v = macSrc[i];
		
		if(v == ':')
			continue;
		if(v >= '0' && v <= '9')
			v-=48;
		if(v >= 'a' && v <= 'f')
			v-=87;
		if(v >= 'A' && v <= 'F')
			v-=55;
		
		if(++n == 1)
			v*=16;
		c+=v;
		
	}
	sprintf(buffer,"%s %02x%02x%02x%02x%02x%02x","ipconfig -e",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}
#endif

void ptp_set_master_mac(char *buffer,u8_t *macSrc)
{	
	u8_t mac[6]={0},c=0,v=0,m=0,i=0,n=0;
	
	
	for(;i<=17;i++)
	{
		if( n == 2 )
		{
			mac[m++]=c;
			n=0;
			c=0;
			continue;
		}
		v = macSrc[i];
		
		if(v == ':')
			continue;
		if(v >= '0' && v <= '9')
			v-=48;
		if(v >= 'a' && v <= 'f')
			v-=87;
		if(v >= 'A' && v <= 'F')
			v-=55;
		
		if(++n == 1)
			v*=16;
		c+=v;
		
	}
	sprintf(buffer,"%02x-%02x-%02x-%02x-%02x-%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}










