#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "lib_fpga.h"
#include "lib_net.h"
#include "alloc.h"

#include "cout_query.h"
#include "cout_set.h"
#include "cout_global.h"
#include "cout_alloc.h"
#include "cout_macro.h"
#include "cout_parser.h"




int argv_parser(int argc, char *argv[], struct outCtx *ctx)
{
	int i;
	int ret = 0;
	struct outsetinfo ds;
	struct outdelay sod;
    
	if(-1 == initializeMessageQueue(&ctx->ipc))
	{
		printf("Failure.\n");
		return -1;
	}

	if(-1 == initializeSemaphoreSet(&ctx->ipc))
	{
		printf("Failure.\n");
		return -1;
	}

	if(-1 == initializeShareMemory(&ctx->ipc))
	{
		printf("Failure.\n");
		return -1;
	}

	if(-1 == readPid(&ctx->ipc, &ctx->pid))
	{
		printf("Failure.\n");
		return -1;
	}
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		printf("Failure.\n");
		return -1;
	}
	if(-1 == readBid(ctx))
	{
		FpgaClose(ctx->fpga_fd);
		printf("Failure.\n");
		return -1;
	}
	
	//查询输出延时补偿、输出信号类型、输出时区
	if(1 == argc)
	{
		if(-1 == port16_query(ctx))
		{
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}
	else if(2 == argc)
	{
		//查询输入时区
		if(0 == memcmp(argv[1], IZONE, strlen(IZONE)))
		{
			if(-1 == in_zone_query(ctx, 0))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		//查询输出时区
		else if(0 == memcmp(argv[1], OZONE, strlen(OZONE)))
		{
			if(-1 == out_zone_query(ctx, 0))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], OBR, strlen(OBR)))
		{
			if(-1 == out_baudrate_query(ctx, 0))//波特率
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], OBSA, strlen(OBSA)))
		{
			if(-1 == out_2mb_sa_query(ctx))//2mb sa
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], OBSSM, strlen(OBSSM)))
		{
			if(-1 == out_2mb_ssm_query(ctx))//2mb SSM, ONLY QUERY
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], IRIGB_AR, strlen(IRIGB_AR)))
		{
			if(-1 == irigb_amplitude_ratio_query(ctx, 0))//查询所有振幅
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], IRIGB_V, strlen(IRIGB_V)))
		{
			if(-1 == irigb_voltage_query(ctx, 0))//查询所有电压
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(1 == slot_validity(argv[1]))
		{
			if(-1 == port1_query(ctx, atoi(argv[1]), 0))//输出端口信息查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], DELAY, strlen(DELAY)))
		{
			if(-1 == type3_delay_query(ctx))//所有输出信号对应的时延
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], HELP, strlen(HELP)))
		{
			helper();
		}
		else
		{
			ret = -1;
		}
	}
	else if(3 == argc)
	{
		if( (0 == memcmp(argv[2], PORTINFO, strlen(PORTINFO))) &&
			(slot_validity(argv[1])) )
		{
			if(0 == portinfo_query(ctx, atoi(argv[1])))//输出端口信息查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}else if( (0 == memcmp(argv[2], SPT_SIGNAL_TBL, strlen(SPT_SIGNAL_TBL))) &&
			(slot_validity(argv[1])) )
		{
			if(0 == spt_signal_table_query(ctx, atoi(argv[1])))//支持的输出信号列表查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}else if( (0 == memcmp(argv[2], SIGNAL, strlen(SIGNAL))) &&
			(slot_validity(argv[1])) )
		{
			if(0 == signal_query(ctx, atoi(argv[1])))//输出信号up/down/unknown查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}else if(0 == memcmp(argv[1], DELAY, strlen(DELAY)))
		{
			if(-1 == type1_delay_query(ctx, argv[2]))//指定输出信号对应的时延查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if((0 == memcmp(argv[1], OZONE, strlen(OZONE))) &&
				(zone_group_validity(argv[2])))
		{
			if(-1 == out_zone_set(ctx, argv[2], 0))//设置所有输出时区
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], IZONE, strlen(IZONE))) &&
				(zone_group_validity(argv[2])))
		{
			if(-1 == in_zone_set(ctx, argv[2], 0))//设置所有输入时区
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], OBR, strlen(OBR))) &&
				(baudrate_group_validity(argv[2])))
		{
			if(-1 == out_baudrate_set(ctx, argv[2], 0))//设置所有输出波特率
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], OBSA, strlen(OBSA))) &&
				(sa_validity(argv[2])))
		{
			if(-1 == out_2mb_sa_set(ctx, (u8_t *)argv[2]))//设置2mb_sa 为2mb输出
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], IRIGB_AR, strlen(IRIGB_AR))) &&
				(amplitude_ratio_group_validity(argv[2])))
		{
			if(-1 == irigb_amplitude_ratio_set(ctx, argv[2], 0))//设置所有振幅
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], IRIGB_V, strlen(IRIGB_V))) &&
				(voltage_group_validity(argv[2])))
		{
			if(-1 == irigb_voltage_set(ctx, argv[2], 0))//设置所有电压
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		//查询输出时区
		else if(0 == memcmp(argv[1], OZONE, strlen(OZONE))&&
				(group4_validity(argv[2])))
		{
			if(-1 == out_zone_query(ctx, atoi(argv[2])))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		//查询输入时区
		else if(0 == memcmp(argv[1], IZONE, strlen(IZONE))&&
				(group4_validity(argv[2])))
		{
			if(-1 == in_zone_query(ctx, atoi(argv[2])))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		//查询输出波特率
		else if(0 == memcmp(argv[1], OBR, strlen(OBR))&&
				(group4_validity(argv[2])))
		{
			if(-1 == out_baudrate_query(ctx, atoi(argv[2])))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		//查询振幅
		else if(0 == memcmp(argv[1], IRIGB_AR, strlen(IRIGB_AR))&&
				(group4_validity(argv[2])))
		{
			if(-1 == irigb_amplitude_ratio_query(ctx, atoi(argv[2])))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		//查询电压
		else if(0 == memcmp(argv[1], IRIGB_V, strlen(IRIGB_V))&&
				(group4_validity(argv[2])))
		{
			if(-1 == irigb_voltage_query(ctx, atoi(argv[2])))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else
		{
			ret = -1;
		}
	}
	else if(4 == argc)
	{
		if((0 == memcmp(argv[1], OZONE, strlen(OZONE))) &&
			(group4_validity(argv[2])) &&
			(zone_validity(argv[3])))
		{
			if(-1 == out_zone_set(ctx, argv[3], atoi(argv[2])))//设置单个输出时区
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], IZONE, strlen(IZONE))) &&
			(group4_validity(argv[2])) &&
			(zone_validity(argv[3])))
		{
			if(-1 == in_zone_set(ctx, argv[3], atoi(argv[2])))//设置单个输入时区
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], OBR, strlen(OBR))) &&
			(group4_validity(argv[2])) &&
			(baudrate_validity(argv[3])))
		{
			if(-1 == out_baudrate_set(ctx, argv[3], atoi(argv[2])))//设置单个组输出波特率
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], IRIGB_AR, strlen(IRIGB_AR))) &&
			(group4_validity(argv[2])) &&
			(amplitude_ratio_validity(argv[3])))
		{
			if(-1 == irigb_amplitude_ratio_set(ctx, argv[3], atoi(argv[2])))//振幅设置
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], IRIGB_V, strlen(IRIGB_V))) &&
			(group4_validity(argv[2])) &&
			(voltage_validity(argv[3])))
		{
			if(-1 == irigb_voltage_set(ctx, argv[3], atoi(argv[2])))//电压设置
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((0 == memcmp(argv[1], DELAY, strlen(DELAY))) &&
			(delay_validity(argv[3])))
		{//输出信号的时延设置
			memset(&sod, 0, sizeof(struct outdelay));
			FIND_SIGN_BY_SIGNME_OUTDELAY(argv[2], strlen(argv[2]), sod.signal);
			
			if (sod.signal <= 0 || sod.signal > OUTDELAY_SIGNALTYPE_LEN){
				ret = -1;
			}else{
				sod.delay = atoi(argv[3]);
				if(-1 == type_delay_set(ctx, &sod))//时延设置
				{
					ret = -1;
				}else{
					ret = 0;
					printf("Success.\n");
				}
			}
		}
		else if((slot_validity(argv[1]) && 0 == memcmp(argv[2], DELAY, strlen(DELAY))) &&
						(delay_validity(argv[3])))
		{//槽位的时延设置，以单盘为单位，已经废弃，现在以信号为单位
			memset(&ds, 0, sizeof(struct outsetinfo));
			ds.slot = atoi(argv[1]);
			ds.set_delay_flag = 0x01;//fpga延时是设置的信号延时，非槽位号，目前fpga暂未使用
			ds.delay = atoi(argv[3]);
			if(-1 == output_board_set(ctx, &ds))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if((slot_validity(argv[1])  && 0 == memcmp(argv[2], SIGNAL, strlen(SIGNAL)))&&
				(signal_validity(argv[3])))
		{
			memset(&ds, 0, sizeof(struct outsetinfo));
			ds.slot = atoi(argv[1]);
			ds.set_signal_type_flag = 0x01;
			memcpy(ds.signal, argv[3], strlen(argv[3]));//输出信号类型设置
			if(-1 == output_board_set(ctx, &ds))
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else
		{
			ret = -1;
		}

	}
	else //argc >=5
	{
		
		if(argc > 4 && 0 == memcmp(argv[1], PTP, strlen(PTP)) &&
			1 == slot_validity(argv[2]))
		{
			
			if(0 == (argc-3) %2)
			{
				memset(&ds, 0, sizeof(struct outsetinfo));
				ds.slot = atoi(argv[2]);
				for(i=3; i<argc; i+=2)
				{
					if((0 == memcmp(argv[i], PTP_TYPE, strlen(PTP_TYPE)))&&
							(ptp_type_validity(argv[i+1])))
					{
						ds.set_ptptype_flag = 0x01;
						ds.ptptype = atoi(argv[i+1]);
					}
					else if((0 == memcmp(argv[i], PTP_DELAY_TYPE, strlen(PTP_DELAY_TYPE)))&&
							(ptp_delay_type_validity(argv[i+1])))
					{
						ds.set_ptp_delay_type_flag = 0x01;
						memcpy(ds.ptpDelayType, argv[i+1], strlen(argv[i+1]));
					}
					/*add  model set */
					else if((0 == memcmp(argv[i], PTP_MULTI_UNI_TYPE, strlen(PTP_MULTI_UNI_TYPE)))&&
							(ptp_multi_uni_type_validity(argv[i+1])))
					{
						ds.set_ptp_multi_uni_type_flag = 0x01;
						memcpy(ds.ptpMulticast, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_LAYER_TYPE, strlen(PTP_LAYER_TYPE)))&&
							(ptp_layer_type_validity(argv[i+1])))
					{
						ds.set_ptp_layer_type_flag = 0x01;
						memcpy(ds.ptpLayer, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_STEP_TYPE, strlen(PTP_STEP_TYPE)))&&
							(ptp_step_type_validity(argv[i+1])))
					{
						ds.set_ptp_step_type_flag = 0x01;
						memcpy(ds.ptpStep, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_SYNC_FREQ, strlen(PTP_SYNC_FREQ)))&&
							(ptp_freq_validity(argv[i+1])))
					{
						ds.set_ptp_sync_freq_flag = 0x01;
						memcpy(ds.ptpSync, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_ANNOUNCE_FREQ, strlen(PTP_ANNOUNCE_FREQ)))&&
							(ptp_freq_validity(argv[i+1])))
					{
						ds.set_ptp_announce_freq_flag = 0x01;
						memcpy(ds.ptpAnnounce, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_P_DELAY_REQ_FREQ, strlen(PTP_P_DELAY_REQ_FREQ)))&&
							(ptp_freq_validity(argv[i+1])))
					{
						ds.set_ptp_pdelay_req_freq_flag = 0x01;
						memcpy(ds.ptpPdelayReq, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_SSM_ENABLE, strlen(PTP_SSM_ENABLE)))&&
							(ptp_ssmEnable_validity(argv[i+1])))
					{
						ds.set_ptp_ssm_flag = 0x01;
						memcpy(ds.ptpssm, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_IP, strlen(PTP_IP)))&&
							(ip_validity(argv[i+1])))
					{
						ds.set_ptp_ip_flag = 0x01;
						memcpy(ds.ptpIp, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_MASK, strlen(PTP_MASK)))&&
							(mask_validity(argv[i+1])))
					{
						ds.set_ptp_mask_flag = 0x01;
						memcpy(ds.ptpMask, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_GW, strlen(PTP_GW)))&&
							(gateway_validity(argv[i+1])))
					{
						ds.set_ptp_gw_flag = 0x01;
						memcpy(ds.ptpGateway, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_MAC, strlen(PTP_MAC)))&&
							(mac_validity(argv[i+1])))
					{
						ds.set_ptp_mac_flag = 0x01;
						memcpy(ds.ptpMac, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_SLAVE_IP, strlen(PTP_SLAVE_IP)))&&
							(ip_group_validity(argv[i+1])))
					{
						ds.set_ptp_slave_ip_flag = 0x01;
						memcpy(ds.ptp_Slave_Ip, argv[i+1], strlen(argv[i+1]));
					}
					else if ((0 == memcmp(argv[i], PTP_LEVEL, strlen(PTP_LEVEL)))&&
							(ptp_priority_validity(argv[i+1])))
					{
						ds.set_ptp_level_flag = 0x01;
						memcpy(ds.ptp_level, argv[i+1], strlen(argv[i+1]));
					}
					else if ((0 == memcmp(argv[i], PTP_PRIORITY, strlen(PTP_PRIORITY)))&&
							(ptp_priority_validity(argv[i+1])))
					{
						ds.set_ptp_priority_flag = 0x01;
						memcpy(ds.ptp_priority, argv[i+1], strlen(argv[i+1]));
					}					
					else if ((0 == memcmp(argv[i], PTP_REGION, strlen(PTP_REGION)))&&
							(ptp_region_validity(argv[i+1])))
					{
						ds.set_ptp_region_flag = 0x01;
						memcpy(ds.ptp_region, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						ret = -1;
						goto exit;
					}
				}
				
				if(-1 == output_board_set(ctx, &ds))//输出板卡设置
				{
					ret = -1;
				}
				else
				{
					ret = 0;
					printf("Success.\n");
				}
			}
			else
			{
				ret = -1;
			}
		}
		else
		{
			ret = -1;
		}
	}

exit:
	//clean
	FpgaClose(ctx->fpga_fd);
	if(-1 == ret)
	{
		printf("Failure.\n");
		return -1;
	}
	else
	{
		return 0;
	}
}


/*
ip1,ip2,ip3...ip16
192.168.1.45,192.168.2.55,...;
  0	不合法
  1 合法
*/
int ip_group_validity(char *ipv4_grp)
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
	do {
		//found = 0;
		//printf("ptr:%s\n", ptr);
		if(inet_pton(AF_INET, ptr, (void *)&addr) != 1){
   			return 0;
		}
		//printf("ptr:%s\tpton:%08x\taddr:%08x\thost:%08x\thost2:%08x\n", ptr, 
		//addr.s_addr,inet_addr(ptr),inet_network(ptr), ntohl(inet_addr(ptr)));

		i++;
	}while((ptr = strtok_r(NULL, ",", &saveptr)) != NULL);
	if(i > MAX_IP_NUM)
		return 0;
	//printf("num:%d\n", i);
	return 1;

}


/*
  1	合法
  0	非法
*/
int slot_validity(char *slot)
{
	int i;
	if(slot)
	{
		for (i = 0; i < strlen(slot); ++i){
			if(isdigit(slot[i]) == 0)
				return 0;//no digit, return failure
		}
		if(atoi(slot) >= SLOT_CURSOR_1+1 && atoi(slot) <= SLOT_CURSOR_ARRAY_SIZE)
		{
			return 1;
		}
	}
	
	return 0;
}

/*
  1	合法
  0	非法
*/
int sa_validity(char *sa)
{
	int i;
	
	for(i=0; i<SA_CURSOR_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gSaTbl[i], sa, strlen(sa)))
		{
			return 1;
		}
	}

	return 0;
}


/*
  1	合法
  0	非法
*/
int zone_validity(char *zone)
{
	int i;
	int len = 0;

	if(!zone)
		return 0;
	for(i=0; i<ARRAY_SIZE(gZoneTbl); i++)
	{
		len = (strlen(zone) > strlen((const char *)gZoneTbl[i])) ? strlen(zone) : strlen((const char *)gZoneTbl[i]);
		if(0 == memcmp(zone, gZoneTbl[i], len))
		{
			return 1;
		}
	}

	return 0;
}


/*
  1	合法
  0	非法
*/

int zone_group_validity(char *zone_grp)
{
	int found = 0;
	char * tmp = NULL;
	char buf[ZONE_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = ":";

	if(strlen(zone_grp)>=ZONE_BUF_LEN || strlen(zone_grp)<= 0)
		return 0;
	memset(buf,0x0, sizeof(buf));
	memcpy(buf, zone_grp, strlen(zone_grp));

	tmp = strtok_r(buf, separator, &saveptr);
	if(!tmp)
		return 0;
	while (NULL != tmp){
		if(zone_validity(tmp) != 1){
			return 0;
		}else{
			found++;
		}		
		tmp = strtok_r(NULL, separator, &saveptr);
	}

	if(found == 4)
		return 1;
	else
		return 0;
}

/*
  1	合法
  0	非法
*/
int group4_validity(char *group)
{
	if(group)
	{
		if(atoi(group) >= 1 && atoi(group) <= 4)
		{
			return 1;
		}
	}
	
	return 0;

}

/*
  1	合法
  0	非法
*/
int baudrate_validity(char *br)
{
	int i;
	int len = 0;

	if(!br)
		return 0;
	for(i=0; i<ARRAY_SIZE(gBaudRateTbl); i++)
	{
		len = (strlen(br) > strlen((const char *)gBaudRateTbl[i])) ? strlen(br) : strlen((const char *)gBaudRateTbl[i]);
		if(0 == memcmp(br, gBaudRateTbl[i], len))
		{
			return 1;
		}
	}

	return 0;
}


/*
  1	合法
  0	非法
*/

int baudrate_group_validity(char *br_grp)
{
	int found = 0;
	char * tmp = NULL;
	char buf[BAUDRATE_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = ":";

	if(strlen(br_grp)>=BAUDRATE_BUF_LEN || strlen(br_grp)<= 0)
		return 0;
	memset(buf,0x0, sizeof(buf));
	memcpy(buf, br_grp, strlen(br_grp));

	tmp = strtok_r(buf, separator, &saveptr);
	if(!tmp)
		return 0;
	while (NULL != tmp){
		if(baudrate_validity(tmp) != 1){
			return 0;
		}else{
			found++;
		}		
		tmp = strtok_r(NULL, separator, &saveptr);
	}

	if(found == 4)
		return 1;
	else
		return 0;
}

/*
  1	合法
  0	非法
*/
int amplitude_ratio_validity(char *ar)
{
	int i;
	int len = 0;

	if(!ar)
		return 0;
	for(i=0; i<ARRAY_SIZE(gAmplitudeRatioTbl); i++)
	{
		len = (strlen(ar) > strlen((const char *)gAmplitudeRatioTbl[i])) ? strlen(ar) : strlen((const char *)gAmplitudeRatioTbl[i]);
		if(0 == memcmp(ar, gAmplitudeRatioTbl[i], len))
		{
			return 1;
		}
	}

	return 0;
}

/*
  1	合法
  0	非法
*/
int amplitude_ratio_group_validity(char *ar_grp)
{
	int found = 0;
	char * tmp = NULL;
	char buf[IAR_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = "|";

	if(strlen(ar_grp)>=IAR_BUF_LEN || strlen(ar_grp)<= 0)
		return 0;
	memset(buf,0x0, sizeof(buf));
	memcpy(buf, ar_grp, strlen(ar_grp));

	tmp = strtok_r(buf, separator, &saveptr);
	if(!tmp)
		return 0;
	while (NULL != tmp){
		if(amplitude_ratio_validity(tmp) != 1){
			return 0;
		}else{
			found++;
		}		
		tmp = strtok_r(NULL, separator, &saveptr);
	}

	if(found == 4)
		return 1;
	else
		return 0;
}

/*
  1	合法
  0	非法
*/
int voltage_validity(char *v)
{
	int i;
	int len = 0;

	if(!v)
		return 0;
	for(i=0; i<ARRAY_SIZE(gVoltageTbl); i++)
	{
		len = (strlen(v) > strlen((const char *)gVoltageTbl[i])) ? strlen(v) : strlen((const char *)gVoltageTbl[i]);
		if(0 == memcmp(v, gVoltageTbl[i], len))
		{
			return 1;
		}
	}

	return 0;
}

/*
  1	合法
  0	非法
*/
int voltage_group_validity(char *v_grp)
{
	int found = 0;
	char * tmp = NULL;
	char buf[IV_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = "|";

	if(strlen(v_grp)>=IV_BUF_LEN || strlen(v_grp)<= 0)
		return 0;
	memset(buf,0x0, sizeof(buf));
	memcpy(buf, v_grp, strlen(v_grp));

	tmp = strtok_r(buf, separator, &saveptr);
	if(!tmp)
		return 0;
	while (NULL != tmp){
		if(voltage_validity(tmp) != 1){
			return 0;
		}else{
			found++;
		}		
		tmp = strtok_r(NULL, separator, &saveptr);
	}

	if(found == 4)
		return 1;
	else
		return 0;
}


/*
  1	合法
  0	非法
*/
int delay_validity(char *delay)
{
	int i;
	int len;

	len = strlen(delay);
	if(1 == len)
	{
		if(!((delay[0] >= '0') && (delay[0] <= '9')))
		{
			return 0;
		}
	}
	else
	{
		if(!(('-' == delay[0]) || ((delay[0] >= '1') && (delay[0] <= '9'))))
		{
			return 0;
		}
		if('-' == delay[0])
		{
			if(!((delay[1] >= '1') && (delay[1] <= '9')))
			{
				return 0;
			}
		}
		
		for(i=1; i<len; i++)
		{
			if(!((delay[i] >= '0') && (delay[i] <= '9')))
			{
				return 0;
			}
		}
	}

	return 1;
}





/*
  1	合法
  0	非法
*/
int signal_validity(char *signal)
{
	int i;

	for(i=0; i<SIGNAL_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gSignalTbl[i], signal, strlen((const char *)gSignalTbl[i])))
		{
			return 1;
		}
	}

	return 0;
}







/*
  1	合法
  0	非法
*/
int ptp_type_validity(char *type)
{
	int i;
	if(type)
	{
		for (i = 0; i < strlen(type); ++i){
			if(isdigit(type[i]) == 0)
				return 0;//no digit, return failure
		}
		if(atoi(type) == PTP_TYPE_CN || atoi(type) == PTP_TYPE_FRGN)
		{
			return 1;
		}
	}
	
	return 0;
}



/*
  1	合法
  0	非法
*/
int ptp_delay_type_validity(char *pdt)
{
	int i;

	for(i=0; i<PTP_DELAY_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gPdtTbl[i], pdt, strlen((const char *)gPdtTbl[i])))
		{
			return 1;
		}
	}

	return 0;
}






/*
  1	合法
  0	非法
*/
int ptp_multi_uni_type_validity(char *pmut)
{
	int i;

	for(i=0; i<PTP_MULTI_UNI_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gPmutTbl[i], pmut, strlen((const char *)gPmutTbl[i])))
		{
			return 1;
		}
	}

	return 0;
}


int ptp_class_validity(char *class)
{
	if ((atoi(class) >= 0) && (atoi(class) <= 255))
	{
		return 1;
	}

	return 0;
}


int ptp_variance_validity(char *variance)
{
	if ((atoi(variance) >= 0) && (atoi(variance) <= 65535))
	{
		return 1;
	}

	return 0;
}

int ptp_ssmEnable_validity(char *trace)
{
	if((atoi(trace) == 0) || (atoi(trace) == 1))
	{
		return 1;
	}
	
	return 0;
}

int ptp_region_validity(char *region)
{
	if ((atoi(region) >= 0) && (atoi(region) <= 255))
	{
		return 1;
	}

	return 0;
}


/*
  1	合法
  0	非法
*/
int ptp_layer_type_validity(char *plt)
{
	int i;

	for(i=0; i<PTP_LAYER_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gPltTbl[i], plt, strlen((const char *)gPltTbl[i])))
		{
			return 1;
		}
	}

	return 0;
}











/*
  1	合法
  0	非法
*/
int ptp_step_type_validity(char *pst)
{
	int i;

	for(i=0; i<PTP_STEP_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gPstTbl[i], pst, strlen((const char *)gPstTbl[i])))
		{
			return 1;
		}
	}

	return 0;
}











/*
  1	合法
  0	非法
*/
int ptp_freq_validity(char *pf)
{
	int i;
	int len = 0;

	for(i=0; i<ARRAY_SIZE(gFreqTbl); i++)
	{
		len = (strlen(pf) > strlen((const char *)gFreqTbl[i])) ? strlen(pf) : strlen((const char *)gFreqTbl[i]); 
		if(0 == memcmp(gFreqTbl[i], pf, len))
		{
			return 1;
		}
	}

	return 0;
}



/*
  1	合法
  0	非法
*/
int ptp_priority_validity(char *priority)
{

	if ((atoi(priority) >= 0) && (atoi(priority) <= 255))
	{
		return 1;
	}

	return 0;
}









void helper(void)
{
	int i;
	
	for(i=0; gHelpTbl[i]; i++)
	{
		printf("%s\n", gHelpTbl[i]);
	}
}




