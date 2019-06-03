#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "calm_macro.h"
#include "calm_history.h"






/*
  >0	成功
  -1	失败
*/
int history_alarm_pages(sqlite3 *pDb, char *pTbl)
{
	char sql[64];
	char **resultp;
	int nrow;
	int ncolumn;
	int ret;
	int page = 0;

	if((NULL == pDb) || (NULL == pTbl))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录数量
		return -1;
	}
	else
	{
		page = atoi(resultp[1]);
		if(page%RECORD_PER_PAGE == 0)
			ret = page/RECORD_PER_PAGE;
		else
			ret = page/RECORD_PER_PAGE +1;
		db_free_table(resultp);
		db_unlock(pDb);
		
		return ret;
	}
}



/*
  >0	成功
  -1	失败
*/
int history_alarm_nums(sqlite3 *pDb, char *pTbl)
{
	char sql[64];
	char **resultp;
	int nrow;
	int ncolumn;
	int ret;

	if((NULL == pDb) || (NULL == pTbl))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录数量
		return -1;
	}
	else
	{
		ret = atoi(resultp[1]);
		db_free_table(resultp);
		db_unlock(pDb);
		
		return ret;
	}
}

/*
   0	成功
  -1	失败
*/
int history_alarm_query(struct almCtx *ctx, char * pTbl, int page)
{
	char sql[256];
	char **resultp;
	int nrow;
	int ncolumn;
	int i;
	int alm_index;
	sqlite3 * pDb = ctx->pDb;

	if((NULL == pDb) || (NULL == pTbl) || (page < 1))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select alarm_id, alarm_start_time, alarm_end_time \
				  from %s order by alarm_end_time desc limit %d offset %d;", 
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE);
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录
		return -1;
	}
	else
	{
		for(i=1; i<(nrow+1); i++)
		{
			//orgin: id | msg | start_time | end_time
			//new: alm_index(orgin: alarm_id) |alm_id| slot(snmp) | port(snmp) |bid(snmp) | slot-port msg | start_time | end_time
			alm_index = atoi(resultp[ncolumn *i +0]);
			if (alm_index < 0 || alm_index >= max_alminx_num){
				db_free_table(resultp);
				db_unlock(pDb);
				return -1;
			}

			switch (pAlmInxTbl[alm_index].alm_id_tbl->alm_type){
				case SMP_SID_SUB_IN:
				case SMP_SID_SUB_OUT:
					printf( "%s | 0x%08x | %d | %d | %d | S:%d P:%d %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						pAlmInxTbl[alm_index].alm_id,
						pAlmInxTbl[alm_index].slot,
						pAlmInxTbl[alm_index].port,
						ctx->bid[pAlmInxTbl[i].slot-1],

						pAlmInxTbl[alm_index].slot,
						pAlmInxTbl[alm_index].port,
						pAlmInxTbl[alm_index].alm_id_tbl->alm_msg,

						resultp[ncolumn *i +1],
						resultp[ncolumn *i +2]);
				break;
				case SMP_SID_SUB_SYS:
					printf( "%s | 0x%08x | %d | %d | %d | SYS: %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						pAlmInxTbl[alm_index].alm_id,
						0,
						0,
						0,	

						pAlmInxTbl[alm_index].alm_id_tbl->alm_msg,

						resultp[ncolumn *i +1],
						resultp[ncolumn *i +2]);
				break;
				case SMP_SID_SUB_PWR:
				printf( "%s | 0x%08x | %d | %d | %d | #%d %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						pAlmInxTbl[alm_index].alm_id,
						pAlmInxTbl[alm_index].slot+OTHER_SLOT_CURSOR_17+1,
						pAlmInxTbl[alm_index].port,
						ctx->bid[pAlmInxTbl[i].slot+OTHER_SLOT_CURSOR_17+1-1],//17/18

						pAlmInxTbl[alm_index].slot,
						pAlmInxTbl[alm_index].alm_id_tbl->alm_msg,

						resultp[ncolumn *i +1],
						resultp[ncolumn *i +2]);
				break;
				case SMP_SID_SUB_SAT:
				printf( "%s | 0x%08x | %d | %d | %d | #%d %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						pAlmInxTbl[alm_index].alm_id,
						pAlmInxTbl[alm_index].slot,
						pAlmInxTbl[alm_index].port,
						ctx->bid[pAlmInxTbl[i].slot-1],

						pAlmInxTbl[alm_index].slot,
						pAlmInxTbl[alm_index].alm_id_tbl->alm_msg,

						resultp[ncolumn *i +1],
						resultp[ncolumn *i +2]);
				break;
			}

		}
		db_free_table(resultp);
		db_unlock(pDb);
		return 0;
	}
}

#if 0
//请使用#if 0保留给函数，不要删除，也许以后用到
/*
   0	成功
  -1	失败
*/
int history_alarm_query(sqlite3 * pDb, char * pTbl, int page)
{
	char sql[256];
	char **resultp;
	int nrow;
	int ncolumn;
	int i;

	if((NULL == pDb) || (NULL == pTbl))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select a.alarm_id, b.alarm_description, a.alarm_start_time, a.alarm_end_time \
				  from %s a, %s b \
				  where a.alarm_id = b.alarm_id \
				  order by a.alarm_end_time desc limit %d offset %d;", 
				  pTbl, TBL_ALARM_LIST, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE);
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录
		return -1;
	}
	else
	{
		for(i=1; i<(nrow+1); i++)
		{
			printf( "%s | %s | %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					resultp[ncolumn *i +2],
					resultp[ncolumn *i +3] );
		}
		db_free_table(resultp);
		db_unlock(pDb);
		return 0;
	}
}
#endif



