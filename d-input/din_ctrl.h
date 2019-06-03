#ifndef	__DIN_CTRL__
#define	__DIN_CTRL__



#include "din_context.h"




bool_t isRunning(struct inCtx *ctx);
int ReadBidFromFpga(struct inCtx * ctx);
void CopyBid(struct inCtx *ctx);
int BoardPushPull(struct inCtx *ctx);
	
int ProcInput(struct inCtx *ctx);






#endif//__DIN_CTRL__


