#include <stdio.h>
#include "lib_msgQ.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_sqlite.h"
#include "lib_fpga.h"

#include "calm_alloc.h"




/*
  -1	失败
   0	成功
*/
int initializeShareMemory(struct ipcinfo *ipc)
{
	int id = -1;
	void *base = (void *)-1;

	//open share memory(alarm)
	//------------------------------------------------------------------------
	id = shm_create(SHM_PATH, SHM_PROJ_ID_ALARM);
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
	id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_ALARM);
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
		return -1;
	}
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
		return -1;
	}
}






/*
  -1	失败
   0	成功
*/
int readPid(struct ipcinfo *ipc, struct pidinfo *pid)
{
	if(1 != sema_lock(ipc->ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	shm_read( ipc->ipc_base, 
			  SHM_OFFSET_PID, 
			  sizeof(struct pidinfo), 
			  (char *)pid, 
			  sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc->ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	return 0;
}

int ReadBidFromFpga(struct almCtx * ctx)
{
	u16_t temp[10];
	int i,j;

	if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S17, &temp[8]))
	{
		return -1;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S18_S19, &temp[9]))
	{
		return -1;
	}

	for(i=SLOT_CURSOR_1,j=0; (i<SLOT_CURSOR_ARRAY_SIZE)&&(j<8); i+=2,j++)
	{
		if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S01_TO_S16(j), &temp[j]))
		{
			return -1;
		}

		ctx->bid[i] = (temp[j]>>8)&0x00FF;
		//if(0x00FF == ctx->bid[i])
		if(BID_ARRAY_SIZE <= ctx->bid[i])
		{
			ctx->bid[i] = 0x00;
		}
		ctx->bid[i+1] = temp[j]&0x00FF;
		//if(0x00FF == ctx->bid[i+1])
		if(BID_ARRAY_SIZE <= ctx->bid[i+1])
		{
			ctx->bid[i+1] = 0x00;
		}
	}

	//slot 17
	ctx->bid[OTHER_SLOT_CURSOR_17] = temp[8] &0x00FF;
	//if(0x00FF == bid)
	if(ctx->bid[OTHER_SLOT_CURSOR_17] >= BID_ARRAY_SIZE)
	{
		ctx->bid[OTHER_SLOT_CURSOR_17] = BID_NONE;
	}

	//slot 18
	ctx->bid[OTHER_SLOT_CURSOR_18] = (temp[9]>>8)&0x00FF;
	//if(0x00FF == bid)
	if(ctx->bid[OTHER_SLOT_CURSOR_18] >= BID_ARRAY_SIZE)
	{
		ctx->bid[OTHER_SLOT_CURSOR_18] = BID_NONE;
	}

	//slot 19
	ctx->bid[OTHER_SLOT_CURSOR_19] = temp[9]&0x00FF;
	//if(0x00FF == bid)
	if(ctx->bid[OTHER_SLOT_CURSOR_19] >= BID_ARRAY_SIZE)
	{
		ctx->bid[OTHER_SLOT_CURSOR_19] = BID_NONE;
	}

	//slot 20
	//del the function
	ctx->bid[OTHER_SLOT_CURSOR_20] = BID_NONE;

	//check len
	if(OTHER_SLOT_CURSOR_20+1 != SLOT_CURSOR_ARRAY_SIZE+1+PWR_SLOT_CURSOR_ARRAY_SIZE+1){
			printf("len error.\n");
			return -1;
	}

	return 0;
}




