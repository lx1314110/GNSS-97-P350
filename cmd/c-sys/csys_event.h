#ifndef __CSYS_EVENT__
#define __CSYS_EVENT__







#include "lib_type.h"

int event_argv_parser(int argc, char *argv[]);

#if 0
#include "lib_sqlite.h"


void event_helper(void);
int event_page_validity(char *, int);
int event_pages(sqlite3 *pDb, char *pTbl);
int event_nums(sqlite3 *pDb, char *pTbl);
int current_event_query(sqlite3 * pDb, char * pTbl, int page);
int history_event_query(sqlite3 * pDb, char * pTbl, int page);
#endif
#endif//__CSYS_EVENT__


