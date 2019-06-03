#include <string.h>
#include <unistd.h>

#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_fpga.h"

#include "addr.h"
#include "dalm_config.h"
#include "dalm_parser.h"


/*
  true:		收到配置命令
  false:	没有收到配置命令
*/
bool_t isNotify(struct almCtx *ctx)
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
int sysLevel(char *data, struct almCtx *ctx)
{
	struct dbginfo *dbg;

	dbg = (struct dbginfo *)data;
	if(0 == set_print_level(dbg->syslog_en, dbg->dbg_level, DAEMON_CURSOR_ALARM))
	{
		print(DBG_ERROR, 
			"Failed to set print level.");	
		return 0;
	}
	print(DBG_INFORMATIONAL, 
		  "--sys syslog:%d dbg_level:%d", dbg->syslog_en,
		  dbg->dbg_level);

	return 1;
}

/*
  1	成功
  0	失败
*/
int sysMask(char *data, struct almCtx *ctx)
{
	struct maskinfo *tmp;
	int i;
	
	tmp = (struct maskinfo *)data;
	if(tmp->alm_inx == SNMP_ALM_ID_LINK_DOWN)
	{
		//对所有输出使能进行设置
		for (i = 0; i < max_alminx_num; ++i){
			if( pAlmInxTbl[i].alm_id == SNMP_ALM_ID_LINK_DOWN)
				ctx->mask[i].mask_sta = tmp->mask_sta;
		}
		
	}
	else 
	{
		//对其他报警屏蔽进行设置
		ctx->mask[tmp->alm_inx].mask_sta = tmp->mask_sta;
	}
	
	print(	DBG_INFORMATIONAL, 
			"--sys alm_inx:%d mask_sta:%d", 
			tmp->alm_inx,
			tmp->mask_sta);
	
	if(0 == WriteMaskToShareMemory(ctx))
	{
		return 0;
	}

	if(0 == WriteAlarmMaskToTable(ctx, tmp, TBL_ALARM_SHIELD))
	{
		return 0;
	}

	return 1;
}

int sys_port_status(char *data, struct almCtx *ctx)
{
	struct portinfo *tmp;
	tmp = (struct portinfo *)data;
	ctx->port_status[tmp->port - 1].enable = tmp->enable;
	
	return 1;
}

int sys_inph_thresold(char *data, struct almCtx *ctx)
{
	struct inph_thresold *tmp;
	tmp = (struct inph_thresold *)data;

	ctx->InSgnl_Threshold.Threshold = tmp->Threshold;
	print(	DBG_INFORMATIONAL, 
			"--sys Threshold: %d",
			//tmp->port,
			tmp->Threshold);

	if(0 == WriteSignalThresholdToShareMemory(ctx))
	{
		return 0;
	}

	if(0 == WriteSignalThresholdToTable(ctx, tmp, TBL_SYS))
	{
		return 0;
	}

	return 1;
}

/*
  1	成功
  0	失败
*/
int sysSelect(char *data, struct almCtx *ctx)
{
	struct selectinfo *tmp;

	tmp = (struct selectinfo *)data;

	if(tmp->alm_board_port > PORT_ALM)
		return 0;

	ctx->select[tmp->alm_board_port -1].alm_inx = tmp->alm_inx;
	print(	DBG_INFORMATIONAL, 
			"--sys port:%d alm_inx:%d",
			tmp->alm_board_port,
			tmp->alm_inx);
	
	if(0 == WriteSelectToShareMemory(ctx))
	{
		return 0;
	}

	if(0 == WriteAlarmSelectToTable(ctx, tmp, TBL_ALARM_SELECT))
	{
		return 0;
	}

	return 1;
}








/*
  1	成功
  0	失败
*/
int ReadMessageQueue(struct almCtx *ctx)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
		ret = MsgQ_Rcv(ctx->ipc[IPC_CURSOR_ALARM].ipc_msgq_id, &msg);
		if(ret < 0)
		{
			print(	DBG_ERROR,
					"Failed to read message queue");
			return 0;
		}
		
		if(ret > 0)
		{
			if(MSG_CFG_DBG == msg.mtype)//调试
			{
				if(0 == sysLevel(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_SELECT == msg.mtype)//告警选择表（输出端口、标识符）
			{
				if(0 == sysSelect(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_MASK == msg.mtype)//告警屏蔽状态
			{
				if(0 == sysMask(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_PORT_STATUS == msg.mtype)//端口使能状态
			{
				if(0 == sys_port_status(msg.mdata, ctx))
				{
					return 0;
				}
			}	//鉴相值阈值/输入信号性能
			else if(MSG_CFG_INPH_THRESHOLD == msg.mtype)//端口使能状态
			{
				if(0 == sys_inph_thresold(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else
			{
				print(	DBG_ERROR, 
						"Invalid message type");
				return 0;
			}
		}
	}while(ret > 0);
	
	return 1;
}





