#ifndef	__DALM_PARSER__
#define	__DALM_PARSER__





#include "dalm_context.h"


bool_t isNotify(struct almCtx *ctx);
int sysLevel(char *data, struct almCtx *ctx);
int sysMask(char *data, struct almCtx *ctx);
int sys_port_status(char *data, struct almCtx *ctx);
int sysSelect(char *data, struct almCtx *ctx);
int sys_inph_thresold(char *data, struct almCtx *ctx);


int ReadMessageQueue(struct almCtx *ctx);







#endif//__DALM_PARSER__



