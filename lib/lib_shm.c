#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "lib_shm.h"


#define PAGE_SIZE 	4096
//the default share memory size
//#define	SHM_SIZE	4096//(4K)
//MAX 32M, but it can increase, by echo "size" >/proc/sys/kernel/shmmax
#define	SHM_SIZE	(4UL*PAGE_SIZE)//(16K)


/*
  create or open share memory.

  @path		an existing, accessible file 
  @proj_id	the least significant 8 bits must be nonzero

  if successful, the share memory identifier(a nonnegative integer)will be returned;
  otherwise, -1 will be returned.
*/
int shm_create(const char *path, int proj_id)
{
	key_t key;
	int shm_id;

	key = ftok(path, proj_id);
	if(-1 == key)
	{
		return(-1);
	}

	shm_id = shmget(key, SHM_SIZE, IPC_CREAT|0666);
	if(-1 == shm_id)
	{
		perror("shmget");
		return(-1);
	}
	
	return shm_id;
}


/*
  attach share memory.

  @shmid	the share memory identifier(a nonnegative integer)

  on success the address of the attached shared memory segment is returned 
  on error (void *) -1 is returned
*/
void * shm_attach(int shmid)
{
	return(shmat(shmid, 0, 0));
}


/*
  write to share memory

  @base		the base address of share memory
  @offset	the offset base on base address of share memory
  @d_size	the destination size
  @src		the pointer point to data source
  @s_size	the size of data source
*/
void shm_write(char *base, int offset, int d_size, char *src, int s_size)
{
	int size;
	size = (s_size > d_size)? d_size : s_size;
	memcpy(base+offset, src, size);
}


/*
  read from share memory

  @base		the base address of share memory
  @offset	the offset base on base address of share memory
  @s_size	the size of data source
  @dst		the pointer point to destination 
  @d_size	the destination size
*/
void shm_read(char *base, int offset, int s_size, char *dst, int d_size)
{
	int size;
	size = (d_size > s_size)? s_size : d_size;
	memcpy(dst, base+offset, size);
}


/*
  detach share memory.

  @base	the address of the attached shared memory segment

  on success, 1 is returned 
  on error, 0 is returned
*/
int shm_detach(const void * base)
{
	//on success shmdt() returns 0
	//on error -1 is returned
	if(!base)
		return 0;
	if( -1 == shmdt(base) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


/*
  mark the state of share memory to removable 

  @shmid	the share memory identifier(a nonnegative integer)

  on success, 1 is returned.
  on error, 0 is returned.
*/
int shm_removable(int shmid)
{
	if(-1 == shmctl(shmid, IPC_RMID, 0) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


