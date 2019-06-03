#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>


#include "lib_fpga.h"
#include "lib_common.h"

//#define PRINT_DISABLE
#include "lib_dbg.h"

#include "lib_time.h"
#include "lib_bit.h"
#include "lib_sema.h"
#include "lib_shm.h"

#include "addr.h"
#include "alloc.h"
#include "din_issue.h"
#include "din_parser.h"
#include "din_satellite.h"
#include "din_source.h"

//bool_t lps = false;


/*
  1	成功
  0	失败
*/
#if 0
void write_fresh_state(u8_t state,struct inCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT] );

		return;
	}
	
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_FRESH, 
			  sizeof(state), 
			  (char *) &state, 
			  sizeof(state));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT] );
		
		return;
	}

	return;
}
#endif
/*****************************************************************************
 函 数 名  : SetSatMode()
 功能描述  : 设置卫星模块的工作模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  :  void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年1月26日
    作    者   : dtt
    修改内容   : 新生成函数

*****************************************************************************/

static int WriteDataToSatBuf(int fpga_fd, int slot, u8_t *buf, int len)
{
	int i;
	for(i=0; i<len; i++)
	{
        if(!FpgaWrite(fpga_fd, FPGA_IN_GB_WADDR(slot), i))
		{
			return 0;
		}
		if(!FpgaWrite(fpga_fd, FPGA_IN_GB_WVALUE(slot), buf[i]))
		{
			return 0;
		}
		if(!FpgaWrite(fpga_fd, FPGA_IN_GB_WEN(slot), 0x0000))
		{
			return 0;
		}
		if(!FpgaWrite(fpga_fd, FPGA_IN_GB_WEN(slot), 0x0001))
		{
			return 0;
		}
		
	}

	if(!FpgaWrite(fpga_fd, FPGA_IN_GB_WREADY(slot), 0x0001))
	{
		return 0;
	}
	if(!FpgaWrite(fpga_fd, FPGA_IN_GB_WREADY(slot), 0x0000))
	{
		return 0;
	}
	usleep(100000);
	return 1;
}
static int SetSatMode(int fpga_fd,int slot)
{
	u8_t buf[128];
	u8_t xor;
	u8_t xstr[3];
	u16_t len;
	
	usleep(100000);
	memset(buf,0,sizeof(buf));
    sprintf((char *)buf,"$CFG-MSG,1,0,0,1,1,0,1,1,1,0,%d*",1);
	len = strlen((const char *)buf);
	xor = data_xor(&buf[1], len-2);
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	//printf("%s\n",xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;
	//printf("[%d] %s\n",__LINE__,buf);
	print(	DBG_INFORMATIONAL, 
				"--buf:%s", buf);

	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write CFG-MSG failure!");
		return 0;
	}	
	
    memset(buf,0x00,sizeof(buf));
    sprintf((char *)buf,"$CFG-PMODE,%d*",3);  //设置卫星工作模式为GPS主用混合模式
    len = strlen((const char *)buf);
	xor = data_xor(&buf[1], len-2);
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;

	print(	DBG_INFORMATIONAL, 
			"--buf:%s",
			buf);

	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write CFG-PMODE failure!");
		return 0;
	}	
    
    memset(buf,0x00,sizeof(buf));
    //sprintf((char *)buf,"$CFG-TP,00030D40,1,%d,0000,00000000*",0);
    //0-一直输出1pps; 1-卫星无效时，不输出
	sprintf((char *)buf,"$CFG-TP,00030D40,1,%d,0000,00000000*",1);
    len = strlen((const char *)buf);
	xor = data_xor(&buf[1], len-2);
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;
	//printf("[%d] %s\n",__LINE__,buf);
	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write CFG-TP failure!");
		return 0;
	}
	
	memset(buf,0x00,sizeof(buf));
    sprintf((char *)buf,"$CFG-CF%c*",'G');
    len = strlen((const char *)buf);
	xor = data_xor(&buf[1], len-2);
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;
	//printf("[%d] %s\n",__LINE__,buf);
	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write CFG-CFG failure!");
		return 0;
	}
    return 1;
	
}

int SetSatellite(struct inCtx *ctx)
{
	u8_t bid;
	int slot;

	for (slot = INPUT_SLOT_CURSOR_1+1; slot < INPUT_SLOT_CURSOR_ARRAY_SIZE+1; ++slot)
	{
		bid = ctx->inSta[slot-1].boardId;
		if( BID_GPSBF == bid ||
			BID_GPSBE == bid ||
			BID_GBDBF == bid ||
			BID_GBDBE == bid ||
			BID_BDBF == bid ||
			BID_BDBE == bid ){

			if(0 == SetSatMode(ctx->fpga_fd,slot)){
				return 0;
			}	
		}
	}

	return 1;

}

/*
  1	成功
  0	失败
*/
//取消该功能
int lola_elev_set(int fpga_fd, int slot, u8_t *lon, u8_t *lat, u8_t *elev)
{
#if 0
	u8_t buf[128];
	u8_t xor;
	u8_t xstr[3];
	u16_t len;
	u16_t i;

	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf, "$CFG-LLA,%s,%c,%s,%c,%s,0*", &lat[1], lat[0], &lon[1], lon[0], elev);
	len = strlen((const char *)buf);
	
	xor = data_xor(&buf[1], len-2);
	print(	DBG_DEBUG, 
			"<%s>--xor:%02X", 
			xor);
	
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;

	print(	DBG_INFORMATIONAL, 
			"--slot:%d buf:%s",  
			slot, buf);

	/*if(!FpgaWrite(fpga_fd, FPGA_S01_IN_GB_WADDR, 128+i))?*/
	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write lola elev failure!");
		return 0;
	}
#endif	
	return 1;
}

/*
  1	成功
  0	失败
*/
int mode_set(int fpga_fd, int slot, u8_t mode)
{
	u8_t buf[128];
	u8_t xor;
	u8_t xstr[3];
	u16_t len;
	u8_t sysmode;

	if(0x01 == mode)
	{
		sysmode = '1';
	}
	else if(0x02 == mode)
	{
		sysmode = '2';
	}
	else if(0x03 == mode)
	{
		sysmode = '3';
	}
	else if(0x04 == mode)
	{
		sysmode = '4';
	}
	else
	{
		return 0;
	}


	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf, "$CFG-PMODE,%c*", sysmode);
	len = strlen((const char *)buf);
	
	xor = data_xor(&buf[1], len-2);
	print(	DBG_DEBUG, 
			"--xor: %02X",
			xor);
	
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;
	//printf("[%d] %s\n",__LINE__,buf);

	print(	DBG_INFORMATIONAL, 
			"--slot:%d buf:%s",
			slot, buf);
	
	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write CFG-PMODE failure!");
		return 0;
	}

	memset(buf, 0, sizeof(buf));
	sprintf((char *)buf,"$CFG-CF%c*",'G');
	len = strlen((const char *)buf);
	xor = data_xor(&buf[1], len-2);
	memset(xstr, 0, sizeof(xstr));
	itohexa(xor, (char *)xstr);
	if(1 == strlen((const char *)xstr))
	{
		buf[len] = '0';
		buf[len+1] = xstr[0];
	}
	else
	{
		buf[len] = xstr[0];
		buf[len+1] = xstr[1];
	}

	buf[len+2] = 0x0d;
	buf[len+3] = 0x0a;
	//printf("[%d] %s\n",__LINE__,buf);
	print(	DBG_INFORMATIONAL, 
			"--slot:%d buf:%s",
			slot, buf);

	if(!WriteDataToSatBuf(fpga_fd, slot, buf, len+4)){
		print(DBG_ERROR, "Write CFG-CFG failure!");
		return 0;
	}

	return 1;
}

/*
  1	成功
  0	失败
*/
int GGA_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	u8_t xor;
	u8_t comma[128];
	//u16_t temp;
	u8_t alm;
	u8_t sysmode = MODE_CURSOR_NONE_VAL;
	#if 0
	struct timeinfo utc;
	#endif
	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
		return 0;
	}

	len = strlen((const char *)data);
	if('*' != data[len-3])
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SGGA.");
		return 0;
	}

	if((data[len-2] >= '0') && (data[len-2] <= '9'))
	{
		xor = 16 *(data[len-2]-0x30);
	}
	else
	{
		xor = 16 *(data[len-2]-0x37);
	}
	if((data[len-1] >= '0') && (data[len-1] <= '9'))
	{
		xor += data[len-1] -0x30;
	}
	else
	{
		xor += data[len-1] -0x37;
	}
	
	if(xor != data_xor(&data[1], len-4))
	{
		print(	DBG_WARNING, 
				"--gga data: %s",
				data);
		
		print(	DBG_WARNING, 
				"--Failed to check 2SGGA.");

		return 0;
	}

	//       0         1          2 3           4 5 6  7    8     9 10   11 1213
	//$2SGGA,020706.00,3032.49927,N,10403.16411,E,G,00,0.00,500.2,M,01.80,M,2,00*55 //14','
	memset(comma, 0, 128);
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}

	if(15 != j)
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SGGA.");
		return 0;
	}

	for(i=0; i<15; i++)
	{
		if(0 == comma[i])
		{
			print(	DBG_WARNING, 
				"--Failed to check 2SGGA.");
			return 0;
		}
	}

	alm = data[comma[5]] -'0';
	if('1' == data[comma[14]])
	{
		sysmode = MODE_CURSOR_GPS_VAL;
	}
	else if('2' == data[comma[14]])
	{
		sysmode = MODE_CURSOR_BD_VAL;
	}
	else if('3' == data[comma[14]])
	{
		sysmode = MODE_CURSOR_MIX_GPS_VAL;
	}
	else if('4' == data[comma[14]])
	{
		sysmode = MODE_CURSOR_MIX_BD_VAL;
	}

	ctx->inSta[slot-1].sys_mode = sysmode;
	
	if(GET_SATELLITE != ctx->inSta[slot-1].satellite_valid )//卫星不可用时，经纬度、海拔信息不使用
	{
		memset(&data[comma[1]],0,7);
		memset(&data[comma[3]],0,8);
		memset(&data[comma[8]],0,comma[9]-comma[8]-1);
	}

	ctx->inSta[slot-1].satcommon.satcomminfo.lat[0] = data[comma[2]];
	memcpy(&(ctx->inSta[slot-1].satcommon.satcomminfo.lat[1]), &data[comma[1]], comma[2]-comma[1]-1);
	
	ctx->inSta[slot-1].satcommon.satcomminfo.lon[0] = data[comma[4]];
	memcpy(&(ctx->inSta[slot-1].satcommon.satcomminfo.lon[1]), &data[comma[3]], comma[4]-comma[3]-1);
	ctx->inSta[slot-1].satcommon.satcomminfo.lon[12] = '\0';
	

	memset(ctx->inSta[slot-1].satcommon.satcomminfo.elev, 0, 8);
	memcpy(ctx->inSta[slot-1].satcommon.satcomminfo.elev, &data[comma[8]], comma[9]-comma[8]-1);
	
	ctx->inSta[slot-1].satcommon.gbs = alm;
	
	
	memcpy(&(ctx->inSta[slot-1].satcommon.rcvrVer[0]), RCVRVER, 4);

	memcpy(ctx->inSta[slot-1].satcommon.rcvrType,RCVRTYPE, 5);
	ctx->inSta[slot-1].satcommon.satcomminfo.sysMode = sysmode;
	
	ctx->inSta[slot-1].satcommon.elevThd = 5;

	/*sprintf( (char *)ctx->inSta[slot-1].gpsbf.utc, 
			 "%04d-%02d-%02d %02d:%02d:%02d", 
			 utc.year+2000, utc.month, utc.day, 
			 utc.hour, utc.minute, utc.second );*/	
	return 1;
}

int STA_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	//static int old_state,new_state;
	u8_t antenna_state = '0';
	u8_t traceGBNum = 0;
	u8_t pst_status;//定位状态
	u8_t xor;
	
	u8_t comma[128];
	int timezone;
	//u8_t time_valid = 0;
	//u16_t clkSta;
	static bool_t fresh[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	static int pps_num[INPUT_SLOT_CURSOR_ARRAY_SIZE], pps_unused_num[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	static bool_t lps_valid[INPUT_SLOT_CURSOR_ARRAY_SIZE];

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
			return 0;
	}

	len = strlen((const char *)data);

	if('*' != data[len-3])
	{
		print(	DBG_WARNING,
			"--Failed to check 2SRMC.");
		return 0;
	}
	
	if((data[len-2] >= '0') && (data[len-2] <= '9'))
	{
		xor = 16 *(data[len-2]-0x30);
	}
	else
	{
		xor = 16 *(data[len-2]-0x37);
	}
	if((data[len-1] >= '0') && (data[len-1] <= '9'))
	{
		xor += data[len-1] -0x30;
	}
	else
	{
		xor += data[len-1] -0x37;
	}
	
	if(xor != data_xor(&data[1], len-4))
	{
		print(	DBG_WARNING, 
				"--sta data: %s",
				data);

		print(	DBG_WARNING, 
				"--Failed to check 2SRMC.");

		return 0;
	}

	//$--STA,hhmmss.sss,yyyy.mm.dd,sppZZzz,t,p,ddmm.mmmm,a,dddmm.mmmm,a,hhhhh.hh,a,vv,cr*hh<CR><LF>
	memset(comma, 0, 128);
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}

	if(13 != j)
	{
		print(	DBG_WARNING,
				"--Failed to check 2SRMC.");
		return 0;
	}

	for(i=0; i<13; i++)
	{
		if(0 == comma[i])
		{
			print(	DBG_WARNING, 
					"--Failed to check 2SRMC.");
			return 0;
		}
	}
	
	#if 1
	//can move to Time_Refresh_Strategy,but not support set gps truely status to led
	if((data[comma[3]] == '1') || (data[comma[3]] == '5'))
	{
			if(pps_num[slot-1] == 60)
			{
				ctx->inSta[slot-1].satellite_valid = GET_SATELLITE;
				
				fresh[slot-1] = false;//卫星重新连接
				//if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_VALID, 0x0))
				//{
				//	return 0;
				//}
				

			}
			else if(pps_num[slot-1] == 70)
			{
				if(fresh[slot-1] == false)
				{
					#if 1  //此处可以让时间在锁定前提前出来//但是为了统一刷新时间，可由选源处或者钟状态变化时刷新，时间会被推迟刷新
					print(DBG_INFORMATIONAL, "Satellite pps ok");
					if(0 == FreshTime_no_leapSecond(ctx))
					{
						return 0;
					}
					#endif
					
					fresh[slot-1] = true;
				}
				#if 0
				write_fresh_state(GET_SATELLITE,ctx);
				#endif
			}
		pps_unused_num[slot-1] = 0;
		pps_num[slot-1]++;		
		
	}else
	{
			if(pps_unused_num[slot-1] == 60)
			{
				#if 0
                write_fresh_state(LOSS_SATELLITE,ctx);
				#endif
				ctx->inSta[slot-1].satellite_valid = LOSS_SATELLITE;

				//if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_VALID, 0x01))
				//{
				//	return 0;
				//}
			}
		pps_num[slot-1] = 0;		
		pps_unused_num[slot-1]++;
	}
	#endif
	if((data[comma[3]] == '4') || (data[comma[3]] == '5'))
	{
		lps_valid[slot-1] = true;
	}else
	{
		lps_valid[slot-1] = false;
	}
	

	
	pst_status = data[comma[4]] -'0';//0-未定位；1:告警；2-2d模式；3-3d模式；4-位置保持
	traceGBNum = 10*(data[comma[11]] -'0') +(data[comma[11] + 1] -'0');
	antenna_state = data[comma[10]];

	//ctx->inSta[slot-1].antenna_state = antenna_state;
	//比逊卫星模块bid:0x63
	switch (antenna_state)
		{
		case '2':
			ctx->inSta[slot-1].antenna_state = SATELLITE_ANTENNA_NORMAL;
			break;
		case '3':
			ctx->inSta[slot-1].antenna_state = SATELLITE_ANTENNA_LOOP;
			break;
		case '4':
			if((data[comma[3]] == '1' || data[comma[3]] == '5')) //1pps valid.
				ctx->inSta[slot-1].antenna_state = SATELLITE_ANTENNA_NORMAL;
			else
				ctx->inSta[slot-1].antenna_state = SATELLITE_ANTENNA_OPEN;
			break;
	}
	
	timezone = 10*(data[comma[2] + 3] -'0') +(data[comma[2] + 4] -'0');

	//only 6 sat type
		ctx->inSta[slot-1].satcommon.fdrStatus = antenna_state;
		ctx->inSta[slot-1].satcommon.trackngb = traceGBNum;
		ctx->inSta[slot-1].satcommon.pst_status = pst_status;

	if(GET_SATELLITE != ctx->inSta[slot-1].satellite_valid)
	{
			memset(&(ctx->inSta[slot-1].satcommon.vBd),0,4*sizeof(struct starinfo));
			ctx->inSta[slot-1].satcommon.nBd = 0;
			ctx->inSta[slot-1].satcommon.nGps =0;
			memset(&(ctx->inSta[slot-1].satcommon.vGps),0,4*sizeof(struct starinfo));
	}
	else
	{	
			if(ctx->inSta[slot-1].satcommon.satcomminfo.sysMode == MODE_CURSOR_GPS_VAL)
			{
				memset(&(ctx->inSta[slot-1].satcommon.vBd),0,4*sizeof(struct starinfo));
				ctx->inSta[slot-1].satcommon.nBd = 0;
			}
			else if(ctx->inSta[slot-1].satcommon.satcomminfo.sysMode == MODE_CURSOR_BD_VAL)
			{
				ctx->inSta[slot-1].satcommon.nGps =0;
				memset(&(ctx->inSta[slot-1].satcommon.vGps),0,4*sizeof(struct starinfo));
			}
	}
	
	return 1;
}

/*
  1	成功
  0	失败
*/
int RMC_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	u8_t xor;
	u8_t comma[128];

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
		return 0;
	}

	len = strlen((const char *)data);

	if('*' != data[len-3])
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SRMC.");
		return 0;
	}
	
	if((data[len-2] >= '0') && (data[len-2] <= '9'))
	{
		xor = 16 *(data[len-2]-0x30);
	}
	else
	{
		xor = 16 *(data[len-2]-0x37);
	}
	if((data[len-1] >= '0') && (data[len-1] <= '9'))
	{
		xor += data[len-1] -0x30;
	}
	else
	{
		xor += data[len-1] -0x37;
	}
	
	if(xor != data_xor(&data[1], len-4))
	{
		print(	DBG_WARNING, 
				"--rmc data: %s",
				data);

		print(	DBG_WARNING, 
				"--Failed to check 2SRMC.");

		return 0;
	}

	//$2SRMC,020845.00,A,3032.49927,N,10403.16411,E,,,010108,,,4*58
	memset(comma, 0, 128);
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}

	if(12 != j)
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SRMC.");
		return 0;
	}

	for(i=0; i<12; i++)
	{
		if(0 == comma[i])
		{
			print(	DBG_WARNING, 
					"--Failed to check 2SRMC.");
			return 0;
		}
	}

	//only 6 sat type
	ctx->inSta[slot-1].satcommon.workMode = data[comma[1]];

	return 1;
}







/*
  1	成功
  0	失败
*/
int GSV_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	u8_t xor;
	u8_t comma[128];
	u8_t page = 0;
	u8_t nGps = 0;

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
		return 0;
	}

	struct starinfo vGps[MAX_GPS_STAR];
	/*
	$GPGSV,3,1,09,02,49,299,31,05,32,210,35,06,53,023,34,09,20,081*54
	$GPGSV,3,2,09,12,44,298,30,17,39,104,31,19,61,075,32,23,10,055,19*7D
	$GPGSV,3,3,09,25,10,322,26*41
	*/
#if 0
	static int te = 0;
	unsigned char *buf[3] = {
		(unsigned char *)"$GPGSV,3,1,09,02,49,299,31,05,32,210,35,06,53,023,34,09,20,081*54\0",
		(unsigned char *)"$GPGSV,3,2,09,12,44,298,30,17,39,104,31,19,61,075,32,23,10,055,19*7D\0",
		(unsigned char *)"$GPGSV,3,3,09,25,10,322,26*41\0",
	};
	if (te == 3){
		te = 0;
	}
	
	
	data = buf[te];
	te++;
#endif


	len = strlen((const char *)data);

	if('*' != data[len-3])
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SGSV.");
		return 0;
	}
	
	if((data[len-2] >= '0') && (data[len-2] <= '9'))
	{
		xor = 16 *(data[len-2]-0x30);
	}
	else
	{
		xor = 16 *(data[len-2]-0x37);
	}
	if((data[len-1] >= '0') && (data[len-1] <= '9'))
	{
		xor += data[len-1] -0x30;
	}
	else
	{
		xor += data[len-1] -0x37;
	}
	
	if(xor != data_xor(&data[1], len-4))
	{
		print(	DBG_WARNING, 
				"--gsv data: %s",
				data);
		
		print(	DBG_WARNING, 
				"--Failed to check 2SGSV.");

		return 0;
	}

	//       0 1 2  3  4  5   6  7  8  9   10 11 12 13  14 15 16 17  18
	//$2SGSV,2,1,06,02,25,242,43,04,61,259,47,10,26,178,42,17,59,013,47*04 //19','
	//$2SGSV,2,1,06,02,25,242,43,04,61,259,47,10,26,178,42*04 //15','
	//$2SGSV,2,2,06,20,33,060,44,28,56,145,45*05 //11','
	//$2SGSV,2,2,06,20,33,060,44*05 //7','
	//$2SGSV,2,2,00*05 //3','
	memset(comma, 0, sizeof(comma));
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}

	
	
	
	nGps = 10*(data[comma[2]] -'0') +(data[comma[2] +1] -'0');
	
	if(GET_SATELLITE != ctx->inSta[slot-1].satellite_valid)
	{
		nGps = 0;
	}

	if(nGps > 4 )
	{

		ctx->inSta[slot-1].satellite_state |= BIT(0);

	}
	else if( nGps < 5)
	{
		if( GET_SATELLITE == ctx->inSta[slot-1].satellite_valid )
		{
			ctx->inSta[slot-1].satellite_state &= ~BIT(0);
		}
		
	}
	
	memset(vGps, 0, MAX_GPS_STAR*sizeof(struct starinfo));
	if(0 != nGps)
	{
		page = (data[comma[1]] -'0') -1;

		if(0 != comma[3])
		{
			vGps[4*page].id = 10*(data[comma[3]]-'0')+(data[comma[3]+1]-'0');
			if(data[comma[6]] == ',')
			{
				vGps[4*page].snr = 255;
			}else if(comma[6] == 0)
			{
				vGps[4*page].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vGps[4*page].snr = 10*(data[comma[6]]-'0')+(data[comma[6]+1]-'0');
			}
				
		}
		if(0 != comma[7])
		{
			vGps[4*page +1].id = 10*(data[comma[7]]-'0')+(data[comma[7]+1]-'0');
			if(data[comma[10]] == ',')
			{
				vGps[4*page +1].snr = 255;
			}else if(comma[10] == 0)
			{
				vGps[4*page +1].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vGps[4*page +1].snr = 10*(data[comma[10]]-'0')+(data[comma[10]+1]-'0');
			}
			
		}
		if(0 != comma[11])
		{
			vGps[4*page +2].id = 10*(data[comma[11]]-'0')+(data[comma[11]+1]-'0');
			if(data[comma[14]] == ',')
			{
				vGps[4*page +2].snr = 255;
			}else if(comma[14] == 0)
			{
				vGps[4*page +2].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vGps[4*page +2].snr = 10*(data[comma[14]]-'0')+(data[comma[14]+1]-'0');
			}
			
		}
		if(0 != comma[15])
		{
			vGps[4*page +3].id = 10*(data[comma[15]]-'0')+(data[comma[15]+1]-'0');
			if(data[comma[18]] == ',')
			{
				vGps[4*page +3].snr = 255;
			}else if(comma[18] == 0)
			{
				vGps[4*page +3].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vGps[4*page +3].snr = 10*(data[comma[18]]-'0')+(data[comma[18]+1]-'0');
			}
			
		}
	}

	//only 6 sat type
	ctx->inSta[slot-1].satcommon.trackMode = 'G';
	ctx->inSta[slot-1].satcommon.nGps = nGps;
	memcpy(&(ctx->inSta[slot-1].satcommon.vGps[4*page]), &vGps[4*page], 4*sizeof(struct starinfo));

	return 1;
}







/*
  1	成功
  0	失败
*/
int BSV_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	u8_t xor;
	u8_t comma[128];
	u8_t page = 0;
	u8_t nBd = 0;
	struct starinfo vBd[MAX_GPS_STAR];

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
		return 0;
	}

	len = strlen((const char *)data);

	if('*' != data[len-3])
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SBSV.");
		return 0;
	}
	
	if((data[len-2] >= '0') && (data[len-2] <= '9'))
	{
		xor = 16 *(data[len-2]-0x30);
	}
	else
	{
		xor = 16 *(data[len-2]-0x37);
	}
	if((data[len-1] >= '0') && (data[len-1] <= '9'))
	{
		xor += data[len-1] -0x30;
	}
	else
	{
		xor += data[len-1] -0x37;
	}
	
	if(xor != data_xor(&data[1], len-4))
	{
		print(	DBG_WARNING, 
				"--bsv data: %s",
				data);
		
		print(	DBG_WARNING, 
				"--Failed to check 2SBSV.");

		return 0;
	}

	//       0 1 2  3  4  5   6  7  8  9   10 11 12 13  14 15 16 17  18
	//$2SBSV,2,1,06,02,25,242,43,04,61,259,47,10,26,178,42,17,59,013,47*04 //19','
	//$2SBSV,2,1,06,02,25,242,43,04,61,259,47,10,26,178,42*04 //15','
	//$2SBSV,2,2,06,20,33,060,44,28,56,145,45*05 //11','
	//$2SBSV,2,2,06,20,33,060,44*05 //7','
	//$2SBSV,2,2,00*05 //3','
	memset(comma, 0, sizeof(comma));
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}
	
	
	
	nBd = 10*(data[comma[2]] -'0') +(data[comma[2] +1] -'0');
	
	if(GET_SATELLITE != ctx->inSta[slot-1].satellite_valid)
	{
		nBd = 0;	
	}

	if(nBd > 4)
	{
		ctx->inSta[slot-1].satellite_state |= BIT(1);	
	}
	else if(nBd < 5)
	{
		if(GET_SATELLITE == ctx->inSta[slot-1].satellite_valid )
		{
			ctx->inSta[slot-1].satellite_state &= ~BIT(1);
		}		
	}
	
	memset(vBd, 0, MAX_GPS_STAR*sizeof(struct starinfo));
	if(0 != nBd)
	{
		page = (data[comma[1]] -'0') -1;

		if(0 != comma[3])
		{
			vBd[4*page].id = 10*(data[comma[3]]-'0')+(data[comma[3]+1]-'0');
			if(data[comma[6]] == ',')
			{
				vBd[4*page].snr = 255;
			}else if(comma[6] == 0)
			{
				vBd[4*page].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vBd[4*page].snr = 10*(data[comma[6]]-'0')+(data[comma[6]+1]-'0');
			}
			
		}
		if(0 != comma[7])
		{
			vBd[4*page +1].id = 10*(data[comma[7]]-'0')+(data[comma[7]+1]-'0');
			if(data[comma[10]] == ',')
			{
				vBd[4*page +1].snr = 255;
			}else if(comma[10] == 0)
			{
				vBd[4*page +1].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vBd[4*page +1].snr = 10*(data[comma[10]]-'0')+(data[comma[10]+1]-'0');
			}
			
		}
		if(0 != comma[11])
		{
			vBd[4*page +2].id = 10*(data[comma[11]]-'0')+(data[comma[11]+1]-'0');
			if(data[comma[14]] == ',')
			{
				vBd[4*page +2].snr = 255;
			}else if(comma[14] == 0)
			{
				vBd[4*page +2].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vBd[4*page +2].snr = 10*(data[comma[14]]-'0')+(data[comma[14]+1]-'0');
			}
			
		}
		if(0 != comma[15])
		{
			vBd[4*page +3].id = 10*(data[comma[15]]-'0')+(data[comma[15]+1]-'0');
			if(data[comma[18]] == ',')
			{
				vBd[4*page +3].snr = 255;
			}
			else if(comma[18] == 0)
			{
				vBd[4*page +3].snr = 255;//无最后一个信噪比时的情况
			}
			else
			{
				vBd[4*page +3].snr = 10*(data[comma[18]]-'0')+(data[comma[18]+1]-'0');
			}
			
		}
	}

	//only 6 sat type
	ctx->inSta[slot-1].satcommon.trackMode = 'B';
	ctx->inSta[slot-1].satcommon.nBd = nBd;
	memcpy(&(ctx->inSta[slot-1].satcommon.vBd[4*page]), &vBd[4*page], 4*sizeof(struct starinfo));

	return 1;
}


int ZDA_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	u8_t comma[128];
	
	//u8_t timezone = 0;

	struct timeinfo utc;

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
		return 0;
	}

	len = strlen((const char *)data);
	//printf("#########data:%s\n",data);
	memset(comma, 0, sizeof(comma));
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}
	
	utc.hour = 10*(data[comma[0]] -'0') +(data[comma[0] + 1] -'0');
	utc.minute = 10*(data[comma[0] + 2] -'0') +(data[comma[0] + 3] -'0');
	utc.second = 10*(data[comma[0] + 4] -'0') +(data[comma[0] + 5] -'0');
	utc.day = 10*(data[comma[1]] -'0') +(data[comma[1] + 1] -'0');
	utc.month = 10*(data[comma[2]] -'0') +(data[comma[2] + 1] -'0');
	utc.year = (u16_t)1000*(data[comma[3]] -'0') + 100*(data[comma[3] + 1] -'0')+ 10*(data[comma[3] + 2] -'0') +(data[comma[3] + 3] -'0');
	//timezone = 10*(data[comma[4]] -'0') +(data[comma[4] + 1] -'0');
	#if 1
	//only 6 sat type
	sprintf( (char *)ctx->inSta[slot-1].satcommon.utc, 
	 "%04d-%02d-%02d %02d:%02d:%02d", 
	 utc.year, utc.month, utc.day, 
	 utc.hour, utc.minute, utc.second );
	#endif
	return 1;
}






/*
  1	成功
  0	失败
*/

int LPS_Parser(struct inCtx *ctx, int slot, u8_t *data)
{
	int len;
	u8_t i,j;
	u8_t xor;
	u8_t comma[128];
	//u8_t lp_state;
	//u32_t tmp;
	struct timeinfo lps_time;
	//struct timeval tv;
	
	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE){
		return 0;
	}

	len = strlen((const char *)data);
	
	if('*' != data[len-3])
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SLPS.");
		return 0;
	}
	
	if((data[len-2] >= '0') && (data[len-2] <= '9'))
	{
		xor = 16 *(data[len-2]-0x30);
	}
	else
	{
		xor = 16 *(data[len-2]-0x37);
	}
	if((data[len-1] >= '0') && (data[len-1] <= '9'))
	{
		xor += data[len-1] -0x30;
	}
	else
	{
		xor += data[len-1] -0x37;
	}
	
	if(xor != data_xor(&data[1], len-4))
	{
		print(	DBG_WARNING, 
				"--lps data: %s",
				data);
		
		print(	DBG_WARNING, 
				"--Failed to check 2SLPS.");

		return 0;
	}

	//        0               1  2  3    4    
	//$BDLPS,10200000000,235960,30,06,2015*5E<CR><LF>//5','
	//$GPLPS,21100000000,235958,31,12,2018*4C<CR><LF>
	memset(comma, 0, sizeof(comma));
	for(i=0,j=0; i<len; i++)
	{
		if(',' == data[i])
		{
			comma[j] = i+1;
			j++;
		}
	}
	
	if(5 != j)
	{
		print(	DBG_WARNING, 
				"--Failed to check 2SLPS.");
		return 0;
	}
	
	
	for(i=0; i<5; i++)
	{
		if(0 == comma[i])
		{
			print(	DBG_WARNING, 
					"--Failed to check 2SLPS.");
			return 0;
		}
	}
	
	
	if(	(BID_GPSBF == ctx->inSta[slot-1].boardId) || 
		(BID_GPSBE == ctx->inSta[slot-1].boardId) || 
		(BID_GBDBF == ctx->inSta[slot-1].boardId) || 
		(BID_GBDBE == ctx->inSta[slot-1].boardId) || 
		(BID_BDBF == ctx->inSta[slot-1].boardId) || 
		(BID_BDBE == ctx->inSta[slot-1].boardId) )
	{
		
		ctx->inSta[slot-1].satcommon.leapstate = (data[comma[0]] - '0');

		ctx->inSta[slot-1].satcommon.leapSecond =  16*(data[comma[0]+1] - '0') + (data[comma[0]+2] - '0');
		if(ctx->inSta[slot-1].satcommon.leapSecond > 2&& ctx->inSta[slot-1].satcommon.leapSecond < 15)
		{
			ctx->inSta[slot-1].satcommon.leapSecond += 14;
		}

		/*
		if( (ctx->lp_new.leapSecond > 0) &&(ctx->lp_old.leapSecond != ctx->lp_new.leapSecond) && (lps_valid) && ( GET_SATELLITE == ctx->satellite1_valid))
		{
			//ctx->lp_new.leapSecond = ctx->lp_new.leapSecond/16*10 + ctx->lp_new.leapSecond%16;
			if(!FpgaWrite(ctx->fpga_fd, FPGA_LPS, ctx->lp_new.leapSecond))//闰秒刷新 16进制
			{
				return 0;
			}
			
			if(!FpgaWrite(ctx->fpga_fd, FPGA_S01_FRESH_TIME, 0x0000))//1号槽时间刷新
			{
				return 0;
			}
			
			if(!FpgaWrite(ctx->fpga_fd, FPGA_S01_FRESH_TIME, 0x0001))
			{
				return 0;
			}
			
			if(!FpgaWrite(ctx->fpga_fd, FPGA_S02_FRESH_TIME, 0x0000))//2号槽时间刷新
			{
				return 0;
			}
			
			if(!FpgaWrite(ctx->fpga_fd, FPGA_S02_FRESH_TIME, 0x0001))
			{
				return 0;
			}*/
			memset(&lps_time,0x00,sizeof(struct timeinfo));
			lps_time.hour = 10*(data[comma[1]] -'0') +(data[comma[1] + 1] -'0');
		
			lps_time.minute = 10*(data[comma[1] + 2] -'0') +(data[comma[1] + 3] -'0');
			
			lps_time.second = 10*(data[comma[1] + 4] -'0') +(data[comma[1] + 5] -'0');
			
			lps_time.day = 10*(data[comma[2]] -'0') +(data[comma[2] + 1] -'0');
			
			lps_time.month = 10*(data[comma[3]] -'0') +(data[comma[3] + 1] -'0');
			
			lps_time.year = 1000*(data[comma[4]] -'0') + 100*(data[comma[4] + 1] -'0')+ 10*(data[comma[4] + 2] -'0') +(data[comma[4] + 3] -'0');

			#if 0
			sprintf(ctx->lp_new.leaptime,"%d-%d-%d %d:%d:%d",
			lps_time.year,
			lps_time.month,
			lps_time.day,
			lps_time.hour,
			lps_time.minute,
			lps_time.second);
			memcpy(ctx->lp_set.leaptime,ctx->lp_new.leaptime,sizeof(ctx->lp_new.leaptime));
			#else
			memcpy(&ctx->inSta[slot-1].satcommon.leaptime,&lps_time,sizeof(struct timeinfo));
			//memcpy(&ctx->lp_old.leaptime,&lps_time,sizeof(struct timeinfo));
			#endif
		//}
		
	}
	else
	{
		return 0;
	}
	
	return 1;
}

/*
  1	成功
  0	失败
*/
int ReadSatellite(struct inCtx *ctx)
{
	int i;
	int len;
	int slot;
	u16_t temp;
	u8_t buffer[128];
	u8_t bid;
	
	//slot 1-4
	//slot = 1;
	for (slot = INPUT_SLOT_CURSOR_1+1; slot < INPUT_SLOT_CURSOR_ARRAY_SIZE+1; ++slot)
	{
		bid = ctx->inSta[slot-1].boardId;
		if( BID_GPSBF == bid || 
			BID_GPSBE == bid || 
			BID_GBDBF == bid || 
			BID_GBDBE == bid || 
			BID_BDBF == bid || 
			BID_BDBE == bid )
		{
			memset(ctx->satelliteBuffer, 0, GB_BUF_LEN +1);
			
			for(i=128; i<GB_BUF_LEN; i++)
			{
				if(!FpgaWrite(ctx->fpga_fd, FPGA_IN_GB_RADDR(slot), i))//接收机读地址
				{
					return 0;
				}
				
				if(!FpgaWrite(ctx->fpga_fd, FPGA_IN_GB_REN(slot), 0x0000))//1号接收机读使能
				{
					return 0;
				}
				if(!FpgaWrite(ctx->fpga_fd, FPGA_IN_GB_REN(slot), 0x0001))//1号接收机读使能
				{
					return 0;
				}
				
				if(FpgaRead(ctx->fpga_fd, FPGA_IN_GB_RVALUE(slot), &temp))//接收机读数据
				{
					ctx->satelliteBuffer[i] = temp &0x00FF;
				}
				else
				{
					return 0;
				}
			}

			
			for(i=128; i<GB_BUF_LEN; i+=128)
			{
				memset(buffer, 0, sizeof(buffer));
				if('$' != ctx->satelliteBuffer[i])
				{
					len = strlen_r((char *)&ctx->satelliteBuffer[i+1], 0x0D);
					if(len >= 128)
	 					len = 127;
					memcpy(buffer, &ctx->satelliteBuffer[i+1], len);
				}
				else
				{
					len = strlen_r((char *)&ctx->satelliteBuffer[i], 0x0D);
					if(len >= 128)
	 					len = 127;
					memcpy(buffer, &ctx->satelliteBuffer[i], len);
				}
					
				print(	DBG_DEBUG, 
						"%d--%s", slot, buffer);
				
				if(0 == *buffer)
				{
					ctx->inSta[slot-1].satellite_valid = LOSS_SATELLITE;
				}

				if(ctx->inSta[slot-1].satcommon.sta_flag != FLAG_SAT_ALL_COMMSTA){
					print(DBG_ERROR, "satellite should support all variable in the sturct satcomm!");
				}
				if(ctx->inSta[slot-1].satcommon.satcomminfo.info_flag != FLAG_SAT_ALL_COMMINFO){
					print(DBG_ERROR, "satellite should support all variable in the sturct satcomminfo!");
				}
				//协议解析
				if((memcmp(buffer,"$GNGGA",6) == 0) || (memcmp(buffer,"$GPGGA",6) == 0)|| (memcmp(buffer,"$BDGGA",6) == 0))
				{
					GGA_Parser(ctx, slot, buffer);//位置信息
				}
				else if((memcmp(buffer,"$GNSTA",6) == 0) || (memcmp(buffer,"$GPSTA",6) == 0)|| (memcmp(buffer,"$BDSTA",6) == 0))
				{
					STA_Parser(ctx, slot, buffer);//配置信息
				}
				else if(0 == memcmp(buffer, "$GPGSV", 6))
				{
					GSV_Parser(ctx, slot, buffer);//可视卫星状态输出语句
				}
				else if(0 == memcmp(buffer, "$BDGSV", 6))
				{
					BSV_Parser(ctx, slot, buffer);//北斗卫星信息
				}
				else if((memcmp(buffer,"$GNZDA",6) == 0) || (memcmp(buffer,"$GPZDA",6) == 0)|| (memcmp(buffer,"$BDZDA",6) == 0))
				{
					ZDA_Parser(ctx, slot, buffer);
				}
				else if((memcmp(buffer,"$GNLPS",6) == 0) || (memcmp(buffer,"$GPLPS",6) == 0)|| (memcmp(buffer,"$BDLPS",6) == 0))
				{
					LPS_Parser(ctx, slot, buffer);
				}
				else if((memcmp(buffer,"$GNRMC",6) == 0) || (memcmp(buffer,"$GPRMC",6) == 0)|| (memcmp(buffer,"$BDRMC",6) == 0))
				{
					RMC_Parser(ctx, slot, buffer);//最简导航传输数据
				}
				else
				{
					//do nothing
				}
			}
		}
	}
	return 1;
}

