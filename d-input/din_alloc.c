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
#include "lib_int.h"
#include "lib_fpga.h"
#include "lib_sqlite.h"

#include "din_alloc.h"
#include "din_global.h"



static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{
		gInCtx.loop_flag = false;
	}
}


static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		gInCtx.notify_flag = true;
	}
}



/*
   0	成功
  -1	失败
*/
int initializeDatabase(struct inCtx *ctx)
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

int ReadLeapsecond(struct inCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select leapsecond,leapmode from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"--Failed to read leapsecond.");
		
		return 0;
	}
	
	ctx->lp_old.leapSecond = atoi(resultp[2]);
	ctx->lp_set.leapSecond = atoi(resultp[2]);
	ctx->lp_set.leapMode = atoi(resultp[3]);
	
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}
/*
int ReadSatelliteModel(struct inCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select model from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read leapsecond.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	
	ctx->satellite_model = atoi(resultp[ncolumn+1]);
	ctx->satellite_mode2 = atoi(resultp[ncolumn*2+1]);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}
*/

/*
   0	成功
  -1	失败
*/
int initializeFpga(struct inCtx *ctx)
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
   0	成功
  -1	失败
*/
int initializeInt(struct inCtx *ctx)
{
	ctx->int_fd = IntOpen(INT2_DEV);
	if(-1 == ctx->int_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open INT2 device.", 
				gDaemonTbl[DAEMON_CURSOR_INPUT] );

		return -1;
	}

	return 0;
}

void initializeContext(struct inCtx *ctx)
{
	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->power_flag = true;
	int i;
	memset(ctx->inSta, 0x0, sizeof(struct inputsta) * INPUT_SLOT_CURSOR_ARRAY_SIZE);
	for(i = INPUT_SLOT_CURSOR_1;i < INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		ctx->inSta[i].satellite_valid = INIT_SATELLITE;
		ctx->inSta[i].sys_mode = MODE_CURSOR_NONE_VAL;
		ctx->inSta[i].satellite_state = 0x03;//gps and bd sate num >5
		ctx->inSta[i].antenna_state = SATELLITE_ANTENNA_NORMAL;
		
	}
	ctx->pDb = NULL;
	ctx->fpga_fd = -1;
	ctx->int_fd = -1;

	ctx->pps_count = 0;
	ctx->pps_flag = false;
	//ctx->gps_leap_forecast = 0;
	ctx->sys_conf.ph_upload_en = 0;
	ctx->clock_type = 0;//1-RB; 2-XO
	memset(&ctx->lp_old, 0, sizeof(struct leapinfo));
	memset(&ctx->lp_new, 0, sizeof(struct leapinfo));
	memset(&ctx->lp_set, 0, sizeof(struct leapinfo));


	ctx->thread_id = -1;

	memset(ctx->oldBid, 0, INPUT_SLOT_CURSOR_ARRAY_SIZE);
	memset(&(ctx->pid), 0, sizeof(struct pidinfo));

	ctx->ipc.ipc_msgq_id = -1;
	ctx->ipc.ipc_sem_id = -1;
	ctx->ipc.ipc_shm_id = -1;
	ctx->ipc.ipc_base = (void *)-1;

	memset(&ctx->cs, 0, sizeof(struct schemainfo));
	ctx->cs.refsrc_is_valid = REFSOURCE_INIT;
	ctx->cs.slot = INPUT_SLOT_CURSOR_1+1;
	ctx->cs.port = 1;
	ctx->cs.bid = BID_NONE;
	ctx->cs.flag = INVALID_SOURCE_FLAG;
	memset(ctx->alarm, 0, ALM_CURSOR_ARRAY_SIZE *sizeof(struct alarmsta));
	memset(ctx->inSta, 0, INPUT_SLOT_CURSOR_ARRAY_SIZE*sizeof(struct inputinfo));

	ctx->satelliteBuffer = malloc(GB_BUF_LEN +1);
	if(ctx->satelliteBuffer)
		memset(ctx->satelliteBuffer, 0, GB_BUF_LEN +1);
	else
		print(DBG_ERROR, "SatelliteBuffer malloc is failure!");
}







void cleanContext(struct inCtx *ctx)
{
	if(ctx->satelliteBuffer){
		free(ctx->satelliteBuffer);
		ctx->satelliteBuffer = NULL;
	}
}


/*
   0	成功
  -1	失败
*/
void SetVarinfo(struct inCtx *ctx)
{
	int i;
	struct sat_priv_data_t *sat_pdata = NULL;
	for (i = INPUT_SLOT_CURSOR_1; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		FIND_MAXPORT_GBATBL_BY_BID(ctx->inSta[i].boardId, ctx->inSta[i].port_num);

		switch (ctx->inSta[i].boardId){
			case BID_INSATE_LOWER ... BID_INSATE_UPPER:
				sat_pdata = NULL;
				FIND_PRIVDATA_GBATBL_BY_BID(ctx->inSta[i].boardId, sat_pdata);
				if (!sat_pdata)
					break;
				ctx->inSta[i].satcommon.sta_flag = sat_pdata->sta_flag;
				ctx->inSta[i].satcommon.satcomminfo.info_flag = sat_pdata->info_flag;
				break;
			default:
				//other board
				//ctx->inSta[i].satcommon.sta_flag = 0;
				//ctx->inSta[i].satcommon.satcomminfo.info_flag = 0;
				break;
		}
	}
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

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR,
				"Failed to attach share memory(input).");

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

	//open message queue(input)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_INPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"Failed to open message queue(input).");

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
	//---------------------------------------------------------------
	id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(id < 0)
	{
		print(	DBG_ERROR,
				"Failed to open semaphore set.");

		return -1;
	}

	ipc->ipc_sem_id = id;

	return 0;
}



/*
   0	成功
  -1	失败
*/
int cleanDatabase(struct inCtx *ctx)
{
	if(NULL != ctx->pDb)
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

	return 0;
}







/*
   0	成功
  -1	失败
*/
int cleanFpga(struct inCtx *ctx)
{
	if(-1 != ctx->fpga_fd)
	{
		if(!FpgaClose(ctx->fpga_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close FPGA device.", 
					gDaemonTbl[DAEMON_CURSOR_INPUT]);

			return -1;
		}
	}

	return 0;
}









/*
   0	成功
  -1	失败
*/
int cleanInt(struct inCtx *ctx)
{
	if(-1 != ctx->int_fd)
	{
		if(!IntClose(ctx->int_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close INT2 device.", 
					gDaemonTbl[DAEMON_CURSOR_INPUT] );

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
							"Failed to remove share memory.");

					return -1;
				}
				ipc->ipc_shm_id = -1;
			}
		}
		else
		{
			print(	DBG_ERROR, 
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
				"Can't set priority.");

		return(-1);
	}
	else
	{
		errno = 0;
		prio = getpriority(PRIO_PROCESS, pid);
		if ( 0 == errno )
		{
			print(DBG_INFORMATIONAL, 
				"\n--Priority is %d\n", 
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



/*
  -1	失败
   0	成功
*/
int writePid(struct ipcinfo *ipc)
{
	struct pidinfo pid;

	pid.p_cursor = DAEMON_CURSOR_INPUT;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"Failed to lock semaphore.");

		return -1;
	}
	shm_write(ipc->ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"Failed to unlock semaphore.");

		return -1;
	}
	
	return 0;
}






