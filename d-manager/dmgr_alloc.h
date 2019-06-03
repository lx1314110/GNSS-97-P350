#ifndef __DMGR_ALLOC__
#define __DMGR_ALLOC__




#include "lib_sqlite.h"
#include "alloc.h"
#include "dmgr_context.h"






int initializeContext(struct mgrCtx *ctx);

int initializeShareMemory(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);


int cleanShareMemory(struct ipcinfo *ipc);
int cleanMessageQueue(struct ipcinfo *ipc);
int cleanSemaphoreSet(struct ipcinfo *ipc);


int initializeExitSignal(void);
int initializeNotifySignal(void);
int initializeChildSignal(void);

int initializePriority(int prio);

int writePid(struct ipcinfo *ipc);

void SetDatabase(void);
void SetVethDatabase(void);
int initializeDatabase(sqlite3 **pDb);
int initializeVethDatabase(sqlite3 **pDb);
int cleanDatabase(sqlite3 *pDb);
int cleanVethDatabase(sqlite3 *pDb);

//int readNetwork(sqlite3 *pDb, char *tbl, struct netinfo *net);
int readWebNetwork(sqlite3 *pDb, char *tbl, struct netinfo *net);
//int writeNetwork(struct netinfo *netp);
int setNetworkUp(void);
int writeWebNetwork(struct netinfo *netp);

int initializeFpga(struct mgrCtx *ctx);
int cleanFpga(struct mgrCtx *ctx);

int initializePS(struct mgrCtx *ctx);
int cleanPS(struct mgrCtx *ctx);








#endif//__DMGR_ALLOC__


