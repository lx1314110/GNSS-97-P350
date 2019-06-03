#ifndef DOU_TIME_H_
#define DOU_TIME_H_

#include "dout_context.h"

#define COMPENSATION_TIME 2
#define COMPENSATION_SYS_TIME 1
#define COMPENSATION_PTP_TIME 1


int CreateThread(struct outCtx *ctx);
int CloseThread(struct outCtx *ctx);


#endif
