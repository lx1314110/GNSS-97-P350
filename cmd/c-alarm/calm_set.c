#include <string.h>

#include "lib_msgQ.h"
#include "lib_common.h"

#include "calm_set.h"






/*
  -1	失败
   0	成功
*/
int alarm_mask_set(struct almCtx *ctx, u32_t alm_inx, u8_t mask_sta)
{
	struct config cfg;
	struct msgbuf msg;

	memset(&cfg, 0, sizeof(struct config));
	cfg.mask_config.alm_inx = alm_inx;
	cfg.mask_config.mask_sta = mask_sta;
	
	msg.mtype = MSG_CFG_MASK;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct maskinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct maskinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

/*
  -1	失败
   0	成功
*/
int alarm_inphase_threshold_set(struct almCtx *ctx, int threshold)
{
	struct config cfg;
	struct msgbuf msg;

	memset(&cfg, 0, sizeof(struct config));
	cfg.thresold_config.Threshold = threshold;
	
	msg.mtype = MSG_CFG_INPH_THRESHOLD;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct inph_thresold));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inph_thresold)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}






/*
  -1	失败
   0	成功
*/
int alarm_select_set(struct almCtx *ctx, u8_t port, u32_t alm_inx)
{
	struct config cfg;
	struct msgbuf msg;

	memset(&cfg, 0, sizeof(struct config));
	cfg.select_config.alm_board_port = port;
	cfg.select_config.alm_inx = alm_inx;
	
	msg.mtype = MSG_CFG_SELECT;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct selectinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct selectinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}




