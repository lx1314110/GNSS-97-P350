#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "lib_shm.h"
#include "lib_fpga.h"
#include "alloc.h"

#include "cin_global.h"
#include "cin_macro.h"
#include "cin_alloc.h"
#include "cin_query.h"
#include "cin_set.h"
#include "cin_parser.h"
#include "lib_net.h"


int argv_parser(int argc, char *argv[], struct inCtx *ctx)
{
	int ret = 0,prio_ret=0;
	int i;
	struct insetinfo ds;

	//initialize
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		printf("Failure.\n");
		return -1;
	}
	
	if(-1 == initializeMessageQueue(&ctx->ipc))
	{
		FpgaClose(ctx->fpga_fd);
		printf("Failure.\n");
		return -1;
	}
	if(-1 == initializeSemaphoreSet(&ctx->ipc))
	{
		FpgaClose(ctx->fpga_fd);
		printf("Failure.\n");
		return -1;
	}
	if(-1 == initializeShareMemory(&ctx->ipc))
	{
		//cleanShareMemory(&ctx->ipc);
		FpgaClose(ctx->fpga_fd);
		printf("Failure.\n");
		return -1;
	}

	if(-1 == readPid(&ctx->ipc, &ctx->pid))
	{
		//cleanShareMemory(&ctx->ipc);
		FpgaClose(ctx->fpga_fd);
		printf("Failure.\n");
		return -1;
	}
	
	if(2 == argc)//查询
	{
		if(0 == memcmp(argv[1], SCHEMA, strlen(SCHEMA)))
		{
			if(-1 == schema_query(ctx))//模式查询
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
		else if(0 == memcmp(argv[1], GPSBD, strlen(GPSBD)))
		{
			if(-1 == gps_bd_query(ctx))//GPS输入信号查询
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
	else if(3 == argc)
	{
		if(0 == memcmp(argv[1], SCHEMA, strlen(SCHEMA)) && \
			0 == memcmp(argv[2], LIST, strlen(LIST)))
		{
			if(-1 == schema_list_query(ctx))//所有支持的选源列表查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], SCHEMA, strlen(SCHEMA)) && \
			0 == memcmp(argv[2], VALID_LIST, strlen(VALID_LIST)))
		{
			if(-1 == schema_cur_valid_list_query(ctx))//有效的选源列表查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if( (0 == memcmp(argv[2], PORTINFO, strlen(PORTINFO))) &&
			(slot_validity(argv[1])) )
		{
			if(0 == portinfo_query(ctx, atoi(argv[1])))//输入端口信息查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], PORTSRCTYPE, strlen(PORTSRCTYPE))) &&
			(slot_validity(argv[1])) )
		{
			if(0 == port_srctype_query(ctx, atoi(argv[1])))//输入端口信息查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], PRIO, strlen(PRIO))) &&
			(slot_validity(argv[1])) )
		{
			if(0 == prio_query(ctx, atoi(argv[1]), 0))//优先级查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[1], SCHEMA, strlen(SCHEMA))) &&
				(0 == memcmp(argv[2], SCHEMA_FREE, strlen(SCHEMA_FREE))) )
		{
			if(0 == schema_set(ctx, 0, 0))//设置输入信号选源模式为FREE
			{
				ret = 0;
				printf("Success.\n");
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], SSM, strlen(SSM))) &&
				(slot_validity(argv[1])) )
		{
			if(0 == ssm_query(ctx, atoi(argv[1]), 0))//时钟等级查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], SA, strlen(SA))) &&
				(slot_validity(argv[1])) )
		{
			if(0 == sa_query(ctx, atoi(argv[1]), 0))//SA 字节查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], SIGNAL, strlen(SIGNAL))) &&
				(slot_validity(argv[1])) )
		{
			if(0 == signal_query(ctx, atoi(argv[1])))//信号查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], MODE, strlen(MODE))) &&
				(slot_validity(argv[1])) )
		{
			if(0 == mode_query(ctx, atoi(argv[1])))//接收机工作模式查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], ELEV, strlen(ELEV))) &&
				(slot_validity(argv[1])) )
		{
			if(0 == elev_query(ctx, atoi(argv[1])))//海拔高度查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], LOLA, strlen(LOLA))) &&
				(slot_validity(argv[1])) )
		{
			if(0 == lola_query(ctx, atoi(argv[1])))//经纬度查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if(0 == memcmp(argv[1], PTP, strlen(PTP)) &&
			slot_validity(argv[2]))
		{
			if(0 == input_ptpin_board_query(ctx, atoi(argv[2])))//ptp输入查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (slot_validity(argv[1])) &&
				(port5_validity(argv[2])) )
		{
			if(0 == port_query(ctx, atoi(argv[1]), atoi(argv[2])))//输入卫星信号查询
			{
				ret = 0;
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
	else if(4 == argc)
	{
		if( (0 == memcmp(argv[3], PRIO, strlen(PRIO))) &&
			(slot_validity(argv[1])) &&
			(port5_validity(argv[2]))
			)
		{
			if(0 == prio_query(ctx, atoi(argv[1]), atoi(argv[2])))//优先级查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], PRIO, strlen(PRIO))) &&
			(slot_validity(argv[1])) &&
			(prio_validity(argv[3])) )
		{
			prio_ret=prio_set(ctx, atoi(argv[1]), 0, (u8_t *)argv[3]);
			if(0 == prio_ret)//设置优先级		
			{
				ret = 0;
				printf("Success.\n");
			}
			else if(-2 == prio_ret)
			{
				printf("Same.\n");
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[3], DELAY, strlen(DELAY))) &&
				(slot_validity(argv[1])) && 
				(port5_validity(argv[2])) )
		{
			if(0 == delay_query(ctx, atoi(argv[1]), atoi(argv[2])))//延时查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[1], SCHEMA, strlen(SCHEMA))) &&
				(0 == memcmp(argv[2], SCHEMA_FORCE, strlen(SCHEMA_FORCE))) &&
				(schema_validity(argv[3])) )
		{
			if(0 == schema_set(ctx, 1, atoi(argv[3])))//选源模式设置
			{
				ret = 0;
				printf("Success.\n");
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[3], SSM, strlen(SSM))) &&
				(slot_validity(argv[1])) && 
				(port5_validity(argv[2])))
		{
			if(0 == ssm_query(ctx, atoi(argv[1]), atoi(argv[2])))//时钟等级查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[3], SA, strlen(SA))) &&
				(slot_validity(argv[1])) && 
				(port5_validity(argv[2])))
		{
			if(0 == sa_query(ctx, atoi(argv[1]), atoi(argv[2])))//SA 字节查询
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], SSM, strlen(SSM))) &&
				(slot_validity(argv[1])) &&
				(ssm_validity(argv[3])) )
		{
			if(0 == ssm_set(ctx, atoi(argv[1]), 0, (u8_t *)argv[3]))//时钟等级设置
			{
				ret = 0;
				printf("Success.\n");
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], SA, strlen(SA))) &&
				(slot_validity(argv[1])) &&
				(sa_validity(argv[3])) )
		{
			if(0 == sa_set(ctx, atoi(argv[1]), 0, (u8_t *)argv[3]))//SA字节设置
			{
				ret = 0;
				printf("Success.\n");
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], MODE, strlen(MODE))) &&
				(slot_validity(argv[1])) &&
				(mode_validity(argv[3])))
		{
			if(0 == mode_set(ctx, atoi(argv[1]), (u8_t *)argv[3]))//接收机接收模式设置
			{
				ret = 0;
				printf("Success.\n");
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[2], ELEV, strlen(ELEV))) &&
				(slot_validity(argv[1])) &&
				(elev_validity(argv[3])))
		{
			if(0 == elev_set(ctx, atoi(argv[1]), (u8_t *)argv[3]))//海拔设置
			{
				ret = 0;
				printf("Success.\n");
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
	else if(5 <= argc)
	{
		if(5 == argc)
		{
			if( (0 == memcmp(argv[3], PRIO, strlen(PRIO))) &&
				(slot_validity(argv[1])) &&
				(port5_validity(argv[2])) &&
				(prio_validity(argv[4])) )
			{
				prio_ret=prio_set(ctx, atoi(argv[1]), atoi(argv[2]), (u8_t *)argv[4]);
				if(0 == prio_ret)//设置优先级		
				{
					ret = 0;
					printf("Success.\n");
				}
				else if(-2 == prio_ret)
				{
					printf("Same.\n");
					ret = 0;
				}
				else
				{
					ret = -1;
				}
				goto exit;
			}
			else if( (0 == memcmp(argv[3], DELAY, strlen(DELAY))) &&
				(slot_validity(argv[1])) && 
				(port5_validity(argv[2])) &&
				(delay_validity(argv[4])) )
			{
				if(0 == delay_set(ctx, atoi(argv[1]), atoi(argv[2]) ,atoi(argv[4])))//时延设置
				{
					ret = 0;
					printf("Success.\n");
				}
				else
				{
					ret = -1;
				}
				goto exit;
			}
			else if( (0 == memcmp(argv[3], SSM, strlen(SSM))) &&
					(slot_validity(argv[1])) &&
					(port5_validity(argv[2])) &&
					(ssm_validity(argv[4])) )
			{
				if(0 == ssm_set(ctx, atoi(argv[1]), atoi(argv[2]), (u8_t *)argv[4]))//时钟等级设置
				{
					ret = 0;
					printf("Success.\n");
				}
				else
				{
					ret = -1;
				}
				goto exit;
			}
			else if( (0 == memcmp(argv[3], SA, strlen(SA))) &&
					(slot_validity(argv[1])) &&
					(port5_validity(argv[2])) &&
					(sa_validity(argv[4])) )
			{
				if(0 == sa_set(ctx, atoi(argv[1]), atoi(argv[2]), (u8_t *)argv[4]))//SA字节设置
				{
					ret = 0;
					printf("Success.\n");
					goto exit;
				}
				else
				{
					ret = -1;
				}
				goto exit;
			}
			else
			{
				ret = -1;
			}
		}
		else if(7 == argc)
		{
			if( (0 == memcmp(argv[2], LOLA, strlen(LOLA))) &&
				(slot_validity(argv[1])) &&
				((1 == strlen(argv[3])) && ('E' == argv[3][0] || 'W' == argv[3][0])) &&
				(lon_validity(argv[4])) &&
				((1 == strlen(argv[5])) && ('N' == argv[5][0] || 'S' == argv[5][0])) &&
				(lat_validity(argv[6])) )
			{
				if(0 == lola_set(ctx, atoi(argv[1]), argv[3][0], (u8_t *)argv[4], argv[5][0], (u8_t *)argv[6]))//经纬设置
				{
					ret = 0;
					printf("Success.\n");
				}
				else
				{
					ret = -1;
				}
				goto exit;
			}
			else
			{
				ret = -1;
			}
		}

		if(argc > 4 && 0 == memcmp(argv[1], PTP, strlen(PTP)) &&
			1 == slot_validity(argv[2]))
		{
			
			if(0 == (argc-3) %2)
			{
				memset(&ds, 0, sizeof(struct insetinfo));
				ds.slot = atoi(argv[2]);
				for(i=3; i<argc; i+=2)
				{
					if((0 == memcmp(argv[i], PTP_DELAY_TYPE, strlen(PTP_DELAY_TYPE)))&&
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
					else if((0 == memcmp(argv[i], PTP_DELAY_REQ_FREQ, strlen(PTP_DELAY_REQ_FREQ)))&&
							(ptp_freq_validity(argv[i+1])))
					{
						ds.set_ptp_delay_req_freq_flag = 0x01;
						memcpy(ds.ptpDelayReq, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_P_DELAY_REQ_FREQ, strlen(PTP_P_DELAY_REQ_FREQ)))&&
							(ptp_freq_validity(argv[i+1])))
					{
						ds.set_ptp_pdelay_req_freq_flag = 0x01;
						memcpy(ds.ptpPdelayReq, argv[i+1], strlen(argv[i+1]));
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
					else if((0 == memcmp(argv[i], PTP_MASTER_IP, strlen(PTP_MASTER_IP)))&&
							(ip_group_validity(argv[i+1])))
					{
						ds.set_ptp_master_ip_flag = 0x01;
						memcpy(ds.ptp_Master_Ip, argv[i+1], strlen(argv[i+1]));
					}
					else if((0 == memcmp(argv[i], PTP_MASTER_MAC, strlen(PTP_MASTER_MAC)))&&
							(mac_validity(argv[i+1])))
					{
						ds.set_ptp_master_mac_flag = 0x01;
						memcpy(ds.ptp_Master_Mac, argv[i+1], strlen(argv[i+1]));
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
				
				if(-1 == input_ptpin_board_set(ctx, &ds))//输出板卡设置
				{
					ret = -1;
				}
				else
				{
					ret = 0;
					printf("Success.\n");
				}
				goto exit;
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
	else
	{
		ret = -1;
	}

exit:
	//clean
	//cleanShareMemory(&ctx->ipc);
	if( 1 == shm_detach(ctx->ipc.ipc_base) )
		ctx->ipc.ipc_base = (void *)-1;
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
int port5_validity(char *port)
{
	int i;
	if(port)
	{
		for (i = 0; i < strlen(port); ++i){
			if(isdigit(port[i]) == 0)
				return 0;//no digit, return failure
		}
		if(atoi(port) >= 1 && atoi(port) <= INPUT_SLOT_MAX_PORT)
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
int schema_validity(char *src_num)
{
	//int len;
	int ret = 0;
	char sel = 0;

	sel=atoi(src_num);
	//len = strlen(src_num);
	if (sel >= 0 && sel < TIME_SOURCE_MAX_NOSOURCE)
	{
		ret = 1;
	}
	return ret;
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
		if(atoi(slot) >= INPUT_SLOT_CURSOR_1+1 && atoi(slot) <= INPUT_SLOT_CURSOR_ARRAY_SIZE)
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
int prio_validity(char *prio)
{
	int i,len;

	len = strlen(prio);
	//if(5 == len)
	if(INPUT_SLOT_MAX_PORT >= len)
	{
		for(i=0; i<len; i++)
		{
			if( !((prio[i] >= '0' && prio[i] <= '9') ||
				('-' == prio[i])) )
			{
				return 0;
			}
		}
		
		return 1;
	}
	else
	{
		return 0;
	}
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
	ssm: 02--000F
	ssm:--
	ssm:02
  1	合法
  0	非法
*/
int ssm_validity(char *ssm)
{
	int i, j;
	int suss = 0;
	char *tmp_ssm = NULL;

	if(!ssm)
		return 0;

	tmp_ssm = ssm;
#if 0
	while(tmp_ssm[0] == ' '){
		tmp_ssm++;//del head space
	}
#endif
	if(strlen(tmp_ssm) > STR_SSM_LEN*INPUT_SLOT_MAX_PORT)
		return 0;
	for (i = 0; i < strlen(tmp_ssm); i+=STR_SSM_LEN){
		suss = 0;
		for(j=0; j<gSsmTbl_len; j++)
		{
			if(0 == memcmp(gSsmTbl[j].v_str, &tmp_ssm[i], STR_SSM_LEN))
			{
				//yes
				suss = 1;
				break;
			}
		}
		if(suss == 0)
			return 0;//sub no found
	}

	if(suss == 1)
		return 1;
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
int mode_validity(char *mode)
{
	int i;
	
	for(i=0; i<MODE_CURSOR_ARRAY_SIZE; i++)
	{
		if(0 == memcmp(gModeTbl[i], mode, strlen(mode)))
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
int elev_validity(char *elev)
{
	int i;
	int len;

	len = strlen(elev);
	if(len < 3)
	{
		return 0;
	}

	if(!((elev[len-1] >= '0') && (elev[len-1] <= '9')))
	{
		return 0;
	}

	if('.' != elev[len-2])
	{
		return 0;
	}

	len -= 2;
	
	if(1 == len)
	{
		if(!((elev[0] >= '0') && (elev[0] <= '9')))
		{
			return 0;
		}
	}
	else
	{
		if(!(('-' == elev[0]) || ((elev[0] >= '1') && (elev[0] <= '9'))))
		{
			return 0;
		}
		if('-' == elev[0])
		{
			if(!((elev[1] >= '0') && (elev[1] <= '9')))
			{
				return 0;
			}
		}
		
		for(i=1; i<len; i++)
		{
			if(!((elev[i] >= '0') && (elev[i] <= '9')))
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
int lon_validity(char *lon)
{
	int i;
	int len;

	len = strlen(lon);
	if(11 != len)
	{
		return 0;
	}

	if('.' != lon[5])
	{
		return 0;
	}

	for(i=0; i<5; i++)
	{
		if(!((lon[i] >= '0') && (lon[i] <= '9')))
		{
			return 0;
		}
	}

	for(i=6; i<len; i++)
	{
		if(!((lon[i] >= '0') && (lon[i] <= '9')))
		{
			return 0;
		}
	}

	return 1;
}



/*
  1	合法
  0	非法
*/
int lat_validity(char *lat)
{
	int i;
	int len;

	len = strlen(lat);
	if(10 != len)
	{
		return 0;
	}

	if('.' != lat[4])
	{
		return 0;
	}

	for(i=0; i<4; i++)
	{
		if(!((lat[i] >= '0') && (lat[i] <= '9')))
		{
			return 0;
		}
	}

	for(i=5; i<len; i++)
	{
		if(!((lat[i] >= '0') && (lat[i] <= '9')))
		{
			return 0;
		}
	}

	return 1;
}


//ptp 

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

#if 0
int ptp_ssmEnable_validity(char *trace)
{
	if((atoi(trace) == 0) || (atoi(trace) == 1))
	{
		return 1;
	}
	
	return 0;
}
#endif

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




