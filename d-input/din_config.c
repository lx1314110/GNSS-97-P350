#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_dbg.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_sqlite.h"
#include "lib_time.h"

#include "din_source.h"
#include "din_config.h"
#include "common.h"

typedef int ReadConfigFun_t(struct inCtx *, char *, int, int);
typedef int WriteConfigFun_t(struct inCtx *, struct inputinfo *, char *);

struct rwConfig_t {
	u8_t  boardId;
	ReadConfigFun_t *ReadConfigFun;
	WriteConfigFun_t *WriteConfigFun;
};

static int SATCOMMReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );

static int SATCOMMWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );

static int PTPINReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );

static int PTPINWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );

#if 0
static int PTPINCOMMReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );

static int PTPINCOMMWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );

static int RTFCOMMReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );

static int RTFCOMMWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );
#endif

static struct rwConfig_t gRWConfig[] = {
	//SAT
	{BID_GPSBF,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_GPSBE,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_GBDBF,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_GBDBE,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_BDBF,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_BDBE,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_BEI,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_BFI,SATCOMMReadConfig,SATCOMMWriteConfig,},
	{BID_BFEI,SATCOMMReadConfig,SATCOMMWriteConfig},
	{BID_BFFI,SATCOMMReadConfig,SATCOMMWriteConfig},
	//PTP_IN, WILL INCREASE
	{BID_PTP_IN,PTPINReadConfig,PTPINWriteConfig},
	//RTF
	//{BID_RTF,RTFCOMMReadConfig,RTFCOMMWriteConfig},
};
#define LEN_GRWCONFIG (sizeof(gRWConfig)/sizeof(struct rwConfig_t))

/*
  @pInCtx		input board context
  @pTbl			runtime config table	
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table
*/
static int SATCOMMReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort )
{
	char sql[512];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select i_sysmode, i_elevation, i_latitude, i_longitude, i_sa \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, pInCtx->inSta[iSlot-1].boardId, iPort);
	
	db_lock(pInCtx->pDb);
	ret1 = db_get_table(pInCtx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(pInCtx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read config of %s.",
				gBoardNameTbl[pInCtx->inSta[iSlot-1].boardId]);

		return 0;
	}

	pInCtx->inSta[iSlot-1].satcommon.satcomminfo.sysMode = atoi(resultp[ncolumn*nrow+0]);
	memset(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.elev, 0x0, 8);
	memcpy(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.elev, resultp[ncolumn*nrow+1], strlen(resultp[ncolumn*nrow+1]));
	memset(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.lat, 0x0, 12);
	memcpy(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.lat, resultp[ncolumn*nrow+2], strlen(resultp[ncolumn*nrow+2]));
	memset(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.lon, 0x0, 13);
	memcpy(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.lon, resultp[ncolumn*nrow+3], strlen(resultp[ncolumn*nrow+3]));
	//memset(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.mh_ssm, 0x0, 3);
	//memcpy(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.mh_ssm, resultp[ncolumn*nrow+4], strlen(resultp[ncolumn*nrow+4]));
	memset(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.mb_sa, 0x0, 4);
	memcpy(pInCtx->inSta[iSlot-1].satcommon.satcomminfo.mb_sa, resultp[ncolumn*nrow+4], strlen(resultp[ncolumn*nrow+4]));

	db_free_table(resultp);
	db_unlock(pInCtx->pDb);
	
	return 1;
}



/*
  @pInCtx		input board context
  @pCfg			config
  @pTbl			runtime config table

  1	成功
  0	失败

  write config to runtime table
*/
static int SATCOMMWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl )
{
	char sql[512];
	bool_t ret1;

	if (pCfg->set_which != INPUTINFO_SET_PRIV_INFO){
		return 1; //ok
	}
	//copy sat config data
	memcpy(&(pInCtx->inSta[pCfg->slot -1].satcommon.satcomminfo), &(pCfg->satcomminfo), sizeof(struct satcommoninfo));
	//copy sat data to incomm data
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set i_sysmode=\'%d\', i_elevation=\'%s\', i_latitude=\'%s\', \
				  i_longitude=\'%s\', i_sa=\'%s\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->satcomminfo.sysMode, 
				  pCfg->satcomminfo.elev,
				  pCfg->satcomminfo.lat,
				  pCfg->satcomminfo.lon,
				  //pCfg->satcomminfo.mh_ssm,
				  pCfg->satcomminfo.mb_sa,
				  pCfg->slot, pCfg->boardId, pCfg->nPort);

	db_lock(pInCtx->pDb);
	ret1 = db_put_table(pInCtx->pDb, sql);
	db_unlock(pInCtx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write config of %s.", 
				gBoardNameTbl[pCfg->boardId] );

		return 0;
	}
	else
	{
		return 1;
	}
}

/*
  @ctx			output board context
  @pTbl			runtime config table	
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table
*/
static int PTPINReadConfig(struct inCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[512];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select \
				  o_delaytype, o_multicast, o_layer, o_step, o_sync, o_announce, \
				  o_delay_req, o_pdelay_req, \
				  o_ip, o_mask, o_gateway, o_peer_ip, o_peer_mac, o_priority1, o_priority2, \
				  o_Region from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_PTP_IN, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read config of %s.",
				gBoardNameTbl[BID_PTP_IN] );
		
		return 0;
	}

	memset(&(ctx->inSta[iSlot-1].ptpin.ptpinInfo), 0, sizeof(struct ptpin_info));
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpDelayType = atoi(resultp[ncolumn*nrow+0]);
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpMulticast = atoi(resultp[ncolumn*nrow+1]);
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpLayer = atoi(resultp[ncolumn*nrow+2]);
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpStep = atoi(resultp[ncolumn*nrow+3]);
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpSync = atoi(resultp[ncolumn*nrow+4]);
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpAnnounce = atoi(resultp[ncolumn*nrow+5]);

	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpdelayReq = atoi(resultp[ncolumn*nrow+6]);
	ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpPdelayReq = atoi(resultp[ncolumn*nrow+7]);

	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpIp, 
		   resultp[ncolumn*nrow+8], 
		   strlen(resultp[ncolumn*nrow+8]));
		   
	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpMask, 
		   resultp[ncolumn*nrow+9], 
		   strlen(resultp[ncolumn*nrow+9]));
	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpGateway, 
		   resultp[ncolumn*nrow+10], 
		   strlen(resultp[ncolumn*nrow+10]));

	str_ip_group_to_uint32(resultp[ncolumn*nrow+11],
		ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpMasterIp,
		&ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpMasterIp_num);
	//memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpMasterIp, 
		//   resultp[ncolumn*nrow+11], 
		//   strlen(resultp[ncolumn*nrow+11]));

	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpMasterMac, 
		   resultp[ncolumn*nrow+12], 
		   strlen(resultp[ncolumn*nrow+12]));
	
	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpLevel, 
		   resultp[ncolumn*nrow+13], 
		   strlen(resultp[ncolumn*nrow+13]));
	
	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpPriority, 
		   resultp[ncolumn*nrow+14], 
		   strlen(resultp[ncolumn*nrow+14]));

	memcpy(ctx->inSta[iSlot-1].ptpin.ptpinInfo.ptpRegion, 
		   resultp[ncolumn*nrow+15], 
		   strlen(resultp[ncolumn*nrow+15]));
	
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	return 1;
}







/*
  @ctx			output board context
  @pCfg			config
  @pTbl			runtime config table

  1	成功
  0	失败

  write config to runtime table
*/
static int PTPINWriteConfig(struct inCtx *pInCtx, struct inputinfo *pCfg, char *pTbl)
{
	char sql[512];
	char des_ipv4_grp[MAX_IP_NUM*MAX_IP_LEN-1];
	bool_t ret1;

	if (pCfg->set_which != INPUTINFO_SET_PRIV_INFO){
			return 1; //ok
	}

	memcpy(&(pInCtx->inSta[pCfg->slot -1].ptpin.ptpinInfo), &(pCfg->ptpinInfo), sizeof(struct ptpin_info));

	memset(des_ipv4_grp, 0x0, sizeof(des_ipv4_grp));
	if(!uint32_ip_group_to_str_ip(pCfg->ptpinInfo.ptpMasterIp,
		pCfg->ptpinInfo.ptpMasterIp_num, des_ipv4_grp))
		return 0;//failure
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delaytype=\'%d\', o_multicast=\'%d\', o_layer=\'%d\', o_step=\'%d\', \
				  o_sync=\'%d\', o_announce=\'%d\', o_delay_req=\'%d\', o_pdelay_req=\'%d\', \
				  o_ip=\'%s\', o_mask=\'%s\', o_gateway=\'%s\', o_peer_ip=\'%s\', o_peer_mac=\'%s\', \
				  o_priority1=\'%s\', o_priority2=\'%s\', o_Region=\'%s\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
	
				  pCfg->ptpinInfo.ptpDelayType,
				  pCfg->ptpinInfo.ptpMulticast,
				  pCfg->ptpinInfo.ptpLayer,
				  pCfg->ptpinInfo.ptpStep,
				  pCfg->ptpinInfo.ptpSync,
				  pCfg->ptpinInfo.ptpAnnounce,
				  pCfg->ptpinInfo.ptpdelayReq,
				  pCfg->ptpinInfo.ptpPdelayReq,
				  pCfg->ptpinInfo.ptpIp,
				  pCfg->ptpinInfo.ptpMask,
				  pCfg->ptpinInfo.ptpGateway,
				  des_ipv4_grp,
				  pCfg->ptpinInfo.ptpMasterMac,
				  pCfg->ptpinInfo.ptpLevel,
				  pCfg->ptpinInfo.ptpPriority,
				  pCfg->ptpinInfo.ptpRegion,
				  pCfg->slot, BID_PTP_IN, pCfg->nPort);
	
	db_lock(pInCtx->pDb);
	ret1 = db_put_table(pInCtx->pDb, sql);
	db_unlock(pInCtx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write config of %s.", 
				gBoardNameTbl[BID_PTP_IN] );
		
		return 0;
	}
	else
	{
		return 1;
	}
}


static int INCOMMReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort )
{
	int i;
	int ssm_val;
	char ssm_str[INPUT_SLOT_MAX_PORT*STR_SSM_LEN+1];
	char sql[512];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select i_prio, i_ssm, \
				  i_port1_delay, i_port2_delay, i_port3_delay, i_port4_delay, i_port5_delay \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, pInCtx->inSta[iSlot-1].boardId, iPort);
	//print(DBG_DEBUG, sql);
	db_lock(pInCtx->pDb);
	ret1 = db_get_table(pInCtx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(pInCtx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read config of %s.",
				gBoardNameTbl[pInCtx->inSta[iSlot-1].boardId] );

		return 0;
	}
	if(nrow == 0 && ncolumn == 0){
		print(	DBG_ERROR, 
				"--no record to read incommconfig tbl:%s of %s slot:%d.",
				pTbl,
				gBoardNameTbl[pInCtx->inSta[iSlot-1].boardId], iSlot );
		return 0;
	}
	memset(pInCtx->inSta[iSlot-1].incommsta.incomminfo.prio, 0x0, INPUT_SLOT_MAX_PORT+1);
	memcpy(pInCtx->inSta[iSlot-1].incommsta.incomminfo.prio, resultp[ncolumn*nrow+0], strlen(resultp[ncolumn*nrow+0]));

	memset(ssm_str, 0x0, sizeof(ssm_str));
	memcpy(ssm_str, resultp[ncolumn*nrow+1], strlen(resultp[ncolumn*nrow+1]));
#if (INPUT_SLOT_MAX_PORT == 5)
	for (i = 0; i < INPUT_SLOT_MAX_PORT; ++i){
		pInCtx->inSta[iSlot-1].incommsta.incomminfo.delay[i] = atoi(resultp[ncolumn*nrow+i+2]);
	}
#else
#error "defined input MAX port or macro INPUT_SLOT_MAX_PORT error! Please update i_portn_delay(current max 5) of the datebase"
#endif
	db_free_table(resultp);
	db_unlock(pInCtx->pDb);

	for (i = 0; i < INPUT_SLOT_MAX_PORT; i++){
		FIND_VAL_BY_SSM_STR(&ssm_str[i*STR_SSM_LEN], ssm_val);
		if(ssm_val != -1)
			pInCtx->inSta[iSlot-1].incommsta.incomminfo.ssm[i] = (u8_t) ssm_val;
	}
		
	return 1;
}

/*
  @pInCtx		input board context
  @pCfg			config
  @pTbl			runtime config table

  1	成功
  0	失败

  write config to runtime table
*/
static int INCOMMWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl )
{
	int i;

	char * tmp_ssm_str = NULL;
	char ssm_str[INPUT_SLOT_MAX_PORT*STR_SSM_LEN+1];
	char sql[512];
	bool_t ret1;

	if (pCfg->set_which != INPUTINFO_SET_INCOMM_INFO){
		return 1; //ok
	}

	//copy common config data
	memcpy(&(pInCtx->inSta[pCfg->slot -1].incommsta.incomminfo), &(pCfg->incomminfo), sizeof(struct incommoninfo));

	memset(ssm_str, 0x0, sizeof(ssm_str));
	for (i = 0; i < INPUT_SLOT_MAX_PORT; i++){
		tmp_ssm_str = NULL;
		FIND_STR_BY_SSM_VAL(pInCtx->inSta[pCfg->slot -1].incommsta.incomminfo.ssm[i], tmp_ssm_str);		
		if(tmp_ssm_str)
			 memcpy(&ssm_str[i*STR_SSM_LEN], tmp_ssm_str, STR_SSM_LEN);
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set i_prio=\'%s\', i_ssm=\'%s\', \
				  i_port1_delay=%d, i_port2_delay=%d, i_port3_delay=%d, i_port4_delay=%d, i_port5_delay=%d \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->incomminfo.prio,
				  ssm_str,
				  pCfg->incomminfo.delay[0],
				  pCfg->incomminfo.delay[1],
				  pCfg->incomminfo.delay[2],
				  pCfg->incomminfo.delay[3],
				  pCfg->incomminfo.delay[4],
				  pCfg->slot, pCfg->boardId, pCfg->nPort);

#if (INPUT_SLOT_MAX_PORT == 5)
#else
#error "defined input MAX port or macro INPUT_SLOT_MAX_PORT error! Please update i_portn_delay(current max 5) of the datebase"
#endif
	db_lock(pInCtx->pDb);
	ret1 = db_put_table(pInCtx->pDb, sql);
	db_unlock(pInCtx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write config of %s.", 
				gBoardNameTbl[pCfg->boardId] );

		return 0;
	}
	else
	{
		return 1;
	}
}

/*
  @pInCtx		input board context
  @pTbl			runtime config table
  @iBid			BoardId
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table,only TBL_IO_RUNTIME
*/
int ReadConfigFromRunTimeTable(
	struct inCtx *ctx,
	char *pTbl,
	int iSlot,
	int iBid,
	int iPort )

{
	int i;
	//public configure
	//for the incommsta
	print(DBG_DEBUG, "ReadConfigFromRunTimeTable: tbl:%s slot:%d bid:%d port:%d",
		pTbl, iSlot, iBid, iPort);
 	if(0 == INCOMMReadConfig(ctx, pTbl, iSlot, iPort))
		return 0;
	 for (i = 0; i < LEN_GRWCONFIG; ++i){
		//private configure
		if(gRWConfig[i].boardId == iBid){
			if(0 == gRWConfig[i].ReadConfigFun(ctx, pTbl, iSlot, iPort)){
				return 0;
			}
			break;
		}
	}
	return 1;

}

/*
  @pInCtx		input board context
  @pTbl			runtime config table
  @iBid			BoardId
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  Write config to runtime table[TBL_IO_RUNTIME] and ctx->insta
*/
int WriteConfigToRunTimeTable(
struct inCtx *ctx,
struct inputinfo *pCfg,
char *pTbl)
{
	int i;
	if(0 == INCOMMWriteConfig(ctx, pCfg, pTbl))
		return 0;
	 for (i = 0; i < LEN_GRWCONFIG; ++i){
		if(gRWConfig[i].boardId == pCfg->boardId){
			if(0 == gRWConfig[i].WriteConfigFun(ctx, pCfg, pTbl)){
				return 0;
			}
			break;
		}
	}
	return 1;

}

/*
  @pInCtx		input board context
  @pTbl			runtime config table
  
  1	成功
  0	失败
  
  read board identifier from runtime table
*/
int ReadBidFromTable(struct inCtx *pInCtx, char *pTbl)
{
	char sql[128];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int i;
	int slot = 0;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select io_slot, io_bid \
				  from %s \
				  where io_port=1 order by io_slot;", pTbl);//limit 2
	
	db_lock(pInCtx->pDb);
	ret1 = db_get_table(pInCtx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(pInCtx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read board identifier from table.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT] );

		return 0;
	}
	
	for(i=1; i<(nrow+1); i++)
	{
		slot = atoi(resultp[ncolumn*i]);
		if (slot > INPUT_SLOT_CURSOR_ARRAY_SIZE || slot <= 0){
			print(	DBG_DEBUG, 
				"<%s>--read slot[%d] over", 
				gDaemonTbl[DAEMON_CURSOR_INPUT],slot);
			continue;
		}
		pInCtx->oldBid[slot-1] = atoi(resultp[ncolumn*i+1]);
		//pInCtx->oldBid[i-1] = atoi(resultp[ncolumn*i+1]);
		print(	DBG_INFORMATIONAL, 
				"--slot:%d oldbid:%d name:%s.",
				slot,
				pInCtx->oldBid[slot-1],
				gBoardNameTbl[pInCtx->oldBid[slot-1]] );
	}
	db_free_table(resultp);
	db_unlock(pInCtx->pDb);
	
	return 1;
}

/*
  @pInCtx		input board context
  @pTbl			runtime config table
  @iSlot		slot of equipment

  1	成功
  0	失败

  delete config of runtime table
*/
int DeleteConfig(struct inCtx *pInCtx, char *pTbl, int iSlot )
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "delete from %s where io_slot = %d;", pTbl, iSlot);
	db_lock(pInCtx->pDb);
	ret1 = db_put_table(pInCtx->pDb, sql);
	db_unlock(pInCtx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to delete config of slot %d.", 
				iSlot );

		return 0;
	}
	else
	{
		return 1;
	}
}

/*
  @pInCtx		input board context
  @pTbl			runtime config table
  @iSlot		slot of equipment

  1	成功
  0	失败

  empty config of runtime table
*/
int EmptyConfig(struct inCtx *pInCtx, char *pTbl, int iSlot )
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "delete from %s where io_slot = %d;", pTbl, iSlot);
	sprintf(sql, "%s insert into %s(io_slot, io_bid, io_port) values(%d, %d, %d);", 
				 sql, pTbl, iSlot, BID_NONE, 1);
	db_lock(pInCtx->pDb);
	ret1 = db_put_table(pInCtx->pDb, sql);
	db_unlock(pInCtx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to empty config of slot %d.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT], 
				iSlot );

		return 0;
	}
	else
	{
		return 1;
	}
}

/*
  @pInCtx		input board context
  @pFromTbl		default config table
  @pToTbl		runtime config table
  @iSlot		slot of equipment
  @iBid			board identifier
  @iPort		port of board

  1	成功
  0	失败

  copy config from default table to runtime table
*/
int CopyConfig(
	struct inCtx *pInCtx, 
	char *pFromTbl, 
	char *pToTbl, 
	int iSlot, 
	int iBid, 
	int iPort )
{
	char sql[1024];
	bool_t ret1;
	print(	DBG_DEBUG, 
					"CopyConfig ftbl:%s totbl:%s slot:%d bid:%d port:%d",
					pFromTbl,pToTbl,iSlot, iBid, iPort);

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "insert into %s select %d, %d, %d, \
				  i_sysmode, i_elevation, i_latitude, i_longitude, i_ssm, i_sa, i_prio, \
				  i_port1_delay, i_port2_delay, i_port3_delay, i_port4_delay, i_port5_delay, \
				  o_delay, o_signaltype, o_delaytype, o_multicast, o_layer, o_step, o_sync, o_announce, \
				  o_delay_req, o_pdelay_req, o_ssm_enable, o_ip, o_mac, o_mask, o_gateway, \
				  o_peer_ip, o_peer_mac, o_priority1, o_priority2, o_Region, o_ptptype \
				  from %s where io_bid = %d and io_port = %d;", 
				  pToTbl, iSlot, iBid, iPort, pFromTbl, iBid, iPort);
	//print(DBG_DEBUG, "%s", sql);	  
	db_lock(pInCtx->pDb);
	ret1 = db_put_table(pInCtx->pDb, sql);
	db_unlock(pInCtx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to copy default config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT], 
				gBoardNameTbl[iBid] );
		return 0;
	}
	else
	{
		return 1;
	}

}

/*
	1	成功
	0	失败
*/
int ReadSchema(struct inCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int tmp;
	int slot = INPUT_SLOT_CURSOR_1+1;
	u8_t src_type = NO_SOURCE_TYPE;
	u8_t src_flag = INVALID_SOURCE_FLAG;
	int max_port = 0, port;
	struct port_attr_t *port_attr = NULL;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select schema from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read schema.");

		return 0;
	}
	
	tmp = atoi(resultp[1]);
	ctx->cs.schema = tmp/100;
	ctx->cs.src_inx = tmp%100;
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	if(1 == ctx->cs.schema)//强制模式
	{
		if(-1 == bis_force(ctx, ctx->cs.src_inx))//FPGA_SYS_INSRC_SELECT选择模式
		{
			print(	DBG_ERROR, 
					"--Failed to force.");

			return 0;
		}
	}
	else//自由模式
	{
		ctx->cs.src_inx = bis_free(ctx);//返回line或NO_SOURCE
		if(-1 == bis_force(ctx, ctx->cs.src_inx))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to free.", 
					gDaemonTbl[DAEMON_CURSOR_INPUT] );

			return 0;
		}
	}

	if(0 != WriteInSourceInfoToShareMemory(ctx))
	{
		print(DBG_ERROR, 
			  "--%s", 
			  "WriteInSourceInfoToShareMemory Error.");
		return -1;
	}

	ctx->cs.slot = 0;
	ctx->cs.bid = BID_NONE;
	ctx->cs.port = 0;
	FIND_TYPE_BY_ID(ctx->cs.src_inx, src_type);
	ctx->cs.type = src_type;
	FIND_FLAG_BY_ID(ctx->cs.src_inx, src_flag);
	ctx->cs.flag = src_flag;
	FIND_SLOT_BY_ID(ctx->cs.src_inx, slot);
	if(slot > INPUT_SLOT_CURSOR_1 && slot <=INPUT_SLOT_CURSOR_ARRAY_SIZE){
		ctx->cs.slot = slot;
		ctx->cs.bid = ctx->inSta[ctx->cs.slot-1].boardId;
		FIND_PORTATTR_GBATBL_BY_BID(ctx->inSta[ctx->cs.slot-1].boardId, port_attr);
		if(!port_attr)
			return 0;
		FIND_MAXPORT_GBATBL_BY_BID(ctx->inSta[ctx->cs.slot-1].boardId, max_port);
		for (port = 0; port < max_port; ++port){
			if (port_attr[port].insrc_type == ctx->cs.type){
				ctx->cs.port = port+1;
			}
		}
	}

	return 1;
}







/*
	1	成功
	0	失败
*/
int WriteSchema(struct inCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set schema=%d;", 
				  pTbl, 
				  (100*(ctx->cs.schema) + ctx->cs.src_inx));
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write schema.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT]);
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
	1	成功
	0	失败
*/
int WriteLeapToShareMemory(struct inCtx *ctx)
{
	struct leapinfo tmp;
	if(1 == ctx->lp_set.leapMode)
	{
		memcpy(&tmp,&ctx->lp_set,sizeof(struct leapinfo));
	}else
	{
		memcpy(&tmp,&ctx->lp_new,sizeof(struct leapinfo));
	}
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT] );

		return 0;
	}
	
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_LPS, 
			  sizeof(struct leapinfo), 
			  (char *)&(tmp), 
			  sizeof(struct leapinfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT] );
		
		return 0;
	}

	return 1;
}


/*
  1		成功
  0		失败
*/
int WriteDataToCurPhasePerf(struct inCtx *ctx,
	struct phase_perf_tbl_t *PhasePerfTbl,
	int PhasePerfLen)
{
	int sys_phase = ~(0);
	u16_t clock_sta = 0;
	static int cur_index = 0;
	struct timeinfo ti;

	if(!(ctx->pDb) || !PhasePerfTbl || PhasePerfLen != TBL_PHASE_PERF_CURRENT_ARRAY_SIZE)
		return 0;
	if(cur_index == PhasePerfLen)
		cur_index = 0;

	SysReadPhase(ctx->fpga_fd, &sys_phase);
	SysReadClockStatus(ctx->fpga_fd, &clock_sta);
					
	PhasePerfTbl[cur_index].index = cur_index + 1;
	PhasePerfTbl[cur_index].val = sys_phase;
	PhasePerfTbl[cur_index].slot = ctx->cs.slot;
	PhasePerfTbl[cur_index].port = ctx->cs.port;
	PhasePerfTbl[cur_index].clock_status = clock_sta;
	memset(&ti, 0x0,sizeof(struct timeinfo));
	if(1 == GetSysTime(&ti)){
		sprintf((char *) PhasePerfTbl[cur_index].time, "%04d-%02d-%02d %02d:%02d:%02d",
					ti.year, ti.month, ti.day, ti.hour, ti.minute, ti.second);
	}	

	//DEBUG
	print(DBG_INFORMATIONAL, "%d | %d | %d | %d | %d | %s",
	PhasePerfTbl[cur_index].index,
	PhasePerfTbl[cur_index].val,
	PhasePerfTbl[cur_index].slot,
	PhasePerfTbl[cur_index].port ,
	PhasePerfTbl[cur_index].clock_status,
	PhasePerfTbl[cur_index].time);

	if(cur_index == PhasePerfLen-1){

		//DEBUG
		print(DBG_DEBUG, "write ph_perf to datebase ...");
		if(0 == WriteCurPhasePerfToTable(ctx->pDb, PhasePerfTbl, PhasePerfLen)){
			print(	DBG_ERROR, 
				"<%s>--Failed to WriteCurPhasePerfToTable.");
			return 0;
		}
		//DEBUG
		print(DBG_DEBUG, "write to ph_perf datebase finish!\n");
	}
	cur_index++;
	return 1;
}

/*
  @ctx		input context
  @pTbl		system table

  1	成功
  0	失败

  read input signal threshold from system table
*/
int ReadPhaseUploadSwitchFromTable(struct inCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select i_ph_upload_en from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read PhaseUploadSwitch.");
		return 0;
	}
	ctx->sys_conf.ph_upload_en = atoi(resultp[ncolumn*nrow+0]);
	//printf("en:%d\n", ctx->sys_conf.ph_upload_en);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}

/*
  1	成功 //tbl_sys
  0	失败
*/
int WritePhaseUploadSwitchToTable(struct inCtx *ctx, char *pTbl)
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set i_ph_upload_en=%d;", 
				  pTbl, ctx->sys_conf.ph_upload_en);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write PhaseUploadSwitch.");
		return 0;
	}
	else
	{
		return 1;
	}
}

int WritePhaseUploadSwitchToShareMemory(struct inCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to lock semaphore.");
		return -1;
	}
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_SYS_CONF_SWI, 
			  sizeof(struct sys_conf_t), 
			  (char *)&(ctx->sys_conf), 
			  sizeof(struct sys_conf_t));
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		return -1;
	}
	return 0;
}


