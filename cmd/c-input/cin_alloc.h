#ifndef	__CIN_ALLOC__
#define	__CIN_ALLOC__




#include "alloc.h"




int initializeShareMemory(struct ipcinfo *ipc);
int initializeMessageQueue(struct ipcinfo *ipc);
int initializeSemaphoreSet(struct ipcinfo *ipc);
int cleanShareMemory(struct ipcinfo *ipc);
int readPid(struct ipcinfo *ipc, struct pidinfo *pid);







#endif//__CIN_ALLOC__


