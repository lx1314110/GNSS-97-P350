#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_ip1725.h"

#include "alloc.h"
#include "cout_global.h"
#include "cout_query.h"
#include "lib_fpga.h"

extern u8_t gtraceTbl[PTP_TRACE_ARRAY_SIZE][2];




/*
  -1	失败
   0	成功
*/
int in_zone_query(struct outCtx *ctx, int group)
{
	struct inzoneinfo tz;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_IZONE, 
			 sizeof(struct inzoneinfo), 
			 (char *)&tz, 
			 sizeof(struct inzoneinfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	if(group == 0){
		if( ((tz.zone[0] >= TZONE_ZERO) && (tz.zone[0] <= TZONE_W1)) &&
			((tz.zone[1] >= TZONE_ZERO) && (tz.zone[1] <= TZONE_W1)) &&
			((tz.zone[2] >= TZONE_ZERO) && (tz.zone[2] <= TZONE_W1)) &&
			((tz.zone[3] >= TZONE_ZERO) && (tz.zone[3] <= TZONE_W1)) )
		{
			printf( "%s:%s:%s:%s\n", 
					gZoneTbl[tz.zone[0] -'@'], gZoneTbl[tz.zone[1] -'@'], 
					gZoneTbl[tz.zone[2] -'@'], gZoneTbl[tz.zone[3] -'@']);
		}
		else
		{
			return -1;
		}

	}else{
		if( tz.zone[group-1] >= TZONE_ZERO && tz.zone[group-1] <= TZONE_W1){
			printf( "%s\n", gZoneTbl[tz.zone[group-1] -'@']);
		}else{
			return -1;
		}
	}
	
	return 0;
}






/*
  -1	失败
   0	成功
   1-4 group
*/
int out_zone_query(struct outCtx *ctx, int group)
{
	struct outzoneinfo tz;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_OZONE, 
			 sizeof(struct outzoneinfo), 
			 (char *)&tz, 
			 sizeof(struct outzoneinfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	if(group == 0){
		if( ((tz.zone[0] >= TZONE_ZERO) && (tz.zone[0] <= TZONE_W1)) &&
			((tz.zone[1] >= TZONE_ZERO) && (tz.zone[1] <= TZONE_W1)) &&
			((tz.zone[2] >= TZONE_ZERO) && (tz.zone[2] <= TZONE_W1)) &&
			((tz.zone[3] >= TZONE_ZERO) && (tz.zone[3] <= TZONE_W1)) )
		{
			printf( "%s:%s:%s:%s\n", 
					gZoneTbl[tz.zone[0] -'@'], gZoneTbl[tz.zone[1] -'@'], 
					gZoneTbl[tz.zone[2] -'@'], gZoneTbl[tz.zone[3] -'@']);
		}
		else
		{
			return -1;
		}

	}else{
		if( tz.zone[group-1] >= TZONE_ZERO && tz.zone[group-1] <= TZONE_W1){
			printf( "%s\n", gZoneTbl[tz.zone[group-1] -'@']);
		}else{
			return -1;
		}
	}
	
	return 0;
}





/*
  -1	失败
   0	成功
*/
int out_2mb_sa_query(struct outCtx *ctx)
{
	struct out2mbsainfo obsa;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_OBSA, 
			 sizeof(struct out2mbsainfo), 
			 (char *)&obsa, 
			 sizeof(struct out2mbsainfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	printf("%s\n",obsa.mb_sa);
	return 0;
}


/*
  -1	失败
   0	成功
*/
int out_2mb_ssm_query(struct outCtx *ctx)
{
	u16_t ssm = 0x0F;
	char ssmbuf[3];
	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, &ssm))
	{
		return -1;
	}
	memset(ssmbuf, 0x0, sizeof(ssmbuf));
	ssm &= 0x000F;
	switch(ssm)
	{
		case 0x02:
			memcpy(ssmbuf, SSM_02, 2);
			break;
				
		case 0x04:
			memcpy(ssmbuf, SSM_04, 2);
			break;
				
		case 0x08:
			memcpy(ssmbuf, SSM_08, 2);
			break;

		case 0x0B:
			memcpy(ssmbuf, SSM_0B, 2);
			break;

		case 0x0F:
			memcpy(ssmbuf, SSM_0F, 2);
			break;

		case 0x00:
			memcpy(ssmbuf, SSM_00, 2);
			break;

		default:
			memcpy(ssmbuf, SSM_00, 2);
	}


	printf("%s\n",ssmbuf);
	return 0;
}






/*
  -1	失败
   0	成功
   group 0 or 1-4
*/
int out_baudrate_query(struct outCtx *ctx, int group)
{
	struct baudrate tmp;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_OBR, 
			 sizeof(struct baudrate), 
			 (char *)&tmp, 
			 sizeof(struct baudrate));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	if(group == 0){
		if( ((tmp.br[0] >= 0) && (tmp.br[1] < ARRAY_SIZE(gBaudRateTbl))) &&
			((tmp.br[1] >= 0) && (tmp.br[1] < ARRAY_SIZE(gBaudRateTbl))) &&
			((tmp.br[2] >= 0) && (tmp.br[2] < ARRAY_SIZE(gBaudRateTbl))) &&
			((tmp.br[3] >= 0) && (tmp.br[3] < ARRAY_SIZE(gBaudRateTbl))) )
		{
			printf( "%s:%s:%s:%s\n", 
					gBaudRateTbl[tmp.br[0]], gBaudRateTbl[tmp.br[1]], 
					gBaudRateTbl[tmp.br[2]], gBaudRateTbl[tmp.br[3]]);
		}
		else
		{
			return -1;
		}
	}else{
		if( tmp.br[group-1] >= 0 && tmp.br[group-1] < ARRAY_SIZE(gBaudRateTbl)){
			printf( "%s\n", gBaudRateTbl[tmp.br[group-1]]);
		}else{
			return -1;
		}
	}
	return 0;
}













/*
  -1	失败
   0	成功
*/
int irigb_amplitude_ratio_query(struct outCtx *ctx, int group)
{
	struct amplitude tmp;
	int pos[4];
	int i;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_AMP, 
			 sizeof(struct amplitude), 
			 (char *)&tmp, 
			 sizeof(struct amplitude));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	/*fpga pos is 3*/

	if(group == 0){
		for (i = 0; i < ARRAY_SIZE(gAmplitudeRatioTbl); i++){
			pos[i] = tmp.amp[i] - 3;
			if ((pos[i] < 0)||(pos[i] > 3)){
				return -1;
			}
		}
		printf( "%s|%s|%s|%s\n", 
				gAmplitudeRatioTbl[pos[0]], gAmplitudeRatioTbl[pos[1]], 
				gAmplitudeRatioTbl[pos[2]], gAmplitudeRatioTbl[pos[3]]);

	}else{

		pos[group-1] = tmp.amp[group-1] - 3;
		if ((pos[group-1] < 0)||(pos[group-1] >= ARRAY_SIZE(gAmplitudeRatioTbl))){
			return -1;
		}
		printf( "%s\n", gAmplitudeRatioTbl[pos[group-1]]);
	}
	return 0;
}













/*
  -1	失败
   0	成功
*/
int irigb_voltage_query(struct outCtx *ctx, int group)
{
	struct voltage tmp;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_VOL, 
			 sizeof(struct voltage), 
			 (char *)&tmp, 
			 sizeof(struct voltage));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	if(group == 0){
		if(	((tmp.vol[0] >= 0) && (tmp.vol[0] < ARRAY_SIZE(gVoltageTbl))) &&
		((tmp.vol[1] >= 0) && (tmp.vol[1] < ARRAY_SIZE(gVoltageTbl))) &&
		((tmp.vol[2] >= 0) && (tmp.vol[2] < ARRAY_SIZE(gVoltageTbl))) &&
		((tmp.vol[3] >= 0) && (tmp.vol[3] < ARRAY_SIZE(gVoltageTbl))) ){
			printf( "%s|%s|%s|%s\n", 
					gVoltageTbl[tmp.vol[0]], gVoltageTbl[tmp.vol[1]], 
					gVoltageTbl[tmp.vol[2]], gVoltageTbl[tmp.vol[3]]);
		}
		else
		{
			return -1;
		}
	}else{
		if( tmp.vol[group-1] >= 0 && tmp.vol[group-1] < ARRAY_SIZE(gVoltageTbl)){
			printf( "%s\n", gVoltageTbl[tmp.vol[group-1]]);
		}else{
			return -1;
		}
	}
	
	return 0;
}


/*
ip1,ip2,ip3...ip16
192.168.1.45,192.168.2.55,...;
  0	不合法
  1 合法
*/
static int uint32_ip_group_to_str_ip(u32_t *ipv4, u8_t ipv4_num, char *des_ipv4_grp)
{
	int i;
	char str_ip[MAX_IP_LEN];
	char str_ip_group[MAX_IP_LEN*MAX_IP_NUM+1];//注意栈溢出问题，必要时用static

	if(ipv4 == NULL || des_ipv4_grp == NULL || ipv4_num > MAX_IP_NUM)
        return 0;
	memset(str_ip_group, 0x0, sizeof(str_ip_group));
	for (i = 0; i < ipv4_num; ++i){
		//if(sta.ptpin.ptpinInfo.ptpMasterIp[i] == 0)
			//continue;
		memset(str_ip, 0x0, MAX_IP_LEN);
		UINTIP_TO_STRIP(ipv4[i], str_ip);
		sprintf(str_ip_group, "%s%s,", str_ip_group, str_ip);
	}
	if(strlen(str_ip_group)>0)
		str_ip_group[strlen(str_ip_group)-1] = '\0';
	memcpy(des_ipv4_grp, str_ip_group, strlen(str_ip_group));
	return 1;
}


/*
  -1	失败
   0	成功
*/
int port1_query(struct outCtx *ctx, int slot, int fPs)
{
	char str_ip_group[MAX_IP_LEN*MAX_IP_NUM+1];//注意栈溢出问题，必要时用static
	int board_type = BOARD_TYPE_NONE;
	struct outputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct outputsta), 
			 sizeof(struct outputsta), 
			 (char *)&sta, 
			 sizeof(struct outputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	if(BID_PTP == ctx->bid[slot -1])
	{

		memset(str_ip_group,0x0, sizeof(str_ip_group));
		if(!uint32_ip_group_to_str_ip(sta.ptp[0].ptpSlaveIp, 
			sta.ptp[0].ptpSlaveIp_num, str_ip_group))
			return -1;
		if(0 == fPs)
		{
			printf(	"%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d\n",
					sta.outcomminfo[0].delay,
					gSignalTbl[sta.outcomminfo[0].signalType],
					gPdtTbl[sta.ptp[0].ptpDelayType],				
					gPmutTbl[sta.ptp[0].ptpMulticast], 
					gPltTbl[sta.ptp[0].ptpLayer],
					gPstTbl[sta.ptp[0].ptpStep], 
					gFreqTbl[sta.ptp[0].ptpSync],
					gFreqTbl[sta.ptp[0].ptpAnnounce],					
					gFreqTbl[sta.ptp[0].ptpPdelayReq],
					gtraceTbl[sta.ptp[0].ptpSsmEnable],
					sta.ptp[0].ptpIp,
					sta.ptp[0].ptpMac,
					str_ip_group,
					sta.ptp[0].ptpLevel,
					sta.ptp[0].ptpPriority,
					sta.ptp[0].ptpRegion,
					sta.ptp[0].ptpMask,//add type
					sta.ptp[0].ptpGateway,
					sta.ptp[0].ptpType);
			
		}
		else
		{
			printf(	"%d|%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d\n", 
					slot, 
					sta.outcomminfo[0].delay,
					gSignalTbl[sta.outcomminfo[0].signalType],
					gPdtTbl[sta.ptp[0].ptpDelayType],
					gPmutTbl[sta.ptp[0].ptpMulticast], 
					gPltTbl[sta.ptp[0].ptpLayer],
					gPstTbl[sta.ptp[0].ptpStep], 
					gFreqTbl[sta.ptp[0].ptpSync],
					gFreqTbl[sta.ptp[0].ptpAnnounce],
					gFreqTbl[sta.ptp[0].ptpPdelayReq],
					gtraceTbl[sta.ptp[0].ptpSsmEnable],					
					sta.ptp[0].ptpIp,
					sta.ptp[0].ptpMac,					
					str_ip_group,
					sta.ptp[0].ptpLevel,
					sta.ptp[0].ptpPriority,					
					sta.ptp[0].ptpRegion,
					sta.ptp[0].ptpMask,//add type
					sta.ptp[0].ptpGateway,
					sta.ptp[0].ptpType);
			
		}
	}
	else if(BID_PTPF == ctx->bid[slot -1])
	{
		if(0 == fPs)
		{
			printf(	"%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", 
					sta.outcomminfo[0].delay,
					gSignalTbl[sta.outcomminfo[0].signalType],//no support set. only show
					gPdtTbl[sta.ptpf[0].ptpDelayType],
					gPmodeTbl[sta.ptpf[0].ptpModelType],
					gPmutTbl[sta.ptpf[0].ptpMulticast], 
					gPltTbl[sta.ptpf[0].ptpLayer],
					gPstTbl[sta.ptpf[0].ptpStep], 
					gFreqTbl[sta.ptpf[0].ptpSync],
					gFreqTbl[sta.ptpf[0].ptpAnnounce],
					gFreqTbl[sta.ptpf[0].ptpDelayReq],
					gFreqTbl[sta.ptpf[0].ptpPdelayReq],
					sta.ptpf[0].ptpIp,
					sta.ptpf[0].ptpMac,
					sta.ptpf[0].ptpMask,
					sta.ptpf[0].ptpGateway,
					sta.ptpf[0].ptpDns1,
					sta.ptpf[0].ptpDns2,
					sta.ptpf[0].ptpSlaveIp);
		}
		else
		{
			printf(	"%d|%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n", 
					slot, 
					sta.outcomminfo[0].delay, 
					gSignalTbl[sta.outcomminfo[0].signalType],
					gPdtTbl[sta.ptpf[0].ptpDelayType],
					gPmodeTbl[sta.ptpf[0].ptpModelType],
					gPmutTbl[sta.ptpf[0].ptpMulticast], 
					gPltTbl[sta.ptpf[0].ptpLayer],
					gPstTbl[sta.ptpf[0].ptpStep], 
					gFreqTbl[sta.ptpf[0].ptpSync],
					gFreqTbl[sta.ptpf[0].ptpAnnounce],
					gFreqTbl[sta.ptpf[0].ptpDelayReq],
					gFreqTbl[sta.ptpf[0].ptpPdelayReq],
					sta.ptpf[0].ptpIp,
					sta.ptpf[0].ptpMac,
					sta.ptpf[0].ptpMask,
					sta.ptpf[0].ptpGateway,
					sta.ptpf[0].ptpDns1,
					sta.ptpf[0].ptpDns2,
					sta.ptpf[0].ptpSlaveIp);
		}
	}
	else
	{
		switch (ctx->bid[slot -1]){
			//these output board no support the arg
			//case BID_NTP:
			//case BID_NTPF:
			case BID_61850:
				return -1;//failure
				break;
		}
		
		FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
		//printf("bid:%d\n", sta.boardId);
		if(board_type == BOARD_TYPE_OUTPUT){
			if(0 == fPs){
				printf("%d|%s\n", sta.outcomminfo[0].delay, gSignalTbl[sta.outcomminfo[0].signalType]);
			}else{
				printf("%d|%d|%s\n", slot, sta.outcomminfo[0].delay, gSignalTbl[sta.outcomminfo[0].signalType]);
			}
		}else{
			return -1;
		}
	}
	
	return 0;
}







/*
  -1	失败
   0	成功
*/
int port16_query(struct outCtx *ctx)
{
	int i;

	for(i=0; i<SLOT_CURSOR_ARRAY_SIZE; i++)
	{
		port1_query(ctx, i+1, 1);
	}

	in_zone_query(ctx, 0);
	out_zone_query(ctx, 0);
	out_baudrate_query(ctx, 0);
	irigb_amplitude_ratio_query(ctx,0);
	irigb_voltage_query(ctx,0);

	return 0;
}





/*
  -1	失败
   0	成功
*/
int type3_delay_query(struct outCtx *ctx)
{
	int i;
	struct outdelay od[OUTDELAY_SIGNALTYPE_LEN];
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ODELAY, 
			 OUTDELAY_SIGNALTYPE_LEN * sizeof(struct outdelay), 
			 (char *)od, 
			 OUTDELAY_SIGNALTYPE_LEN * sizeof(struct outdelay));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	#if 0
	for(i=0; i<OUTDELAY_SIGNALTYPE_LEN; i++)
	{
		if(OUTDELAY_SIGNAL_TOD == od[i].signal)
		{
			printf("%s %d\n", "tod", od[i].delay);
		}
		else if(OUTDELAY_SIGNAL_IRIGB == od[i].signal)
		{
			printf("%s %d\n", "irigb", od[i].delay);
		}
		else if(OUTDELAY_SIGNAL_PPX == od[i].signal)
		{
			printf("%s %d\n", "ppx", od[i].delay);
		}
		else if(OUTDELAY_SIGNAL_PTP == od[i].signal)
		{
			printf("%s %d\n", "ptp", od[i].delay);
		}
		else
		{
			return -1;
		}
	}
	#else

	char * sign_name = NULL;
	for(i=0; i<OUTDELAY_SIGNALTYPE_LEN; i++) {
		sign_name = NULL;
		FIND_SIGNME_BY_SIGN_OUTDELAY(od[i].signal, sign_name);
		if(sign_name)
			printf("%s %d\n", sign_name, od[i].delay*10);
		else
			return -1;
	}
	#endif
	
	return 0;
}





/*
  -1	失败
   0	成功
*/
int type1_delay_query(struct outCtx *ctx, char *type)
{
	int cnt = 0;
	struct outdelay od;

	#if 0
	if(0 == memcmp("tod", type, strlen(type)))
	{
		cnt = OUTDELAY_SIGNAL_TOD-1;
	}
	else if(0 == memcmp("irigb", type, strlen(type)))
	{
		cnt = OUTDELAY_SIGNAL_IRIGB-1;
	}
	else if(0 == memcmp("ppx", type, strlen(type)))
	{
		cnt = OUTDELAY_SIGNAL_PPX-1;
	}
	else if(0 == memcmp("ptp", type, strlen(type)))
	{
		cnt = OUTDELAY_SIGNAL_PTP-1;
	}
	else
	{
		return -1;
	}
	#else
	FIND_SIGN_BY_SIGNME_OUTDELAY(type, strlen(type), cnt);
	if(cnt <= 0 || cnt > OUTDELAY_SIGNALTYPE_LEN){
		return -1;
	}
	cnt--;
	#endif

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_ODELAY + cnt *sizeof(struct outdelay), 
			 sizeof(struct outdelay), 
			 (char *)&od, 
			 sizeof(struct outdelay));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	printf("%d\n", od.delay*10);
	
	return 0;
}

/*
  -1	失败
   0	成功
*/
int portinfo_query(struct outCtx *ctx, int slot)
{
	int i;
	int bd_maxport = 0;
	int board_type = BOARD_TYPE_NONE;
	struct outputsta sta;
	
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct outputsta), 
			 sizeof(struct outputsta), 
			 (char *)&sta, 
			 sizeof(struct outputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	switch (sta.boardId){
			//these output board no support set outcommon
		//case BID_NTP:
		//case BID_NTPF:
		case BID_61850:
			return -1;
			break;
	}
	FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
	if(board_type != BOARD_TYPE_OUTPUT)
		return -1;

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}

	printf("%d",bd_maxport);
	for (i = 0; i < bd_maxport; ++i){
		printf(":%s", gSignalTbl[sta.outcomminfo[0].signalType]);
	}
	printf("\n");
	return 0;
}

/*
up/down/unkown, only for ntp
  -1	失败
   0	成功
*/
int signal_query(struct outCtx *ctx, int slot)
{
	int i;
	int bd_maxport = 0;
	int board_type = BOARD_TYPE_NONE;
	unsigned int portStatus = 0;
	int per_ntp_st_portidx = 0;
	u8_t linkstatus[V_ETH_MAX];
	struct outputsta sta;
	char *port_status_tbl[] = {
		"down",
		"up",
		"n/a",
	};
	
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct outputsta), 
			 sizeof(struct outputsta), 
			 (char *)&sta, 
			 sizeof(struct outputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	switch (sta.boardId){
			//these output board no support set outcommon
		//case BID_NTP:
		//case BID_NTPF:
		case BID_61850:
			return -1;
			break;
	}
	FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
	if(board_type != BOARD_TYPE_OUTPUT)
		return -1;

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}

	switch (sta.boardId){
		case BID_NTP:
		case BID_NTPF:
			memset(linkstatus,0x0, sizeof(linkstatus));
			u8_t cur_linkstatus = 0;
			dt_port_linkstatus_all_get(ctx->fpga_fd,&portStatus);
			for(i=1;i<=V_ETH_MAX;i++)
			{
				if (portStatus & (1<<i))
				{
					linkstatus[i-1] = 1;
				} 
				else
				{
					linkstatus[i-1] = 0;
				}
			}

			/*for output or ntp port up/down alarm*/
			//ntp_start slot 12
			per_ntp_st_portidx = (slot - 12)*PORT_NTP;

			//printf("%d",bd_maxport);
			for (i = 0; i < bd_maxport; ++i){
				cur_linkstatus = linkstatus[per_ntp_st_portidx+i];
				if(i == 0)
					printf("%s", port_status_tbl[cur_linkstatus]);
				else
					printf(":%s", port_status_tbl[cur_linkstatus]);
			}
			printf("\n");
			break;
		default:
			//printf("%d",bd_maxport);
			for (i = 0; i < bd_maxport; ++i){
				if(i == 0)
					printf("%s", port_status_tbl[2]);
				else
					printf(":%s", port_status_tbl[2]);
			}
			printf("\n");
			break;
		
		
		
	}
	return 0;
}

int spt_signal_table_query(struct outCtx *ctx, int slot)
{
	int i;
	int supported_signal = BIT(SIGNAL_NONE);
	int supported_signal_nums = 0;
	int board_type = BOARD_TYPE_NONE;
	struct outputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct outputsta), 
			 sizeof(struct outputsta), 
			 (char *)&sta, 
			 sizeof(struct outputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	
		switch (ctx->bid[slot -1]){
			//these output board no support the arg
			//case BID_NTP:
			//case BID_NTPF:
			case BID_61850:
				return -1;//failure
				break;
		}
		
		FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
		//printf("bid:%d\n", sta.boardId);
		if(board_type != BOARD_TYPE_OUTPUT){
			return -1;
		}

		FIND_SUP_SIGNAL_GBATBL_BY_BID(sta.boardId, supported_signal);
		if (SIGNAL_ARRAY_SIZE >= 32){
			return -1;
		}

		for (i = 0; i < SIGNAL_ARRAY_SIZE; ++i){
			if(supported_signal & BIT(i)){
				supported_signal_nums++;
			}
		}
		printf("%d", supported_signal_nums);
		for (i = 0; i < SIGNAL_ARRAY_SIZE; ++i){
			if(supported_signal & BIT(i)){
				printf(":%s", gSignalTbl[i]);
			}
		}
		printf("\n");

	return 0;
}


