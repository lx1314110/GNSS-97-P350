#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_sqlite.h"

#include "dalm_config.h"

/*
  1	成功
  0	失败
*/
int ReadBidFromFpga(struct almCtx * ctx)
{
	u16_t temp[10];
	int i,j;

	if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S17, &temp[8]))
	{
		return 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S18_S19, &temp[9]))
	{
		return 0;
	}

	for(i=SLOT_CURSOR_1,j=0; (i<SLOT_CURSOR_ARRAY_SIZE)&&(j<8); i+=2,j++)
	{
		if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S01_TO_S16(j), &temp[j]))
		{
			return 0;
		}

		ctx->bid[i] = (temp[j]>>8)&0x00FF;
		//if(0x00FF == ctx->bid[i])
		if(BID_ARRAY_SIZE <= ctx->bid[i])
		{
			ctx->bid[i] = 0x00;
		}
		ctx->bid[i+1] = temp[j]&0x00FF;
		//if(0x00FF == ctx->bid[i+1])
		if(BID_ARRAY_SIZE <= ctx->bid[i+1])
		{
			ctx->bid[i+1] = 0x00;
		}
	}

	//slot 17
	ctx->bid[OTHER_SLOT_CURSOR_17] = temp[8] &0x00FF;
	//if(0x00FF == bid)
	if(ctx->bid[OTHER_SLOT_CURSOR_17] >= BID_ARRAY_SIZE)
	{
		ctx->bid[OTHER_SLOT_CURSOR_17] = BID_NONE;
	}

	//slot 18
	ctx->bid[OTHER_SLOT_CURSOR_18] = (temp[9]>>8)&0x00FF;
	//if(0x00FF == bid)
	if(ctx->bid[OTHER_SLOT_CURSOR_18] >= BID_ARRAY_SIZE)
	{
		ctx->bid[OTHER_SLOT_CURSOR_18] = BID_NONE;
	}

	//slot 19
	ctx->bid[OTHER_SLOT_CURSOR_19] = temp[9]&0x00FF;
	//if(0x00FF == bid)
	if(ctx->bid[OTHER_SLOT_CURSOR_19] >= BID_ARRAY_SIZE)
	{
		ctx->bid[OTHER_SLOT_CURSOR_19] = BID_NONE;
	}

	//slot 20
	//del the function
	ctx->bid[OTHER_SLOT_CURSOR_20] = BID_NONE;

	//check len
	if(OTHER_SLOT_CURSOR_20+1 != SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1){
			print(DBG_ERROR, "len error.\n");
			return 0;
	}

	return 1;
}

/*所有告警表*/
int WriteAlarmToShareMemory(struct almCtx *ctx)
{

	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			  (char *)(ctx->new_alarm), 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}
	return 1;

}

/*所有告警和当前告警列表*/
int WriteAlarmInfoToShareMemory(struct almCtx *ctx)
{

	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			  (char *)(ctx->new_alarm), 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	//为当前告警查询
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_CURALM_INFO, 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)(ctx->cur_alarm), 
			  CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	//为了告警主动上报，类似事件
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_UPLOADALM_INFO, 
			  UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t), 
			  (char *)(ctx->upload_alarm), 
			  UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}

	return 1;

}


int WriteEnableToShareMemory(struct almCtx *ctx)
{
	
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");

		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_PORT, 
			  V_ETH_MAX*sizeof(struct portinfo), 
			  (char *)&(ctx->port_status), 
			  V_ETH_MAX*sizeof(struct portinfo));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");

		return 0;
	}

	return 1;
	
}




/*
  1	成功
  0	失败
*/
int WriteMaskToShareMemory(struct almCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_ALMPRO_MASKSTA, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta), 
			  (char *)(ctx->mask), 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}

	return 1;
}




/*
  1	成功
  0	失败
*/
int WriteSelectToShareMemory(struct almCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_ALMPRO_SELECTSTA, 
			  PORT_ALM*sizeof(struct selectsta), 
			  (char *)(ctx->select), 
			  PORT_ALM*sizeof(struct selectsta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}

	return 1;
}

/*
  1	成功
  0	失败
*/
int WriteSignalThresholdToShareMemory(struct almCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_IPH_THRESHOLD, 
			  sizeof(struct inph_thresold), 
			  (char *)(&ctx->InSgnl_Threshold), 
			  sizeof(struct inph_thresold));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}

	return 1;
}





/*
  1	成功
  0	失败
*/
int WriteAlarmSelectToTable(struct almCtx *ctx, struct selectinfo *cfg, char *pTbl)
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set alarm_id=%d \
				  where o_port=%d;", 
				  pTbl, 
				  cfg->alm_inx,
				  cfg->alm_board_port);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write alarm select.");
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
  1	成功 //tbl_sys
  0	失败
*/
int WriteSignalThresholdToTable(struct almCtx *ctx, struct inph_thresold *cfg, char *pTbl)
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set i_ph_threshold=%d;", 
				  pTbl, cfg->Threshold);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write SignalThreshold.");
		return 0;
	}
	else
	{
		return 1;
	}
}




static int InsertAlarmMaskToTable(struct almCtx *ctx, char *pTbl)
{
	int rc = 0, i = 0;
	char sql[128];
	bool_t ret = true;
	sqlite3_stmt *stmt = NULL;

	memset(sql, 0, sizeof(sql));
	//(pd_id, pd_value)
	sprintf( sql,"insert into %s values(?, ?);", 
			 pTbl);
	db_lock(ctx->pDb);
	rc = sqlite3_prepare(ctx->pDb, sql, strlen(sql), &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		print(	DBG_ERROR, 
				"--%s.", 
				sqlite3_errmsg(ctx->pDb));
		db_unlock(ctx->pDb);
		return false;
	}

	for (i = 0; i < ALM_CURSOR_ARRAY_SIZE; ++i){
		sqlite3_bind_int(stmt, 1, i);//alm_index
		sqlite3_bind_int(stmt, 2, ctx->mask[i].mask_sta);//alm_mask
		//sqlite3_bind_text(stmt,2,gAlarmTbl[i].alarm_msg,strlen(gAlarmTbl[i].alarm_msg),NULL);
		rc = sqlite3_step(stmt);
		if( rc!=SQLITE_DONE){
			print(	DBG_ERROR, 
				"--%s.", 
				sqlite3_errmsg(ctx->pDb));
			ret = false;
			break;
		}
		//printf("step_ret:%d\n",rc);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	db_unlock(ctx->pDb);
	return ret ;
}

/*
  1	成功
  0	失败
*/
int FlushAlarmMaskTable(struct almCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp = NULL;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read alarm mask.");
		return false;
	}
	nrow = atoi(resultp[1]);

	db_free_table(resultp);
	db_unlock(ctx->pDb);

	if(nrow != ALM_CURSOR_ARRAY_SIZE){
		if(false == empty_table(ctx->pDb, pTbl)) {	
			print(	DBG_ERROR, 
					"--Failed to empty alarm shiled table.");
			return false;
		}

		if(false == InsertAlarmMaskToTable(ctx, pTbl)) {			
			print(	DBG_ERROR, 
					"--Failed to InsertAlarmMaskToTable.");
			return false;
		}
		
	}
	
	return true;
}


/*
  1	成功
  0	失败
*/
int WriteAlarmMaskToTable(struct almCtx *ctx, struct maskinfo *cfg, char *pTbl)
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set alarm_shield_state=\'%d\' \
				  where alarm_id=%d;", 
				  pTbl, 
				  cfg->mask_sta,
				  cfg->alm_inx);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write alarm mask.");
		return 0;
	}
	else
	{
		return 1;
	}
}








/*
  1	成功
  0	失败
*/
int ReadAlarmSelectFromTable(struct almCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int i;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select * from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read alarm select.");
		return 0;
	}
	
	for(i=1; i<(nrow+1); i++)
	{
		ctx->select[i-1].alm_inx = atoi(resultp[ncolumn * i +1]);
	}
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}

/*
  @ctx		output board context
  @pTbl		system table

  1	成功
  0	失败

  read input signal threshold from system table
*/
int ReadSignalThresholdFromTable(struct almCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select i_ph_threshold from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read SignalThreshold.");
		return 0;
	}
	ctx->InSgnl_Threshold.Threshold = atoi(resultp[ncolumn*nrow+0]);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}






/*
  1	成功
  0	失败
*/
int ReadAlarmMaskFromTable(struct almCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int i;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select * from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read alarm mask.");
		return 0;
	}
	
	for(i=1; i<(nrow+1); i++)
	{
		ctx->mask[i-1].mask_sta = atoi(resultp[ncolumn * i +1]);
	}
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}


#ifdef NEW_ALARMID_COMM
//no need the TBL_ALARM_LIST
#else
static int InsertAlarmListToTable(struct almCtx *ctx, char *pTbl)
{
	int rc = 0, i = 0;
	char sql[128];
	bool_t ret = true;
	sqlite3_stmt *stmt = NULL;

	memset(sql, 0, sizeof(sql));
	//(pd_id, pd_value)
	sprintf( sql,"insert into %s values(?, ?);", 
			 pTbl);
	db_lock(ctx->pDb);
	rc = sqlite3_prepare(ctx->pDb, sql, strlen(sql), &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		print(	DBG_ERROR, 
				"--%s.", 
				sqlite3_errmsg(ctx->pDb));
		db_unlock(ctx->pDb);
		return false;
	}

	for (i = 0; i < ALM_CURSOR_ARRAY_SIZE; ++i){
		sqlite3_bind_int(stmt, 1, gAlarmTbl[i].alarm_id);
		sqlite3_bind_text(stmt,2,gAlarmTbl[i].alarm_msg,strlen(gAlarmTbl[i].alarm_msg),NULL);
		rc = sqlite3_step(stmt);
		if( rc!=SQLITE_DONE){
			print(	DBG_ERROR, 
				"--%s.", 
				sqlite3_errmsg(ctx->pDb));
			ret = false;
			break;
		}
		//printf("step_ret:%d\n",rc);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	db_unlock(ctx->pDb);
	return ret ;
}

/*
  1	成功
  0	失败
*/
int FlushAlarmListTable(struct almCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp = NULL;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read alarm mask.");
		return false;
	}
	nrow = atoi(resultp[1]);

	db_free_table(resultp);
	db_unlock(ctx->pDb);

	if(nrow != ALM_CURSOR_ARRAY_SIZE){
		if(false == empty_table(ctx->pDb, pTbl)) {	
			print(	DBG_ERROR, 
					"--Failed to empty alarm list table.");
			return false;
		}

		if(false == InsertAlarmListToTable(ctx, pTbl)) {
			db_unlock(ctx->pDb);
			
			print(	DBG_ERROR, 
					"--Failed to InsertAlarmListToTable.");
			return false;
		}
		
	}
	
	return true;
}
#endif







