#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_sqlite.h"
#include "lib_fpga.h"
#include "dntp_global.h"
#include "dntp_server.h"
#include "dntp_alloc.h"
#include "dntp_socket.h"







static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{ 
		gNtpCtx.loop_flag = false;
	}
}







static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		gNtpCtx.notify_flag = true;
	}
}







static void broadcastHandler(int sig)
{    
	int i = 0;
    if  (( SIGALRM == sig ))
    {
        for(i = 0; i < sizeof(port)/sizeof(struct port_info); ++i)
        {
            if(port[i].sock > 0)
            {
				if(!VaBroadcast(&port[i]))
				{
					gNtpCtx.loop_flag = false;
				}
            }
        }
    }

}









/*
  false		失败
  true		成功
*/
bool_t broadcast_enable(struct ntpCtx *ctx)
{
	struct itimerval timer;

	if( SIG_ERR == signal(SIGALRM, broadcastHandler) )//定时广播
	{
		print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Can't install signal broadcastHandler for SIGURG!");
		return false;
	}
	
	timer.it_value.tv_sec = 5;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = ctx->ntpSta.bCast.bcInterval;
	timer.it_interval.tv_usec = 0;
	if(-1 == setitimer(ITIMER_REAL, NULL, NULL))
    {
        return false;
    }
	if( -1 == setitimer(ITIMER_REAL, &timer, NULL) )//实现精度较高的定时功能，ITIMER_REAL: 以系统真实的时间来计算，它送出SIGALRM信号
	{
		return false;
	}

	return true;
}









/*
  false		失败
  true		成功
*/
bool_t broadcast_disable(void)
{
	if( -1 == setitimer(ITIMER_REAL, NULL, NULL) )
	{
		return false;
	}
	
	if( SIG_ERR == signal(SIGALRM, SIG_IGN) )
	{
		return false;
	}

	return true;
}









/*
  -1	失败
   0	成功
*/
int initializeSignal(void)
{
	//installs new signal handler for specific signal.
	if ( SIG_ERR == signal( EXIT_SIGNAL1, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Can't install signal exitHandler for SIGINT!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Can't install signal exitHandler for SIGTERM!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Can't install signal exitHandler for SIGHUP!");
		return(-1);
	}

	
	if ( SIG_ERR == signal( NOTIFY_SIGNAL, notifyHandler ) )
	{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Can't install signal notifyHandler for SIGURG!");
			return(-1);
	}

	
	return 0;
}





/*
  -1	失败
   0	成功
*/
int initializePriority(int prio)
{
	pid_t pid;
	
	//set priority
	pid = getpid();
	if ( 0 != setpriority(PRIO_PROCESS, pid, prio) )
	{			
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Can't set priority.");
		return(-1);
	}
	else
	{
		errno = 0;
		prio = getpriority(PRIO_PROCESS, pid);
		if ( 0 == errno )
		{
			print(DBG_INFORMATIONAL, "\n--Priority is %d\n", 
					prio);
			return 0;
		}
		else
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Can't get priority.");
			return -1;
		}
	}
}






static int init_database(char * path, sqlite3 **pDb)
{
	if(db_open(path, pDb))
	{
		return 1;
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--%s %s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Can't open database", path);
		return 0;
	}
}
/*
   1	成功
   0	失败
*/
int initializeDatabase(sqlite3 **pDb)
{
	return init_database(DB_PATH, pDb);
}

/*
   1	成功
   0	失败
*/
int initializeVethDatabase(sqlite3 **pDb)
{
	
	int ret = 0;
	if(access(MAC_PATH,F_OK) == 0){
		ret = init_database(MAC_PATH, pDb);
		print(DBG_INFORMATIONAL, "p350_ntp: using veth %s\n", MAC_PATH);
	} else if(access(MAC_ORGIN_PATH,F_OK) == 0){
		ret = init_database(MAC_ORGIN_PATH, pDb);
		print(DBG_INFORMATIONAL, "p350_ntp: using veth %s\n", MAC_ORGIN_PATH);
	} else {
		#if 0
		unsigned char * cmd[128];
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "mkdir -p %s",USER_DATA_MAC_DIR);
		system(cmd);
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "cp -r %s %s", DB_PATH, MAC_ORGIN_PATH);
		system(cmd);
		sync();
		sync();
		ret = init_database(MAC_ORGIN_PATH, pDb);
		if (ret == 1)
			printf("p350_ntp: using p350-veth %s\n", MAC_ORGIN_PATH);
		#endif
		print(DBG_ERROR, "p350_ntp: using veth database failure\n");
	}

	return ret;
}








/*
  -1	失败
   0	成功
*/
int initializeShareMemory(struct ipcinfo *ipc)
{
	int id = -1;
	void *base = (void *)-1;

	//open share memory(ntp)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to open share memory(ntp).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to attach share memory(ntp).");
		return -1;
	}

	ipc->ipc_shm_id = id;
	ipc->ipc_base = base;

	return 0;
}








/*
  -1	失败
   0	成功
*/
int initializeMessageQueue(struct ipcinfo *ipc)
{
	int id = -1;

	//open message queue(ntp)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to open message queue(ntp).");
		return -1;
	}
	ipc->ipc_msgq_id = id;

	return 0;
}








/*
  -1	失败
   0	成功
*/
int initializeSemaphoreSet(struct ipcinfo *ipc)
{
	int id = -1;
	
	//open semaphore set
	id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to open semaphore set.");
		return -1;
	}

	ipc->ipc_sem_id = id;

	return 0;
}


/*
  -1	失败
   0	成功
*/

int initializeFpga(struct ntpCtx *ctx)
{
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open FPGA device.", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);

		return -1;
	}

	return 0;
}

/*
   0	成功
  -1	失败
*/
int cleanFpga(struct ntpCtx *ctx)
{
	if(-1 != ctx->fpga_fd)
	{
		if(!FpgaClose(ctx->fpga_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close FPGA device.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );

			return -1;
		}
	}

	return 0;
}


void cleanSock()
{
    int i;
	for(i = 0; i < ARRAY_SIZE(port); i++)
	{
		close(port[i].sock);
	}
}

void initializeContext(struct ntpCtx *ctx)
{
	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->pDb = NULL;
	ctx->pVethDb = NULL;
	
	ctx->ntpSock = -1;
	ctx->fpga_fd = -1;

    ctx->thread_id = -1;

	ctx->ipc.ipc_msgq_id = -1;
	ctx->ipc.ipc_sem_id = -1;
	ctx->ipc.ipc_shm_id = -1;
	ctx->ipc.ipc_base = (void *)-1;
	memset(&ctx->pool, 0, sizeof(struct threadpool));
	memset(&ctx->alarm, ALM_OFF, sizeof(struct alarmsta));
	memset(&ctx->ntpSta, 0, sizeof(struct ntpsta));
}







/*
   1	成功
   0	失败
*/
int cleanDatabase(sqlite3 *pDb)
{
	if(NULL != pDb)
	{
		if(db_close(pDb))
		{
			return 1;
		}
		else
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Can't close database.");
			return 0;
		}
	}

	return 1;
}

int cleanVethDatabase(sqlite3 *pDb)
{
	return cleanDatabase(pDb);
}







/*
  -1	失败
   0	成功
*/
int cleanShareMemory(struct ipcinfo *ipc)
{
	if((void *)-1 != ipc->ipc_base)
	{
		if( 1 == shm_detach(ipc->ipc_base) )
		{
			ipc->ipc_base = (void *)-1;
			if(-1 != ipc->ipc_shm_id)
			{
				if( 0 == shm_removable(ipc->ipc_shm_id) )
				{
					print(	DBG_ERROR, 
							"<%s>--%s", 
							gDaemonTbl[DAEMON_CURSOR_NTP], 
							"Failed to remove share memory.");
					return -1;
				}
				ipc->ipc_shm_id = -1;
			}
		}
		else
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_NTP], 
					"Failed to detach share memory.");
			return -1;
		}
	}

	return 0;
}








/*
  -1	失败
   0	成功
*/
int writePid(struct ipcinfo *ipc)
{
	struct pidinfo pid;

	pid.p_cursor = DAEMON_CURSOR_NTP;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to lock semaphore.");
		return -1;
	}
	shm_write(ipc->ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_NTP))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to unlock semaphore.");
		return -1;
	}
	
	return 0;
}





