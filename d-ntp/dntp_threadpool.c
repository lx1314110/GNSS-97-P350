#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib_dbg.h"
#include "alloc.h"
#include "dntp_threadpool.h"
#include "dntp_server.h"




void * read_time_usable(void *args)
{
	
	
	//u8_t state = 0;
	struct schemainfo schema;
	struct clock_stainfo clock_sta;
	u16_t clock_state;
	
	bool_t  loop_flag= (bool_t)args;

	int id = -1;
	void *input_shm_base = (void *)-1;
	void *alarm_shm_base = (void *)-1;
	int sem_id = -1;
	
	//open semaphore set
	sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(sem_id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to open semaphore set.");
		goto pth_exit;
	}

	//open share memory(ntp)
	id = shm_create(SHM_PATH, SHM_PROJ_ID_INPUT);
	if(id < 0)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to open share memory(ntp).");
		goto pth_exit;
	}

	input_shm_base = shm_attach(id);
	if( ((void *)-1) == input_shm_base )
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to attach share memory(ntp).");
		goto pth_exit;
	}

	id = shm_create(SHM_PATH, SHM_PROJ_ID_ALARM);
	if(-1 == id)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to open share memory(ntp).");
		goto pth_exit;
	}

	alarm_shm_base = shm_attach(id);
	if(((void *)-1) == alarm_shm_base)
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to attach share memory(ntp).");
		goto pth_exit;
	}
	
	while(loop_flag)
	{
		if(1 != sema_lock(sem_id, SEMA_MEMBER_INPUT))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to lock semaphore.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );

			break;
		}

		#if 0
		shm_read(input_shm_base, 
				  SHM_OFFSET_FRESH, 
				  sizeof(state), 
				  (char *)&state, 
				  sizeof(state));
		#else
			shm_read(input_shm_base, 
			 SHM_OFFSET_SCHEMA, 
			 sizeof(struct schemainfo), 
			 (char *)&schema, 
			 sizeof(struct schemainfo));
		#endif
		if(1 != sema_unlock(sem_id, SEMA_MEMBER_INPUT))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to unlock semaphore.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );
			
			break;
		}


		if(1 != sema_lock(sem_id, SEMA_MEMBER_ALARM))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to lock semaphore.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );

			break;
		}
		
		shm_read(alarm_shm_base, 
				 SHM_OFFSET_CLOCK, 
				 sizeof(clock_sta), 
				  (char *)&clock_sta, 
				 sizeof(clock_sta));
		
		if(1 != sema_unlock(sem_id, SEMA_MEMBER_ALARM))
		{
			print(	DBG_ERROR, 
					"<%s>--Failed to unlock semaphore.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );
			
			break;
		}
		clock_state = clock_sta.state;
		if( (REFSOURCE_VALID == schema.refsrc_is_valid) && (3 <= clock_state))
		{
			li = 0;
			break;
		}
		else 
		{
			li = 3;
		}
		sleep(1);
	}


pth_exit:
	if( 1 == shm_detach(input_shm_base) )
		input_shm_base = (void *)-1;
	if( 1 == shm_detach(alarm_shm_base) )
		alarm_shm_base = (void *)-1;

	print(DBG_NOTICE, "Thread exit clearly!\n");
	pthread_exit(NULL);

	
}


/*
  1	成功
  0	失败
*/
int CreateThread(bool_t loop_flag,pthread_t *thread_id)
{
	int retval;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	retval = pthread_create(thread_id, &attr, read_time_usable, (void*)loop_flag);
	pthread_attr_destroy(&attr);
	if(0 != retval)
	{
		return 0;
	}

	return 1;
}


/*
  1
  0
*/
/*
int CloseThread(struct wgCtx *ctx)
{
	int retval;
	retval = pthread_join(ctx->thread_1, NULL);
	if( 0 != retval )
	{
		return 0;
	}
	
	retval = pthread_join(ctx->thread_2, NULL);
	if( 0 != retval )
	{
		return 0;
	}
	
	return 1;
}
*/


void * thread_start_routine(void *args)
{
	struct tasknode *taskp = NULL;
	struct threadpool *pool = (struct threadpool *)args;
	
	while(1)
	{
		pthread_mutex_lock(&(pool->lock));
		
		if( pool->bExit && STAILQ_EMPTY( &(pool->tqHead) ) )
		{
			pool->curThreads -= 1;
			pthread_mutex_unlock(&(pool->lock));
			goto exit;
		}
		
		//avoid resource unavailable when pthread_cond_wait return
		while( STAILQ_EMPTY( &(pool->tqHead) ) )
		{
			pthread_cond_wait(&(pool->notify), &(pool->lock));
			if( pool->bExit && STAILQ_EMPTY( &(pool->tqHead) ) )
			{
				pool->curThreads -= 1;
				pthread_mutex_unlock(&(pool->lock));
				goto exit;
			}
		}
		
		taskp = STAILQ_FIRST( &(pool->tqHead) );
		STAILQ_REMOVE_HEAD( &(pool->tqHead), entries );
		pool->curTasks -= 1;

		pthread_mutex_unlock(&(pool->lock));

		(taskp->task_handler)(taskp->args);
		free(taskp->args);
		taskp->args = NULL;
		free(taskp);
		taskp = NULL;
	}
	
exit:
	pthread_exit(NULL);
}








/*
  1		success
  0		failure
*/
bool_t tpCreate(struct threadpool *pool, u32_t threads, u32_t tasks)
{
	int i;
	
	if(	!((NULL != pool) && 
		 ((threads > 0) && (threads <= MAX_THREAD)) && 
		 ((tasks > 0) && (tasks <= MAX_TASK))) )
	{
		return false;
	}

	memset(pool, 0, sizeof(struct threadpool));
	pool->bExit = 0;
	
	pool->curTasks = 0;
	pool->maxTasks = tasks;
	STAILQ_INIT( &(pool->tqHead) );

	if( 0 != pthread_mutex_init(&(pool->lock), NULL) )
	{
		return false;
	}
	
	if( 0 != pthread_cond_init(&(pool->notify), NULL) )
	{
		pthread_mutex_destroy( &(pool->lock) );
		
		return false;
	}
	
	for(i=0; i<threads; i++)
	{
		if(0 != pthread_create(&(pool->threadId[i]), NULL, thread_start_routine, (void*)pool))
		{
			pthread_cond_destroy( &(pool->notify) );
			pthread_mutex_destroy( &(pool->lock) );
			
			return false;
		}
		
		pool->curThreads += 1;
	}
	
	return true;
}








/*
  1		success
  0		failure
*/
bool_t tpAddTask(struct threadpool *pool, void (*task_handler)(void *), void *args)
{
	struct tasknode *taskp = NULL;

	if( NULL == pool )
	{
		return false;
	}

	if( 0 != pthread_mutex_lock(&(pool->lock)) )
	{
		return false;
	}

	if( pool->curTasks >= pool->maxTasks )
	{
		pthread_mutex_unlock(&pool->lock);
		print(	DBG_WARNING, 
				"<%s>--Task queue is full", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);
		
		return false;
	}

	//thread pool has been destroyed
	if(pool->bExit)
	{
		pthread_mutex_unlock(&pool->lock);
		print(	DBG_WARNING, 
				"<%s>--Thread pool has been destroyed", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);
		
		return false;
	}

	//allocate new task
	taskp = (struct tasknode *)malloc(sizeof(struct tasknode));
	if(NULL == taskp)
	{
		pthread_mutex_unlock(&pool->lock);
		
		return false;
	}

	taskp->task_handler = task_handler;
	taskp->args = args;
	STAILQ_INSERT_TAIL(&(pool->tqHead), taskp, entries);//把新任务添加进队列
	pool->curTasks += 1;
	
	if( 0 != pthread_cond_signal(&(pool->notify)) )//通知进程执行任务(发送一个信号给另外一个正在处于阻塞等待状态的线程,使其脱离阻塞状态,继续执行)
	{
		free(taskp);
		pthread_mutex_unlock(&pool->lock);
		
		return false;
	}

	if( 0 != pthread_mutex_unlock(&pool->lock) )
	{
		free(taskp);
		return false;
	}

	return true;
}







/*
  1		success
  0		failure
*/
bool_t tpDestroy(struct threadpool *pool)
{
	int i, tmp, num;
	
	if(NULL == pool)
	{
		return false;
	}
	if( 0 != pthread_mutex_lock(&(pool->lock)) )
	{
		return false;
	}
	pool->bExit = 1;
	num = pool->curThreads;
	
	if( 0 != pthread_mutex_unlock(&(pool->lock)) )
	{
		return false;
	}

	do
	{
		pthread_mutex_lock(&(pool->lock));
		tmp = pool->curThreads;
		pthread_cond_broadcast( &(pool->notify) );
		pthread_mutex_unlock(&(pool->lock));
		/*sleep 1 sce for other thread run when use FIFO sched */
		sleep(1);
	}while(0 != tmp);
	
	for(i=0; i<num; i++)
	{
		pthread_join( pool->threadId[i], NULL );
	}
	
	pthread_mutex_destroy(&(pool->lock));
	pthread_cond_destroy(&(pool->notify));
	
	return true;
}





