#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_net.h"
#include "lib_fpga.h"
#include "lib_common.h"

#include "alloc.h"
#include "addr.h"
#include "dmgr_version.h"
#include "dmgr_global.h"
#include "dmgr_alloc.h"
#include "dmgr_global.h"






static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) )
	{
		gMgrCtx.loop_flag = false;
	}
}

static void notifyHandler(int signum)
{
	if( NOTIFY_SIGNAL == signum )
	{
		gMgrCtx.notify_flag = true;
	}
}




static void ChlidHandler(int signum)
{
	int status;
	pid_t pid;
	int i;
	
	if( SIGCHLD == signum ){
			while((pid=waitpid(-1,&status,WNOHANG)) > 0){
			//printf("chlid is exit %d\n",pid);	
			for(i = 0; i < DAEMON_CURSOR_ARRAY_SIZE;i++){
				if (gMgrCtx.pid[i].p_id == pid)
					gMgrCtx.pid[i].p_id =0;
			}
		}
	}
}




/*
  -1	失败
   0	成功
*/
int initializeContext(struct mgrCtx *ctx)
{
	int i;
	
	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->guard_flag = 0;
	ctx->pDb = NULL;
	ctx->pVethDb = NULL;
	ctx->fpga_fd = -1;
	ctx->ps_fd = -1;
	ctx->led_sta = false;

	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ctx->ipc[i].ipc_msgq_id = -1;
		ctx->ipc[i].ipc_sem_id = -1;
		ctx->ipc[i].ipc_shm_id = -1;
		ctx->ipc[i].ipc_base = (void *)-1;
	}

	memset(ctx->pid, 0, DAEMON_CURSOR_ARRAY_SIZE*sizeof(struct pidinfo));
	memset(&ctx->net, 0, sizeof(struct netinfo));
	
	memset(&ctx->ver, 0, sizeof(struct verinfo));
	memcpy(ctx->ver.swVer, SW_VER, 6);
	memcpy(ctx->ver.hwVer, HW_VER, 6);
	
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
	
	//initialize share memory(input)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_INPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(input).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to attach share memory(input).");
		return -1;
	}

	ipc[IPC_CURSOR_INPUT].ipc_shm_id = id;
	ipc[IPC_CURSOR_INPUT].ipc_base = base;

	//initialize share memory(output)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_OUTPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(output).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to attach share memory(output).");
		return -1;
	}

	ipc[IPC_CURSOR_OUTPUT].ipc_shm_id = id;
	ipc[IPC_CURSOR_OUTPUT].ipc_base = base;

	//initialize share memory(ntp)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_NTP);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(ntp).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to attach share memory(ntp).");
		return -1;
	}

	ipc[IPC_CURSOR_NTP].ipc_shm_id = id;
	ipc[IPC_CURSOR_NTP].ipc_base = base;

	//initialize share memory(alarm)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_ALARM);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(alarm).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to attach share memory(alarm).");
		return -1;
	}

	ipc[IPC_CURSOR_ALARM].ipc_shm_id = id;
	ipc[IPC_CURSOR_ALARM].ipc_base = base;

	//initialize share memory(clock)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_CLOCK);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(clock).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to attach share memory(clock).");
		return -1;
	}

	ipc[IPC_CURSOR_CLOCK].ipc_shm_id = id;
	ipc[IPC_CURSOR_CLOCK].ipc_base = base;

	//initialize share memory(manager)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_MANAGER);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(manager).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to attach share memory(manager).");
		return -1;
	}

	ipc[IPC_CURSOR_MANAGER].ipc_shm_id = id;
	ipc[IPC_CURSOR_MANAGER].ipc_base = base;

	//initialize share memory(keylcd)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_KEYLCD);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create share memory(keylcd).");
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
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

	//create message queue(input)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_INPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(input).");
		return -1;
	}
	ipc[IPC_CURSOR_INPUT].ipc_msgq_id = id;

	//create message queue(output)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_OUTPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(output).");
		return -1;
	}
	ipc[IPC_CURSOR_OUTPUT].ipc_msgq_id = id;

	//create message queue(ntp)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_NTP);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(ntp).");
		return -1;
	}
	ipc[IPC_CURSOR_NTP].ipc_msgq_id = id;

	//create message queue(alarm)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_ALARM);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(alarm).");
		return -1;
	}
	ipc[IPC_CURSOR_ALARM].ipc_msgq_id = id;

	//create message queue(clock)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_CLOCK);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(clock).");
		return -1;
	}
	ipc[IPC_CURSOR_CLOCK].ipc_msgq_id = id;

	//create message queue(manager)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_MANAGER);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(manager).");
		return -1;
	}
	ipc[IPC_CURSOR_MANAGER].ipc_msgq_id = id;

	//create message queue(keylcd)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_KEYLCD);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create message queue(keylcd).");
		return -1;
	}
	ipc[IPC_CURSOR_KEYLCD].ipc_msgq_id = id;

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
	
	//initialize semaphore set
	id = sema_create(SEMA_PATH, SEMA_PROJ_ID, SEMA_MEMBER_ARRAY_SIZE);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to create semaphore set.");
		return -1;
	}

	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		ipc[i].ipc_sem_id = id;//all process ipc_sem_id is same
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
								"<%s>--%s", 
								gDaemonTbl[DAEMON_CURSOR_MANAGER], 
								"Failed to remove share memory.");
						return -1;
					}
					ipc[i].ipc_shm_id = -1;
				}
			}
			else
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to detach share memory.");
				return -1;
			}
		}
	}

	return 0;
}




/*
  -1	失败
   0	成功
*/
int cleanMessageQueue(struct ipcinfo *ipc)
{
	int i;

	for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
	{
		if(-1 != ipc[i].ipc_msgq_id)
		{
			if(!MsgQ_Remove(ipc[i].ipc_msgq_id))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to remove message queue.");
				return -1;
			}
			ipc[i].ipc_msgq_id = -1;
		}
	}

	return 0;
}




/*
  -1	失败
   0	成功
*/
int cleanSemaphoreSet(struct ipcinfo *ipc)
{
	int i;
	
	if(-1 != ipc[0].ipc_sem_id)
	{
		if( 0 == sema_remove(ipc[0].ipc_sem_id) )
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Failed to remove semaphore set.");
			return -1;
		}
		for(i=0; i<IPC_CURSOR_ARRAY_SIZE; i++)
		{
			ipc[i].ipc_sem_id = -1;
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
	if ( SIG_ERR == signal( EXIT_SIGNAL1, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Can't install signal handler for SIGINT!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL2, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Can't install signal handler for SIGTERM!");
		return(-1);
	}
	if ( SIG_ERR == signal( EXIT_SIGNAL3, exitHandler ) )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Can't install signal handler for SIGHUP!");
		return(-1);
	}

	return 0;
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
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Can't install signal handler for SIGURG!");
		return(-1);
	}

	return 0;
}

/*chlid signale*/
int initializeChildSignal(void)
{
	if ( SIG_ERR == signal( SIGCHLD, ChlidHandler ) )
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Can't install signal handler for SIGURG!");
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
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
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
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
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
	
	pid.p_cursor = DAEMON_CURSOR_MANAGER;
	pid.p_id = getpid();

	if(1 != sema_lock(ipc[IPC_CURSOR_MANAGER].ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to lock semaphore.");
		return -1;
	}
	
	shm_write(ipc[IPC_CURSOR_MANAGER].ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)&pid, 
			  sizeof(struct pidinfo));
	
	if(1 != sema_unlock(ipc[IPC_CURSOR_MANAGER].ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to unlock semaphore.");
		return -1;
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
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Can't open database", path);
		return 0;
	}
}

#if 1
/*
   1	成功
   0	失败
*/
int initializeDatabase(sqlite3 **pDb)
{
	return init_database(DB_PATH, pDb);
}
#endif
/*
   1	成功
   0	失败
*/
int initializeVethDatabase(sqlite3 **pDb)
{
	
	int ret = 0;
	if(access(MAC_PATH,F_OK) == 0){
		ret = init_database(MAC_PATH, pDb);
		//printf("p350_dmgr: using veth %s\n", MAC_PATH);
	} else if(access(MAC_ORGIN_PATH,F_OK) == 0){
		ret = init_database(MAC_ORGIN_PATH, pDb);
		//printf("p350_dmgr: using veth %s\n", MAC_ORGIN_PATH);
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
		print(DBG_ERROR, "p350_dmgr: using veth database failure\n");
	}

	return ret;
}


/*
   1	成功
   0	失败
*/
void SetDatabase(void)
{
	char cmd[128];
	if(access(DB_PATH,F_OK) == 0){
		print(DBG_INFORMATIONAL, "p350_manager: we will use database %s\n", DB_PATH);
	} else if(access(DB_ORGIN_PATH,F_OK) == 0){
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "mkdir -p %s",USER_DATA_DB_DIR);
		system(cmd);
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "cp -r %s %s", DB_ORGIN_PATH, DB_PATH);
		print(DBG_INFORMATIONAL, "p350_manager: copy %s to %s\n",DB_ORGIN_PATH, DB_PATH);
		system(cmd);
		print(DBG_INFORMATIONAL, "p350_manager: we will use database %s\n", DB_PATH);
		sync();
		sync();
	} else {
		print(DBG_ERROR, "p350_manager: not fould database\n");
	}
}


/*
   1	成功
   0	失败
*/
void SetVethDatabase(void)
{
	char cmd[128];
	if(access(MAC_PATH,F_OK) == 0){
		print(DBG_INFORMATIONAL, "p350_manager: we will use veth %s\n", MAC_PATH);
	} else if(access(MAC_BAKUP_PATH,F_OK) == 0){
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "mkdir -p %s",USER_DATA_MAC_DIR);
		system(cmd);
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "cp -r %s %s", MAC_BAKUP_PATH, MAC_PATH);
		print(DBG_INFORMATIONAL, "p350_manager: copy %s to %s\n",MAC_BAKUP_PATH, MAC_PATH);
		system(cmd);
		sync();
		sync();
	} else if(access(MAC_ORGIN_PATH,F_OK) == 0){
		//FOR support old device in the user's locale
		print(DBG_INFORMATIONAL, "p350_manager: we will use veth %s\n", MAC_ORGIN_PATH);
	} else if(access(DB_ORGIN_PATH,F_OK) == 0){
		//FOR support old device in the user's locale
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "mkdir -p %s",USER_DATA_MAC_DIR);
		system(cmd);
		memset(cmd, 0x0, sizeof(cmd));
		sprintf(cmd, "cp -r %s %s", DB_ORGIN_PATH, MAC_ORGIN_PATH);
		print(DBG_INFORMATIONAL, "p350_manager: copy %s to %s\n",DB_ORGIN_PATH, MAC_ORGIN_PATH);
		system(cmd);
		sync();
		sync();
	} else {
		print(DBG_ERROR, "no find any database file for the veth configure!\n");
	}
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
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
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

int readWebNetwork(sqlite3 *pDb, char *tbl, struct netinfo *net)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select v_ip, v_mac, v_mask, v_gateway from %s where v_port=%d;", tbl,21);
	
	db_lock(pDb);
	ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(pDb);
		return 0;
	}

	memcpy(net->ip, resultp[ncolumn], strlen(resultp[ncolumn]));
	memcpy(net->mac, resultp[ncolumn+1], strlen(resultp[ncolumn+1]));
	memcpy(net->mask, resultp[ncolumn+2], strlen(resultp[ncolumn+2]));
	memcpy(net->gateway, resultp[ncolumn+3], strlen(resultp[ncolumn+3]));
	db_free_table(resultp);
	db_unlock(pDb);
	return 1;
}

/*
  0	失败
  1	成功
*/
#ifdef ETH0_NET_CONFIG
int readNetwork(sqlite3 *pDb, char *tbl, struct netinfo *net)
{
	char sql[128];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select ip,gateway,mask,mac from %s;", tbl);
	
	db_lock(pDb);
	ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	memcpy(net->ip, resultp[4], strlen(resultp[4]));
	memcpy(net->gateway, resultp[5], strlen(resultp[5]));
	memcpy(net->mask, resultp[6], strlen(resultp[6]));
	memcpy(net->mac, resultp[7], strlen(resultp[7]));
	db_free_table(resultp);
	db_unlock(pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to read network.", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER]);
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
  0	失败
  1	成功
*/
int writeNetwork(struct netinfo *netp)
{
	char old_gateway[16];

	if('\0' != netp->ip[0])
	{
		if(0 == __Sock_SetIp(NETWORK_CARD, netp->ip))//设置ip
		{
			return 0;
		}
	}
	/*
	if('\0' != netp->mask[0])
	{
		if(0 == __Sock_SetMask(NETWORK_CARD, netp->mask))//设子网掩码
		{
			return 0;
		}
	}

	if('\0' != netp->mac[0])
	{
		if(0 == __Sock_SetMac(NETWORK_CARD, netp->mac))//设MAC
		{
			return 0;
		}
	}

	if('\0' != netp->gateway[0])
	{
		if(0 == __Sock_GetGateway(old_gateway))//获取网关失败
		{
			if(0 == __Sock_SetGateway(NETWORK_CARD, netp->gateway, netp->gateway))//设置新网关
			{
				return 0;
			}
		}
		else
		{
			if(0 == __Sock_SetGateway(NETWORK_CARD, old_gateway, netp->gateway))//获取成功则设置网关
			{
				return 0;
			}
		}
	}
	*/
	return 1;
}
#endif

int setNetworkUp(void)
{
	return ethn_up(NETWORK_CARD);
}

int writeWebNetwork(struct netinfo *netp)
{
	char networkcard[20];
	int ret = SET_OK;

	sprintf(networkcard, "%s", NETWEB_CARD);
	ret = ethn_add(networkcard, netp->ip, netp->mac, netp->mask, netp->gateway);
	if(ret < 0){
		if(SET_IP_ERR == ret)
		{
			print(	DBG_ERROR, "%s[%d]", "Failed to add virtual ether.", ret);
			return 0;
		}else{
			print(	DBG_ERROR, "%s[%d]", "Failed to add virtual ether.", ret);
		}
	}
	return 1;

}

/*
   0	成功
  -1	失败
*/
int initializeFpga(struct mgrCtx *ctx)
{
	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open FPGA device.", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER]);
		return -1;
	}

	return 0;
}







/*
   0	成功
  -1	失败
*/
int cleanFpga(struct mgrCtx *ctx)
{
	if(-1 != ctx->fpga_fd)
	{
		if(!FpgaClose(ctx->fpga_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close FPGA device.", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER]);
			return -1;
		}
	}

	return 0;
}







/*
   0	成功
  -1	失败
*/
int initializePS(struct mgrCtx *ctx)
{
	ctx->ps_fd = open( PS_DEV, O_RDWR);
	if(-1 == ctx->ps_fd)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to open PS device.", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER]);
		return -1;
	}

	return 0;
}







/*
   0	成功
  -1	失败
*/
int cleanPS(struct mgrCtx *ctx)
{
	if(-1 != ctx->ps_fd)
	{
		if(0 != close(ctx->ps_fd))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to close PS device.", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER]);
			return -1;
		}
	}

	return 0;
}




