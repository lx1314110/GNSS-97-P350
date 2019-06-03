#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_sqlite.h"

#include "addr.h"
#include "dout_config.h"
#include "common.h"


typedef int ReadConfigFun_t(struct outCtx *, char *, int, int);
typedef int WriteConfigFun_t(struct outCtx *, struct outputinfo *, char *);

struct rwConfig_t {
	u8_t  boardId;
	ReadConfigFun_t *ReadConfigFun;
	WriteConfigFun_t *WriteConfigFun;
};

static int PTPReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int PTPWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int PTPFReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int PTPFWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

#if 0
static int RS232ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int RS232WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int RS485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int RS485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int BDCTReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int BDCTWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int BACReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int BACWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int SF4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot,int iPort);
static int SF4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int BDC485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int BDC485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int KJD8ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int KJD8WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);
static int KJD4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int KJD4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int TTL8ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int TTL8WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int PPX485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int PPX485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int OUTEReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int OUTEWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int OUTHReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int OUTHWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int TP4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int TP4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

static int SC4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
static int SC4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

#endif

static struct rwConfig_t gRWConfig[] = {
	{BID_PTP,PTPReadConfig,PTPWriteConfig},
	{BID_PTPF,PTPFReadConfig,PTPFWriteConfig},

};
#define LEN_GRWCONFIG (sizeof(gRWConfig)/sizeof(struct rwConfig_t))



/*
  1	成功
  0	失败
*/
int ReadBidFromFpga(struct outCtx * ctx)
{
	u16_t temp[8];
	int i,j;

	for(i=SLOT_CURSOR_1,j=0; (i<SLOT_CURSOR_ARRAY_SIZE)&&(j<8); i+=2,j++)
	{
		if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S01_TO_S16(j), &temp[j]))
		{
			print(	DBG_ERROR, 
				"--Failed to read board identifier[%d] from FPGA.",j);
			return 0;
		}

		ctx->outSta[i].boardId = (temp[j]>>8)&0x00FF;
		//if(0x00FF == ctx->outSta[i].boardId)
		if(BID_ARRAY_SIZE <= ctx->outSta[i].boardId)
		{
			ctx->outSta[i].boardId = 0x00;
		}
		ctx->outSta[i+1].boardId = temp[j]&0x00FF;
		//if(0x00FF == ctx->outSta[i+1].boardId)
		if(BID_ARRAY_SIZE <= ctx->outSta[i+1].boardId)
		{
			ctx->outSta[i+1].boardId = 0x00;
		}

		print(	DBG_DEBUG, 
				"--%d %d %s",
				i+1,
				ctx->outSta[i].boardId,
				gBoardNameTbl[ctx->outSta[i].boardId] );
		
		print(	DBG_DEBUG, 
				"--%d %d %s",
				i+2,
			 	ctx->outSta[i+1].boardId,
				gBoardNameTbl[ctx->outSta[i+1].boardId] );
	}

	return 1;
}








/*
  @ctx		output board context	输出板的环境
  @pTbl		runtime config table	运行配置表
  
  1	成功
  0	失败
  
  read board identifier from runtime table
*/
int ReadBidFromTable(struct outCtx *ctx, char *pTbl)
{
	char sql[128];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int i;
	int slot = 0;

	if(NULL == ctx->pDb)
	{
		return 0;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select io_slot, io_bid \
				  from %s \
				  where io_port=1 order by io_slot;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read board identifier from table.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	
	for(i=1; i<(nrow+1); i++)
	{
		slot = atoi(resultp[ncolumn*i]);
		if (slot > SLOT_CURSOR_ARRAY_SIZE || slot <= 0){
			print(	DBG_DEBUG, 
				"--read slot[%d] over",slot);
			continue;
		}

		ctx->oldBid[slot-1] = atoi(resultp[ncolumn*i+1]);
		//ctx->oldBid[i-1] = atoi(resultp[ncolumn*i+1]);
		print(	DBG_INFORMATIONAL, 
				"--slot:%d oldbid:%d name:%s", 
				slot,
				ctx->oldBid[slot-1],
				gBoardNameTbl[ctx->oldBid[slot-1]] );
	}
//exit:
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}







/*
  @ctx		output board context
  @pTbl		runtime config table
  @iSlot	slot of equipment

  1	成功
  0	失败

  delete config of runtime table
*/
int DeleteConfig(struct outCtx *ctx, char *pTbl, int iSlot)
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "delete from %s where io_slot = %d;", pTbl, iSlot);
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to delete config of slot %d.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], iSlot );
		
		return 0;
	}
	else
	{
		return 1;
	}
}







/*
  @ctx		output board context
  @pTbl		runtime config table
  @iSlot	slot of equipment

  1	成功
  0	失败

  empty config of runtime table
*/
int EmptyConfig(struct outCtx *ctx, char *pTbl, int iSlot)
{
	char sql[128];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "delete from %s where io_slot = %d;", pTbl, iSlot);
	sprintf(sql, "%s insert into %s(io_slot, io_bid, io_port) values(%d, %d, %d);", 
				 sql, pTbl, iSlot, BID_NONE, 1);
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to empty config of slot %d.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], iSlot );
		
		return 0;
	}
	else
	{
		return 1;
	}
}







/*
  @ctx		output board context
  @pFromTbl	default config table
  @pToTbl	runtime config table
  @iSlot	slot of equipment
  @iBid		board identifier
  @iPort	port of board

  1	成功
  0	失败

  copy config from default table to runtime table
*/
int CopyConfig(struct outCtx *ctx, char *pFromTbl, char *pToTbl, int iSlot, int iBid, int iPort)
{
	char sql[1024];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "insert into %s select %d, %d, %d, \
				  i_sysmode, i_elevation, i_latitude, i_longitude, i_ssm, i_sa, i_prio, \
				  i_port1_delay, i_port2_delay, i_port3_delay, i_port4_delay, i_port5_delay, \
				  o_delay, o_signaltype, o_delaytype, o_multicast, o_layer, \
				  o_step, o_sync, o_announce, o_delay_req, o_pdelay_req, \
				  o_ssm_enable, o_ip, o_mac, o_mask, o_gateway,\
				  o_peer_ip, o_peer_mac, o_priority1, o_priority2, o_Region, o_ptptype \
				  from %s where io_bid = %d and io_port = %d;", 
				  pToTbl, iSlot, iBid, iPort, pFromTbl, iBid, iPort);
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to copy default config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[iBid] );
		
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
static int OUTCOMMReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, ctx->outSta[iSlot-1].boardId, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read config of %s.", 
				gBoardNameTbl[ctx->outSta[iSlot-1].boardId] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].outcomminfo[0]), 0, sizeof(struct outcommoninfo));
	ctx->outSta[iSlot-1].outcomminfo[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].outcomminfo[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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

  write config to runtime table and ctx
*/
static int OUTCOMMWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	if (pCfg->set_which != OUTPUTINFO_SET_OUTCOMM_INFO){
			return 1; //ok
	}

	memcpy(&(ctx->outSta[pCfg->slot -1].outcomminfo[0]), &(pCfg->outcomminfo), sizeof(struct outcommoninfo));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->outcomminfo.delay, 
				  pCfg->outcomminfo.signalType, 
				  pCfg->slot, pCfg->boardId, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
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




#if 0
/*
  @ctx			output board context
  @pTbl			runtime config table	
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table
*/
static int RS232ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_RS232, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_RS232] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].rs232[0]), 0, sizeof(struct rs232info));
	ctx->outSta[iSlot-1].rs232[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].rs232[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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

  write config to runtime table and ctx
*/
static int RS232WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].rs232[0]), &(pCfg->rs232), sizeof(struct rs232info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->rs232.delay, 
				  pCfg->rs232.signalType, 
				  pCfg->slot, BID_RS232, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_RS232] );
		
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
static int RS485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_RS485, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_RS485] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].rs485[0]), 0, sizeof(struct rs485info));
	ctx->outSta[iSlot-1].rs485[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].rs485[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int RS485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].rs485[0]), &(pCfg->rs485), sizeof(struct rs485info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->rs485.delay, 
				  pCfg->rs485.signalType, 
				  pCfg->slot, BID_RS485, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_RS485] );
		
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
static int BDCTReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_BDCT, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_BDCT] );

		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].bdct[0]), 0, sizeof(struct bdctinfo));
	ctx->outSta[iSlot-1].bdct[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].bdct[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int BDCTWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].bdct[0]), &(pCfg->bdct), sizeof(struct bdctinfo));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->bdct.delay, 
				  pCfg->bdct.signalType, 
				  pCfg->slot, BID_BDCT, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_BDCT] );
		
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
static int BACReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_BAC, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_BAC] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].bac[0]), 0, sizeof(struct bacinfo));
	ctx->outSta[iSlot-1].bac[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].bac[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int BACWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].bac[0]), &(pCfg->bac), sizeof(struct bacinfo));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->bac.delay, 
				  pCfg->bac.signalType, 
				  pCfg->slot, BID_BAC, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_BAC] );
		
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
static int SF4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_SF4, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_SF4] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].sf4[0]), 0, sizeof(struct sf4info));
	ctx->outSta[iSlot-1].sf4[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].sf4[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int SF4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].sf4[0]), &(pCfg->sf4), sizeof(struct sf4info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->sf4.delay, 
				  pCfg->sf4.signalType, 
				  pCfg->slot, BID_SF4, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_SF4] );
		
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
static int BDC485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_BDC485, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_BDC485] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].bdc485[0]), 0, sizeof(struct bdc485info));
	ctx->outSta[iSlot-1].bdc485[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].bdc485[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int BDC485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].bdc485[0]), &(pCfg->bdc485), sizeof(struct bdc485info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->bdc485.delay, 
				  pCfg->bdc485.signalType, 
				  pCfg->slot, BID_BDC485, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_BDC485] );
		
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
static int KJD8ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_KJD8, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_KJD8] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].kjd8[0]), 0, sizeof(struct kjd8info));
	ctx->outSta[iSlot-1].kjd8[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].kjd8[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int KJD8WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].kjd8[0]), &(pCfg->kjd8), sizeof(struct kjd8info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->kjd8.delay, 
				  pCfg->kjd8.signalType, 
				  pCfg->slot, BID_KJD8, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_KJD8] );
		
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
static int KJD4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_KJD4, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_KJD4] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].kjd4[0]), 0, sizeof(struct kjd4info));
	ctx->outSta[iSlot-1].kjd4[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].kjd4[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int KJD4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].kjd4[0]), &(pCfg->kjd4), sizeof(struct kjd4info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->kjd4.delay, 
				  pCfg->kjd4.signalType, 
				  pCfg->slot, BID_KJD4, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_KJD4] );
		
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
static int TTL8ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_TTL8, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_TTL8] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].ttl8[0]), 0, sizeof(struct ttl8info));
	ctx->outSta[iSlot-1].ttl8[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].ttl8[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int TTL8WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].ttl8[0]), &(pCfg->ttl8), sizeof(struct ttl8info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->ttl8.delay, 
				  pCfg->ttl8.signalType, 
				  pCfg->slot, BID_TTL8, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_TTL8] );
		
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
static int PPX485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_PPX485, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_PPX485] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].ppx485[0]), 0, sizeof(struct ppx485info));
	ctx->outSta[iSlot-1].ppx485[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].ppx485[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int PPX485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].ppx485[0]), &(pCfg->ppx485), sizeof(struct ppx485info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->ppx485.delay, 
				  pCfg->ppx485.signalType, 
				  pCfg->slot, BID_PPX485, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_PPX485] );
		
		return 0;
	}
	else
	{
		return 1;
	}
}
#endif






/*
  @ctx			output board context
  @pTbl			runtime config table	
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table
*/
static int PTPReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[512];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay, o_signaltype, \
				  o_delaytype, o_multicast, o_layer, o_step, o_sync, o_announce, \
				  o_pdelay_req, o_ssm_enable, \
				  o_ip, o_mac, o_peer_ip, o_priority1, o_priority2, \
				  o_Region, o_mask, o_gateway, o_ptptype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_PTP, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_PTP] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].ptp[0]), 0, sizeof(struct ptpinfo));
	//ctx->outSta[iSlot-1].ptp[0].ptpDelay = atoi(resultp[ncolumn*nrow+0]);
	//ctx->outSta[iSlot-1].ptp[0].ptpSignalType = atoi(resultp[ncolumn*nrow+1]);
	ctx->outSta[iSlot-1].ptp[0].ptpDelayType = atoi(resultp[ncolumn*nrow+2]);
	ctx->outSta[iSlot-1].ptp[0].ptpMulticast = atoi(resultp[ncolumn*nrow+3]);
	ctx->outSta[iSlot-1].ptp[0].ptpLayer = atoi(resultp[ncolumn*nrow+4]);
	ctx->outSta[iSlot-1].ptp[0].ptpStep = atoi(resultp[ncolumn*nrow+5]);
	ctx->outSta[iSlot-1].ptp[0].ptpSync = atoi(resultp[ncolumn*nrow+6]);
	ctx->outSta[iSlot-1].ptp[0].ptpAnnounce = atoi(resultp[ncolumn*nrow+7]);
	
	ctx->outSta[iSlot-1].ptp[0].ptpPdelayReq = atoi(resultp[ncolumn*nrow+8]);
	ctx->outSta[iSlot-1].ptp[0].ptpSsmEnable = atoi(resultp[ncolumn*nrow+9]);
	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpIp, 
		   resultp[ncolumn*nrow+10], 
		   strlen(resultp[ncolumn*nrow+10]));
		   
	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpMac, 
		   resultp[ncolumn*nrow+11], 
		   strlen(resultp[ncolumn*nrow+11]));

	str_ip_group_to_uint32(resultp[ncolumn*nrow+12],
		ctx->outSta[iSlot-1].ptp[0].ptpSlaveIp,
		&ctx->outSta[iSlot-1].ptp[0].ptpSlaveIp_num);
	
	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpLevel, 
		   resultp[ncolumn*nrow+13], 
		   strlen(resultp[ncolumn*nrow+13]));
	
	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpPriority, 
		   resultp[ncolumn*nrow+14], 
		   strlen(resultp[ncolumn*nrow+14]));

	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpRegion, 
		   resultp[ncolumn*nrow+15], 
		   strlen(resultp[ncolumn*nrow+15]));
	//add mask, gateway, ptptype, by xpc
	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpMask, 
		   resultp[ncolumn*nrow+16], 
		   strlen(resultp[ncolumn*nrow+16]));
	memcpy(ctx->outSta[iSlot-1].ptp[0].ptpGateway, 
		   resultp[ncolumn*nrow+17], 
		   strlen(resultp[ncolumn*nrow+17]));
	ctx->outSta[iSlot-1].ptp[0].ptpType = atoi(resultp[ncolumn*nrow+18]);//only for ptp
	
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
static int PTPWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[512];
	char des_ipv4_grp[MAX_IP_NUM*MAX_IP_LEN-1];
	bool_t ret1;

	if (pCfg->set_which != OUTPUTINFO_SET_PRIV_INFO){
			return 1; //ok
	}

	memcpy(&(ctx->outSta[pCfg->slot -1].ptp[0]), &(pCfg->ptp), sizeof(struct ptpinfo));

	memset(des_ipv4_grp, 0x0, sizeof(des_ipv4_grp));
	if(!uint32_ip_group_to_str_ip(pCfg->ptp.ptpSlaveIp,
		pCfg->ptp.ptpSlaveIp_num, des_ipv4_grp))
		return 0;//failure
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delaytype=\'%d\', o_multicast=\'%d\', o_layer=\'%d\', o_step=\'%d\', \
				  o_sync=\'%d\', o_announce=\'%d\', o_pdelay_req=\'%d\', o_ssm_enable=\'%d\', \
				  o_ip=\'%s\', o_mask=\'%s\', o_gateway=\'%s\', o_mac=\'%s\', o_peer_ip=\'%s\', o_priority1=\'%s\', o_priority2=\'%s\', \
				  o_Region=\'%s\', o_ptptype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  //pCfg->ptp.ptpDelay, 
				  //pCfg->ptp.ptpSignalType,
				  pCfg->ptp.ptpDelayType,
				  pCfg->ptp.ptpMulticast,
				  pCfg->ptp.ptpLayer,
				  pCfg->ptp.ptpStep,
				  pCfg->ptp.ptpSync,
				  pCfg->ptp.ptpAnnounce,
				  pCfg->ptp.ptpPdelayReq,
				  pCfg->ptp.ptpSsmEnable,//only 国内模块
				  pCfg->ptp.ptpIp,
				  pCfg->ptp.ptpMask,//only国外模块
				  pCfg->ptp.ptpGateway,//only国外模块
				  pCfg->ptp.ptpMac,//only 国内模块
				  des_ipv4_grp,
				  pCfg->ptp.ptpLevel,
				  pCfg->ptp.ptpPriority,
				  pCfg->ptp.ptpRegion,
				  pCfg->ptp.ptpType,
				  pCfg->slot, BID_PTP, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_PTP] );
		
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
static int PTPFReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[512];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay, o_signaltype, \
				  o_delaytype, o_multicast, o_layer, o_step, \
				  o_sync, o_announce, o_delay_req, o_pdelay_req, \
				  o_ip, o_mac, o_mask, o_gateway, \
				  o_dns1, o_dns2, o_mode , o_peer_ip\
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_PTPF, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_PTPF] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].ptpf[0]), 0, sizeof(struct ptpfinfo));
	//ctx->outSta[iSlot-1].ptpf[0].ptpDelay = atoi(resultp[ncolumn*nrow+0]);
	//ctx->outSta[iSlot-1].ptpf[0].ptpSignalType = atoi(resultp[ncolumn*nrow+1]);
	ctx->outSta[iSlot-1].ptpf[0].ptpDelayType = atoi(resultp[ncolumn*nrow+2]);
	ctx->outSta[iSlot-1].ptpf[0].ptpMulticast = atoi(resultp[ncolumn*nrow+3]);
	ctx->outSta[iSlot-1].ptpf[0].ptpLayer = atoi(resultp[ncolumn*nrow+4]);
	ctx->outSta[iSlot-1].ptpf[0].ptpStep = atoi(resultp[ncolumn*nrow+5]);
	ctx->outSta[iSlot-1].ptpf[0].ptpSync = atoi(resultp[ncolumn*nrow+6]);
	ctx->outSta[iSlot-1].ptpf[0].ptpAnnounce = atoi(resultp[ncolumn*nrow+7]);
	ctx->outSta[iSlot-1].ptpf[0].ptpDelayReq = atoi(resultp[ncolumn*nrow+8]);
	ctx->outSta[iSlot-1].ptpf[0].ptpPdelayReq = atoi(resultp[ncolumn*nrow+9]);
	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpIp, 
		   resultp[ncolumn*nrow+10], 
		   strlen(resultp[ncolumn*nrow+10]));
		   
	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpMac, 
		   resultp[ncolumn*nrow+11], 
		   strlen(resultp[ncolumn*nrow+11]));
		   
	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpMask, 
		   resultp[ncolumn*nrow+12], 
		   strlen(resultp[ncolumn*nrow+12]));
		   
	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpGateway, 
		   resultp[ncolumn*nrow+13], 
		   strlen(resultp[ncolumn*nrow+13]));

	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpDns1, 
		   resultp[ncolumn*nrow+14], 
		   strlen(resultp[ncolumn*nrow+14]));

	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpDns2, 
		   resultp[ncolumn*nrow+15], 
		   strlen(resultp[ncolumn*nrow+15]));
	ctx->outSta[iSlot-1].ptpf[0].ptpModelType = atoi(resultp[ncolumn*nrow+16]);

	memcpy(ctx->outSta[iSlot-1].ptpf[0].ptpSlaveIp, 
			resultp[ncolumn*nrow+17], 
			strlen(resultp[ncolumn*nrow+17]));
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
static int PTPFWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[512];
	bool_t ret1;

	if (pCfg->set_which != OUTPUTINFO_SET_PRIV_INFO){
				return 1; //ok
	}

	memcpy(&(ctx->outSta[pCfg->slot -1].ptpf[0]), &(pCfg->ptpf), sizeof(struct ptpfinfo));
//set o_delay=%d, o_signaltype=\'%d\'
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delaytype=\'%d\', o_multicast=\'%d\', o_layer=\'%d\', o_step=\'%d\', \
				  o_sync=\'%d\', o_announce=\'%d\', o_delay_req=\'%d\', o_pdelay_req=\'%d\', \
				  o_ip=\'%s\', o_mac=\'%s\', o_mask=\'%s\', o_gateway=\'%s\', \
				  o_dns1=\'%s\', o_dns2=\'%s\' ,o_mode='\%d\' ,o_peer_ip=\'%s\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  //pCfg->ptpf.ptpDelay, 
				  //pCfg->ptpf.ptpSignalType, 
				  pCfg->ptpf.ptpDelayType,
				  pCfg->ptpf.ptpMulticast,
				  pCfg->ptpf.ptpLayer,
				  pCfg->ptpf.ptpStep,
				  pCfg->ptpf.ptpSync,
				  pCfg->ptpf.ptpAnnounce,
				  pCfg->ptpf.ptpDelayReq,
				  pCfg->ptpf.ptpPdelayReq,
				  pCfg->ptpf.ptpIp,
				  pCfg->ptpf.ptpMac,
				  pCfg->ptpf.ptpMask,
				  pCfg->ptpf.ptpGateway,
				  pCfg->ptpf.ptpDns1,
				  pCfg->ptpf.ptpDns2,
				  pCfg->ptpf.ptpModelType,
				  pCfg->ptpf.ptpSlaveIp,
				  pCfg->slot, BID_PTPF, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_PTPF] );
		
		return 0;
	}
	else
	{
		return 1;
	}
}






#if 0
/*
  @ctx			output board context
  @pTbl			runtime config table	
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table
*/
static int OUTEReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_OUTE, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_OUTE] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].oute[0]), 0, sizeof(struct outeinfo));
	ctx->outSta[iSlot-1].oute[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].oute[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int OUTEWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].oute[0]), &(pCfg->oute), sizeof(struct outeinfo));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->oute.delay, 
				  pCfg->oute.signalType, 
				  pCfg->slot, BID_OUTE, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_OUTE] );
		
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
static int OUTHReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_OUTH, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_OUTH] );

		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].outh[0]), 0, sizeof(struct outhinfo));
	ctx->outSta[iSlot-1].outh[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].outh[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int OUTHWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].outh[0]), &(pCfg->outh), sizeof(struct outhinfo));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->outh.delay, 
				  pCfg->outh.signalType, 
				  pCfg->slot, BID_OUTH, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_OUTH] );

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
static int TP4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_TP4, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_TP4] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].tp4[0]), 0, sizeof(struct tp4info));
	ctx->outSta[iSlot-1].tp4[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].tp4[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int TP4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].tp4[0]), &(pCfg->tp4), sizeof(struct tp4info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->tp4.delay, 
				  pCfg->tp4.signalType, 
				  pCfg->slot, BID_TP4, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_TP4] );
		
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
static int SC4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_delay,o_signaltype \
				  from %s \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, iSlot, BID_SC4, iPort);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_SC4] );
		
		return 0;
	}

	memset(&(ctx->outSta[iSlot-1].sc4[0]), 0, sizeof(struct sc4info));
	ctx->outSta[iSlot-1].sc4[0].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->outSta[iSlot-1].sc4[0].signalType = atoi(resultp[ncolumn*nrow+1]);
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
static int SC4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memcpy(&(ctx->outSta[pCfg->slot -1].sc4[0]), &(pCfg->sc4), sizeof(struct sc4info));

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				  set o_delay=%d, o_signaltype=\'%d\' \
				  where io_slot=%d and io_bid=%d and io_port=%d;", 
				  pTbl, 
				  pCfg->sc4.delay, 
				  pCfg->sc4.signalType, 
				  pCfg->slot, BID_SC4, pCfg->nPort);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write config of %s.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				gBoardNameTbl[BID_SC4] );
		
		return 0;
	}
	else
	{
		return 1;
	}
}
#endif


/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read output timezone from system table
*/
int ReadOutTimezone(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select out_zone from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read output timezone.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	memset(ctx->out_zone.zone, 0, 5);
	memcpy(ctx->out_zone.zone, resultp[1], 4);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}






/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败

  write output timezone to system table
*/
int WriteOutTimezone(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set out_zone=\'%s\';", 
				  pTbl, 
				  ctx->out_zone.zone);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write output timezone.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	else
	{
		return 1;
	}
}








/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read input timezone from system table
*/
int ReadInTimezone(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select in_zone from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read input timezone.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	memset((void *)ctx->in_zone.zone, 0, 5);
	memcpy((void *)ctx->in_zone.zone, resultp[1], 4);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}







/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败

  write input timezone to system table
*/
int WriteInTimezone(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set in_zone=\'%s\';", 
				  pTbl, 
				  ctx->in_zone.zone);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write input timezone.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read leapsecond from system table
*/
int ReadLeapsecond(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select leapsecond, leapmode from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read leapsecond.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	
	ctx->lp.leapSecond = atoi(resultp[2]);
	ctx->lp.leapMode = atoi(resultp[3]);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}







/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  write leapsecond to system table
*/
int WriteLeapsecond(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set leapsecond=%d, leapmode=%d;", 
				  pTbl, 
				  ctx->lp.leapSecond,ctx->lp.leapMode);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write leapsecond.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	else
	{
		return 1;
	}
}


/*
  @ctx		output board context
  @pTbl		system table

  1	成功
  0	失败

  read sa of ssm from system table for 2mb output
*/
int ReadOut2mbSA(struct outCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_2mb_sa from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);	
		print(	DBG_ERROR, "--Failed to read out 2mbSA.");
		return 0;
	}
	memcpy(ctx->out_sa.mb_sa, resultp[ncolumn*nrow+0], strlen(resultp[ncolumn*nrow+0]));
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}


/*
  @ctx		output board context
  @pTbl		system table

  1	成功
  0	失败

  write sa of ssm to system table for 2mb output
*/
int WriteOut2mbSA(struct outCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				 set o_2mb_sa=\'%s\';", 
				 pTbl, 
				 ctx->out_sa.mb_sa);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, "--Failed to write out 2mbSA.");
		return 0;
	}
	else
	{
		return 1;
	}
}



/*
  @ctx		output board context
  @pTbl		system table

  1	成功
  0	失败

  read output signal delay from system table
*/
int ReadDelay(struct outCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select o_tod_delay, o_irigb_delay, o_ppx_delay, o_ptp_delay from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read delay.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	ctx->od[OUTDELAY_SIGNAL_TOD-1].delay = atoi(resultp[ncolumn*nrow+0]);
	ctx->od[OUTDELAY_SIGNAL_IRIGB-1].delay = atoi(resultp[ncolumn*nrow+1]);
	ctx->od[OUTDELAY_SIGNAL_PPX-1].delay = atoi(resultp[ncolumn*nrow+2]);
	ctx->od[OUTDELAY_SIGNAL_PTP-1].delay = atoi(resultp[ncolumn*nrow+3]);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}







/*
  @ctx		output board context
  @pTbl		system table

  1	成功
  0	失败

  write output signal delay to system table
*/
int WriteDelay(struct outCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				 set o_tod_delay=%d, o_irigb_delay=%d, o_ppx_delay=%d, o_ptp_delay=%d;", 
				 pTbl, 
				 ctx->od[OUTDELAY_SIGNAL_TOD-1].delay, 
				 ctx->od[OUTDELAY_SIGNAL_IRIGB-1].delay,
				 ctx->od[OUTDELAY_SIGNAL_PPX-1].delay,
				 ctx->od[OUTDELAY_SIGNAL_PTP-1].delay);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write delay.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	else
	{
		return 1;
	}
}

















/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read output baudrate from system table
*/
int ReadOutBaudrate(struct outCtx *ctx, char *pTbl)
{
	int i;
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select out_br from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read output baudrate.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	memset(ctx->out_br.br, 0, 5);
	for(i=0; i<4; i++)
	{
		ctx->out_br.br[i] = resultp[1][i] - '0';
	}

	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}







/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败

  write output baudrate to system table
*/
int WriteOutRaudrate(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	u8_t temp[5];
	int i;

	memset(temp, 0, 5);
	for(i=0; i<4; i++)
	{
		temp[i] = ctx->out_br.br[i] +'0';
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set out_br=\'%s\';", 
				  pTbl, 
				  temp);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write output baudrate.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	else
	{
		return 1;
	}
}













/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read irigb amplitude from system table
*/
int ReadIrigbAmplitude(struct outCtx *ctx, char *pTbl)
{
	int i;
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select amplitude from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read irigb amplitude.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	memset(ctx->irigb_amp.amp, 0, 5);
	for(i=0; i<4; i++)
	{
		ctx->irigb_amp.amp[i] = resultp[1][i] - '0';
	}

	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}










/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败

  write irigb amplitude to system table
*/
int WriteIrigbAmplitude(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	u8_t temp[5];
	int i;

	memset(temp, 0, 5);
	for(i=0; i<4; i++)
	{
		temp[i] = ctx->irigb_amp.amp[i] +'0';
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set amplitude=\'%s\';", 
				  pTbl, 
				  temp);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write irigb amplitude.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	else
	{
		return 1;
	}
}













/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read irigb voltage from system table
*/
int ReadIrigbVoltage(struct outCtx *ctx, char *pTbl)
{
	int i;
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select voltage from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read irigb voltage.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	memset(ctx->irigb_vol.vol, 0, 5);
	for(i=0; i<4; i++)
	{
		ctx->irigb_vol.vol[i] = resultp[1][i] - '0';
	}

	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}










/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败

  write irigb voltage to system table
*/
int WriteIrigbVoltage(struct outCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	u8_t temp[5];
	int i;

	memset(temp, 0, 5);
	for(i=0; i<4; i++)
	{
		temp[i] = ctx->irigb_vol.vol[i] +'0';
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set voltage=\'%s\';", 
				  pTbl, 
				  temp);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write irigb voltage.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	else
	{
		return 1;
	}
}











#if 0
/*
  1	成功
  0	失败
*/
int ReadConfigFromTable(struct outCtx *ctx, char *pTbl, int iSlot, int iBid, int iPort)
{
	int ret = 1;
	
	if(BID_RS232 == iBid)
	{
		if(0 == RS232ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			//失败
			ret = 0;
		}
	}
	else if(BID_RS485 == iBid)
	{
		if(0 == RS485ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_BDCT == iBid)
	{
		if(0 == BDCTReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_BAC == iBid)
	{
		if(0 == BACReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_SF4 == iBid)
	{
		if(0 == SF4ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_BDC485 == iBid)
	{
		if(0 == BDC485ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_KJD8 == iBid)
	{
		if(0 == KJD8ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_KJD4 == iBid)
	{
		if(0 == KJD4ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_TTL8 == iBid)
	{
		if(0 == TTL8ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_PPX485 == iBid)
	{
		if(0 == PPX485ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_PTP == iBid)
	{
		if(0 == PTPReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_PTPF == iBid)
	{
		if(0 == PTPFReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_OUTE == iBid)
	{
		if(0 == OUTEReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_OUTH == iBid)
	{
		if(0 == OUTHReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_TP4 == iBid)
	{
		if(0 == TP4ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else if(BID_SC4 == iBid)
	{
		if(0 == SC4ReadConfig(ctx, TBL_IO_RUNTIME, iSlot, iPort))
		{
			ret = 0;
		}
	}
	else
	{
		ret = 1;//no [61850 and so on] return success
		print(	DBG_DEBUG,
							"<%s>--bid:%d no config.",
							gDaemonTbl[DAEMON_CURSOR_OUTPUT], iBid);
	}

	return ret;
}
#endif

/*
  @ctx		output board context
  @pTbl			runtime config table
  @iBid			BoardId
  @iSlot		slot of equipment
  @iPort		port of board

  1	成功
  0	失败

  read config from runtime table,only TBL_IO_RUNTIME
*/
int ReadConfigFromRunTimeTable(
	struct outCtx *ctx,
	char *pTbl,
	int iSlot,
	int iBid,
	int iPort )

{
	int i;
	switch (iBid){
		//these output board no support set the table
		//case BID_NTP:
		//case BID_NTPF://only read/save. no support set issue
		case BID_61850:
			return 1;//OK
			break;
	}
	 if(0 == OUTCOMMReadConfig(ctx, pTbl, iSlot, iPort))
		return 0;
	 for (i = 0; i < LEN_GRWCONFIG; ++i){
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
  @ctx		output board context
  @pCfg		output configinfo
  @pTbl			runtime config table

  1	成功
  0	失败

  Write config to runtime table[TBL_IO_RUNTIME] and ctx->outsta
*/
int WriteConfigToRunTimeTable(
struct outCtx *ctx,
struct outputinfo *pCfg,
char *pTbl)
{
	int i;
	switch (pCfg->boardId){
		//these output board no support set the io table
		case BID_NTP:
		case BID_NTPF://only read. no support set 
		case BID_61850:
			return 1;//OK
			break;
	}
	if(0 == OUTCOMMWriteConfig(ctx, pCfg, pTbl))
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
  1	成功
  0	失败
*/
int WriteConfigToShareMemory(struct outCtx *ctx, int iSlot)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_STA +(iSlot-1)*sizeof(struct outputsta), 
			  sizeof(struct outputsta), 
			  (char *)&(ctx->outSta[iSlot-1]), 
			  sizeof(struct outputsta));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	return 1;
}






/*
  1	成功
  0	失败
*/
int WriteOutZoneToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_OZONE, 
			  sizeof(struct outzoneinfo), 
			  (char *)&(ctx->out_zone), 
			  sizeof(struct outzoneinfo));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	return 1;
}







/*
  1	成功
  0	失败
*/
int WriteInZoneToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_IZONE, 
			  sizeof(struct inzoneinfo), 
			  (char *)&(ctx->in_zone), 
			  sizeof(struct inzoneinfo));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}

	return 1;
}







/*
  1	成功
  0	失败
*/
int WriteLeapToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_LPS, 
			  sizeof(struct leapinfo), 
			  (char *)&(ctx->lp), 
			  sizeof(struct leapinfo));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	return 1;
}






/*
  1	成功
  0	失败
*/
int WriteDelayToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print( DBG_ERROR, 
			   "<%s>--Failed to lock semaphore.", 
			   gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	print(	DBG_INFORMATIONAL, 
			"--outdelay: %d %d %d %d outdelay_sharemem(must < 240):%d",
			ctx->od[OUTDELAY_SIGNAL_TOD-1].delay, 
			ctx->od[OUTDELAY_SIGNAL_IRIGB-1].delay,
			ctx->od[OUTDELAY_SIGNAL_PPX-1].delay,
			ctx->od[OUTDELAY_SIGNAL_PTP-1].delay,
			OUTDELAY_SIGNALTYPE_LEN*sizeof(struct outdelay));
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_ODELAY, 
			  OUTDELAY_SIGNALTYPE_LEN*sizeof(struct outdelay), 
			  (char *)ctx->od, 
			  OUTDELAY_SIGNALTYPE_LEN*sizeof(struct outdelay));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	return 1;
}

/*
  1	成功
  0	失败
*/
int WriteOut2mbSAToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print( DBG_ERROR,"--Failed to lock semaphore.");
		
		return 0;
	}

	print(	DBG_INFORMATIONAL, 
			"--out 2mb sa: %s", ctx->out_sa.mb_sa);
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_OBSA, 
			  sizeof(struct out2mbsainfo), 
			  (char *)&ctx->out_sa, 
			  sizeof(struct out2mbsainfo));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, "--Failed to unlock semaphore.");		
		return 0;
	}

	return 1;
}








/*
  1	成功
  0	失败
*/
int WriteOutBaudrateToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print( DBG_ERROR, 
			   "<%s>--Failed to lock semaphore.", 
			   gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	print(	DBG_INFORMATIONAL, 
			"--baudrate: %s %s %s %s", 
			gBaudRateTbl[ctx->out_br.br[0]], 
			gBaudRateTbl[ctx->out_br.br[1]],
			gBaudRateTbl[ctx->out_br.br[2]],
			gBaudRateTbl[ctx->out_br.br[3]] );
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_OBR, 
			  sizeof(struct baudrate), 
			  (char *)&ctx->out_br, 
			  sizeof(struct baudrate));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	return 1;
}











/*
  1	成功
  0	失败
*/
int WriteIrigbAmplitudeToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print( DBG_ERROR, 
			   "--Failed to lock semaphore.");
		
		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_AMP, 
			  sizeof(struct amplitude), 
			  (char *)&ctx->irigb_amp, 
			  sizeof(struct amplitude));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"--Failed to unlock semaphore.");
		
		return 0;
	}

	return 1;
}












/*
  1	成功
  0	失败
*/
int WriteIrigbVoltageToShareMemory(struct outCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print( DBG_ERROR, 
			   "<%s>--Failed to lock semaphore.", 
			   gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_VOL, 
			  sizeof(struct voltage), 
			  (char *)&ctx->irigb_vol, 
			  sizeof(struct voltage));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	return 1;
}













/*
  1	成功
  0	失败
*/
int WriteAlarmToShareMemory(struct outCtx *ctx)
{
	//write alarm to share memory
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT]);
		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta), 
			  (char *)ctx->alarm, 
			  ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT]);
		return 0;
	}

	return 1;
}












