#ifndef	__LIB_LOG__
#define	__LIB_LOG__







#include "lib_sqlite.h"













#define	MAX_LOG				10000













int db_log_write(sqlite3 *db, char *table, char *desc);

int send_log_message(char *buf);
















#endif//__LIB_LOG__


