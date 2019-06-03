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
#include "lib_sqlite.h"

#include "dalm_global.h"
#include "dalm_alloc.h"






static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{
		gAlmCtx.loop_flag = false;
	}
}




static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		gAlmCtx.notify_flag = true;
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
	if ( SIG_ERR == signal( EXIT_SIGNAL1, exitHandler ) )//SIGINT用户键入INTR字符(通常是Ctrl-C)时发出
	{
		print(	DBG_ERROR, 
				"Can't install signal handler for SIGINT!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )//SIGTERM程序自己正常退出
	{
		print(	DBG_ERROR,
				"Can't install signal handler for SIGTERM!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )//SIGHUP关闭终端时发出
	{
		print(	DBG_ERROR,
				"Can't install signal handler for SIGHUP!");
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
				"Can't install signal handler for SIGURG!");
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
	if ( 0 != setpriority(PRIO_PROCESS, pid, prio) )//设置进程、进程组和用户的进程执行优先权
	{			
		print(	DBG_ERROR,
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
					"Can't get priority.");
			return -1;
		}
	}
}







void initializeContext(struct almCtx *ctx)
{
	int i;
	
	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->pDb = NULL;
	ctx->fpga_fd = -1;
	ctx->alm_tag = 0;
	ctx->first_flag = FIRST;
	memset(ctx->bid, 0x00, sizeof(ctx->bid));
	ctx->InSgnl_Threshold.Threshold = THRESHOLD_PHVALUE_INIT;
	//ctx->satellite_usestate = INIT_SATELLITE;
	memset(&ctx->refsrc_state, 0x0, sizeof(struct schemainfo));
	ctx->refsrc_state.refsrc_is_valid = REFSOURCE_INIT;
	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ctx->ipc[i].ipc_msgq_id = -1;
		ctx->ipc[i].ipc_sem_id = -1;
		ctx->ipc[i].ipc_shm_id = -1;
		ctx->ipc[i].ipc_base = (void *)-1;
	}

	
	memset(&ctx->port_status, 0, V_ETH_MAX*sizeof(struct portinfo));
	memset(ctx->select, 0, PORT_ALM *sizeof(struct selectsta));
	memset(ctx->mask, 0, ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta));
	memset(ctx->old_alarm, 0, ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	memset(ctx->new_alarm, 0, ALM_CURSOR_ARRAY_SIZE*sizeof(struct alarmsta));
	memset(ctx->ts, 0, ALM_CURSOR_ARRAY_SIZE*sizeof(struct tssta));
	memset(ctx->cur_alarm, 0, CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
	memset(ctx->upload_alarm, 0, UPLOAD_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t));
}






/*
   0	成功
  -1	失败
*/
int initializeDatabase(struct almCtx *ctx)
{
	if(db_open(DB_PATH, &ctx->pDb))
	{
		return 0;
	}
	else
	{
		print(	DBG_ERROR,
				"Can't open database.");
		return -1;
	}
}






/*
   0	成功
  -1	失败
*/
int initializeFpga(struct almCtx *ctx)
{
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		print(	DBG_ERROR, 
				"--Failed to open FPGA device.");
		return -1;
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
				"Failed to open share memory(input).");
		return -1;
	}

	base = shm_attach(id);// 访问key的共享内存
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
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
				"Failed to open share memory(output).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
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
				"Failed to open share memory(ntp).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR,
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
				"Failed to open share memory(alarm).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR,
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
				"Failed to open share memory(clock).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR,
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
				"Failed to open share memory(mgr).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
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
				"Failed to open share memory(keylcd).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR,
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
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_ALARM);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"Failed to open message queue(alarm).");
		return -1;
	}
	ipc[IPC_CURSOR_ALARM].ipc_msgq_id = id;

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
int cleanShareMemory(struct ipcinfo *ipc)
{
	int i;

	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		if((void *)-1 != ipc[i].ipc_base)
		{
			if( 1 == shm_detach(ipc[i].ipc_base) )
			{
				ipc[i].ipc_base = (void *)-1;
				if(-1 != ipc[i].ipc_shm_id)
				{
					if( 0 == shm_removable(ipc[i].ipc_shm_id) )
					{
						print(	DBG_ERROR,
								"Failed to remove share memory.");
						return -1;
					}
					ipc[i].ipc_shm_id = -1;
				}
			}
			else
			{
				print(	DBG_ERROR,
						"Failed to detach share memory.");
				return -1;
			}
		}
	}

	return 0;
}






/*
   0	成功
  -1	失败
*/
int cleanDatabase(struct almCtx *ctx)
{
	if(db_close(ctx->pDb))
	{
		return 0;
	}
	else
	{
		print(	DBG_ERROR, 
				"Can't close database.");
		return -1;
	}
}







/*
   0	成功
  -1	失败
*/
int cleanFpga(struct almCtx *ctx)
{
	if(-1 != ctx->fpga_fd)
	{
		if(!FpgaClose(ctx->fpga_fd))
		{
			print(	DBG_ERROR, 
					"--Failed to close FPGA device.");
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
	
	pid.p_cursor = DAEMON_CURSOR_ALARM;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR,
				"Failed to lock semaphore.");
		return -1;
	}
	shm_write(ipc[IPC_CURSOR_ALARM].ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		print(	DBG_ERROR,
				"Failed to unlock semaphore.");
		return -1;
	}
	
	return 0;
}







