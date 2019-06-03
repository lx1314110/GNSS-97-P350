#ifndef	__D61850_CONTEXT__
#define	__D61850_CONTEXT__





#include "alloc.h"






/*
  iec61850 context
*/
struct iec61850_Ctx {
	//loop flag
	bool_t loop_flag;

	//notify flag
	bool_t notify_flag;

	//database handle
	sqlite3 *pDb;
	
	//FPGA file descriptor
	int fpga_fd;

	//serial file descriptor
	int serial_fd;

	struct ipcinfo ipc[IPC_CURSOR_ARRAY_SIZE];
	
};












#endif


