#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/time.h>


#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_sqlite.h"
#include "lib_fpga.h"
#include "dwg_global.h"
#include "dwg_server.h"
#include "dwg_alloc.h"
#include "dwg_parser.h"


static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{ 
		gWgCtx.loop_flag = false;
	}
}

static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		gWgCtx.notify_flag = true;
	}
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
				gDaemonTbl[DAEMON_CURSOR_WG], 
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
					gDaemonTbl[DAEMON_CURSOR_WG], 
					"Can't get priority.");

			return -1;
		}
	}
}

/*
  -1	失败
   0	成功
*/
int initializeNotifySignal(void)
{
	//installs new signal handler for specific signal.
	if ( SIG_ERR == signal( NOTIFY_SIGNAL, notifyHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Can't install signal handler for SIGURG!");
		return(-1);
	}

	return 0;
}



int initializeFpga(struct wgCtx *ctx)
{
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open FPGA device.", 
				gDaemonTbl[DAEMON_CURSOR_WG]);

		return -1;
	}

	return 0;
}

/*
   0	成功
  -1	失败
*/
int cleanFpga(struct wgCtx *ctx)
{
	if(-1 != ctx->fpga_fd)
	{
		if(!FpgaClose(ctx->fpga_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close FPGA device.", 
					gDaemonTbl[DAEMON_CURSOR_WG] );

			return -1;
		}
	}

	return 0;
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
				gDaemonTbl[DAEMON_CURSOR_WG], 
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
		print(DBG_INFORMATIONAL, "p350_wg: using veth %s\n", MAC_PATH);
	} else if(access(MAC_ORGIN_PATH,F_OK) == 0){
		ret = init_database(MAC_ORGIN_PATH, pDb);
		print(DBG_INFORMATIONAL, "p350_wg: using veth %s\n", MAC_ORGIN_PATH);
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
			printf("p350_wg: using p350-veth %s\n", MAC_ORGIN_PATH);
		#endif
		print(DBG_ERROR, "p350_ntp: using veth database failure\n");
	}
	
	return ret;
}


void initializeContext(struct wgCtx *ctx)
{
	int i;

	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->ne_changflag = 0;
	ctx->pDb = NULL;
	ctx->pVethDb = NULL;
	
	ctx->sendSock = -1;
	ctx->fpga_fd = -1;
	ctx->thread_1 = -1;
	ctx->thread_2 = -1;
	
	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ctx->ipc[i].ipc_msgq_id = -1;
		ctx->ipc[i].ipc_sem_id = -1;
		ctx->ipc[i].ipc_shm_id = -1;
		ctx->ipc[i].ipc_base = (void *)-1;
	}
	
	ctx->old_AlmState = 0;
	ctx->new_AlmState = 0;
	ctx->old_clkSta = 0;
	ctx->new_clkSta = 0;
	ctx->pid = -1;
	ctx->rec_pid = -1;
	memset(ctx->ip, 0, sizeof(ctx->ip));
	memcpy(ctx->old_TimeSource,"NONE",5);
	memcpy(ctx->new_TimeSource,"NONE",5);
}


/*
  -1	失败
   0	成功
*/
int initializeExitSignal(void)
{
	//installs new signal handler for specific signal.
	if ( SIG_ERR == signal( EXIT_SIGNAL1, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Can't install signal handler for SIGINT!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Can't install signal handler for SIGTERM!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Can't install signal handler for SIGHUP!");
		return(-1);
	}

	return 0;
}



/*
  -1	失败
   0	成功
*/
int initializeShareMemory(struct ipcinfo *ipc)
{
	int id = -1;
	void *base = (void *)-1;
	
	//open share memory(input)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_INPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(input).");
		return -1;
	}
	base = shm_attach(id);// ����key�Ĺ����ڴ�
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(input).");
		return -1;
	}

	ipc[IPC_CURSOR_INPUT].ipc_shm_id = id;
	ipc[IPC_CURSOR_INPUT].ipc_base = base;

	//open share memory(output)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_OUTPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(output).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(output).");
		return -1;
	}

	ipc[IPC_CURSOR_OUTPUT].ipc_shm_id = id;
	ipc[IPC_CURSOR_OUTPUT].ipc_base = base;

	//open share memory(ntp)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(ntp).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(ntp).");
		return -1;
	}

	ipc[IPC_CURSOR_NTP].ipc_shm_id = id;
	ipc[IPC_CURSOR_NTP].ipc_base = base;

	//open share memory(alarm)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_ALARM);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(alarm).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(alarm).");
		return -1;
	}

	ipc[IPC_CURSOR_ALARM].ipc_shm_id = id;
	ipc[IPC_CURSOR_ALARM].ipc_base = base;

	//open share memory(clock)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_CLOCK);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(clock).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(clock).");
		return -1;
	}

	ipc[IPC_CURSOR_CLOCK].ipc_shm_id = id;
	ipc[IPC_CURSOR_CLOCK].ipc_base = base;

	//open share memory(mgr)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_MANAGER);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(mgr).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(mgr).");
		return -1;
	}

	ipc[IPC_CURSOR_MANAGER].ipc_shm_id = id;
	ipc[IPC_CURSOR_MANAGER].ipc_base = base;

	//open share memory(keylcd)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_KEYLCD);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(keylcd).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(keylcd).");
		return -1;
	}

	ipc[IPC_CURSOR_KEYLCD].ipc_shm_id = id;
	ipc[IPC_CURSOR_KEYLCD].ipc_base = base;
	//open share memory(WG)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_WG);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open share memory(wg).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to attach share memory(wg).");
		return -1;
	}

	ipc[IPC_CURSOR_WG].ipc_shm_id = id;
	ipc[IPC_CURSOR_WG].ipc_base = base;

	return 0;
}

/*
  -1	失败
   0	成功
*/
int initializeMessageQueue(struct ipcinfo *ipc)
{
	int id = -1;

	//open message queue(alarm)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_WG);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open message queue(alarm).");
		return -1;
	}
	ipc[IPC_CURSOR_WG].ipc_msgq_id = id;

	return 0;
}

/*
  -1	失败
   0	成功
*/
int initializeSemaphoreSet(struct ipcinfo *ipc)
{
	int id = -1;
	int i;
	
	//open semaphore set
	//---------------------------------------------------------------
	id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to open semaphore set.");
		return -1;
	}
	
	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ipc[i].ipc_sem_id = id;
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

	pid.p_cursor = DAEMON_CURSOR_WG;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc[IPC_CURSOR_WG].ipc_sem_id, SEMA_MEMBER_WG))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to lock semaphore.");
		return -1;
	}
	shm_write(ipc[IPC_CURSOR_WG].ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc[IPC_CURSOR_WG].ipc_sem_id, SEMA_MEMBER_WG))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to unlock semaphore.");
		return -1;
	}
	
	return 0;
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
					gDaemonTbl[DAEMON_CURSOR_WG], 
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

int sendne(struct wgCtx *ctx)
{
	char NeMessag[20];
	memset(NeMessag,0,sizeof(NeMessag));
	sprintf(NeMessag,"%d|%d|%s",ctx->new_AlmState,ctx->new_clkSta,ctx->new_TimeSource);
	if(0 == sendmessage(NeMessag,ctx , "ne"))//获取设备运行状�?
	{
		/*
		print(DBG_EMERGENCY, 
			  "<%s>--%s", 
			  gDaemonTbl[DAEMON_CURSOR_WG], 
			  "loss connection.");
		*/
	}
	return 1;
}

static void sendHandler(int sig)
{    
	if( SIGALRM == sig )
	{
		if(0 == sendne(&gWgCtx))
		{
			gWgCtx.loop_flag = false;
		}
	}
}

/*
  false		失败
  true		成功
*/
bool_t send_ne_timer(struct wgCtx *ctx)
{
	struct itimerval timer;
	
	if( SIG_ERR == signal(SIGALRM, SIG_IGN) )
	{
		return false;
	}

	if( SIG_ERR == signal(SIGALRM, sendHandler) )//定时发送网元信�?
	{
		return false;
	}

	timer.it_value.tv_sec = 300;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 300;
	timer.it_interval.tv_usec = 0;
	
	if( -1 == setitimer(ITIMER_REAL, &timer, 0) )//实现精度较高的定时功能，ITIMER_REAL: 以系统真实的时间来计算，它送出SIGALRM信号
	{
		return false;
	}
	
	return true;
}

int ReadipFromTable(struct wgCtx *ctx, char *pTbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
//	int i,j;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select v_ip from %s where v_port=%d;", pTbl,21);
	
	db_lock(ctx->pVethDb);
	ret1 = db_get_table(ctx->pVethDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pVethDb);

		return 0;
	}
	
	memcpy(ctx->ip, resultp[1], strlen(resultp[1]));
	db_free_table(resultp);
	db_unlock(ctx->pVethDb);
	return 1;
}


int write_virtual_eth(struct wgCtx *ctx, char *tbl_sys,struct veth *netp)
{
	char sql[512];
	bool_t ret1;
	int port = netp->v_port;//21
	memset(sql, 0, sizeof(sql));
	if (netp->v_ip[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_ip=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl_sys, 
			 netp->v_ip,
			 port );
	}
	if (netp->v_mask[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_mask=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl_sys,  
			 netp->v_mask,
			 port );
	}
	if(netp->v_gateway[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_gateway=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl_sys,
			 netp->v_gateway,
			 port );
	}
	if(netp->v_mac[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_mac=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl_sys, 
			 netp->v_mac,
			 port );
	}

	//printf("%s\n", sql);return 0;

	if(sql[0] == '\0')
		return -1;
	db_lock(ctx->pVethDb);
	ret1 = db_put_table(ctx->pVethDb, sql);
	db_unlock(ctx->pVethDb);
	if(false == ret1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


