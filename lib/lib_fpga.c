#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "lib_fpga.h"


/* FPGA Command */
#define FPGA_WRITE	0x00
#define FPGA_READ	0x01




/*
  open FPGA device.
  
  @path	FPGA device identifier

  if successful, the FPGA device file descriptor will be returned; 
  otherwise -1 will be returned.
*/
int FpgaOpen(const char *path)
{
	int fpga_fd = -1;

	fpga_fd = open( path, O_RDWR);
	if( fpga_fd < 0 )
	{
		return(-1);
	}
	else
	{
		return(fpga_fd);
	}
}


/*
  close FPGA device.
  
  @fpga_fd	FPGA device file descriptor

  if successful, true will be returned; otherwise false will be returned.
*/
bool_t FpgaClose(int fpga_fd)
{
	return( (0 == close(fpga_fd))? true : false );
}


/*
  read from FPGA device.
  
  @addr		address
  @value	value

  if successful, true will be returned; otherwise false will be returned.
*/
bool_t FpgaRead( int fpga_fd, const u32_t addr, u16_t *value )
{
	u32_t temp;

	temp = addr << 16;
	if( 0 < ioctl( fpga_fd, FPGA_READ, &temp ) )
	{
		*value = (u16_t)( temp & 0xFFFF );
		return (true);
	}
	else
	{
		return (false);
	}
}


/*
  write to FPGA device.
  
  @addr		address
  @value	value

  if successful, true will be returned; otherwise false will be returned.
*/
bool_t FpgaWrite( int fpga_fd, const u32_t addr, u16_t value )
{
	u32_t temp;

	temp = (addr << 16) | value;
	if ( 0 > ioctl( fpga_fd, FPGA_WRITE, &temp ) )
	{
		return (false);
	}
	else
	{
		return (true);
	}
}


