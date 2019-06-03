#ifndef	__DNTP_ALLOC__
#define	__DNTP_ALLOC__






#include "dntp_context.h"







bool_t broadcast_enable(struct ntpCtx *ctx);
bool_t broadcast_disable(void);

int initializeSignal(void);

int initializePriority(int prio);
int initializeDatabase(sqlite3 **pDb);
int initializeVethDatabase(sqlite3 **pDb);

int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);
int initializeFpga(struct ntpCtx *ctx);
int cleanFpga(struct ntpCtx *ctx);

void cleanSock();


void initializeContext(struct ntpCtx *ctx);

int cleanDatabase(sqlite3 *pDb);
int cleanVethDatabase(sqlite3 *pDb);

int cleanShareMemory(struct ipcinfo *ipc);
int writePid(struct ipcinfo *ipc);










#endif//__DNTP_ALLOC__


