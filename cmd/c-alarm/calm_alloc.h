#ifndef	__CALM_ALLOC__
#define	__CALM_ALLOC__






#include "alloc.h"
#include "calm_context.h"








int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);
int initializeDatabase(struct almCtx *ctx);
int cleanDatabase(struct almCtx *ctx);
int readPid(struct ipcinfo *ipc, struct pidinfo *pid);

int ReadBidFromFpga(struct almCtx * ctx);










#endif//__CALM_ALLOC__


