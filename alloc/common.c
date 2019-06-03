#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "addr.h"
#include "lib_fpga.h"

#include "lib_bit.h"
#include "alloc.h"
#include "common.h"


/*
  1	成功
  0	失败
*/

int SysReadClockType(int fpga_fd, u16_t *clock_type)
{

	if(!FpgaRead(fpga_fd, FPGA_RBXO_TYP, clock_type))
	{
		print(DBG_ERROR, "--Failed to read clock type.");
		
		return 0;
	}
	return 1;
}

/*
  1	成功
  0	失败
*/

int SysReadClockStatus(int fpga_fd, u16_t *clock_sta)
{

	if(!FpgaRead(fpga_fd, FPGA_RBXO_STA, clock_sta))
	{
		print(DBG_ERROR, "--Failed to read clock status.");
		
		return 0;
	}
	return 1;
}


/*
  1	成功
  0	失败
*/
int SysReadPhase(int fpga_fd, int * sys_phase)
{
	int sign;
	int phase;
	u16_t ph_low, ph_high;

		// system
		if(!FpgaRead(fpga_fd, FPGA_SYS_PH_LOW16, &ph_low))
		{
			print(	DBG_ERROR,
					"--Failed to read phase of SYS.");
			return 0;
		}
		if(!FpgaRead(fpga_fd, FPGA_SYS_PH_HIGH12, &ph_high))
		{
			print(	DBG_ERROR,
					"--Failed to read phase of SYS.");
			return 0;
		}
		phase = (ph_high << 16)| ph_low;

		if(phase &BIT(23))
		{
			sign = -1;
		}
		else
		{
			sign = 1;
		}
		phase &= (~BIT(23));
		phase *= sign;

		*sys_phase = phase;

	return 1;
}

/*
  1	成功
  0	失败
*/

#define CHECK_FPGA_RUN_STATUS_VAL 0xAD52 //0xAD25
int SysCheckFpgaRunStatus(int fpga_fd)
{
	u16_t result_val = 0;
	static u16_t check_val = CHECK_FPGA_RUN_STATUS_VAL;
	int ret = 0;
	// system
	if(!FpgaWrite(fpga_fd, FPGA_CHECK_RUN_STATUS , check_val))
	{
		print(	DBG_ERROR,
				"--Failed to write check run status reg.");
		return 0;
	}
	if(!FpgaRead(fpga_fd, FPGA_CHECK_RUN_STATUS , &result_val))
	{
		print(	DBG_ERROR,
				"--Failed to read check run status reg.");
		return 0;
	}
	#if 0
	print(	DBG_ERROR,
				"--write_check_val:0x%04x ~check_val:0x%04x, read_check_val:0x%04x.",
				check_val,(u16_t)~check_val, result_val);//test
	#endif
	if((u16_t)~check_val == result_val)	//~check_val default 32bit
		ret = 1;
	return ret;
}


void SysWaitFpgaRunStatusOk(int fpga_fd)
{
	while(1){
		if(1 == SysCheckFpgaRunStatus(fpga_fd)){
			#if 0
			print(	DBG_ERROR,
					"<%s>-- fpga status ok!", who);//test
			#endif
			break;
		}
		print(	DBG_ERROR,
					"-- fpga status wait...");//test
		sleep(3);
	}
}


/*
  -1	失败
   0	成功
*/
int initializeCommExitSignal(sighandler_t handler)
{
	//installs new signal handler for specific signal.
	//--------------------------------------------------------------------------------------------------------
	if ( SIG_ERR == signal( EXIT_SIGNAL1, handler ) )
	{
		print(	DBG_ERROR,
				"Can't install signal handler for SIGINT!");

		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, handler ) )
	{
		print(	DBG_ERROR,
				"Can't install signal handler for SIGTERM!");

		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, handler ) )
	{
		print(	DBG_ERROR, 
				"Can't install signal handler for SIGHUP!");

		return(-1);
	}
	//--------------------------------------------------------------------------------------------------------

	return 0;
}


/*
ip1,ip2,ip3...ip16
192.168.1.45,192.168.2.55,...;
  0	不合法
  1 合法
*/
int str_ip_group_to_uint32(char *ipv4_grp, u32_t *des_ipv4, u8_t * des_ipv4_num)
{
	#if 0
	if( INADDR_NONE == inet_addr((const char *)ip) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
	#endif
	int i,j;
	//int found = 0;
	char *ptr = NULL;
	char *saveptr = NULL;
	char t_ipv4_grp[MAX_IP_LEN*MAX_IP_NUM+1];
	u32_t t_des_ipv4[MAX_IP_NUM];
	struct in_addr addr;
    if(ipv4_grp == NULL || strlen(ipv4_grp)<7 || strlen(ipv4_grp) > MAX_IP_LEN*MAX_IP_NUM)
        return 0;
	memset(t_ipv4_grp, 0x0, sizeof(t_ipv4_grp));

	//del " ","\n", "\r"
	for (i = 0, j = 0; i < strlen(ipv4_grp); ++i){
		if(ipv4_grp[i] == ' ' || ipv4_grp[i] == '\n'
			|| ipv4_grp[i] == '\r' || ipv4_grp[i] == '\t')
			continue;
		t_ipv4_grp[j++] = ipv4_grp[i];
	}

	ptr = strtok_r(t_ipv4_grp, ",", &saveptr);
	if(!ptr)
		return 0;

	i = 0;
	memset(t_des_ipv4, 0x0, sizeof(t_des_ipv4));
	do {
		//found = 0;
		//printf("ptr:%s\n", ptr);
		if(inet_pton(AF_INET, ptr, (void *)&addr) != 1){
   			return 0;
		}
		t_des_ipv4[i] = ntohl(addr.s_addr);
		//printf("ptr:%s\tpton:%08x\taddr:%08x\thost:%08x\thost2:%08x\n", ptr, 
		//addr.s_addr,inet_addr(ptr),inet_network(ptr), ntohl(inet_addr(ptr)));
		i++;
	}while((ptr = strtok_r(NULL, ",", &saveptr)) != NULL);
	if(i > MAX_IP_NUM)
		return 0;
	*des_ipv4_num = i;
	memcpy(des_ipv4, t_des_ipv4, (*des_ipv4_num) * sizeof(u32_t));
	//printf("num:%d\n", *des_ipv4_num);
	return 1;

}

int uint32_ip_group_to_str_ip(u32_t *ipv4, u8_t ipv4_num, char *des_ipv4_grp)
{
	int i;
	char str_ip[MAX_IP_LEN];
	char str_ip_group[MAX_IP_LEN*MAX_IP_NUM+1];//注意栈溢出问题，必要时用static

	if(ipv4 == NULL || des_ipv4_grp == NULL || ipv4_num > MAX_IP_NUM)
        return 0;
	memset(str_ip_group, 0x0, sizeof(str_ip_group));
	for (i = 0; i < ipv4_num; ++i){
		//if(sta.ptpin.ptpinInfo.ptpMasterIp[i] == 0)
			//continue;
		memset(str_ip, 0x0, MAX_IP_LEN);
		UINTIP_TO_STRIP(ipv4[i], str_ip);
		sprintf(str_ip_group, "%s%s,", str_ip_group, str_ip);
	}
	if(strlen(str_ip_group)>0)
		str_ip_group[strlen(str_ip_group)-1] = '\0';
	memcpy(des_ipv4_grp, str_ip_group, strlen(str_ip_group));
	return 1;
}


