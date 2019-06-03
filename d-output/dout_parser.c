#include <string.h>

#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_shm.h"
#include "lib_sema.h"

#include "alloc.h"

#include "dout_issue.h"
#include "dout_config.h"
#include "dout_parser.h"


extern u8_t gtraceTbl[PTP_TRACE_ARRAY_SIZE][2];


/*
  true:		收到配置命令
  false:	没有收到配置命令
*/
bool_t isNotify(struct outCtx *ctx)
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
int sysLevel(char *data, struct outCtx *ctx)
{
	struct dbginfo *dbg;

	dbg = (struct dbginfo *)data;
	if(0 == set_print_level(dbg->syslog_en, dbg->dbg_level, DAEMON_CURSOR_OUTPUT))//设置打印级别
	{
		print( DBG_ERROR, 
			   "%s","Failed to set print level.");
		
		return 0;
	}
	
	print( DBG_INFORMATIONAL, 
		   "syslog:%d dbg_level:%d", dbg->syslog_en, dbg->dbg_level);
	
	return 1;
}








/*
  1	成功
  0	失败
*/
int sysInZone(char *data, struct outCtx *ctx)
{
	struct inzoneinfo *pCfg;

	pCfg = (struct inzoneinfo *)data;
	memcpy(&ctx->in_zone, pCfg, sizeof(struct inzoneinfo));

	if(0 == WriteInTimezone(ctx, TBL_SYS))//写时区到数据库
	{
		return 0;
	}

	print(	DBG_INFORMATIONAL, 
			"<%s>--%s", 
			gDaemonTbl[DAEMON_CURSOR_OUTPUT],
			pCfg->zone);

	if(0 == WriteInZoneToShareMemory(ctx))//写时区到共享内存
	{
		return 0;
	}

	return 1;
}








/*
  1	成功
  0	失败
*/
int sysOutZone(char *data, struct outCtx *ctx)
{
	struct outzoneinfo *pCfg;

	pCfg = (struct outzoneinfo *)data;
	memcpy(&ctx->out_zone, pCfg, sizeof(struct outzoneinfo));

	if(0 == IssueIrigbZone(ctx))
	{
		return 0;
	}

	if(0 == WriteOutTimezone(ctx, TBL_SYS))
	{
		return 0;
	}

	if(0 == WriteOutZoneToShareMemory(ctx))
	{
		return 0;
	}

	print(	DBG_INFORMATIONAL, 
			"<%s>--%s", 
			gDaemonTbl[DAEMON_CURSOR_OUTPUT],
			pCfg->zone	);

	return 1;
}






/*
  1	成功
  0	失败
*/
int sysLeapsecond(char *data, struct outCtx *ctx)
{
	struct leapinfo *pCfg;

	pCfg = (struct leapinfo *)data;
	memcpy(&ctx->lp, pCfg, sizeof(struct leapinfo));
	
	if( ctx->lp.leapSecond != 0)
	{
		if(0 == WriteLeapsecond(ctx, TBL_SYS))
		{
			return 0;
		}
		/*
		if(0 == WriteLeapToShareMemory(ctx))
		{
			return 0;
		}
		*/
	}
	

	print(	DBG_INFORMATIONAL, 
			"<%s>--leapSecond:%d", 
			gDaemonTbl[DAEMON_CURSOR_OUTPUT],
			pCfg->leapSecond	);

	return 1;
}






/*
  copy pcfg->xxboardinfo [port 0..or3] to ctx->outsta[slot].xxboardinfo[port0]
   未来若每个port的配置不一致，可以改为memcpy(&(ctx->outSta[pCfg->slot -1].rs232[pCfg->nPort]), &(pCfg->rs232), sizeof(struct rs232info));
  1	成功
  0	失败
*/
int sysOutput(char *data, struct outCtx *ctx)
{
	int slot = 0;
	struct outputinfo *pCfg;

	pCfg = (struct outputinfo *)data;
	slot = pCfg->slot;

	//ctx->inSta[slot -1].boardId = BID_xxx;
	if(0 == WriteConfigToRunTimeTable(ctx, pCfg, TBL_IO_RUNTIME))//写配置到实时配置表
	{
		return 0;
	}
	/*为了支持snmp多次配置一个板卡，如ptp；p350out ptp 10 pdt e2e; p350out ptp 10 pmut uni; p350out ptp 10 preg 254
	cmd命令在设置之前，需要首先读取共享内存的原所有值，再修改相应的项，所以这里要及时的反馈到共享内存中
	建议snmp设置ptp的每个项时，间隔时间为5s*/
	if(0 == WriteConfigToShareMemory(ctx, pCfg->slot))
	{
		return 0;
	}

	if(0 == IssueConfig(ctx, pCfg->slot, pCfg->boardId))//设置out信息[到fpga]
	{
		return 0;
	}

	/*仅仅是为了把issueConfig产生的old值保存，比如oldptpSlaveIp*/
	if(0 == WriteConfigToShareMemory(ctx, pCfg->slot))
	{
		return 0;
	}

	return 1;
}




/*
  1	成功
  0	失败
*/
int sysDelay(char *data, struct outCtx *ctx)
{
	struct outdelay *tmp;

	tmp = (struct outdelay *)data;
	if (tmp->signal > OUTDELAY_SIGNALTYPE_LEN || tmp->signal <= 0) {
		return 0;
	}
	ctx->od[tmp->signal-1].delay = tmp->delay;

	if(0 == IssueDelay(ctx))
	{
		return 0;
	}

	if(0 == WriteDelay(ctx, TBL_SYS))
	{
		return 0;
	}

	if(0 == WriteDelayToShareMemory(ctx))
	{
		return 0;
	}

	return 1;
}




/*
  1	成功
  0	失败
*/
int sysOut2mbSA(char *data, struct outCtx *ctx)
{
	struct out2mbsainfo *tmp;

	tmp = (struct out2mbsainfo *)data;
	memcpy(&ctx->out_sa, tmp, sizeof(struct out2mbsainfo));

	if(0 == IssueOut2mbSA(ctx))
	{
		return 0;
	}

	if(0 == WriteOut2mbSA(ctx, TBL_SYS))
	{
		return 0;
	}

	if(0 == WriteOut2mbSAToShareMemory(ctx))
	{
		return 0;
	}

	return 1;
}









/*
  1	成功
  0	失败
*/
int sysOutBaudrate(char *data, struct outCtx *ctx)
{
	struct baudrate *tmp;

	tmp = (struct baudrate *)data;
	memcpy(&ctx->out_br, tmp, sizeof(struct baudrate));

	if(0 == IssueOutBaudrate(ctx))
	{
		return 0;
	}

	if(0 == WriteOutRaudrate(ctx, TBL_SYS))
	{
		return 0;
	}

	if(0 == WriteOutBaudrateToShareMemory(ctx))
	{
		return 0;
	}

	return 1;
}














/*
  1	成功
  0	失败
*/
int sysIrigbAmplitude(char *data, struct outCtx *ctx)
{
	struct amplitude *tmp;

	tmp = (struct amplitude *)data;
	memcpy(&ctx->irigb_amp, tmp, sizeof(struct amplitude));

	if(0 == IssueIrigbAmplitude(ctx))
	{
		return 0;
	}

	if(0 == WriteIrigbAmplitude(ctx, TBL_SYS))
	{
		return 0;
	}

	if(0 == WriteIrigbAmplitudeToShareMemory(ctx))
	{
		return 0;
	}

	return 1;
}














/*
  1	成功
  0	失败
*/
int sysIrigbVoltage(char *data, struct outCtx *ctx)
{
	struct voltage *tmp;

	tmp = (struct voltage *)data;
	memcpy(&ctx->irigb_vol, tmp, sizeof(struct voltage));

	if(0 == IssueIrigbVoltage(ctx))
	{
		return 0;
	}

	if(0 == WriteIrigbVoltage(ctx, TBL_SYS))
	{
		return 0;
	}

	if(0 == WriteIrigbVoltageToShareMemory(ctx))
	{
		return 0;
	}

	return 1;
}












/*
  1	成功
  0	失败
*/
int ReadMessageQueue(struct outCtx *ctx)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
		ret = MsgQ_Rcv(ctx->ipc[IPC_CURSOR_OUTPUT].ipc_msgq_id, &msg);
		if(-1 == ret)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
			else if(MSG_CFG_OUT == msg.mtype)//保存配置以及向对应的槽发送输出信号
			{
				if(0 == sysOutput(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_IZONE == msg.mtype)//保存输入时区到数据库、共享内存
			{
				if(0 == sysInZone(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_OZONE == msg.mtype)//保存输出时区到数据库、共享内存
			{
				if(0 == sysOutZone(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_LPS == msg.mtype)//保存闰秒
			{
				if(0 == sysLeapsecond(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_OUT_DELAY == msg.mtype)//保存输出延时
			{
				if(0 == sysDelay(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_OBSA == msg.mtype)//保存2mb out SA
			{
				if(0 == sysOut2mbSA(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_OBR == msg.mtype)//保存波特率
			{
				if(0 == sysOutBaudrate(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_AMP == msg.mtype)//保存振幅
			{
				if(0 == sysIrigbAmplitude(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_VOL == msg.mtype)//保存电压
			{
				if(0 == sysIrigbVoltage(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
						"Invalid message type");
				return 0;
			}
		}
	}while(ret > 0);
	
	return 1;
}











