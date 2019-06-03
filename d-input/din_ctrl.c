#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_msgQ.h"
#include "lib_sqlite.h"
#include "lib_log.h"
#include "lib_bit.h"


#include "common.h"
#include "addr.h"
#include "din_issue.h"
#include "din_alloc.h"
#include "din_parser.h"
#include "din_ssm.h"
#include "din_phase.h"
#include "din_config.h"
#include "din_source.h"
#include "din_alarm.h"
#include "din_satellite.h"
#include "din_int.h"
#include "din_ctrl.h"



static unsigned int pps_phaseperf = 0;


/*
  true:		程序运行
  false:	程序退出
*/
bool_t isRunning(struct inCtx *ctx)
{
	return ctx->loop_flag;
}

int initializeStavalid(struct inCtx *ctx)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_VALID, 0x01))//设置被选信号不可用
	{
		return -1;
	}
	return 0;
}

//发生闰秒的时间
#if 0
static int SateCheckLeapforecastTime(struct inCtx *ctx)
{
	struct timeinfo sys_time;
	struct timeinfo gp_forecast_time;
	memcpy(&gp_forecast_time, &ctx->lp_new.leaptime, sizeof(struct timeinfo));
	
	//-59s
	if(gp_forecast_time.second == 60)
		gp_forecast_time.second = 1;
	if(gp_forecast_time.second == 58){
		gp_forecast_time.second = 59;
		gp_forecast_time.minute = gp_forecast_time.minute-1; 
	}

	
	GetUtcSysTime(&sys_time);
	if( (sys_time.second == gp_forecast_time.second) &&
		sys_time.year == gp_forecast_time.year &&
		sys_time.month == gp_forecast_time.month &&
		sys_time.day == gp_forecast_time.day &&
		sys_time.hour == gp_forecast_time.hour &&
		sys_time.minute == gp_forecast_time.minute )
		ctx->gps_leap_forecast = 1;
	return 0;
}
#endif

void Report_TimesourceToWg(struct inCtx *ctx)
{
	int i;
	int satellite_flag = 0;
	static u8_t new_timesource[5],old_timesource[5];
	char buf[256];
	static u8_t gps_num[INPUT_SLOT_CURSOR_ARRAY_SIZE],bd_num[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	static u8_t first_getsatellite = 0;
	static u8_t satellite_count = 0;

	//init first
	if(first_getsatellite != 1)
		memcpy(new_timesource,"NONE\0",5);

	for(i = INPUT_SLOT_CURSOR_1; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		if(GET_SATELLITE == ctx->inSta[i].satellite_valid){

			first_getsatellite = 1;
			satellite_flag = 1;
			switch(ctx->inSta[i].satcommon.satcomminfo.sysMode)
			{
				case MODE_CURSOR_GPS_VAL:
				case MODE_CURSOR_MIX_GPS_VAL:
					gps_num[i]++;
					if(gps_num[i] > 9)//Satellite jitter prevention 
					{
						memcpy(new_timesource,"GPS\0",4);
										
					}
					bd_num[i] = 0;
					break;
				case MODE_CURSOR_BD_VAL:
				case MODE_CURSOR_MIX_BD_VAL:
					bd_num[i]++;
					if(bd_num[i] > 9)
					{
						memcpy(new_timesource,"BD\0",3);
						
					}	
					gps_num[i] = 0;
					break;
				
			}
		}
	}
	
	if(satellite_flag != 1)
	{
		satellite_count++;
		if(satellite_count > 9){
			memcpy(new_timesource,"NONE\0",5);
			satellite_count = 0;
		}
	}else{
		//no signal
		satellite_count = 0;
	}


	//printf("timesource:%s  %d\n",new_timesource,ctx->inSta[0].gbdbe.gbdbe.sysMode);

	if((0 !=  memcmp(new_timesource,old_timesource,sizeof(new_timesource))) && (first_getsatellite == 1))
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "3|%s", new_timesource);

		if(-1 == send_log_message(buf))
		{
			print(	DBG_DEBUG,
					"Send log message to dwg error.");
			return;
		}
	}
	memcpy(old_timesource,new_timesource,sizeof(new_timesource));

}


/*
  1	成功
  0	失败
*/
int ReadBidFromFpga(struct inCtx *ctx)
{
	u16_t temp = 0;
	int i;

#define TEMP_LEN (INPUT_SLOT_CURSOR_ARRAY_SIZE/2+INPUT_SLOT_CURSOR_ARRAY_SIZE%2)
		int j;
	for(i=INPUT_SLOT_CURSOR_1,j=0; (i<INPUT_SLOT_CURSOR_ARRAY_SIZE)&&(j<TEMP_LEN); i+=2,j++){
		if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S01_TO_S16(j), &temp)){
			return 0;
		}
		ctx->inSta[i].boardId = (temp>>8) &0x00FF;
		ctx->inSta[i+1].boardId = temp &0x00FF;
		//if(0xFF == ctx->inSta[i].boardId)
		if(BID_ARRAY_SIZE <= ctx->inSta[i].boardId)
			ctx->inSta[i].boardId = 0x00;
		//if(0xFF == ctx->inSta[i+1].boardId)
		if(BID_ARRAY_SIZE <= ctx->inSta[i+1].boardId)
			ctx->inSta[i+1].boardId = 0x00;
		
		if ( ctx->inSta[i].boardId  < BID_ARRAY_SIZE){
			print(	DBG_DEBUG, 
			"--#%d %d %s.",
			i+1,ctx->inSta[i].boardId,
			gBoardNameTbl[ctx->inSta[i].boardId]);
		}else{
			print(	DBG_ERROR, 
			"--#%d Unidentification board %d.",
			i+1,ctx->inSta[i].boardId);

		}
		if ( ctx->inSta[i+1].boardId  < BID_ARRAY_SIZE){
			print(	DBG_DEBUG, 
			"--#%d %d %s.",
			i+2,ctx->inSta[i+1].boardId,
			gBoardNameTbl[ctx->inSta[i+1].boardId]);
		}else{
			print(	DBG_ERROR, 
			"--#%d Unidentification board %d.",
			i+2,ctx->inSta[i+1].boardId);
		}
	}
	
#ifdef NEW_ALARMID_COMM
#else
	for (i = INPUT_SLOT_CURSOR_1; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		switch (ctx->inSta[i].boardId){
			case BID_INSATE_LOWER ... BID_INSATE_UPPER:
				FLUSH_GALARMTBL_BID(i+1, ctx->inSta[i].boardId, BID_GPSBF);
				break;
			}
	}
#endif
	return 1;
}


static void FlushTimeClockStateToLock(struct inCtx *ctx)
{
	u16_t clock_state = 0;
	static u8_t pre_clock_state;//判断FREE-FAST肯定是无HOLD

	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_STA, (u16_t *)&clock_state))
	{
		print(DBG_ERROR, "--Failed to read clock state.");
		
		return;
	}

	//reserve, fpga no process it
	if( pre_clock_state == 3 && clock_state == 4){
		print(DBG_INFORMATIONAL, "Clockstate fast to lock:");
		FreshTime_no_leapSecond(ctx);
	}
	pre_clock_state = clock_state;
#ifdef RCS_DEBUG
	printf("s:%d clock_state:%d\n",s,*clock_state);
#endif	
}

static int Time_Refresh_Strategy(struct inCtx *ctx)
{
	u16_t ptp_leap = 0;
	int irigb_leap = 0;
	static u8_t leapForecast_old = 0,leapForecast_new = 0;
	u8_t port_alm = ALM_ON;
	static int alm_num,unalm_num;
	struct leapinfo real_lp;
	u16_t tmp;
	int tsrc_type = NO_SOURCE_TYPE;
	int tsrc_slot = INPUT_SLOT_CURSOR_1+1;
	int tsrc_bid = BID_NONE;
	static u8_t schema = 0, old_schema = 0;
	//检查选源是否改变
	schema = ctx->cs.schema;
	tsrc_type = ctx->cs.type;
	tsrc_slot = ctx->cs.slot;
	tsrc_bid = ctx->cs.bid;
	memset(&real_lp,0,sizeof(real_lp));

	//set default value, 否则在无源时，闰秒可能会被修改为0
	real_lp.leapMode = ctx->lp_set.leapMode;
	real_lp.leapSecond = ctx->lp_old.leapSecond;	
	//闰秒模式 1为强制刷新，0为自动刷新

	if (tsrc_type == IRIGB1_SOURCE_TYPE){
		FpgaRead(ctx->fpga_fd, FPGA_IN_IRIGB1_LPS(tsrc_slot), &tmp);
		leapForecast_new = tmp&BIT(0);
	}
	if (tsrc_type == IRIGB2_SOURCE_TYPE){
		FpgaRead(ctx->fpga_fd, FPGA_IN_IRIGB1_LPS(tsrc_slot), &tmp);
		leapForecast_new = tmp&BIT(0);
	}
	if( leapForecast_new == 0 && leapForecast_old == 1 )
	{
		irigb_leap = 1;
		leapForecast_old = leapForecast_new;
	}


	if(1 == ctx->lp_set.leapMode)
	{
		
		if( irigb_leap == 1 )
			ctx->lp_set.leapSecond += 1;
		real_lp.leapSecond = ctx->lp_set.leapSecond;	
		
	}
	else
	{
		
		switch(tsrc_type){
			case TOD_SOURCE_TYPE:
				if(ctx->cs.slot <= INPUT_SLOT_CURSOR_1 || ctx->cs.slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
					print(	DBG_ERROR,
							"--invalid slot:%d.",ctx->cs.slot);
					return -1;
				}
				if(!FpgaRead(ctx->fpga_fd, FPGA_IN_TOD_LEAP(ctx->cs.slot), &ptp_leap))
				{
					print(	DBG_ERROR, 
							"--Failed to read ptp_leap.");
					return -1;
				}

				ctx->lp_new.leapSecond = ptp_leap & 0x00ff;//低8位是1号槽位ptp闰秒
				real_lp.leapSecond = ctx->lp_new.leapSecond;
			break;
			case GPS_SOURCE_TYPE:
				memcpy(&real_lp, &ctx->lp_new, sizeof(struct leapinfo));
				break;
			case S2MH_SOURCE_TYPE:
			case S2MB_SOURCE_TYPE:
			case IRIGB1_SOURCE_TYPE:
			case IRIGB2_SOURCE_TYPE:
			case S10MH1_SOURCE_TYPE:
			case S10MH2_SOURCE_TYPE:
			case S1PPS_SOURCE_TYPE:
			case NTP_SOURCE_TYPE:
				//use orgin(last) value
				ctx->lp_new.leapSecond = ctx->lp_old.leapSecond;
				if( irigb_leap == 1 )
					ctx->lp_new.leapSecond += 1;//irigb leap++
				//mode:0
				//printf("leapsec:%d mode:%d\n",ctx->lp_new.leapSecond, ctx->lp_new.leapMode);
				memcpy(&real_lp, &ctx->lp_new, sizeof(struct leapinfo));
				break;
			default:
				break;
		}
		if(ctx->lp_old.leapSecond != real_lp.leapSecond){
			SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SYSLEAP_UPDATE, SID_SYS_SLOT, SID_SYS_PORT);
		}
		
	}
	
	//无告警60秒刷新;稳定判断选源
	int alm_inx = -1;

	switch (tsrc_type){
		case GPS_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_GPBD_SGN_LOS, tsrc_slot, SAT_PORT_SAT, alm_inx);
			//因为选源可能为强制选到无存在的源，所以应该去掉此处return，进行无源稳定性判断
			break;
		case S2MH_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MHZ_IN_LOS, tsrc_slot, SAT_PORT_2MH, alm_inx);
			break;
		case S2MB_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_IN_LOS, tsrc_slot, SAT_PORT_2MB, alm_inx);
			break;
		case IRIGB1_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, tsrc_slot, SAT_PORT_IRIGB1, alm_inx);
			break;
		case IRIGB2_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, tsrc_slot, SAT_PORT_IRIGB2, alm_inx);
			break;
		case TOD_SOURCE_TYPE:
			alm_inx = -1;
			switch (tsrc_bid){
				case BID_PTP_IN:
					FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_PTP_IN_LOS, tsrc_slot, 1, alm_inx);
					break;
				case BID_RTF:
					FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_TOD_IN_LOS, tsrc_slot, RTF_PORT_1PPS_TOD, alm_inx);
					break;
				default:
					break;
			}
			break;
		case S10MH1_SOURCE_TYPE:
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, tsrc_slot, RTF_PORT_10MH1, alm_inx);
			break;
		case S10MH2_SOURCE_TYPE:
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, tsrc_slot, RTF_PORT_10MH2, alm_inx);
			break;
		case S1PPS_SOURCE_TYPE:
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_IN_LOS, tsrc_slot, RTF_PORT_1PPS, alm_inx);
			break;
		case NTP_SOURCE_TYPE:
			alm_inx = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_IN_LOS, tsrc_slot, 1, alm_inx);
			break;
		case NO_SOURCE_TYPE:
			break;
		default:
			print(DBG_DEBUG, "Not support source type:%d", tsrc_type);
	}

	if(alm_inx >= 0 && ctx->inSta[tsrc_slot-1].boardId != BID_NONE)
		//tsrc_bid != BID_NONE 如果用此参数，则盘被突然拔掉时无法被发现,必须重启程序；使用insta的bid则无此问题
		//强制选源中，slot可能有效，告警可能无，为了不误判，所以需要检测实际bid
		port_alm = ctx->alarm[alm_inx].alm_sta;
	else
		port_alm = ALM_ON;

	struct port_attr_t *port_attr = NULL;//查看当前槽位板卡是否支持选源
	FIND_PORTATTR_GBATBL_BY_BID(tsrc_bid, port_attr);
	if (!port_attr){
		port_alm = ALM_ON;
	}
	if(schema != old_schema){
		//如果选源改变，将需要重新求源的稳定性。
		alm_num = 0;
		unalm_num = 0;
	}
	old_schema = schema;
	if( port_alm == ALM_OFF )
	{
		if(alm_num == 50)
		{
			if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_VALID, 0x0))//设置被选源信号可用
			{
				return -1;
			}
			#if 0
			write_fresh_state(REFSOURCE_VALID,ctx);
			#else
			ctx->cs.refsrc_is_valid = REFSOURCE_VALID;
			#endif
			#if 0
			//for the gpsled move to sta_parse
			switch(ctx->cs.port){//as same as sta_parse of din_satellite.c
				case GPS_SOURCE_SOLT1:
					ctx->inSta[0].satellite_valid = GET_SATELLITE;
					break;
				case GPS_SOURCE_SOLT2:
					ctx->inSta[1].satellite_valid = GET_SATELLITE;
			}
			#endif
		}
		else if(alm_num == 60)
		{
		
			print(DBG_INFORMATIONAL,"Get the stable Source[%d]: %s ",
				ctx->cs.src_inx, gTimeSourceTbl[ctx->cs.src_inx].msg);
			
			if(0 == FreshTime(ctx,real_lp.leapSecond))
			{
				return -1;
			}

		}
		unalm_num = 0;
		alm_num++;		
		
	}
	else if( port_alm == ALM_ON )
	{
		if(unalm_num == 60)
		{
			#if 0
			write_fresh_state(REFSOURCE_INVALID,ctx);
			#else
			ctx->cs.refsrc_is_valid = REFSOURCE_INVALID;
			#endif
			print(DBG_INFORMATIONAL,"Lost the stable Source. CurSource[%d]: %s.", 
				ctx->cs.src_inx, gTimeSourceTbl[ctx->cs.src_inx].msg);
			if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_VALID, 0x01))//设置被选源信号不可用
			{
				return -1;
			}
			#if 0
			//for the gpsled move to sta_parse
			switch(ctx->cs.port){//as same as sta_parse of din_satellite.c
				case GPS_SOURCE_SOLT1:
					ctx->inSta[0].satellite_valid = LOSS_SATELLITE;
					break;
				case GPS_SOURCE_SOLT2:
					ctx->inSta[1].satellite_valid = LOSS_SATELLITE;
			}
			#endif
		}
		alm_num = 0;		
		unalm_num++;
	}
	
	//闰秒变化时刷新
	if((ctx->lp_old.leapSecond != real_lp.leapSecond) || (ctx->lp_old.leapMode != real_lp.leapMode))
	{
		if (ctx->lp_old.leapMode != real_lp.leapMode)
			print(DBG_INFORMATIONAL,"User LeapMode change");
		//real_lp.leapMode = ctx->lp_set.leapMode;
		IssueLeapForecast(&real_lp);
		//memcpy(&ctx->lp_old, &real_lp, sizeof(struct leapinfo));
		if(port_alm == ALM_OFF && alm_num > 60)
		{
			if (ctx->lp_old.leapSecond != real_lp.leapSecond){
				print(DBG_INFORMATIONAL,"LeapSecond change:");
				if(0 == FreshTime(ctx,real_lp.leapSecond))
				{	
					print(	DBG_ERROR, 
							"<%s>--%s", 
							gDaemonTbl[DAEMON_CURSOR_INPUT], 
							"leap change  FreshTime Error." );
					return -1;
				}
			}
		}

		ctx->lp_old.leapSecond = real_lp.leapSecond;
		ctx->lp_old.leapMode = real_lp.leapMode;
		
	}

	//闰秒预告时刷新
	if(ctx->lp_old.leapForecast != real_lp.leapForecast){
		IssueLeapForecast(&real_lp);
		
		ctx->lp_old.leapForecast = real_lp.leapForecast;
		//memcpy(&ctx->lp_old, &real_lp, sizeof(struct leapinfo));
	}

	//钟从快捕进入锁定时刷新
	FlushTimeClockStateToLock(ctx);
	return 0;
}

int Time_Insource_and_Refresh_process(struct inCtx *ctx)
{
	if(0 != best_input_source(ctx))//最佳选源模式设置
	{
		print(DBG_ERROR, 
			  "<%s>--%s", 
			  gDaemonTbl[DAEMON_CURSOR_INPUT], 
			  "best_input_source Error.");
		return -1;
	}

	if(0 != Time_Refresh_Strategy(ctx))//最佳选源模式设置和时间刷新
	{
		print(DBG_ERROR, 
			  "--%s", "Time_Refresh_Strategy Error.");
		return -1;
	}

	if(0 != WriteInSourceInfoToShareMemory(ctx))
	{
		print(DBG_ERROR, 
			  "--%s", 
			  "WriteInSourceInfoToShareMemory Error.");
		return -1;
	}
	return 0;
	
}


void CopyBid(struct inCtx *ctx)
{
	int i;
	for (i = 0; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		ctx->oldBid[i] = ctx->inSta[i].boardId;
	}
}



/*
  1	成功
  0	失败
*/
int BoardPushPull(struct inCtx *ctx)
{
	int i;

	for(i=INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++)
	{
		if(((ctx->inSta[i].boardId >= BID_IN_LOWER) && (ctx->inSta[i].boardId <= BID_IN_UPPER))||
		   (BID_NONE == ctx->inSta[i].boardId))
		{
			//掉电前类型为无板卡--->当前板卡类型为有板卡
			if((BID_NONE == ctx->oldBid[i]) && (BID_NONE != ctx->inSta[i].boardId))
			{
				//拷贝默认配置
				if(0 == DeleteConfig(ctx, TBL_IO_RUNTIME, i+1))
				{
					return 0;
				}
				if(0 == CopyConfig(ctx, TBL_IO_DEFAULT, TBL_IO_RUNTIME, i+1, ctx->inSta[i].boardId, 1))
				{
					return 0;
				}
				if(0 == ReadConfigFromRunTimeTable(ctx, TBL_IO_RUNTIME, i+1, ctx->inSta[i].boardId, 1))
				{
					return 0;
				}
				if(0 == IssueConfig(ctx, i+1, ctx->inSta[i].boardId, INPUTINFO_SET_ALL_INFO))
				{
					return 0;
				}
			}
			//有板卡--->无板卡
			else if((BID_NONE != ctx->oldBid[i]) && (BID_NONE == ctx->inSta[i].boardId))
			{
				//清空当前槽位配置
				if(0 == EmptyConfig(ctx, TBL_IO_RUNTIME, i+1))
				{
					return 0;
				}

				ctx->inSta[i].sys_mode = MODE_CURSOR_NONE_VAL;
				ctx->inSta[i].satellite_state = 0x03;
			}
			//有板卡--->有板卡
			else if((BID_NONE != ctx->oldBid[i]) && (BID_NONE != ctx->inSta[i].boardId))
			{
				if(ctx->oldBid[i] != ctx->inSta[i].boardId)
				{
					//拷贝默认配置
					if(0 == DeleteConfig(ctx, TBL_IO_RUNTIME, i+1))
					{
						return 0;
					}
					if(0 == CopyConfig(ctx, TBL_IO_DEFAULT, TBL_IO_RUNTIME, i+1, ctx->inSta[i].boardId, 1))
					{
						return 0;
					}
					if(0 == ReadConfigFromRunTimeTable(ctx, TBL_IO_RUNTIME, i+1, ctx->inSta[i].boardId, 1))
					{
						return 0;
					}

					if(0 == IssueConfig(ctx, i+1, ctx->inSta[i].boardId, INPUTINFO_SET_ALL_INFO))
					{
						return 0;
					}
				}
				else
				{
					//设备上电后，如果当前的板卡类型与掉电前的板卡类型相等，则需要读取掉电前的配置，并下发配置。
					print(	DBG_DEBUG, 
							"--Same board identifier.");
					
					if(ctx->power_flag)
					{
						if(0 == ReadConfigFromRunTimeTable(ctx, TBL_IO_RUNTIME, i+1, ctx->inSta[i].boardId, 1))
						{
							return 0;
						}
						if(0 == IssueConfig(ctx, i+1, ctx->inSta[i].boardId, INPUTINFO_SET_ALL_INFO))
						{
							return 0;
						}
					}
				}
			}
			//无板卡--->无板卡
			else
			{
				//do nothing
			}
		}
	}

	CopyBid(ctx);
	print(	DBG_DEBUG, 
			"--Copy board identifier.");
	
	//清除设备上电后设置的上电标志。
	if(ctx->power_flag)
	{
		ctx->power_flag = false;
	}
	
	return 1;
}


/*
   0	成功
  -1	失败
*/
int ProcInput(struct inCtx *ctx)
{
	int ret = 0;
	int i;
	int tsrc_type = NO_SOURCE_TYPE;

	set_print_level(true, DBG_INFORMATIONAL,DAEMON_CURSOR_INPUT);
	syslog_init(NULL);
	initializeBoardNameTable();
	initializeContext(ctx);

#ifdef NEW_ALARMID_COMM
	if(-1 == initializeEventIndexTable())
	{
		ret = __LINE__;
		goto exit0;
	}

	if(-1 == initializeAlarmIndexTable())
	{
		ret = __LINE__;
		goto exit1;
	}
#endif

	if(-1 == initializeFpga(ctx))//打开FPGA
	{
		ret = __LINE__;
		goto exit1;
	}

	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit2;
	}
	
	if(-1 == initializeInt(ctx))//打开pps 2号interrupt
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializePriority(DAEMON_PRIO_INPUT))
	{
		ret = __LINE__;
		goto exit3;
	}

	if(-1 == initializeExitSignal())
	{
		ret = __LINE__;
		goto exit3;
	}

	if(-1 == initializeNotifySignal())
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if(-1 == initializeStavalid(ctx))
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if(-1 == initializeShareMemory(&(ctx->ipc)))
	{
		ret = __LINE__;
		//should free the ipc_base of all process 
		goto exit3;
	}
	
	if(-1 == initializeMessageQueue(&(ctx->ipc)))
	{
		ret = __LINE__;
		goto exit3;
	}

	if(-1 == initializeSemaphoreSet(&(ctx->ipc)))
	{
		ret = __LINE__;
		goto exit3;
	}

	if(-1 == initializeDatabase(ctx))
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if(0 == ReadLeapsecond(ctx, TBL_SYS))//从数据库中读闰秒
	{
		ret = __LINE__;
		goto exit4;
	}

	if(0 == ReadPhaseUploadSwitchFromTable(ctx, TBL_SYS))//从数据库中读性能上报是否使能
	{
		ret = __LINE__;
		goto exit4;
	}
	if(-1 == WritePhaseUploadSwitchToShareMemory(ctx))
	{
		ret = __LINE__;
		goto exit4;
	}

	
	if(-1 == writePid(&(ctx->ipc)))
	{
		ret = __LINE__;
		goto exit4;
	}

	if(0 == ReadBidFromTable(ctx, TBL_IO_RUNTIME))//从数据库中读取实时单板编号
	{
		ret = __LINE__;
		goto exit4;
	}
	if(0 == ReadBidFromFpga(ctx))//从FPGA中读取单板编号
	{
		ret = __LINE__;
		goto exit4;
	}

	if(0 == SysReadClockType(ctx->fpga_fd, (u16_t *)&ctx->clock_type))//从FPGA中读取钟的类型
	{
		ret = __LINE__;
		goto exit4;
	}

	SetVarinfo(ctx);
	//default:configure satellite, before BoardPushPull()
	if(0 == SetSatellite(ctx))
	{
		print(DBG_WARNING, "set satellite failed\n");
	}

	if(0 == BoardPushPull(ctx))//板卡配置初始化
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(0 == ReadSchema(ctx, TBL_SYS))//读取选源模式
	{
		ret = __LINE__;
		goto exit4;
	}
	#if 0
	write_fresh_state(REFSOURCE_INVALID,ctx);
	#endif
	if(0 == CreateThread(ctx))
	{
		ret = __LINE__;
		goto exit4;
	}

	//write last leapSecond to fpga
	if (0 == FreshLeapSecond(ctx->fpga_fd, ctx->lp_old.leapSecond)){
		print(DBG_ERROR, 
					  "<%s>--%s", 
					  gDaemonTbl[DAEMON_CURSOR_INPUT], 
					  "FreshLeapSecond Error.");
		ret = __LINE__;
		goto exit5;
	}
	print(DBG_INFORMATIONAL, "init_leapSecond:%d\n", ctx->lp_old.leapSecond);
	
	while(isRunning(ctx))
	{
		SysWaitFpgaRunStatusOk(ctx->fpga_fd);
		if(0 == ReadBidFromFpga(ctx))//从FPGA中读取单板编号
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "ReadBidFromFpga Error.");
			break;
		}
		INIT_EVNTFLAG(0);
		SetVarinfo(ctx);

		if(isNotify(ctx))//收到配置命令
		{
			if(0 == ReadMessageQueue(ctx))//通知输入控制
			{
				ret = __LINE__;
				print(DBG_ERROR, 
					  "<%s>--%s", 
					  gDaemonTbl[DAEMON_CURSOR_INPUT], 
					  "ReadMessageQueue Error.");
				break;
			}
		}

		if(0 == BoardPushPull(ctx))//设置板卡默认配置
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "BoardPushPull Error.");
			break;
		}

		
		
		if(0 == ReadSSM(ctx))//从fpga读取2MB时钟等级
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "ReadSSM Error.");
			break;
		}
		
		if(0 == ReadPhase(ctx))//读取相位
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "ReadPhase Error.");
			break;
		}
		
		if(true == ctx->pps_flag)//pps为输出信号类型
		{
			ctx->pps_flag = false;

			if(1 == ctx->sys_conf.ph_upload_en && pps_phaseperf >= READ_A_PHASE_PER_SECOND){
				pps_phaseperf = 0;
				if(0 == WriteDataToCurPhasePerf(ctx, gPhasePerfTbl, gPhasePerfTbl_len)){//收集性能数据
					ret = __LINE__;
					print(	DBG_ERROR, 
							"--%s", 
							"WriteDataToCurPhasePerf Error." );
					break;
				}
			}
			pps_phaseperf ++;
			if(0 == ReadSatellite(ctx))//读取接收来的GPS信息
			{
				ret = __LINE__;
				print(	DBG_ERROR, 
						"--%s",
						"ReadSatellite Error." );
				break;
			}

			tsrc_type = ctx->cs.type;
			if(tsrc_type == GPS_SOURCE_TYPE)
			{
				memcpy(&ctx->lp_new.leaptime, &ctx->inSta[ctx->cs.slot-1].satcommon.leaptime, sizeof(struct timeinfo));
				ctx->lp_new.leapstate = ctx->inSta[ctx->cs.slot-1].satcommon.leapstate;
				ctx->lp_set.leapstate = ctx->lp_new.leapstate;
				ctx->lp_new.leapSecond = ctx->inSta[ctx->cs.slot-1].satcommon.leapSecond;

				//SateCheckLeapforecastTime(ctx);
				//when a forecast come...
				if(1 == ctx->inSta[ctx->cs.slot-1].satcommon.leapstate)
				{
					ctx->pps_count++;
					
					if(1 == ctx->pps_count)
					{
						ctx->lp_new.leapForecast = 1;//闰秒预告, 在闰秒变化前59s置1, 闰秒变化后置0
						//IssueLeapForecast(&ctx->lp_new);//发出闰秒预告
					}
					if(60 <= ctx->pps_count)
					{
						ctx->lp_new.leapForecast = 0;
						//IssueLeapForecast(&ctx->lp_new);
						ctx->inSta[ctx->cs.slot-1].satcommon.leapstate = 0;
						ctx->pps_count = 0;
					}
				}else{
					ctx->pps_count = 0;
				}
			}

		}

		if(0 == ReadAlarm(ctx))//读取告警信息
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "ReadAlarm Error.");
			break;
		}
		
		if(0 != Time_Insource_and_Refresh_process(ctx))//最佳选源模式设置和时间刷新
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "Time_Refresh_Strategy Error.");
			break;
		}
		//for ptp/ssm
		if(-1 == stratum_input_source(ctx))//输入时钟设置
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "stratum_input_source Error.");
			break;
		}

		if(!ledCtrl(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_INPUT], 
				  "ledCtrl Error.");
			break;
		}

		close_unrelated_alm(ctx);

		Report_TimesourceToWg(ctx);//向集中网管上报时间源信息
		
		
		
		if(0 == WriteLeapToShareMemory(ctx))//向共享内存写入闰秒信息
		{
			ret = __LINE__;
			break;
		}
		
		if(0 == WriteAlarmToShareMemory(ctx))//向共享内存写入告警信息
		{
			ret = __LINE__;
			break;
		}

		print(DBG_DEBUG, "Sizeof inputsta:%d\n", sizeof(struct inputsta));//280
		for (i = INPUT_SLOT_CURSOR_1; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
			if(0 == WriteConfigToShareMemory(ctx, i+1)){//向共享内存写入n号槽配置信息
				ret = __LINE__;
				break;
			}
		}

		if(0 == WriteCurEventToTable(ctx->pDb, pEvntInxTbl, max_evntinx_num))//把事件写入到事件表
		{
			ret = __LINE__;
			break;
		}
		usleep(200000);//(200ms)
	}

exit5:
	CloseThread(ctx);
exit4:
	cleanDatabase(ctx);
exit3:
	cleanInt(ctx);
exit2:
	cleanFpga(ctx);
#ifdef NEW_ALARMID_COMM
	cleanAlarmIndexTable();
exit1:
#endif
	cleanEventIndexTable();
exit0:
	cleanContext(ctx);

	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();

	return ret;
}






