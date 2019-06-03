#include <stdio.h>
#include <unistd.h>

#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_ip1725.h"

#include "addr.h"

#include "dalm_alarm.h"
#include "dalm_alloc.h"
#include "dalm_parser.h"
#include "dalm_config.h"
#include "dalm_ctrl.h"
#include "common.h"


extern u32_t count_hold;





/*
  true:
  false:
*/
bool_t isRunning(struct almCtx *ctx)
{
	return ctx->loop_flag;
}

/**/
__inline__ void timer_handler(int msg)
{
    switch(msg) {
        case SIGALRM:
			count_hold++;
            break;

        default:
            break;
    }

    return;
}


/*SIGALRM*/
void timer_init(void)
{
	/*ִֻ��һ��*/
	static int timerinit = 0;
	if (timerinit == 0) {
		signal(SIGALRM, timer_handler);
		timerinit = 1;
		count_hold = 0;
	}
}





/*
   0	ok
  -1	error
*/
int ProcAlarm(struct almCtx *ctx)
{
	int ret = 0;
	int i = 0;
	unsigned int portStatus = 0;
	
	initializeContext(ctx);
	initializeBoardNameTable();	
	set_print_level(true, DBG_INFORMATIONAL, DAEMON_CURSOR_ALARM);
	syslog_init(NULL);

#ifdef NEW_ALARMID_COMM

	if(-1 == initializeEventIndexTable())
	{
		ret = __LINE__;
		goto exit_1;
	}
	if(-1 == initializeAlarmIndexTable())
	{
		ret = __LINE__;
		goto exit0;
	}
#endif

	if(-1 == initializeDatabase(ctx))
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializePriority(DAEMON_PRIO_ALARM))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeExitSignal())
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeNotifySignal())//ctx->notify_flag = ture
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeShareMemory(ctx->ipc))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeMessageQueue(ctx->ipc))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeSemaphoreSet(ctx->ipc))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == writePid(ctx->ipc))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeFpga(ctx))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit;
	}
	
#ifdef NEW_ALARMID_COMM
	if(0 == FlushAlarmMaskTable(ctx, TBL_ALARM_SHIELD))
	{
		ret = __LINE__;
		goto exit;
	}
	//no need the TBL_ALARM_LIST
#else
	if(0 == FlushAlarmListTable(ctx, TBL_ALARM_LIST))
	{
		ret = __LINE__;
		goto exit;
	}
#endif

	if(0 == ReadAlarmSelectFromTable(ctx, TBL_ALARM_SELECT))
	{
		ret = __LINE__;
		goto exit;
	}
	if(0 == WriteSelectToShareMemory(ctx))
	{
		ret = __LINE__;
		goto exit;
	}

	if(0 == ReadAlarmMaskFromTable(ctx, TBL_ALARM_SHIELD))
	{
		ret = __LINE__;
		goto exit;
	}
	if(0 == WriteMaskToShareMemory(ctx))
	{
		ret = __LINE__;
		goto exit;
	}

	if(0 == ReadSignalThresholdFromTable(ctx, TBL_SYS))
	{
		ret = __LINE__;
		goto exit;
	}
	if(0 == WriteSignalThresholdToShareMemory(ctx))
	{
		ret = __LINE__;
		goto exit;
	}

	timer_init();
	

	while(isRunning(ctx))
	{
		INIT_EVNTFLAG(0);
		//FPGA STATUS INVAID WAIT ...
		while(isRunning(ctx)){
			if(1 == SysCheckFpgaRunStatus(ctx->fpga_fd)){
				#if 0
				print(	DBG_ERROR,
						"<%s>-- fpga status ok!", who);//test
				#endif
				break;
			}
			CollectAlarm_FPGAInvalid(ctx);
			CompareAlarm(ctx);
			WriteAlarmInfoToShareMemory(ctx);//把告警写入共享内存
			print(	DBG_ERROR,
						"<%s>-- fpga status wait...", gDaemonTbl[DAEMON_CURSOR_ALARM]);//test
			sleep(3);
		}

		//STATUS OK
		if(isNotify(ctx))
		{
			if(0 == ReadMessageQueue(ctx))
			{
				ret = __LINE__;
				print(DBG_ERROR, 
				  	  "ReadMessageQueue Error.");
				break;
			}
		}	
		if(0 == ReadBidFromFpga(ctx))//从FPGA中读取单板编号
		{
			ret = __LINE__;
			print(DBG_ERROR,
				  "ReadBidFromFpga Error.");
			break;
		}

		dt_port_linkstatus_all_get(ctx->fpga_fd,&portStatus);
		
		for(i=1;i<=V_ETH_MAX;i++)
		{
			if (portStatus & (1<<i))
			{
				ctx->port_status[i-1].linkstatus = 1;
			} 
			else
			{
				ctx->port_status[i-1].linkstatus = 0;
			}
			//printf("%d ", ctx->port_status[i-1].linkstatus);
		}
		//printf("#########\n");
		
		if(0 == WriteEnableToShareMemory(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR,
				  "WriteEnableToShareMemory Error.");
			break;
		}
		//read refsource state
		if(-1 == read_fresh_state(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR,
				  "read_fresh_state Error.");
			break;
		}
		if(0 == CollectEvent(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "CollectEvent Error.");
			break;
		}
		
		if(0 == CollectAlarm(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "CollectAlarm Error.");
			break;
		}
	
		//if (ctx->satellite_usestate == GET_SATELLITE || ctx->satellite_usestate == LOSS_SATELLITE)
		if(ctx->refsrc_state.refsrc_is_valid != REFSOURCE_INIT)//选源后再发送报警到网页
		{
			if(0 == CompareAlarm(ctx))
			{
				ret = __LINE__;
				print(DBG_ERROR, 
					  "CompareAlarm Error.");
				break;
			}
		}
		
		
        //printf("1111111[%d]\n", __LINE__);
		if(!ledCtrl(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR,
				  "ledCtrl Error.");
			break;
		}

		//for alarmboard 17slot
		if(!RefreshAlarm(ctx))
		{
			ret = __LINE__;
			print(DBG_ERROR,
				  "RefreshAlarm Error.");
			break;
		}

		if(0 == WriteAlarmInfoToShareMemory(ctx))//把告警写入共享内存
		{
			ret = __LINE__;
			break;
		}

		if(0 == WriteCurEventToTable(ctx->pDb, pEvntInxTbl, max_evntinx_num))//把事件写入到事件表
		{
			ret = __LINE__;
			break;
		}
		usleep(100000);//(100ms)
	}
 
exit:
	cleanFpga(ctx);
exit2:
	cleanDatabase(ctx);

exit1:
#ifdef NEW_ALARMID_COMM
	cleanAlarmIndexTable();
exit0:
	cleanEventIndexTable();
exit_1:
#endif
	SwitchClean();//may no need it

	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);

	syslog_exit();
	
	return ret;
}






