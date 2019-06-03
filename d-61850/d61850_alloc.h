#ifndef	__D61850_ALLOC__
#define	__D61850_ALLOC__






#include "d61850_context.h"




bool_t isNotify(struct iec61850_Ctx *ctx);

int initializeDatabase(struct iec61850_Ctx *ctx);

int initializeMessageQueue(struct ipcinfo *ipc);


int initializeShareMemory(struct ipcinfo *ipc);

int initializeSemaphoreSet(struct ipcinfo *ipc);

int writePid(struct ipcinfo *ipc);

int ReadMessageQueue(struct iec61850_Ctx *ctx);

int set61850_network(struct iec61850_Ctx *ctx, char *pTbl);




void initializeContext(struct iec61850_Ctx *ctx);
int initializeSerial(struct iec61850_Ctx *ctx);
int initializeFpga(struct iec61850_Ctx *ctx);
int cleanFpga(struct iec61850_Ctx *ctx);
int cleanSerial(struct iec61850_Ctx *ctx);

int cleanShareMemory(struct ipcinfo *ipc);

int cleanDatabase(struct iec61850_Ctx *ctx);


int initializeExitSignal(void);
int initializeNotifySignal(void);











#endif


