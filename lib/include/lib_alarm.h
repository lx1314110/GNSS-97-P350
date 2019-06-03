#ifndef	__LIB_ALARM__
#define	__LIB_ALARM__







#include "lib_sqlite.h"













#define	MAX_ALM		10000













int db_alarm_write(sqlite3 *db, char *table, int alarm, char *start, char *end);


















#endif//__LIB_ALARM__


