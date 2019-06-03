#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_msgQ.h"
#include "lib_common.h"

#include "alloc.h"
#include "cin_macro.h"
#include "cin_set.h"


static int str_ip_group_to_uint32(char *ipv4_grp, u32_t *des_ipv4, u8_t * des_ipv4_num);

/*
  -1	失败
   0	成功
*/
#define CHECK_VALID_SCHEMA
int schema_set(struct inCtx *ctx, int schema, int which_src)
{
	struct config cfg;
	struct msgbuf msg;


#ifdef CHECK_VALID_SCHEMA
	int found = 0;
	int i,j;
	int s = 0;
	int bd_maxport = 0;
	int board_type = BOARD_TYPE_NONE;
	struct inputsta sta[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	struct port_attr_t *port_attr = NULL;
	
if(schema != 0){
	//force mode
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 INPUT_SLOT_CURSOR_ARRAY_SIZE*sizeof(struct inputsta), 
			 (char *)sta, 
			 INPUT_SLOT_CURSOR_ARRAY_SIZE*sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	for (s = 0; s < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++s){
		board_type = BOARD_TYPE_NONE;
		FIND_BDTYPE_GBATBL_BY_BID(sta[s].boardId, board_type);
		if(board_type != BOARD_TYPE_INPUT)
			continue;

		bd_maxport = 0;
		FIND_MAXPORT_GBATBL_BY_BID(sta[s].boardId, bd_maxport);
		if (bd_maxport <= 0){
			return -1;
		}

		port_attr = NULL;
		FIND_PORTATTR_GBATBL_BY_BID(sta[s].boardId, port_attr);
		if (!port_attr){
			return -1;
		}

		//printf("%d",bd_maxport);
		for (i = 0; i < bd_maxport; ++i){
			for (j = 0; j < TIME_SOURCE_LEN; ++j){
				if(gTimeSourceTbl[j].slot == s+1 &&
					gTimeSourceTbl[j].type == port_attr[i].insrc_type){
					if(gTimeSourceTbl[j].id == which_src){
						found = 1;
						break;
					}
				}
			}
		}
	}

	if(!found){
		//printf("valid source\n");
		return -1;
	}
}
#endif

	memset(&cfg, 0, sizeof(struct config));
	cfg.schema_config.schema = schema;
	cfg.schema_config.src_inx = which_src;
	
	msg.mtype = MSG_CFG_SCHEMA;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct schemainfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct schemainfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
   port is 0, set the prio of all port for the slot
	port is 1-5
*/
int prio_set(struct inCtx *ctx, int slot, int port, u8_t *prio)
{
	int bd_maxport = 0;
	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	//int i,j,slot_another,k;
	//u8_t all[10]={0};

	//slot_another=(slot==1)?2:1;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	#if 0 //no check prio is same
	if(sta2.gpsbf.gpsbf.prio[0] == 0)
	{
		k=5;
	}
	else
	{
		k=10;
	}
	memcpy(all,prio,5);
	memcpy(&all[5],sta2.gpsbf.gpsbf.prio,5);
	
	for(i=0;i<k-1;i++)
	{
		for(j=i+1;j<k;j++)
		{
			if(all[i]==all[j])
			{
				return -2;
			}
		}
	}
	#endif
	
	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_INCOMM_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;


	//backup orgin info
	memcpy(&tmp.in_config.incomminfo, &sta.incommsta.incomminfo, sizeof(struct incommoninfo));
	//modify new a config
	if(port == 0){
		memcpy(tmp.in_config.incomminfo.prio, prio, strlen((const char *)prio));
	}else{
		FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
		if (bd_maxport <= 0){
			return -1;
		}
		if(bd_maxport < port){
			return -1;
		}
		memcpy(&tmp.in_config.incomminfo.prio[port-1], &prio[0], sizeof(char));
	}
	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
*/
int delay_set(struct inCtx *ctx, int slot, int port, int delay)
{
	int bd_maxport = 0;
	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;

	delay /= 10;//because fpga x10
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_INCOMM_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;
	//backup orgin info
	memcpy(&tmp.in_config.incomminfo, &sta.incommsta.incomminfo, sizeof(struct incommoninfo));

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	if(bd_maxport < port){
		return -1;
	}
	//set new a config
	tmp.in_config.incomminfo.delay[port-1] = delay;


	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
   当前支持频率源设置，时间源，2mb频率源设置暂不支持，若支持请更改数据库i_ssm: --to0x
*/
int ssm_set(struct inCtx *ctx, int slot, int port, u8_t *ssm)
{
	u8_t src_flag = INVALID_SOURCE_FLAG;
	int bd_maxport = 0;
	int i;
	struct port_attr_t *port_attr = NULL;

	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_INCOMM_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;

	//backup orgin info
	memcpy(&tmp.in_config.incomminfo, &sta.incommsta.incomminfo, sizeof(struct incommoninfo));

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
	if (!port_attr){
		return -1;
	}

	//modify new a config
	int ssm_val = -1;
	if(port == 0){
		//for (i = 0; i < INPUT_SLOT_MAX_PORT; i++){
		for (i = 0; i < bd_maxport; i++){
			ssm_val = -1;
			FIND_VAL_BY_SSM_STR(&ssm[i*STR_SSM_LEN], ssm_val);
			if(ssm_val != -1){
				src_flag = INVALID_SOURCE_FLAG;
				FIND_FLAG_BY_TYPE(port_attr[i].insrc_type, src_flag);
				if(S2MB_SOURCE_TYPE != port_attr[i].insrc_type && 
					FREQ_SOURCE_FLAG == src_flag &&
					ssm_val != SSM_VAL_NE){
					//2mb and time source disable set. set value not --
					tmp.in_config.incomminfo.ssm[i] = (u8_t) ssm_val;
				}
				//全部设置时，当某个端口为--时，不报错误，但是该端口设置--不生效
			}else{
				return -1;
			}
		}
	}else{
		if(strlen((char *)ssm) > (size_t) STR_SSM_LEN)
			return -1;
		if(bd_maxport < port)
			return -1;

		ssm_val = -1;
		FIND_VAL_BY_SSM_STR(ssm, ssm_val);
		if(ssm_val != -1&& ssm_val != SSM_VAL_NE){
			src_flag = INVALID_SOURCE_FLAG;
			FIND_FLAG_BY_TYPE(port_attr[port-1].insrc_type, src_flag);
			if(S2MB_SOURCE_TYPE != port_attr[port-1].insrc_type && 
				FREQ_SOURCE_FLAG == src_flag){
				//2mb and time source disable set, set value not --
				tmp.in_config.incomminfo.ssm[port-1] = (u8_t) ssm_val;
			}else{
				return -1;
			}
		}else{
				return -1;
		}
	}
	
	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo))){
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL)){
			return 0;
		}else{
			return -1;
		}
	}else{
		return -1;
	}
}

/*==========================follow is SAT PRIVATE=================================*/

/*
  -1	失败
   0	成功
   当前仅仅支持卫星盘1路2mb
*/
int sa_set(struct inCtx *ctx, int slot, int port, u8_t *sa)
{
	int bd_maxport = 0;
	int found = 0;
	int i;
	struct port_attr_t *port_attr = NULL;

	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}

	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_MB_SA))
		return -1;
	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_PRIV_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;

	if(port != 0){
		//is 2mb port
		FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
		if (bd_maxport <= 0){
			return -1;
		}
		FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
		if (!port_attr){
			return -1;
		}

		for (i = 0; i < bd_maxport; ++i){
			if(S2MB_SOURCE_TYPE == port_attr[i].insrc_type && port == i+1){
				//only support a ssm for a slot
				found = 1;
				break;
			}
		}
		if(found == 0)
			return -1;
	}

	//backup the orgin config
	memcpy(&tmp.in_config.satcomminfo, &sta.satcommon.satcomminfo, sizeof(struct satcommoninfo));
	memcpy(tmp.in_config.satcomminfo.mb_sa, sa, strlen((const char *)sa));

	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
*/
int mode_set(struct inCtx *ctx, int slot, u8_t *mode)
{
	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	u8_t sysmode;

	if(0 == memcmp(gModeTbl[MODE_CURSOR_GPS], mode, strlen((const char *)mode)))
	{
		sysmode = MODE_CURSOR_GPS_VAL;
	}
	else if(0 == memcmp(gModeTbl[MODE_CURSOR_BD], mode, strlen((const char *)mode)))
	{
		sysmode = MODE_CURSOR_BD_VAL;
	}
	else if(0 == memcmp(gModeTbl[MODE_CURSOR_MIX_GPS], mode, strlen((const char *)mode)))
	{
		sysmode = MODE_CURSOR_MIX_GPS_VAL;
	}
	else
	{
		sysmode = MODE_CURSOR_MIX_BD_VAL;
	}
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_SYSMODE))
		return -1;

	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_PRIV_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;

	//backup the orgin config
	memcpy(&tmp.in_config.satcomminfo, &sta.satcommon.satcomminfo, sizeof(struct satcommoninfo));
	tmp.in_config.satcomminfo.sysMode = sysmode;

	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
*/
int elev_set(struct inCtx *ctx, int slot, u8_t *elev)
{
	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_ELEV))
		return -1;

	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_PRIV_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;

	//backup the orgin config
	memcpy(&tmp.in_config.satcomminfo, &sta.satcommon.satcomminfo, sizeof(struct satcommoninfo));
	memcpy(tmp.in_config.satcomminfo.elev, elev, strlen((const char *)elev));

	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
*/
int lola_set(struct inCtx *ctx, int slot, u8_t lot, u8_t *lov, u8_t lat, u8_t *lav)
{
	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_LON))
		return -1;

	memset(&tmp, 0, sizeof(struct config));
	tmp.in_config.set_which = INPUTINFO_SET_PRIV_INFO;
	tmp.in_config.slot = slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;

	//backup the orgin config
	memcpy(&tmp.in_config.satcomminfo, &sta.satcommon.satcomminfo, sizeof(struct satcommoninfo));
	tmp.in_config.satcomminfo.lon[0] = lot;
	memcpy(&tmp.in_config.satcomminfo.lon[1], lov, strlen((const char *)lov));
	tmp.in_config.satcomminfo.lat[0] = lat;
	memcpy(&tmp.in_config.satcomminfo.lat[1], lav, strlen((const char *)lav));

	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}


/*==========================follow is PTPIN PRIVATE=================================*/

/*
ip1,ip2,ip3...ip16
192.168.1.45,192.168.2.55,...;
  0	不合法
  1 合法
*/
static int str_ip_group_to_uint32(char *ipv4_grp, u32_t *des_ipv4, u8_t * des_ipv4_num)
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

//#test##############
#if 0
static int uint32_ip_group_to_str_ip(u32_t *ipv4, u8_t ipv4_num, char *des_ipv4_grp)
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
#endif
//####################
/*
  -1	失败
   0	成功
*/
int input_ptpin_board_set(struct inCtx *ctx, struct insetinfo *ds)
{
	//int board_type = BOARD_TYPE_NONE;
	//int supported_signal = BIT(SIGNAL_NONE);
	struct inputsta sta;
	struct config tmp;
	struct msgbuf msg;
	
	u8_t id_pdt = 0;
	u8_t id_pmut = 0;
	//u8_t id_signal = 0;
	//u8_t id_pmodel = 0;
	u8_t id_plt = 0;
	u8_t id_pst = 0;
	u8_t id_psf = 0;
	u8_t id_paf = 0;
	u8_t id_pdrf = 0;
	u8_t id_ppdrf = 0;
	//u8_t id_pssm = 0;
	int len = 0;

	memset(&tmp, 0, sizeof(struct config));
	//default priv
	tmp.in_config.set_which = INPUTINFO_SET_PRIV_INFO;
	
	if(ds->set_ptp_delay_type_flag)
	{
		for(id_pdt=0; id_pdt<PTP_DELAY_ARRAY_SIZE; id_pdt++)
		{
			if(0 == memcmp(ds->ptpDelayType, gPdtTbl[id_pdt], strlen((const char *)gPdtTbl[id_pdt])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_multi_uni_type_flag)
	{
		for(id_pmut=0; id_pmut<PTP_MULTI_UNI_ARRAY_SIZE; id_pmut++)
		{
			if(0 == memcmp(ds->ptpMulticast, gPmutTbl[id_pmut], strlen((const char *)gPmutTbl[id_pmut])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_layer_type_flag)
	{
		for(id_plt=0; id_plt<PTP_LAYER_ARRAY_SIZE; id_plt++)
		{
			if(0 == memcmp(ds->ptpLayer, gPltTbl[id_plt], strlen((const char *)gPltTbl[id_plt])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_step_type_flag)
	{
		for(id_pst=0; id_pst<PTP_STEP_ARRAY_SIZE; id_pst++)
		{
			if(0 == memcmp(ds->ptpStep, gPstTbl[id_pst], strlen((const char *)gPstTbl[id_pst])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_sync_freq_flag)
	{
		for(id_psf=0; id_psf<ARRAY_SIZE(gFreqTbl); id_psf++)
		{
			len = (strlen((const char *)ds->ptpSync) > strlen((const char *)gFreqTbl[id_psf])) ? strlen((const char *)ds->ptpSync) : strlen((const char *)gFreqTbl[id_psf]);
			if(0 == memcmp(ds->ptpSync, gFreqTbl[id_psf], len))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_announce_freq_flag)
	{
		for(id_paf=0; id_paf<ARRAY_SIZE(gFreqTbl); id_paf++)
		{
			len = (strlen((const char *)ds->ptpAnnounce) > strlen((const char *)gFreqTbl[id_paf])) ? strlen((const char *)ds->ptpAnnounce) : strlen((const char *)gFreqTbl[id_paf]);
			if(0 == memcmp(ds->ptpAnnounce, gFreqTbl[id_paf], len))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_delay_req_freq_flag)
	{
		for(id_pdrf=0; id_pdrf<ARRAY_SIZE(gFreqTbl); id_pdrf++)
		{
			len = (strlen((const char *)ds->ptpDelayReq) > strlen((const char *)gFreqTbl[id_pdrf])) ? strlen((const char *)ds->ptpDelayReq) : strlen((const char *)gFreqTbl[id_pdrf]);
			if(0 == memcmp(ds->ptpDelayReq, gFreqTbl[id_pdrf], len))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_pdelay_req_freq_flag)
	{
		for(id_ppdrf=0; id_ppdrf<ARRAY_SIZE(gFreqTbl); id_ppdrf++)
		{
			len = (strlen((const char *)ds->ptpPdelayReq) > strlen((const char *)gFreqTbl[id_ppdrf])) ? strlen((const char *)ds->ptpPdelayReq) : strlen((const char *)gFreqTbl[id_ppdrf]);
			if(0 == memcmp(ds->ptpPdelayReq, gFreqTbl[id_ppdrf], len))
			{
				break;
			}
		}
	}
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(ds->slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	tmp.in_config.slot = ds->slot;
	tmp.in_config.boardId = sta.boardId;
	tmp.in_config.nPort = 1;


	//only support ptp in
	switch (sta.boardId){
		case BID_PTP_IN://by supported_signal filter
			break;
		default:
			return -1;//break;//debug
	}

	memcpy(&tmp.in_config.ptpinInfo, &sta.ptpin, sizeof(struct ptpin_info));

	#if 0
	if(ds->set_delay_flag)
		tmp.out_config.ptp.ptpDelay = ds->delay;
	#endif
	if(ds->set_ptp_delay_type_flag)
		tmp.in_config.ptpinInfo.ptpDelayType = id_pdt;
		
	if(ds->set_ptp_multi_uni_type_flag)
		tmp.in_config.ptpinInfo.ptpMulticast = id_pmut;
		
	if(ds->set_ptp_layer_type_flag)
		tmp.in_config.ptpinInfo.ptpLayer = id_plt;
		
	if(ds->set_ptp_step_type_flag)
		tmp.in_config.ptpinInfo.ptpStep = id_pst;
		
	if(ds->set_ptp_sync_freq_flag)
		tmp.in_config.ptpinInfo.ptpSync = id_psf;
		
	if(ds->set_ptp_announce_freq_flag)
		tmp.in_config.ptpinInfo.ptpAnnounce = id_paf;

	if(ds->set_ptp_delay_req_freq_flag)
		tmp.in_config.ptpinInfo.ptpdelayReq = id_pdrf;

 	if(ds->set_ptp_pdelay_req_freq_flag)
		tmp.in_config.ptpinInfo.ptpPdelayReq = id_ppdrf;
	
	if(ds->set_ptp_ip_flag)
		memcpy(tmp.in_config.ptpinInfo.ptpIp, ds->ptpIp, MAX_IP_LEN);
		
	if(ds->set_ptp_mask_flag)
		memcpy(tmp.in_config.ptpinInfo.ptpMask, ds->ptpMask, MAX_MASK_LEN);
	if(ds->set_ptp_gw_flag)
		memcpy(tmp.in_config.ptpinInfo.ptpGateway, ds->ptpGateway, MAX_GATEWAY_LEN);
		
	if(ds->set_ptp_master_ip_flag){
		//printf("test_start\n");
		if(!str_ip_group_to_uint32((char *)ds->ptp_Master_Ip, tmp.in_config.ptpinInfo.ptpMasterIp, &tmp.in_config.ptpinInfo.ptpMasterIp_num))
			return -1;
		#if 0
		int i;
		for (i = 0; i < tmp.in_config.ptpinInfo.ptpMasterIp_num; ++i){
			printf("%d####:%08x\n", i+1,tmp.in_config.ptpinInfo.ptpMasterIp[i]);
		}
		char des_ipv4_grp[MAX_IP_LEN*MAX_IP_NUM+1];
		memset(des_ipv4_grp, 0x0, sizeof(des_ipv4_grp));
		if(!uint32_ip_group_to_str_ip(tmp.in_config.ptpinInfo.ptpMasterIp,tmp.in_config.ptpinInfo.ptpMasterIp_num, des_ipv4_grp))
			return -1;
		printf("test:%s\n", des_ipv4_grp);
		return 0;
		#endif
	}
	if(ds->set_ptp_master_mac_flag)
		memcpy(tmp.in_config.ptpinInfo.ptpMasterMac, ds->ptp_Master_Mac, MAX_MAC_LEN);

	if(ds->set_ptp_level_flag)
		memcpy(tmp.in_config.ptpinInfo.ptpLevel, ds->ptp_level, 4);

	if(ds->set_ptp_priority_flag)
		memcpy(tmp.in_config.ptpinInfo.ptpPriority, ds->ptp_priority, 4);
	if(ds->set_ptp_region_flag)			
		memcpy(tmp.in_config.ptpinInfo.ptpRegion, ds->ptp_region, 4);

	//printf("struct inputinfo sizeof:%d\n", sizeof(struct inputinfo));//172 to 272
	msg.mtype = MSG_CFG_IN;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct inputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

