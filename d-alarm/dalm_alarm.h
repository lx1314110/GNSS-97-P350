#ifndef	__DALM_ALARM__
#define	__DALM_ALARM__






#include "dalm_context.h"







bool_t ledAlm(struct almCtx *ctx, u16_t led_sta);
bool_t ledCtrl(struct almCtx *ctx);

bool_t RefreshAlarm(struct almCtx *ctx);

int write_clock_state(u16_t state, int phase, struct almCtx *ctx);


int read_fresh_state(struct almCtx *ctx);

int CollectAlarm(struct almCtx *ctx);
int CompareAlarm(struct almCtx *ctx);
int CollectAlarm_FPGAInvalid(struct almCtx *ctx);
int CollectEvent(struct almCtx *ctx);










#endif//__DALM_ALARM__


