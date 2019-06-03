#include <string.h>

#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_log.h"
#include "lib_time.h"

#include "din_source.h"
#include "din_issue.h"
#include "din_config.h"
#include "din_parser.h"
#include "alloc.h"





/*
  true:		收到配置命令
  false:	没有收到配置命令
*/
bool_t isNotify(struct inCtx *ctx)
{
	if(ctx->notify_flag)
	{
 		ctx->notify_flag = false;
 		return true;
	}
	else
	{
		return false;
	}
}



/*
  1	成功
  0	失败
*/
int sysLevel(char *data, struct inCtx *ctx)
{
	struct dbginfo *dbg;

	dbg = (struct dbginfo *)data;
	if(0 == set_print_level(dbg->syslog_en, dbg->dbg_level, DAEMON_CURSOR_INPUT))
	{
		print(	DBG_ERROR, 
				"%s", "Failed to set print level.");
		return 0;
	}
	print(DBG_INFORMATIONAL, 
			  "--sys syslog:%d dbg_level:%d", dbg->syslog_en,
			  dbg->dbg_level);

	return 1;
}

#ifdef NEW_ISSUECONFIG_INTERFACE
/*
  1	成功
  0	失败
*/
int sysInput(char *data, struct inCtx *ctx)
{
	int slot = 0;
	struct inputinfo *pCfg;

	pCfg = (struct inputinfo *)data;
	slot = pCfg->slot;

	//ctx->inSta[slot -1].boardId = BID_GPSBF;
	print(DBG_INFORMATIONAL, "setting slot:%d boardId:%d set_which:%d", slot, pCfg->boardId, pCfg->set_which);
	if(0 == WriteConfigToRunTimeTable(ctx, pCfg, TBL_IO_RUNTIME))//写配置到实时配置表
	{
		return 0;
	}

	if(0 == IssueConfig(ctx, pCfg->slot, pCfg->boardId, pCfg->set_which))//设置配置信息[到fpga]
	{
		return 0;
	}

	return 1;
}

#else
/*
  1	成功
  0	失败
*/
int sysInput(char *data, struct inCtx *ctx)
{
	struct inputinfo *pCfg;

	pCfg = (struct inputinfo *)data;
	if(BID_GPSBF == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_GPSBF;//why?I tested, it shoule not do so.
		if(0 == GPSBFWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))//写配置到实时配置表
		{
			return 0;
		}

		if(0 == GPSBFIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].gpsbf.gpsbf), pCfg->slot))//设置GPS信息
		{
			return 0;
		}
	}
	else if(BID_GPSBE == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_GPSBE;
		if(0 == GPSBEWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == GPSBEIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].gpsbe.gpsbe), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_GBDBF == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_GBDBF;
		if(0 == GBDBFWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == GBDBFIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].gbdbf.gbdbf), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_GBDBE == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_GBDBE;
		if(0 == GBDBEWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == GBDBEIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].gbdbe.gbdbe), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_BDBF == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_BDBF;
		if(0 == BDBFWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == BDBFIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].bdbf.bdbf), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_BDBE == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_BDBE;
		if(0 == BDBEWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == BDBEIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].bdbe.bdbe), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_BEI == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_BEI;
		if(0 == BEIWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == BEIIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].bei.bei), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_BFI == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_BFI;
		if(0 == BFIWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == BFIIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].bfi.bfi), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_BFEI == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_BFEI;
		if(0 == BFEIWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == BFEIIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].bfei.bfei), pCfg->slot))
		{
			return 0;
		}
	}
	else if(BID_BFFI == pCfg->boardId)
	{
		ctx->inSta[pCfg->slot -1].boardId = BID_BFFI;
		if(0 == BFFIWriteConfig(ctx, pCfg, TBL_IO_RUNTIME))
		{
			return 0;
		}

		if(0 == BFFIIssueConfig(ctx->fpga_fd, &(ctx->inSta[pCfg->slot -1].bffi.bffi), pCfg->slot))
		{
			return 0;
		}
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_INPUT], 
				"Invalid board identifier.");

		return 0;
	}
	
	return 1;
}
#endif



/*
  1	成功
  0	失败
*/
int sysSchema(char *data, struct inCtx *ctx)
{
	u8_t tmp;
	int slot = INPUT_SLOT_CURSOR_1+1;
	u8_t src_type = NO_SOURCE_TYPE;
	u8_t src_flag = INVALID_SOURCE_FLAG;
	int max_port = 0, port;
	struct port_attr_t *port_attr = NULL;

	struct schemainfo *pCfg;
	char buf[256];

	pCfg = (struct schemainfo *)data;
	ctx->cs.schema = pCfg->schema;
	
	if(1 == ctx->cs.schema)//force模式
	{
		if(ctx->cs.src_inx != pCfg->src_inx)
		{
			ctx->cs.src_inx = pCfg->src_inx;
			if(-1 == bis_force(ctx, ctx->cs.src_inx))
			{
				return 0;
			}

			//log
			//---------------------------------------------------------
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%s%s", "Time Source is ", gTimeSourceTbl[ctx->cs.src_inx].msg);
			db_log_write(ctx->pDb, TBL_LOG, buf);
			//---------------------------------------------------------
		}
	}
	else//free模式
	{
		tmp = bis_free(ctx);
		if(ctx->cs.src_inx != tmp)
		{
			ctx->cs.src_inx = tmp;
			if(-1 == bis_force(ctx, ctx->cs.src_inx))
			{
				return 0;
			}

			//log
			//---------------------------------------------------------
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%s%s", "Time Source is ", gTimeSourceTbl[ctx->cs.src_inx].msg);
			db_log_write(ctx->pDb, TBL_LOG, buf);
			//---------------------------------------------------------
		}
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

	if(0 == WriteSchema(ctx, TBL_SYS))//向数据库写入选源信息
	{
		return 0;
	}

	if(0 != WriteInSourceInfoToShareMemory(ctx))
	{
		print(DBG_ERROR, 
			  "--%s", 
			  "WriteInSourceInfoToShareMemory Error.");
		return 0;
	}

	return 1;
}


/*
  1	成功
  0	失败
*/
int leapModeSet(char *data, struct inCtx *ctx)
{
	struct leapinfo *pCfg;
	pCfg = (struct leapinfo *)data;
	memcpy(&ctx->lp_set, pCfg, sizeof(struct leapinfo));
	return 1;
}

/*
  1	成功
  0	失败
*/
//include PhaseUploadSet
int SysConfSet(char *data, struct inCtx *ctx)
{
	struct sys_conf_t *pCfg;
	pCfg = (struct sys_conf_t *)data;
	memcpy(&ctx->sys_conf, pCfg, sizeof(struct sys_conf_t));

	
	if(0 == WritePhaseUploadSwitchToTable(ctx, TBL_SYS))//向数据库写入信息
	{
		return 0;
	}
	if(0 != WritePhaseUploadSwitchToShareMemory(ctx))
	{
		print(DBG_ERROR, 
			  "--%s", 
			  "WritePhaseUploadSwitchToShareMemory Error.");
		return 0;
	}
	return 1;
}


/*
  1	成功
  0	失败
*/
int ReadMessageQueue(struct inCtx *ctx)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
		ret = MsgQ_Rcv(ctx->ipc.ipc_msgq_id, &msg);
		if(-1 == ret)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_INPUT], 
					"Failed to read message queue");
			return 0;
		}
		
		if(ret > 0)
		{
			if(MSG_CFG_DBG == msg.mtype)
			{
				if(0 == sysLevel(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_IN == msg.mtype)//输入信号配置
			{
				if(0 == sysInput(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_SCHEMA == msg.mtype)//选源设置及保存
			{
				if(0 == sysSchema(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_LPS_MODE_SET == msg.mtype)//闰秒模式设置及保存
			{
				if(0 == leapModeSet(msg.mdata, ctx))
				{
					return 0;
				}
				//sysleap set
				SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SYSLEAP_SET, SID_SYS_SLOT, SID_SYS_PORT);
			}
			else if(MSG_CFG_SYS_CONF_SWI == msg.mtype)//性能上报开关设置及保存
			{
				if(0 == SysConfSet(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else
			{
				print(	DBG_ERROR, 
						"--%s", 
						"Invalid message type");
				return 0;
			}
		}
	}while(ret > 0);

	return 1;
}






/*
  1	成功
  0	失败
*/
int WriteConfigToShareMemory(struct inCtx *ctx, int iSlot)
{
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT]);
		return 0;
	}
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_STA +(iSlot-1)*sizeof(struct inputsta), 
			  sizeof(struct inputsta), 
			  (char *)&(ctx->inSta[iSlot-1]), 
			  sizeof(struct inputsta));
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT]);
		return 0;
	}

	return 1;
}





/*
  1	成功
  0	失败
*/
int WriteAlarmToShareMemory(struct inCtx *ctx)
{
	
	//write alarm to share memory
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT]);
		return 0;
	}
	
	shm_write(ctx->ipc.ipc_base, 
			  SHM_OFFSET_ALM, 
			  ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta), 
			  (char *)ctx->alarm, 
			  ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT]);
		return 0;
	}

	return 1;
}




