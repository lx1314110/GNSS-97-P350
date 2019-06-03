#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib_alarm.h"
















/*
	alarm is alarm_index(already alm_id)
  1		成功
  0		失败
*/
int db_alarm_write(sqlite3 *db, char *table, int alarm, char *start, char *end)
{
	int nrow;
	int ncolumn;
	char **resultp;
	char sql[128];

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", table);
	
	db_lock(db);
	if(db_get_table(db, sql, &resultp, &nrow, &ncolumn))
	{
		nrow = atoi(resultp[1]);
		db_free_table(resultp);
		
		if(MAX_ALM <= nrow)
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s where rowid <= (select (min(rowid) + max(rowid))/2 from %s);", table, table);
			if(!db_put_table(db, sql))
			{
				db_unlock(db);
		
				return 0;
			}
		}
		
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "insert into %s values(%d, \'%s\', \'%s\');", table, alarm, start, end);
		if(db_put_table(db, sql))
		{
			db_unlock(db);
		
			return 1;
		}
		else
		{
			db_unlock(db);
		
			return 0;
		}
	}
	else
	{
		db_unlock(db);
		
		return 0;
	}
}











