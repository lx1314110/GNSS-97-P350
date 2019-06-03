#ifndef	__CIN_QUERY__
#define	__CIN_QUERY__




#include "cin_context.h"




int portinfo_query(struct inCtx *ctx, int slot);
int port_srctype_query(struct inCtx *ctx, int slot);
int schema_query(struct inCtx *ctx);
int schema_list_query(struct inCtx *ctx);
int schema_cur_valid_list_query(struct inCtx *ctx);

int prio_query(struct inCtx *ctx, int slot, int port);
int delay_query(struct inCtx *ctx, int slot, int port);
int ssm_query(struct inCtx *ctx, int slot, int port);
int sa_query(struct inCtx *ctx, int slot, int port);
int signal_query(struct inCtx *ctx, int slot);
int mode_query(struct inCtx *ctx, int slot);
int elev_query(struct inCtx *ctx, int slot);
int lola_query(struct inCtx *ctx, int slot);
int port_query(struct inCtx *ctx, int slot, int port);
int gps_bd_query(struct inCtx *ctx);


//ptp in query
int input_ptpin_board_query(struct inCtx *ctx, int slot);





#endif//__CIN_QUERY__


