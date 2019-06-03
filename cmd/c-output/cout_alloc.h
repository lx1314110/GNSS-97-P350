#ifndef	__COUT_ALLOC__
#define	__COUT_ALLOC__



#include "cout_context.h"
#include "alloc.h"




int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);

int readPid(struct ipcinfo *ipc, struct pidinfo *pid);
int readBid(struct outCtx * ctx);








#endif//__COUT_ALLOC__

