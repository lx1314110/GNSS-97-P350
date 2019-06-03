#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>



#include "lib_fpga.h"
#include "lib_common.h"
#include "lib_dbg.h"
#include "lib_int.h"

#include "addr.h"
#include "alloc.h"
#include "dout_tod.h"
#include "dout_ctrl.h"
#include "dout_global.h"

#include "dout_time.h"
#include "dout_context.h"
#include "common.h"


#define	SEC1900_1970	0x83aa7e80 /*2208988800*/
#define	SEC1900_2018  	0xDDF3F880 /*3723753600*/
#define	SEC1900_2036  	0xFFFFFFFF /*~~*/



int write_tod_freq(struct outCtx *ctx)
{
	u16_t pf1=0, pf2=0;
	double pff=0.0;
	u16_t pf_int=0, pf_fra=0;
	u16_t tmp_int=0, tmp_fra=0;

	if(!FpgaRead(ctx->fpga_fd, FPGA_1ST_PWR_FREQ, &pf1))//读频率
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to read frequency of first power.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_2ND_PWR_FREQ, &pf2))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to read frequency of second power.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}

	print(	DBG_DEBUG, 
			"--pf1:%d pf2:%d", 
			pf1, pf2 );

	/*
	  10000 / Hz
	*/
	if(pf1 >= 1250 && pf1 <= 2500)
	{
		pff = (double)pf1;
	}
	else if(pf2 >= 1250 && pf2 <= 2500)
	{
		pff = (double)pf2;
	}
	else
	{
		#if 0
		print(	DBG_ERROR, 
				"<%s>--Invalid frequency.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		#endif
		return 0;
	}

	pff = 100000.000/pff;
	print(	DBG_DEBUG, 
			"--pff:%f", 
			pff );

	pf_int = (int)pff;
	pf_fra = ((int)(1000*pff))%1000;

	tmp_int = decimal2bcd(pf_int);
	tmp_fra = decimal2bcd((pf_fra/100) &0x00FF);
	tmp_fra <<= 8;
	tmp_fra |= decimal2bcd((pf_fra%100) &0x00FF);
	print(	DBG_DEBUG, 
			"--%02x.%03x", 
			tmp_int, tmp_fra );

	if(!FpgaWrite(ctx->fpga_fd, FPGA_PWR_FREQ_INT, tmp_int))
	{
		print(	DBG_ERROR, 
				"--Failed to write integral part of frequency.");

		return 0;
	}

	if(!FpgaWrite(ctx->fpga_fd, FPGA_PWR_FREQ_FRA, tmp_fra))
	{
		print(	DBG_ERROR, 
				"--Failed to write fractional part of frequency.");

		return 0;
	}

	return 1;
}

#if 0

static void  time_src_date_to_days( struct srcinfo * time)
{
	time->timeSrc.days = DateToDays(time->timeSrc.year, time->timeSrc.month, time->timeSrc.day);
}


static void time_src_days_to_date(struct srcinfo * time)
{
	unsigned short date[2];
	
	DayToDate(time->timeSrc.year,time->timeSrc.days,date);
	time->timeSrc.month = date[0];
	time->timeSrc.day = date[1];
}

static void time_src_add_hour(struct srcinfo * time , int hour)
{
	AddHourYmd(&time->timeSrc, hour);
	time_src_date_to_days(time);

}


static void time_src_sub_hour(struct srcinfo * time , int hour)
{
	SubHourYmd(&time->timeSrc, hour);
	time_src_date_to_days(time);
}



static void time_src_add_second(struct srcinfo * time , int  second)
{
	AddSecondYmd(&time->timeSrc, second);
	time_src_date_to_days(time);
}

static void time_src_sub_second(struct srcinfo * time , int  second)
{
	SubSecondYmd(&time->timeSrc, second);
	time_src_date_to_days(time);
}



static  void time_zone_convert(const struct srcinfo * src_time, struct srcinfo * out_time)
{
	int zone_off;
	
	zone_off = gTzTbl[out_time->time_zone - '@'] - gTzTbl[src_time->time_zone - '@'];
	out_time->timeSrc = src_time->timeSrc;
	if(0 != zone_off)
	{
		if(zone_off < 0){
			time_src_sub_hour(out_time, -zone_off);
		}
		else{
			time_src_add_hour(out_time,zone_off);
		}
	}
}



static void time_gps_to_utc(struct srcinfo *out_time , struct srcinfo *src_time, int leap)
{

	*out_time = *src_time;
	if (leap > 0){
		time_src_sub_second(out_time,leap);

	}else{
		time_src_add_second(out_time,-leap);
	}

	// 0 zone
	out_time->time_zone = '@';

}


static void time_utc_to_gps(struct srcinfo *out_time ,const struct srcinfo *src_time, int leap)
{
	out_time->time_zone = '@';
	time_zone_convert(src_time, out_time);

	if (leap > 0){
		time_src_add_second(out_time,leap);

	}else{
		time_src_sub_second(out_time,-leap);
	}


}
#endif







#if 0
int read_gb_solt1(int fpga_fd, struct srcinfo *src_time, int leap)
{
	u16_t value;
	struct srcinfo gb_time;
	
	if(!FpgaWrite(fpga_fd, FPGA_S01_LOCK_TIME, 0x0000))//时间锁存
	{
		return 1;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S01_LOCK_TIME, 0x0001))//时间锁存
	{
		return 1;
	}
	
	if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_SECOND, &value))
	{
		return 1;
	}
	gb_time.timeSrc.second = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_MINUTE, &value))
	{
		return 1;
	}
	gb_time.timeSrc.minute = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_HOUR, &value))
	{
		return 1;
	}
	gb_time.timeSrc.hour = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_DAY, &value))
	{
		return 1;
	}
	gb_time.timeSrc.day = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_MONTH, &value))
	{
		return 1;
	}
	gb_time.timeSrc.month = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S01_IN_GB_YEAR, &value))
	{
		return 1;
	}
	gb_time.timeSrc.year = 2000 + bcd2decimal(value &0x00FF);
	/*
	if(!FpgaRead(fpga_fd, FPGA_S01_HIGH_MICO_SEC, &value))
	{
		return 1;
	}
	gb_time.timeSrc.MiliSecond = value;
	if(!FpgaRead(fpga_fd, FPGA_S01_LOW_MICO_SEC, &value))
	{
		return 1;
	}
	gb_time.timeSrc.MicroSecond = value;
	*/
	time_gps_to_utc(src_time,&gb_time,0);

	
	
	print(	DBG_INFORMATIONAL, 
			"<%s>-GPS01-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
			gDaemonTbl[DAEMON_CURSOR_OUTPUT],
			src_time->timeSrc.year,
			src_time->timeSrc.month,
			src_time->timeSrc.day,
			src_time->timeSrc.hour,
			src_time->timeSrc.minute,
			src_time->timeSrc.second,		
			src_time->timeSrc.days,
			src_time->time_zone - '@');
					

	
	return 0;
}


int read_gb_solt2(int fpga_fd, struct srcinfo *src_time, int leap)
{
	u16_t value;
	struct srcinfo gb_time;

	if(!FpgaWrite(fpga_fd, FPGA_S02_LOCK_TIME, 0x0000))//时间锁存
	{
		return 1;
	}
	
	if(!FpgaWrite(fpga_fd, FPGA_S02_LOCK_TIME, 0x0001))//时间锁存
	{
		return 1;
	}
	
	if(!FpgaRead(fpga_fd, FPGA_S02_IN_GB_SECOND, &value))
	{
		return 1;
	}
	gb_time.timeSrc.second = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S02_IN_GB_MINUTE, &value))
	{
		return 1;
	}
	gb_time.timeSrc.minute = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S02_IN_GB_HOUR, &value))
	{
		return 1;
	}
	gb_time.timeSrc.hour = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S02_IN_GB_DAY, &value))
	{
		return 1;
	}
	gb_time.timeSrc.day = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S02_IN_GB_MONTH, &value))
	{
		return 1;
	}
	gb_time.timeSrc.month = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, FPGA_S02_IN_GB_YEAR, &value))
	{
		return 1;
	}
	gb_time.timeSrc.year = 2000 + bcd2decimal(value &0x00FF);
	
	if(!FpgaRead(fpga_fd, FPGA_S02_HIGH_MICO_SEC, &value))
	{
		return 1;
	}
	gb_time.timeSrc.MiliSecond = value;
	if(!FpgaRead(fpga_fd, FPGA_S02_LOW_MICO_SEC, &value))
	{
		return 1;
	}
	gb_time.timeSrc.MicroSecond = value;
	time_gps_to_utc(src_time,&gb_time,0);

	
	
	print(	DBG_INFORMATIONAL, 
			"<%s>-GPS01-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
			gDaemonTbl[DAEMON_CURSOR_OUTPUT],
			src_time->timeSrc.year,
			src_time->timeSrc.month,
			src_time->timeSrc.day,
			src_time->timeSrc.hour,
			src_time->timeSrc.minute,
			src_time->timeSrc.second,		
			src_time->timeSrc.days,
			src_time->time_zone - '@');					
					
	
	return 0;
}
#endif




#if 0
int read_irigb(const int fpga_fd, const unsigned int base_addr,struct srcinfo *ti)
{
	unsigned short  value;
	unsigned int addr;

	addr = base_addr;
	
	if(!FpgaRead(fpga_fd, addr, &value))
	{
		return 1;
	}
	ti->timeSrc.second = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, (addr+2), &value))
	{
		return 1;
	}
	ti->timeSrc.minute = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, (addr+4), &value))
	{
		return 1;
	}
	ti->timeSrc.hour = bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, (addr+6), &value))
	{
		return 1;
	}
	ti->timeSrc.days = 100*bcd2decimal((value>>8) &0x00FF);
	ti->timeSrc.days += bcd2decimal(value &0x00FF);

	if(!FpgaRead(fpga_fd, (addr+8), &value))
	{
		return 1;
	}
	ti->timeSrc.year = 2000 + bcd2decimal(value &0x00FF);

	time_src_days_to_date(ti);
	return 0;
}
#endif
#if 0
int read_irigb1_solt1(int fpga_fd, struct srcinfo *src_time, unsigned char in_zone)
{	
	int rt;
	rt = read_irigb(fpga_fd, FPGA_S01_IN_IRIGB1_SECOND, src_time);
	src_time->time_zone = in_zone;
	print(	DBG_INFORMATIONAL, 
		"<%s>-IRIGB1_SOLT1-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		src_time->timeSrc.year,
		src_time->timeSrc.month,
		src_time->timeSrc.day,
		src_time->timeSrc.hour,
		src_time->timeSrc.minute,
		src_time->timeSrc.second,		
		src_time->timeSrc.days,
		src_time->time_zone - '@');
	return rt;
}

int read_irigb2_solt1(int fpga_fd, struct srcinfo *src_time, unsigned char in_zone)
{	
	int rt;
	rt = read_irigb(fpga_fd, FPGA_S01_IN_IRIGB2_SECOND, src_time);
	src_time->time_zone = in_zone;
	print(	DBG_INFORMATIONAL, 
		"<%s>-IRIGB2_SOLT1-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		src_time->timeSrc.year,
		src_time->timeSrc.month,
		src_time->timeSrc.day,
		src_time->timeSrc.hour,
		src_time->timeSrc.minute,
		src_time->timeSrc.second,		
		src_time->timeSrc.days,
		src_time->time_zone - '@');
	return rt;
}
#endif
#if 0
int read_irigb1_solt2(int fpga_fd, struct srcinfo *src_time, unsigned char in_zone)
{	
	int rt;
	rt = read_irigb(fpga_fd, FPGA_S02_IN_IRIGB1_SECOND, src_time);
	src_time->time_zone = in_zone;
	print(	DBG_INFORMATIONAL, 
		"<%s>-IRIGB1_SOLT2-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		src_time->timeSrc.year,
		src_time->timeSrc.month,
		src_time->timeSrc.day,
		src_time->timeSrc.hour,
		src_time->timeSrc.minute,
		src_time->timeSrc.second,		
		src_time->timeSrc.days,
		src_time->time_zone - '@');
	return rt;
}
int read_irigb2_solt2(int fpga_fd, struct srcinfo *src_time, unsigned char in_zone)
{	
	int rt;
	rt = read_irigb(fpga_fd, FPGA_S02_IN_IRIGB2_SECOND, src_time);
	src_time->time_zone = in_zone;
	print(	DBG_INFORMATIONAL, 
		"<%s>-IRIGB2_SOLT2-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		src_time->timeSrc.year,
		src_time->timeSrc.month,
		src_time->timeSrc.day,
		src_time->timeSrc.hour,
		src_time->timeSrc.minute,
		src_time->timeSrc.second,		
		src_time->timeSrc.days,
		src_time->time_zone - '@');
	return rt;
}
#endif

#if 0
int read_ptp_solt1(int fpga_fd, struct srcinfo *src_time, int leap)
{
	unsigned int week_sec;
	unsigned short week_sec_low;
	unsigned short week_sec_high;
	unsigned short week;
	struct srcinfo time;

	
	//long  days;
	GPSTime gps_time;
	if(!FpgaRead(fpga_fd, FPGA_S01_IN_PTP_WEEK_SEC_LOW16, &week_sec_low))
	{
		return 1;
	}
	if(!FpgaRead(fpga_fd, FPGA_S01_IN_PTP_WEEK_SEC_HIGH16, &week_sec_high))
	{
		return 1;
	}
	week_sec = (week_sec_high << 16)|week_sec_low;
	
	if(!FpgaRead(fpga_fd, FPGA_S01_IN_PTP_WEEK, &week))
	{
		return 1;
	}
	print(DBG_DEBUG, 
			  "<%s>--sec high is %x sec sec_low is %x week_sec is %x .", 
			  gDaemonTbl[DAEMON_CURSOR_OUTPUT],week_sec_high,week_sec_low,week_sec);
	
	gps_time.gps_tow.gps_sow = week_sec;
	gps_time.gps_tow.gps_ds = 0;
	gps_time.gps_week = week;
	
	GPSTimeToCommonTime(&gps_time,&time.timeSrc);
	time_gps_to_utc(src_time,&time,leap);

	print(	DBG_INFORMATIONAL, 
		"<%s>-PTP01-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		src_time->timeSrc.year,
		src_time->timeSrc.month,
		src_time->timeSrc.day,
		src_time->timeSrc.hour,
		src_time->timeSrc.minute,
		src_time->timeSrc.second,		
		src_time->timeSrc.days,
		src_time->time_zone - '@');
	return 0;
	
}
#endif

#if 0

#define GROUP_NO	4
#define GROUP_1 	(unsigned short *)0x1000
#define GROUP_2		(unsigned short *)0x1100
#define GROUP_3 	(unsigned short *)0x1200
#define GROUP_4 	(unsigned short *)0x1300

#define IRIGB_POS	0

#define TOD_POS		8



unsigned short * group_tab[GROUP_NO]= {GROUP_1,GROUP_2,GROUP_3,GROUP_4};
//unsigned short * tod_fpga_array[] = {};





int time_out_irigb(unsigned short * base,int fpga_fd, struct timeinfo *ti)
{
	u16_t value;
	unsigned short * group_base;
	group_base = base + IRIGB_POS;
	value = decimal2bcd(ti->second);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(ti->minute);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(ti->hour);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(((ti->days)/100) &0x00FF);
	value <<= 8;
	value |= decimal2bcd(((ti->days)%100) &0x00FF);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd((ti->year -2000) &0x00FF);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base, value))
	{
		return 0;
	}

	return 1;
}

int time_out_tod(unsigned short * base,int fpga_fd, struct timeinfo *ti)
{

	u16_t value;
	unsigned short * group_base;
	group_base = base + TOD_POS;
	
	value = decimal2bcd(ti->second);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(ti->minute);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(ti->hour);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(ti->day);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(ti->month);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base++, value))
	{
		return 0;
	}

	value = decimal2bcd(((ti->year)/100) &0x00FF);
	value <<= 8;
	value |= decimal2bcd(((ti->year)%100) &0x00FF);
	if(!FpgaWrite(fpga_fd, (unsigned int )group_base, value))
	{
		return 0;
	}

	return 1;

}

int time_output_ptp(int fpga_fd,const struct srcinfo * src_time , int leap)
{
	/*GPS*/
	struct srcinfo out_time;
	GPSTime gpst;
	time_utc_to_gps(&out_time,src_time,leap);
	time_src_add_second(&out_time, COMPENSATION_PTP_TIME);

	print(	DBG_DEBUG, 
	"<%s>-Ptptime-%04d-%02d-%02d %02d:%02d:%02d\n.", 
	gDaemonTbl[DAEMON_CURSOR_OUTPUT],
	out_time.timeSrc.year,
	out_time.timeSrc.month,
	out_time.timeSrc.day,
	out_time.timeSrc.hour,
	out_time.timeSrc.minute,
	out_time.timeSrc.second);
	
	memset(&gpst, 0, sizeof(GPSTime));
	CommonTimeToGPSTime(&out_time.timeSrc, &gpst);
	if(!FpgaWrite(fpga_fd, FPGA_PTP_GPS_WEEK, gpst.gps_week))
	{
		return 0;
	}

	if(!FpgaWrite(fpga_fd, FPGA_PTP_GPS_SOW_LOW16, (gpst.gps_tow.gps_sow)&0x0000FFFF))
	{
		return 0;
	}

	if(!FpgaWrite(fpga_fd, FPGA_PTP_GPS_SOW_HIGH16, ((gpst.gps_tow.gps_sow) >> 16)&0x0000FFFF))
	{
		return 0;
	}

	return 1;
}
#endif

#if 0
int time_output_board(struct outCtx *ctx)
{
	struct srcinfo out_time;
	int i;

	/*group 1-4Êä³ö*/
	for ( i = 0; i < GROUP_NO; i++){
		memset(&out_time,0,sizeof(struct srcinfo));
		out_time.time_zone = ctx->out_zone.zone[i];
		time_zone_convert(&ctx->sys_time,&out_time);
		time_src_add_second(&out_time, COMPENSATION_TIME);
		time_out_irigb(group_tab[i],ctx->fpga_fd,&out_time.timeSrc);
		time_out_tod(group_tab[i],ctx->fpga_fd,&out_time.timeSrc);
	}

	time_output_ptp(ctx->fpga_fd, &ctx->sys_time, ctx->lp.leapSecond);
	return 0;
}
#endif
#if 0
int get_time_source(struct outCtx *ctx)
{
	int rt = 0;
	//get time source 
	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_INSRC_SELECT, &ctx->new_ref_source))
	{
		return 1;
	}
	
	switch(ctx->new_ref_source){
	case GPS_SOURCE_SOLT1:
		rt= read_gb_solt1(ctx->fpga_fd, &ctx->src_time, ctx->lp.leapSecond);
	break;
	case S2MH_SOURCE_SOLT1:
		;
	break;
	case S2MB_SOURCE_SOLT1:
		;
	break;
	case IRIGB1_SOURCE_SOLT1:
		read_irigb1_solt1(ctx->fpga_fd, &ctx->src_time, ctx->in_zone.zone[0]);
	break;
	case IRIGB2_SOURCE_SOLT1:
		read_irigb2_solt1(ctx->fpga_fd, &ctx->src_time, ctx->in_zone.zone[1]);
	break;
	case GPS_SOURCE_SOLT2:
		rt= read_gb_solt2(ctx->fpga_fd, &ctx->src_time, ctx->lp.leapSecond);
	break;
	case S2MH_SOURCE_SOLT2:
		;
	break;
	case S2MB_SOURCE_SOLT2:
		;
	break;
	case IRIGB1_SOURCE_SOLT2:
		read_irigb1_solt2(ctx->fpga_fd, &ctx->src_time, ctx->in_zone.zone[2]);
	break;
	case IRIGB2_SOURCE_SOLT2:
		read_irigb2_solt2(ctx->fpga_fd, &ctx->src_time, ctx->in_zone.zone[3]);
	break;
	case PTP_SOURCE_SOLT1:
		 read_ptp_solt1(ctx->fpga_fd, &ctx->src_time, ctx->lp.leapSecond);
		;
	break;
	case NO_SOURCE:
		rt= read_gb_solt1(ctx->fpga_fd, &ctx->src_time, ctx->lp.leapSecond);
		;
	break;
	default:
		
		break;
	}	
	return rt;
}
#endif


#if 0
int time_set_sysytem(struct srcinfo * sys_time)
{
		/*GPS*/
	struct srcinfo out_time;
	memset(&out_time,0,sizeof(struct srcinfo));
	out_time.time_zone = 'H';
	time_zone_convert(sys_time,&out_time);
			
	//time_src_add_second(&out_time, COMPENSATION_SYS_TIME);
	SetSysTime(&out_time.timeSrc);
	return 0;

}
#endif
/*
0-success
-1-errror
*/
int set_systime(struct outCtx *ctx)
{
	int ret = 0;
	struct timeval tv;	
	u16_t high_sec,med_sec,low_sec;

	u64_t tv_tmp_sec = 0;
	static u64_t pre_tv_sec = 0;

	//u8_t SYS_TimeZone;

	//SYS_TimeZone = ctx->out_zone.zone[0] - '@';
	//print(DBG_DEBUG, "set_systime TimeZone:%d\n", SYS_TimeZone);
		
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_LOCK_TIME, 0x0000))//时间锁存
	{
		return -1;
	}

	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_LOCK_TIME, 0x0001))//时间锁存
	{
		return -1;
	}
	
	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_HIGH_SECOND, &high_sec))
	{
		return -1;
	}
	
	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_MED_SECOND, &med_sec))
	{
		return -1;
	}
		
	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_LOW_SECOND, &low_sec))
	{
		return -1;
	}
	
	//tv.tv_sec =(u32_t) ((((u64_t) high_sec<<32) | (med_sec<<16) | low_sec) + SYS_TimeZone*3600 - SEC1900_1970);
	//check time
	tv_tmp_sec = ((u64_t)high_sec<<32) | ((u64_t)med_sec<<16) | (u64_t)low_sec;
	if (SEC1900_2018 > tv_tmp_sec || SEC1900_2036 < tv_tmp_sec || pre_tv_sec > tv_tmp_sec){

			print(	DBG_ERROR, 
				"<%s>--last-sec(0x%08llx), cur-sec(0x%08llx) time is not updated!",gDaemonTbl[DAEMON_CURSOR_OUTPUT],
				pre_tv_sec, tv_tmp_sec);
			ret = -1;
	}else{
			
			pre_tv_sec = tv_tmp_sec;//save last trued time
			//tv.tv_sec =(u32_t) ((((u64_t) high_sec<<32) | (med_sec<<16) | low_sec) - SEC1900_1970);
			tv.tv_sec = (time_t) (tv_tmp_sec - SEC1900_1970);
			//print(DBG_DEBUG, "tv.tv_sec:%lx",tv.tv_sec);
			tv.tv_usec = 0;//us

			if(0 != settimeofday(&tv, NULL)){
				return -1;
			}
			print(	DBG_INFORMATIONAL,
				"<%s>--last-sec(0x%08llx), cur-sec(0x%08llx) time is updated!",gDaemonTbl[DAEMON_CURSOR_OUTPUT],
				pre_tv_sec, tv_tmp_sec);
			ret = 0;
			//ok
	}
	return ret;
}

#if 0
#define TIME_PERIOD 30

int check_time_src(struct outCtx *ctx)
{
	ctx->sys_time = ctx->src_time;
	/*
	static int timer = 0;
	static struct srcinfo last_time;

	time_src_add_second(&ctx->sys_time,1);	
	if (memcmp(&ctx->src_time,&ctx->sys_time,sizeof(struct srcinfo)) == 0 ){
		//p350 run ok
		;
	}else{
		time_src_add_second(&last_time,1);
		print(	DBG_INFORMATIONAL, 
		"<%s>-Lasttime-%04d-%02d-%02d %02d:%02d:%02d %02d %02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		last_time.timeSrc.year,
		last_time.timeSrc.month,
		last_time.timeSrc.day,
		last_time.timeSrc.hour,
		last_time.timeSrc.minute,
		last_time.timeSrc.second,
		last_time.timeSrc.days,
		last_time.time_zone);
		if (memcmp(&ctx->src_time, &last_time, sizeof(struct srcinfo)) == 0 ){
			timer++;
			//printf("get sys_time form src_time %d\n",timer);
			if (timer == 30){
				ctx->sys_time = ctx->src_time;
				printf(" get sys_time form src_time \n");
			}
			if (timer > 32){
				timer = 32;
			}
		}else{
			//printf("sys_time comp err\n");
			last_time = ctx->src_time;
			timer = 0;	
		}
	}
	print(	DBG_INFORMATIONAL, 
		"<%s>-Systime-%04d-%02d-%02d %02d:%02d:%02d\n.", 
		gDaemonTbl[DAEMON_CURSOR_OUTPUT],
		ctx->sys_time.timeSrc.year,
		ctx->sys_time.timeSrc.month,
		ctx->sys_time.timeSrc.day,
		ctx->sys_time.timeSrc.hour,
		ctx->sys_time.timeSrc.minute,
		ctx->sys_time.timeSrc.second);
		*/
	return 0;
}
#endif

void * thread_start_routine(void *args)
{
	u8_t i;
	int ret = 0;
	int timeout = 10;//10s
	fd_set rds;
	u16_t clock_state, pre_clock_state = 0;
	u8_t enable_update_time_flag = 0;
	struct outCtx *ctx;

	ctx = (struct outCtx *)args;

	//ignore
	for(i=0; i<3; i++)
	{
		FD_ZERO(&rds);
		FD_SET(ctx->int_fd, &rds);
		select(ctx->int_fd +1, &rds, NULL, NULL, NULL);
	}

	while(isRunning(ctx))
	{
		FD_ZERO(&rds);
		FD_SET(ctx->int_fd, &rds);
		if(0 < select(ctx->int_fd +1, &rds, NULL, NULL, NULL))
		{
			if(FD_ISSET(ctx->int_fd, &rds))
			{
				print(DBG_DEBUG, 
			  "--out interrupt. timeout:%d",  timeout);
				//get_time_source(ctx);
				//check_time_src(ctx);
				//time_output_board(ctx);
				//time_set_sysytem(&ctx->sys_time);
				//first 10, else 60
				INIT_EVNTFLAG_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SYSTIME_UPDATE, SID_SYS_SLOT, SID_SYS_PORT, 0);
				if(timeout-- == 0){
					if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_STA, &clock_state)){
						print(DBG_ERROR, "--Failed to read clock state.");
						ret = __LINE__;
						break;
					}

					/*注意：
					*由快捕或者锁定状态调到保持，仍然更新时间；
					*但由自由运行，不经过快捕或锁定的情况，直接跳到保持，系统时间不会被更新*/
					//print(DBG_DEBUG, 
			 		//		 "clock_state:%d pre_clock_state:%d\n", clock_state,pre_clock_state);
					if(clock_state >= 3 && pre_clock_state >= 3){
						enable_update_time_flag = 1;//连续1分钟为快捕或者锁定，更新系统时间
					}else if (clock_state == 0 && pre_clock_state == 0){
						enable_update_time_flag = 0;//连续1分钟为自由运行，不更新系统时间
					}
					pre_clock_state = clock_state;
					print(DBG_DEBUG, 
			 				 "enable_update_time_flag:%d\n",  enable_update_time_flag);
					if(1 == enable_update_time_flag && 0 == set_systime(ctx)){
						timeout = 3600;//时间更新后，以后每一个小时更新一次
						//event
						SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SYSTIME_UPDATE, SID_SYS_SLOT, SID_SYS_PORT);
					}else{
						//自由运行，不具备更新时间的条件时，每1分钟检测一次钟
						timeout = 60;//60s update time
					}
				}
				#ifdef CHINA_MIIT_TOD_RULE
				china_mobile_miit_tod(ctx);
				#endif
				//usleep(60000);
				write_tod_freq(ctx);
				if(0 == WriteCurEventToTable(ctx->pDb, pEvntInxTbl, max_evntinx_num)){//把事件写入到事件表
						ret = __LINE__;
						break;
				}
			}
		}
	}

	print(DBG_NOTICE, "Thread exit clearly![%d]\n",ret);
	pthread_exit(NULL);
}


int CreateThread(struct outCtx *ctx)
{
	int retval;
	
	retval = pthread_create(&(ctx->thread_id), NULL, thread_start_routine, (void*)ctx);
	if(0 != retval)
	{
		return 0;
	}

	return 1;
}


int CloseThread(struct outCtx *ctx)
{
	int retval;
	ctx->loop_flag = false;
	retval = pthread_join(ctx->thread_id, NULL);
	if( 0 != retval )
	{
		return 0;
	}
	
	return 1;
}



