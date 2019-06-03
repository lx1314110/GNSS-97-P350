#ifndef	__DOUT_PARSER__
#define	__DOUT_PARSER__




#include "dout_context.h"




bool_t isNotify(struct outCtx *ctx);
int sysLevel(char *data, struct outCtx *ctx);
int sysInZone(char *data, struct outCtx *ctx);
int sysOutZone(char *data, struct outCtx *ctx);
int sysLeapsecond(char *data, struct outCtx *ctx);
int sysOutput(char *data, struct outCtx *ctx);
int sysDelay(char *data, struct outCtx *ctx);
int sysOutBaudrate(char *data, struct outCtx *ctx);
int sysIrigbAmplitude(char *data, struct outCtx *ctx);
int ReadMessageQueue(struct outCtx *ctx);







#endif//__DOUT_PARSER__


