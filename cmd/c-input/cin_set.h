#ifndef	__CIN_SET__
#define	__CIN_SET__




#include "cin_context.h"




//common
int schema_set(struct inCtx *ctx, int schema, int which_src);
int prio_set(struct inCtx *ctx, int slot, int port, u8_t *prio);
int delay_set(struct inCtx *ctx, int slot, int port, int delay);
//sat
int ssm_set(struct inCtx *ctx, int slot, int port, u8_t *ssm);
int sa_set(struct inCtx *ctx, int slot, int port, u8_t *sa);
int mode_set(struct inCtx *ctx, int slot, u8_t *mode);
int elev_set(struct inCtx *ctx, int slot, u8_t *elev);
int lola_set(struct inCtx *ctx, int slot, u8_t lot, u8_t *lov, u8_t lat, u8_t *lav);

//ptp
int input_ptpin_board_set(struct inCtx *ctx, struct insetinfo *ds);








#endif//__CIN_SET__


