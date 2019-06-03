#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "lib_sqlite.h"

/*
  @filename	Database filename (UTF-8)
  @ppDb		SQLite db handle
*/
bool_t db_open(char *filename, sqlite3 **ppDb)
{
	if( SQLITE_OK != sqlite3_open((const char *)filename, ppDb) )
	{
		sqlite3_close(*ppDb);
		return false;
	}
	else
	{
		return true;
	}
}



/*
  @pDb		SQLite db handle
*/
bool_t db_close(sqlite3 *pDb)
{
	if( SQLITE_OK != sqlite3_close(pDb) )
	{
		return false;
	}
	else
	{
		pDb = NULL;
		return true;
	}
}



/*
  @pDb	An open database
  @sql	SQL to be executed
*/
bool_t db_put_table(sqlite3 *pDb, char *sql)
{
	char *errmsg = NULL;//Error msg written here
	
	if( SQLITE_OK != sqlite3_exec(pDb, sql, NULL, NULL, &errmsg) )

	{
		fprintf(stderr,"%s:%s:%d:%s\n",__FILE__, __func__, __LINE__, errmsg);
		if( NULL != errmsg )
		{
			sqlite3_free(errmsg);
		}
		return false;
	}
	else
	{
		return true;
	}
}




/*
  @pDb	An open database
  @sql	SQL to be executed
  @resultp	Result written to a char *[] that this points to
  @nrow		Number of result rows written here
  @ncolumn	Number of result columns written here
*/
bool_t db_get_table(
	sqlite3 *pDb,
	char *sql,
	char ***resultp,
	int *nrow,
	int *ncolumn )
{
	char *errmsg = NULL;//Error msg written here

	if( SQLITE_OK != sqlite3_get_table(pDb, sql, resultp, nrow, ncolumn, &errmsg) )
	{
		fprintf(stderr,"%s:%s:%d:%s\n",__FILE__, __func__, __LINE__, errmsg);
		if( NULL != errmsg )
		{
			sqlite3_free(errmsg);
		}
		return false;
	}
	else
	{
		return true;
	}
}





void db_free_table(char **resultp)
{
	sqlite3_free_table(resultp);
}



/*
  @pDb	An open database
*/
void db_lock(sqlite3 *pDb)
{
	char *errmsg = NULL;//Error msg written here
	int timeout = 10000;
	while( SQLITE_OK != sqlite3_exec(pDb, "begin immediate;", NULL, NULL, &errmsg) && timeout--)
	{
		//fprintf(stderr,"%s:%s:%d:%s\n",__FILE__, __func__, __LINE__, errmsg);
		if( NULL != errmsg )
		{
			sqlite3_free(errmsg);
			errmsg = NULL;
		}
		usleep(2000);
	}
}



/*
  @pDb	An open database
*/
void db_unlock(sqlite3 *pDb)
{
	char *errmsg = NULL;//Error msg written here
	int timeout = 10000;
	while( SQLITE_OK != sqlite3_exec(pDb, "commit;", NULL, NULL, &errmsg) && timeout--)
	{
		//fprintf(stderr,"%s:%s:%d:%s\n",__FILE__, __func__, __LINE__, errmsg);
		if( NULL != errmsg )
		{
			sqlite3_free(errmsg);
			errmsg = NULL;
		}
		usleep(2000);
	}
}



/*
  @pDb	An open database
*/
void db_rollback(sqlite3 *pDb)
{
	char *errmsg = NULL;//Error msg written here
	int timeout = 10000;
	while( SQLITE_OK != sqlite3_exec(pDb, "rollback;", NULL, NULL, &errmsg) && timeout--)
	{
		fprintf(stderr,"%s:%s:%d:%s\n",__FILE__, __func__, __LINE__, errmsg);
		if( NULL != errmsg )
		{
			sqlite3_free(errmsg);
			errmsg = NULL;
		}
		usleep(2000);
	}
}

bool_t empty_table(sqlite3 *pDb, char * pTbl)
{
	//int nrow;
	//int ncolumn;
	char sql[128];

	db_lock(pDb);
		
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "delete from %s;", 
		pTbl);

	if(!db_put_table(pDb, sql))
	{
		db_unlock(pDb);

		return false;
	}
	db_unlock(pDb);
	return true;
}


