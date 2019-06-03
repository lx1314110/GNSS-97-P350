#ifndef	__COUT_QUERY__
#define	__COUT_QUERY__




#include "cout_context.h"



//may no use it
int in_zone_query(struct outCtx *ctx, int group);

int out_zone_query(struct outCtx *ctx, int group);
int out_baudrate_query(struct outCtx *ctx, int group);
int irigb_amplitude_ratio_query(struct outCtx *ctx, int group);
int irigb_voltage_query(struct outCtx *ctx, int group);

int out_2mb_sa_query(struct outCtx *ctx);
int out_2mb_ssm_query(struct outCtx *ctx);//only query

int port1_query(struct outCtx *ctx, int slot, int fPs);
int port16_query(struct outCtx *ctx);
int type3_delay_query(struct outCtx *ctx);
int type1_delay_query(struct outCtx *ctx, char *type);
int portinfo_query(struct outCtx *ctx, int slot);
int signal_query(struct outCtx *ctx, int slot);
int spt_signal_table_query(struct outCtx *ctx, int slot);







#endif//__COUT_QUERY__


