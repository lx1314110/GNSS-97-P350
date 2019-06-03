#ifndef __DNTP_THREADPOOL__
#define __DNTP_THREADPOOL__






#include <pthread.h>
#include "lib_type.h"
#include "dntp_queue.h"
#include "lib_shm.h"
#include "lib_sema.h"







struct tasknode {
	void (*task_handler)(void *args);
	void *args;
	STAILQ_ENTRY(tasknode) entries;
};





#define	MAX_THREAD	32
#define	MAX_TASK	2048






struct threadpool {
	u8_t bExit;			//terminate all threads
	
	u32_t curTasks;		//current quantity of tasks
	u32_t maxTasks;		//maximum quantity of tasks
	STAILQ_HEAD(taskhead, tasknode) tqHead;	//task queue head

	pthread_mutex_t lock;	//pool lock
	pthread_cond_t notify;	//notify a thread to process task

	u32_t curThreads;				//current quantity of threads
	pthread_t threadId[MAX_THREAD];	//threads identifier
};





int CreateThread(bool_t loop_flag,pthread_t *thread_id);


bool_t tpCreate(struct threadpool *pool, u32_t threads, u32_t tasks);
bool_t tpAddTask(struct threadpool *pool, void (*task_handler)(void *), void *args);
bool_t tpDestroy(struct threadpool *pool);








#endif//__DNTP_THREADPOOL__


