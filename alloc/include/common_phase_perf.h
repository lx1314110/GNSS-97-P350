#include "alloc.h"
#include "lib_dbg.h"
#include "lib_sqlite.h"


int WriteCurPhasePerfToTable(sqlite3 *pDb,
	struct phase_perf_tbl_t *PhasePerfTbl,
	int PhasePerfLen);

int initCurPhasePerfAndHistoryTable(sqlite3 *pDb);

