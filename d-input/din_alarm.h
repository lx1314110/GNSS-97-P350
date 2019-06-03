#ifndef	__DIN_ALARM__
#define	__DIN_ALARM__





#include "din_context.h"







bool_t ledBd(struct inCtx *ctx, u16_t led_sta);
bool_t ledGps(struct inCtx *ctx, u16_t led_sta);
bool_t ledIrigb(struct inCtx *ctx, u16_t led_sta);
bool_t ledFreq(struct inCtx *ctx, u16_t led_sta);
bool_t ledCtrl(struct inCtx *ctx);

int ReadAlarm(struct inCtx *ctx);

void close_unrelated_alm(struct inCtx *ctx);








#endif//__DIN_ALARM__


