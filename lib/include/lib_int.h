#ifndef	__LIB_INT__
#define	__LIB_INT__





#include "lib_type.h"
#include <unistd.h>








int IntOpen(const char *path);
bool_t IntClose(int int_fd);










/*
  On success, select() return the number of file descriptors contained in the three returned descriptor sets
  (the total number of bits that are set in readfds, writefds, exceptfds)
  which may be zero if the timeout expires before anything interesting happens.
  On error, -1 is returned, and errno is set appropriately; the sets and timeout become undefined, 
  so do not rely on their contents after an error.
*/
#define	IntSelect(nfds, readfds, writefds, exceptfds, timeout)	select(nfds, readfds, writefds, exceptfds, timeout)










/*
  On  success, the number of bytes read is returned (zero indicates end of file), 
  and the file position is advanced by this number.  It is not an error if
  this number is smaller than the number of bytes requested; 
  this may happen for example because fewer bytes are actually available right now (maybe  because
  we  were  close to end-of-file, or because we are reading from a pipe, or from a terminal), 
  or because IntRead() was interrupted by a signal.  
  On error, -1 is returned
*/
#define	IntRead(fd, buf, count)	read(fd, buf, count)













#endif//__LIB_INT__

