#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lib_shm.h"
#include "lib_alarm.h"
#include "lib_fpga.h"
#include "alloc.h"

#include "calm_alloc.h"
#include "calm_macro.h"
#include "calm_global.h"
#include "calm_history.h"
#include "calm_set.h"
#include "calm_query.h"
#include "calm_parser.h"




int argv_parser(int argc, char *argv[], struct almCtx *ctx)
{
	int ret = 0;
	int tmp;
	
	if(-1 == initializeMessageQueue(&ctx->ipc))
	{
		ret = -1;
		goto exit0;
	}

	if(-1 == initializeSemaphoreSet(&ctx->ipc))
	{
		ret = -1;
		goto exit0;
	}

	if(-1 == initializeShareMemory(&ctx->ipc))
	{
		ret = -1;
		goto exit0;
	}
	if(-1 == initializeDatabase(ctx))
	{
		ret = -1;
		goto exit1;
	}
	if(-1 == readPid(&ctx->ipc, &ctx->pid))//从共享内存读PID
	{
		ret = -1;
		goto exit2;
	}

	initializeBoardNameTable();

#ifdef NEW_ALARMID_COMM
	//initialize
	ctx->fpga_fd = -1;
	memset(ctx->bid,0x0,sizeof(ctx->bid));
	memset(ctx->bid_alm_num,0x0,sizeof(ctx->bid_alm_num));
	memset(ctx->cur_alarm, 0x0, sizeof(ctx->cur_alarm));
	memset(ctx->upload_alarm, 0x0, sizeof(ctx->upload_alarm));

	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		ret = -1;
		goto exit2;
	}

	if(-1 == ReadBidFromFpga(ctx))//从FPGA读bid
	{
		ret = -1;
		goto exit3;
	}

	if(0 != initializeAlarmIndexTable())
	{
		ret = -1;
		goto exit3;
	}
#endif

	if(1 == argc)
	{
		if(-1 == current_alarm_list_query(ctx))//显示当前所有告警列表及屏蔽状态
		{
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}
	else if(2 == argc)
	{
		if(0 == memcmp(argv[1], HELP, strlen(HELP)))
		{
			helper();
		}else if(0 == memcmp(argv[1], BOARDALM, strlen(BOARDALM)))
		{
			if(-1 == bd_alarm_sta_query(ctx))//单板是否有告警查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], THRESHOLD, strlen(THRESHOLD)))
		{
			if(-1 == alarm_inphase_threshold_query(ctx))//信号/鉴相值门限查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], CURRENT, strlen(CURRENT)) ||
			0 == memcmp(argv[1], UPLOAD, strlen(UPLOAD)))
		{
			tmp = 1;//1
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[1], CURRENTNUM, strlen(CURRENTNUM)))
		{
			tmp = current_alarm_nums(ctx);//获取当前所有告警数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[1], UPLOADNUM, strlen(UPLOADNUM)))
		{
			tmp = upload_alarm_nums(ctx);//获取当前所有告警数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[1], HISTORY, strlen(HISTORY)))
		{
			tmp = history_alarm_pages(ctx->pDb, TBL_ALARM_HISTORY);//获取记录数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[1], HISTORYNUM, strlen(HISTORYNUM)))
		{
			tmp = history_alarm_nums(ctx->pDb, TBL_ALARM_HISTORY);//获取记录数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[1], "delete", strlen("delete")))
		{
			char sql[128];
			bool_t ret1;

			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s;", TBL_ALARM_HISTORY);
			db_lock(ctx->pDb);
			ret1 = db_put_table(ctx->pDb, sql);
			db_unlock(ctx->pDb);
			if(false == ret1)
			{
				printf("fail to delete log\n");
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else
		{
			ret = -1;
		}
	}
	else if(3 == argc)
	{
		if( (0 == memcmp(argv[1], MASK, strlen(MASK))) &&
			(alarm_id_validity(argv[2])) )
		{
			if(-1 == alarm_mask_query(ctx, atoi(argv[2])))//告警屏蔽表查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if( (0 == memcmp(argv[1], SELECT, strlen(SELECT))) &&
				(almport_validity(argv[2])) )
		{
			if(-1 == alarm_select_query(ctx, atoi(argv[2])))//告警输出板输出查询
			{
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else if( (0 == memcmp(argv[1], THRESHOLD, strlen(THRESHOLD))) &&
				(threshold_validity(argv[2])) )
		{
			if(-1 == alarm_inphase_threshold_set(ctx, atoi(argv[2])))//信号/鉴相值门限设置
			{
				ret = -1;
			}
			else
			{
				printf("Success.\n");
				ret = 0;
			}
		}
		else if(0 == memcmp(argv[1], CURRENT, strlen(CURRENT)))
		{
			if(atoi(argv[2]) == 1)//暂只支持1
			{
				if(-1 == current_alarm_query(ctx))//当前发生的告警和屏蔽状态查询
				{
					ret = -1;
				}
				else
				{
					ret = 0;
				}
			}
			else
			{
				ret = -1;
			}
		}
		else if(0 == memcmp(argv[1], UPLOAD, strlen(UPLOAD)))
		{
			if(atoi(argv[2]) == 1)//暂只支持1
			{
				if(-1 == upload_alarm_query(ctx))//告警上报
				{
					ret = -1;
				}
				else
				{
					ret = 0;
				}
			}
			else
			{
				ret = -1;
			}
		}
		else if( (0 == memcmp(argv[1], HISTORY, strlen(HISTORY))) &&
				(page_validity(argv[2])) )
		{
			tmp = history_alarm_pages(ctx->pDb, TBL_ALARM_HISTORY);
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				if(atoi(argv[2]) <= tmp)
				{
					if(-1 == history_alarm_query(ctx, TBL_ALARM_HISTORY, atoi(argv[2])))//告警记录查询
					{
						ret = -1;
					}
					else
					{
						ret = 0;
					}
				}
				else
				{
					ret = -1;
				}
			}
		}
		else
		{
			ret = -1;
		}
	}
	else if(4 == argc)
	{
		if( (0 == memcmp(argv[1], MASK, strlen(MASK))) &&
			(alarm_id_validity(argv[2])) &&
			(mask_sta_validity(argv[3])) )
		{
			if(-1 == alarm_mask_set(ctx, atoi(argv[2]), atoi(argv[3])))//告警掩码设置
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else if( (0 == memcmp(argv[1], SELECT, strlen(SELECT))) &&
				(almport_validity(argv[2])) &&
				(alarm_id_validity(argv[3])) )
		{
			if(-1 == alarm_select_set(ctx, atoi(argv[2]), atoi(argv[3])))//告警输出板输出设置
			{
				ret = -1;
			}
			else
			{
				ret = 0;
				printf("Success.\n");
			}
		}
		else
		{
			ret = -1;
		}
	}
	else
	{
		ret = -1;
	}

#ifdef NEW_ALARMID_COMM
	cleanAlarmIndexTable();
exit3:
	FpgaClose(ctx->fpga_fd);
exit2:
	//clean
	cleanDatabase(ctx);
exit1:
	SHM_DETACH(ctx->ipc.ipc_base);
#endif
exit0:
	if(-1 == ret)
	{
		printf("Failure.\n");
		return -1;
	}
	else
	{
		return 0;
	}
}


/*
  1	合法
  0	非法
*/
int almport_validity(char *port)
{
	if(1 == strlen(port))
	{
		if((port[0] >= '1') && (port[0] <= '8'))
		{
			return 1;
		}
	}
	
	return 0;
}





/*
  1	合法
  0	非法
*/
int alarm_id_validity(char *alarm_id)
{
	int ret = 1;//true
	if(atoi(alarm_id) < 0 || atoi(alarm_id) >= ALM_CURSOR_ARRAY_SIZE){
		ret = 0;
	}

	return ret;
}





/*
  1	合法
  0	非法
*/
int mask_sta_validity(char *mask_sta)
{
	if(1 == strlen(mask_sta))
	{
		if((mask_sta[0] >= '0') && (mask_sta[0] <= '1'))
		{
			return 1;
		}
	}
	
	return 0;
}





/*
  1	合法
  0	非法
*/
int page_validity(char *page)
{
	int len;
	int ret = 0;

	len = strlen(page);
#if 0
	if(1 == len)
	{
		if((page[0] >= '1') && (page[0] <= '9'))
		{
			ret = 1;
		}
	}
	else if(2 == len)
	{
		if( ((page[0] >= '1') && (page[0] <= '9')) &&
			((page[1] >= '0') && (page[1] <= '9')) &&
			((atoi(page) >= 1) && (atoi(page) <= MAX_ALM/RECORD_PER_PAGE)) )
		{
			ret = 1;
		}
	}
	else
	{
		//do nothing
	}
#else
	int i = 0;
	ret = 1;
	for (i = 0; i < len; i++){
		if(isdigit(page[i]) == 0)
			ret = 0;
	}
	int record_num = MAX_ALM;
	int max_page = 0;
	if(record_num % RECORD_PER_PAGE == 0)
		max_page = record_num/RECORD_PER_PAGE;
	else
		max_page = record_num/RECORD_PER_PAGE +1;
	if( atoi(page) < 1 || atoi(page) > max_page){
		ret = 0;
	}
#endif
	return ret;
}

/*
  1	合法
  0	非法
*/
int threshold_validity(char *threshold)
{
	int len;
	int ret = 0;

	len = strlen(threshold);
	int i = 0;
	ret = 1;
	for (i = 0; i < len; i++){
		if(isdigit(threshold[i]) == 0)
			ret = 0;
	}
	if(atoi(threshold) > THRESHOLD_PHVALUE_MAX){
		ret = 0;
	}
	return ret;
}



void helper(void)
{
	int i = 0;

	for (i = 0; gHelpTbl[i]; i++){
		printf("%s\n", gHelpTbl[i]);
	}

	printf("\n");
	for(i=0; i<gAlmIdTbl_len; i++)
	{
		printf("             0x%08x | %s\n", gAlmIdTbl[i].alm_id, gAlmIdTbl[i].alm_msg);
	}

}





