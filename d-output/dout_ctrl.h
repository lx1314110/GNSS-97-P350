#ifndef __DOUT_CTRL__
#define __DOUT_CTRL__




#include "dout_context.h"





bool_t isRunning(struct outCtx *ctx);
void CopyBid(struct outCtx *ctx);
int BoardPushPull(struct outCtx *ctx);
int ProcOutput(struct outCtx *ctx);








#endif//__DOUT_CTRL__


