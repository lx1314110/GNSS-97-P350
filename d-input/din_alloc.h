#ifndef	__DIN_ALLOC__
#define	__DIN_ALLOC__




#include "din_context.h"




int initializeDatabase(struct inCtx *ctx);
int initializeFpga(struct inCtx *ctx);
int initializeInt(struct inCtx *ctx);
void initializeContext(struct inCtx *ctx);
void cleanContext(struct inCtx *ctx);

int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);

int cleanDatabase(struct inCtx *ctx);
int cleanFpga(struct inCtx *ctx);
int cleanInt(struct inCtx *ctx);
int cleanShareMemory(struct ipcinfo *ipc);

int initializeExitSignal(void);
int initializeNotifySignal(void);
int initializePriority(int prio);

int writePid(struct ipcinfo *ipc);

int ReadLeapsecond(struct inCtx *ctx, char *pTbl);

void SetVarinfo(struct inCtx *ctx);




#endif//__DIN_ALLOC__


