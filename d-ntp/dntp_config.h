#ifndef	__DNTP_CONFIG__
#define	__DNTP_CONFIG__






#include "dntp_context.h"


#define TIME_USE_FPGA_TIME 






int md5_key_read(struct ntpCtx *ctx, char *tbl);
int md5_key_add(struct ntpCtx *ctx, struct ntpmk *mk, char *tbl);
int md5_key_del(struct ntpCtx *ctx, struct ntpmk *mk, char *tbl);
int md5_key_mod(struct ntpCtx *ctx, struct ntpmk *mk, char *tbl);

int version_enable_read(struct ntpCtx *ctx, char *tbl);
int version_enable_write(struct ntpCtx *ctx, struct ntpve *ve, char *tbl);

int broadcast_enable_interval_read(struct ntpCtx *ctx, char *tbl);
int broadcast_enable_interval_write(struct ntpCtx *ctx, struct ntpbc *bc, char *tbl);

int md5_enable_read(struct ntpCtx *ctx, char *tbl);
int md5_enable_write(struct ntpCtx *ctx, struct ntpme *me, char *tbl);

int read_virtual_eth(struct ntpCtx *ctx, char *tbl);
int read_eth(struct ntpCtx *ctx, char *tbl,u8_t v_port);
int write_virtual_eth(struct ntpCtx *ctx, struct veth *vp, char *tbl);
int ReadLeapsecond(struct ntpCtx *ctx, char *pTbl);











#endif//__DNTP_CONFIG__


