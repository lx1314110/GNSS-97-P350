#include "lib_fpga.h"
#include "lib_bit.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_dbg.h"

#include "addr.h"
#include "alloc.h"
#include "din_alarm.h"







bool_t ledBd(struct inCtx *ctx, u16_t led_sta)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_LED_BD, led_sta))
	{
		return false;
	}

	return true;
}






bool_t ledGps(struct inCtx *ctx, u16_t led_sta)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_LED_GPS, led_sta))
	{
		return false;
	}

	return true;
}






bool_t ledIrigb(struct inCtx *ctx, u16_t led_sta)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_LED_IRIGB, led_sta))
	{
		return false;
	}

	return true;
}






bool_t ledFreq(struct inCtx *ctx, u16_t led_sta)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_LED_FREQ, led_sta))
	{
		return false;
	}

	return true;
}

bool_t ledCtrl(struct inCtx *ctx)
{
	u16_t tmp;
	int i;
	int led_bd_status = LED_OFF;
	int led_gps_status = LED_OFF;
	tmp = ctx->cs.src_inx;

	int tsrc_type, tsrc_flag;
	tsrc_type = ctx->cs.type;
	tsrc_flag = ctx->cs.flag;

	//BD
				//only have signal
	for(i = INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		if (ctx->inSta[i].satellite_valid == GET_SATELLITE){
				led_bd_status = LED_ON;
				break;
		}
	}
	//have signal and choose it
	if(GPS_SOURCE_TYPE == tsrc_type){
		for(i = INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
			if((ctx->inSta[i].satellite_valid == GET_SATELLITE) &&
			(MODE_CURSOR_BD_VAL == ctx->inSta[i].sys_mode || MODE_CURSOR_MIX_BD_VAL == ctx->inSta[i].sys_mode)){
				led_bd_status = LED_ALT;
			}
		}
	}

	if(!ledBd(ctx, led_bd_status)){
		return false;
	}

	//GPS
	//only have signal
	for(i = INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		if (ctx->inSta[i].satellite_valid == GET_SATELLITE){
					led_gps_status = LED_ON;
					break;
		}
	}
	//have signal and choose it
	if(GPS_SOURCE_TYPE == tsrc_type) {
		for(i = INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
			if((ctx->inSta[i].satellite_valid == GET_SATELLITE) &&
			(MODE_CURSOR_GPS_VAL == ctx->inSta[i].sys_mode || MODE_CURSOR_MIX_GPS_VAL == ctx->inSta[i].sys_mode)){
				led_gps_status = LED_ALT;
			}
		}
	}

	if(!ledGps(ctx, led_gps_status)){
		return false;
	}

	
	//IRIGB
	int index_irigb1, index_irigb2;
	int valid_irigb = 0;
	for(i = INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		if (BID_INSATE_LOWER <= ctx->inSta[i].boardId && 
			BID_INSATE_UPPER >= ctx->inSta[i].boardId){
			//query irigb1/irigb2 alarm of the satellite board
			index_irigb1 = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, i+1, SAT_PORT_IRIGB1, index_irigb1);
			if(index_irigb1 == -1)
				return false;
			index_irigb2 = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, i+1, SAT_PORT_IRIGB2, index_irigb2);
			if(index_irigb2 == -1)
				return false;
			if (ALM_OFF == ctx->alarm[index_irigb1].alm_sta ||
				ALM_OFF == ctx->alarm[index_irigb2].alm_sta){
				valid_irigb = 1;
				break;
			}
		}
	}
	if( valid_irigb == 1 && \
		(IRIGB1_SOURCE_TYPE == tsrc_type || IRIGB2_SOURCE_TYPE == tsrc_type)
	){
		
		if(!ledIrigb(ctx, LED_ALT))
		{
			return false;
		}
	}
	else if( valid_irigb == 1)
	{
		
		if(!ledIrigb(ctx, LED_ON))
		{
			return false;
		}
	}
	else
	{
		
		if(!ledIrigb(ctx, LED_OFF))
		{
			return false;
		}
	}
	
	//freq
	int index_2mh, index_2mb, inx_2mb_other;
	int index_10mh1, index_10mh2, index_1pps;
	int valid_2mb = 0;
	int valid_freq = 0;
	for(i = INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		switch (ctx->inSta[i].boardId){
			case BID_INSATE_LOWER ... BID_INSATE_UPPER:
				//query 2mh/2mb alarm of the satellite board
				//2mh
				index_2mh = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MHZ_IN_LOS, i+1, SAT_PORT_2MH, index_2mh);
				if(index_2mh == -1)
					return false;
				if (ALM_OFF == ctx->alarm[index_2mh].alm_sta){
					valid_freq = 1;
					break;
				}

				//2mb, all 2mb should no alm, valid
				valid_2mb = 1;

				index_2mb = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_IN_LOS, i+1, SAT_PORT_2MB, index_2mb);
				if(index_2mb == -1)
					return false;
				if(ALM_ON == ctx->alarm[index_2mb].alm_sta)
					valid_2mb = 0;

				inx_2mb_other = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_AIS, i+1, SAT_PORT_2MB, inx_2mb_other);
				if(inx_2mb_other == -1)
					return false;
				if(ALM_ON == ctx->alarm[inx_2mb_other].alm_sta)
					valid_2mb = 0;

				inx_2mb_other = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_BPV, i+1, SAT_PORT_2MB, inx_2mb_other);
				if(inx_2mb_other == -1)
					return false;
				if(ALM_ON == ctx->alarm[inx_2mb_other].alm_sta)
					valid_2mb = 0;

				inx_2mb_other = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_CRC, i+1, SAT_PORT_2MB, inx_2mb_other);
				if(inx_2mb_other == -1)
					return false;
				if(ALM_ON == ctx->alarm[inx_2mb_other].alm_sta)
					valid_2mb = 0;

				inx_2mb_other = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_LOF, i+1, SAT_PORT_2MB, inx_2mb_other);
				if(inx_2mb_other == -1)
					return false;
				if(ALM_ON == ctx->alarm[inx_2mb_other].alm_sta)
					valid_2mb = 0;

				if (valid_2mb == 1){
					valid_freq = 1;
					break;
				}
				break;
			case BID_RTF:
				//10mh
				index_10mh1 = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, i+1, RTF_PORT_10MH1, index_10mh1);
				if(index_10mh1 == -1)
					return false;
				if (ALM_OFF == ctx->alarm[index_10mh1].alm_sta){
					valid_freq = 1;
					break;
				}
				index_10mh2 = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, i+1, RTF_PORT_10MH2, index_10mh2);
				if(index_10mh2 == -1)
					return false;
				if (ALM_OFF == ctx->alarm[index_10mh2].alm_sta){
					valid_freq = 1;
					break;
				}
				//1pps
				index_1pps = -1;
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_IN_LOS, i+1, RTF_PORT_1PPS, index_1pps);
				if(index_1pps == -1)
					return false;
				if (ALM_OFF == ctx->alarm[index_1pps].alm_sta){
					valid_freq = 1;
					break;
				}
				break;	
			}		
	}

	if( valid_freq == 1 && FREQ_SOURCE_FLAG == tsrc_flag){
		if(!ledFreq(ctx, LED_ALT))
		{
			return false;
		}
	}else if(valid_freq == 1){
		if(!ledFreq(ctx, LED_ON))
		{
			return false;
		}
	}
	else
	{
		if(!ledFreq(ctx, LED_OFF))
		{
			return false;
		}
	}
	return true;
}


/*
  1	成功
  0	失败
*/
/*manage the SMP_SID_SUB_SAT and SMP_SID_SUB_IN*/
int ReadAlarm(struct inCtx *ctx)
{
	u16_t alm;
	int slot;
	int index = -1;

	for (slot = INPUT_SLOT_CURSOR_1+1; slot < INPUT_SLOT_CURSOR_ARRAY_SIZE+1; ++slot){
		
		if(!FpgaRead(ctx->fpga_fd, FPGA_IN_SIGNAL(slot), &alm)){
			return 0;
		}

		//i2mhz/2mb/irigb1/irigb2 alarm of the satellite board
		if (BID_INSATE_LOWER <= ctx->inSta[slot-1].boardId && 
			BID_INSATE_UPPER >= ctx->inSta[slot-1].boardId){

			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_IN_LOS, slot, SAT_PORT_2MB, index);
			if(index == -1)
				return 0;
			if(alm &BIT(0))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			// #1 2mh alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MHZ_IN_LOS, slot, SAT_PORT_2MH, index);
			if(index == -1)
				return 0;
			if(alm &BIT(1))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			// #1 irigb1 alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, slot, SAT_PORT_IRIGB1, index);
			if(index == -1)
				return 0;
			if(alm &BIT(2))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			// #1 irigb2 alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, slot, SAT_PORT_IRIGB2, index);
			if(index == -1)
				return 0;
			if(alm &BIT(3))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}
		}
		// #1 gps/bd ptp alarm

		int index_sat = -1, index_ptpin = -1, index_1pps_tod = -1;//RTF
		int index_ntpin = -1;
		FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_GPBD_SGN_LOS, slot, SID_PWR_PORT, index_sat);
		if(index_sat == -1)
			return 0;
		FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_PTP_IN_LOS, slot, 1, index_ptpin);
		if(index_ptpin == -1)
			return 0;
		FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_TOD_IN_LOS, slot, RTF_PORT_1PPS_TOD, index_1pps_tod);
		if(index_1pps_tod == -1)
			return 0;
		FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_IN_LOS, slot, 1, index_ntpin);
		if(index_ntpin == -1)
			return 0;

		if(alm &BIT(4)){
			switch (ctx->inSta[slot-1].boardId){
				//4 no signal, all signal of bit(4) alarm on:(ptp+satellite+(1pps+tod))
				case BID_GPSBF...BID_BFFI:
					ctx->alarm[index_sat].alm_sta = ALM_ON;
					break;
				case BID_PTP_IN:
					ctx->alarm[index_ptpin].alm_sta = ALM_ON;
					break;
				case BID_RTF:
					ctx->alarm[index_1pps_tod].alm_sta = ALM_ON;
					break;
				case BID_NTP_IN:
					ctx->alarm[index_ntpin].alm_sta = ALM_ON;
					break;
			}

		} else {
			u16_t ntp_time_valid = 0;
			switch (ctx->inSta[slot-1].boardId){
				
				case BID_GPSBF...BID_BFFI:
					ctx->alarm[index_sat].alm_sta = ALM_OFF;
					break;
				
				case BID_PTP_IN://1st PTP输入告警
					ctx->alarm[index_ptpin].alm_sta = ALM_OFF;
					break;

				case BID_RTF:
					ctx->alarm[index_1pps_tod].alm_sta = ALM_OFF;
					break;
				case BID_NTP_IN:
					if(!FpgaRead(ctx->fpga_fd, FPGA_IN_NTP_IN_TIME_VAILD(slot), &ntp_time_valid)){
						return 0;
					}
					if(ntp_time_valid == 1)
						ctx->alarm[index_ntpin].alm_sta = ALM_OFF;
					else
						ctx->alarm[index_ntpin].alm_sta = ALM_ON;
					break;
			}

		}

		
		int index_1pps = -1, index_10mh1 = -1, index_10mh2 = -1;
		switch (ctx->inSta[slot-1].boardId){
			//4 no signal, all signal of bit(5) alarm on:10mh)
			case BID_RTF:
				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, slot, RTF_PORT_10MH1, index_10mh1);
				if(index_10mh1 == -1)
					return 0;
				if(INRTF_10MH1_SGN_PORT3(alm))
					ctx->alarm[index_10mh1].alm_sta = ALM_ON;
				else
					ctx->alarm[index_10mh1].alm_sta = ALM_OFF;

				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, slot, RTF_PORT_10MH2, index_10mh2);
				if(index_10mh2 == -1)
					return 0;
				if(INRTF_10MH2_SGN_PORT4(alm))
					ctx->alarm[index_10mh2].alm_sta = ALM_ON;
				else
					ctx->alarm[index_10mh2].alm_sta = ALM_OFF;

				FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_IN_LOS, slot, RTF_PORT_1PPS, index_1pps);
				if(index_1pps == -1)
					return 0;
				if(INRTF_1PPS_SGN_PORT2(alm))
					ctx->alarm[index_1pps].alm_sta = ALM_ON;
				else
					ctx->alarm[index_1pps].alm_sta = ALM_OFF;

				break;
		}

		//2mb/satellite private alarm of the satellite board
		if (BID_INSATE_LOWER <= ctx->inSta[slot-1].boardId && 
			BID_INSATE_UPPER >= ctx->inSta[slot-1].boardId){
			//2mb private
			if(!FpgaRead(ctx->fpga_fd, FPGA_IN_2MB_ALM(slot), &alm))
			{
				return 0;
			}

			// #1 bpv alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_BPV, slot, SAT_PORT_2MB, index);
			if(index == -1)
				return 0;
			if(alm &BIT(0))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			// #1 ais alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_AIS, slot, SAT_PORT_2MB, index);
			if(index == -1)
				return 0;
			if(alm &BIT(1))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			// #1 crc alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_CRC, slot, SAT_PORT_2MB, index);
			if(index == -1)
				return 0;
			if(alm &BIT(3))
			{
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			// #1 sync alarm
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_LOF, slot, SAT_PORT_2MB, index);
			if(index == -1)
				return 0;
			if(alm &BIT(2))
			{	//sync/lof
				ctx->alarm[index].alm_sta = ALM_ON;
			}
			else
			{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}


			//----------------------------------------------------------------------------------------------------
			//卫星信号劣化告警
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_GPBD_SGN_DGD, slot, SID_SAT_PORT, index);
			if(index == -1)
				return 0;
			if(0x00 == ctx->inSta[slot-1].satellite_state && GET_SATELLITE == ctx->inSta[slot-1].satellite_valid){
				//卫星劣化event
				//SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SAT_LNUM, slot, SID_SAT_PORT);
				SET_EVNTFLAGVALID_BID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SAT_LNUM, slot, SID_SAT_PORT, ctx->inSta[slot-1].boardId);
				ctx->alarm[index].alm_sta = ALM_ON;
			}else{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			//短路告警
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_ANT_SHORT, slot, SID_SAT_PORT, index);
			if(index == -1)
				return 0;
			if(SATELLITE_ANTENNA_LOOP == ctx->inSta[slot-1].antenna_state){
				ctx->alarm[index].alm_sta = ALM_ON;
			}else{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}

			//开路告警
			index = -1;
			FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_ANT_OPEN, slot, SID_SAT_PORT, index);
			if(index == -1)
				return 0;
			if(SATELLITE_ANTENNA_OPEN == ctx->inSta[slot-1].antenna_state){
				ctx->alarm[index].alm_sta = ALM_ON;
			}else{
				ctx->alarm[index].alm_sta = ALM_OFF;
			}
		}
	}

	return 1;
}

#if 1
void close_unrelated_alm(struct inCtx *ctx)
{
	//只需要屏蔽2mh的其他告警,选源不需要屏蔽告警了。
}
#else
/*****************************************************************************
 函 数 名  : close_unrelated_alm
 功能描述  : 关闭和选源无关的告警
 输入参数  : ctx: 存放输入信息  
            
 输出参数  : 无
 返 回 值  : 无
  
  
 
 修改历史      :
  1.日    期   : 2016年9月13日
    作    者   : dtt
    修改内容   : 新生成函数

*****************************************************************************/
void close_unrelated_alm(struct inCtx *ctx)
{
	int i,j;
#if 0
	//ONLY 1号槽有源
	if((ctx->inSta[0].boardId >= BID_IN_LOWER && ctx->inSta[0].boardId <= BID_IN_UPPER) &&
		(ctx->inSta[1].boardId <BID_IN_LOWER || ctx->inSta[1].boardId > BID_IN_UPPER))
		{

			//屏蔽其他输入槽报警 19-32
			for(i = ALM_CURSOR_SLOT2_START;i < ALM_CURSOR_SLOT2_END+1;i++)
			{
				ctx->alarm[i-1].alm_sta = ALM_OFF;
			}
			
		}
	else if((ctx->inSta[0].boardId <BID_IN_LOWER || ctx->inSta[0].boardId > BID_IN_UPPER) &&
		(ctx->inSta[1].boardId >= BID_IN_LOWER && ctx->inSta[1].boardId <= BID_IN_UPPER))
		{//ONLY 2号槽有源

			//屏蔽其他输入槽报警 4-18
			for(i = ALM_CURSOR_SLOT1_START;i < ALM_CURSOR_SLOT1_END+1;i++)
			{
				ctx->alarm[i-1].alm_sta = ALM_OFF;
			}
		}
	else if((ctx->inSta[0].boardId >= BID_IN_LOWER && ctx->inSta[0].boardId <= BID_IN_UPPER) &&
		(ctx->inSta[1].boardId >= BID_IN_LOWER && ctx->inSta[1].boardId <= BID_IN_UPPER))
		{//1和 2号槽都有源

			//n号输入盘如果不是卫星盘，应屏蔽当前槽位卫星盘上的其他信号报警
			//1号
			if (ctx->inSta[INPUT_SLOT_CURSOR_1].boardId < BID_INSATE_LOWER || ctx->inSta[INPUT_SLOT_CURSOR_1].boardId > BID_INSATE_UPPER){
				for(i = ALM_CURSOR_SLOT1_SATE_OTHER_START ;i < ALM_CURSOR_SLOT1_SATE_OTHER_END+1;i++)
				{
					ctx->alarm[i-1].alm_sta = ALM_OFF;
				}
			}
			//2号
			if (ctx->inSta[INPUT_SLOT_CURSOR_2].boardId < BID_INSATE_LOWER || ctx->inSta[INPUT_SLOT_CURSOR_2].boardId > BID_INSATE_UPPER){
				for(i = ALM_CURSOR_SLOT2_SATE_OTHER_START ;i < ALM_CURSOR_SLOT2_SATE_OTHER_END+1;i++)
				{
					ctx->alarm[i-1].alm_sta = ALM_OFF;
				}
			}

			//可以像选源那样，添加查找宏定义，添加bid和alarm_id以及slot的映射表格，通过通用查找的方式，彻底解决以下特指
			//和read alarm， Time_Insource_and_Refresh_process相关联（bit4. ptp，1pps+tod，卫星）
			if(ctx->inSta[INPUT_SLOT_CURSOR_1].boardId != BID_PTP_IN)
				ctx->alarm[ALM_CURSOR_SLOT1_PTP_IN-1].alm_sta = ALM_OFF;
			if(ctx->inSta[INPUT_SLOT_CURSOR_2].boardId != BID_PTP_IN)
				ctx->alarm[ALM_CURSOR_SLOT2_PTP_IN-1].alm_sta = ALM_OFF;

		}
	else
		{

		}


	//1
	if(ctx->alarm[ALM_CURSOR_SLOT1_2MB1 -1].alm_sta != ALM_OFF)
	{
		//如果本槽位2mbit发生输入报警，则屏蔽本槽位2mbit其他报警15-18
		for(i = ALM_CURSOR_SLOT1_2MB_OTHER_START;i < ALM_CURSOR_SLOT1_2MB_OTHER_END+1;i++)
		{
			ctx->alarm[i-1].alm_sta = ALM_OFF;
		}
	}

	//2
	if(ctx->alarm[ALM_CURSOR_SLOT2_2MB1 -1].alm_sta != ALM_OFF)
	{
		//如果本槽位2mbit发生输入报警，则屏蔽本槽位2mbit其他报警29-32
		for(i = ALM_CURSOR_SLOT2_2MB_OTHER_START;i < ALM_CURSOR_SLOT2_2MB_OTHER_END+1;i++)
		{
			ctx->alarm[i-1].alm_sta = ALM_OFF;
		}
	
	}

#else
	//特殊处理2MB信号...
	int mb_alm_flag = 0;

	for (j = INPUT_SLOT_CURSOR_1; j < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++j){

		mb_alm_flag = 0;
		for (i = ALM_CURSOR_INSLOT_START-1; i < ALM_CURSOR_INSLOT_END; ++i){
			if (gAlarmTbl[i].alarm_slot == SLOT_NONE){
				continue;//currently no need
			}
			if(j == gAlarmTbl[i].alarm_slot-1){
				if (gAlarmTbl[i].alarm_type_bid == BID_NONE){
					continue;//currently no need
				}
				if(ctx->inSta[j].boardId != gAlarmTbl[i].alarm_type_bid)//屏蔽处理当前槽位上其他的报警id
					ctx->alarm[i].alm_sta = ALM_OFF;

				//特殊处理2MB信号...		1-2slot		
				if(i == ALM_CURSOR_SLOT1_2MB1 || i == ALM_CURSOR_SLOT2_2MB1){
					if(ctx->alarm[i].alm_sta != ALM_OFF){
						mb_alm_flag = 1;
					}
				}
				if (mb_alm_flag == 1 && gAlarmTbl[i].alarm_signal == SIGNAL_2MB){
					if(i != ALM_CURSOR_SLOT1_2MB1 || i != ALM_CURSOR_SLOT2_2MB1){
						//如果本槽位2mbit发生输入报警，则屏蔽本槽位2mbit其他报警15-18
						ctx->alarm[i].alm_sta = ALM_OFF;
					}
				}

			}
		}
	}
#endif

}
#endif

