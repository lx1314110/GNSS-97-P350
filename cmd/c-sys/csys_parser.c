#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "lib_time.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_shm.h"
#include "lib_fpga.h"
#include "lib_bit.h"
#include "lib_common.h"
#include "lib_sqlite.h"

#include "alloc.h"
#include "addr.h"
#include "csys_user.h"
#include "csys_event.h"
#include "csys_net.h"
#include "csys_macro.h"
#include "csys_leap.h"
#include "csys_common.h"
#include "csys_global.h"

#include "csys_parser.h"



/*
  1	合法
  0	非法
*/
int vport_validity(char *vport)
{
	int i, len;
	int tmp;

	len = strlen(vport);
	if((vport[0] < '1') || (vport[0] > '9'))
	{
		return 0;
	}
	
	for(i=1; i<len; i++)
	{
		if((vport[i] < '0') || (vport[i] > '9'))
		{
			return 0;
		}
	}

	tmp = atoi(vport);
	if(tmp < 1 || tmp > 20)
		return 0;

	return 1;
}







/*
  1	合法
  0	非法
*/
int mid_validity(char *md5_id)
{
	int i, len;

	len = strlen(md5_id);
	if((md5_id[0] < '1') || (md5_id[0] > '9'))
	{
		return 0;
	}
	
	for(i=1; i<len; i++)
	{
		if((md5_id[i] < '0') || (md5_id[i] > '9'))
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
int mkey_validity(char *md5_key)
{
	int len;

	len = strlen(md5_key);
	if(len >= 1 && len <= MAX_MD5KEY_LEN)
		return 1;
	else
		return 0;
}


/*
  1	合法
  0	非法
*/
int interval_validity(char *interval)
{
	int i, len, tmp;

	len = strlen(interval);
	if((interval[0] < '1') || (interval[0] > '9'))
	{
		return 0;
	}
	
	for(i=1; i<len; i++)
	{
		if((interval[i] < '0') || (interval[i] > '9'))
		{
			return 0;
		}
	}

	tmp = atoi(interval);
	if(tmp >= MIN_BCAST_ITVL && tmp <= MAX_BCAST_ITVL)
		return 1;
	else
		return 0;
}







/*
  -1	失败
   0	成功
*/
int user_argv_parser(int argc, char *argv[])
{
	int ret1;
	struct userinfo users[20];
	int i;
	u16_t unum;
	
	if(2 == argc)
	{
		helper();
		return -1;
	}

	if( 0 == memcmp(argv[2], USER_ADD, strlen(USER_ADD)) )
	{
		u8_t user_permission = 0;
		if(5 == argc || 6 == argc)
		{
			if(5 == argc)
				user_permission = MINIMUM_PERMISSION;//no set, default: set min_permission
			else	//6
				user_permission = (u8_t) atoi(argv[5]);
			if( (0 == username_validity(argv[3])) && (0 == password_validity(argv[4])) )
			{
				if ( 0 != permission_validity(user_permission)){
					printf("Invalid permission.\n");
					return -1;
				}
				
				ret1 = add_user(DB_PATH, TBL_USER, argv[3], argv[4], user_permission);
				if(0 == ret1)
				{
					printf("Success.\n");
					return 0;
				}
				else if(-3 == ret1)
				{
					printf("User exist.\n");
					return -1;
				}
				else if(-5 == ret1)
				{
					printf("Already have 20 users.\n");
					return -1;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Invalid username or password.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}

	}
	else if( 0 == memcmp(argv[2], USER_DEL, strlen(USER_DEL)) )
	{
		if(4 == argc)
		{
			if(0 == username_validity(argv[3]))
			{
				ret1 = delete_user(DB_PATH, TBL_USER, argv[3]);
				if(0 == ret1)
				{
					printf("Success.\n");
					return 0;
				}
				else if(-3 == ret1)
				{
					printf("User does not exist.\n");
					return -1;
				}
				else if(-5 == ret1)
				{
					printf("Forbid to delete the last user.\n");
					return -1;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Invalid username.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else if( 0 == memcmp(argv[2], USER_MOD, strlen(USER_MOD)) )
	{
		if(6 == argc)
		{
			if( (0 == username_validity(argv[3]))&&
			    (0 == password_validity(argv[4]))&&
			    (0 == password_validity(argv[5])) )
			{
			    ret1 = modify_user(DB_PATH, TBL_USER, argv[3], argv[4], argv[5]);
			    if(0 == ret1)
			    {
			    	printf("Success.\n");
			    	return 0;
			    }
			    else if(-3 == ret1)
			    {
			    	printf("User does not exist.\n");
			    	return -1;
			    }
			    else if(-5 == ret1)
			    {
			    	printf("Mismatched original username and password.\n");
			    	return -1;
			    }
			    else
			    {
			    	printf("Failure.\n");
			    	return -1;
			    }
			}
			else
			{
				printf("Invalid username or password.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else if( 0 == memcmp(argv[2], USER_PERM_MOD, strlen(USER_PERM_MOD)) )
	{
		if(5 == argc)
		{
			if( 0 == username_validity(argv[3])&&
			    0 == permission_validity((u8_t)atoi(argv[4])) )
			{
			    ret1 = modify_user_mod(DB_PATH, TBL_USER, argv[3], (u8_t)atoi(argv[4]));
			    if(0 == ret1)
			    {
			    	printf("Success.\n");
			    	return 0;
			    }
			    else if(-3 == ret1)
			    {
			    	printf("User does not exist.\n");
			    	return -1;
			    }
			    else
			    {
			    	printf("Failure.\n");
			    	return -1;
			    }
			}
			else
			{
				printf("Invalid username or permission.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else if( 0 == memcmp(argv[2], USER_QUERY, strlen(USER_QUERY)) )
	{
		if(3 == argc)
		{
			ret1 = query_user(DB_PATH, TBL_USER, users, &unum);
			if(0 == ret1)
			{
				for(i=0; i<unum; i++)
				{
					printf("%s:%s:%d\n", users[i].username, users[i].password,users[i].permission);
				}
				return 0;
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else if( 0 == memcmp(argv[2], USER_CHECK, strlen(USER_CHECK)) )
	{
		if(5 == argc)
		{
			if((0 == username_validity(argv[3]))&&(0 == password_validity(argv[4])))
			{
				ret1 = check_user(DB_PATH, TBL_USER, argv[3], argv[4]);
				if(0 == ret1)
				{
					if(0 == log_user(DB_PATH, TBL_LOG, argv[3]))
					{
						printf("Success.\n");
						return 0;
					}
					else
					{
						printf("Failure.\n");
						return -1;
					}
				}
				else if(-3 == ret1)
				{
					printf("User does not exist.\n");
					return -1;
				}
				else if(-5 == ret1)
				{
					printf("Mismatched username and password.\n");
					return -1;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Invalid username or password.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}

	//no exec it
	printf("Success.\n");
	return 0;
}


/*
  -1	失败
   0	成功
*/
int net_argv_parser(int argc, char *argv[])
{
	int ret1;
	struct netinfo netp, tmp;
	struct ipcinfo ipc,wg_ipc;
	struct pidinfo pid_wg;
	struct msgbuf msg;
	struct config cfg;
	int i,j;
	struct ntpsta sta;
	#if MY_SWITCH
	char networkcard[20];
	char v_bcast[16];
	char r_bcast[16];
	struct in_addr in;
	#endif
	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct ntpsta), 
			 (char *)&sta, 
			 sizeof(struct ntpsta));
	
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	
	
	wg_ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_WG);
	if(-1 == wg_ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	wg_ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_WG);
	if(-1 == wg_ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	wg_ipc.ipc_base = shm_attach(wg_ipc.ipc_shm_id);
	if(((void *)-1) == wg_ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	wg_ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == wg_ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(wg_ipc.ipc_sem_id, SEMA_MEMBER_WG))
	{
		printf("Failure.\n");
		return -1;
	}
	
	shm_read(wg_ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid_wg, 
			 sizeof(struct pidinfo));//从共享内存读PID
	
	if(1 != sema_unlock(wg_ipc.ipc_sem_id, SEMA_MEMBER_WG))
	{
		printf("Failure.\n");
		return -1;
	}

	
	if(2 == argc)
	{
		memset(&netp, 0, sizeof(struct netinfo));
		ret1 = query_net_param(NETWEB_CARD, &netp);
		//if(0 == ret1){
			printf("ip %s\n", netp.ip);
			printf("mac %s\n", netp.mac);
			printf("mask %s\n", netp.mask);
			printf("gateway %s\n", netp.gateway);
			SHM_DETACH(ipc.ipc_base);
			return 0;
		//}else{
		//	printf("Failure.\n");
		//	return -1;
		//}
	}
	else
	{
		if(0 == argc%2)
		{
			memset(&netp, 0, sizeof(struct netinfo));
			memset(&tmp, 0, sizeof(struct netinfo));
			for(i=2; i<argc; i+=2)
			{
				if(0 == memcmp(argv[i], NET_IP, strlen(NET_IP)))
				{
					if(ip_validity(argv[i+1]))
					{
						//物理网卡IP与虚拟网卡IP不相同
						//物理网卡IP与虚拟网卡广播地址不相同
						for(j=0; j<20; j++)
						{
							if(	0 == strcmp((char *)sta.vp[j].v_ip, argv[i+1]) )
							{
								printf("Ip address conflict.\n");
										
								return -1;
							}
							#if MY_SWITCH
							memset(networkcard, 0, sizeof(networkcard));
							sprintf(networkcard, "eth0.%d", i+4);
							memset(v_bcast, 0, 16);
							if(0 == __Sock_GetBcast(networkcard, v_bcast))
							{
								printf("Failure.\n");
								return -1;
							}

							if(	0 == strcmp(v_bcast, argv[i+1]) )
							{
								printf("Ip address conflict.\n");
										
								return -1;
							}
							#endif
						}
						
						memcpy(netp.ip, argv[i+1], strlen(argv[i+1]));
						memcpy(tmp.ip, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid ip.\n");
						return -1;
					}
				}
				else if(0 == memcmp(argv[i], NET_GATEWAY, strlen(NET_GATEWAY)))
				{
					if(gateway_validity(argv[i+1]))
					{
						//物理网卡网关与虚拟网卡IP不相同
						//物理网卡网关与虚拟网卡广播地址不相?
						#if MY_SWITCH
						for(j=0; j<20; j++)
						{
							if(	0 == strcmp((char *)sta.vp[j].v_ip, argv[i+1]) )
							{
								printf("Gateway conflict.\n");
										
								return -1;
							}
							
							memset(networkcard, 0, sizeof(networkcard));
							sprintf(networkcard, "eth0.%d", i+4);
							memset(v_bcast, 0, 16);
							if(0 == __Sock_GetBcast(networkcard, v_bcast))
							{
								printf("Failure.\n");
								return -1;
							}

							if(	0 == strcmp(v_bcast, argv[i+1]) )
							{
								printf("Gateway conflict.\n");
										
								return -1;
							}
						}
						#endif
						memcpy(netp.gateway, argv[i+1], strlen(argv[i+1]));
						memcpy(tmp.gateway, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid gateway.\n");
						return -1;
					}
				}
				else if(0 == memcmp(argv[i], NET_MAC, strlen(NET_MAC)))
				{
					if(mac_validity(argv[i+1]))
					{
						memcpy(netp.mac, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid mac.\n");
						return -1;
					}
				}
				else if(0 == memcmp(argv[i], NET_MASK, strlen(NET_MASK)))
				{
					if(mask_validity(argv[i+1]))
					{
						memcpy(netp.mask, argv[i+1], strlen(argv[i+1]));
						memcpy(tmp.mask, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid mask.\n");
						return -1;
					}
				}
				else
				{
					helper();
					return -1;
				}
			}
			
			memset(&cfg, 0, sizeof(struct config));
			cfg.vp_config.v_port = 21;
			memcpy(cfg.vp_config.v_ip, netp.ip, strlen(netp.ip));
			memcpy(cfg.vp_config.v_mac, netp.mac, strlen(netp.mac));
			memcpy(cfg.vp_config.v_mask, netp.mask, strlen(netp.mask));
			memcpy(cfg.vp_config.v_gateway, netp.gateway, strlen(netp.gateway));
			//should memcpy(cfg.vp_config.v_ip, netp.ip, strlen(netp.ip)); by xpc
			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_WG_NET;//日志
			memcpy(msg.mdata,cfg.buffer, sizeof(struct veth));//更新配置
			/*
			if(-1 == write_virtual_eth(DB_PATH, TBL_VETH, &netp))//更新网卡信息
			{
				printf("Failure.\n");
				return -1;
			}
			*/
		
			if(MsgQ_Send(wg_ipc.ipc_msgq_id, &msg,  sizeof(struct veth)))//发送配置消息
			{
			
				if(notify(pid_wg.p_id, NOTIFY_SIGNAL))//发送通知信号
				{
					printf("Success.\n");
					SHM_DETACH(ipc.ipc_base);
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
			
		}
		else
		{
			helper();
			return -1;
		}
	}
}


/*
  -1	失败
   0	成功
*/
int leap_argv_parser(int argc, char *argv[])
{
	struct leapinfo lp;
	struct ipcinfo ipc;
	struct config update;
	struct msgbuf msg;
	struct pidinfo pid;

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_INPUT);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_INPUT);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}
	
	if(2 == argc)//从共享内存读闰秒	
	{
		if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
		{
			printf("Failure.\n");
			return -1;
		}
		shm_read(ipc.ipc_base, 
				 SHM_OFFSET_LPS, 
				 sizeof(struct leapinfo), 
				 (char *)&lp, 
				 sizeof(struct leapinfo));
		if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
		{
			printf("Failure.\n");
			return -1;
		}

		printf("%d | %d | %d\n",lp.leapMode,lp.leapSecond,lp.leapstate);
		SHM_DETACH(ipc.ipc_base);
		return 0;
	}
	else if(3 == argc)
	{
		if(atoi(argv[2]) == 0)//自动
		{
			memset(&update, 0, sizeof(struct config));
			update.leap_config.leapForecast = 0;
			update.leap_config.leapSecond = 0;
			update.leap_config.leapMode = atoi(argv[2]);
			
			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_LPS_MODE_SET;//闰秒
			memcpy(msg.mdata, update.buffer, sizeof(struct leapinfo));//更新配置闰秒
			
			if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			shm_read(ipc.ipc_base, 
					 SHM_OFFSET_PID, 
					 sizeof(struct pidinfo), 
					 (char *)&pid, 
					 sizeof(struct pidinfo));//从共享内存读PID
			if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct leapinfo)))//发送配置消息
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))//发送通知信号
				{
					printf("Success.\n");
					SHM_DETACH(ipc.ipc_base);
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else if(4 == argc)//手动
	{
		if((leap_validity(argv[3])) && atoi(argv[2]) == 1)
		{
			memset(&update, 0, sizeof(struct config));
			update.leap_config.leapForecast = 0;
			update.leap_config.leapSecond = atoi(argv[3]);
			update.leap_config.leapMode = atoi(argv[2]);

			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_LPS_MODE_SET;//闰秒
			memcpy(msg.mdata, update.buffer, sizeof(struct leapinfo));//更新配置闰秒

			if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			shm_read(ipc.ipc_base, 
					 SHM_OFFSET_PID, 
					 sizeof(struct pidinfo), 
					 (char *)&pid, 
					 sizeof(struct pidinfo));//从共享内存读PID
			if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct leapinfo)))//发送配置消息
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))//发送通知信号
				{
					SHM_DETACH(ipc.ipc_base);
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}


/*
  -1	失败
   0	成功
*/
int time_argv_parser(int argc, char *argv[])
{
	struct timeinfo ti;
	char buf[64];

	if(2 == argc)//获得系统时间
	{
		if(GetSysTime(&ti))
		{
			printf(	"%04d-%02d-%02d %02d:%02d:%02d\n",
					ti.year, 
					ti.month,
					ti.day,
					ti.hour,
					ti.minute,
					ti.second);
			
			return 0;
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else if(4 == argc)//设置系统时间
	{
		if( date_validity(argv[2]) && time_validity(argv[3]) )
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "date -s \"%s %s\"", argv[2], argv[3]);
			if(-1 != system(buf))
			{
				printf("Success.\n");
				
				return 0;
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			printf("Invalid time.\n");
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}


/*
  -1	失败
   0	成功
*/
int update_argv_parser(int argc, char *argv[])
{
	struct ipcinfo ipc;
	
	if(2 == argc)
	{
		ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_MANAGER);
		if(-1 == ipc.ipc_msgq_id)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_MANAGER);
		if(-1 == ipc.ipc_shm_id)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
		if(((void *)-1) == ipc.ipc_base)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
		if(-1 == ipc.ipc_sem_id)
		{
			printf("Failure.\n");
			return -1;
		}

		if(sysUpdate(&ipc))
		{
			printf("Success.\n");
			return 0;
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}



/*
  -1	失败
   0	成功
*/
int reboot_argv_parser(int argc, char *argv[])
{
	struct ipcinfo ipc;
	
	if(2 == argc)
	{
		ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_MANAGER);
		if(-1 == ipc.ipc_msgq_id)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_MANAGER);
		if(-1 == ipc.ipc_shm_id)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
		if(((void *)-1) == ipc.ipc_base)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
		if(-1 == ipc.ipc_sem_id)
		{
			printf("Failure.\n");
			return -1;
		}

		if(sysReboot(&ipc))
		{
			printf("Success.\n");
			return 0;
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}



/*
  -1	失败
   0	成功
*/
int guard_argv_parser(int argc, char *argv[])
{
	struct ipcinfo ipc;
	struct msgbuf msg;
	struct pidinfo pid;
	if(3 == argc)
	{
		if ((strcmp(argv[2],"on") != 0) &&((strcmp(argv[2],"off") != 0))){
		
				printf("Failure.\n");
				return -1;

		}
		ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_MANAGER);
		if(-1 == ipc.ipc_msgq_id)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_MANAGER);
		if(-1 == ipc.ipc_shm_id)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
		if(((void *)-1) == ipc.ipc_base)
		{
			printf("Failure.\n");
			return -1;
		}

		ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
		if(-1 == ipc.ipc_sem_id)
		{
			printf("Failure.\n");
			return -1;
		}

		if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_MANAGER))
		{
			printf("Failure.\n");
			return -1;
		}
		shm_read(ipc.ipc_base, 
				 SHM_OFFSET_PID, 
				 sizeof(struct pidinfo), 
				 (char *)&pid, 
				 sizeof(struct pidinfo));
		if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_MANAGER))
		{
			printf("Failure.\n");
			return -1;
		}

	
		memset(&msg, 0, sizeof(struct msgbuf));
		msg.mtype = MSG_CFG_GUARD;
		strcpy(msg.mdata,argv[2]);
		
		if(MsgQ_Send(ipc.ipc_msgq_id, &msg, strlen(msg.mdata)))
		{
			if(notify(pid.p_id, NOTIFY_SIGNAL))
				return 1;
			else
				return 0;
		}
		else
		{
			return 0;
		}

	}
	else
	{
		helper();
		return -1;
	}

}

/*
  -1	失败
   0	成功
*/
int bid_argv_parser(int argc, char *argv[])
{
	u16_t temp[10];
	//s16_t pwr_vol;
	//16+alm_board+2(pwr)+1vol=20
	u16_t bid[SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1];
	u8_t bd_maxport[SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1];
	int i,j;
	int fpga_fd;
	int board_type = BOARD_TYPE_NONE;

	memset(bd_maxport, 0x0, sizeof(bd_maxport));
	fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == fpga_fd)
	{
		printf("Failure.\n");
		return -1;
	}

	initializeBoardNameTable();

	if(!FpgaRead(fpga_fd, FPGA_BID_S17, &temp[8]))
	{
		printf("Failure.\n");
		return -1;
	}

	if(!FpgaRead(fpga_fd, FPGA_BID_S18_S19, &temp[9]))
	{
		printf("Failure.\n");
		return -1;
	}

	#if 0
	if(!FpgaRead(fpga_fd, FPGA_INT_PWR_VOL, (u16_t *)&pwr_vol))
	{
		printf("Failure.\n");
		return -1;
	}

	if(0x1FF == pwr_vol)
	{
		//无电池
		pwr_vol = 0;
	}
	else
	{
		//计算电池电量
		pwr_vol = 100*(pwr_vol - PWR_VOL_LOWER)/(PWR_VOL_UPPER - PWR_VOL_LOWER);
		
		//纠正电池电量
		if(pwr_vol <= 0)
		{
			pwr_vol = 0;
		}
		else if(pwr_vol > 100)
		{
			pwr_vol = 100;
		}
		else
		{
			//do nothing
		}
	}
	#endif
	//slot 1-16
	for(i=SLOT_CURSOR_1,j=0; (i<SLOT_CURSOR_ARRAY_SIZE)&&(j<8); i+=2,j++)
	{
		if(!FpgaRead(fpga_fd, FPGA_BID_S01_TO_S16(j), &temp[j]))
		{
			printf("Failure.\n");
			return -1;
		}

		bid[i] = (temp[j]>>8)&0x00FF;
		//if(0x00FF == bid)
		if(bid[i] >= BID_ARRAY_SIZE)
		{
			bid[i] = BID_NONE;
		}

		bid[i+1] = temp[j]&0x00FF;
		//if(0x00FF == bid)
		if(bid[i+1] >= BID_ARRAY_SIZE)
		{
			bid[i+1] = BID_NONE;
		}
	}

	//slot 17
	bid[OTHER_SLOT_CURSOR_17] = temp[8] &0x00FF;
	//if(0x00FF == bid)
	if(bid[OTHER_SLOT_CURSOR_17] >= BID_ARRAY_SIZE)
	{
		bid[OTHER_SLOT_CURSOR_17] = BID_NONE;
	}

	//slot 18
	bid[OTHER_SLOT_CURSOR_18] = (temp[9]>>8)&0x00FF;
	//if(0x00FF == bid)
	if(bid[OTHER_SLOT_CURSOR_18] >= BID_ARRAY_SIZE)
	{
		bid[OTHER_SLOT_CURSOR_18] = BID_NONE;
	}

	//slot 19
	bid[OTHER_SLOT_CURSOR_19] = temp[9]&0x00FF;
	//if(0x00FF == bid)
	if(bid[OTHER_SLOT_CURSOR_19] >= BID_ARRAY_SIZE)
	{
		bid[OTHER_SLOT_CURSOR_19] = BID_NONE;
	}

	//slot 20
	//del the function
	#if 0
	if(0x00 == pwr_vol)
		printf("%d:%s\n", BID_NONE, gBoardNameTbl[BID_NONE]);
	else
		printf("%d:%d\n", BID_PWRL, pwr_vol);
	#else
	bid[OTHER_SLOT_CURSOR_20] = BID_NONE;
	#endif
	FpgaClose(fpga_fd);

	if(OTHER_SLOT_CURSOR_20+1 != SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1){
		printf("len error.\n");
		printf("Failure.\n");
		return -1;
	}
	//total: 20
	for (i = 0; i < SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1; ++i){
		FIND_MAXPORT_GBATBL_BY_BID(bid[i], bd_maxport[i]);
		board_type = BOARD_TYPE_NONE;
		FIND_BDTYPE_GBATBL_BY_BID(bid[i], board_type);
		printf("%d:%s:%d:%s\n", bid[i], gBoardNameTbl[bid[i]], bd_maxport[i], gBdTypeTbl[board_type]);
	}
	return 0;
}

/*
  -1	失败
   0	成功
*/
int slotinfo_argv_parser(int argc, char *argv[])
{

	//max_inputslot: max:outputslot
	printf("in:%d-%d\nout:%d-%d\nalm:%d-%d\npwr:%d-%d\n",
	INPUT_SLOT_CURSOR_1+1, INPUT_SLOT_CURSOR_ARRAY_SIZE,
	SLOT_CURSOR_1+1, SLOT_CURSOR_ARRAY_SIZE, 
	OTHER_SLOT_CURSOR_17+1,OTHER_SLOT_CURSOR_17+1,
	OTHER_SLOT_CURSOR_18+1,OTHER_SLOT_CURSOR_19+1);
	return 0;
}



/*
  -1	失败
   0	成功
*/
#define CLOCK_INFO_ADD_SSM_SHOW
int clock_argv_parser(int argc, char *argv[])
{
	int i;
	int fpga_fd;
	u16_t clkType;
	u16_t clkSta;
	int phase = ~0;
	u16_t clock_state;
	char *ref_status = NULL;
	struct ipcinfo ipc;
	struct ipcinfo ipc_input;
	struct clock_stainfo clock_sta;
	struct schemainfo mode;

	//clock
	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_ALARM);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}
	
	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}


	//input
	ipc_input.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_INPUT);
	if(-1 == ipc_input.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc_input.ipc_base = shm_attach(ipc_input.ipc_shm_id);
	if(((void *)-1) == ipc_input.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}
	
	ipc_input.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc_input.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	//clock_sta
	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		printf("Failure.\n");
		return -1;
	}
	
	shm_read(ipc.ipc_base,
			 SHM_OFFSET_CLOCK,
			 sizeof(clock_sta), 
			  (char *)&clock_sta, 
			 sizeof(clock_sta));
	
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		printf("Failure.\n");
		return -1;
	}
	clock_state = clock_sta.state;
	phase = clock_sta.phase;

	//input source
	if(1 != sema_lock(ipc_input.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ipc_input.ipc_base,
			 SHM_OFFSET_SCHEMA, 
			 sizeof(struct schemainfo), 
			 (char *)&mode, 
			 sizeof(struct schemainfo));
	
	if(1 != sema_unlock(ipc_input.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == fpga_fd)
	{
		printf("Failure.\n");
		return -1;
	}

#ifdef CLOCK_INFO_ADD_SSM_SHOW
	u16_t ssm = 0x0F;
	char ssmbuf[3];
	if(!FpgaRead(fpga_fd, FPGA_SYS_2MB_OUT_SSM, &ssm))
	{
		return -1;
	}
	memset(ssmbuf, 0x0, sizeof(ssmbuf));
	ssm &= 0x000F;
	switch(ssm)
	{
		case 0x02:
			memcpy(ssmbuf, SSM_02, 2);
			break;
				
		case 0x04:
			memcpy(ssmbuf, SSM_04, 2);
			break;
				
		case 0x08:
			memcpy(ssmbuf, SSM_08, 2);
			break;

		case 0x0B:
			memcpy(ssmbuf, SSM_0B, 2);
			break;

		case 0x0F:
			memcpy(ssmbuf, SSM_0F, 2);
			break;

		case 0x00:
			memcpy(ssmbuf, SSM_00, 2);
			break;

		default:
			memcpy(ssmbuf, SSM_00, 2);
	}

#endif
	//SysReadPhase(fpga_fd, &phase);

	if(!FpgaRead(fpga_fd, FPGA_RBXO_TYP, &clkType))
	{
		printf("Failure.\n");
		return -1;
	}

	if(!FpgaRead(fpga_fd, FPGA_RBXO_STA, &clkSta))
	{
		printf("Failure.\n");
		return -1;
	}

	FpgaClose(fpga_fd);

	
	if((1 != clkType) && (2 != clkType))
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 == clkType)//钟类型
	{
		printf("%s:", "RB");
	}
	else
	{
		printf("%s:", "XO");
	}

	
	if(0 == clock_state)//跟踪模式
	{
		printf("%s\n", "FREE");
	}
	else if((1 == clock_state) || (2 == clock_state))
	{
		printf("%s\n", "HOLD");
	}
	else if(3 == clock_state)
	{
		printf("%s\n", "FAST");
	}
	else
	{
		printf("%s\n", "LOCK");
	}
	printf("PHASE:%d\n",phase);
#ifdef CLOCK_INFO_ADD_SSM_SHOW
	printf("SYS_SSM:%s\n", ssmbuf);
#endif
	char * type_name = NULL;
	//TIME_SOURCE_LEN: include nosource
	for (i = 0; i < TIME_SOURCE_LEN; ++i){
		if (gTimeSourceTbl[i].id == mode.src_inx){
			if(gTimeSourceTbl[i].type == TOD_SOURCE_TYPE){
				switch (mode.bid){
					case BID_PTP_IN:
						type_name = SIGNAL_TYPE_PTP_IN;
						break;
					case BID_RTF:
						type_name = SIGNAL_TYPE_RTF_1PPS_TOD;
						break;
				}
			}else{
				type_name = gTimeSourceTbl[i].type_name;
			}
			printf("SRC_INFO:S%dP%d-%s\n", gTimeSourceTbl[i].slot, mode.port,type_name);

			switch (mode.refsrc_is_valid){
				case REFSOURCE_INIT:
					ref_status = "INIT";
					break;
				case REFSOURCE_VALID:
					ref_status = "VALID";
					break;
				case REFSOURCE_INVALID:
					ref_status = "INVALID";
					break;
				default:
					ref_status = "N/A";
					break;
			}
			printf("SRC_STATUS:%s\n", ref_status);
		}
	}

	SHM_DETACH(ipc.ipc_base);
	SHM_DETACH(ipc_input.ipc_base);
	return 0;
}


#define WEB_FIND_STR	"web_ver="
static void get_web_ver(char *ver)
{

	char cmd[128];
	char buf[64];
	char * tmp = NULL, * tmp2 = NULL;
	const char * delim = "=";
	FILE *readfp = NULL;

	if(access(WWW_VERSION_FILE,F_OK) != 0){
		return;
	}

	memset(cmd, 0x0, sizeof(cmd));
	sprintf(cmd, "cat %s | grep \"^%s\"", WWW_VERSION_FILE, WEB_FIND_STR);
	if((readfp = popen(cmd, "r" ))==NULL)
		return;
	//printf("cmd:%s\n",cmd);
	memset(buf, 0x0, sizeof(buf));
	if(fgets(buf, sizeof(buf)-1, readfp)!=NULL)
	{
		tmp = strtok(buf, delim);
		if(tmp == NULL){
			pclose( readfp );
			return;
		}
		tmp = strtok(NULL, delim);
		if(tmp == NULL){
			pclose( readfp );
			return;
		}
		tmp2 = strchr(tmp, '\n');
		if(tmp2)
			tmp2[0]='\0';
		tmp2 = strchr(tmp, '\r');
		if(tmp2)
			tmp2[0]='\0';
		//printf("ver:%s\n",tmp);
		memcpy(ver, tmp, strlen(tmp));
		pclose( readfp );
		return;
	}
	pclose( readfp );
	return;
	
}

#define SNMP_MIB_FIND_STR	"version:"
static void get_snmp_mib_ver(char *ver)
{

	char cmd[128];
	char buf[64];
	char * tmp = NULL, * tmp2 = NULL;
	const char * delim = ":";
	FILE *readfp = NULL;

	if(access(SNMP_MIB_VERSION_FILE,F_OK) != 0){
		return;
	}

	memset(cmd, 0x0, sizeof(cmd));
	sprintf(cmd, "cat %s | grep \"^%s\"", SNMP_MIB_VERSION_FILE, SNMP_MIB_FIND_STR);
	if((readfp = popen(cmd, "r" ))==NULL)
		return;
	//printf("cmd:%s\n",cmd);
	memset(buf, 0x0, sizeof(buf));
	if(fgets(buf, sizeof(buf)-1, readfp)!=NULL)
	{
		tmp = strtok(buf, delim);
		if(tmp == NULL){
			pclose( readfp );
			return;
		}
		tmp = strtok(NULL, delim);
		if(tmp == NULL){
			pclose( readfp );
			return;
		}
		//del space
		while(tmp[0] == ' ')
			tmp++;
		tmp2 = strchr(tmp, '\n');
		if(tmp2)
			tmp2[0]='\0';
		tmp2 = strchr(tmp, '\r');
		if(tmp2)
			tmp2[0]='\0';
		//printf("ver:%s\n",tmp);
		memcpy(ver, tmp, strlen(tmp));
		pclose( readfp );
		return;
	}
	pclose( readfp );
	return;
	
}


/*
  -1	失败
   0	成功
*/
int ver_argv_parser(int argc, char *argv[])
{
	char web_ver[64];
	char snmp_mib_ver[64];
	struct verinfo ver;
	struct ipcinfo ipc;

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_MANAGER);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}
		
	memset(&ver, 0, sizeof(struct verinfo));
	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		printf("Failure.\n");
		return -1;
	}
		
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct verinfo), 
			 (char *)&ver, 
			 sizeof(struct verinfo));
		
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		printf("Failure.\n");
		return -1;
	}

	memset(web_ver, 0x0, sizeof(web_ver));
	get_web_ver(web_ver);
	memset(snmp_mib_ver, 0x0, sizeof(snmp_mib_ver));
	get_snmp_mib_ver(snmp_mib_ver);
	printf("HW_VER | %s\n", ver.hwVer);
	printf("SW_VER | %s\n", ver.swVer);
	printf("FPGA_VER | %s\n", ver.fpgaVer);
	printf("WEB_VER | %s\n", web_ver);
	printf("SNMP_MIB_VER | %s\n", snmp_mib_ver);
	printf("GCC_VER | V%s\n", __VERSION__);
	printf("BUILD_TIME | %s %s\n", __DATE__,__TIME__);

	return 0;
}







/*
  -1	失败
   0	成功
*/
int dev_type_argv_parser(int argc, char *argv[])
{
	int fpga_fd;
	u16_t devType;

	fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == fpga_fd)
	{
		printf("Failure.\n");
		return -1;
	}

	if(!FpgaRead(fpga_fd, FPGA_DEVICE_TYPE, &devType))
	{
		printf("Failure.\n");
		return -1;
	}

	FpgaClose(fpga_fd);


	if(0x02 == devType)
	{
		printf("%s\n", "master");
	}
	else if(0x01 == devType)
	{
		printf("%s\n", "slave");
	}
	else
	{
		printf("%s\n", "master");
	}
	
	return 0;
}








/*
  -1	失败
   0	成功
*/
int ntp_veth_argv_parser(int argc, char *argv[])
{
	int ret = -1;
	int i, j, tmp;
	int v_port;
	int status_monitor = 0;
	struct ipcinfo ipc1,ipc2;
	struct config cfg;
	struct msgbuf msg;
	struct pidinfo pid1,pid2;
	struct portinfo port_status[V_ETH_MAX];
	struct netinfo netp;
	struct ntpsta sta;
	char r_ip[16];
	char r_gw[16];
	char r_bcast[16];
	//struct in_addr in;
	//char v_bcast[16];
//	char networkcard[20];//eth0:x
	memset(&ipc1, 0x0, sizeof(ipc1));
	memset(&ipc2, 0x0, sizeof(ipc2));

	ipc1.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(-1 == ipc1.ipc_msgq_id)
	{
		ret = -1;
		goto exit0;
	}

	ipc1.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(-1 == ipc1.ipc_shm_id)
	{
		ret = -1;
		goto exit0;
	}

	ipc1.ipc_base = shm_attach(ipc1.ipc_shm_id);
	if(((void *)-1) == ipc1.ipc_base)
	{
		ret = -1;
		goto exit0;
	}

	ipc1.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc1.ipc_sem_id)
	{
		ret = -1;
		goto exit1;
	}

	if(1 != sema_lock(ipc1.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		ret = -1;
		goto exit1;
	}
	shm_read(ipc1.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid1, 
			 sizeof(struct pidinfo));
	
	shm_read(ipc1.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct ntpsta), 
			 (char *)&sta, 
			 sizeof(struct ntpsta));
	if(1 != sema_unlock(ipc1.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		ret = -1;
		goto exit1;
	}
	
	ipc2.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_ALARM);
	if(-1 == ipc2.ipc_msgq_id)
	{
		ret = -1;
		goto exit1;
	}
	
	ipc2.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_ALARM);
	if(-1 == ipc2.ipc_shm_id)
	{
		ret = -1;
		goto exit1;
	}

	ipc2.ipc_base = shm_attach(ipc2.ipc_shm_id);
	if(((void *)-1) == ipc2.ipc_base)
	{
		ret = -1;
		goto exit1;
	}

	ipc2.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc2.ipc_sem_id)
	{
		ret = -1;
		goto exit2;
	}

	if(1 != sema_lock(ipc2.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		ret = -1;
		goto exit2;
	}
	shm_read(ipc2.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid2, 
			 sizeof(struct pidinfo));
	
	shm_read(ipc2.ipc_base, 
			 SHM_OFFSET_PORT, 
			 V_ETH_MAX*sizeof(struct portinfo), 
			 (char *)&(port_status), 
			 V_ETH_MAX*sizeof(struct portinfo));
	if(1 != sema_unlock(ipc2.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		ret = -1;
		goto exit2;
	}
	
	if(3 == argc)
	{
		for(i=0; i<V_ETH_MAX; i++)
		{
			printf("%d %s %s %s %s %d %d\n", sta.vp[i].v_port, sta.vp[i].v_ip, sta.vp[i].v_mac, sta.vp[i].v_mask, sta.vp[i].v_gateway,port_status[i].linkstatus,port_status[i].enable);
		}

		ret = 0;
		goto no_show_exit2;
		
	}
	else if((4 == argc) && vport_validity(argv[3]))
	{
		tmp = atoi(argv[3]) -1;
		printf("%s %s %s %s %d %d\n", sta.vp[tmp].v_ip, sta.vp[tmp].v_mac, sta.vp[tmp].v_mask, sta.vp[tmp].v_gateway,port_status[tmp].linkstatus,port_status[tmp].enable);

		ret = 0;
		goto no_show_exit2;
	}
	else
	{
		if((argc > 4) && vport_validity(argv[3]) && ((argc-4)%2 == 0)){
				// if((8 == argc) && ip_validity(argv[4]) && mask_validity(argv[6]))

			v_port = atoi(argv[3]);
			memset(&netp, 0, sizeof(struct netinfo));
			for(i=4; i<argc; i+=2)
			{
				if(0 == memcmp(argv[i], NET_IP, strlen(NET_IP))){
					if(ip_validity(argv[i+1]))
					{
						//虚拟网卡IP与其它虚拟网卡IP不相同
						for(j=0; j<V_ETH_MAX; j++)
						{
							if(j+1 != v_port)
							{
								if(	0 == strcmp((char *)sta.vp[j].v_ip, argv[i+1]) )
								{
									printf("Ip address conflict.\n");
									
									ret = -1;
									goto no_show_exit2;
								}
							}
						}
						#if 0//MY_SWITCH
						//虚拟网卡IP与其它虚拟网卡广播地址不相同
						for(j=0; j<20; j++)
						{
							if(j+1 != tmp)
							{
								memset(networkcard, 0, sizeof(networkcard));
								sprintf(networkcard, "eth0.%d", j+5);
								memset(v_bcast, 0, 16);
								if(0 == __Sock_GetBcast(networkcard, v_bcast))
								{
									printf("Failure.\n");
									ret = -1;
									goto no_show_exit2;
								}

								if(0 == strcmp(v_bcast, argv[4]))
								{
									printf("Ip address conflict.");			
									ret = -1;
									goto no_show_exit2;
								}
							}
						}

						//虚拟网卡广播地址与其它虚拟网卡IP不相同
						memset(v_bcast, 0, 16);
						in.s_addr = htonl(	(ntohl(inet_addr(argv[4]))&ntohl(inet_addr(argv[5]))) | 
											(~ntohl(inet_addr(argv[5])))	);
						strncpy(v_bcast, inet_ntoa(in), 15);
						for(j=0; j<20; j++)
						{
							if(j+1 != tmp)
							{
								if(0 == strcmp(v_bcast, (char *)sta.vp[j].v_ip))
								{
									printf("Broadcast address conflict.");
													
									ret = -1;
									goto no_show_exit2;
								}
							}
						}
						#endif

						//虚拟网卡IP与物理网卡IP不相同
						memset(r_ip, 0, 16);
						if(0 == __Sock_GetIp(NETWEB_CARD, r_ip))
						{
							printf("Failure.\n");
							ret = -1;
							goto no_show_exit2;
						}
						if(0 == strcmp(r_ip, argv[i+1]))
						{
							printf("Ip address conflict.\n");
											
							ret = -1;
							goto no_show_exit2;
						}
					
						//虚拟网卡IP与物理网卡广播地址不相同
						memset(r_bcast, 0, 16);
						if(0 == __Sock_GetBcast(NETWEB_CARD, r_bcast))
						{
							printf("Failure.\n");
							ret = -1;
							goto no_show_exit2;
						}
						if(0 == strcmp(r_bcast, argv[i+1]))
						{
							printf("Ip address conflict.\n");				
							ret = -1;
							goto no_show_exit2;
						}
					
						//虚拟网卡IP与物理网卡网关不相同
						memset(r_gw, 0, 16);
						if(0 == __Sock_GetGateway(r_gw))
						{
							printf("Failure.\n");
							ret = -1;
							goto no_show_exit2;
						}
						if(0 == strcmp(r_gw, argv[i+1]))
						{
							printf("Ip address conflict.\n");				
							ret = -1;
							goto no_show_exit2;
						}
						memcpy(netp.ip, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid ip.\n");
						ret = -1;
						goto no_show_exit2;
					}
				}
				else if(0 == memcmp(argv[i], NET_GATEWAY, strlen(NET_GATEWAY)))
				{
					if(gateway_validity(argv[i+1]))
					{
						//物理网卡网关与虚拟网卡IP不相同
						//物理网卡网关与虚拟网卡广播地址不相?
						#if MY_SWITCH
						for(j=0; j<20; j++)
						{
							if(	0 == strcmp((char *)sta.vp[j].v_ip, argv[i+1]) )
							{
								printf("Gateway conflict.\n");
								ret = -1;		
								goto no_show_exit2;
							}
							
							memset(networkcard, 0, sizeof(networkcard));
							sprintf(networkcard, "eth0.%d", j+5);
							memset(v_bcast, 0, 16);
							if(0 == __Sock_GetBcast(networkcard, v_bcast))
							{
								printf("Failure.\n");
								ret = -1;
								goto no_show_exit2;
							}

							if(	0 == strcmp(v_bcast, argv[i+1]) )
							{
								printf("Gateway conflict.\n");
								ret = -1;
								goto no_show_exit2;
							}
						}
						#endif
						memcpy(netp.gateway, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid gateway.\n");
						ret = -1;
						goto no_show_exit2;
					}
				}
				else if(0 == memcmp(argv[i], NET_MAC, strlen(NET_MAC)))
				{
					if(mac_validity(argv[i+1]))
					{
						memcpy(netp.mac, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid mac.\n");
						ret = -1;
						goto no_show_exit2;
					}
				}
				else if(0 == memcmp(argv[i], NET_MASK, strlen(NET_MASK)))
				{
					if(mask_validity(argv[i+1]))
					{
						memcpy(netp.mask, argv[i+1], strlen(argv[i+1]));
					}
					else
					{
						printf("Invalid mask.\n");
						ret = -1;
						goto no_show_exit2;;
					}
				}
				else if(0 == memcmp(argv[i], NET_MONITOR, strlen(NET_MONITOR)))
				{
					status_monitor = atoi(argv[i+1]);
					if(0 > status_monitor || 1 < status_monitor)
					{
						printf("Invalid monitor.\n");
						ret = -1;
						goto no_show_exit2;;
					}

					memset(&cfg, 0, sizeof(struct config));
					cfg.port_status.port = v_port;
					cfg.port_status.enable = status_monitor;
					
					memset(&msg, 0, sizeof(struct msgbuf));
					msg.mtype = MSG_CFG_PORT_STATUS;
					memcpy(msg.mdata, cfg.buffer, sizeof(struct portinfo));
					
					if(MsgQ_Send(ipc2.ipc_msgq_id, &msg, sizeof(struct portinfo)))
					{
						if(notify(pid2.p_id, NOTIFY_SIGNAL))
						{
							ret = 0;
						}
						else
						{
							ret = -1;
							goto exit2;
						}
					}
					else
					{
						ret = -1;
						goto exit2;
					}
				}
				else
				{
					helper();
					ret = -1;
					goto no_show_exit2;
				}
				
			}

			if(netp.ip[0] != '\0' ||
				netp.mask[0] != '\0' ||
				netp.gateway[0] != '\0' ||
				netp.mac[0] != '\0')
			{
				memset(&cfg, 0, sizeof(struct config));
				cfg.vp_config.v_port = v_port;
				memcpy(cfg.vp_config.v_ip, netp.ip, strlen(netp.ip));

				memcpy(cfg.vp_config.v_mac, netp.mac, strlen(netp.mac));
				memcpy(cfg.vp_config.v_mask, netp.mask, strlen(netp.mask));
				memcpy(cfg.vp_config.v_gateway, netp.gateway, strlen(netp.gateway));
			
				memset(&msg, 0, sizeof(struct msgbuf));
				msg.mtype = MSG_CFG_NTP_VP;
				memcpy(msg.mdata, cfg.buffer, sizeof(struct veth));
				
				if(MsgQ_Send(ipc1.ipc_msgq_id, &msg, sizeof(struct veth)))
				{
					if(notify(pid1.p_id, NOTIFY_SIGNAL))
					{
						ret = 0;
						goto exit2;
					}
					else
					{
						ret = -1;
						goto exit2;
					}
				}
				else
				{
					ret = -1;
					goto exit2;
				}
			}
		}
		else
		{
			helper();
			ret = -1;
			goto no_show_exit2;
		}
		
	}
exit2:
	SHM_DETACH(ipc2.ipc_base);
exit1:
	SHM_DETACH(ipc1.ipc_base);
exit0:

	if (ret == 0){
		printf("Success.\n");
	}else{
		printf("Failure.\n");
	}
	return ret;

//no show result
no_show_exit2:
	SHM_DETACH(ipc2.ipc_base);
//no_show_exit1:
	SHM_DETACH(ipc1.ipc_base);
//no_show_exit0:

	return ret;

	
}



/*
  -1	失败
   0	成功
*/
int ntp_md5_argv_parser(int argc, char *argv[])
{
	int i, tmp;
	struct ipcinfo ipc;
	struct config cfg;
	struct msgbuf msg;
	struct pidinfo pid;
	struct ntpsta sta;

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));
	
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct ntpsta), 
			 (char *)&sta, 
			 sizeof(struct ntpsta));
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	
	if(4 == argc)
	{
		if(0 == memcmp(NTP_M_Q, argv[3], strlen(argv[3])))
		{
			printf("Total %d\n", sta.totalKey);
			for(i=0; i<MAX_MD5KEY_NUM; i++)
			{
				if(0 != sta.mKey[i].md5Id)
				{
					printf("%d %s\n", sta.mKey[i].md5Id, sta.mKey[i].md5Key);
				}
			}
			
			return 0;
		}
		else if(0 == memcmp(NTP_M_E, argv[3], strlen(argv[3])))
		{
			printf("%d\n", sta.mEn.md5Enable);

			return 0;
		}
		else
		{
			helper();
			return -1;
		}
	}
	else if(5 == argc)
	{
		if((0 == memcmp(NTP_M_D, argv[3], strlen(argv[3]))) && 
			mid_validity(argv[4]) )
		{
			tmp = atoi(argv[4]);
			for(i=0; i<MAX_MD5KEY_NUM; i++)
			{
				if(tmp == sta.mKey[i].md5Id)
				{
					memset(&cfg, 0, sizeof(struct config));
					cfg.mk_config.md5Id = tmp;

					memset(&msg, 0, sizeof(struct msgbuf));
					msg.mtype = MSG_CFG_NTP_MK_DEL;
					memcpy(msg.mdata, cfg.buffer, sizeof(struct ntpmk));
					
					if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct ntpmk)))
					{
						if(notify(pid.p_id, NOTIFY_SIGNAL))
						{
							printf("Success.\n");
							return 0;
						}
						else
						{
							printf("Failure.\n");
							return -1;
						}
					}
					else
					{
						printf("Failure.\n");
						return -1;
					}
				}
			}

			printf("Md5 key does not exist\n");
			return -1;
		}
		else if((0 == memcmp(NTP_M_E, argv[3], strlen(argv[3]))) && 
				 enable_validity(argv[4]) )
		{
			memset(&cfg, 0, sizeof(struct config));
			cfg.me_config.md5Enable = atoi(argv[4]);

			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_NTP_ME;
			memcpy(msg.mdata, cfg.buffer, sizeof(struct ntpme));
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct ntpme)))
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))
				{
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else if(6 == argc)
	{
		if((0 == memcmp(NTP_M_A, argv[3], strlen(argv[3]))) && 
			mid_validity(argv[4]) && 
			mkey_validity(argv[5]) )
		{
			if(sta.totalKey == MAX_MD5KEY_NUM)
			{
				printf("Too much md5 key\n");
				return -1;
			}
			
			tmp = atoi(argv[4]);
			for(i=0; i<MAX_MD5KEY_NUM; i++)
			{
				if(tmp == sta.mKey[i].md5Id)
				{
					printf("Md5 key already exists\n");
					return -1;
				}
			}
			memset(&cfg, 0, sizeof(struct config));
			cfg.mk_config.md5Id = tmp;
			cfg.mk_config.md5Len = strlen(argv[5]);
			memcpy(cfg.mk_config.md5Key, argv[5], cfg.mk_config.md5Len);

			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_NTP_MK_ADD;
			memcpy(msg.mdata, cfg.buffer, sizeof(struct ntpmk));
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct ntpmk)))
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))
				{
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else if((0 == memcmp(NTP_M_M, argv[3], strlen(argv[3]))) && 
			mid_validity(argv[4]) && 
			mkey_validity(argv[5]) )
		{
			tmp = atoi(argv[4]);
			for(i=0; i<MAX_MD5KEY_NUM; i++)
			{
				if(tmp == sta.mKey[i].md5Id)
				{
					memset(&cfg, 0, sizeof(struct config));
					cfg.mk_config.md5Id = tmp;
					cfg.mk_config.md5Len = strlen(argv[5]);
					memcpy(cfg.mk_config.md5Key, argv[5], cfg.mk_config.md5Len);

					memset(&msg, 0, sizeof(struct msgbuf));
					msg.mtype = MSG_CFG_NTP_MK_MOD;
					memcpy(msg.mdata, cfg.buffer, sizeof(struct ntpmk));
					
					if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct ntpmk)))
					{
						if(notify(pid.p_id, NOTIFY_SIGNAL))
						{
							printf("Success.\n");
							return 0;
						}
						else
						{
							printf("Failure.\n");
							return -1;
						}
					}
					else
					{
						printf("Failure.\n");
						return -1;
					}
				}
			}

			printf("Md5 key does not exist\n");
			return -1;
		}
		else
		{
			helper();
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}






/*
  -1	失败
   0	成功
*/
int ntp_bei_argv_parser(int argc, char *argv[])
{
	struct ipcinfo ipc;
	struct config cfg;
	struct msgbuf msg;
	struct pidinfo pid;
	struct ntpsta sta;

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));
	
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct ntpsta), 
			 (char *)&sta, 
			 sizeof(struct ntpsta));
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	
	if(3 == argc)
	{
		printf("%d %d\n", sta.bCast.bcEnable, sta.bCast.bcInterval);
			
		return 0;
	}
	else if(5 == argc)
	{
		if(	enable_validity(argv[3]) && interval_validity(argv[4]) )
		{
			memset(&cfg, 0, sizeof(struct config));
			cfg.bc_config.bcEnable = atoi(argv[3]);
			cfg.bc_config.bcInterval = atoi(argv[4]);

			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_NTP_BC;
			memcpy(msg.mdata, cfg.buffer, sizeof(struct ntpbc));
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct ntpbc)))
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))
				{
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}






/*
  -1	失败
   0	成功
*/
int ntp_ve_argv_parser(int argc, char *argv[])
{
	struct ipcinfo ipc;
	struct config cfg;
	struct msgbuf msg;
	struct pidinfo pid;
	struct ntpsta sta;

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));
	
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct ntpsta), 
			 (char *)&sta, 
			 sizeof(struct ntpsta));
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_NTP))
	{
		printf("Failure.\n");
		return -1;
	}
	
	if(3 == argc)
	{
		printf(	"%c%c%c\n", 
				(((sta.vEn.VnEnable)&BIT(0))? '1' : '0'), 
				(((sta.vEn.VnEnable)&BIT(1))? '1' : '0'), 
				(((sta.vEn.VnEnable)&BIT(2))? '1' : '0') );
			
		return 0;
	}
	else if(4 == argc)
	{
		if(	('0' == argv[3][0] || '1' == argv[3][0]) && 
			('0' == argv[3][1] || '1' == argv[3][1]) &&
			('0' == argv[3][2] || '1' == argv[3][2]) )
		{
			memset(&cfg, 0, sizeof(struct config));
			if('0' == argv[3][0])
			{
				cfg.ve_config.VnEnable &= ~BIT(0);
			}
			else
			{
				cfg.ve_config.VnEnable |= BIT(0);
			}

			if('0' == argv[3][1])
			{
				cfg.ve_config.VnEnable &= ~BIT(1);
			}
			else
			{
				cfg.ve_config.VnEnable |= BIT(1);
			}

			if('0' == argv[3][2])
			{
				cfg.ve_config.VnEnable &= ~BIT(2);
			}
			else
			{
				cfg.ve_config.VnEnable |= BIT(2);
			}

			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_NTP_VE;
			memcpy(msg.mdata, cfg.buffer, sizeof(struct ntpve));
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct ntpve)))
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))
				{
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			helper();
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}







/*
  -1	失败
   0	成功
*/
int ntp_argv_parser(int argc, char *argv[])
{
	if(argc >= 3)
	{
		if(0 == memcmp(NTP_M, argv[2], strlen(argv[2])))//查询NTP的MD5（total,ID,KEY）
		{
			return(ntp_md5_argv_parser(argc, argv));
		}
		else if(0 == memcmp(NTP_B, argv[2], strlen(argv[2])))//查询NTP广播使能和间隔 或发送广播
		{
			return(ntp_bei_argv_parser(argc, argv));
		}
		else if(0 == memcmp(NTP_V, argv[2], strlen(argv[2])))//查询或设置NTP版本
		{
			return(ntp_ve_argv_parser(argc, argv));
		}
		else if(0 == memcmp(NTP_P, argv[2], strlen(argv[2])))//查询（所有或指定）虚拟网卡信息，设置
		{
			return(ntp_veth_argv_parser(argc, argv));
		}
		else
		{
			helper();
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
}

int ReadWgIp(sqlite3 *pDb, char *pTbl ,char *ip)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select wg_ip from %s;", pTbl);
	
	db_lock(pDb);
	ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(pDb);
		
		return 0;
	}
	
	memset(ip, 0, 16);
	memcpy(ip, resultp[1], strlen(resultp[1]));
	db_free_table(resultp);
	db_unlock(pDb);
	
	return 1;
}

int wg_argv_parser(int argc, char *argv[])
{
	struct ipcinfo wg_ipc;
	struct pidinfo pid_wg;
	struct msgbuf msg;
	struct config cfg;
	sqlite3 *pDb = NULL;
	char ip[16];
	
	wg_ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_WG);
	if(-1 == wg_ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	wg_ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_WG);
	if(-1 == wg_ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	wg_ipc.ipc_base = shm_attach(wg_ipc.ipc_shm_id);
	if(((void *)-1) == wg_ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	wg_ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == wg_ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(wg_ipc.ipc_sem_id, SEMA_MEMBER_WG))
	{
		printf("Failure.\n");
		return -1;
	}
	
	shm_read(wg_ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid_wg, 
			 sizeof(struct pidinfo));//从共享内存读PID
	
	if(1 != sema_unlock(wg_ipc.ipc_sem_id, SEMA_MEMBER_WG))
	{
		printf("Failure.\n");
		return -1;
	}

	if( (argc == 3) && (0 == memcmp(argv[2], NET_IP, strlen(NET))) )//网络参数查询
	{
		
		if(0 == initializeDatabase(&pDb))//初始化数据库
		{
			printf("Failure.\n");
			return -1;
		}
		
		if(0 == ReadWgIp(pDb,TBL_SYS,ip))//初始化数据库
		{
			printf("Failure.\n");
			return -1;
		}
		if(-1 == cleanDatabase(pDb))//初始化数据库
		{
			printf("Failure.\n");
			return -1;
		}
		
		printf("%s\n",ip);
		return 0;
		
		
	}
	else if((argc == 4) && (0 == memcmp(argv[2], NET_IP, strlen(NET))) )
	{
			memset(&cfg, 0, sizeof(struct config));
			memcpy(cfg.vp_config.v_ip, argv[3], strlen(argv[3]));
			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_WG_SERVER_IP;//日志
			memcpy(msg.mdata,cfg.buffer, sizeof(struct veth));//更新配置
		
			if(MsgQ_Send(wg_ipc.ipc_msgq_id, &msg,  sizeof(struct veth)))//发送配置消息
			{
			
				if(notify(pid_wg.p_id, NOTIFY_SIGNAL))//发送通知信号
				{
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
	}
	else
	{
		helper();
		return -1;
	}
}

int Read61850Ip(sqlite3 *pDb, char *pTbl ,char *ip)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select ip from %s;", pTbl);
	
	db_lock(pDb);
	ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(pDb);
		
		return 0;
	}
	
	memset(ip, 0, 16);
	memcpy(ip, resultp[1], strlen(resultp[1]));
	db_free_table(resultp);
	db_unlock(pDb);
	
	return 1;
}



int mini61850_argv_parser(int argc, char *argv[])
{
	struct ipcinfo ipc;
	struct pidinfo pid;
	struct msgbuf msg;
	struct config cfg;
	sqlite3 *pDb = NULL;
	char ip[16];

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_61850);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_61850);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}

	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_61850))
	{
		printf("Failure.\n");
		return -1;
	}
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));//´Ó¹²ÏíÄÚ´æ¶ÁPID
	
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_61850))
	{
		printf("Failure.\n");
		return -1;
	}

	if( argc == 2 )
	{
		if(0 == initializeDatabase(&pDb))//³õÊ¼»¯Êý¾Ý¿â
		{
			printf("Failure.\n");
			return -1;
		}
		
		if(0 == Read61850Ip(pDb,TBL_61850,ip))//³õÊ¼»¯Êý¾Ý¿â
		{
			printf("Failure.\n");
			return -1;
		}
		if(-1 == cleanDatabase(pDb))//³õÊ¼»¯Êý¾Ý¿â
		{
			printf("Failure.\n");
			return -1;
		}
		
		printf("IP=%s\n",ip);
		return 0;
	}
	else if( argc == 3 )
	{
		memset(&cfg, 0, sizeof(struct config));
		memcpy(cfg.vp_config.v_ip, argv[2], strlen(argv[2]));
		memset(&msg, 0, sizeof(struct msgbuf));
		msg.mtype = MSG_CFG_61850_IP;//Íø¹ÜIP
		memcpy(msg.mdata,cfg.buffer, sizeof(struct veth));//¸üÐÂÅäÖÃ
		
		if(MsgQ_Send(ipc.ipc_msgq_id, &msg,  sizeof(struct veth)))//·¢ËÍÅäÖÃÏûÏ¢
		{
		
			if(notify(pid.p_id, NOTIFY_SIGNAL))//·¢ËÍÍ¨ÖªÐÅºÅ
			{
				printf("Success.\n");
				return 0;
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else
	{
		helper();
		return -1;
	}
	
}


/*
  -1	失败
   0	成功
*/
int argv_parser(int argc, char *argv[])//系统管理
{
	if(1 == argc)
	{
		helper();
		return -1; 
	}

	if( 0 == memcmp(argv[1], NET, strlen(NET)) )//网络参数查询
	{
		return(net_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], VER, strlen(VER)) )//版本查询
	{
		return(ver_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], DEV_TYPE, strlen(DEV_TYPE)) )//设备类型查询
	{
		return(dev_type_argv_parser(argc, argv));
	}
	
	else if( 0 == memcmp(argv[1], USER, strlen(USER)) )//用户设置
	{
		return(user_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], LEAP, strlen(LEAP)) )//闰秒查询及设置
	{
		return(leap_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], TIME, strlen(TIME)) )//系统时间获取和设置
	{
		return(time_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], UPDATE, strlen(UPDATE)) )//系统更新
	{
		return(update_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], REBOOT, strlen(REBOOT)) )//重启
	{
		return(reboot_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], GUARD, strlen(GUARD)) )//保护、告警控制（ON,OFF）
	{
		return(guard_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], CLOCK, strlen(CLOCK)) )//查询钟控类型和跟踪模式
	{
		return(clock_argv_parser(argc, argv));
	}

	else if( 0 == memcmp(argv[1], BTYPE, strlen(BTYPE)) )//查询单板类型：单板名称
	{
		return(bid_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], SLOTINFO, strlen(SLOTINFO)) )//查询槽位统计
	{
		return(slotinfo_argv_parser(argc, argv));
	}
	else if(0 == memcmp(NTP, argv[1], strlen(argv[1])))//查询或设置NTP相关信息
	{
		return(ntp_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], NET_WG, strlen(NET_WG)) )//帮助
	{
		return(wg_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], IP_61850, strlen(IP_61850)) )
	{
		return(mini61850_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], EVENT, strlen(EVENT)) )
	{
		return(event_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], PHASE_UPLOAD, strlen(PHASE_UPLOAD)) )//性能上报查询及设置
	{
		return(ph_upload_argv_parser(argc, argv));
	}
	else if( 0 == memcmp(argv[1], HELP, strlen(HELP)) )//帮助
	{
		helper();
		return 0;
	}
	else
	{
		helper();
		return -1;
	}
}





void helper(void)
{
	int i;
	
	for(i=0; gHelpTbl[i]; i++)
	{
		printf("%s\n", gHelpTbl[i]);
	}
}





