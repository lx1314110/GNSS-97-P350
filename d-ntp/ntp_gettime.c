
#include "sys/time.h"
#include <stdio.h>
#include <string.h>
#include "ntp_gettime.h"
#include "lib_time.h"
#include "addr.h"
#include "lib_fpga.h"
#include "lib_common.h"
#include "lib_dbg.h"
#include "alloc.h"
/*get time form fpga  */



u8_t GetSysTimeTV(struct timeval *tv)
{
	if(0 == gettimeofday(tv, NULL))
	{
		tv->tv_sec += SEC1900_1970;
		tv->tv_sec -= 28800;
		return 0;
	}
	else
	{
		return 1;
	}
}



#if 0
int ntpgettimefpga(int fd ,int leapsecond,struct timeval*tv, struct timezone *tz)
{
		
		u16_t value;
		int fpga_fd;
		struct timeinfo ti;
		struct timeval tv_tmp;
		u32_t tmp;
		
		fpga_fd = fd;
		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_SECOND, &value))
		{
			return -1;
		}
		ti.second = bcd2decimal(value &0x00FF);
		
		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_MINUTE, &value))
		{
			return -1;
		}
		ti.minute = bcd2decimal(value &0x00FF);
		
		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_HOUR, &value))
		{
			return -1;
		}
		ti.hour = bcd2decimal(value &0x00FF);
		
		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_DAY, &value))
		{
			return -1;
		}
		ti.day = bcd2decimal(value &0x00FF);
		
		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_MONTH, &value))
		{
			return -1;
		}
		ti.month = bcd2decimal(value &0x00FF);
		
		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_YEAR, &value))
		{
			return -1;
		}
		ti.year = 2000 + bcd2decimal(value &0x00FF);
		
		
		ti.days = 0;


		//AddHourYmd(&ti, 8);
		tmp = DateToSecond(1900, &ti);
		tv_tmp.tv_sec = tmp + FPGA_TIME_SEC_OFFSET - leapsecond;


		if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_MICO_SEC, &value))
		{
			return -1;
		}
		tv_tmp.tv_usec = value * 25;

		*tv = tv_tmp;

		print(DBG_DEBUG, "%s fpga read time  leapsecond %d %04d-%02d-%02d %02d:%02d:%02d:%lu\n", 
		gDaemonTbl[DAEMON_CURSOR_INPUT],
		leapsecond,
		ti.year,
		ti.month,
		ti.day,
		ti.hour,
		ti.minute,
		ti.second,
		tv_tmp.tv_usec);
		//tz = NULL;

		

		return 0;
}
#endif

int ntpgettimefpga(int fd ,int leapsecond,struct timeval*tv, struct timezone *tz)
{
		
		u16_t MiliSecond = 0,MicroSecond = 0,high_sec = 0,med_sec = 0,low_sec = 0;
		int fpga_fd = 0;
		struct timeval tv_tmp;
		
		fpga_fd = fd;
		
		if(!FpgaWrite(fpga_fd, FPGA_SYS_LOCK_TIME, 0x0000))//时间锁存
		{
			return -1;
		}
	
		if(!FpgaWrite(fpga_fd, FPGA_SYS_LOCK_TIME, 0x0001))//时间锁存
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_HIGH_MICO_SEC, &MiliSecond))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_LOW_MICO_SEC, &MicroSecond))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_HIGH_SECOND, &high_sec))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_MED_SECOND, &med_sec))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_LOW_SECOND, &low_sec))
		{
			return -1;
		}
		
		//tv_tmp.tv_sec = (high_sec<<32) | (med_sec<<16) | low_sec ;
		tv_tmp.tv_sec = (time_t) ((long long int)high_sec<<32) | (med_sec<<16) | low_sec ;
		//tv_tmp.tv_usec = MiliSecond*1000 + MicroSecond;//us
		tv_tmp.tv_usec = (time_t) MiliSecond*1000 + MicroSecond;
//#define DBUG_LOGGER
#ifdef DBUG_LOGGER
	struct timeval tv_tmpbf;
	tv_tmpbf.tv_sec = ((long long int)high_sec<<32) | ((time_t)med_sec<<16) | low_sec ;
	tv_tmpbf.tv_usec = (time_t) MiliSecond*1000 + MicroSecond;
	//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
#endif

#define MOVING_TIME
#ifdef MOVING_TIME
	//adjust from low to high
	if(tv_tmp.tv_usec < 130)
	{
		tv_tmp.tv_sec -= 1;
		tv_tmp.tv_usec += 999870;
		
	}
	else
	{
		tv_tmp.tv_usec -= 130;
	}
#endif

#ifdef DBUG_LOGGER
	#define LOGGER_FILE "/data/ntp.log"
			char logger[128];
			static int init_ntp_flag = 0;
			static FILE  *fp = NULL;
			//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
			if(init_ntp_flag == 0){
				fp = fopen(LOGGER_FILE,"a+");
				if(NULL == fp)
				{
					perror("fopen");
					return -1;
				}
				//4 4   8 4
				fprintf(stderr, "time_t: s:%d us:%d\n",sizeof(tv_tmp.tv_sec),sizeof(tv_tmp.tv_usec));
				fprintf(stderr, "lld:%d  ld:%d\n",sizeof(long long int),sizeof(long int));
				init_ntp_flag = 1;
				char header[] = "high_sec\tmed_sec\tlow_sec\tMiliSecond\tMicroSecond\ttv_sec_bf\ttv_sec_bf(+)\ttv_usec_bf\ttv_sec\ttv_sex(hex)\ttv_usec\n\0";
				fseek(fp, 0L, SEEK_SET);
				fwrite((void *)header,strlen(header),1,fp);
			}
			//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
			memset(logger,0x0,sizeof(logger));
			//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
			sprintf(logger, "%d\t%d\t%d\t%d\t%d\t%ld\t%u\t%ld\t%u\t%lx\t%ld\n",high_sec,med_sec,low_sec,MiliSecond,MicroSecond,
				tv_tmpbf.tv_sec,tv_tmpbf.tv_sec,tv_tmpbf.tv_usec,tv_tmp.tv_sec,tv_tmp.tv_sec,tv_tmp.tv_usec);
			//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
			if (fwrite(logger,strlen(logger),1,fp) != 1){
				fprintf(stderr, "fwrite error!\n");
			}
			//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
			fflush(fp);
			//fprintf(stderr, "%s:%d\n",__func__,__LINE__);
#endif

		*tv = tv_tmp;
		return 0;
}

int ntpchecktimefpga(int fd)
{
		int ret = 0;
		u16_t MiliSecond = 0,MicroSecond = 0;
		u16_t high_sec = 0,med_sec = 0,low_sec = 0;
		int fpga_fd = 0;
		u64_t tv_tmp_sec = 0;
		static u64_t pre_tv_sec = 0;

		u32_t tv_tmp_usec = 0;
		static u32_t pre_tv_usec = 0;

		fpga_fd = fd;
		
		if(!FpgaWrite(fpga_fd, FPGA_SYS_LOCK_TIME, 0x0000))//鏃堕棿閿佸瓨
		{
			return -1;
		}
	
		if(!FpgaWrite(fpga_fd, FPGA_SYS_LOCK_TIME, 0x0001))//鏃堕棿閿佸瓨
		{
			return -1;
		}
		#if 1
		if(!FpgaRead(fpga_fd, FPGA_SYS_HIGH_MICO_SEC, &MiliSecond))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_LOW_MICO_SEC, &MicroSecond))
		{
			return -1;
		}
		#endif
		if(!FpgaRead(fpga_fd, FPGA_SYS_HIGH_SECOND, &high_sec))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_MED_SECOND, &med_sec))
		{
			return -1;
		}
		
		if(!FpgaRead(fpga_fd, FPGA_SYS_LOW_SECOND, &low_sec))
		{
			return -1;
		}
		//high_sec is 0, (0XFFFFFFFF)max 136 year (4 types) to 2036year
		tv_tmp_sec = ((u64_t)high_sec<<32) | ((u64_t)med_sec<<16) | (u64_t)low_sec ;
		tv_tmp_usec = (u32_t) MiliSecond*1000 + MicroSecond;

		#if 0 //DEBUG
		print(	DBG_ERROR, 
						"<%s>--last time value(0x%08llx), time value(0x%08llx) may fault!",gDaemonTbl[DAEMON_CURSOR_NTP],
						pre_tv_sec, tv_tmp_sec);
		#endif
		//beijing:2018-07-11 17:4?----3740291800   //SEC1900_2036 import:last-sec(0x000000b4), cur-sec(0x100010001)<==>last-us(0x3317a),cur-us(0x03e9)
		//if (SEC1900_2018 > tv_tmp_sec || pre_tv_sec > tv_tmp_sec){
		if (SEC1900_2018 > tv_tmp_sec || SEC1900_2036 < tv_tmp_sec || pre_tv_sec > tv_tmp_sec ||
			(pre_tv_sec == tv_tmp_sec && pre_tv_usec == tv_tmp_usec)){
			print(	DBG_ERROR, 
				"<%s>---",gDaemonTbl[DAEMON_CURSOR_NTP]);
			print(	DBG_ERROR, 
				"<%s>--last-sec(0x%08llx), cur-sec(0x%08llx)<==>last-us(0x%04x),cur-us(0x%04x) may fault!",gDaemonTbl[DAEMON_CURSOR_NTP],
				pre_tv_sec, tv_tmp_sec, pre_tv_usec, tv_tmp_usec);
			//print(	DBG_ERROR, 
			//	"<%s>----last-us(0x%04x),cur-us(0x%04x) may fault!",gDaemonTbl[DAEMON_CURSOR_NTP],
			//	pre_tv_usec, tv_tmp_usec);
			ret = 0;
		}else{
			print(	DBG_DEBUG,
				"<%s>--last-sec(0x%08llx), cur-sec(0x%08llx)<==>last-us(0x%04x),cur-us(0x%04x) is ok!",gDaemonTbl[DAEMON_CURSOR_NTP],
				pre_tv_sec, tv_tmp_sec, pre_tv_usec, tv_tmp_usec);
			pre_tv_sec = tv_tmp_sec;
			pre_tv_usec = tv_tmp_usec;//save last trued time
			ret = 1;//ok
		}
		return ret;
}

