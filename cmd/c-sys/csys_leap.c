#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_sqlite.h"
#include "csys_leap.h"




/*
  0	不合法
  1 合法
*/
int leap_validity(char *leap)
{
	int num;
	int i, len;

	len = strlen(leap);
	for(i=0; i<len; i++)
	{
		if(leap[i] < '0' || leap[i] > '9')
			return 0;
	}

	num = atoi(leap);
	if(num < 0 || num >99)
		return 0;
	else
		return 1;
}



/*
  -1	无法打开数据库
  -2	无法设置闰秒
   0	成功设置闰秒
*/
s32_t modify_leap(char *pDbPath, char *tbl_sys, struct leapinfo *pLeap)
{
	sqlite3 *pDb = NULL;
	char sql[256];
	bool_t ret1;

	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}
	
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set leapsecond=%d, leapmode=%d;", tbl_sys, pLeap->leapSecond,pLeap->leapMode);
	db_lock(pDb);
	
	ret1 = db_put_table(pDb, sql);
	if(false == ret1)
	{
		db_unlock(pDb);
		db_close(pDb);
		//无法设置闰秒
		return -2;
	}

	db_unlock(pDb);
	db_close(pDb);
	//成功设置闰秒
	return 0;
}




/*
  -1	无法打开数据库
  -2	无法获取闰秒
   0	成功获取闰秒
*/
s32_t query_leap(char *pDbPath, char *tbl_sys, struct leapinfo *pLeap)
{
	bool_t ret2;
	char sql[128];
	sqlite3 *pDb = NULL;
	char **resultp;
	int nrow;
	int ncolumn;

	ret2 = db_open(pDbPath, &pDb);
	if( false == ret2 )
	{
		//无法打开数据库
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select leapsecond, leapmode from %s;", tbl_sys);
	db_lock(pDb);
	ret2 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret2)
	{
		db_unlock(pDb);
		db_close(pDb);
		//无法获取闰秒
		return -2;
	}
	else
	{
		pLeap->leapSecond = atoi(resultp[2]);
		pLeap->leapMode= atoi(resultp[3]);
		db_free_table(resultp);
		db_unlock(pDb);
		db_close(pDb);
		//成功获取闰秒
		return 0;
	}
}



