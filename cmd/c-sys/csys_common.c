#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_shm.h"
#include "lib_common.h"

#include "alloc.h"
#include "csys_common.h"


/*
  0	不合法
  1 合法
*/
int switch_validity(char *swi)
{

	if(!swi)
		return 0;
	//len = (strlen(swi) > strlen("disable") ) ?  strlen(swi):strlen("disable")
	if (memcmp(swi, "on", strlen("on")) == 0){
		return 1;
	}
	if (memcmp(swi, "off", strlen("off")) == 0){
		return 1;
	}
	return 0;
}

/*
  1	合法
  0	非法
*/
int enable_validity(char *en)
{
	int len;

	len = strlen(en);
	if(	(1 == len) && 
		(('0' == en[0]) || ('1' == en[0])) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int initializeDatabase(sqlite3 **pDb)
{
	if(db_open(DB_PATH, pDb))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int cleanDatabase(sqlite3 *pDb)
{
	if(db_close(pDb))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


/*
  1	成功
  0	失败
*/
int sysUpdate(struct ipcinfo *ipc)
{
	struct config update;
	struct msgbuf msg;
	struct pidinfo pid;

	memset(&update, 0, sizeof(struct config));
	update.update_config.bUpdate = 0x01;

	memset(&msg, 0, sizeof(struct msgbuf));
	msg.mtype = MSG_CFG_UPDATE;
	memcpy(msg.mdata, update.buffer, sizeof(struct updateinfo));

	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		return 0;
	}
	shm_read(ipc->ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		return 0;
	}
	
	if(MsgQ_Send(ipc->ipc_msgq_id, &msg, sizeof(struct updateinfo)))
	{
		if(notify(pid.p_id, NOTIFY_SIGNAL))
			return 1;
		else
			return 0;
	}
	else
	{
		return 0;
	}
}



/*
  1	成功
  0	失败
*/
int sysReboot(struct ipcinfo *ipc)
{
	struct config reboot;
	struct msgbuf msg;
	struct pidinfo pid;

	memset(&reboot, 0, sizeof(struct config));
	reboot.reboot_config.bReboot = 0x01;

	memset(&msg, 0, sizeof(struct msgbuf));
	msg.mtype = MSG_CFG_REBOOT;
	memcpy(msg.mdata, reboot.buffer, sizeof(struct rebootinfo));

	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		return 0;
	}
	shm_read(ipc->ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		return 0;
	}
	
	if(MsgQ_Send(ipc->ipc_msgq_id, &msg, sizeof(struct rebootinfo)))
	{
		if(notify(pid.p_id, NOTIFY_SIGNAL))
			return 1;
		else
			return 0;
	}
	else
	{
		return 0;
	}
}


