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

#include "d61850_global.h"
#include "d61850_alloc.h"
#include "lib_serial.h"





static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{
		iec61850_Ctx.loop_flag = false;
	}
}




static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		iec61850_Ctx.notify_flag = true;
	}
}

/*
  true:		收到配置命令
  false:	没有收到配置命令
*/
bool_t isNotify(struct iec61850_Ctx *ctx)
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
   0	成功
  -1	失败
*/
int initializeDatabase(struct iec61850_Ctx *ctx)
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
				"Can't install signal handler for SIGINT!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )
	{
		print(	DBG_ERROR,  
				"Can't install signal handler for SIGTERM!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )
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


void initializeContext(struct iec61850_Ctx *ctx)
{
	int i;
	ctx->loop_flag = true;
	ctx->fpga_fd = -1;
	ctx->serial_fd = -1;
	ctx->pDb = NULL;
	
	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ctx->ipc[i].ipc_msgq_id = -1;
		ctx->ipc[i].ipc_sem_id = -1;
		ctx->ipc[i].ipc_shm_id = -1;
		ctx->ipc[i].ipc_base = (void *)-1;
	}
}


int initializeSerial(struct iec61850_Ctx *ctx)
{
	ctx->serial_fd= serial_init();
	if(-1 == ctx->serial_fd)
	{
		print(	DBG_ERROR, 
				"--Failed to open Serial device.");

		return -1;
	}

	return 0;
}
int cleanSerial(struct iec61850_Ctx *ctx)
{
	if(-1 != ctx->serial_fd)
	{
		if(!close(ctx->serial_fd))
		{
			print(	DBG_ERROR, 
					"--Failed to close Serial device.");

			return -1;
		}
	}

	return 0;
}


int initializeFpga(struct iec61850_Ctx *ctx)
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
int initializeMessageQueue(struct ipcinfo *ipc)
{
	int id = -1;

	//open message queue(alarm)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_61850);
	if(id < 0)
	{
		print(	DBG_ERROR,
				"Failed to open message queue(61850).");
		return -1;
	}
	ipc[IPC_CURSOR_61850].ipc_msgq_id = id;

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

	//open share memory(keylcd)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_KEYLCD);
	if(id < 0)
	{
		print(	DBG_ERROR,
				"Failed to open share memory(61850).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR,
				"Failed to attach share memory(61850).");
		return -1;
	}

	ipc[IPC_CURSOR_61850].ipc_shm_id = id;
	ipc[IPC_CURSOR_61850].ipc_base = base;

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
int writePid(struct ipcinfo *ipc)
{
	struct pidinfo pid;
	
	pid.p_cursor = DAEMON_CURSOR_IEC61850;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc[IPC_CURSOR_61850].ipc_sem_id, SEMA_MEMBER_61850))
	{
		print(	DBG_ERROR, 
				"Failed to lock semaphore.");
		return -1;
	}
	shm_write(ipc[IPC_CURSOR_61850].ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc[IPC_CURSOR_61850].ipc_sem_id, SEMA_MEMBER_61850))
	{
		print(	DBG_ERROR,
				"Failed to unlock semaphore.");
		return -1;
	}
	
	return 0;
}

int Write61850Ip(struct iec61850_Ctx *ctx, struct veth *tmp, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				 set ip=\'%s\';", 
				 pTbl, 			
				 tmp->v_ip);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"--Failed to write 61850 ip.");
		
		return 0;
	}
	else
	{
		return 1;
	}
}


int SysSetIp(char *data, struct iec61850_Ctx *ctx)
{
	struct veth *tmp;
	tmp = (struct veth *)data;	
	
	if(0 == Write61850Ip(ctx, tmp, TBL_61850))
	{
		print(	DBG_ERROR, 
					"Failed to Write61850Ip");
		return 0;
	}
	return 1;
}





/*
  1	成功
  0	失败
*/
int ReadMessageQueue(struct iec61850_Ctx *ctx)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
		ret = MsgQ_Rcv(ctx->ipc[IPC_CURSOR_61850].ipc_msgq_id, &msg);
		if(ret < 0)
		{
			print(	DBG_ERROR, 
					"Failed to read message queue");
			return 0;
		}
		
		if(ret > 0)
		{
			if(MSG_CFG_61850_IP == msg.mtype)//调试
			{
				if(0 == SysSetIp(msg.mdata, ctx))
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

int set61850_network(struct iec61850_Ctx *ctx, char *pTbl)
{
	char sql[64],ip[16] = {0};
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
//	int tmp;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select ip from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read 61850 network.");

		return -1;
	}
	
	memcpy(ip, resultp[0], strlen(resultp[0]));
	db_free_table(resultp);
	db_unlock(ctx->pDb);

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
int cleanDatabase(struct iec61850_Ctx *ctx)
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



int cleanFpga(struct iec61850_Ctx *ctx)
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








