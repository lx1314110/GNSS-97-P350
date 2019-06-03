#ifndef	__DNTP_PARSER__
#define	__DNTP_PARSER__






#include "dntp_context.h"






int WriteEnableToShareMemory(struct ntpCtx *ctx);
int WriteAlarmToShareMemory(struct ntpCtx *ctx);

bool_t isNotify(struct ntpCtx *ctx);
int ReadMessageQueue(struct ntpCtx *ctx);

int mod_log_level(char *data, struct ntpCtx *ctx);

int add_md5_key(char *data, struct ntpCtx *ctx);
int del_md5_key(char *data, struct ntpCtx *ctx);
int mod_md5_key(char *data, struct ntpCtx *ctx);

int mod_broadcast_enable_interval(char *data, struct ntpCtx *ctx);
int mod_version_enable(char *data, struct ntpCtx *ctx);
int mod_md5_enable(char *data, struct ntpCtx *ctx);

int sysVport(char *data, struct ntpCtx *ctx);

int writeConfig(struct ntpCtx *ctx);













#endif//__DNTP_PARSER__


