#ifndef	__COUT_SET__
#define	__COUT_SET__





#include "cout_context.h"

#define IV_BUF_LEN 32
#define IAR_BUF_LEN 32
#define ZONE_BUF_LEN 32
#define BAUDRATE_BUF_LEN 32


//may no use it
int in_zone_set(struct outCtx *ctx, char *zone_grp, int group);

int out_zone_set(struct outCtx *ctx, char *zone_grp, int group);
int out_baudrate_set(struct outCtx *ctx, char *br_grp, int group);
int irigb_amplitude_ratio_set(struct outCtx *ctx, char *iar_grp, int group);
int irigb_voltage_set(struct outCtx *ctx, char *iv_grp, int group);
int output_board_set(struct outCtx *ctx, struct outsetinfo *ds);
int type_delay_set(struct outCtx *ctx, struct outdelay *od);

int out_2mb_sa_set(struct outCtx *ctx, u8_t *sa);




#endif//__COUT_SET__


