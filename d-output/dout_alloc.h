#ifndef	__DOUT_ALLOC__
#define	__DOUT_ALLOC__



#include "dout_context.h"




int initializeDatabase(struct outCtx *ctx);
int initializeFpga(struct outCtx *ctx);
int initializeInt(struct outCtx *ctx);
void initializeContext(struct outCtx *ctx);
int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);

int cleanDatabase(struct outCtx *ctx);
int cleanFpga(struct outCtx *ctx);
int cleanInt(struct outCtx *ctx);
int cleanShareMemory(struct ipcinfo *ipc);

int initializeExitSignal(void);
int initializeNotifySignal(void);
int initializePriority(int prio);

int writePid(struct ipcinfo *ipc);





#endif//__DOUT_ALLOC__


