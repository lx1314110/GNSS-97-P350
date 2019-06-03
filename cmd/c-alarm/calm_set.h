#ifndef	__CALM_SET__
#define	__CALM_SET__






#include "calm_context.h"






int alarm_mask_set(struct almCtx *ctx, u32_t alm_inx, u8_t mask_sta);
int alarm_select_set(struct almCtx *ctx, u8_t port, u32_t alm_inx);
int alarm_inphase_threshold_set(struct almCtx *ctx, int threshold);












#endif//__CALM_SET__


