#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib_time.h"
#include "lib_log.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_shm.h"
#include "alloc.h"
#include "lib_common.h"














/*
  1		成功
  0		失败
*/
int db_log_write(sqlite3 *db, char *table, char *desc)
{
	int nrow;
	int ncolumn;
	char **resultp;
	struct timeinfo ti;
	char sql[128];

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", table);
	
	db_lock(db);
	if(db_get_table(db, sql, &resultp, &nrow, &ncolumn))
	{
		nrow = atoi(resultp[1]);
		db_free_table(resultp);
		
		if(MAX_LOG <= nrow)
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s where rowid <= (select (min(rowid) + max(rowid))/2 from %s);", table, table);
			if(!db_put_table(db, sql))
			{
				db_unlock(db);
		
				return 0;
			}
		}

		if(0 == GetSysTime(&ti))
		{
			db_unlock(db);
		
			return 0;
		}
		
		memset(sql, 0, sizeof(sql));
		sprintf(sql, 
				"insert into %s values(\'%04d-%02d-%02d %02d:%02d:%02d\', \'%s\');", 
				table, 
				ti.year, ti.month, ti.day, ti.hour, ti.minute, ti.second, 
				desc);
		if(db_put_table(db, sql))
		{
			db_unlock(db);
		
			return 1;
		}
		else
		{
			db_unlock(db);
		
			return 0;
		}
	}
	else
	{
		db_unlock(db);
		
		return 0;
	}
}

/*
成功  0
失败  -1
*/
int send_log_message(char *buf)
{
	int ret = -1;
	struct ipcinfo ipc;
	struct msgbuf msg;
	struct pidinfo pid;
	struct timeinfo ti;
	
	if(0 == GetSysTime(&ti))
	{
		return -1;
	}
	
	sprintf(buf, "%s|%04d-%02d-%02d %02d:%02d:%02d", buf,ti.year, ti.month, ti.day, ti.hour, ti.minute, ti.second);
	
	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_WG);
	if(-1 == ipc.ipc_msgq_id)
	{
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_WG);
	if(-1 == ipc.ipc_shm_id)
	{
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		goto quit;
	}
	
	memset(&msg, 0, sizeof(struct msgbuf));
	msg.mtype = MSG_CFG_WG_LOG;//日志
	memcpy(msg.mdata, buf, strlen(buf));//更新配置
	//printf("%s\n",msg.mdata);
	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_WG))
	{
		goto quit;
	}
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));//从共享内存读PID
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_WG))
	{
		goto quit;
	}
	
	if(MsgQ_Send(ipc.ipc_msgq_id, &msg, strlen(buf)))//发送配置消息
	{
		if(notify(pid.p_id, NOTIFY_SIGNAL))//发送通知信号
		{
			ret = 0;
			goto quit;
		}
		else
		{
			goto quit;
		}
	}
	else
	{
		goto quit;
	}

	quit:
	if( 1 == shm_detach(ipc.ipc_base) )
		ipc.ipc_base = (void *)-1;
	return ret;
}








