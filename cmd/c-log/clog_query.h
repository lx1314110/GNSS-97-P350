#ifndef	__CLOG_QUERY__
#define	__CLOG_QUERY__






#include "lib_sqlite.h"

#include "clog_context.h"








int log_pages(sqlite3 *pDb, char *pTbl);
int log_query(sqlite3 * pDb, char * pTbl, int page);













#endif//__CLOG_QUERY__


