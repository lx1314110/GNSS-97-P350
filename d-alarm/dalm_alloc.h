#ifndef	__DALM_ALLOC__
#define	__DALM_ALLOC__






#include "dalm_context.h"






void initializeContext(struct almCtx *ctx);
int initializeExitSignal(void);
int initializeNotifySignal(void);
int initializePriority(int prio);
int initializeDatabase(struct almCtx *ctx);
int initializeFpga(struct almCtx *ctx);
int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);


int cleanShareMemory(struct ipcinfo *ipc);
int cleanDatabase(struct almCtx *ctx);
int cleanFpga(struct almCtx *ctx);

int writePid(struct ipcinfo *ipc);










#endif//__DALM_ALLOC__


