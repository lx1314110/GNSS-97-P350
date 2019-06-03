#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "common.h"

#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_bit.h"
#include "lib_time.h"
#include "lib_log.h"
#include "lib_alarm.h"

#include "addr.h"
#include "alloc.h"
#include "dalm_config.h"
#include "dalm_alarm.h"


/*计数*/
u32_t count_hold = 0;

/*保证每次设置或者清除时间只执行一次*/
static int set_timer_flag = 0;
/*设置定时器*/
void set_refresh_time(int sec)
{
    struct itimerval nvalue;
    nvalue.it_value.tv_sec = sec;
    nvalue.it_value.tv_usec = 0;
    nvalue.it_interval.tv_sec = sec;//每隔sec时间触发一次定时器
    nvalue.it_interval.tv_usec = 0;

	if(set_timer_flag == 0){
		/*此处使用的是ITIMER_REAL，所以对应的是SIGALRM信号*/
		setitimer(ITIMER_REAL, &nvalue, NULL);
		set_timer_flag = 1;
	}
}


/*清除定时器*/
void clear_refresh_time(void)
{
    struct itimerval nvalue;
    nvalue.it_value.tv_sec = 0;
    nvalue.it_value.tv_usec = 0;
    nvalue.it_interval.tv_sec = 0;//每隔sec时间触发一次定时器
    nvalue.it_interval.tv_usec = 0;

	if(set_timer_flag == 1){
		/*清除定时器*/
		setitimer(ITIMER_REAL, &nvalue, NULL);
		/*恢复初始值*/
		count_hold = 0;
		set_timer_flag = 0;
	}
}





bool_t ledAlm(struct almCtx *ctx, u16_t led_sta)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_LED_ALM, led_sta))
	{
		return false;
	}

	return true;
}






bool_t ledCtrl(struct almCtx *ctx)
{
	int i;
	int sum = 0;

	//alarm
	for(i=0; i<ALM_CURSOR_ARRAY_SIZE; i++)
	{
		if(ctx->new_alarm[i].alm_sta && !ctx->mask[i].mask_sta){
			sum++;	
		}
		
	}
	
	if(0 == sum)
	{
		if(!ledAlm(ctx, LED_OFF))
		{
			return false;
		}
	}
	else
	{
		if(!ledAlm(ctx, LED_ON))
		{
			return false;
		}
	}

	return true;
}









bool_t RefreshAlarm(struct almCtx *ctx)
{
	u8_t i;
	u16_t almTag = 0;

	for(i=0; i<PORT_ALM; i++)
	{
		if(ALM_ON == ctx->new_alarm[ctx->select[i].alm_inx].alm_sta)
		{
			almTag |= BIT(7-i);
		}
		else
		{
			almTag &= ~BIT(7-i);
		}

		print(	DBG_DEBUG, 
				"--alarmboard port:%d alm_inx:%d alm_sta:%d",
				i,
				ctx->select[i].alm_inx,
				ctx->new_alarm[ctx->select[i].alm_inx].alm_sta );
	}

	print(	DBG_DEBUG, 
			"--(to fpga)almTag: %X", 
			almTag	);

	if(almTag != ctx->alm_tag)
	{
		ctx->alm_tag = almTag;

		if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_ALM_TAG, 0x00))
		{
			return false;
		}
		
		if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_ALM_TAG, almTag))
		{
			return false;
		}
	}

	return true;
}

int write_clock_state(u16_t state,int phase, struct almCtx *ctx)
{
	struct clock_stainfo clock_sta;
	clock_sta.state = state;
	clock_sta.phase = phase;
	
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");

		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_CLOCK, 
			  sizeof(clock_sta), 
			  (char *)&clock_sta, 
			 sizeof(clock_sta));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");

		return 0;
	}

	return 1;
}

//reserve, fpga no process it
//#define FRESH_TIME_IN_RCS
//#ifdef FRESH_TIME_IN_RCS
#if 0
static int FreshTime_no_leapSecond(int fpga_fd)
{
	/*
	if(!FpgaWrite(fpga_fd, FPGA_LPS, leapSecond))//闰秒刷新
	{
		return 0;
	}*/
	print(	DBG_INFORMATIONAL, 
			"<%s>--%s", 
			gDaemonTbl[DAEMON_CURSOR_ALARM],
			"FreshTime_time in the fast to lock");
	if(!FpgaWrite(fpga_fd, FPGA_S01_FRESH_TIME, 0x0000))//1号槽时间刷新
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S01_FRESH_TIME, 0x0001))
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S02_FRESH_TIME, 0x0000))//2号槽时间刷新
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S02_FRESH_TIME, 0x0001))
	{
		return 0;
	}
	printf("FreshTime: clockstate fast to lock.\n");
	return 1;
}
#endif

#if 0
//move to input process
void real_ClockState(u16_t *clock_state,struct almCtx *ctx)
{
	static u8_t pre_clock_state;//判断FREE-FAST肯定是无HOLD
#if 0	//fpga to process it:2018-02-08
	//if(ctx->satellite_usestate == GET_SATELLITE)
	if(ctx->refsource_usestate == REFSOURCE_VALID)
	{
		clear_refresh_time();
	}
//#define RCS_DEBUG
#ifdef RCS_DEBUG
	printf("refsource_state:%d  count_hold:%d last_clock_state:%d fpga_clock_state:%d\t\t",
	ctx->refsource_usestate,count_hold, pre_clock_state,*clock_state);
#endif
	switch (*clock_state)
	{
		case 0:
			*clock_state = 0;
			s = 0;
			break;
		case 1:
			set_refresh_time(1);
			
			if(ctx->first_flag == FIRST)
			{
				
				*clock_state = 0;
			}
			else if(ctx->refsource_usestate == REFSOURCE_INVALID && count_hold >= 86400)//no any a source
			{				
				*clock_state = 0;
				s = 0;
			}
			else
			{
				*clock_state = 1;
			}
			
			break;
		case 2:
			*clock_state = pre_clock_state;
			break;
		case 3:
		case 4:	
			*clock_state = 3;
			s = 1;
			break;
		default:
			*clock_state = 4;
			break;
	}

	if(s == 0 && *clock_state == 1 && ctx->refsource_usestate == REFSOURCE_VALID)//ctx->satellite_usestate == GET_SATELLITE)
	{
		*clock_state = 3;
	}
#endif
#ifdef FRESH_TIME_IN_RCS
	//reserve, fpga no process it
	if( pre_clock_state == 3 && *clock_state == 4){
		FreshTime_no_leapSecond(ctx->fpga_fd);
	}
#endif	
	pre_clock_state = *clock_state;
#ifdef RCS_DEBUG
	printf("s:%d clock_state:%d\n",s,*clock_state);
#endif	
}
#endif

int read_fresh_state(struct almCtx *ctx)
{
	//u8_t fresh_state=0;
	struct schemainfo schema;

	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");

		return -1;
	}

	#if 0
	shm_read(ctx->ipc[IPC_CURSOR_INPUT].ipc_base, 
			  SHM_OFFSET_FRESH, 
			  sizeof(fresh_state), 
			  (char *)&fresh_state, 
			 sizeof(fresh_state));
	#else
	shm_read(ctx->ipc[IPC_CURSOR_INPUT].ipc_base, 
			  SHM_OFFSET_SCHEMA, 
			  sizeof(struct schemainfo), 
			  (char *)&schema, 
			 sizeof(struct schemainfo));
	#endif

	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");

		return -1;
	}

   
	
	//ctx->satellite_usestate = fresh_state;
	memcpy(&ctx->refsrc_state, &schema, sizeof(struct schemainfo));
	//ctx->refsrc_state = schema.refsrc_is_valid;
	
	

	return 0;
	
}

/*
  1	成功
  0	失败
*/
int CollectEvent(struct almCtx *ctx)
{
	u16_t clock_state = 0;

	int src_type = NO_SOURCE_TYPE,src_slot = 1;
	int src_bid = BID_NONE;
	int event_id = 0;
	src_slot = ctx->refsrc_state.slot;
	src_type = ctx->refsrc_state.type;
	src_bid = ctx->refsrc_state.bid;

	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_STA, (u16_t *)&clock_state))
	{
		print(DBG_ERROR, "--Failed to read clock state.");
		
		return 0;
	}

	switch (src_type){
		case GPS_SOURCE_TYPE:
			//move to proc
			//INIT_EVNTFLAG_BY_SLOT_PORT_GRP(SMP_EVNT_GRP_SATE_CLKSTA, src_slot, SAT_PORT_SAT, 0);
			switch (clock_state){
				case 0:
					event_id = SNMP_EVNT_ID_SAT_FREE;
					break;
				case 4:
					event_id = SNMP_EVNT_ID_SAT_LOCK;
					break;
			}
			//event
			//SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(event_id, src_slot, SAT_PORT_SAT);
			SET_EVNTFLAGVALID_BID_BY_SLOT_PORT_ID(event_id, src_slot, SAT_PORT_SAT, src_bid);
			break;
	}

	//move to porc
	//INIT_EVNTFLAG_BY_SLOT_PORT_GRP(SMP_EVNT_GRP_CLKSTA, SID_CLK_SLOT, SID_CLK_PORT, 0);
	event_id = 0;
	switch (clock_state){
		case 0:
			event_id = SNMP_EVNT_ID_CLK_FREE;
			break;
		case 1:
		case 2:
			event_id = SNMP_EVNT_ID_CLK_HOLD;
			break;
		case 3:
			event_id = SNMP_EVNT_ID_CLK_FAST;
			break;
		case 4:
			event_id = SNMP_EVNT_ID_CLK_LOCK;
			break;
	}
	//event
	SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(event_id, SID_CLK_SLOT, SID_CLK_PORT);

	return 1;
}


/*
  1	成功
  0	失败
*/
int CollectAlarm(struct almCtx *ctx)
{
	int alm_inx = -1;
	int alm_id = 0;
	u16_t ext_pwr_alm = 0;
	u16_t pwr_vol;
	u16_t clock_type;
	u16_t clock_state;
	int sys_phase = ~(0);
	u16_t temp;
	u16_t bidpwr1,bidpwr2;
	
	int i=0;
	struct alarmsta tmp[ALM_CURSOR_ARRAY_SIZE];

	/*
	 *read alarm from the input PROCESS
	 */
	 memset(tmp, 0x0, sizeof(struct alarmsta));
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_read(ctx->ipc[IPC_CURSOR_INPUT].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			  (char *)tmp, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}
	for (i = 0; i < max_alminx_num; ++i){
		if (pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_IN &&
			pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_SAT)
			continue;
		alm_id = pAlmInxTbl[i].alm_id;
		ctx->new_alarm[i].alm_sta = tmp[i].alm_sta;
	}

	//输出进程目前并无任何告警可读
	#if 0
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_read(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			  (char *)tmp, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}
	//取消了时间跳跃告警
	ctx->new_alarm[ALM_CURSOR_TIME_HOPPING -1].alm_sta = tmp[ALM_CURSOR_TIME_HOPPING -1].alm_sta;//时间跳跃告警状态
	#endif

	/*for output or ntp port up/down alarm*/
	int per_ntp_port;
	int per_ntp_slot = 12;	//ntp start slot
	for(i=0;i<20;i++){
		per_ntp_port = i%4+1;//1...4
		per_ntp_slot = 12+i/4;//12-16
		alm_inx = -1;
		if(ctx->bid[per_ntp_slot-1] == BID_NTP ||
		   ctx->bid[per_ntp_slot-1] == BID_NTPF){
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_LINK_DOWN, per_ntp_slot, per_ntp_port, alm_inx);
			if(alm_inx == -1)
				return 0;

			if(ctx->port_status[i].linkstatus == 0 && (ctx->port_status[i].enable == 1) ){
				//event
				//SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_LINK_DOWN, per_ntp_slot, per_ntp_port);
				SET_EVNTFLAGVALID_BID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_LINK_DOWN, per_ntp_slot, per_ntp_port, ctx->bid[per_ntp_slot-1]);
				ctx->new_alarm[alm_inx].alm_sta = ALM_ON;
			}else{
				ctx->new_alarm[alm_inx].alm_sta = ALM_OFF;
	        }
		}		
	}

	/*
	 *ALM PROCESS READ ALARM
	 */
	if(!FpgaRead(ctx->fpga_fd, FPGA_PWR_ALM, &ext_pwr_alm))
	{
		print(DBG_ERROR, "--Failed to read alarm of external power.");
		
		return 0;
	}


	if(!FpgaRead(ctx->fpga_fd, FPGA_INT_PWR_VOL, (u16_t *)&pwr_vol))
	{
		print(DBG_ERROR, "--Failed to read alarm of internal power.");
		
		return 0;
	}


	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_TYP, (u16_t *)&clock_type))
	{
		print(DBG_ERROR, "--Failed to read clock type.");
		
		return 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_STA, (u16_t *)&clock_state))
	{
		print(DBG_ERROR, "--Failed to read clock state.");
		
		return 0;
	}

	if(clock_state >= 3)
	{
		ctx->first_flag = OTHER;
	}
	
	//状态机
	//real_ClockState(&clock_state,ctx);
	SysReadPhase(ctx->fpga_fd, &sys_phase);
	write_clock_state(clock_state,sys_phase,ctx);
		
	if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S18_S19, &temp))
	{
		print(	DBG_ERROR, 
				"--Failed to read FPGA_BID_S18_S19.");
		return 0;
	}
	bidpwr1 = (temp>>8)&0x00FF;//18
	bidpwr2 = temp&0x00FF;//19 slot
	//二号外部电源输入告警状态
	alm_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_EXT_PWR_INPUT, PWR_SLOT_CURSOR_2+1, SID_PWR_PORT, alm_inx);
	if(alm_inx == -1)
		return 0;
	if(!(ext_pwr_alm &BIT(0)) || bidpwr2 == 0x00FF )
	{
		ctx->new_alarm[alm_inx].alm_sta = ALM_OFF;
	}
	else
	{
		ctx->new_alarm[alm_inx].alm_sta = ALM_ON;
	}
	//一号外部电源输入告警状态
	alm_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_EXT_PWR_INPUT, PWR_SLOT_CURSOR_1+1, SID_PWR_PORT, alm_inx);
	if(alm_inx == -1)
		return 0;
	if(!(ext_pwr_alm &BIT(1)) || bidpwr1 == 0x00FF )
	{
		ctx->new_alarm[alm_inx].alm_sta = ALM_OFF;
	}
	else
	{
		ctx->new_alarm[alm_inx].alm_sta = ALM_ON;
	}

	#if 0
	//内部电源输入告警状态
	if(0x1FF == pwr_vol)
	{
		ctx->new_alarm[ALM_CURSOR_INT_PWR1 -1].alm_sta = ALM_ON;
	}
	else
	{
		ctx->new_alarm[ALM_CURSOR_INT_PWR1 -1].alm_sta = ALM_OFF;
	}
	#endif
	//鉴相值/输入信号门限告警
	int sys_phase_abs = 0;
	if (sys_phase < 0){
		sys_phase_abs = -sys_phase;
	}else{
		sys_phase_abs = sys_phase;
	}

	int src_type = NO_SOURCE_TYPE,src_slot = 1;
	int src_bid = BID_NONE;
	src_slot = ctx->refsrc_state.slot;
	src_type = ctx->refsrc_state.type;
	src_bid = ctx->refsrc_state.bid;

	//init thrsd alm
	for (i = 0; i < max_alminx_num; ++i){
		if (pAlmInxTbl[i].alm_id_tbl->alm_type != SMP_SID_SUB_IN)
			continue;
		alm_id = pAlmInxTbl[i].alm_id;
		/*grep -nsr _THRSD alloc/include/alloc_snmp.h*/
		switch (alm_id){
			case SNMP_ALM_ID_GPBD_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_2MHZ_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_2MB_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_IRIGB_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_PTP_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_1PPS_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_1PPS_TOD_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
			case SNMP_ALM_ID_NTP_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
			case SNMP_ALM_ID_10MHZ_THRSD:
				ctx->new_alarm[i].alm_sta = ALM_OFF;
				break;
		}
	}

	//find thrsd alarm
	alm_inx = -1;
	switch (src_type){
		case GPS_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_GPBD_THRSD, src_slot, SAT_PORT_SAT, alm_inx);
			break;
		case S2MH_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MHZ_THRSD, src_slot, SAT_PORT_2MH, alm_inx);
			break;
		case S2MB_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_THRSD, src_slot, SAT_PORT_2MB, alm_inx);
			break;
		case IRIGB1_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_THRSD, src_slot, SAT_PORT_IRIGB1, alm_inx);
			break;
		case IRIGB2_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_THRSD, src_slot, SAT_PORT_IRIGB2, alm_inx);
			break;
		case TOD_SOURCE_TYPE:
			alm_inx = -1;
			switch (src_bid){
				case BID_PTP_IN:
					FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_PTP_THRSD, src_slot, 1, alm_inx);
					break;
				case BID_RTF:
					FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_TOD_THRSD, src_slot, RTF_PORT_1PPS_TOD, alm_inx);
					break;
				default:
					break;
			}
			break;
		case S10MH1_SOURCE_TYPE:
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, src_slot, RTF_PORT_10MH1, alm_inx);
			break;
		case S10MH2_SOURCE_TYPE:
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, src_slot, RTF_PORT_10MH2, alm_inx);
			break;
		case S1PPS_SOURCE_TYPE:
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_IN_LOS, src_slot, RTF_PORT_1PPS, alm_inx);
			break;
		case NTP_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_THRSD, src_slot, 1, alm_inx);
			break;
		case NO_SOURCE_TYPE:
			break;
		default:
			print(DBG_DEBUG, "Not support source type:%d", src_type);
		}

		if(alm_inx >= 0){
			if(sys_phase_abs > ctx->InSgnl_Threshold.Threshold){
				ctx->new_alarm[alm_inx].alm_sta = ALM_ON;
			}else{
				ctx->new_alarm[alm_inx].alm_sta = ALM_OFF;
			}
		}

	//钟控告警状态
	int alm_inx_rb = -1,alm_inx_xo = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_RB_UNLOCK, 1, SID_SYS_PORT, alm_inx_rb);
	if(alm_inx_rb == -1)
		return 0;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_XO_UNLOCK, 1, SID_SYS_PORT, alm_inx_xo);
	if(alm_inx_xo == -1)
		return 0;
		
	if(1 == clock_type)//RB
	{
		if(4 != clock_state)//FREE
		{
			ctx->new_alarm[alm_inx_rb].alm_sta = ALM_ON;
		}
		else
		{
			ctx->new_alarm[alm_inx_rb].alm_sta = ALM_OFF;
		}
		
		ctx->new_alarm[alm_inx_xo].alm_sta = ALM_OFF;
	}
	else if(2 == clock_type)//XO
	{
		if(4 != clock_state)//FREE
		{
			ctx->new_alarm[alm_inx_xo].alm_sta = ALM_ON;
		}
		else
		{
			ctx->new_alarm[alm_inx_xo].alm_sta = ALM_OFF;
		}
		
		ctx->new_alarm[alm_inx_rb].alm_sta = ALM_OFF;
	}
	else
	{
		ctx->new_alarm[alm_inx_rb].alm_sta = ALM_OFF;
		ctx->new_alarm[alm_inx_xo].alm_sta = ALM_OFF;
	}

	//read ntp
	#if 1
	alm_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_EXIT, 1, SID_SYS_PORT, alm_inx);
	if(alm_inx == -1)
		return 0;

	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_NTP].ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_read(ctx->ipc[IPC_CURSOR_NTP].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			  (char *)tmp, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_NTP].ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}
	ctx->new_alarm[alm_inx].alm_sta = tmp[alm_inx].alm_sta;//NTP时间告警状态
	#endif

	alm_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_HW_FPGA_HALT, 1, SID_SYS_PORT, alm_inx);
	if(alm_inx == -1)
		return 0;
	ctx->new_alarm[alm_inx].alm_sta = ALM_OFF;

	//write to share
	return 1;
}

/*=======================当前告警查询处理===========================*/
/*
添加一条告警到当然告警列表，告警缓存为CUR_ALM_MAX_ARRAY_SIZE 50条
  0	成功
  -1 失败
*/
static int set_cur_alarm_info(int alm_inx, struct timeinfo * ti, struct alminfo_t *cur_alarm)
{
	int i;
	static u32_t record_inx = 0;
	//struct cur_alminfo * cur_alarm = ctx->cur_alarm;

	if(NULL == cur_alarm || NULL == ti)
		return -1;
	
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		if(ALM_OFF == cur_alarm[i].alm_sta){
			cur_alarm[i].id = ++record_inx;//1-0xffffffff
			cur_alarm[i].alm_inx = alm_inx;
			cur_alarm[i].alm_sta = ALM_ON;
			memcpy(&cur_alarm[i].ti, ti, sizeof(struct timeinfo));
			if(record_inx >= (u32_t)~0)
				record_inx = 0;
			break;
		}
	}
	if(i >= CUR_ALM_MAX_ARRAY_SIZE){
		print(	DBG_ERROR, "cur alarm list is full!");//目前不会发生告警大于50条的情况
	}
	return 0;
}

/*
从当然告警列表删除一条告警，告警缓存为CUR_ALM_MAX_ARRAY_SIZE 50条
  0	成功
  -1 失败
*/
static int del_cur_alarm_info(int alm_inx, struct timeinfo * ti, struct alminfo_t *cur_alarm)
{
	int i;
	//static int record_inx = 0;
	//struct cur_alminfo * cur_alarm = ctx->cur_alarm;

	if(NULL == cur_alarm || NULL == ti)
		return -1;
	
	for (i = 0; i < CUR_ALM_MAX_ARRAY_SIZE; ++i){
		if(ALM_ON == cur_alarm[i].alm_sta && alm_inx == cur_alarm[i].alm_inx){
			cur_alarm[i].id = 0;
			cur_alarm[i].alm_inx = 0;
			cur_alarm[i].alm_sta = ALM_OFF;
			//memcpy(&cur_alarm[i].ti, ti, sizeof(sturct timeinfo));
			memset(&cur_alarm[i].ti, 0, sizeof(struct timeinfo));
			break;
		}
	}
	return 0;
}

/*================告警主动上报处理================*/
/*
添加一条告警产生/告警消失记录到当然列表，告警缓存为UPLOAD_ALM_MAX_ARRAY_SIZE 20条
  0	成功
  1 成功，id已经到达最大，需要用户立即同步
  -1 失败
*/
static int set_upload_alarm_info(int alm_inx, int alm_sta, struct timeinfo * ti, struct alminfo_t *upload_alarm)
{
	int i;
	int cur_inx = 0, cur_minid_inx = 0;
	//int found = 0;
	u32_t min_id = ~0;
	static u32_t record_inx = 0;
	//struct cur_alminfo * cur_alarm = ctx->cur_alarm;
	//printf("############0x%08x\n", min_id);
	if(NULL == upload_alarm || NULL == ti)
		return -1;

	for (i = 0; i < UPLOAD_ALM_MAX_ARRAY_SIZE; ++i){
		if(0 == upload_alarm[i].id){
			//查找有空位的话就插入
			cur_inx = i;
			//printf("konginsert:%d\n", cur_inx);
			goto insert_record;
			//break;
		} else {
			//列表满了，没有空位，就从整个列表中找id最小值的索引
			if(min_id > upload_alarm[i].id){
				//printf("%d-$$$$$$$$$$$$$0x%08x, 0x%08x\n", i, min_id, upload_alarm[i].id);
				min_id = upload_alarm[i].id;
				cur_minid_inx = i;
			}
		}
	}
	cur_inx = cur_minid_inx;

insert_record:
	//found = 1;//又没有空位，又找不到最小id，则覆盖第一条数据，当然这种情况不会发生
	upload_alarm[cur_inx].id = ++record_inx;//1-0xffffffff
	upload_alarm[cur_inx].alm_inx = alm_inx;
	upload_alarm[cur_inx].alm_sta = alm_sta;
	memcpy(&upload_alarm[cur_inx].ti, ti, sizeof(struct timeinfo));

	print(DBG_DEBUG, "[%d] id:%d alm_id:%d alm_sta:%d \
	%04d-%02d-%02d %02d:%02d:%02d\n", cur_inx, 
	upload_alarm[cur_inx].id,
	upload_alarm[cur_inx].alm_inx,
	upload_alarm[cur_inx].alm_sta,
	ti->year, ti->month, ti->day, ti->hour, ti->minute, ti->second);
	if(cur_inx >= UPLOAD_ALM_MAX_ARRAY_SIZE){
		print(	DBG_ERROR, "upload alarm list is full!");//不会发生
	}

	if(record_inx >= (u32_t)~0 ){
		record_inx = 0;
		//此时需要立刻将消息导入到共享内存后，进行清0操作
		return 1;
	}
	return 0;
}


/*
  1	成功
  0	失败
*/
int CompareAlarm(struct almCtx *ctx)
{
	int ret = -1;
	int i;
	struct timeinfo ti;
	char buf[256];
	
	for(i=0; i<ALM_CURSOR_ARRAY_SIZE; i++)
	{
		if((ALM_OFF == ctx->old_alarm[i].alm_sta) && (ALM_ON == ctx->new_alarm[i].alm_sta))
		{
			if(0 == GetSysTime(&ti))
			{
				return 0;
			}
			sprintf((char *)(ctx->ts[i].alm_start_time), 
					"%04d-%02d-%02d %02d:%02d:%02d",
					ti.year, ti.month, ti.day, ti.hour, ti.minute, ti.second);
			print(	DBG_INFORMATIONAL, 
					"--[alm_inx:%d] alm_start_time: %s",
					i, ctx->ts[i].alm_start_time);

			if(-1 == set_cur_alarm_info(i, &ti, ctx->cur_alarm)){
				print(	DBG_ERROR,
					"--set_cur_alarm_info error.");
			}
			ret = set_upload_alarm_info(i, ctx->new_alarm[i].alm_sta, &ti, ctx->upload_alarm);
			if(-1 == ret){
				print(	DBG_ERROR,
					"--set_upload_alarm_info error.");
			} else if(1 == ret) {
				//record_id已到达最大，把告警立即写入共享内存, 并清理
				if(0 == WriteAlarmInfoToShareMemory(ctx)){
					print(	DBG_ERROR, "--WriteAlarmInfoToShareMemory.");
					return 0;
				}
				sleep(1);
				memset(ctx->upload_alarm, 0x0, UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
			}
			memset(buf, 0, sizeof(buf));
			//index
			sprintf(buf, "2|%d\\%s", i,gAlarmStatusTbl[ALM_ON]);
			sleep(1);
			if(-1 == send_log_message(buf))//for gw, should use the slot,port,alm_id
			{
				print(	DBG_ERROR,
					"--Send log message to dwg error.");
			}
			
			//log日志
			//------------------------------------------------------------------------------------
			memset(buf, 0, sizeof(buf));

			switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
			case SMP_SID_SUB_IN:
			case SMP_SID_SUB_OUT:
				sprintf(buf, "Slot:%d Port:%d %s %s", pAlmInxTbl[i].slot,
					pAlmInxTbl[i].port,
					pAlmInxTbl[i].alm_id_tbl->alm_msg,
					gAlarmStatusTbl[ALM_ON]);
			break;
			case SMP_SID_SUB_SYS:
				sprintf(buf, "SYS: %s %s",
					pAlmInxTbl[i].alm_id_tbl->alm_msg,
					gAlarmStatusTbl[ALM_ON]);
			break;
			case SMP_SID_SUB_PWR:
				sprintf(buf, "#%d %s %s", pAlmInxTbl[i].slot,
						pAlmInxTbl[i].alm_id_tbl->alm_msg,
						gAlarmStatusTbl[ALM_ON]);
			break;
			case SMP_SID_SUB_SAT:
				sprintf(buf, "#%d %s %s", pAlmInxTbl[i].slot,
						pAlmInxTbl[i].alm_id_tbl->alm_msg,
						gAlarmStatusTbl[ALM_ON]);
			break;
			}
			
			
			SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_ALM_GENERATE, SID_SYS_SLOT, SID_SYS_PORT);
			if(0 == db_log_write(ctx->pDb, TBL_LOG, buf))
			{
				return 0;
			}
			//------------------------------------------------------------------------------------
		}
		else if((ALM_ON == ctx->old_alarm[i].alm_sta) && (ALM_OFF == ctx->new_alarm[i].alm_sta))
		{
			if(0 == GetSysTime(&ti))
			{
				return 0;
			}
			sprintf((char *)(ctx->ts[i].alm_end_time), 
					"%04d-%02d-%02d %02d:%02d:%02d",
					ti.year, ti.month, ti.day, ti.hour, ti.minute, ti.second);
			print(	DBG_INFORMATIONAL, 
					"--[alm_inx:%d] alm_end_time: %s",
					i, ctx->ts[i].alm_end_time);
			if(-1 == del_cur_alarm_info(i, &ti, ctx->cur_alarm)){
				print(	DBG_ERROR,
					"--set_cur_alarm_info error.");
			}
			ret = set_upload_alarm_info(i, ctx->new_alarm[i].alm_sta, &ti, ctx->upload_alarm);
			if(-1 == ret ){
				print(	DBG_ERROR,
					"--set_upload_alarm_info error.");
			}else if(1 == ret){
				//record_id已到达最大，把告警立即写入共享内存
				if(0 == WriteAlarmInfoToShareMemory(ctx)){
					print(	DBG_ERROR, "--WriteAlarmInfoToShareMemory.");
					return 0;
				}
				memset(ctx->upload_alarm, 0x0, UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
			}
					
			//告警历史
			if(0 == db_alarm_write(ctx->pDb, TBL_ALARM_HISTORY, i, (char *)ctx->ts[i].alm_start_time, (char *)ctx->ts[i].alm_end_time))
			{
				return 0;
			}
			
			memset(buf, 0, sizeof(buf));			
			sprintf(buf, "2|%d\\%s", i,gAlarmStatusTbl[ALM_OFF]);
			if(-1 == send_log_message(buf)) //for gw, should use the slot,port,alm_id
			{
				print(	DBG_ERROR,
					"Send log message to dwg error.");
			}
			
			//log
			//------------------------------------------------------------------------------------
			memset(buf, 0, sizeof(buf));
			switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
			case SMP_SID_SUB_IN:
			case SMP_SID_SUB_OUT:
				sprintf(buf, "Slot:%d Port:%d %s %s", pAlmInxTbl[i].slot,
					pAlmInxTbl[i].port,
					pAlmInxTbl[i].alm_id_tbl->alm_msg,
					gAlarmStatusTbl[ALM_OFF]);
			break;
			case SMP_SID_SUB_SYS:
				sprintf(buf, "SYS: %s %s",
					pAlmInxTbl[i].alm_id_tbl->alm_msg,
					gAlarmStatusTbl[ALM_OFF]);
			break;
			case SMP_SID_SUB_PWR:
				sprintf(buf, "#%d %s %s", pAlmInxTbl[i].slot,
						pAlmInxTbl[i].alm_id_tbl->alm_msg,
						gAlarmStatusTbl[ALM_OFF]);
			break;
			case SMP_SID_SUB_SAT:
				sprintf(buf, "#%d %s %s", pAlmInxTbl[i].slot,
						pAlmInxTbl[i].alm_id_tbl->alm_msg,
						gAlarmStatusTbl[ALM_OFF]);
			break;
			}

			SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_ALM_DISAPPEAR, SID_SYS_SLOT, SID_SYS_PORT);
			if(0 == db_log_write(ctx->pDb, TBL_LOG, buf))
			{
				return 0;
			}
			//------------------------------------------------------------------------------------
		}
		else
		{
			//do nothing
		}
	}
	
	memcpy(ctx->old_alarm, ctx->new_alarm, ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));

	return 1;
}

/*
  1	成功
  0	失败
*/
int CollectAlarm_FPGAInvalid(struct almCtx *ctx)
{
	int alm_inx = -1;
	struct alarmsta tmp[ALM_CURSOR_ARRAY_SIZE];
	//read
	#if 1
	alm_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_EXIT, 1, SID_SYS_PORT, alm_inx);
	if(alm_inx == -1)
		return 0;

	memset(tmp, 0x0, sizeof(struct alarmsta));
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_NTP].ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return 0;
	}
	shm_read(ctx->ipc[IPC_CURSOR_NTP].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta), 
			  (char *)tmp, 
			  ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_NTP].ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return 0;
	}

	ctx->new_alarm[alm_inx].alm_sta = tmp[alm_inx].alm_sta;//NTP时间告警状态
	#endif

	alm_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_HW_FPGA_HALT, 1, SID_SYS_PORT, alm_inx);
	if(alm_inx == -1)
		return 0;
	ctx->new_alarm[alm_inx].alm_sta = ALM_ON;

	//write to share

	return 1;
}

