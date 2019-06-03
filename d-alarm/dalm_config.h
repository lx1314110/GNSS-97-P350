#ifndef	__DALM_CONFIG__
#define	__DALM_CONFIG__





#include "alloc.h"
#include "dalm_context.h"

int ReadBidFromFpga(struct almCtx * ctx);

int WriteAlarmToShareMemory(struct almCtx *ctx);
int WriteAlarmInfoToShareMemory(struct almCtx *ctx);
int WriteMaskToShareMemory(struct almCtx *ctx);
int WriteSelectToShareMemory(struct almCtx *ctx);
int WriteSignalThresholdToShareMemory(struct almCtx *ctx);

int WriteAlarmSelectToTable(struct almCtx *ctx, struct selectinfo *cfg, char *pTbl);

int FlushAlarmMaskTable(struct almCtx *ctx, char *pTbl);
int WriteAlarmMaskToTable(struct almCtx *ctx, struct maskinfo *cfg, char *pTbl);
int WriteSignalThresholdToTable(struct almCtx *ctx, struct inph_thresold *cfg, char *pTbl);

int ReadAlarmSelectFromTable(struct almCtx *ctx, char *pTbl);
int ReadAlarmMaskFromTable(struct almCtx *ctx, char *pTbl);
int ReadSignalThresholdFromTable(struct almCtx *ctx, char *pTbl);


int WriteHistoryAlarmToTable(struct almCtx *ctx, char *pTbl, u8_t alm_id, struct tssta *ts);
int ReadHistoryAlarmCountFromTable(struct almCtx *ctx, char *pTbl);
int TruncateHistoryAlarm(struct almCtx *ctx, char *pTbl);
int WriteEnableToShareMemory(struct almCtx *ctx);


#ifdef NEW_ALARMID_COMM
//no need the TBL_ALARM_LIST
#else
int FlushAlarmListTable(struct almCtx *ctx, char *pTbl);
#endif










#endif//__DALM_CONFIG__


