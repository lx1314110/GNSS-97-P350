#include <stdio.h>
#include "lib_sema.h"
#include "lib_shm.h"

#include "calm_global.h"
#include "calm_query.h"

typedef  int (*filter_callback_t)(struct almCtx *, struct alarmsta *, struct masksta *, int);

filter_callback_t filter_callback;

static int filter_invaid_alm(
	struct almCtx *ctx,
	struct alarmsta *alarm,
	struct masksta *mask,
	filter_callback_t callback);

static int printf_current_alarm(
	struct almCtx *ctx,
	struct alarmsta *alarm,
	struct masksta *mask,
	 int index);

static int process_board_alm(
	struct almCtx *ctx,
	struct alarmsta *alarm,
	struct masksta *mask,
	 int index);

/*
  -1	失败
   0	成功
*/
int alarm_mask_query(struct almCtx *ctx, u32_t alm_inx)
{
	struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALMPRO_MASKSTA, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			 (char *)mask, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	printf("%d\n", mask[alm_inx].mask_sta);

	return 0;
}







/*
  -1	失败
   0	成功
*/
int alarm_select_query(struct almCtx *ctx, u8_t port)
{
	struct selectsta select[PORT_ALM];
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALMPRO_SELECTSTA, 
			 PORT_ALM*sizeof(struct selectsta), 
			 (char *)select, 
			 PORT_ALM*sizeof(struct selectsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	printf("%d\n", select[port -1].alm_inx);
	
	return 0;
}


/*
  -1	失败
   0	成功
*/
int alarm_inphase_threshold_query(struct almCtx *ctx)// int signal
{
	struct inph_thresold thresold;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_IPH_THRESHOLD, 
			 sizeof(struct inph_thresold), 
			 (char *)&thresold, 
			 sizeof(struct inph_thresold));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	printf("%d\n", thresold.Threshold);
	
	return 0;
}

//=============================================================
/*
  -1	失败
   0	成功
*/
static int filter_invaid_alm(struct almCtx *ctx,
	struct alarmsta *alarm,
	struct masksta *mask,
	filter_callback_t callback)
//int current_alarm_query(struct almCtx *ctx)
{

	int i,j;
	int cur_slot = 1, max_port = 0,board_type = BOARD_TYPE_NONE;
	int len_spt_alm = 0;
	int found = 0;
	struct alm_attr_t * spt_alm = NULL;

	if(!alarm)
		return -1;
	
	for(i=0; i<ALM_CURSOR_ARRAY_SIZE; i++)
	{
		//orgin: alm_state | mask_state | alm_id | msg
		//new: alm_state | mask_state |alm_index(orgin: alarm_id) | slot-port msg
//#define DEBUG
#ifdef DEBUG
		printf(	"%d | %d | %d | S:%d P:%d BID:%d %s\n", 
					alarm[i].alm_sta,
					mask[i].mask_sta,
					i,
					pAlmInxTbl[i].slot,
					pAlmInxTbl[i].port,
					ctx->bid[pAlmInxTbl[i].slot-1],
					pAlmInxTbl[i].alm_id_tbl->alm_msg);
#else
		cur_slot = pAlmInxTbl[i].slot;// ALM slot max  SLOT_CURSOR_ARRAY_SIZE 16
		if (pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_SYS && \
			pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_PWR){
			//只限制槽位有关的告警,过滤掉空槽位的告警
			if (ctx->bid[cur_slot-1] == BID_NONE ){
				//no show no_bid alarm, sys alm no need check
				continue;
			}
		}

		FIND_MAXPORT_GBATBL_BY_BID(ctx->bid[cur_slot-1], max_port);
		FIND_BDTYPE_GBATBL_BY_BID(ctx->bid[cur_slot-1], board_type);
		FIND_SPT_ALM_GBATBL_BY_BID(ctx->bid[cur_slot-1], spt_alm);
		FIND_SPT_LEN_ALM_GBATBL_BY_BID(ctx->bid[cur_slot-1], len_spt_alm);
		//for SMP_SID_SUB_IN SMP_SID_SUB_OUT SMP_SID_SUB_SAT
		if (pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_SYS && \
			pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_PWR //&&
			//pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_SAT
			){
			//只限制跟端口有关的告警
			if (pAlmInxTbl[i].port > max_port ){
				//no show invaid port
				continue;
			}

			if(spt_alm){
				//输入输出告警类型过滤掉无用的端口报警，仅仅显示自己端口支持的告警
				found = 0;
				for (j = 0; j < len_spt_alm; ++j){
					if(pAlmInxTbl[i].alm_id == spt_alm[j].alm_id){
						found = 1;
						break;
					} 
				}
				if(found == 1){
					//printf("####line-%d slot:%d port:%d support_almbit:%d\n",__LINE__,
					//pAlmInxTbl[i].slot, pAlmInxTbl[i].port, spt_alm[j].supported_port);
					if (!(BIT(pAlmInxTbl[i].port) & spt_alm[j].supported_port)){
						//过滤掉本端口port不支持的告警
						continue;
					}
				}else{
					//过滤掉本板卡slot不支持的告警
					continue;
				}
			} else {
				//过滤掉不支持告警的盘的告警
				continue;
			}
			
		}

		//过滤掉输入盘的输出类型告警
		if (pAlmInxTbl[i].alm_id_tbl->alm_type == SMP_SID_SUB_IN){
			if (board_type != BOARD_TYPE_INPUT){
					continue;//must is input board
				}
		}
		//过滤掉输出盘的输入类型告警
		if (pAlmInxTbl[i].alm_id_tbl->alm_type == SMP_SID_SUB_OUT){
			if (board_type != BOARD_TYPE_OUTPUT){
					continue;//must is output board
				}
		}

		if(callback){
			if(0 != callback(ctx, alarm, mask, i))
				return -1;
		}
#endif
	}
	
	return 0;
}


static int process_board_alm(
	struct almCtx *ctx,
	struct alarmsta *alarm,
	struct masksta *mask,
	 int index)
{
	int i = index;
	if(!alarm)
		return -1;
	
	switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
		case SMP_SID_SUB_IN:
		case SMP_SID_SUB_OUT:
			//pAlmInxTbl[i].slot// slot_max 1-16
			if(alarm[i].alm_sta == ALM_ON)
				ctx->bid_alm_num[pAlmInxTbl[i].slot-1]++;
		break;
		case SMP_SID_SUB_SYS:
		break;
		case SMP_SID_SUB_PWR:
			//pAlmInxTbl[i].slot// slot_max 1-2
			if(alarm[i].alm_sta == ALM_ON)
				ctx->bid_alm_num[pAlmInxTbl[i].slot+(OTHER_SLOT_CURSOR_17+1)-1]++;
		break;
		case SMP_SID_SUB_SAT:
			if(alarm[i].alm_sta == ALM_ON)
				ctx->bid_alm_num[pAlmInxTbl[i].slot-1]++;
		break;
	}
	return 0;

}

int bd_alarm_sta_query(struct almCtx *ctx)
{
	int i;
	struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALM, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			 (char *)alarm, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}


	if (0 != filter_invaid_alm(ctx, alarm, NULL, &process_board_alm))
		return -1;

	for (i = SLOT_CURSOR_1; i < SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1; ++i){
		//FIND_MAXPORT_GBATBL_BY_BID(bid[i], bd_maxport[i]);
		//slot:bid:bid_name:alm_num
		printf("%d:%d:%s:%d\n", i+1, ctx->bid[i], gBoardNameTbl[ctx->bid[i]], ctx->bid_alm_num[i]);
	}
	return 0;

}

static int printf_current_alarm(
	struct almCtx *ctx,
	struct alarmsta *alarm,
	struct masksta *mask,
	 int index)
{
	int i = index;
	if(!mask || !alarm)
		return -1;
	//alm_sta| mask|alm_index| alm_id|slot|port|bid |s:p:msg
	switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
		case SMP_SID_SUB_IN:
		case SMP_SID_SUB_OUT:
			printf( "%d | %d | %d | 0x%08x | %d | %d | %d | S:%d P:%d %s\n", 
				alarm[i].alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot-1],

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				pAlmInxTbl[i].alm_id_tbl->alm_msg);
		break;
		case SMP_SID_SUB_SYS:
			printf( "%d | %d | %d | 0x%08x | %d | %d | %d | SYS: %s\n", 
				alarm[i].alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				0,
				0,
				0,

				pAlmInxTbl[i].alm_id_tbl->alm_msg);
		break;
		case SMP_SID_SUB_PWR:
		printf( "%d | %d | %d | 0x%08x | %d | %d | %d | #%d %s\n", 
				alarm[i].alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1-1],//17/18

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].alm_id_tbl->alm_msg);
		break;
		case SMP_SID_SUB_SAT:
		printf( "%d | %d | %d | 0x%08x | %d | %d | %d | #%d %s\n", 
				alarm[i].alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot-1],

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].alm_id_tbl->alm_msg);
		break;
	}
	return 0;

}

int current_alarm_list_query(struct almCtx *ctx)
{
	struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_ALMPRO_MASKSTA, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			 (char *)mask, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALM, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			 (char *)alarm, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	shm_read(ctx->ipc.ipc_base, 
			  SHM_OFFSET_CURALM_INFO, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)ctx->cur_alarm, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));//暂未用
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	if (0 != filter_invaid_alm(ctx, alarm, mask, &printf_current_alarm))
		return -1;


	#if 0
	int i;
	struct cur_alminfo *cur_alm = ctx->cur_alarm;
	struct timeinfo *ti = NULL;
	printf("============================\n");
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		ti = &cur_alm[i].ti;
		printf("%d | %d | %d | "
		"%04d-%02d-%02d %02d:%02d:%02d\n",
			cur_alm[i].id, 
			cur_alm[i].alm_sta, 
			cur_alm[i].alm_id,
			ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	}
	#endif
	return 0;

}
//======================new interface==================================
/*====================告警查询处理============*/

static int printf_current_alarm_and_time(
	struct almCtx *ctx,
	struct masksta *mask,
	struct alminfo_t *pcur_alm)
{
	u32_t id = pcur_alm->id;
	u16_t i = pcur_alm->alm_inx;
	struct timeinfo *ti = &pcur_alm->ti;
	char time[20];
	
	if(!mask || !pcur_alm || i >= ALM_CURSOR_ARRAY_SIZE )
		return -1;

	if(id == 0)
		return 0;

	memset(time,0x0, sizeof(time));
	sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d", ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	//alm_sta| mask|alm_index| alm_id|slot|port|bid |s:p:msg | time | id
	switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
		case SMP_SID_SUB_IN:
		case SMP_SID_SUB_OUT:
			printf( "%d | %d | %d | 0x%08x | %d | %d | %d | S:%d P:%d %s | %s | %d\n", 
				pcur_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot-1],

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time,
				id);
		break;
		case SMP_SID_SUB_SYS:
			printf( "%d | %d | %d | 0x%08x | %d | %d | %d | SYS: %s | %s | %d\n", 
				pcur_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				0,
				0,
				0,

				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time,
				id);
		break;
		case SMP_SID_SUB_PWR:
		printf( "%d | %d | %d | 0x%08x | %d | %d | %d | #%d %s | %s | %d\n", 
				pcur_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1-1],//17/18

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time,
				id);
		break;
		case SMP_SID_SUB_SAT:
		printf( "%d | %d | %d | 0x%08x | %d | %d | %d | #%d %s | %s | %d\n", 
				pcur_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot-1],

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time,
				id);
		break;
	}
	return 0;

}

int current_alarm_query(struct almCtx *ctx)
{
	int i,j;
	struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	//struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	struct alminfo_t *cur_alm = ctx->cur_alarm;
	struct alminfo_t *sort_alm[CUR_ALM_MAX_ARRAY_SIZE];
	struct alminfo_t **minid_alm = NULL;
	struct alminfo_t *tmp_alm = NULL;

	//printf("sizeof: *sort_alm[]=%d\n sort_alm[]=%d\n **minid_alm=%d\n *tmp_alm=%d\n",
		//sizeof(sort_alm), sizeof(sort_alm[CUR_ALM_MAX_ARRAY_SIZE]), sizeof(minid_alm), sizeof(tmp_alm));//4*50 4 4 4
	//printf("sizeof:cur_alminfo:%d\n timeinfo:%d\n", sizeof(struct cur_alminfo),sizeof(struct timeinfo));//24 14
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_ALMPRO_MASKSTA, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			 (char *)mask, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	/*shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALM, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			 (char *)alarm, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));*/
	shm_read(ctx->ipc.ipc_base, 
			  SHM_OFFSET_CURALM_INFO, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)ctx->cur_alarm, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	#if 0
	struct timeinfo *ti = NULL;
	printf("============================\n");
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		ti = &cur_alm[i].ti;
		printf("%d | %d | %d | "
		"%04d-%02d-%02d %02d:%02d:%02d\n",
			cur_alm[i].id, 
			cur_alm[i].alm_sta, 
			cur_alm[i].alm_id,
			ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	}
	#endif
	//按id从小到大排序
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		//memset(sort_alm[i], 0x0, sizeof(struct cur_alminfo));
		sort_alm[i] = &cur_alm[i];
	}
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE - 1; ++i){
		minid_alm = &sort_alm[i];
		for (j = i+1; j < CUR_ALM_MAX_ARRAY_SIZE; ++j){
			if(sort_alm[j]->id == 0 || sort_alm[j]->alm_sta == ALM_OFF)
				continue;
			if((*minid_alm)->id > sort_alm[j]->id)
				minid_alm = &sort_alm[j];
		}
		tmp_alm = sort_alm[i];
		sort_alm[i] = *minid_alm;
		*minid_alm = tmp_alm;
	}

	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		//printf_current_alarm_and_time(ctx, alarm, mask, &cur_alm[i]);//未排序
		printf_current_alarm_and_time(ctx, mask, sort_alm[i]);//排序
	}
	

	return 0;
}

int current_alarm_nums(struct almCtx *ctx)
{
	int i;
	int cur_alm_num = 0;
	//struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	//struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	struct alminfo_t *cur_alm = ctx->cur_alarm;

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	/*shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_ALMPRO_MASKSTA, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			 (char *)mask, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALM, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			 (char *)alarm, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));*/
	shm_read(ctx->ipc.ipc_base, 
			  SHM_OFFSET_CURALM_INFO, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)ctx->cur_alarm, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	#if 0
	struct timeinfo *ti = NULL;
	printf("============================\n");
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		ti = &cur_alm[i].ti;
		printf("%d | %d | %d | "
		"%04d-%02d-%02d %02d:%02d:%02d\n",
			cur_alm[i].id, 
			cur_alm[i].alm_sta, 
			cur_alm[i].alm_id,
			ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	}
	#endif

	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		if(cur_alm[i].id != 0 && cur_alm[i].alm_sta == ALM_ON)
			cur_alm_num++;
	}
	

	return cur_alm_num;
}



/*====================告警上报处理============*/
static int printf_upload_alarm_and_time(
	struct almCtx *ctx,
	struct masksta *mask,
	struct alminfo_t *pupl_alm)
{
	u32_t id = pupl_alm->id;
	u16_t i = pupl_alm->alm_inx;
	struct timeinfo *ti = &pupl_alm->ti;
	char time[20];
	
	if(!mask || !pupl_alm || i >= ALM_CURSOR_ARRAY_SIZE )
		return -1;

	if(id == 0)
		return 0;

	memset(time,0x0, sizeof(time));
	sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d", ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	//id | alm_sta| mask|alm_index| alm_id|slot|port|bid |s:p:msg | time
	switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
		case SMP_SID_SUB_IN:
		case SMP_SID_SUB_OUT:
			printf( "%d | %d | %d | %d | 0x%08x | %d | %d | %d | S:%d P:%d %s | %s\n",
				id,
				pupl_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot-1],

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time);
		break;
		case SMP_SID_SUB_SYS:
			printf( "%d | %d | %d | %d | 0x%08x | %d | %d | %d | SYS: %s | %s\n",
				id,
				pupl_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				0,
				0,
				0,

				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time);
		break;
		case SMP_SID_SUB_PWR:
		printf( "%d | %d | %d | %d | 0x%08x | %d | %d | %d | #%d %s | %s\n",
				id,
				pupl_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1-1],//17/18

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time);
		break;
		case SMP_SID_SUB_SAT:
		printf( "%d | %d | %d | %d | 0x%08x | %d | %d | %d | #%d %s | %s\n",
				id,
				pupl_alm->alm_sta,
				mask[i].mask_sta,
				i,
				pAlmInxTbl[i].alm_id,
				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].port,
				ctx->bid[pAlmInxTbl[i].slot-1],

				pAlmInxTbl[i].slot,
				pAlmInxTbl[i].alm_id_tbl->alm_msg,
				time);
		break;
	}
	return 0;

}


int upload_alarm_query(struct almCtx *ctx)
{
	int i,j;
	struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	//struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	struct alminfo_t *upload_alm = ctx->upload_alarm;
	struct alminfo_t *sort_alm[UPLOAD_ALM_MAX_ARRAY_SIZE];
	struct alminfo_t **minid_alm = NULL;
	struct alminfo_t *tmp_alm = NULL;

	//printf("sizeof: *sort_alm[]=%d\n sort_alm[]=%d\n **minid_alm=%d\n *tmp_alm=%d\n",
		//sizeof(sort_alm), sizeof(sort_alm[UPLOAD_ALM_MAX_ARRAY_SIZE]), sizeof(minid_alm), sizeof(tmp_alm));//4*50 4 4 4
	//printf("sizeof:upload_alminfo:%d\n timeinfo:%d\n", sizeof(struct upload_alminfo),sizeof(struct timeinfo));//24 14
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_ALMPRO_MASKSTA, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			 (char *)mask, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	/*shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALM, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			 (char *)alarm, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));*/
	shm_read(ctx->ipc.ipc_base, 
			  SHM_OFFSET_UPLOADALM_INFO, 
			  UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)ctx->upload_alarm, 
			  UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	#if 0
	struct timeinfo *ti = NULL;
	printf("============================\n");
	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE; ++i){
		ti = &upload_alm[i].ti;
		printf("%d | %d | %d | %d | "
		"%04d-%02d-%02d %02d:%02d:%02d\n",
			i,
			upload_alm[i].id, 
			upload_alm[i].alm_sta, 
			upload_alm[i].alm_inx,
			ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	}
	printf("============================\n");
	#endif
	//按id从小到大排序
	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE; ++i){
		//memset(sort_alm[i], 0x0, sizeof(struct upload_alminfo));
		sort_alm[i] = &upload_alm[i];
	}
	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE - 1; ++i){
		minid_alm = &sort_alm[i];
		for (j = i+1; j < UPLOAD_ALM_MAX_ARRAY_SIZE; ++j){
			if(sort_alm[j]->id == 0)
				continue;
			if((*minid_alm)->id > sort_alm[j]->id)
				minid_alm = &sort_alm[j];
		}
		tmp_alm = sort_alm[i];
		sort_alm[i] = *minid_alm;
		*minid_alm = tmp_alm;
	}

	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE; ++i){
		//printf_uploadrent_alarm_and_time(ctx, alarm, mask, &upload_alm[i]);//未排序
		printf_upload_alarm_and_time(ctx, mask, sort_alm[i]);//排序
	}
	

	return 0;
}



int upload_alarm_nums(struct almCtx *ctx)
{
	int i;
	int upload_alm_num = 0;
	//struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	//struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	struct alminfo_t *upload_alm = ctx->upload_alarm;

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	/*shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_ALMPRO_MASKSTA, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			 (char *)mask, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ALM, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			 (char *)alarm, 
			 ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));*/
	shm_read(ctx->ipc.ipc_base, 
			  SHM_OFFSET_UPLOADALM_INFO, 
			  UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)ctx->upload_alarm, 
			  UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	#if 0
	struct timeinfo *ti = NULL;
	printf("============================\n");
	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE; ++i){
		ti = &upload_alm[i].ti;
		printf("%d | %d | %d | "
		"%04d-%02d-%02d %02d:%02d:%02d\n",
			upload_alm[i].id, 
			upload_alm[i].alm_sta, 
			upload_alm[i].alm_inx,
			ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	}
	#endif

	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE; ++i){
		if(upload_alm[i].id != 0)
			upload_alm_num++;
	}
	

	return upload_alm_num;
}


