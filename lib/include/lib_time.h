#ifndef	__LIB_TIME__
#define	__LIB_TIME__





#include "lib_type.h"








#define	DAY_IN_SECOND			86400L
#define	HOUR_IN_SECOND			3600
#define	MINUTE_IN_SECOND		60
#define	HOUR_IN_MINUTE			60
#define	DAY_IN_HOUR				24

#define	SEC1900_1970	0x83aa7e80







typedef struct {
	u32_t 	gps_sow;	//整数秒
	float	gps_ds;		//小数秒
} TOW;//周内秒




typedef struct {
	u32_t	gps_week;	//周数
	TOW		gps_tow;	//周内秒
} GPSTime;//GPS时




typedef struct {
	u32_t	jd_sod;	//整数秒
	float	jd_ds;	//小数秒
} TOD;//天内秒




typedef struct {
	u32_t	jd_day;
	TOD		jd_tod;
} JULIANDAY;//儒略时





typedef struct
{
	u32_t	JD_integer;//整数天
	float	JD_decimal;//小数天
}JULIANDAY2;






struct timeinfo {
	u16_t year;//年
	u16_t days;//天
	u8_t month;//月
	u8_t day;//日
	u8_t hour;//时
	u8_t minute;//分
	u8_t second;//秒
	u16_t MiliSecond;//毫秒
	u16_t MicroSecond;//微秒
};




u8_t IsLeapYear(u16_t year);
u32_t YearToDay(u16_t year);
u32_t AllYearSecond(u16_t startyear,u16_t endyear);
u32_t DateToSecond(u16_t startyear, struct timeinfo *time);
u16_t DateToDays(u16_t year,u8_t month,u8_t day);
void DayToDate(u16_t year, u16_t day, u16_t date[2]);

u8_t date_validity(char *date);
u8_t time_validity(char *time);
u8_t GetUtcSysTime(struct timeinfo *ti);
u8_t GetSysTime(struct timeinfo *ti);
u8_t SetSysTime(struct timeinfo *ti);


void AddHourYmd(struct timeinfo *ct, int add_h/*1~23*/);
void SubHourYmd(struct timeinfo *ct, int sub_h/*1~23*/);
void AddSecondYmd(struct timeinfo *ct, int add_s/*1~99*/);
void SubSecondYmd(struct timeinfo *ct, int sub_s/*1~99*/);

void AddHourYd(struct timeinfo *ct2, int add_h/*1~23*/);
void SubHourYd(struct timeinfo *ct2, int sub_h/*1~23*/);
void AddSecondYd(struct timeinfo *ct2, int add_s/*1~99*/);
void SubSecondYd(struct timeinfo *ct2, int sub_s/*1~99*/);


float frac(float origin);
void GPSTimeToJulianDay (GPSTime *gpst, JULIANDAY *jld, JULIANDAY2 *jd);
void JulianDayToCommonTime (JULIANDAY *jld, JULIANDAY2 *jd, struct timeinfo *ct);
void GPSTimeToCommonTime(GPSTime *gpst, struct timeinfo *ct);

void GpsTime2CT(GPSTime *gps_time, struct timeinfo *ct2);
void CommonTimeToGPSTime(struct timeinfo *ct, GPSTime *gpst);
void CommonTimeToJulianDay(struct timeinfo *ct, JULIANDAY2 *jd);
u16_t CommonTimeToDOY(struct timeinfo *ct);


long GpsToMjd (long GpsCycle, long GpsWeek, long GpsSeconds);

void MjdToDate (long Mjd, long *Year, long *Month, long *Day);








#endif//__LIB_TIME__


