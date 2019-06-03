#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "lib_sema.h"
#include "lib_dbg.h"


#define	RESOURCE_MAX	 1 /* Initial value of all semaphores */
#define	SEMMSL			32 /* the maximum number of members in semaphore set */


union semun {
	int				val;		/* Value for SETVAL */
	struct semid_ds	*buf;		/* Buffer for IPC_STAT, IPC_SET */
	unsigned short	*array;		/* Array for GETALL, SETALL */
	struct seminfo	*__buf;		/* Buffer for IPC_INFO (Linux-specific) */
};


/*
  create semaphore set.
  
  @path		an existing, accessible file 
  @proj_id	the least significant 8 bits must be nonzero
  @nsems	the number of members in semaphore set
  
  if successful, the return value will be the semaphore set identifier (a nonnegative 
  integer), otherwise -1 is returned.
*/
int sema_create(const char *path, int proj_id, int nsems)
{
	int sem_id;
	key_t key;
	int cntr;
	union semun semopt;
	
	if(nsems > SEMMSL || nsems <= 0)
	{
		return -1;
	}
	
	/* Create unique key via call to ftok() */
	key = ftok(path, proj_id);
	if(-1 == key)
	{
		return -1;
	}
	
	sem_id = semget(key, nsems, IPC_CREAT| IPC_EXCL |0666);
	if(sem_id >= 0){
		semopt.val = RESOURCE_MAX;
		for(cntr=0; cntr<nsems; cntr++)
		{
			if(-1 == semctl(sem_id, cntr, SETVAL, semopt) )
			{
				return -1;
			}
		}
	}else if(errno == EEXIST){
		sem_id = semget(key, 0, 0666);
		if(-1 == sem_id)
		{
			return -1;
		}
	}else{
		return -1;
	}
	
	
	return sem_id;
}


/*
  open semaphore set.
  
  @path		an existing, accessible file 
  @proj_id	the least significant 8 bits must be nonzero
  
  if successful, the return value will be the semaphore set identifier (a nonnegative 
  integer), otherwise -1 is returned.
*/
int sema_open(const char *path, int proj_id)
{
	int sem_id;
	key_t key;
	
	/* Create unique key via call to ftok() */
	key = ftok(path, proj_id);
	if(-1 == key)
	{
		return -1;
	}
	
	sem_id = semget(key, 0, 0666);
	if(-1 == sem_id)
	{
		return -1;
	}
	
	return sem_id;
}


/*
  calculate the number of members in the semaphore set.
  
  @sem_id	the semaphore set identifier (a nonnegative integer)

  if successful,the return value will be the number of members in semaphore set,
  otherwise 0 is returned.
*/
unsigned short sema_member_count(int sem_id)
{
	struct semid_ds semds;
	
	if(-1 == semctl(sem_id, 0, IPC_STAT, &semds))
	{
		return 0;
	}
	//return number of members in the semaphore set
	return(semds.sem_nsems);
}


/*
  lock resource.
  
  @sem_id	the semaphore set identifier (a nonnegative integer)
  @member	the member identifier in semaphore set

  if successful,1 will be returned,otherwise -1 or 0 will be returned.
*/
int sema_lock(int sem_id, int member)
{
	//通常为flag, SEM_UNDO,使操作系统跟踪信号，进程正常或者异常退出时归还信号量值，防止死锁
	struct sembuf sema_lock = { 0, -1, SEM_UNDO};
	
	/*if( member<0 || member>(sema_member_count(sem_id)-1) )
	{
		return -1;
	}*/
	
	sema_lock.sem_num = member;
	if(-1 == semop(sem_id, &sema_lock, 1))
	{
		perror("semop");
		return 0;
	}
	
	return 1;
}


/*
  unlock resource.
  
  @sem_id	the semaphore set identifier (a nonnegative integer)
  @member	the member identifier in semaphore set

  if successful,1 will be returned,otherwise -3,-2,-1 or 0 will be returned.
*/
int sema_unlock(int sid, int member)
{
	int semval;
	struct sembuf sema_unlock={ 0, 1, SEM_UNDO};
	
	/*if( member<0 || member>(sema_member_count(sid)-1) )
	{
		return -3;
	}*/
		
	semval = semctl(sid, member, GETVAL, 0);
	if(-1 == semval)
	{
		perror("semctl");
		return -2;
	}
	if(RESOURCE_MAX == semval)
	{
		//semaphore is unlocked
		return -1;
	}
	
	sema_unlock.sem_num = member;
	if( -1 == semop(sid, &sema_unlock, 1) )
	{
		perror("semop");
		return 0;
	}
	
	return 1;
}


/*
  remove semaphore set.

  @sid	the semaphore set identifier (a nonnegative integer)

  if successful,1 will be returned; otherwise 0 will be returned.
*/
int sema_remove(int sid)
{
	if(-1 == semctl(sid, 0, IPC_RMID, 0) )
	{
		return 0;
	}
	return 1;
}


