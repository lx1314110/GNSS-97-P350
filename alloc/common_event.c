#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


#include "lib_bit.h"
#include "common_event.h"

static int WriteOneCurEventToTable(sqlite3 *pDb, struct evnt_inx_tbl_t EvntInxTbl,
	char *pTbl);

/*
  1		成功
  0		失败
*/
int WriteOneCurEventToTable(sqlite3 *pDb,
	struct evnt_inx_tbl_t EvntInxTbl,
	char *pTbl)
{
	int nrow = 0;
	int count = 0;
	int ncolumn = 0;
	int seq = 0;
	char **resultp;
	char sql[512];

	if(!pDb || !pTbl)
		return false;

#define TBL_EVENT_CURRENT_ID_PRI_KEY_AUTOINC
	#ifdef TBL_EVENT_CURRENT_ID_PRI_KEY_AUTOINC
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	//printf("%s\n",sql);

	db_lock(pDb);

	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		db_unlock(pDb);
		print(	DBG_ERROR,"WriteOneCurEventToTable db_get_table failure!");
		return 0;
	}
	count = atoi(resultp[1]);
	db_free_table(resultp);
	if(count == 0){
		//cur event table is blank
		goto insert_one_data;
	}
	
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select seq from sqlite_sequence where name=\'%s\';", pTbl);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		db_unlock(pDb);
		return 0;
	}
	seq = atoi(resultp[1]);//query id INTEGER PRIMARY KEY AUTOINCREMENT max value, seq
	db_free_table(resultp);

	if(TBL_EVENT_CURRENT_ID_MAX_SEQ <= seq){
		//move all cur-event to history database and delete the cur all event
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into %s(event_id, event_tid, slot, port, bid, event_start_time) \
			select event_id, event_tid, slot, port, bid, event_start_time \
		    from %s; \
		    delete from %s;delete from sqlite_sequence where name='%s';", 
		  TBL_EVENT_HISTORY, pTbl, pTbl, pTbl);
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			db_unlock(pDb);
			print(	DBG_ERROR,"WriteOneCurEventToTable db_put_table failure!");
			return 0;
		}
	}
	db_unlock(pDb);
	#endif

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	//printf("%s\n",sql);
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		db_unlock(pDb);
		print(	DBG_ERROR,"WriteOneCurEventToTable db_get_table failure!");
		return 0;
	}
	count = atoi(resultp[1]);
	db_free_table(resultp);

	if(TBL_EVENT_CURRENT_ARRAY_SIZE <= count){
		//move old one cur-event to history database
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into %s(event_id, event_tid, slot, port, bid, event_start_time) \
			select event_id, event_tid, slot, port, bid, event_start_time \
		    from %s where rowid <= (select min(rowid) from %s); \
		    delete from %s where rowid <= (select min(rowid) from %s);", 
		  TBL_EVENT_HISTORY, pTbl, pTbl, pTbl, pTbl);
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			db_unlock(pDb);
			print(	DBG_ERROR,"WriteOneCurEventToTable db_put_table failure!");
			return 0;
		}
	}

insert_one_data:
	//printf("seq:%d nrow:%d\n",seq, count);
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "insert into %s values(NULL, %d, %d, %d, %d, %d, \'%s\', %d);", pTbl,
	EvntInxTbl.index, EvntInxTbl.evnt_id, EvntInxTbl.slot, EvntInxTbl.port, EvntInxTbl.bid, EvntInxTbl.start_time, 0);
	//printf("%s\n",sql);
	if(!db_put_table(pDb, sql)){
		db_unlock(pDb);
		print(	DBG_ERROR,"WriteOneCurEventToTable db_put_table failure!");
		return 0;
	}

	db_unlock(pDb);
	return 1;
}

int MoveAllCurEventToHistoryTable(sqlite3 *pDb,
	char *pTbl)
{
	int nrow = 0;
	int ncolumn = 0;
	int count = 0;
	int seq = 0;
	char **resultp;
	char sql[512];

	if(!pDb || !pTbl)
		return false;

#define TBL_EVENT_CURRENT_ID_PRI_KEY_AUTOINC
	#ifdef TBL_EVENT_CURRENT_ID_PRI_KEY_AUTOINC
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	//printf("%s\n",sql);
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		db_unlock(pDb);
		print(	DBG_ERROR,"WriteOneCurEventToTable db_get_table failure!");
		return 0;
	}
	count = atoi(resultp[1]);
	db_free_table(resultp);
	//printf("count:%d\n",count);
	if(count > 0){

		//update seq/ID to MAX(ID) for cur_event
		memset(sql, 0, sizeof(sql));
		sprintf(sql, 
		"update sqlite_sequence set seq=(select max(ID) from %s) \
		where name='%s';",
		  pTbl, pTbl);//delete from sqlite_sequence where name='%s';", 
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			db_unlock(pDb);
			print(	DBG_ERROR,"update sqlite_sequence to max db_put_table failure!");
			return 0;
		}
	
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "select seq from sqlite_sequence where name=\'%s\';", pTbl);
		if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
			db_unlock(pDb);
			return 0;
		}
		seq = atoi(resultp[1]);//query id INTEGER PRIMARY KEY AUTOINCREMENT max value, seq
		db_free_table(resultp);
		print(DBG_INFORMATIONAL, "%s: last PRI_KEY_AUTOINC:%d", pTbl, seq);

		//copy all cur-event to history database and delete the cur all event
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into %s(event_id, event_tid, slot, port, bid, event_start_time) \
			select event_id, event_tid, slot, port, bid, event_start_time \
		    from %s; delete from %s;",
		  TBL_EVENT_HISTORY, pTbl, pTbl);//delete from sqlite_sequence where name='%s';", 
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			db_unlock(pDb);
			print(	DBG_ERROR,"MoveAllCurEventToHistoryTable db_put_table failure!");
			return 0;
		}
	}
	db_unlock(pDb);
	#endif
	return 1;
}


/*
* 不能超过10000，否则将删除前5000条
  1		成功
  0		失败
*/
int CheckHistoryEventNumTable(sqlite3 *pDb, char *pTbl)
{
	int nrow;
	int ncolumn;
	char **resultp;
	char sql[128];

	if(!pDb || !pTbl)
		return false;
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		db_unlock(pDb);
		return 0;
	}

	nrow = atoi(resultp[1]);
	db_free_table(resultp);
	
	if(TBL_EVENT_HISTORY_ARRAY_SIZE <= nrow)
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "delete from %s where rowid <= (select (min(rowid) + max(rowid))/2 from %s);", pTbl, pTbl);
		if(!db_put_table(pDb, sql)){
			db_unlock(pDb);
			return 0;
		}
	}

	db_unlock(pDb);
	return 1;
}

/*
  1		成功
  0		失败
*/
int WriteCurEventToTable(sqlite3 *pDb,
	struct evnt_inx_tbl_t *EvntInxTbl,
	int EvntInxLen)
{
	int i;

	if(!pDb || !EvntInxTbl || EvntInxLen <= 0)
		return false;

	for (i = 0; i < EvntInxLen; ++i){
		if (EvntInxTbl[i].valid == 1){
			EvntInxTbl[i].valid = 0;
			if(!CheckHistoryEventNumTable(pDb, TBL_EVENT_HISTORY)){
				print(	DBG_ERROR, 
						"Check historyeventNum failure!");
				return false;
			}
			if(!WriteOneCurEventToTable(pDb, EvntInxTbl[i], TBL_EVENT_CURRENT)){
				print(DBG_ERROR, "WriteOneCurEventToTable[%d] index:%d slot:%d port:%d bid:%d event:%s failure!",
				i,EvntInxTbl[i].index, EvntInxTbl[i].slot, EvntInxTbl[i].port, EvntInxTbl[i].bid,
				EvntInxTbl[i].evnt_id_tbl->evnt_msg);
				return false;
			}
		}

		EvntInxTbl[i].old_flag = EvntInxTbl[i].new_flag;
	}
	return true;
}




int initCurEventAndHistoryTable(sqlite3 *pDb)
{

	if(!pDb)
		return false;

	if(!CheckHistoryEventNumTable(pDb, TBL_EVENT_HISTORY)){
		print(	DBG_ERROR, 
				"Check historyeventNum failure!");
		return false;
	}
	if(!MoveAllCurEventToHistoryTable(pDb, TBL_EVENT_CURRENT)){
		print(	DBG_ERROR, "MoveAllCurEventToHistoryTable failure!");
		return false;
	}
	return true;
}

