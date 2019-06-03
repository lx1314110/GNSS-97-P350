#ifndef NTP_GET_TIME_H
#define NTP_GET_TIME_H
#include "lib_type.h"
#define FPGA_TIME_SEC_OFFSET	1



//The time interval between 1900-01-01 00:00:00 and 1970-01-01 00:00:00
#define	SEC1900_1970	0x83aa7e80 /*2208988800*/      
#define	SEC1900_2011  	0xD0C8ED00 /*3502828800*/
#define	SEC1900_2012  	0xD2AA2080 /*3534364800*/
#define	SEC1900_2013  	0xD48CA580 /*3565987200*/
#define	SEC1900_2014  	0xD66DD900 /*3597523200*/
#define	SEC1900_2015  	0xD84F0C80 /*3629059200*/
#define	SEC1900_2016  	0xDA304000 /*3660595200*/
#define	SEC1900_2017  	0xDC12C500 /*3692217600*/
#define	SEC1900_2018  	0xDDF3F880 /*3723753600*/
#define	SEC1900_2019  	0xDFD52C00 /*3755289600*/
#define	SEC1900_2020  	0xE1B65F80 /*3786825600*/
#define	SEC1900_2021  	0xE398E480 /*3818448000*/
#define	SEC1900_2022  	0xE57A1800 /*3849984000*/

#define	SEC1900_2036  	0xFFFFFFFF /*~~*/

int ntpgettimefpga(int fd ,int leapsecond,struct timeval*tv, struct timezone *tz);
u8_t GetSysTimeTV(struct timeval *tv);
int ntpchecktimefpga(int fd);

#endif

