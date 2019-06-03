#ifndef __LIB_SEMA__
#define __LIB_SEMA__



int sema_create(const char *path, int proj_id, int nsems);
int sema_open(const char *path, int proj_id);
unsigned short sema_member_count(int sem_id);
int sema_lock(int sem_id, int member);
int sema_unlock(int sid, int member);
int sema_remove(int sid);



#endif//__LIB_SEMA__


