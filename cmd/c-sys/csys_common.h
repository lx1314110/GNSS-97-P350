#ifndef __CSYS_COMMON__
#define __CSYS_COMMON__


#include "alloc.h"
#include "lib_sqlite.h"

int switch_validity(char *swi);
int enable_validity(char *en);

int initializeDatabase(sqlite3 **pDb);

int cleanDatabase(sqlite3 *pDb);
int sysUpdate(struct ipcinfo *ipc);
int sysReboot(struct ipcinfo *ipc);





#endif//__CSYS_COMMON__

