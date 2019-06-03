#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "lib_dbg.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_shm.h"
#include "lib_common.h"

#include "alloc.h"
#include "cdbg_global.h"
#include "cdbg_parser.h"







void helper(void)
{
	int i;

	for(i=0; gHelpTbl[i]; i++)
	{
		printf("%s\n", gHelpTbl[i]);
	}

	printf("        PROC is");
	for(i=DAEMON_CURSOR_NONE+1; i<DAEMON_CURSOR_ARRAY_SIZE; i++)
	{
		printf(" %s", gDaemonTbl[i]);
	}
	printf("\n");

	printf("        LEVEL is 0~8\n");
	printf("        0	disable\n");
	printf("        1	emergency\n");
	printf("        2	alert\n");
	printf("        3	critical\n");
	printf("        4	error\n");
	printf("        5	warning\n");
	printf("        6	notice\n");
	printf("        7	informational\n");
	printf("        8	debug\n");

	printf("        SYSLOG_EN is 0~1\n");
	printf("        0	disable\n");
	printf("        1	enable(default)\n");
}






/*
  -1	失败
   0	成功
*/
int argv_parser(int argc, char *argv[])
{
	int i,j,k,m;
	u8_t syslog_en;
	u8_t dbg_level;
	u8_t found = 0;
	struct config dbg;
	struct msgbuf msg;
	struct pidinfo pid;
	struct ipcinfo ipc;

	if(2 == argc){
		if(0 == memcmp(HELP, argv[1], strlen(argv[1])))
		{
			helper();
			return 0;
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	} else if(3 == argc || 4 == argc) {

		dbg_level = (u8_t) atoi(argv[2]);
		if (dbg_level > DBG_DEBUG || dbg_level < DBG_EMERGENCY){
			printf("Failure.\n");
			return -1;
		}
		if (4 == argc){
			syslog_en = (u8_t) atoi(argv[3]);
		}else{
			syslog_en = true;
		}
		if (syslog_en != false && syslog_en != true){
			printf("Failure.\n");
			return -1;
		}
		for(i=DAEMON_CURSOR_NONE+1,j=MSGQ_PROJ_ID_INPUT,k=SHM_PROJ_ID_INPUT,m=SEMA_MEMBER_INPUT; 
			i<(DAEMON_CURSOR_ARRAY_SIZE -2); 
			i++,j++,k++,m++)
		{
			if(0 == memcmp(gDaemonTbl[i], argv[1], strlen(argv[1])))
			{
				found = 1;
				ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, j);
				if(-1 == ipc.ipc_msgq_id)
				{
					printf("Failure.\n");
					return -1;
				}

				ipc.ipc_shm_id = shm_create(SHM_PATH, k);
				if(-1 == ipc.ipc_shm_id)
				{
					printf("Failure.\n");
					return -1;
				}

				ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
				if(((void *)-1) == ipc.ipc_base)
				{
					printf("Failure.\n");
					return -1;
				}

				ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
				if(-1 == ipc.ipc_sem_id)
				{
					printf("Failure.\n");
					return -1;
				}
				
				memset(&dbg, 0, sizeof(struct config));
				dbg.dbg_config.dbg_level = dbg_level;
				dbg.dbg_config.syslog_en = syslog_en;

				memset(&msg, 0, sizeof(struct msgbuf));
				msg.mtype = MSG_CFG_DBG;
				memcpy(msg.mdata, dbg.buffer, sizeof(struct dbginfo));//调试信息

				if(1 != sema_lock(ipc.ipc_sem_id, m))
				{
					printf("Failure.\n");
					return -1;
				}
				shm_read(ipc.ipc_base, 
						 SHM_OFFSET_PID, 
						 sizeof(struct pidinfo), 
						 (char *)&pid, 
						 sizeof(struct pidinfo));
				if(1 != sema_unlock(ipc.ipc_sem_id, m))
				{
					printf("Failure.\n");
					return -1;
				}

				if( 1 == shm_detach(ipc.ipc_base) )
					ipc.ipc_base = (void *)-1;
				
				if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct dbginfo)))
				{
					if(notify(pid.p_id, NOTIFY_SIGNAL))//通知进程进行处理
					{
						printf("Success.\n");
						return 0;
					}
					else
					{
						printf("Failure.\n");
						return -1;
					}
				}
				else
				{
					
					printf("Failure.\n");
					return -1;
				}
			}
		}

		if(found == 0){
			printf("Failure.\n");
			return -1;
		}
	}else{
		printf("Failure.\n");
		return -1;
	}

	//no exec it
	printf("Success.\n");
	return 0;
}




