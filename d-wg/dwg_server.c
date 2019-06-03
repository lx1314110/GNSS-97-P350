#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lib_dbg.h"
#include "lib_net.h"
#include "alloc.h"

#include "dwg_global.h"
#include "dwg_alloc.h"
#include "dwg_parser.h"
#include "dwg_server.h"
#include "common.h"

bool_t initializeSocket(struct wgCtx *ctx, char *ip)
{
	struct timeval tv;
	int so_reuseaddr = 1;
	memset( &(ctx->sa_send), 0, sizeof(struct sockaddr_in) );
	ctx->sa_send.sin_family = AF_INET;
	ctx->sa_send.sin_port = htons(WG_SEND_PORT);
	ctx->sa_send.sin_addr.s_addr = inet_addr(ip);
	
	ctx->sendSock = socket(AF_INET, SOCK_DGRAM, 0);
	if( ctx->sendSock < 0 )
	{
		return false;
	}

	if( 0 != setsockopt( ctx->sendSock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(int) ) )//允许地址重用
	{
		close(ctx->sendSock);
		return false;
	}
	
	memset( &tv, 0, sizeof(struct timeval) );
	tv.tv_sec = 0;
	tv.tv_usec = 20000;
	
	if( 0 != setsockopt( ctx->sendSock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval) ) )//设置发送超时
	{
		close(ctx->sendSock);
		return false;
	}
	
	if( 0 != setsockopt( ctx->sendSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval) ) )//设置接收超时
	{
		close(ctx->sendSock);
		return false;
	}
	
	return true;
}

static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) ){
		gWgCtx.loop_flag = false;
		print(DBG_NOTICE, "received signo:%d", signum);
	}
}


int ProcWg(struct wgCtx *ctx)
{
	int ret = 0;
	char NeMessag[20];
	char wg_ip[16];
	
	initializeContext(ctx);//初始化环境
	initializeBoardNameTable();	//初始化单盘名称索引
	set_print_level(true, DBG_INFORMATIONAL, DAEMON_CURSOR_WG);
	syslog_init(NULL);
	Init_buf();

	if(-1 == initializePriority(DAEMON_PRIO_WG))
	{
		ret = __LINE__;
		goto exit_0;
	}

	if(-1 == initializeCommExitSignal(&exitHandler))
	{
		ret = __LINE__;
		goto exit0;
	}

	if(-1 == initializeEventIndexTable())
	{
		ret = __LINE__;
		goto exit_0;
	}
	if(0 == initializeDatabase(&ctx->pDb))//初始化数据库
	{
		ret = __LINE__;
		goto exit0;
	}

	if(0 == initializeVethDatabase(&ctx->pVethDb)) //init mac ip veth database
	{
		ret = __LINE__;
		goto exit1;
	}

	if(-1 == initializeExitSignal())//初始化退出信号
	{
		ret = __LINE__;
		goto exit2;
	}
	
	if(-1 == initializeNotifySignal())//初始化自定义信号
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeShareMemory(ctx->ipc))//初始化共享内存
	{
		ret = __LINE__;
		goto exit2;
	}
	
	if(-1 == initializeMessageQueue(ctx->ipc))//初始化消息队列
	{
		ret = __LINE__;
		goto exit2;
	}
	
	if(-1 == initializeSemaphoreSet(ctx->ipc))//初始化信号量
	{
		ret = __LINE__;
		goto exit2;
	}
	
	
	
	if(-1 == writePid(ctx->ipc))//把ipc里的pid拷贝到共享内存
	{
		ret = __LINE__;
		goto exit2;
	}
	
	if(-1 == initializeFpga(ctx))//打开FPGA
	{
		ret = __LINE__;
		goto exit2;
	}

	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit3;
	}

	if(0 == ReadWgIp(ctx,TBL_SYS,wg_ip))//打开FPGA
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if( !initializeSocket(ctx,wg_ip) )//初始化套接字
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if(-1 == ReadipFromTable(ctx,TBL_VETH))
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(0 == CreateThread(ctx))
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if( !send_ne_timer(ctx) )//每5分钟发一次
	{
		ret = __LINE__;
		goto exit;
	}
	
	//memcpy(ctx->ip,"192.168.22.22",sizeof(ctx->ip));
	
	while(isRunning(ctx))
	{
		SysWaitFpgaRunStatusOk(ctx->fpga_fd);
		INIT_EVNTFLAG(0);
		if(isNotify(ctx))//收到配置命令
		{
			if(0 == ReadMessageQueue(ctx))//读消息并保存对应信息
			{
				ret = __LINE__;
				print(DBG_ERROR, 
				  	  "<%s>--%s", 
				  	  gDaemonTbl[DAEMON_CURSOR_WG], 
				  	  "ReadMessageQueue Error.");
				break;
			}
		}
		/*
		if( 0 == info_ip(ctx->pDb, TBL_VETH, ctx->ip) )
		{
			ret = -1;
			print(DBG_EMERGENCY, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_WG], 
				  "__Sock_GetIp err.");
			break;
		}
		*/
		if(0 == ReadAlmState(ctx))//获取告警状态
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_WG], 
				  "ReadAlmState Error.");
			break;
		}	
		
		if(0 == ReadClockState(ctx))//获取设备运行状态
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_WG], 
				  "ReadClockstate Error.");
			break;
		}
		
		if(0 == ReadTimeSource(ctx))//获取设备时间源
		{
			ret = __LINE__;
			print(DBG_ERROR, 
				  "<%s>--%s", 
				  gDaemonTbl[DAEMON_CURSOR_WG], 
				  "ReadTimesource Error.");
			break;
		}
		
		
		if(ctx->ne_changflag)
		{
			memset(NeMessag,0,sizeof(NeMessag));
			sprintf(NeMessag,"%d|%d|%s",ctx->new_AlmState,ctx->new_clkSta,ctx->new_TimeSource);
			//printf("%s\n",NeMessag);
			
			if(0 == sendmessage(NeMessag,ctx , "ne"))//获取设备运行状态
			{
				/*
				print(DBG_EMERGENCY, 
					  "<%s>--%s", 
					  gDaemonTbl[DAEMON_CURSOR_WG], 
					  "loss connection.");
					  */
			}
			
		}
	
		if(0 == WriteCurEventToTable(ctx->pDb, pEvntInxTbl, max_evntinx_num))//把事件写入到事件表
		{
			ret = __LINE__;
			break;
		}
		ReNew_value(ctx);
		usleep(200000);
	}

exit:
	CloseThread(ctx);
exit4:
	close(ctx->sendSock);
exit3:
	cleanFpga(ctx);
exit2:
	cleanVethDatabase(ctx->pVethDb);
exit1:
	cleanDatabase(ctx->pDb);
exit0:
	cleanEventIndexTable();
exit_0:
	Clean_buf();

	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();
	
	return ret;
}

