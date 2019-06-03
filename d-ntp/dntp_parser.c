#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lib_msgQ.h"
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_dbg.h"
#include "lib_net.h"

#include "dntp_config.h"
#include "dntp_alloc.h"
#include "dntp_parser.h"
#include "lib_ip1725.h"
#include "dntp_socket.h"

#if 0
void dt_port_linkstatus_all_get(int fpga_fd, unsigned int *link_status)
{
	int glport               = 3;
	unsigned char addr_base  = 0x66;
	unsigned int port_status = 0;
	unsigned short reg_value   = 0;
	
	for (addr_base = 0x65; addr_base <= 0x6d; addr_base++)
	{
		SwitchRegRead(fpga_fd, addr_base, &reg_value);
		//printf("addr[%02x] val[%04x]\n", addr_base, reg_value);
		
		if (0x65 == addr_base)
		{	
				if (reg_value & (0x1 << 5))
				{
					port_status |= 1 << 21; /* ?????˿ں? */
				}
		}
		else if (0x66 == addr_base)
		{
			if (reg_value & (0x1 << 5)) 
			{
				port_status |= 1 << 1;	/* ?˿?1 */
			}
			
			if (reg_value & (0x1 << 10)) 
			{
				port_status |= 1 << 2;  /* ?˿?2 */
			}
		}
		else /* ?˿?3-?˿?20 */
		{
			if (reg_value & 0x1) 
			{
				port_status |= 1 << glport;	
			}
			
			if (reg_value & (0x1 << 5)) 
			{
				port_status |= 1 << (glport + 1);	
			}
			
			if (reg_value & (0x1 << 10)) 
			{
				port_status |= 1 << (glport + 2);	 
			}
			
			glport += 3; /* ÿ?????˿ڶ?Ӧһ???Ĵ??? */
		}
	}
	
	*link_status = port_status;
	//printf("port_status[%0x]\n", port_status);
	
	return;		
}
#endif

int WriteEnableToShareMemory(struct ntpCtx *ctx)
{
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);

		return 0;
	}
	
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_STA, 
			  sizeof(struct ntpsta), 
			  (char *)&(ctx->ntpSta), 
			  sizeof(struct ntpsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);

		return 0;
	}

	return 1;
	
}

int WriteAlarmToShareMemory(struct ntpCtx *ctx)
{
	//write alarm to share memory
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta), 
			  (char *)ctx->alarm, 
			  ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}

	return 1;
}

bool_t isNotify(struct ntpCtx *ctx)
{
	if(ctx->notify_flag)
	{
 		ctx->notify_flag = false;
 		return true;
	}
	else
	{
		return false;
	}
}








/*
  1	?ɹ?
  0	ʧ??
*/
int ReadMessageQueue(struct ntpCtx *ctx)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
		ret = MsgQ_Rcv(ctx->ipc.ipc_msgq_id, &msg);
		if(-1 == ret)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Failed to read message queue");
			return 0;
		}
		
		if(ret > 0)
		{
			if(MSG_CFG_DBG == msg.mtype)
			{
				if(0 == mod_log_level(msg.mdata, ctx))//???ô?ӡ???ȼ?
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_MK_ADD == msg.mtype)
			{
				if(0 == add_md5_key(msg.mdata, ctx))//????MD5?ܳ?
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_MK_DEL == msg.mtype)//ɾ??MD5 KEY
			{
				if(0 == del_md5_key(msg.mdata, ctx))
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_MK_MOD == msg.mtype)//?޸?MD5 KEY
			{
				if(0 == mod_md5_key(msg.mdata, ctx))
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_BC == msg.mtype)
			{
				if(0 == mod_broadcast_enable_interval(msg.mdata, ctx))//?޸????ݿ??еĹ㲥�?ܺ͹�???����???ʱ?�?
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_VE == msg.mtype)
			{
				if(0 == mod_version_enable(msg.mdata, ctx))//?޸????ݿ??еİ汾�??
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_ME == msg.mtype)//?޸????ݿ??е?MD5ʹ??
			{
				if(0 == mod_md5_enable(msg.mdata, ctx))
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_NTP_VP == msg.mtype)//?????˿?
			{
				if(0 == sysVport(msg.mdata, ctx))//????????????
				{
					return 0;
				}

				if(0 == writeConfig(ctx))
				{
					return 0;
				}
			}
			else
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_NTP], 
						"Invalid message type");
				return 0;
			}
		}
	}while(ret > 0);
	
	return 1;
}









/*
  1	?ɹ?
  0	ʧ??
*/
int mod_log_level(char *data, struct ntpCtx *ctx)
{
	struct dbginfo *dbg;

	dbg = (struct dbginfo *)data;

	print(	DBG_INFORMATIONAL, 
			"<%s>--%d", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			dbg->dbg_level);
	
	if(0 == set_print_level(dbg->syslog_en, dbg->dbg_level,DAEMON_CURSOR_NTP))
	{
		print(	DBG_ERROR, 
				"%s", "Failed to set print level.");
		
		return 0;
	}

	return 1;
}









/*
  1	?ɹ?
  0	ʧ??
*/
int add_md5_key(char *data, struct ntpCtx *ctx)
{
	int i;
	struct ntpmk *mk;

	mk = (struct ntpmk *)data;
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%d %d %s", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			mk->md5Id, 
			mk->md5Len, 
			mk->md5Key );

	if(	(mk->md5Len <= 0) || 
		(mk->md5Len > MAX_MD5KEY_LEN) || 
		(mk->md5Len != strlen((const char *)mk->md5Key)) )
	{
		print(	DBG_ERROR, 
				"<%s>--Invalid md5 key.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	
	if(ctx->ntpSta.totalKey >= MAX_MD5KEY_NUM)
	{
		print(	DBG_ERROR, 
				"<%s>--Too much md5 key.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}

	for(i=0; i<MAX_MD5KEY_NUM; i++)
	{
		if(mk->md5Id == ctx->ntpSta.mKey[i].md5Id)
		{
			print(	DBG_ERROR, 
					"<%s>--Md5 key already exists.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );
			
			return 0;
		}
	}

	for(i=0; i<MAX_MD5KEY_NUM; i++)
	{
		if(0 == ctx->ntpSta.mKey[i].md5Id)
		{
			memcpy(&(ctx->ntpSta.mKey[i]), mk, sizeof(struct ntpmk));
			break;
		}
	}

	if(0 == md5_key_add(ctx, mk, TBL_MD5_KEY))
	{
		return 0;
	}
	
	return 1;
}









/*
  1	?ɹ?
  0	ʧ??
*/
int del_md5_key(char *data, struct ntpCtx *ctx)
{
	int i;
	struct ntpmk *mk;

	mk = (struct ntpmk *)data;
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%d", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			mk->md5Id );

	for(i=0; i<MAX_MD5KEY_NUM; i++)
	{
		if(mk->md5Id == ctx->ntpSta.mKey[i].md5Id)
		{
			memset(&(ctx->ntpSta.mKey[i]), 0, sizeof(struct ntpmk));
			
			if(0 == md5_key_del(ctx, mk, TBL_MD5_KEY))
			{
				return 0;
			}

			return 1;
		}
	}

	print(	DBG_ERROR, 
			"<%s>--Md5 key does not exist.", 
			gDaemonTbl[DAEMON_CURSOR_NTP] );

	return 0;
}









/*
  1	?ɹ?
  0	ʧ??
*/
int mod_md5_key(char *data, struct ntpCtx *ctx)
{
	int i;
	struct ntpmk *mk;

	mk = (struct ntpmk *)data;
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%d %d %s", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			mk->md5Id, 
			mk->md5Len, 
			mk->md5Key );

	if(	(mk->md5Len <= 0) || 
		(mk->md5Len > MAX_MD5KEY_LEN) || 
		(mk->md5Len != strlen((const char *)mk->md5Key)) )
	{
		print(	DBG_ERROR, 
				"<%s>--Invalid md5 key.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}

	for(i=0; i<MAX_MD5KEY_NUM; i++)
	{
		if(mk->md5Id == ctx->ntpSta.mKey[i].md5Id)
		{
			ctx->ntpSta.mKey[i].md5Len = mk->md5Len;
			memset(ctx->ntpSta.mKey[i].md5Key, 0, MAX_MD5KEY_LEN +1);
			memcpy(ctx->ntpSta.mKey[i].md5Key, mk->md5Key, mk->md5Len);
			
			if(0 == md5_key_mod(ctx, mk, TBL_MD5_KEY))
			{
				return 0;
			}

			return 1;
		}
	}

	print(	DBG_ERROR, 
			"<%s>--Md5 key does not exist.", 
			gDaemonTbl[DAEMON_CURSOR_NTP] );

	return 0;
}









/*
  1	?ɹ?
  0	ʧ??
*/
int mod_broadcast_enable_interval(char *data, struct ntpCtx *ctx)
{
	struct ntpbc *bc;

	bc = (struct ntpbc *)data;
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%d %d", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			bc->bcEnable, 
			bc->bcInterval );

	if(	((0 == bc->bcEnable) || (1 == bc->bcEnable)) && 
		((bc->bcInterval >= MIN_BCAST_ITVL) && (bc->bcInterval <= MAX_BCAST_ITVL)) )
	{
		ctx->ntpSta.bCast.bcEnable = bc->bcEnable;
		ctx->ntpSta.bCast.bcInterval = bc->bcInterval;
		
		if(ctx->ntpSta.bCast.bcEnable)//?㲥�??
		{
			if(!broadcast_enable(ctx))//??ʱ???͹�?
			{
				print(	DBG_ERROR, 
						"<%s>--Failed to enable broadcast.", 
						gDaemonTbl[DAEMON_CURSOR_NTP] );
				
				return 0;
			}			
		}
		else
		{
			if(!broadcast_disable())
			{
				print(	DBG_ERROR, 
						"<%s>--Failed to disable broadcast.", 
						gDaemonTbl[DAEMON_CURSOR_NTP] );
				
				return 0;
			}
		}

		if(0 == broadcast_enable_interval_write(ctx, bc, TBL_SYS))//???????ݿ??еĹ㲥�?ܺ͹㲥�??????
		{
			return 0;
		}

		return 1;
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--Invalid broadcast enable and interval.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
}









/*
  1	?ɹ?
  0	ʧ??
*/
int mod_version_enable(char *data, struct ntpCtx *ctx)
{
	struct ntpve *ve;

	ve = (struct ntpve *)data;
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%02X", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			ve->VnEnable );

	if((ve->VnEnable >= 0x00) && (ve->VnEnable <= 0x07))
	{
		ctx->ntpSta.vEn.VnEnable = ve->VnEnable;
		
		if(0 == version_enable_write(ctx, ve, TBL_SYS))//???????ݿ??汾�??
		{
			return 0;
		}

		return 1;
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--Invalid version enable.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
}










/*
  1	?ɹ?
  0	ʧ??
*/
int mod_md5_enable(char *data, struct ntpCtx *ctx)
{
	struct ntpme *me;

	me = (struct ntpme *)data;
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%02X", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			me->md5Enable );

	if((0 == me->md5Enable) || (1 == me->md5Enable))
	{
		ctx->ntpSta.mEn.md5Enable = me->md5Enable;

		if(0 == md5_enable_write(ctx, me, TBL_SYS))
		{
			return 0;
		}

		return 1;
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--Invalid md5 enable.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
}











/*
  1	?ɹ?
  0	ʧ??
*/
int sysVport(char *data, struct ntpCtx *ctx)
{
	struct veth *tmp;
	int i;
	char networkcard[20];//eth0:x
	char r_ip[16];
	char r_gw[16];
	char r_bcast[16];
//	struct in_addr in;
//	char v_bcast[16];
//	char networkword[40];
	int im[4];
		

	tmp = (struct veth *)data;
	

	#if 0//MY_SWITCH
	//
	for(i=0; i<V_ETH_MAX; i++)
	{
		if(i+1 != tmp->v_port)
		{
			if(	0 == strcmp((char *)ctx->ntpSta.vp[i].v_ip, (char *)tmp->v_ip) )
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_NTP], 
						"Ip address conflict." );
				
				return 0;
			}
		}
	}
	
	//????????IP???????????????�??ַ????ͬ
	for(i=0; i<20; i++)
	{
		if(i+1 != tmp->v_port)
		{
		
			memset(networkcard, 0, sizeof(networkcard));
			sprintf(networkcard, "eth0.%d", i+5);
			memset(v_bcast, 0, 16);
			if(0 == __Sock_GetBcast(networkcard, v_bcast))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_NTP], 
						"Failed to get broadcast address." );
				
				return 0;
			}

			if(0 == strcmp(v_bcast, (char *)tmp->v_ip))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_NTP], 
						"Ip address conflict." );
					
				return 0;
			}
		}
	}
	
	//?????????�??ַ??????????????IP????ͬ
	memset(v_bcast, 0, 16);
	in.s_addr = htonl(	(ntohl(inet_addr((char *)tmp->v_ip))&ntohl(inet_addr((char *)tmp->v_mask))) | 
						(~ntohl(inet_addr((char *)tmp->v_mask)))	);
	strncpy(v_bcast, inet_ntoa(in), 15);
	for(i=0; i<V_ETH_MAX; i++)
	{
		if(i+1 != tmp->v_port)
		{
			if(0 == strcmp(v_bcast, (char *)ctx->ntpSta.vp[i].v_ip))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_NTP], 
						"Broadcast address conflict." );
						
				return 0;
			}
		}
	}
	#endif
	if(tmp->v_ip[0] != '\0'){
		//校验ip地址
		memset(r_ip, 0, 16);
		if(0 == __Sock_GetIp(NETWEB_CARD, r_ip))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Failed to get ip address." );
			
			return 0;
		}
		if(0 == strcmp(r_ip, (char *)tmp->v_ip))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Ip address conflict." );
						
			return 0;
		}
		
		//检查ip和广播ip是否相同
		memset(r_bcast, 0, 16);
		if(0 == __Sock_GetBcast(NETWEB_CARD, r_bcast))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Failed to get broadcast address." );
			
			return 0;
		}
		if(0 == strcmp(r_bcast, (char *)tmp->v_ip))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Ip address conflict." );
						
			return 0;
		}

		//检查ip是否和网关重复
		memset(r_gw, 0, 16);
		if(0 == __Sock_GetGateway(r_gw))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Failed to get gateway." );
			
			return 0;
		}
		if(0 == strcmp(r_gw, (char *)tmp->v_ip))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Ip address conflict." );
							
			return 0;
		}
	}

	ctx->ntpSta.vp[tmp->v_port -1].v_port = tmp->v_port;

	if(tmp->v_ip[0] != '\0'){
		memset(&ctx->ntpSta.vp[tmp->v_port -1].v_ip, 0x0, sizeof(ctx->ntpSta.vp[tmp->v_port -1].v_ip));
		memcpy(ctx->ntpSta.vp[tmp->v_port -1].v_ip, tmp->v_ip, strlen((const char *)tmp->v_ip));
		if(sscanf((char *)ctx->ntpSta.vp[tmp->v_port -1].v_ip, "%d.%d.%d.%d", &im[0], &im[1], &im[2], &im[3]) != 4){
		      return 0;
		}
		
		for(i=0;i<4;i++){
			port[tmp->v_port-1].ip_a[i] = im[i];
			//printf("\n ip:%d",im[i]);
		}
		//printf("\n");
		port[tmp->v_port-1].ip = (port[tmp->v_port-1].ip_a[0] << 24) | (port[tmp->v_port-1].ip_a[1] << 16) | (port[tmp->v_port-1].ip_a[2] << 8) | port[tmp->v_port-1].ip_a[3];
	}

	if(0 == read_eth(ctx,TBL_VETH,tmp->v_port))//读取mac地址
	{	
		return 0;
	}
	//memcpy(ctx->ntpSta.vp[tmp->v_port -1].v_mac, tmp->v_mac, strlen((const char *)tmp->v_mac));

	if(tmp->v_mask[0] != '\0'){
		memset(&ctx->ntpSta.vp[tmp->v_port -1].v_mask, 0x0, sizeof(ctx->ntpSta.vp[tmp->v_port -1].v_mask));
		memcpy(ctx->ntpSta.vp[tmp->v_port -1].v_mask, tmp->v_mask, strlen((const char *)tmp->v_mask));

		 if(sscanf((char *)ctx->ntpSta.vp[tmp->v_port -1].v_mask, "%d.%d.%d.%d", &im[0], &im[1], &im[2], &im[3]) != 4){
		      return 0;
		 }
		
		for(i=0;i<4;i++){
			port[tmp->v_port-1].mask_a[i] = im[i];
			//printf("\n mask:%d",im[i]);
		}
		//printf("\n");
		port[tmp->v_port-1].mask = (port[tmp->v_port-1].mask_a[0] << 24) | (port[tmp->v_port-1].mask_a[1] << 16) | (port[tmp->v_port-1].mask_a[2] << 8) |port[tmp->v_port-1].mask_a[3];

	}

	if(tmp->v_gateway[0] != '\0'){
		memset(&ctx->ntpSta.vp[tmp->v_port -1].v_gateway, 0x0, sizeof(ctx->ntpSta.vp[tmp->v_port -1].v_gateway));
		memcpy(ctx->ntpSta.vp[tmp->v_port -1].v_gateway, tmp->v_gateway, strlen((const char *)tmp->v_gateway));
	}
	//for(i=0; i<V_ETH_MAX; i++)
	//{
		#if 0// MY_SWITCH
		memset(networkcard, 0, sizeof(networkcard));
		sprintf(networkcard, "eth0.%d", tmp->v_port+4);
		/*
		sprintf(networkword, "ifconfig %s %s netmask %s up", networkcard,ctx->ntpSta.vp[tmp->v_port -1].v_ip,ctx->ntpSta.vp[tmp->v_port -1].v_mask);
		system(networkword);
		*/
		
		if(0 == ethn_add(networkcard, (char *)ctx->ntpSta.vp[tmp->v_port -1].v_ip, NULL, (char *)ctx->ntpSta.vp[tmp->v_port -1].v_mask, (char *)ctx->ntpSta.vp[tmp->v_port -1].v_gateway))//????????
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Failed to modify virtual ether.");
			
			return 0;
		}
	//}
		#endif

	memset(networkcard, 0, sizeof(networkcard));
	sprintf(networkcard, "eth0.%d",tmp->v_port+4);
	strncpy(port[tmp->v_port-1].name, networkcard, IF_NAMESIZE);
	//printf("eth[%d]:%s\n",tmp->v_port+4,networkcard);

	//system(&networkcard);
	if(0 == write_virtual_eth(ctx, tmp, TBL_VETH))//写配置信息到数据库
	{
		
		return 0;
	}
	
	return 1;
}









/*
  1	?ɹ?
  0	ʧ??

  write config to share memory
*/
int writeConfig(struct ntpCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);

		return 0;
	}
	
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_STA, 
			  sizeof(struct ntpsta), 
			  (char *)&(ctx->ntpSta), 
			  sizeof(struct ntpsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);

		return 0;
	}

	return 1;
}







