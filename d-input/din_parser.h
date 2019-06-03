#ifndef	__DIN_PARSER__
#define	__DIN_PARSER__




#include "din_context.h"



bool_t isNotify(struct inCtx *ctx);
int sysLevel(char *data, struct inCtx *ctx);
int sysInput(char *data, struct inCtx *ctx);
int sysSchema(char *data, struct inCtx *ctx);
int ReadMessageQueue(struct inCtx *ctx);
int WriteConfigToShareMemory(struct inCtx *ctx, int iSlot);
int WriteAlarmToShareMemory(struct inCtx *ctx);
int SysConfSet(char *data, struct inCtx *ctx);






#endif//__DIN_PARSER__


