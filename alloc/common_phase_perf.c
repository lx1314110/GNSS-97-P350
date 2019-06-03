#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


#include "lib_bit.h"
#include "common.h"

#define  DB_LOCK(pDb, isLock)	do { \
if(isLock) \
	db_lock(pDb); \
}while(0)

#define  DB_UNLOCK(pDb, isLock)	do { \
if(isLock) \
	db_unlock(pDb); \
}while(0)

static int CheckHistoryPhasePerfNumTable(sqlite3 *pDb, char *pTbl, int is_lock);
static int WriteOneCurPhasePerfToTable(sqlite3 *pDb, struct phase_perf_tbl_t PhasePerfTbl,
	char *pTbl, int is_lock);
static int MoveAllCurPhasePerfToHistoryTable(sqlite3 *pDb,
	char *pTbl, int is_lock);

/*
  1		成功
  0		失败
*/
static int WriteOneCurPhasePerfToTable(sqlite3 *pDb,
	struct phase_perf_tbl_t PhasePerfTbl,
	char *pTbl, int is_lock)
{
	int nrow = 0;
	int count = 0;
	int ncolumn = 0;
	int seq = 0;
	char **resultp;
	char sql[512];

	if(!pDb || !pTbl)
		return false;

#define TBL_PhasePerf_CURRENT_ID_PRI_KEY_AUTOINC
	#ifdef TBL_PhasePerf_CURRENT_ID_PRI_KEY_AUTOINC
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	//printf("%s\n",sql);

	DB_LOCK(pDb, is_lock);

	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		db_unlock(pDb);
		print(	DBG_ERROR,"WriteOneCurPhasePerfToTable db_get_table failure!");
		return 0;
	}
	count = atoi(resultp[1]);
	db_free_table(resultp);
	if(count == 0){
		//cur PhasePerf table is blank
		goto insert_one_data;
	}
	
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select seq from sqlite_sequence where name=\'%s\';", pTbl);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		DB_UNLOCK(pDb, is_lock);
		return 0;
	}
	seq = atoi(resultp[1]);//query id INTEGER PRIMARY KEY AUTOINCREMENT max value, seq
	db_free_table(resultp);

	if(TBL_PHASE_PERF_CURRENT_ID_MAX_SEQ <= seq){
		//move all cur-PhasePerf to history database and delete the cur all PhasePerf
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into %s(ID, phase_inx, phase_val, slot, port, clock_status, phase_time) \
			select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
		    from %s; \
		    delete from %s;delete from sqlite_sequence where name='%s';", 
		  TBL_PERF_HISTORY, pTbl, pTbl, pTbl);
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			DB_UNLOCK(pDb, is_lock);
			print(	DBG_ERROR,"WriteOneCurPhasePerfToTable db_put_table failure!");
			return 0;
		}
	}
	DB_UNLOCK(pDb, is_lock);
	#endif

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	//printf("%s\n",sql);
	DB_LOCK(pDb, is_lock);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		DB_UNLOCK(pDb, is_lock);
		print(	DBG_ERROR,"WriteOneCurPhasePerfToTable db_get_table failure!");
		return 0;
	}
	count = atoi(resultp[1]);
	db_free_table(resultp);

	if(TBL_PHASE_PERF_CURRENT_ARRAY_SIZE <= count){
		//move old one cur-PhasePerf to history database-end，一条一条移动
		//因为60s才写一次数据库，所以此处也可以改为一次移动TBL_PHASE_PERF_CURRENT_ARRAY_SIZE多条数据
		memset(sql, 0, sizeof(sql));
		#if 1
		//move a data
		sprintf(sql, "insert into %s(ID, phase_inx, phase_val, slot, port, clock_status, phase_time) \
			select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
		    from %s where rowid <= (select min(rowid) from %s); \
		    delete from %s where rowid <= (select min(rowid) from %s);", 
		  TBL_PERF_HISTORY, pTbl, pTbl, pTbl, pTbl);
		#else
		//move all cur-data to history end
		sprintf(sql, "insert into %s(ID, phase_inx, phase_val, slot, port, clock_status, phase_time) \
			select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
		    from %s; \
		    delete from %s;", 
		  TBL_PERF_HISTORY, pTbl, pTbl);
		#endif
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			DB_UNLOCK(pDb, is_lock);
			print(	DBG_ERROR,"WriteOneCurPhasePerfToTable db_put_table failure!");
			return 0;
		}
	}

insert_one_data:
	//printf("seq:%d nrow:%d\n",seq, count);ID, phase_inx, phase_val, slot, port, clock_status, phase_time, is_read
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "insert into %s values(NULL, %d, %d, %d, %d, %d, \'%s\', %d);", pTbl,
	PhasePerfTbl.index, PhasePerfTbl.val, PhasePerfTbl.slot, PhasePerfTbl.port,
	PhasePerfTbl.clock_status, PhasePerfTbl.time, 0);
	//printf("%s\n",sql);
	if(!db_put_table(pDb, sql)){
		DB_UNLOCK(pDb, is_lock);
		print(	DBG_ERROR,"WriteOneCurPhasePerfToTable db_put_table failure!");
		return 0;
	}

	DB_UNLOCK(pDb, is_lock);
	return 1;
}

static int MoveAllCurPhasePerfToHistoryTable(sqlite3 *pDb,
	char *pTbl, int is_lock)
{
	int nrow = 0;
	int ncolumn = 0;
	int count = 0;
	int seq = 0;
	char **resultp;
	char sql[512];

	if(!pDb || !pTbl)
		return false;

#define TBL_PhasePerf_CURRENT_ID_PRI_KEY_AUTOINC
	#ifdef TBL_PhasePerf_CURRENT_ID_PRI_KEY_AUTOINC
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	//printf("%s\n",sql);
	DB_LOCK(pDb, is_lock);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		DB_UNLOCK(pDb, is_lock);
		print(	DBG_ERROR,"WriteOneCurPhasePerfToTable db_get_table failure!");
		return 0;
	}
	count = atoi(resultp[1]);
	db_free_table(resultp);
	//printf("count:%d\n",count);
	if(count > 0){

		//update seq/ID to MAX(ID) for cur_PhasePerf, 也可以不用管，自动会保留
		memset(sql, 0, sizeof(sql));
		sprintf(sql, 
		"update sqlite_sequence set seq=(select max(ID) from %s) \
		where name='%s';",
		  pTbl, pTbl);//delete from sqlite_sequence where name='%s';", 
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			DB_UNLOCK(pDb, is_lock);
			print(	DBG_ERROR,"update sqlite_sequence to max db_put_table failure!");
			return 0;
		}
	
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "select seq from sqlite_sequence where name=\'%s\';", pTbl);
		if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
			DB_UNLOCK(pDb, is_lock);
			return 0;
		}
		seq = atoi(resultp[1]);//query id INTEGER PRIMARY KEY AUTOINCREMENT max value, seq
		db_free_table(resultp);
		print(DBG_INFORMATIONAL, "%s: last PRI_KEY_AUTOINC:%d", pTbl, seq);

		//copy all cur-PhasePerf to history database and delete the cur all PhasePerf
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into %s(ID, phase_inx, phase_val, slot, port, clock_status, phase_time) \
			select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
		    from %s; delete from %s;",
		  TBL_PERF_HISTORY, pTbl, pTbl);//delete from sqlite_sequence where name='%s';", 
		//printf("%s\n",sql);
		if(!db_put_table(pDb, sql)){
			DB_UNLOCK(pDb, is_lock);
			print(	DBG_ERROR,"MoveAllCurPhasePerfToHistoryTable db_put_table failure!");
			return 0;
		}
	}
	DB_UNLOCK(pDb, is_lock);
	#endif
	return 1;
}


/*
* 不能超过10000，否则将删除前5000条
  1		成功
  0		失败
*/
static int CheckHistoryPhasePerfNumTable(sqlite3 *pDb, char *pTbl, int is_lock)
{
	int nrow;
	int ncolumn;
	char **resultp;
	char sql[128];

	if(!pDb || !pTbl)
		return false;
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	DB_LOCK(pDb, is_lock);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn)){
		DB_UNLOCK(pDb, is_lock);
		return 0;
	}

	nrow = atoi(resultp[1]);
	db_free_table(resultp);
	
	if(TBL_PHASE_PERF_HISTORY_ARRAY_SIZE <= nrow)
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "delete from %s where rowid <= (select (min(rowid) + max(rowid))/2 from %s);", pTbl, pTbl);
		if(!db_put_table(pDb, sql)){
			DB_UNLOCK(pDb, is_lock);
			return 0;
		}
	}

	DB_UNLOCK(pDb, is_lock);
	return 1;
}

/*
  1		成功
  0		失败
*/
int WriteCurPhasePerfToTable(sqlite3 *pDb,
	struct phase_perf_tbl_t *PhasePerfTbl,
	int PhasePerfLen)
{
	int i;

	if(!pDb || !PhasePerfTbl || PhasePerfLen <= 0)
		return false;
	DB_LOCK(pDb, 1);
	for (i = 0; i < PhasePerfLen; ++i){
		if(!CheckHistoryPhasePerfNumTable(pDb, TBL_PERF_HISTORY, 0)){
			print(	DBG_ERROR, 
					"Check historyPhasePerfNum failure!");
			return false;
		}
		if(!WriteOneCurPhasePerfToTable(pDb, PhasePerfTbl[i], TBL_PERF_CURRENT, 0)){
			print(	DBG_ERROR, "WriteOneCurPhasePerfToTable[%d] index:%d phase:%d slot:%d port:%d clk_sta:%d time:%s failure!",
			i,PhasePerfTbl[i].index, PhasePerfTbl[i].val, PhasePerfTbl[i].slot, PhasePerfTbl[i].port,
			PhasePerfTbl[i].clock_status,
			PhasePerfTbl[i].time);
			return false;
		}
	}
	DB_UNLOCK(pDb, 1);
	return true;
}

int initCurPhasePerfAndHistoryTable(sqlite3 *pDb)
{

	if(!pDb)
		return false;

	if(!CheckHistoryPhasePerfNumTable(pDb, TBL_PERF_HISTORY, 1)){
		print(	DBG_ERROR, 
				"Check historyPhasePerfNum failure!");
		return false;
	}
	if(!MoveAllCurPhasePerfToHistoryTable(pDb, TBL_PERF_CURRENT, 1)){
		print(	DBG_ERROR, "MoveAllCurPhasePerfToHistoryTable failure!");
		return false;
	}
	return true;
}

