#ifndef	__DIN_CONTEXT__
#define	__DIN_CONTEXT__




#include "alloc.h"








#define		GB_BUF_LEN		2048







/*
  input context
*/
struct inCtx {
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

	//old board identifier
	u8_t oldBid[INPUT_SLOT_CURSOR_ARRAY_SIZE];

	//pps
	u8_t pps_count;
	bool_t pps_flag;

	//gps leap forecast
	//u8_t gps_leap_forecast;

	struct sys_conf_t sys_conf;//some setvalue configure.phasePerformance upload switch；1-enable 0-disable
	//clock type
	u16_t clock_type;//1-RB; 2-XO
	struct leapinfo lp_old;//prev leapinfo
	struct leapinfo lp_new;//default from satellite
	struct leapinfo lp_set;//default from set

	//thread identifier
	pthread_t thread_id;

	struct pidinfo pid;
	struct ipcinfo ipc;
	struct schemainfo cs;
	//old
	struct alarmsta alarm[ALM_CURSOR_ARRAY_SIZE];

	//struct alm_inx_sta *alarm_used;
	//u32_t alarm_num;
	struct inputsta inSta[INPUT_SLOT_CURSOR_ARRAY_SIZE];

	u8_t * satelliteBuffer;
};






#endif//__DIN_CONTEXT__



