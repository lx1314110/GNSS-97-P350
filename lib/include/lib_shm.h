#ifndef	__LIB_SHM__
#define	__LIB_SHM__



int shm_create(const char *path, int proj_id);
void * shm_attach(int shmid);
void shm_write(char *base, int offset, int d_size, char *src, int s_size);
void shm_read(char *base, int offset, int s_size, char *dst, int d_size);
int shm_detach(const void * base);
int shm_removable(int shmid);


#define SHM_DETACH(a) shm_detach((void *)(a)),(a)=NULL


#endif//__LIB_SHM__


