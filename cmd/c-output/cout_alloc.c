#include "lib_msgQ.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_fpga.h"

#include "addr.h"
#include "cout_alloc.h"




/*
  -1	失败
   0	成功
*/
int initializeShareMemory(struct ipcinfo *ipc)
{
	int id = -1;
	void *base = (void *)-1;

	//open share memory(output)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_OUTPUT);
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

	//open message queue(output)
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_OUTPUT);
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
int readPid(struct ipcinfo *ipc, struct pidinfo *pid)
{
	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	shm_read(ipc->ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	return 0;
}







/*
  -1	失败
   0	成功
*/
int readBid(struct outCtx * ctx)
{
	u16_t temp[8];
	int i,j;

	for(i=SLOT_CURSOR_1,j=0; (i<SLOT_CURSOR_ARRAY_SIZE)&&(j<8); i+=2,j++)
	{
		if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S01_TO_S16(j), &temp[j]))
		{
			return -1;
		}
	
		ctx->bid[i]= (temp[j]>>8)&0x00FF;
		if(0x00FF == ctx->bid[i])
		{
			ctx->bid[i] = 0x00;
		}
		ctx->bid[i+1] = temp[j]&0x00FF;
		if(0x00FF == ctx->bid[i+1])
		{
			ctx->bid[i+1] = 0x00;
		}
	}

	return 0;
}





