#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_fpga.h"
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_log.h"
#include "lib_time.h"

#include "addr.h"
#include "alloc.h"
#include "din_config.h"
#include "din_source.h"


int FreshTime(struct inCtx *ctx, int leapSecond)
{

	int fpga_fd = -1;
	fpga_fd = ctx->fpga_fd;

	if( gTimeSourceTbl[ctx->cs.src_inx].flag == FREQ_SOURCE_FLAG){
		print(DBG_WARNING,"FreshTime: Current source:%s not support flush time!\n", 
			gTimeSourceTbl[ctx->cs.src_inx].msg);
		return 1;
	}

	if(!FreshLeapSecond(fpga_fd,leapSecond))//闰秒刷新
	{
		return 0;
	}

	if(!FpgaWrite(fpga_fd, FPGA_SYS_FRESH_TIME, 0x0000))//时间刷新
	{
		return 0;
	}

	if(!FpgaWrite(fpga_fd, FPGA_SYS_FRESH_TIME, 0x0001))
	{
		return 0;
	}

	print(DBG_INFORMATIONAL," --din: freshLeapSecond(%d) and FreshTime\n",leapSecond);

	
	return 1;
}

int FreshTime_no_leapSecond(struct inCtx *ctx)
{
	int fpga_fd = -1;
	fpga_fd = ctx->fpga_fd;

	if( gTimeSourceTbl[ctx->cs.src_inx].flag == FREQ_SOURCE_FLAG){
		print(DBG_WARNING, "FreshTime_no_leapSecond: Current source:%s not support flush time!\n", 
			gTimeSourceTbl[ctx->cs.src_inx].msg);
		return 1;
	}

		if(!FpgaWrite(fpga_fd, FPGA_SYS_FRESH_TIME, 0x0000))//时间刷新
		{
			return 0;
		}
	
		if(!FpgaWrite(fpga_fd, FPGA_SYS_FRESH_TIME, 0x0001))
		{
			return 0;
		}

	print(DBG_INFORMATIONAL, " --din: FreshTime no leapSecond\n");
	return 1;
}

int FreshLeapSecond(int fpga_fd,int leapSecond)
{
	if(!FpgaWrite(fpga_fd, FPGA_SYS_LPS, leapSecond))//闰秒刷新
	{
		return 0;
	}

	print(DBG_INFORMATIONAL, " --din: FreshLeapSecond(%d)\n",leapSecond);
	return 1;
}



/*
  best input source
*/
struct bisinfo {
	u8_t	bis_enable[TIME_SOURCE_VALID_LEN];
	u8_t	bis_alm[TIME_SOURCE_VALID_LEN];//4SLOT
	u8_t	bis_ssm[TIME_SOURCE_VALID_LEN];
	u8_t	bis_prio[TIME_SOURCE_VALID_LEN];
	u8_t	bis_flag[TIME_SOURCE_VALID_LEN];//bothtime,time,freq
};

/*
  0~10
*/
#define USE_PORT_ATTR
#define USE_INCOMM_PRO
int bis_free(struct inCtx *ctx)
{
	int i;
	int slot = INPUT_SLOT_CURSOR_1+1;
	struct bisinfo bis;//best input source
	u8_t	tmp_bis_ssm = 0x00;
	u8_t line;
	int min;
	int alg[TIME_SOURCE_VALID_LEN];

	memset(&bis, 0, sizeof(struct bisinfo));

	u8_t src_type = NO_SOURCE_TYPE;
	u8_t src_slot = 0;
	u8_t src_flag = INVALID_SOURCE_FLAG;
#if 0
#ifdef USE_PORT_ATTR
	int max_port = 0;
	int port = 0;
	struct port_attr_t *port_attr = NULL;
#else
	u32_t supported_source = BIT(NO_SOURCE_TYPE);
#endif

	for (slot = INPUT_SLOT_CURSOR_1+1; slot < INPUT_SLOT_CURSOR_ARRAY_SIZE+1; ++slot){
		#ifdef USE_PORT_ATTR
		FIND_PORTATTR_GBATBL_BY_BID(ctx->inSta[slot-1].boardId, port_attr);
		if(!port_attr)
			continue;
		FIND_MAXPORT_GBATBL_BY_BID(ctx->inSta[slot-1].boardId, max_port);
		#else
		supported_source = BIT(NO_SOURCE_TYPE);
		FIND_SUP_SRC_GBATBL_BY_BID(ctx->inSta[slot-1].boardId, supported_source);
		#endif
		for(i=0; i<TIME_SOURCE_VALID_LEN; i++){
			src_type = NO_SOURCE_TYPE;
			src_slot = 0;
			FIND_TYPE_BY_ID(i, src_type);
			FIND_SLOT_BY_ID(i, src_slot);
			if (src_slot == slot){
				#ifdef USE_PORT_ATTR
				for (port = 0; port < max_port; ++port){
					if (port_attr[port].insrc_type == src_type){
						bis.bis_enable[i] = 1;
					}
					#ifdef USE_INCOMM_PRO
						//要求制定盘每个端口的选源信号不可以定义一样，如若有相同信号，也应该定义不同选源，如irigb，应该定义成irigb1和irigb2
						bis.bis_prio[i] = ctx->inSta[slot-1].incommsta.incomminfo.prio[port];
					#endif
				}
				#else
				if(BIT(src_type)&supported_source)
					bis.bis_enable[i] = 1;
				#endif
			}
			printf("time_src:%d boardid:%d bit_src_type:%d bis_enable:%d\n",
			i,ctx->inSta[src_slot-1].boardId, BIT(src_type),
			bis.bis_enable[i]);
		}

	}
#else
#ifdef USE_PORT_ATTR
		int max_port = 0;
		int port = 0;
		struct port_attr_t *port_attr = NULL;
#else
		u32_t supported_source = BIT(NO_SOURCE_TYPE);
#endif

	for(i=0; i<TIME_SOURCE_VALID_LEN; i++){
		src_type = NO_SOURCE_TYPE;
		src_slot = 0;
		FIND_TYPE_BY_ID(i, src_type);
		FIND_SLOT_BY_ID(i, src_slot);
		FIND_FLAG_BY_ID(i, src_flag);
		bis.bis_flag[i] = src_flag;
		if(src_slot > INPUT_SLOT_CURSOR_1 && src_slot <= INPUT_SLOT_CURSOR_ARRAY_SIZE){
			#ifdef USE_PORT_ATTR
			FIND_PORTATTR_GBATBL_BY_BID(ctx->inSta[src_slot-1].boardId, port_attr);
			if(!port_attr)
				continue;
			FIND_MAXPORT_GBATBL_BY_BID(ctx->inSta[src_slot-1].boardId, max_port);
			
			for (port = 0; port < max_port; ++port){
				if (port_attr[port].insrc_type == src_type){
					bis.bis_enable[i] = 1;//该处可以去掉无效的盘的选源
					#ifdef USE_INCOMM_PRO
					//要求制定盘每个端口的选源信号不可以定义一样，如若有相同信号，也应该定义不同选源，如irigb，应该定义成irigb1和irigb2
					bis.bis_prio[i] = ctx->inSta[src_slot-1].incommsta.incomminfo.prio[port];
					#endif
					bis.bis_ssm[i] = ctx->inSta[src_slot-1].incommsta.incomminfo.ssm[port];
				}
			}
			//printf("time_src:%d boardid:%d bit_src_type:%d bis_enable:%d\n",
			//i, ctx->inSta[src_slot-1].boardId, BIT(src_type),
			//bis.bis_enable[i]);
			#else
			supported_source = BIT(NO_SOURCE_TYPE);
			FIND_SUP_SRC_GBATBL_BY_BID(ctx->inSta[src_slot-1].boardId, supported_source);

			if(BIT(src_type) & supported_source)
				bis.bis_enable[i] = 1;
			//printf("time_src:%d boardid:%d bit_src_type:%d supported_source:%d bis_enable:%d\n",
			//i, ctx->inSta[src_slot-1].boardId, BIT(src_type),supported_source,
			//bis.bis_enable[i]);
			#endif

		}else{
			bis.bis_enable[i] = 0;
		}
	}
#endif

	int alm_inx = -1;
	for (slot = INPUT_SLOT_CURSOR_1+1; slot < INPUT_SLOT_CURSOR_ARRAY_SIZE+1; ++slot){

		for(i=0; i<TIME_SOURCE_VALID_LEN; i++){
			src_type = NO_SOURCE_TYPE;
			src_slot = 0;
			FIND_TYPE_BY_ID(i, src_type);
			FIND_SLOT_BY_ID(i, src_slot);
			if (src_slot == slot){
				alm_inx = -1;
				switch (src_type){
					case GPS_SOURCE_TYPE:
						//输入，天线短路，信号劣化，只有一方有问题，则打开报警，不被选源
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_GPBD_SGN_LOS, src_slot, SAT_PORT_SAT, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}
						
						alm_inx = -1;
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_GPBD_SGN_DGD, src_slot, SAT_PORT_SAT, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}
						
						alm_inx = -1;
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_ANT_SHORT, src_slot, SAT_PORT_SAT, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}
						break;
					case S2MH_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MHZ_IN_LOS, src_slot, SAT_PORT_2MH, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case S2MB_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_IN_LOS, src_slot, SAT_PORT_2MB, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}

						alm_inx = -1;
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_AIS, src_slot, SAT_PORT_2MB, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}
						
						alm_inx = -1;
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_BPV, src_slot, SAT_PORT_2MB, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}

						alm_inx = -1;
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_CRC, src_slot, SAT_PORT_2MB, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}

						alm_inx = -1;
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_2MB_LOF, src_slot, SAT_PORT_2MB, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
							break;
						}
						break;
					case IRIGB1_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, src_slot, SAT_PORT_IRIGB1, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case IRIGB2_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_IRIGB_IN_LOS, src_slot, SAT_PORT_IRIGB2, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case TOD_SOURCE_TYPE:
						switch (ctx->inSta[slot-1].boardId){
							case BID_PTP_IN:
								FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_PTP_IN_LOS, src_slot, 1, alm_inx);
								break;
							case BID_RTF:
								FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_TOD_IN_LOS, src_slot, RTF_PORT_1PPS_TOD, alm_inx);
								break;
							default:
								break;
						}
						break;
					case S10MH1_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, src_slot, RTF_PORT_10MH1, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case S10MH2_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_10MHZ_IN_LOS, src_slot, RTF_PORT_10MH2, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case S1PPS_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_1PPS_IN_LOS, src_slot, RTF_PORT_1PPS, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case NTP_SOURCE_TYPE:
						FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_IN_LOS, src_slot, 1, alm_inx);
						if(alm_inx == -1)
							return TIME_SOURCE_MAX_NOSOURCE;
						break;
					case NO_SOURCE_TYPE:
						break;
					default:
						print(DBG_DEBUG, "Not support source type:%d", src_type);
				}
				if (alm_inx >= 0){
					if (ALM_ON == ctx->alarm[alm_inx].alm_sta){
						bis.bis_alm[i] = ALM_ON;
					}else{
						bis.bis_alm[i] = ALM_OFF;
					}
				}else{
					bis.bis_alm[i] = ALM_ON;//never exec it
				}
			}
		}
	}

	//bis.bis_prio[TOD_SOURCE_ID(1)] = '5';//satellite default:6[2143]

//#define SSM_CHOICE_SRC_ENABLE

	for(i=0; i<TIME_SOURCE_VALID_LEN; i++)
	{
		if( 1 == bis.bis_enable[i] &&
			ALM_OFF == bis.bis_alm[i] &&
			'-' != bis.bis_prio[i] && '0'<= bis.bis_prio[i] )
		{
#ifdef SSM_CHOICE_SRC_ENABLE
			if(SSM_VAL_00 == bis.bis_ssm[i])
				tmp_bis_ssm = SSM_VAL_0F;
			else
				tmp_bis_ssm = bis.bis_ssm[i];
#else
			tmp_bis_ssm = 1;
#endif
			
			alg[i] = (tmp_bis_ssm) *(bis.bis_prio[i] - '0'+1);//prio small, choose it, 0-9, - no choose

			if(bis.bis_flag[i] == FREQ_SOURCE_FLAG){
				if (ctx->clock_type == 1){
					//RB
					if(SSM_VAL_08 == bis.bis_ssm[i] || SSM_VAL_0B == bis.bis_ssm[i] || SSM_VAL_0F == bis.bis_ssm[i])
						alg[i] = 500;
				}else if(ctx->clock_type == 2){
					//XO
					if(SSM_VAL_0F == bis.bis_ssm[i])
						alg[i] = 500;
				}else {
					alg[i] = 500;
				}
			}
		}
		else
		{
			alg[i] = 500;
		}
	}

	min = alg[0];
	line = 0;
	print(DBG_DEBUG, "clk_type: [%d]%s\n",ctx->clock_type, ((ctx->clock_type == 1) ? "RB":"XO/UNKWN"));
	print(DBG_DEBUG, "src:%d %s enable:%d alm:%d prio:%d ssm:%d result:%d\n", 0 , gTimeSourceTbl[0].msg, 
		bis.bis_enable[0], bis.bis_alm[0],bis.bis_prio[0], bis.bis_ssm[0],alg[0]);
	for(i=1; i<TIME_SOURCE_VALID_LEN; i++)
	{
		if(alg[i] < min)
		{
			//foud min
			min = alg[i];
			line = i;
		}
		print(DBG_DEBUG, "src:%d %s enable:%d alm:%d prio:%d ssm:%d result:%d\n", i , gTimeSourceTbl[i].msg, 
			bis.bis_enable[i], bis.bis_alm[i],bis.bis_prio[i], bis.bis_ssm[i],alg[i]);
	}

	if(500 == min)
	{
		return TIME_SOURCE_MAX_NOSOURCE;
	}
	else
	{
		return line;
	}
}

/*
// WRITE SRC TO FPGA
  -1	失败
   0	成功
*/
int bis_force(struct inCtx *ctx, int line)
{
	if(line < 0 || line > TIME_SOURCE_VALID_LEN)
	{
		return -1;
	}
	else
	{
		line = gTimeSourceTbl[line].g_val; //line = NO_SOURCE;
		print(DBG_INFORMATIONAL, "Schema mode:%d, Setting fpga for the cur source[%d]: %s, bid:%d g_val:0x%04x\n",
			ctx->cs.schema, ctx->cs.src_inx, 
			gTimeSourceTbl[ctx->cs.src_inx].msg,
			ctx->cs.bid,
			gTimeSourceTbl[ctx->cs.src_inx].g_val);

		if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_SELECT, line))
		{
			return -1;
		}
	}

	return 0;
}





/*
  -1	失败
   0	成功
*/
int best_input_source(struct inCtx *ctx)
{
	int src = 0;
	u8_t src_type = NO_SOURCE_TYPE;
	u8_t src_flag = INVALID_SOURCE_FLAG;
	int slot = INPUT_SLOT_CURSOR_1+1;
	int max_port = 0, port;
	struct port_attr_t *port_attr = NULL;
	char buf[256];
	
	if(0 == ctx->cs.schema)//选源模式为free
	{
		src = bis_free(ctx);//最佳输入源
		if(ctx->cs.src_inx != src)
		{
			ctx->cs.src_inx = src;

			ctx->cs.slot = 0;
			ctx->cs.bid = BID_NONE;
			ctx->cs.port = 0;
			FIND_TYPE_BY_ID(ctx->cs.src_inx, src_type);
			ctx->cs.type = src_type;
			FIND_FLAG_BY_ID(ctx->cs.src_inx, src_flag);
			ctx->cs.flag = src_flag;
			FIND_SLOT_BY_ID(ctx->cs.src_inx, slot);
			if(slot > INPUT_SLOT_CURSOR_1 && slot <= INPUT_SLOT_CURSOR_ARRAY_SIZE){
				ctx->cs.slot = slot;
				ctx->cs.bid = ctx->inSta[ctx->cs.slot-1].boardId;
				FIND_PORTATTR_GBATBL_BY_BID(ctx->inSta[ctx->cs.slot-1].boardId, port_attr);
				if(!port_attr)
					return -1;
				FIND_MAXPORT_GBATBL_BY_BID(ctx->inSta[ctx->cs.slot-1].boardId, max_port);
				for (port = 0; port < max_port; ++port){
					if (port_attr[port].insrc_type == ctx->cs.type){
						ctx->cs.port = port+1;
					}
				}
			}

			if(-1 == bis_force(ctx, ctx->cs.src_inx))//设置最佳输入源
			{
				return -1;
			}

			//log
			//---------------------------------------------------------

			//if( INIT_SATELLITE != ctx->inSta[0].satellite_valid && INIT_SATELLITE == ctx->inSta[1].satellite_valid)
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%s%s", "Time Source is ", gTimeSourceTbl[ctx->cs.src_inx].msg);
				db_log_write(ctx->pDb, TBL_LOG, buf);
			}
			
			//---------------------------------------------------------
		}
	}

	return 0;
}

int WriteInSourceInfoToShareMemory(struct inCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return -1;
	}
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_SCHEMA, 
			  sizeof(struct schemainfo), 
			  (char *)&(ctx->cs), 
			  sizeof(struct schemainfo));
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return -1;
	}
	return 0;
}












/*
  -1	失败
   0	成功
*/
int stratum_input_source(struct inCtx *ctx)
{
	int slot;
	int cur_gps_slot = 1;//default 1
	int src_port = 1;
	u16_t clk_src_typ = 0x00;
	u16_t clock_source_alarm = 0x0;
	u16_t clock_source_status = 0x0f;

	int pst_status = 0;
	int traceGBNum = 0;

	u16_t stratum = 0x0;
	u8_t ssm = SSM_VAL_NE;
	//u8_t mh_ssm_value = 0x0;

	int tsrc_type = NO_SOURCE_TYPE;
	tsrc_type = ctx->cs.type;

	if (INPUT_SLOT_CURSOR_1+1 > ctx->cs.slot || INPUT_SLOT_CURSOR_ARRAY_SIZE < ctx->cs.slot){
		return 0;
	}

	//读取slot槽对应板卡的ssm时钟等级

	slot = ctx->cs.slot; //cur_src_slot
	src_port = ctx->cs.port;

	if(src_port > 0 && src_port < INPUT_SLOT_MAX_PORT){
		ssm = ctx->inSta[slot-1].incommsta.incomminfo.ssm[src_port-1];
		print(DBG_DEBUG, "src_slot:%d src_port:%d ssm:%02X", slot, src_port, ssm);
	}

	//读取slot槽位时钟等级
	if(	GPS_SOURCE_ID(slot) == ctx->cs.src_inx || 
		IRIGB1_SOURCE_ID(slot) == ctx->cs.src_inx || 
		IRIGB2_SOURCE_ID(slot) == ctx->cs.src_inx ||
		TOD_SOURCE_ID(slot) == ctx->cs.src_inx){
		//GPS, IRIGB, TOD/PTP/1PPS+TOD
		stratum = 0x00;
	}else if(FREQ_SOURCE_FLAG == ctx->cs.flag){
		//2MH
		if (ssm == SSM_VAL_02)
		{
			stratum = 0x00;
			//mh_ssm_value = 0x02;
		}
		else if(ssm == SSM_VAL_04)
		{
			stratum = 0x01;
			//mh_ssm_value = 0x04;
		}
		else if(ssm == SSM_VAL_08)
		{
			stratum = 0x02;
			//mh_ssm_value = 0x08;
		}
		else if(ssm == SSM_VAL_0B)
		{
			stratum = 0x02;
			//mh_ssm_value = 0x0B;
		}
		else if(ssm == SSM_VAL_0F)
		{
			stratum = 0xFF;
			//mh_ssm_value = 0x0F;
		}
		else
		{
			stratum= 0xFF;
			//mh_ssm_value = 0x00;
		}
		#if 1
		if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_CUR_INSRC_SSM, ssm)){
			return -1;
		}
		#endif
	}
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_STRATUM, stratum)){
		return -1;
	}
	print(	DBG_DEBUG, 
		"--input source stratum %02X", 
		stratum	);

	//设置PTP/TOD时钟源类型
	if(	(GPS_SOURCE_TYPE == tsrc_type) && 
		((MODE_CURSOR_BD_VAL == ctx->inSta[slot-1].sys_mode) || (MODE_CURSOR_MIX_BD_VAL == ctx->inSta[slot-1].sys_mode))	){
		clk_src_typ = 0x00;//gps
	}else if(	(GPS_SOURCE_TYPE == tsrc_type) && 
				((MODE_CURSOR_GPS_VAL == ctx->inSta[slot-1].sys_mode) || (MODE_CURSOR_MIX_GPS_VAL == ctx->inSta[slot-1].sys_mode))){
		clk_src_typ = 0x01;//bd
	}else if(TOD_SOURCE_TYPE == tsrc_type){
		clk_src_typ = 0x02;
	}else{
		clk_src_typ = 0x03;//其他
	}

	//TOD/PTP
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_CLK_SRC_TYPE, clk_src_typ))
	{
		return -1;
	}

	//china unicom miit tod
#if 1

	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_INSRC_STRATUM, stratum))
	{
		return -1;
	}

	print(	DBG_DEBUG, 
			"--input source stratum %02X", 
			stratum);
	//for gps
	if (GPS_SOURCE_TYPE == tsrc_type){
		cur_gps_slot = gTimeSourceTbl[ctx->cs.src_inx].slot;
	}	

	if (GPS_SOURCE_TYPE == tsrc_type){
		switch (ctx->inSta[cur_gps_slot-1].antenna_state){
			case SATELLITE_ANTENNA_LOOP:
				clock_source_alarm = BIT(2);
				break;
			case SATELLITE_ANTENNA_OPEN:
				clock_source_alarm = BIT(1);
				break;
		}
		if(MODE_CURSOR_GPS_VAL == ctx->inSta[cur_gps_slot-1].sys_mode){
			clock_source_alarm = BIT(5);
		}

		switch (ctx->inSta[cur_gps_slot-1].boardId){
			 case BID_INSATE_LOWER ... BID_INSATE_UPPER:
				if ((ctx->inSta[cur_gps_slot-1].satcommon.sta_flag & FLAG_SAT_COMMSTA_TRACKNGB) && \
					(ctx->inSta[cur_gps_slot-1].satcommon.sta_flag & FLAG_SAT_COMMSTA_PST_STATUS)){
					//only 6 sat type
					traceGBNum = ctx->inSta[cur_gps_slot-1].satcommon.trackngb;
					pst_status = ctx->inSta[cur_gps_slot-1].satcommon.pst_status;
				}
				break;
		}

		if(traceGBNum == 0)
			clock_source_alarm = BIT(3);

		switch (pst_status)
			{
			case 0://no position
				clock_source_status = 0x0;
				break;
			case 2:	//2d
				clock_source_status = 0x2;
				break;
			case 3:	//3d
				clock_source_status = 0x3;
				break;
			default:
				clock_source_status = 0x0f;
				break;		
			}
	}
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_CLK_SRC_ALARM, clock_source_alarm))
	{
		return 0;
	}

	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_CLK_SRC_STATE, clock_source_status))
	{
		return 0;
	}
#endif
	print(	DBG_DEBUG, 
			"--input source type %02X",
			clk_src_typ	);

	return 0;
}

