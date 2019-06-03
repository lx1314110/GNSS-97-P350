#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_fpga.h"
#include "lib_bit.h"

#include "addr.h"
#include "cin_macro.h"
#include "cin_query.h"




/*
  -1	失败
   0	成功
*/
int schema_query(struct inCtx *ctx)
{
	struct schemainfo mode;
	char *ref_status = NULL;

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_SCHEMA, 
			 sizeof(struct schemainfo), 
			 (char *)&mode, 
			 sizeof(struct schemainfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	switch (mode.refsrc_is_valid){
		case REFSOURCE_INIT:
			ref_status = "INIT";
			break;
		case REFSOURCE_VALID:
			ref_status = "VALID";
			break;
		case REFSOURCE_INVALID:
			ref_status = "INVALID";
			break;
		default:
			ref_status = "N/A";
			break;
	}

	printf("%s | %d | %s\n", (0 == mode.schema) ? SCHEMA_FREE:SCHEMA_FORCE,
		mode.src_inx, ref_status);
	
	return 0;
}

/*
  -1	失败
   0	成功
*/
int schema_list_query(struct inCtx *ctx)
{
	int i;
	for (i = 0; i < TIME_SOURCE_LEN; ++i){
		printf("%d:%s:%s\n", gTimeSourceTbl[i].id, gTimeSourceTbl[i].type_name, gTimeSourceTbl[i].msg);
	}	
	return 0;
}

/*
  -1	失败
   0	成功
*/
int schema_cur_valid_list_query(struct inCtx *ctx)
{
	int i,j;
	int s = 0;
	int bd_maxport = 0;
	int board_type = BOARD_TYPE_NONE;
	struct inputsta sta[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	struct port_attr_t *port_attr = NULL;
	
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 INPUT_SLOT_CURSOR_ARRAY_SIZE*sizeof(struct inputsta), 
			 (char *)sta, 
			 INPUT_SLOT_CURSOR_ARRAY_SIZE*sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	for (s = 0; s < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++s){
		board_type = BOARD_TYPE_NONE;
		FIND_BDTYPE_GBATBL_BY_BID(sta[s].boardId, board_type);
		if(board_type != BOARD_TYPE_INPUT)
			continue;

		bd_maxport = 0;
		FIND_MAXPORT_GBATBL_BY_BID(sta[s].boardId, bd_maxport);
		if (bd_maxport <= 0){
			return -1;
		}

		port_attr = NULL;
		FIND_PORTATTR_GBATBL_BY_BID(sta[s].boardId, port_attr);
		if (!port_attr){
			return -1;
		}

		char * type_name = NULL;
		//printf("%d",bd_maxport);
		for (i = 0; i < bd_maxport; ++i){
			for (j = 0; j < TIME_SOURCE_LEN; ++j){
				if(gTimeSourceTbl[j].slot == s+1 &&
					gTimeSourceTbl[j].type == port_attr[i].insrc_type){
					//src_id:slot:port:signal_type:descrip
					if(gTimeSourceTbl[j].type == TOD_SOURCE_TYPE){
						switch (sta[s].boardId){
							case BID_PTP_IN:
								type_name = SIGNAL_TYPE_PTP_IN;
								break;
							case BID_RTF:
								type_name = SIGNAL_TYPE_RTF_1PPS_TOD;
								break;
						}
					}else{
						type_name = gTimeSourceTbl[j].type_name;
					}
						
					printf("%d:%d:%d:%s:%s\n", 
					gTimeSourceTbl[j].id, gTimeSourceTbl[j].slot, i+1,
					type_name,
					gTimeSourceTbl[j].msg);
					break;
				}
			}
		}
	}
	//print no source type;src_id:slot:port:signal_type:descrip. only query, not support setting.
	printf("%d:%d:%d:%s:%s\n", 
					gTimeSourceTbl[TIME_SOURCE_MAX_NOSOURCE].id,
					gTimeSourceTbl[TIME_SOURCE_MAX_NOSOURCE].slot, 0,
					gTimeSourceTbl[TIME_SOURCE_MAX_NOSOURCE].type_name,
					gTimeSourceTbl[TIME_SOURCE_MAX_NOSOURCE].msg);
	return 0;

}


/*
  -1	失败
   0	成功
*/
int portinfo_query(struct inCtx *ctx, int slot)
{
	int i,j;
	int bd_maxport = 0;
	int board_type = BOARD_TYPE_NONE;
	struct inputsta sta;
	struct port_attr_t *port_attr = NULL;
	
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
	if(board_type != BOARD_TYPE_INPUT)
		return -1;

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
	if (!port_attr){
		return -1;
	}

	char * type_name = NULL;
	printf("%d",bd_maxport);
	for (i = 0; i < bd_maxport; ++i){
		for (j = 0; j < TIME_SOURCE_LEN; ++j){
			if(//gTimeSourceTbl[j].slot == slot && \ , else cannot show NONE SIGNAL
				gTimeSourceTbl[j].type == port_attr[i].insrc_type){

				if(gTimeSourceTbl[j].type == TOD_SOURCE_TYPE){
						switch (sta.boardId){
							case BID_PTP_IN:
								type_name = SIGNAL_TYPE_PTP_IN;
								break;
							case BID_RTF:
								type_name = SIGNAL_TYPE_RTF_1PPS_TOD;
								break;
						}
				}else{
					type_name = gTimeSourceTbl[j].type_name;
				}
				printf(":%s",type_name);
				break;
			}
		}
	}
	printf("\n");
	return 0;
}

/*
  -1	失败
   0	成功
*/
int port_srctype_query(struct inCtx *ctx, int slot)
{
	int i,j;
	int bd_maxport = 0;
	int board_type = BOARD_TYPE_NONE;
	struct inputsta sta;
	struct port_attr_t *port_attr = NULL;
	
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
	if(board_type != BOARD_TYPE_INPUT)
		return -1;

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
	if (!port_attr){
		return -1;
	}

	printf("%d",bd_maxport);
	for (i = 0; i < bd_maxport; ++i){
		for (j = 0; j < TIME_SOURCE_LEN; ++j){
			if(//gTimeSourceTbl[j].slot == slot && \ , else cannot show NONE SIGNAL
				gTimeSourceTbl[j].type == port_attr[i].insrc_type){
					printf(":%s",gSrcFlagTbl[gTimeSourceTbl[j].flag]);
				break;
			}
		}
	}
	printf("\n");
	return 0;
}



/*
  -1	失败
   0	成功
   port 0, query the prio of all port of cur slot
   port 1-5, query the prio of cur port of cur slot
*/
int prio_query(struct inCtx *ctx, int slot, int port)
{
	int bd_maxport = 0;
	u8_t prio[INPUT_SLOT_MAX_PORT+1];
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	memset(prio, 0, sizeof(prio));

	memcpy(prio, sta.incommsta.incomminfo.prio, strlen((const char *)sta.incommsta.incomminfo.prio));

	if(prio[0] == '\0')
		return -1;
	
	if(port == 0){
		printf("%s\n", prio);
	}else{
		FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
		if (bd_maxport <= 0){
			return -1;
		}
		if(bd_maxport < port){
			return -1;
		}
		printf("%c\n", prio[port-1]);
	}
	return 0;
}





/*
  -1	失败
   0	成功
*/
int delay_query(struct inCtx *ctx, int slot, int port)
{
	int bd_maxport = 0;
	int delay;
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	if(bd_maxport < port){
		return -1;
	}
	delay = sta.incommsta.incomminfo.delay[port-1];

	printf("%d\n", delay*10);
	
	return 0;
}

/*
freq ssm
  -1	失败
   0	成功
   当前支持信号的ssm 查询
*/
int ssm_query(struct inCtx *ctx, int slot, int port)
{
	u8_t src_flag = INVALID_SOURCE_FLAG;
	int bd_maxport = 0;
	int i;
	struct port_attr_t *port_attr = NULL;
	u16_t signal = 0xffff;

	char * tmp_ssm_str = NULL;
	char ssm_str[INPUT_SLOT_MAX_PORT*STR_SSM_LEN+1];
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_IN_SIGNAL(slot), &signal)){
		return -1;
	}


	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
	if (!port_attr){
		return -1;
	}

	if(port == 0){

		memset(ssm_str, 0x0, sizeof(ssm_str));
		for (i = 0; i < INPUT_SLOT_MAX_PORT; i++){
		//for (i = 0; i < bd_maxport; i++){
			tmp_ssm_str = NULL;
			FIND_STR_BY_SSM_VAL(sta.incommsta.incomminfo.ssm[i], tmp_ssm_str);		
			if(tmp_ssm_str){

				if(port_attr[i].insrc_type == S2MB_SOURCE_TYPE)
					tmp_ssm_str = (INSATE_2MB_SGN_PORT3(signal) > 0) ? (char *) SSM_NE : tmp_ssm_str;

				#if 0
				src_flag = INVALID_SOURCE_FLAG;
				FIND_FLAG_BY_TYPE(port_attr[i].insrc_type, src_flag)
				if(FREQ_SOURCE_FLAG == src_flag){
				//if(S2MB_SOURCE_TYPE != port_attr[i].insrc_type && 
				//	FREQ_SOURCE_FLAG == src_flag){
					//2mb and time source disable set
					memcpy(&ssm_str[i*STR_SSM_LEN], tmp_ssm_str, STR_SSM_LEN);
				}else{
					memcpy(&ssm_str[i*STR_SSM_LEN], SSM_NE, STR_SSM_LEN);
				}
				#else
				memcpy(&ssm_str[i*STR_SSM_LEN], tmp_ssm_str, STR_SSM_LEN);
				#endif
				 
			}else{
				return -1;
			}
		}
		printf("%s\n", ssm_str);
	}else{

		if (bd_maxport < port){
			return -1;
		}

		tmp_ssm_str = NULL;
		FIND_STR_BY_SSM_VAL(sta.incommsta.incomminfo.ssm[port-1], tmp_ssm_str);		
		if(tmp_ssm_str){

			if(port_attr[port-1].insrc_type == S2MB_SOURCE_TYPE)
				tmp_ssm_str = (INSATE_2MB_SGN_PORT3(signal) > 0) ? (char *) SSM_NE : tmp_ssm_str;

			#if 1
			src_flag = INVALID_SOURCE_FLAG;
			FIND_FLAG_BY_TYPE(port_attr[port-1].insrc_type, src_flag);
			memset(ssm_str, 0x0, sizeof(ssm_str));
			if(FREQ_SOURCE_FLAG == src_flag){
			//if(S2MB_SOURCE_TYPE != port_attr[port-1].insrc_type && 
			//	FREQ_SOURCE_FLAG == src_flag){
				//2mb and time source disable set
				memcpy(ssm_str, tmp_ssm_str, STR_SSM_LEN);
			}else{
				memcpy(ssm_str, SSM_NE, STR_SSM_LEN);
			}
			#else
			memcpy(ssm_str, tmp_ssm_str, STR_SSM_LEN);
			#endif
		}else{
			return -1;
		}
		printf("%s\n", ssm_str);
			
	}

	return 0;
}


/*===============bellow SAT PRIVATE=============*/

/*
  -1	失败
   0	成功
   当前仅仅支持卫星槽位的一个sa，端口为3
*/
int sa_query(struct inCtx *ctx, int slot, int port)
{
	int bd_maxport = 0;
	int i;
	struct port_attr_t *port_attr = NULL;

	u8_t sa[4];
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_MB_SA))
		return -1;

	memset(sa, 0, sizeof(sa));
	memcpy(sa, sta.satcommon.satcomminfo.mb_sa, 3);

	if(sa[0] == '\0')
		return -1;

	if(port == 0){
		printf("2MB SA:%s\n", sa);
	}else{
		FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
		if (bd_maxport <= 0){
			return -1;
		}
		FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
		if (!port_attr){
			return -1;
		}

		for (i = 0; i < bd_maxport; ++i){
			if(S2MB_SOURCE_TYPE == port_attr[i].insrc_type && port == i+1){
				//only support a ssm for a slot
				printf("%s\n", sa);
				return 0;
			}
		}
		return -1;
	}

	
	return 0;
}

#define SQRY_IN_SGN_NONE	"NONE"
/*
  -1	失败
   0	成功
*/
int signal_query(struct inCtx *ctx, int slot)
{
	int i,j;
	int bd_maxport = 0;
	u16_t signal=0xffff;
	//u16_t sa_valid = 1;
	char *signal_name[INPUT_SLOT_MAX_PORT];
	u16_t ntp_time_valid = 0;//INVAILD
	struct port_attr_t *port_attr = NULL;

	struct inputsta sta;
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	shm_read(ctx->ipc.ipc_base,
			 SHM_OFFSET_STA + (slot-1)*sizeof(struct inputsta),
			 sizeof(struct inputsta),
			 (char *)&sta,
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	/*
	  输入信号状态，bit0~bit4，0表示有输入，1表示无输入
	  bit0表示2mb，bit1表示2mh，bit2表示irigb1，bit3表示irigb2，bit4表示参考输入（卫星，ptp，1pps+tod）
	*/
	if(!FpgaRead(ctx->fpga_fd, FPGA_IN_SIGNAL(slot), &signal)){
		return -1;
	}


	FIND_MAXPORT_GBATBL_BY_BID(sta.boardId, bd_maxport);
	if (bd_maxport <= 0){
		return -1;
	}
	FIND_PORTATTR_GBATBL_BY_BID(sta.boardId, port_attr);
	if (!port_attr){
		return -1;
	}

	if(bd_maxport > INPUT_SLOT_MAX_PORT)
		return -1;

	char * type_name = NULL;
	for (i = 0; i < bd_maxport; ++i){
		for (j = 0; j < TIME_SOURCE_LEN; ++j){
			signal_name[i] = gSignalTbl[SIGNAL_NONE];
			if(//gTimeSourceTbl[j].slot == slot && \ , else cannot show NONE SIGNAL
				gTimeSourceTbl[j].type == port_attr[i].insrc_type){
				if(gTimeSourceTbl[j].type == TOD_SOURCE_TYPE){
					switch (sta.boardId){
						case BID_PTP_IN:
							type_name = SIGNAL_TYPE_PTP_IN;
							break;
						case BID_RTF:
							type_name = SIGNAL_TYPE_RTF_1PPS_TOD;
							break;
					}
				}else{
					type_name = gTimeSourceTbl[j].type_name;
				}
				signal_name[i] = type_name;
				break;
			}
		}		
	}
	
	switch (sta.boardId){//no signal
		
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			printf( "%s:%s:%s:%s:%s\n", 
					//(((signal &BIT(4)) > 0 || sa_valid > 0)? gSignalTbl[SIGNAL_NONE] : gSignalTbl[SIGNAL_SATELLITE]), sate/2mh/2mb/irigb1/irigb2
					((INSATE_BD_GPS_SGN_PORT1(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[SAT_PORT_SAT-1]), 
					((INSATE_2MH_SGN_PORT2(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[SAT_PORT_2MH-1]), 
					((INSATE_2MB_SGN_PORT3(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[SAT_PORT_2MB-1]), 
					((INSATE_IRIGB1_SGN_PORT4(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[SAT_PORT_IRIGB1-1]), 
					((INSATE_IRIGB2_SGN_PORT5(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[SAT_PORT_IRIGB2-1]));
			break;
		
		case BID_PTP_IN:
				printf( "%s\n",
				(INPTP_PTP_SGN_PORT1(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[0]); //"signal_name[0]:TOD"
			break;
		case BID_RTF:
				printf( "%s:%s:%s:%s\n",
				(INRTF_1PPS_TOD_SGN_PORT1(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[RTF_PORT_1PPS_TOD-1],
				(INRTF_1PPS_SGN_PORT2(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[RTF_PORT_1PPS-1],
				(INRTF_10MH1_SGN_PORT3(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[RTF_PORT_10MH1-1],
				(INRTF_10MH2_SGN_PORT4(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[RTF_PORT_10MH2-1]);
				
			break;
		case BID_NTP_IN:
				if(!FpgaRead(ctx->fpga_fd, FPGA_IN_NTP_IN_TIME_VAILD(slot), &ntp_time_valid)){
					return 0;
				}
				printf( "%s\n",
				(ntp_time_valid == 0 || INPTP_NTP_SGN_PORT1(signal) > 0)? gSignalTbl[SIGNAL_NONE] : signal_name[0]);
				break;
		default:
			return -1;
	}

	return 0;
}






/*
  -1	失败
   0	成功
*/
int mode_query(struct inCtx *ctx, int slot)
{
	u8_t sysmode;
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_SYSMODE))
		return -1;

	sysmode = sta.satcommon.satcomminfo.sysMode;

	switch(sysmode)
	{
		case MODE_CURSOR_GPS_VAL:
			printf("%s\n", MODE_GPS);
			break;
		case MODE_CURSOR_BD_VAL:
			printf("%s\n", MODE_BD);
			break;
		case MODE_CURSOR_MIX_GPS_VAL:
			printf("%s\n", MODE_MIX_GPS);
			break;
		case MODE_CURSOR_MIX_BD_VAL:
			printf("%s\n", MODE_MIX_BD);
			break;
	}
	
	return 0;
}






/*
  -1	失败
   0	成功
*/
int elev_query(struct inCtx *ctx, int slot)
{
	u8_t elev[8];
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_ELEV))
		return -1;

	memset(elev, 0, sizeof(elev));
	memcpy(elev, sta.satcommon.satcomminfo.elev, strlen((const char *)sta.satcommon.satcomminfo.elev));

	printf("%s\n", elev);
	
	return 0;
}

/*
 转换经纬度输出格式
*/

void convert_lola(u8_t *lola,u8_t *result)
{
	int found = 0;
	int count = 0;
	if(*lola != 0)
	{
		u8_t du[5]={0},fen[9]={0};
		u8_t *ptr;
		double m;
		ptr = lola;
		count++;
		while(*(++lola) != '\0'){
			if(*lola == '.'){
				found = 1;
				break;
			}
			count++;
		}

		if(found != 1 || count < 2)
			return;
		memcpy(fen,lola-2,2);
#if 0
		u8_t sec[6]={0},
		memcpy(sec,lola+1,3);
		m = atoi(sec)*0.06;
#else
		u8_t secf[8];
		memset(secf, 0x0, sizeof(secf));
		memcpy(secf,lola,strlen((char *)lola));
		m = (float) atof((char *)secf)*60;
#endif	
		memcpy(du,ptr,lola-ptr-2);
		
		sprintf((char *)result,"%s'%s'%0.1f\"",du,fen,m);
	}
		
	
}

/*
  -1	失败
   0	成功
*/
int lola_query(struct inCtx *ctx, int slot)
{
	u8_t lon[13];
	u8_t lat[12];
	u8_t result[13];
	
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if (!(sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_LON))
		return -1;

	memset(lon, 0, sizeof(lon));
	memset(lat, 0, sizeof(lat));
	memcpy(lon, sta.satcommon.satcomminfo.lon, strlen((const char *)sta.satcommon.satcomminfo.lon));
	memcpy(lat, sta.satcommon.satcomminfo.lat, strlen((const char *)sta.satcommon.satcomminfo.lat));

	//printf("#####lon:%s  lat:%s\n",lon,lat);
	memset(result,0,sizeof(result));
	convert_lola(&lon[1],result);
	printf("%c | %s\n", lon[0], result);
	memset(result,0,sizeof(result));
	convert_lola(&lat[1],result);
	printf("%c | %s\n", lat[0], result);
	
	return 0;
}

/*satellite
  -1	失败
   0	成功
*/
int port_query(struct inCtx *ctx, int slot, int port)
{
	int i;
	int sate_signal_valid = 0;
	u16_t signal;
	struct inputsta sta;
	u8_t result[13];
	char * tmp_ssm_str = NULL;

	if(!FpgaRead(ctx->fpga_fd, FPGA_IN_SIGNAL(slot), &signal)){
		return -1;
	}
	memset(&sta, 0x0, sizeof(struct inputsta));
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_INSATE_LOWER ... BID_INSATE_UPPER:
			break;
		default:
			//other board
			return -1;
	}
	if(1 == port)
	{
		switch (sta.boardId){//no signal
		
			case BID_INSATE_LOWER ... BID_INSATE_UPPER:
				if(!(signal &BIT(4)))//signal ok
					sate_signal_valid = 1;
				break;	
		}
		if(!sate_signal_valid)
		{
			return -1;
		}
		else
		{
			//GPSBF ... BDBE
			//switch (sta.boardId){//no signal
		
			//case BID_GPSBF ... BID_BDBE:
				// 1
				if (sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_SYSMODE){
					switch(sta.satcommon.satcomminfo.sysMode){
						case MODE_CURSOR_GPS_VAL:
						case MODE_CURSOR_MIX_GPS_VAL:
							printf("%s\n", "GPS");
							break;
						case MODE_CURSOR_BD_VAL:
						case MODE_CURSOR_MIX_BD_VAL:
							printf("%s\n", "BD");
							break;
						default:
							printf("%s\n", "N/A");
							break;
					}
				}
	
				// 2
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_TRACKMODE){
					printf("%s\n", (('A' == sta.satcommon.trackMode)? "HOLD" : "POSI"));
				}
				// 3
				if (sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_SYSMODE){
					switch(sta.satcommon.satcomminfo.sysMode){
						case MODE_CURSOR_GPS_VAL:
							printf("%s\n", MODE_GPS);
							break;
						case MODE_CURSOR_BD_VAL:
							printf("%s\n", MODE_BD);
							break;
						case MODE_CURSOR_MIX_GPS_VAL:
							printf("%s\n", MODE_MIX_GPS);
							break;
						case MODE_CURSOR_MIX_BD_VAL:
							printf("%s\n", MODE_MIX_BD);
							break;
					}
				}
				// 4
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_ELEVTHD){
					printf("%d\n", sta.satcommon.elevThd);
				}
	
				// 5
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_FDRSTATUS){
					if(sta.satcommon.fdrStatus == 0)
						printf("\n");
					else
						printf("%c\n", sta.satcommon.fdrStatus);
				}
	
				// 6
				if (sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_LON){
					memset(result,0,sizeof(result));
					convert_lola(&sta.satcommon.satcomminfo.lon[1],result);
					printf("%c | %s\n", sta.satcommon.satcomminfo.lon[0], result);
					memset(result,0,sizeof(result));
					convert_lola(&sta.satcommon.satcomminfo.lat[1],result);
					printf("%c | %s\n", sta.satcommon.satcomminfo.lat[0], result);
				}
				// 7
				if (sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_ELEV){
					printf("%s\n", sta.satcommon.satcomminfo.elev);
				}
				// 8
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_RCVRVER){
					printf("%s\n", sta.satcommon.rcvrVer);
				}
				// 9
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_RCVRTYPE){
					printf("%s\n", sta.satcommon.rcvrType);
				}
				// 10
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_UTC){
					printf("%s\n", sta.satcommon.utc);
				}
				// 11
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_NGPS){
					if(0 == sta.satcommon.nGps){
						printf("\n");
					}else{
						printf("%s | %d", "GPS", sta.satcommon.nGps);
						
						for(i=0; i<sta.satcommon.nGps; i++)
						{
							if (sta.satcommon.vGps[i].snr == 255){
								printf(" | %d,*", sta.satcommon.vGps[i].id);
							}else{
								printf(" | %d,%d", sta.satcommon.vGps[i].id, sta.satcommon.vGps[i].snr);
							}
						}
						printf("\n");
					}
				}
				// 12
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_NBD){
					if(0 == sta.satcommon.nBd){
						printf("\n");
					}else{
						printf("%s | %d", "BD", sta.satcommon.nBd);
						
						for(i=0; i<sta.satcommon.nBd; i++)
						{
							if (sta.satcommon.vBd[i].snr == 255){
								printf(" | %d,*", sta.satcommon.vBd[i].id);
							}else{
								printf(" | %d,%d", sta.satcommon.vBd[i].id, sta.satcommon.vBd[i].snr);
							}
						}
						printf("\n");
					}
				}
	
				// 13
				if (sta.incommsta.incomminfo.prio[port-1] == 0)
					printf("\n");
				else
					printf("%c\n", sta.incommsta.incomminfo.prio[port-1]);
				printf("%d\n", sta.incommsta.incomminfo.delay[port-1]);
	
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_PH){
					printf("%d\n", sta.satcommon.ph[port-1]);
				}
				// 14
				if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_GBS){
					printf("%d\n", sta.satcommon.gbs);
				}
			//	break;
			//default:
			//	return -1;
			//}
		}
	}
	else if(2 == port)
	{
		if(signal &BIT(1))
		{
			return -1;
		}
		else
		{
			switch (sta.boardId){
				//2mh, no bei bfi
				case BID_BEI:
				case BID_BFI:
					return -1;
					break;
			}
			tmp_ssm_str = NULL;
			FIND_STR_BY_SSM_VAL(sta.incommsta.incomminfo.ssm[port-1], tmp_ssm_str);		
			if(tmp_ssm_str)
				printf("%s\n", tmp_ssm_str);
			else
				printf("\n");
			
			if (sta.incommsta.incomminfo.prio[port-1] == 0)
				printf("\n");
			else
				printf("%c\n", sta.incommsta.incomminfo.prio[port-1]);
			printf("%d\n", sta.incommsta.incomminfo.delay[port-1]);
			if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_PH){
				printf("%d\n", sta.satcommon.ph[port-1]);
			}
		}
	}
	else if(3 == port)
	{
		if(signal &BIT(0))
		{
			return -1;
		}
		else
		{
			switch (sta.boardId){
				//2mb no bei bfi
				case BID_BEI:
				case BID_BFI:
					return -1;
					break;
			}
			if (sta.satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_MB_SA){
				printf("%s\n", sta.satcommon.satcomminfo.mb_sa);
			}
			tmp_ssm_str = NULL;
			FIND_STR_BY_SSM_VAL(sta.incommsta.incomminfo.ssm[port-1], tmp_ssm_str);		
			if(tmp_ssm_str)
				printf("%s\n", tmp_ssm_str);
			else
				printf("\n");
			if (sta.incommsta.incomminfo.prio[port-1] == 0)
				printf("\n");
			else
				printf("%c\n", sta.incommsta.incomminfo.prio[port-1]);
			printf("%d\n", sta.incommsta.incomminfo.delay[port-1]);
			if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_PH){
				printf("%d\n", sta.satcommon.ph[port-1]);
			}

		}
	}
	else
	{
		if(4 == port)
		{
			if(signal &BIT(2))
			{
				return -1;
			}
		}
		else//5
		{
			if(signal &BIT(3))
			{
				return -1;
			}
		}
		//irigb1/irigb2, all 10 sat
		if (sta.incommsta.incomminfo.prio[port-1] == 0)
			printf("\n");
		else
			printf("%c\n", sta.incommsta.incomminfo.prio[port-1]);
		printf("%d\n", sta.incommsta.incomminfo.delay[port-1]);
		if (sta.satcommon.sta_flag & FLAG_SAT_COMMSTA_PH){
			printf("%d\n", sta.satcommon.ph[port-1]);
		}
	}

	return 0;
}


/*
  -1	失败
   0	成功
   for the lcd show.
*/
int gps_bd_query(struct inCtx *ctx)
{
	int i;
	int sate_type = 0;
	int no_sate_count = 0;
	int sate_signal_valid[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	u16_t signal[INPUT_SLOT_CURSOR_ARRAY_SIZE];
	struct inputsta sta[INPUT_SLOT_CURSOR_ARRAY_SIZE];

	memset(signal, 0x0, sizeof(signal));
	memset(sta, 0x0, sizeof(sta));
	memset(sate_signal_valid, 0x0, sizeof(sate_signal_valid));

	for (i = 0; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		if(!FpgaRead(ctx->fpga_fd, FPGA_IN_SIGNAL(i+1), &signal[i])){
			return -1;
		}

	}
	

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA, 
			 sizeof(struct inputsta) * INPUT_SLOT_CURSOR_ARRAY_SIZE, 
			 (char *)sta, 
			 sizeof(struct inputsta) * INPUT_SLOT_CURSOR_ARRAY_SIZE);

	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	for (i = 0; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){

		sate_type = 0;
		switch (sta[i].boardId){//no signal
			
			case BID_INSATE_LOWER ... BID_INSATE_UPPER:
				sate_type = 1;
				if(!(signal[i] &BIT(4)))//signal ok
					sate_signal_valid[i] = 1;
				break;
		}


		if (!sate_type){
			//nothing, not sateboard
			no_sate_count++;
		}
		else if(!sate_signal_valid[i])
		{
			printf("S%d GPS-00 ", i+1);
			printf("BD-00\n");
		}
		else
		{
			//6 sat
			if (sta[i].satcommon.sta_flag & FLAG_SAT_COMMSTA_NGPS){
				printf("S%d %s-%02d ", i+1, "GPS", sta[i].satcommon.nGps);
			}
			if (sta[i].satcommon.sta_flag & FLAG_SAT_COMMSTA_NBD){	
				printf("%s-%02d\n", "BD", sta[i].satcommon.nBd);
			}

		}
	}

	if(no_sate_count == INPUT_SLOT_CURSOR_ARRAY_SIZE)
		printf("No GPS/BD Board\n");//not over 15 byte
	return 0;
}

/********************blew ptp in******************************/

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
int input_ptpin_board_query(struct inCtx *ctx, int slot)
{
	char str_ip_group[MAX_IP_LEN*MAX_IP_NUM+1];//注意栈溢出问题，必要时用static
	struct inputsta sta;
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(slot-1)*sizeof(struct inputsta), 
			 sizeof(struct inputsta), 
			 (char *)&sta, 
			 sizeof(struct inputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}

	//only support sat
	switch (sta.boardId){
		case BID_PTP_IN:
			break;
		default:
			//other board
			return -1;
	}

	memset(str_ip_group,0x0, sizeof(str_ip_group));
	if(!uint32_ip_group_to_str_ip(sta.ptpin.ptpinInfo.ptpMasterIp, 
	sta.ptpin.ptpinInfo.ptpMasterIp_num, str_ip_group))
		return -1;

	printf(	"%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n",
			gPdtTbl[sta.ptpin.ptpinInfo.ptpDelayType],				
			gPmutTbl[sta.ptpin.ptpinInfo.ptpMulticast], 
			gPltTbl[sta.ptpin.ptpinInfo.ptpLayer],
			gPstTbl[sta.ptpin.ptpinInfo.ptpStep], 
			gFreqTbl[sta.ptpin.ptpinInfo.ptpSync],
			gFreqTbl[sta.ptpin.ptpinInfo.ptpAnnounce],
			gFreqTbl[sta.ptpin.ptpinInfo.ptpdelayReq],
			gFreqTbl[sta.ptpin.ptpinInfo.ptpPdelayReq],
			sta.ptpin.ptpinInfo.ptpIp,
			sta.ptpin.ptpinInfo.ptpMask,
			sta.ptpin.ptpinInfo.ptpGateway,
			str_ip_group,
			sta.ptpin.ptpinInfo.ptpMasterMac,
			sta.ptpin.ptpinInfo.ptpLevel,
			sta.ptpin.ptpinInfo.ptpPriority,
			sta.ptpin.ptpinInfo.ptpRegion);

	return 0;
}

