#ifndef	__DOUT_CONTEXT__
#define	__DOUT_CONTEXT__




#include "lib_time.h"
#include "alloc.h"





struct srcinfo {
	u8_t time_zone;
	struct timeinfo timeSrc;
};




/*
  output context
*/
struct outCtx {
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

	//INT file descriptor
	int int_fd;

	//PPS count
	int pps_count;

	//thread identifier
	pthread_t thread_id;

	u16_t old_ref_source;
	u16_t new_ref_source;

	//old board identifier
	u8_t oldBid[SLOT_CURSOR_ARRAY_SIZE];

	struct ipcinfo ipc[IPC_CURSOR_ARRAY_SIZE];
	struct out2mbsainfo out_sa;//out 2mb sa of ssm
	struct inzoneinfo in_zone;
	struct outzoneinfo out_zone;
	struct baudrate out_br;
	struct amplitude irigb_amp;
	struct voltage irigb_vol;
	struct leapinfo lp;
	struct srcinfo sys_time;
	struct srcinfo src_time;
	struct outdelay od[OUTDELAY_SIGNALTYPE_LEN];
	struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];
	struct outputsta outSta[SLOT_CURSOR_ARRAY_SIZE];
};





#endif//__DOUT_CONTEXT__


