#ifndef	__DIN_SATELLITE__
#define	__DIN_SATELLITE__




#include "lib_type.h"
#include "din_context.h"


#if 0
void write_fresh_state(u8_t state,struct inCtx *ctx);
#endif
int SetSatellite(struct inCtx *ctx);



int lola_elev_set(int fpga_fd, int slot, u8_t *lon, u8_t *lat, u8_t *elev);
int mode_set(int fpga_fd, int slot, u8_t mode);
int GGA_Parser(struct inCtx *ctx, int slot, u8_t *data);
int RMC_Parser(struct inCtx *ctx, int slot, u8_t *data);
int CFG_Parser(struct inCtx *ctx, int slot, u8_t *data);
int GSV_Parser(struct inCtx *, int, u8_t *);
int BSV_Parser(struct inCtx *ctx, int slot, u8_t *data);
int LPS_Parser(struct inCtx *ctx, int slot, u8_t *data);
int ReadSatellite(struct inCtx *ctx);











#endif//__DIN_SATELLITE__


