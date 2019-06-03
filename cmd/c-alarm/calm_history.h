#ifndef	__CALM_HISTORY__
#define	__CALM_HISTORY__




#include "lib_sqlite.h"
#include "calm_context.h"



int history_alarm_pages(sqlite3 *pDb, char *pTbl);
int history_alarm_query(struct almCtx * ctx, char * pTbl, int page);
int history_alarm_nums(sqlite3 *pDb, char *pTbl);

#endif//__CALM_HISTORY__


