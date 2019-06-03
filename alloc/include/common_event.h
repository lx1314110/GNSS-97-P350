#include "alloc.h"
#include "lib_dbg.h"
#include "lib_sqlite.h"

int CheckHistoryEventNumTable(sqlite3 *pDb, char *pTbl);
int WriteCurEventToTable(sqlite3 *pDb,
	struct evnt_inx_tbl_t *EvntInxTbl,
	int EvntInxLen);

int initCurEventAndHistoryTable(sqlite3 *pDb);

