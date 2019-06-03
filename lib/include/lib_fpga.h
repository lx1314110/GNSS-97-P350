#ifndef	__LIB_FPGA__
#define	__LIB_FPGA__



#include "lib_type.h"





int FpgaOpen(const char *path);
bool_t FpgaClose(int fpga_fd);
bool_t FpgaRead( int fpga_fd, const u32_t addr, u16_t *value );
bool_t FpgaWrite( int fpga_fd, const u32_t addr, u16_t value );



#endif//__LIB_FPGA__


