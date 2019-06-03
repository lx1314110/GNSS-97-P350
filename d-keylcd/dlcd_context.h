#ifndef	__DLCD_CONTEXT__
#define	__DLCD_CONTEXT__



#include "alloc.h"



/*
  output context
*/
struct lcdCtx {
	//loop flag
	bool_t loop_flag;

	//notify flag
	bool_t notify_flag;

	//power flag
	bool_t power_flag;

	//database handle
	sqlite3 *pDb;

	//FPGA file descriptor
	int fpga_fd;

	//FPGA int file descriptor
	int int_fd;

	//gpio_ps file descriptor
	int ps_fd;

	

};





#endif//__DLCD_CONTEXT__


