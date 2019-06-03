#ifndef	__CALM_CONTEXT__
#define	__CALM_CONTEXT__





#include "alloc.h"






struct almCtx {
	int fpga_fd;
	u8_t bid[SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1];
	u8_t bid_alm_num[SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1];
	sqlite3 *pDb;
	struct ipcinfo ipc;
	struct pidinfo pid;
	struct alminfo_t cur_alarm[CUR_ALM_MAX_ARRAY_SIZE];//new current alarm query interface
	struct alminfo_t upload_alarm[UPLOAD_ALM_MAX_ARRAY_SIZE];//new current alarm upload interface
};







#endif//__CALM_CONTEXT__


