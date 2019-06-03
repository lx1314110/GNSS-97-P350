#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "lib_log.h"
#include "lib_sqlite.h"

#include "alloc.h"
#include "csys_event.h"
#include "csys_common.h"



#undef DEBUG
//#define DEBUG

#define	HISTORY				"history"
#define	CURRENT				"current"

#define	HISTORYNUM		 	"hstynum"
#define	CURRENTNUM		 	"curnum"

#define	RECORD_PER_PAGE		TBL_EVENT_CURRENT_ARRAY_SIZE

void event_helper(void);
int event_page_validity(char *, int);
int event_pages(sqlite3 *pDb, char *pTbl);
int event_nums(sqlite3 *pDb, char *pTbl);
int current_event_query(sqlite3 * pDb, char * pTbl, int page);
int history_event_query(sqlite3 * pDb, char * pTbl, int page);

//========================================================================

/*
  1	合法
  0	非法
*/
int event_page_validity(char *page, int record_num)
{
	int len;
	int ret = 0;

	len = strlen(page);
#if 0
	if(1 == len)
	{
		if((page[0] >= '1') && (page[0] <= '9'))
		{
			ret = 1;
		}
	}
	else if(2 == len)
	{
		if( ((page[0] >= '1') && (page[0] <= '9')) &&
			((page[1] >= '0') && (page[1] <= '9')) &&
			((atoi(page) >= 1) && (atoi(page) <= MAX_ALM/RECORD_PER_PAGE)) )
		{
			ret = 1;
		}
	}
	else
	{
		//do nothing
	}
#else
	int i = 0;
	ret = 1;
	for (i = 0; i < len; i++){
		if(isdigit(page[i]) == 0)
			ret = 0;
	}
	int max_page = 0;
	if(record_num % RECORD_PER_PAGE == 0)
		max_page = record_num/RECORD_PER_PAGE;
	else
		max_page = record_num/RECORD_PER_PAGE +1;
	if( atoi(page) < 1 || atoi(page) > max_page){
		ret = 0;
	}
#endif
	return ret;
}


/*
  >0	成功
  -1	失败
*/
int event_pages(sqlite3 *pDb, char *pTbl)
{
	char sql[64];
	char **resultp;
	int nrow;
	int ncolumn;
	int ret = 0;
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
int event_nums(sqlite3 *pDb, char *pTbl)
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
int current_event_query(sqlite3 * pDb, char * pTbl, int page)
{
	char sql[512];
	char **resultp;
	int nrow;
	int ncolumn;
	int i;
	int evnt_index;

	if(NULL == pDb || NULL == pTbl || page < 1)
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
#if 1
	//倒序排列
	sprintf(sql, "select ID, event_id, event_tid, slot, port, bid, event_start_time, is_read \
				  from %s order by rowid desc limit %d offset %d; \
				  update %s set is_read=1 where is_read=0;",
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE,
				  pTbl);
#else
	sprintf(sql, "select ID, event_id, event_tid, slot, port, bid, event_start_time \
				  from %s where is_read=0 order by rowid desc limit %d offset %d; \
				  update %s set is_read=1 where is_read=0;", 
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE,
				  pTbl);
#endif
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录
		return -1;
	}

	for(i=1; i<(nrow+1); i++)
	{

		//new: ID(KEY) | event_index| event_tid | slot(snmp) | port(snmp)  | bid | slot-port msg | start_time |is read
		evnt_index = atoi(resultp[ncolumn *i +1]);
		if (evnt_index < 0 || evnt_index >= max_evntinx_num){
			db_free_table(resultp);
			db_unlock(pDb);
			return -1;
		}
		#ifdef DEBUG
		#if 0
		printf( "%s | %s | 0x%08x | %d | %d | %d | %s | %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					resultp[ncolumn *i +2],//event_tid
					pEvntInxTbl[evnt_index].slot,
					pEvntInxTbl[evnt_index].port,
					pEvntInxTbl[evnt_index].bid,
					pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,
					resultp[ncolumn *i +6],
					resultp[ncolumn *i +7]);
		#else
		printf( "%s | %s | 0x%08x | %d | %d | %d | %s | %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					atoi(resultp[ncolumn *i +2]),//event_tid
					atoi(resultp[ncolumn *i +3]),//slot
					atoi(resultp[ncolumn *i +4]),//port
					atoi(resultp[ncolumn *i +5]),//bid
					pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,
					resultp[ncolumn *i +6],
					resultp[ncolumn *i +7]);

		#endif
		#else
		//is_read; 0-not read  1-read
		switch (pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_type){
			case SMP_SID_SUB_IN:
			case SMP_SID_SUB_OUT:
				printf( "%s | %s | 0x%08x | %d | %d | %d | S:%d P:%d %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					atoi(resultp[ncolumn *i +2]),//event_tid
					atoi(resultp[ncolumn *i +3]),//slot
					atoi(resultp[ncolumn *i +4]),//port
					atoi(resultp[ncolumn *i +5]),//bid

					atoi(resultp[ncolumn *i +3]),//slot
					atoi(resultp[ncolumn *i +4]),//port
					pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,

					resultp[ncolumn *i +6]);
			break;
			case SMP_SID_SUB_SYS:
			case SMP_SID_SUB_CLK:
				printf( "%s | %s | 0x%08x | %d | %d | %d | SYS: %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					atoi(resultp[ncolumn *i +2]),//event_tid
					0,
					0,
					0,

					pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,

					resultp[ncolumn *i +6]);
			break;
			case SMP_SID_SUB_SAT:
			printf( "%s | %s | 0x%08x | %d | %d | %d | #%d %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					atoi(resultp[ncolumn *i +2]),//event_tid
					atoi(resultp[ncolumn *i +3]),//slot
					atoi(resultp[ncolumn *i +4]),//port
					atoi(resultp[ncolumn *i +5]),//bid

					atoi(resultp[ncolumn *i +3]),//slot
					pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,

					resultp[ncolumn *i +6]);
			break;	
		}
		#endif
	}
	db_free_table(resultp);
	db_unlock(pDb);
	return 0;

}


/*
   0	成功
  -1	失败
*/
int history_event_query(sqlite3 * pDb, char * pTbl, int page)
{
	char sql[256];
	char **resultp;
	int nrow;
	int ncolumn;
	int i;
	int evnt_index;

	if(NULL == pDb || NULL == pTbl || page < 1)
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
#ifdef DEBUG
	sprintf(sql, "select event_id, event_tid, slot, port, bid, event_start_time \
				  from %s order by rowid desc limit %d offset %d;", 
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE);
#else
	sprintf(sql, "select event_id, event_tid, slot, port, bid, event_start_time \
				  from %s order by rowid desc limit %d offset %d;", 
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE);
#endif
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

			//new: event_index| event_tid | slot(snmp) | port(snmp) | slot-port msg | start_time
			evnt_index = atoi(resultp[ncolumn *i +0]);
			if (evnt_index < 0 || evnt_index >= max_evntinx_num){
				db_free_table(resultp);
				db_unlock(pDb);
				return -1;
			}
			#ifdef DEBUG
			#if 1
			printf( "%s | 0x%08x | %s | %s | %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						atoi(resultp[ncolumn *i +1]),
						resultp[ncolumn *i +2],
						resultp[ncolumn *i +3],
						resultp[ncolumn *i +4],
						pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,
						resultp[ncolumn *i +5]);
			#else
			printf( "%s | 0x%08x | %d | %d | %d | %s | %s\n", 
						resultp[ncolumn *i +0],
						pEvntInxTbl[evnt_index].evnt_id,//event_tid
						pEvntInxTbl[evnt_index].slot,
						pEvntInxTbl[evnt_index].port,
						pEvntInxTbl[evnt_index].bid,
						pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,
						resultp[ncolumn *i +5]);

			#endif
			#else
			switch (pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_type){
				case SMP_SID_SUB_IN:
				case SMP_SID_SUB_OUT:
					printf( "%s | 0x%08x | %d | %d | %d | S:%d P:%d %s | %s\n", 
						resultp[ncolumn *i +0],//event_id,index
						atoi(resultp[ncolumn *i +1]),//event_tid
						atoi(resultp[ncolumn *i +2]),//slot
						atoi(resultp[ncolumn *i +3]),//port
						atoi(resultp[ncolumn *i +4]),//bid

						atoi(resultp[ncolumn *i +2]),//slot
						atoi(resultp[ncolumn *i +3]),//port
						pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,

						resultp[ncolumn *i +5]);
				break;
				case SMP_SID_SUB_SYS:
				case SMP_SID_SUB_CLK:
					printf( "%s | 0x%08x | %d | %d | %d | SYS: %s | %s\n", 
						resultp[ncolumn *i +0],
						atoi(resultp[ncolumn *i +1]),//event_tid
						0,
						0,
						0,

						pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,

						resultp[ncolumn *i +5]);
				break;
				case SMP_SID_SUB_SAT:
				printf( "%s | 0x%08x | %d | %d | %d | #%d %s | %s\n", 
						resultp[ncolumn *i +0],
						atoi(resultp[ncolumn *i +1]),//event_tid
						atoi(resultp[ncolumn *i +2]),//slot
						atoi(resultp[ncolumn *i +3]),//port
						atoi(resultp[ncolumn *i +4]),//bid

						atoi(resultp[ncolumn *i +2]),//slot
						pEvntInxTbl[evnt_index].evnt_id_tbl->evnt_msg,

						resultp[ncolumn *i +5]);
				break;
			}
			#endif
		}
		db_free_table(resultp);
		db_unlock(pDb);
		return 0;
	}
}

/*
  -1	失败
   0	成功
*/
int event_argv_parser(int argc, char *argv[])
{
	int ret = 0;
	int tmp;
	sqlite3 *pDb = NULL;

	//HELP
	if(2 == argc){
		event_helper();
		return -1;

	} else if(3 == argc){
		if(0 == memcmp(argv[2], HELP, strlen(HELP)) )
		{
			event_helper();
			return -1;
		}
	}

	initializeBoardNameTable();

	//initialize
	if(1 != initializeDatabase(&pDb))
	{
		ret = -1;
		goto exit0;
	}
	if(0 != initializeEventIndexTable())
	{
		ret = -1;
		goto exit1;
	}

	if(3 == argc)
	{
		if(0 == memcmp(argv[2], CURRENT, strlen(CURRENT)))
		{
			tmp = event_pages(pDb, TBL_EVENT_CURRENT);//获取记录的页数
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[2], HISTORY, strlen(HISTORY)))
		{
			tmp = event_pages(pDb, TBL_EVENT_HISTORY);//获取记录的页数
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[2], CURRENTNUM, strlen(CURRENTNUM)))
		{
			tmp = event_nums(pDb, TBL_EVENT_CURRENT);//获取记录数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[2], HISTORYNUM, strlen(HISTORYNUM)))
		{
			tmp = event_nums(pDb, TBL_EVENT_HISTORY);//获取记录数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else
		{
			ret = -1;
		}
	}
	else if(4 == argc)
	{
		if( (0 == memcmp(argv[2], CURRENT, strlen(CURRENT))) &&
				(event_page_validity(argv[3], TBL_EVENT_CURRENT_ARRAY_SIZE)) )
		{
			tmp = event_pages(pDb, TBL_EVENT_CURRENT);
			if(-1 == tmp){
				ret = -1;
			}else{
				if(atoi(argv[3]) <= tmp){
					if(-1 == current_event_query(pDb, TBL_EVENT_CURRENT, atoi(argv[3]))){//告警记录查询
						ret = -1;
					}else{
						ret = 0;
					}
				}else{
					ret = -1;
				}
			}
		}
		else if( (0 == memcmp(argv[2], HISTORY, strlen(HISTORY))) &&
				(event_page_validity(argv[3],TBL_EVENT_HISTORY_ARRAY_SIZE)) )
		{
			tmp = event_pages(pDb, TBL_EVENT_HISTORY);
			if(-1 == tmp){
				ret = -1;
			}else{
				if(atoi(argv[3]) <= tmp){
					if(-1 == history_event_query(pDb, TBL_EVENT_HISTORY, atoi(argv[3]))){//告警记录查询
						ret = -1;
					}else{
						ret = 0;
					}
				}else{
					ret = -1;
				}
			}
		}
		else if(0 == memcmp(argv[2], HISTORY, strlen(HISTORY)) &&
			0 == memcmp(argv[3], "delete", strlen("delete")))
		{
			char sql[128];
			bool_t ret1;

			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s;", TBL_EVENT_HISTORY);
			db_lock(pDb);
			ret1 = db_put_table(pDb, sql);
			db_unlock(pDb);
			if(false == ret1)
			{
				printf("fail to delete log\n");
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
	else
	{
		ret = -1;
	}


	cleanEventIndexTable();
exit1:
	//clean
	cleanDatabase(pDb);
exit0:
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


static char *gEvntHelpTbl[] = {
	"p350sys event --help",
	"p350sys event current",
	"        query the cur-event page",
	"p350sys event current [PAGE]",
	"        query the cur-event list of cur PAGE",
	"p350sys event curnum",
	"        query the cur-event record num",
	"p350sys event history",
	"        query the history-event page",
	"p350sys event history [PAGE]",
	"        query the history-event list of cur PAGE",
	"p350sys event hstynum",
	"        query the history-event record num",
	"p350sys event history delete",
	"        delete all history-event",
	NULL
	};


void event_helper(void)
{
	int i;
	
	for(i=0; gEvntHelpTbl[i]; i++)
	{
		printf("%s\n", gEvntHelpTbl[i]);
	}

	printf("\n");
	for(i=0; i<gEvntIdTbl_len; i++)
	{
		printf("             0x%08x | %s\n", gEvntIdTbl[i].evnt_id, gEvntIdTbl[i].evnt_msg);
	}
}

