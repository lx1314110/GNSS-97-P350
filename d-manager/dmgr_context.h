#ifndef __DMGR_CONTEXT__
#define	__DMGR_CONTEXT__



#include "alloc.h"








/*
  manager context
*/
struct mgrCtx {
	//loop flag
	bool_t loop_flag;

	//notify flag
	bool_t notify_flag;

	int guard_flag;

	//database handle
	sqlite3 *pDb;
	//veth_info database
	sqlite3 *pVethDb;

	//FPGA file descriptor
	int fpga_fd;

	//PS file descriptor
	int ps_fd;

	//LED
	bool_t led_sta;

	//IPC info
	struct ipcinfo ipc[IPC_CURSOR_ARRAY_SIZE];

	//PID info
	struct pidinfo pid[DAEMON_CURSOR_ARRAY_SIZE];

	//net info
	struct netinfo net;

	//ver info
	struct verinfo ver;
};



#endif//__DMGR_CONTEXT__


