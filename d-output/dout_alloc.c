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
#include "lib_fpga.h"
#include "lib_int.h"
#include "lib_sqlite.h"

#include "dout_alloc.h"
#include "dout_global.h"

static void exitHandler( int signum );
static void notifyHandler(int signum);

static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{
		gOutCtx.loop_flag = false;
	}
}




static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		gOutCtx.notify_flag = true;
	}
}




/*
  -1	失败
   0	成功
*/
int initializeExitSignal(void)
{
	//installs new signal handler for specific signal.
	//--------------------------------------------------------------------------------------------------------
	if ( SIG_ERR == signal( EXIT_SIGNAL1, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"Can't install signal handler for SIGINT!" );

		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"Can't install signal handler for SIGTERM!" );

		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )
	{
		print(	DBG_ERROR,
				"Can't install signal handler for SIGHUP!" );

		return(-1);
	}
	//--------------------------------------------------------------------------------------------------------

	return 0;
}





/*
  -1	失败
   0	成功
*/
int initializeNotifySignal(void)
{
	//installs new signal handler for specific signal.
	//--------------------------------------------------------------------------------------------------------
	if ( SIG_ERR == signal( NOTIFY_SIGNAL, notifyHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Can't install signal handler for SIGURG!" );

		return(-1);
	}
	//--------------------------------------------------------------------------------------------------------

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
	if ( 0 != setpriority(PRIO_PROCESS, pid, prio) )//设置进程执行优先权
	{			
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Can't set priority." );

		return(-1);
	}
	else
	{
		errno = 0;
		prio = getpriority(PRIO_PROCESS, pid);//获取进程执行优先权
		if ( 0 == errno )
		{
			print(DBG_INFORMATIONAL, "\n--Priority is %d\n", 
					prio );

			return 0;
		}
		else
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
					"Can't get priority." );

			return -1;
		}
	}
}





/*
   0	成功
  -1	失败
*/
int initializeDatabase(struct outCtx *ctx)
{
	if(db_open(DB_PATH, &ctx->pDb))
	{
		return 0;
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Can't open database." );

		return -1;
	}
}






/*
   0	成功
  -1	失败
*/
int initializeFpga(struct outCtx *ctx)
{
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open FPGA device.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return -1;
	}

	return 0;
}





/*
   0	成功
  -1	失败
*/
int initializeInt(struct outCtx *ctx)
{
	ctx->int_fd = IntOpen(INT1_DEV);
	if(-1 == ctx->int_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open INT1 device.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

		return -1;
	}

	return 0;
}





void initializeContext(struct outCtx *ctx)
{
	int i;
	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->power_flag = true;
	ctx->pDb = NULL;
	ctx->fpga_fd = -1;
	ctx->int_fd = -1;
	ctx->pps_count = 0;
	ctx->thread_id = -1;

	ctx->old_ref_source = 0x0B;
	ctx->new_ref_source = 0x0B;

	memset(ctx->oldBid, BID_NONE, SLOT_CURSOR_ARRAY_SIZE);

	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ctx->ipc[i].ipc_msgq_id = -1;
		ctx->ipc[i].ipc_sem_id = -1;
		ctx->ipc[i].ipc_shm_id = -1;
		ctx->ipc[i].ipc_base = (void *)-1;
	}

	memset(&ctx->out_sa, 0, sizeof(struct out2mbsainfo));
	memset(&ctx->in_zone, 0, sizeof(struct inzoneinfo));
	memset(&ctx->out_zone, 0, sizeof(struct outzoneinfo));
	memset(&ctx->out_br, 0, sizeof(struct baudrate));
	memset(&ctx->irigb_amp, 0, sizeof(struct amplitude));
	memset(&ctx->irigb_vol, 0, sizeof(struct voltage));
	memset(&(ctx->lp), 0, sizeof(struct leapinfo));
	memset(&(ctx->src_time), 0, sizeof(struct srcinfo));
	memset(&(ctx->sys_time), 0, sizeof(struct srcinfo));

	
	for (i = 0; i < OUTDELAY_SIGNALTYPE_LEN; ++i){
		ctx->od[i].signal = gOutdelayTbl[i].signal;
		ctx->od[i].delay = 0;
	}
	
	memset(ctx->alarm, 0, ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta));
	memset(ctx->outSta, 0, SLOT_CURSOR_ARRAY_SIZE*sizeof(struct outputinfo));
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(input).");
		return -1;
	}

	base = shm_attach(id);// 访问key的共享内存
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(output)." );

		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(ntp).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(alarm).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(clock).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(mgr).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open share memory(keylcd).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to attach share memory(keylcd).");
		return -1;
	}

	ipc[IPC_CURSOR_KEYLCD].ipc_shm_id = id;
	ipc[IPC_CURSOR_KEYLCD].ipc_base = base;

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
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_OUTPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to open message queue(output)." );

		return -1;
	}
	ipc[IPC_CURSOR_OUTPUT].ipc_msgq_id = id;

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
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
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
   0	成功
  -1	失败
*/
int cleanDatabase(struct outCtx *ctx)
{
	if(db_close(ctx->pDb))
	{
		return 0;
	}
	else
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Can't close database." );

		return -1;
	}
}







/*
   0	成功
  -1	失败
*/
int cleanFpga(struct outCtx *ctx)
{
	if(-1 != ctx->fpga_fd)
	{
		if(!FpgaClose(ctx->fpga_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close FPGA device.", 
					gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

			return -1;
		}
	}

	return 0;
}





/*
   0	成功
  -1	失败
*/
int cleanInt(struct outCtx *ctx)
{
	if(-1 != ctx->int_fd)
	{
		if(!IntClose(ctx->int_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close INT1 device.", 
					gDaemonTbl[DAEMON_CURSOR_OUTPUT] );

			return -1;
		}
	}

	return 0;
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
							gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
							"Failed to remove share memory." );

					return -1;
				}
				ipc->ipc_shm_id = -1;
			}
		}
		else
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
					"Failed to detach share memory." );

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

	pid.p_cursor = DAEMON_CURSOR_OUTPUT;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to lock semaphore." );

		return -1;
	}
	shm_write(ipc[IPC_CURSOR_OUTPUT].ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc[IPC_CURSOR_OUTPUT].ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT], 
				"Failed to unlock semaphore." );

		return -1;
	}
	
	return 0;
}





