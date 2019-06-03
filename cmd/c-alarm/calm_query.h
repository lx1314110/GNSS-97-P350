#ifndef	__CALM_QUERY__
#define	__CALM_QUERY__





#include "calm_context.h"





int alarm_mask_query(struct almCtx *ctx, u32_t alm_inx);
int alarm_select_query(struct almCtx *ctx, u8_t port);
int alarm_inphase_threshold_query(struct almCtx *ctx);// int signal

int current_alarm_list_query(struct almCtx *ctx);
int current_alarm_query(struct almCtx *ctx);//1page
int current_alarm_nums(struct almCtx *ctx);

int upload_alarm_query(struct almCtx *ctx);//1page
int upload_alarm_nums(struct almCtx *ctx);


int bd_alarm_sta_query(struct almCtx *ctx);











#endif//__CALM_QUERY__


