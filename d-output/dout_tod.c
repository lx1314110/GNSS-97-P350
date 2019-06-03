#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_time.h"
#include "lib_bit.h"

#include "addr.h"
#include "dout_global.h"
#include "dout_tod.h"








/*
  1		成功
  0		失败
*/
#ifdef CHINA_MIIT_TOD_RULE
int china_mobile_miit_tod(struct outCtx *ctx)
{
	/*
	  0		自由运行
	  1,2	保持
	  3		快捕
	  4+	锁定
	*/
	u16_t clock_state;
	static u16_t clock_state_has_lock = 0;

	//1-RB 2-XO
	u16_t clock_type;

	/*
	  0x00		一级钟
	  0x01		二级钟
	  0x02		三级钟
	  0x03-0xFF	保留
	*/
	u16_t clock_stratum;

	/*old
	  0x00	#1 卫星输入
	  0x01	#1 2mh输入
	  0x02	#1 2mb输入
	  0x03	#1 irigb1输入
	  0x04	#1 irigb2输入
	  
	  0x05	#2 卫星输入
	  0x06	#2 2mh输入
	  0x07	#2 2mb输入
	  0x08	#2 irigb1输入
	  0x09	#2 irigb2输入

	  0x0A	输入不可用、无输入
	*/
	u16_t clock_source;
	u8_t tsrc_type = NO_SOURCE_TYPE;
	u8_t  tsrc_flag = INVALID_SOURCE_FLAG;

	u16_t pps_state;

	u16_t elec_zone_offset = 0;
	u16_t elec_tod_tag1 = 0;
	u16_t elec_tod_tag2 = 0;
	u16_t elec_tod_tag3 = 0;
	u16_t elec_tod_tag4 = 0;
	u16_t elec_time_quality = 0x0F;

	u16_t elec_irigb_leap_forecast = 0;

	//irigb leap forecast
	if(ctx->lp.leapForecast)
	{
		elec_irigb_leap_forecast |= BIT(0);
	}
	else
	{
		elec_irigb_leap_forecast &= ~BIT(0);
	}

	//if(ctx->lp.leapSecond < 0)
	if(ctx->lp.leapstate == 2)//负闰秒
	{
		elec_irigb_leap_forecast |= BIT(1);
	}
	else
	{
		elec_irigb_leap_forecast &= ~BIT(1);
	}

	//group 1
	elec_zone_offset = gTzTbl[ctx->out_zone.zone[0] -'@'];
	if(elec_zone_offset < 0)
	{
		elec_zone_offset = -elec_zone_offset;
		elec_tod_tag1 |= BIT(4);
	}
	else
	{
		elec_tod_tag1 &= ~BIT(4);
	}
	elec_tod_tag1 |= (elec_zone_offset <<8);

	if(ctx->lp.leapForecast)
	{
		elec_tod_tag1 |= BIT(1);
	}
	else
	{
		elec_tod_tag1 &= ~BIT(1);
	}

	//group 2
	elec_zone_offset = gTzTbl[ctx->out_zone.zone[1] -'@'];
	if(elec_zone_offset < 0)
	{
		elec_zone_offset = -elec_zone_offset;
		elec_tod_tag2 |= BIT(4);
	}
	else
	{
		elec_tod_tag2 &= ~BIT(4);
	}
	elec_tod_tag2 |= (elec_zone_offset <<8);

	if(ctx->lp.leapForecast)
	{
		elec_tod_tag2 |= BIT(1);
	}
	else
	{
		elec_tod_tag2 &= ~BIT(1);
	}

	//group 3
	elec_zone_offset = gTzTbl[ctx->out_zone.zone[2] -'@'];
	if(elec_zone_offset < 0)
	{
		elec_zone_offset = -elec_zone_offset;
		elec_tod_tag3 |= BIT(4);
	}
	else
	{
		elec_tod_tag3 &= ~BIT(4);
	}
	elec_tod_tag3 |= (elec_zone_offset <<8);

	if(ctx->lp.leapForecast)
	{
		elec_tod_tag3 |= BIT(1);
	}
	else
	{
		elec_tod_tag3 &= ~BIT(1);
	}

	//group 4
	elec_zone_offset = gTzTbl[ctx->out_zone.zone[3] -'@'];
	if(elec_zone_offset < 0)
	{
		elec_zone_offset = -elec_zone_offset;
		elec_tod_tag4 |= BIT(4);
	}
	else
	{
		elec_tod_tag4 &= ~BIT(4);
	}
	elec_tod_tag4 |= (elec_zone_offset <<8);

	if(ctx->lp.leapForecast)
	{
		elec_tod_tag4 |= BIT(1);
	}
	else
	{
		elec_tod_tag4 &= ~BIT(1);
	}
	

	ctx->pps_count++;
	if(ctx->pps_count > 172800)/* 48 hour */
	{
		ctx->pps_count = 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_STA, &clock_state))
	{
		return 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_TYP, &clock_type))
	{
		return 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_INSRC_STRATUM, &clock_stratum))
	{
		return 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_INSRC_SELECT, &clock_source))
	{
		return 0;
	}
	FIND_TYPE_BY_FPGA_VAL(clock_source, tsrc_type);
	FIND_FLAG_BY_FPGA_VAL(clock_source, tsrc_flag);

	//曾经锁定过
	if (clock_state == 4)
		clock_state_has_lock = 1;

	//自由运行
	if(0 == clock_state){
		//ctx->pps_count = 0;
		pps_state = 0x02;//
		if (clock_state_has_lock == 1){
			elec_time_quality = 0x0F;//0x09
		}else{
			elec_time_quality = 0x0F;
		}
	}else if(3 == clock_state){//快捕
		//ctx->pps_count = 0;
		pps_state = 0x02;//
		elec_time_quality = 0x0A;
	}
	//锁定, 时间
	else if(clock_state >= 4)
	{
		//锁定，跟踪卫星
		if(GPS_SOURCE_TYPE == tsrc_type ||
			IRIGB1_SOURCE_TYPE == tsrc_type || IRIGB2_SOURCE_TYPE == tsrc_type || 		
			 TOD_SOURCE_TYPE == tsrc_type){
			 //ctx->pps_count = 0;
			pps_state = 0x00;
			elec_time_quality = 0x00;
		}

			//锁定, 频率(2mh/2mb 频率输入设备)
		if(tsrc_flag == FREQ_SOURCE_FLAG){
			pps_state = 0x01;
			elec_time_quality = 0x04;
		}
		
	}
	//保持
	else if((1 == clock_state) || (2 == clock_state) ){

		//保持, (晶体/三级)钟, 频率, (2mh/2mb 频率输入设备)
		if(
			(clock_type == 2) || 
				(2 == clock_stratum &&
			(tsrc_flag == FREQ_SOURCE_FLAG))
		){
			pps_state = 0x03;
			elec_time_quality = 0x06;
		}
			//保持, (铷/二级)钟, 频率, (2mh/2mb 频率输入设备)
		if(
			(clock_type == 1) || 
				(1 == clock_stratum &&
			(tsrc_flag == FREQ_SOURCE_FLAG))
		){
			pps_state = 0x05;
			elec_time_quality = 0x05;
		}
	}else{
		//ctx->pps_count = 0;
		pps_state = 0x02;

		elec_time_quality = 0x0F;
	}


	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_PPS_STATE, pps_state))
	{
		return 0;
	}
	#if 0	//sys_leap to process
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_PTP_LEAP_SECOND, ctx->lp.leapSecond))
	{
		return 0;
	}
	#endif
	//elec_time_quality
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_PPS_JITTER, elec_time_quality)) //orgin value:0x000F
	{
		return 0;
	}

	//因为主要是对卫星状态的获取，所以该部分移到input进程stratum_input_source()
	#if 0
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_CLK_SRC_STATE, 0x00FF))
	{
		return 0;
	}
	#endif
	//因为主要是对卫星状态的获取，所以该部分移到input进程stratum_input_source()
	#if 0
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_CLK_SRC_ALARM, 0x0000))
	{
		return 0;
	}
	#endif


	elec_tod_tag1 |= (elec_time_quality <<12);

	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_TAG, elec_tod_tag1))
	{
		return 0;
	}

	//irigb leap forecast
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_LEAP_FORECAST, elec_irigb_leap_forecast))
	{
		return 0;
	}
	print(	DBG_DEBUG,
			"--pps state %02X",
			pps_state	);

	return 1;
}
#endif







