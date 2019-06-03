#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include "lib_dbg.h"
#include "lib_int.h"

#include "din_ctrl.h"











void * thread_start_routine(void *args)
{
	fd_set rds;
	struct inCtx *ctx;

	ctx = (struct inCtx *)args;

	while(isRunning(ctx))
	{
		FD_ZERO(&rds);
		FD_SET(ctx->int_fd, &rds);
		if(0 < select(ctx->int_fd +1, &rds, NULL, NULL, NULL))
		{
			if(FD_ISSET(ctx->int_fd, &rds))
			{
				ctx->pps_flag = true;
			}
		}
	}

	print(DBG_NOTICE, "Thread exit clearly!\n");
	pthread_exit(NULL);
}









/*
  1	成功
  0	失败
*/
int CreateThread(struct inCtx *ctx)
{
	int retval;
	
	retval = pthread_create(&(ctx->thread_id), NULL, thread_start_routine, (void*)ctx);
	if(0 != retval)
	{
		return 0;
	}

	return 1;
}









/*
  1	成功
  0	失败
*/
int CloseThread(struct inCtx *ctx)
{
	int retval;
	ctx->loop_flag = false;
	retval = pthread_join(ctx->thread_id, NULL);
	if( 0 != retval )
	{
		return 0;
	}
	
	return 1;
}





