#ifndef	__DALM_CONTEXT__
#define	__DALM_CONTEXT__





#include "alloc.h"






/*
  alarm context
*/
struct almCtx {
	//loop flag
	bool_t loop_flag;

	//notify flag
	bool_t notify_flag;
	
	struct ntpsta ntpSta;

	//database handle
	sqlite3 *pDb;

	//FPGA file descriptor
	int fpga_fd;

	//alarm tag
	u16_t alm_tag;

	//first Power on 
	u8_t first_flag;

	//board bid: 16+almboard1+pwr2+battary1=20
	u8_t bid[SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1];
	//refsource available state
	struct schemainfo refsrc_state;
	//u8_t refsource_usestate;

	//输入信号门限
	struct inph_thresold InSgnl_Threshold;
	//int InSgnl_Threshold;
	//int Threshold_max;
	//int Threshold_min;

	struct portinfo port_status[V_ETH_MAX];
	//IPC info
	struct ipcinfo ipc[IPC_CURSOR_ARRAY_SIZE];

	struct selectsta select[PORT_ALM];
	struct masksta mask[ALM_CURSOR_ARRAY_SIZE];
	struct alarmsta old_alarm[ALM_CURSOR_ARRAY_SIZE];
	struct alarmsta new_alarm[ALM_CURSOR_ARRAY_SIZE];
	struct tssta ts[ALM_CURSOR_ARRAY_SIZE];

	struct alminfo_t cur_alarm[CUR_ALM_MAX_ARRAY_SIZE];//当前告警查询，新接口
	struct alminfo_t upload_alarm[UPLOAD_ALM_MAX_ARRAY_SIZE];//当前告警主动上报，为snmp，新接口
};












#endif//__DALM_CONTEXT__


