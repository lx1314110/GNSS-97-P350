#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_sema.h"
#include "lib_shm.h"

#include "clog_global.h"
#include "clog_query.h"









/*
  >0	成功
  -1	失败
*/
int log_pages(sqlite3 *pDb, char *pTbl)
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
   0	成功
  -1	失败
*/
int log_query(sqlite3 * pDb, char * pTbl, int page)
{
	char sql[256];
	char **resultp;
	int nrow;
	int ncolumn;
	int i;

	if((NULL == pDb) || (NULL == pTbl) || (page < 1))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select log_time, log_description from %s order by log_time desc limit %d offset %d;", 
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
			printf( "%s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1] );
		}
		db_free_table(resultp);
		db_unlock(pDb);
		return 0;
	}
}









