#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "lib_int.h"




/*
  open INT device.
  
  @path	INT device identifier

  if successful, the INT device file descriptor will be returned; 
  otherwise -1 will be returned.
*/
int IntOpen(const char *path)
{
	int int_fd = -1;

	int_fd = open( path, O_RDWR);
	if( int_fd < 0 )
	{
		return(-1);
	}
	else
	{
		return(int_fd);
	}
}



/*
  close INT device.
  
  @int_fd	INT device file descriptor

  if successful, true will be returned; otherwise false will be returned.
*/
bool_t IntClose(int int_fd)
{
	return( (0 == close(int_fd))? true : false );
}






