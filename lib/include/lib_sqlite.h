#ifndef __LIB_SQLITE__
#define __LIB_SQLITE__


#include "sqlite3.h"

#include "lib_type.h"



bool_t db_open(char *filename, sqlite3 **ppDb);
bool_t db_close(sqlite3 *pDb);

bool_t db_put_table(sqlite3 *pDb, char *sql);

bool_t db_get_table(
	sqlite3 *pDb,
	char *sql,
	char ***resultp,
	int *nrow,
	int *ncolumn );
void db_free_table(char **resultp);

void db_lock(sqlite3 *pDb);
void db_unlock(sqlite3 *pDb);
void db_rollback(sqlite3 *pDb);

/*have lock and unlock*/
bool_t empty_table(sqlite3 *pDb, char * pTbl);




#endif//__LIB_SQLITE__

