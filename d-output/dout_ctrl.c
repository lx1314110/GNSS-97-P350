#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_msgQ.h"
#include "lib_int.h"
#include "lib_time.h"
#include "lib_common.h"

#include "common.h"
#include "addr.h"
#include "dout_alloc.h"
#include "dout_time.h"
#include "dout_parser.h"
#include "dout_config.h"
#include "dout_issue.h"
#include "dout_ctrl.h"

//#define OUT_FIFO_89

/*
  true:		程序运行
  false:	程序退出
*/
bool_t isRunning(struct outCtx *ctx)
{
	return ctx->loop_flag;
}


/*
  1	成功
  0	失败
*/
/*
int FreshTime(int fpga_fd,int leapSecond)
{
	
	if(!FpgaWrite(fpga_fd, FPGA_LPS, leapSecond))//闰秒刷新
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S01_FRESH_TIME, 0x0000))//1号槽时间刷新
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S01_FRESH_TIME, 0x0001))
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S02_FRESH_TIME, 0x0000))//2号槽时间刷新
	{
		return 0;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S02_FRESH_TIME, 0x0001))
	{
		return 0;
	}
	return 1;
}
*/


void CopyBid(struct outCtx *ctx)
{
	int i; 

	for(i=SLOT_CURSOR_1; i<SLOT_CURSOR_ARRAY_SIZE; i++)
	{
		ctx->oldBid[i] = ctx->outSta[i].boardId;
	}
}






/*
  1	成功
  0	失败
*/
int BoardPushPull(struct outCtx *ctx)
{
	int i;

	for(i=SLOT_CURSOR_1; i<SLOT_CURSOR_ARRAY_SIZE; i++)
	{
		if(((ctx->outSta[i].boardId >= BID_OUT_LOWER) && (ctx->outSta[i].boardId <= BID_OUT_UPPER))||
		   (BID_NONE == ctx->outSta[i].boardId))
		{
			//无板卡--->有板卡
			if((BID_NONE == ctx->oldBid[i]) && (BID_NONE != ctx->outSta[i].boardId))
			{
				//拷贝默认配置
				if(0 == DeleteConfig(ctx, TBL_IO_RUNTIME, i+1))
				{
					return 0;
				}
				if(0 == CopyConfig(ctx, TBL_IO_DEFAULT, TBL_IO_RUNTIME, i+1, ctx->outSta[i].boardId, 1))
				{
					return 0;
				}
				//NTP、NTPF板卡不需要下发配置
				//if((BID_NTP != ctx->outSta[i].boardId) && (BID_NTPF != ctx->outSta[i].boardId))
				//{
					if(0 == ReadConfigFromRunTimeTable(ctx, TBL_IO_RUNTIME, i+1, ctx->outSta[i].boardId, 1))
					{
						return 0;
					}
					if(0 == WriteConfigToShareMemory(ctx, i+1))
					{
						return 0;
					}

					if(0 == IssueConfig(ctx, i+1, ctx->outSta[i].boardId))
					{
						return 0;
					}

					/*仅仅是为了把issueConfig产生的old值保存，比如oldptpSlaveIp*/
					if(0 == WriteConfigToShareMemory(ctx, i+1))
					{
						return 0;
					}
				//}
			}
			//有板卡--->无板卡
			else if((BID_NONE != ctx->oldBid[i]) && (BID_NONE == ctx->outSta[i].boardId))
			{
				//清空当前槽位配置
				if(0 == EmptyConfig(ctx, TBL_IO_RUNTIME, i+1))
				{
					return 0;
				}
				//may no need
				if(0 == WriteConfigToShareMemory(ctx, i+1))
				{
					return 0;
				}
			}
			//有板卡--->有板卡
			else if((BID_NONE != ctx->oldBid[i]) && (BID_NONE != ctx->outSta[i].boardId))
			{
				if(ctx->oldBid[i] != ctx->outSta[i].boardId)
				{

					//拷贝默认配置
					if(0 == DeleteConfig(ctx, TBL_IO_RUNTIME, i+1))
					{
						return 0;
					}

					if(0 == CopyConfig(ctx, TBL_IO_DEFAULT, TBL_IO_RUNTIME, i+1, ctx->outSta[i].boardId, 1))
					{
						return 0;
					}

					//NTP、NTPF板卡不需要下发配置
					//if((BID_NTP != ctx->outSta[i].boardId) && (BID_NTPF != ctx->outSta[i].boardId))
					//{
						if(0 == ReadConfigFromRunTimeTable(ctx, TBL_IO_RUNTIME, i+1, ctx->outSta[i].boardId, 1))
						{
							return 0;
						}
						if(0 == WriteConfigToShareMemory(ctx, i+1))
						{
							return 0;
						}

						if(0 == IssueConfig(ctx, i+1, ctx->outSta[i].boardId))
						{
								return 0;
						}

						/*仅仅是为了把issueConfig产生的old值保存，比如oldptpSlaveIp*/
						if(0 == WriteConfigToShareMemory(ctx, i+1))
						{
							return 0;
						}
					//}
				}
				else
				{

					//设备上电后，如果当前的板卡类型与掉电前的板卡类型相等，则需要读取掉电前的配置，并下发配置。
					print(	DBG_DEBUG, 
							"<%s>--Same board identifier.", 
					 		gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
					
					if(ctx->power_flag)
					{
						//NTP、NTPF板卡不需要下发配置
						//if((BID_NTP != ctx->outSta[i].boardId) && (BID_NTPF != ctx->outSta[i].boardId))
						//{
							if(0 == ReadConfigFromRunTimeTable(ctx, TBL_IO_RUNTIME, i+1, ctx->outSta[i].boardId, 1))
							{
								return 0;
							}
							if(0 == WriteConfigToShareMemory(ctx, i+1))
							{
								return 0;
							}

							if(0 == IssueConfig(ctx, i+1, ctx->outSta[i].boardId))
							{
									return 0;
							}

							/*仅仅是为了把issueConfig产生的old值保存，比如oldptpSlaveIp*/
							if(0 == WriteConfigToShareMemory(ctx, i+1))
							{
								return 0;
							}
						//}
					}
				}
			}
			//无板卡--->无板卡
			else
			{
				//do nothing
			}
		}
	}

	print(	DBG_DEBUG, 
			"<%s>--Copy board identifier.", 
			gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
	
	CopyBid(ctx);
	//清除设备上电后设置的上电标志。
	if(ctx->power_flag)
	{
		ctx->power_flag = false;
	}
	
	return 1;
}




int set_pro_fifo(void)
{
	struct sched_param param;
	int maxpri;

	maxpri = sched_get_priority_max(SCHED_FIFO);

	if (maxpri == -1)
	{
		return -1;
	}
#ifdef OUT_FIFO_89
	param.sched_priority = maxpri-10;
	print(DBG_INFORMATIONAL, "p350_out rtpriority is %d \n",maxpri-10);
#else
	param.sched_priority = maxpri-1;
	print(DBG_INFORMATIONAL, "p350_out rtpriority is %d \n",maxpri-1);
#endif
	if (sched_setscheduler(getpid(),SCHED_FIFO,&param) == -1)
	{
		return -1;
	}

	return 0;
}


/*
   0	成功
  -1	失败
*/
int ProcOutput(struct outCtx *ctx)
{
	int ret = 0;
	
	initializeContext(ctx);//初始化数据
	initializeBoardNameTable();	//初始化单盘名称索引
	set_print_level(true, DBG_INFORMATIONAL,DAEMON_CURSOR_OUTPUT);
	syslog_init(NULL);

	if(-1 == initializeDatabase(ctx))//初始化数据库
	{
		ret = __LINE__;
		goto exit_0;
	}

	if(-1 == initializeEventIndexTable())
	{
		ret = __LINE__;
		goto exit0;
	}

#if 1

	if(-1 == initializePriority(DAEMON_PRIO_OUTPUT))//初始化当前进程的优先级
	{
		ret = __LINE__;
		goto exit1;
	}
#else
	if (-1==set_pro_fifo())//设置当前进程优先级为最大
	{
		ret = __LINE__;
		goto exit1;
	}
#endif
	if(-1 == initializeExitSignal())//初始化退出信号
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializeNotifySignal())//初始化自定义信号
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializeShareMemory(ctx->ipc))//初始化共享内存
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializeMessageQueue(ctx->ipc))//初始化消息队列
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializeSemaphoreSet(ctx->ipc))//初始化信号量
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == writePid(ctx->ipc))//把ipc里的pid拷贝到共享内存
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializeFpga(ctx))//打开FPGA
	{
		ret = __LINE__;
		goto exit2;
	}

	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeInt(ctx))//打开pps 1号interrupt
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if(0 == ReadOutTimezone(ctx, TBL_SYS))//read output timezone from system table
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == IssueIrigbZone(ctx))//Irigb表示输出信号类型，向Irigb中发送时区
		{
			ret = __LINE__;
			goto exit3;
		}

		if(0 == WriteOutZoneToShareMemory(ctx))//向共享内存写入输出时区
		{
			ret = __LINE__;
			goto exit3;
		}
	}
	
	if(0 == ReadInTimezone(ctx, TBL_SYS))//从数据库中读输入时区
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == WriteInZoneToShareMemory(ctx))//向共享内存写入输入时区
		{
			ret = __LINE__;
			goto exit3;
		}
	}
	
	if(0 == ReadLeapsecond(ctx, TBL_SYS))//从数据库中读闰秒
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == WriteLeapToShareMemory(ctx))//向共享内存写入闰秒
		{
			ret = __LINE__;
			goto exit3;
		}
	}
	
	if(0 == ReadDelay(ctx, TBL_SYS))//从数据库中读输出信号延时补偿
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == IssueDelay(ctx))//发送延时补偿
		{
			ret = __LINE__;
			goto exit3;
		}
		if(0 == WriteDelayToShareMemory(ctx))//向共享内存写入延时补偿
		{
			ret = __LINE__;
			goto exit3;
		}
	}

	if(0 == ReadOut2mbSA(ctx, TBL_SYS))//从数据库中读SA为2MB输出
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == IssueOut2mbSA(ctx))//发送SA到fpga为2MB输出
		{
			ret = __LINE__;
			goto exit3;
		}

		if(0 == WriteOut2mbSAToShareMemory(ctx))//向共享内存写入SA为2MB输出
		{
			ret = __LINE__;
			goto exit3;
		}
	}

	if(0 == ReadOutBaudrate(ctx, TBL_SYS))//从数据库中读输出波特率
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == IssueOutBaudrate(ctx))//发送输出波特率
		{
			ret = __LINE__;
			goto exit3;
		}
		if(0 == WriteOutBaudrateToShareMemory(ctx))//向共享内存写入输出波特率
		{
			ret = __LINE__;
			goto exit3;
		}
	}
	
	if(0 == ReadIrigbAmplitude(ctx, TBL_SYS))//从数据库中读Irigb的振幅
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == IssueIrigbAmplitude(ctx))//发送Irigb的振幅
		{
			ret = __LINE__;
			goto exit3;
		}
		if(0 == WriteIrigbAmplitudeToShareMemory(ctx))//向共享内存写入Irigb的振幅
		{
			ret = __LINE__;
			goto exit3;
		}
	}
	
	if(0 == ReadIrigbVoltage(ctx, TBL_SYS))//从数据库中读Irigb的电压
	{
		ret = __LINE__;
		goto exit3;
	}
	else
	{
		if(0 == IssueIrigbVoltage(ctx))//发送Irigb的电压
		{
			ret = __LINE__;
			goto exit3;
		}
		if(0 == WriteIrigbVoltageToShareMemory(ctx))//向共享内存写入Irigb的电压
		{
			ret = __LINE__;
			goto exit3;
		}
	}

	if(0 == IssueIrigbDST(ctx))//发送Irigb的夏令时
	{
		ret = __LINE__;
		goto exit3;
	}
	/*
	if(0 == FreshTime(ctx->fpga_fd, ctx->lp.leapSecond))
	{
		ret = -1;
		goto exit;
	}
	*/
	
	if(0 == ReadBidFromTable(ctx, TBL_IO_RUNTIME))//从数据库中读单板编号
	{
		ret = __LINE__;
		goto exit3;
	}

	if(0 == ReadBidFromFpga(ctx))//从FPGA中读单板编号
	{
		ret = __LINE__;
		goto exit3;
	}

	if(0 == BoardPushPull(ctx))//板卡配置
	{
		ret = __LINE__;
		goto exit3;
	}
	if(0 == CreateThread(ctx))
	{
		ret = __LINE__;
		goto exit4;
	}

	while(isRunning(ctx))
	{
		SysWaitFpgaRunStatusOk(ctx->fpga_fd);
		if(isNotify(ctx))//收到配置命令
		{
			if(0 == ReadMessageQueue(ctx))//读消息并保存对应信息
			{
				ret = __LINE__;
				print(DBG_ERROR,
				  	  "<%s>--%s", 
				  	  gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				  	  "ReadMessageQueue Error.");
				break;
			}
		}

		if(0 == ReadBidFromFpga(ctx))//从FPGA中读单板编号
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				  "ReadBidFromFpga Error.");
			break;
		}

		if(0 == BoardPushPull(ctx))//板卡默认配置
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				  "BoardPushPull Error.");
			break;
		}

		if(SetOutBoardClockClass(ctx) < 0)//板卡默认配置
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				  "SetOutBoardClockClass Error.");
			break;
		}

		//目前没什么告警，都是写alm_off
		if(0 == WriteAlarmToShareMemory(ctx))//向共享内存写入告警信息
		{
			ret = __LINE__;
			break;
		}
		
		usleep(200000);//(200ms)
	}

exit4:
	CloseThread(ctx);
exit3:
	cleanInt(ctx);
exit2:
	cleanFpga(ctx);
exit1:
	cleanEventIndexTable();
exit0:
	cleanDatabase(ctx);
exit_0:

	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();
	
	return ret;
}




