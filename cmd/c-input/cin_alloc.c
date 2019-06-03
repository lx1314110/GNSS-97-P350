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

#include "cin_alloc.h"




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
		return -1;
	}

	base = shm_attach(id);
	if( ((void *)-1) == base )
	{
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
		return -1;
	}

	ipc->ipc_sem_id = id;

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
					return -1;
				}
				ipc->ipc_shm_id = -1;
			}
		}
		else
		{
			return -1;
		}
	}

	return 0;
}




/*
  -1	失败
   0	成功
*/
int readPid(struct ipcinfo *ipc, struct pidinfo *pid)
{
	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	shm_read(ipc->ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		return -1;
	}
	
	return 0;
}





