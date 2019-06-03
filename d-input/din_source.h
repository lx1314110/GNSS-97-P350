#ifndef	__DIN_SOURCE__
#define	__DIN_SOURCE__




#include "din_context.h"


int FreshLeapSecond(int fpga_fd,int leapSecond);
int FreshTime(struct inCtx *ctx,int leapSecond);
int FreshTime_no_leapSecond(struct inCtx *ctx);


int bis_free(struct inCtx *ctx);
int bis_force(struct inCtx *ctx, int line);

int best_input_source(struct inCtx *ctx);

int stratum_input_source(struct inCtx *ctx);

int WriteInSourceInfoToShareMemory(struct inCtx *ctx);







#endif//__DIN_SOURCE__


