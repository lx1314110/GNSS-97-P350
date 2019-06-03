#ifndef	__DNTP_SERVER__
#define	__DNTP_SERVER__







#include "dntp_context.h"





extern u8_t li;

bool_t isRunning(struct ntpCtx *ctx);


bool_t initializeSocket(struct ntpCtx *ctx);


bool_t VnBroadcast(int soc, u8_t Vn, struct recvaddr_t *bcast_addr);

bool_t VaBroadcast(struct port_info *pInfo);


int virtual_ethn_add(struct ntpCtx *ctx);

struct ntpreq * reqReceive(int sid,struct port_info *info);
void reqResponse(void *args);
int ProcNtp(struct ntpCtx *ctx);














#endif//__DNTP_SERVER__


