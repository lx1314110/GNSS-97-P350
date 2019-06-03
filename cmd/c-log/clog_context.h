#ifndef	__CLOG_CONTEXT__
#define	__CLOG_CONTEXT__








#include "alloc.h"









struct logCtx {
	sqlite3 *pDb;
	struct ipcinfo ipc;
	struct pidinfo pid;
};










#endif//__CLOG_CONTEXT__


