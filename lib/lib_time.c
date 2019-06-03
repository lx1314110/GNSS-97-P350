#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "lib_time.h"










//判断year年是否是闰年 
u8_t IsLeapYear(u16_t year)
{
	u8_t LeapYear;
	LeapYear = ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0);
	return LeapYear;
}





//计算year年有多少天 
u32_t YearToDay(u16_t year)
{
	u8_t LeapYear;
	u32_t OneYear;
	
	LeapYear = ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0);
	if( LeapYear ) 
	{
		OneYear = 366;
	}
	else
	{
		OneYear = 365;
	}
	return OneYear;
}






//startyear年1月1日0点0分0秒到endyear年1月1日0点0分0秒的所有秒数
u32_t AllYearSecond(u16_t startyear,u16_t endyear)
{
	u32_t AllYear_second = 0;
	u16_t i;
	
	for( i=startyear; i<endyear; i++)
	{
		AllYear_second += YearToDay(i);
	}
	
	AllYear_second *= 86400;
   
	return AllYear_second;
}






//根据年月日时分秒计算出从startyear年1月1日0点0分0秒到现在的秒数 
u32_t DateToSecond(u16_t startyear, struct timeinfo *time)
{
	u8_t i;
	u16_t alldays = 0;
	u32_t allsecond = 0;
	u8_t mday[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}, 
						{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}	};
	
	if( ((time->year%4 == 0) && (time->year%100 != 0)) || (time->year%400 == 0) )
	{
		for(i = 1; i < time->month; i++)
	    {
	    	alldays += mday[1][i-1];
	    }
	}
	else
	{
		for(i = 1; i < time->month; i++)
	    {
			alldays += mday[0][i-1]; 
		}
	}
	alldays += time->day;

	allsecond = AllYearSecond(startyear,time->year) + ((alldays-1)*24*60*60) + (time->hour*60*60) + (time->minute*60) + time->second;
     
	return allsecond;    
}






//根据year年month月day日计算出是year年的第几天 
u16_t DateToDays(u16_t year,u8_t month,u8_t day)
{
	u8_t i;
	u16_t alldays = 0;
	u8_t mday[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}, 
						{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}	};
	
	if( ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0) )
	{
		for(i = 1; i < month; i++)
	    {
	        alldays += mday[1][i-1];
	    }
	}
	else
	{
		for(i = 1; i < month; i++)
		{
	     	alldays += mday[0][i-1]; 
		}
	}
	alldays += day;
	
	return(alldays);
}







//根据year年的第day天计算出year年的某月某日  
void DayToDate(u16_t year, u16_t day, u16_t date[2])
{
	u8_t i;
	u8_t mday[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}, 
						{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}	};
	
	if( ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0) )
	{
		for(i=0; day>mday[1][i]; i++)
		{
			day -= mday[1][i];
		}
	}
	else
	{
		for(i=0; day>mday[0][i]; i++)
		{
			day -= mday[0][i];
		}	
	}

	date[0] = i+1;
	date[1] = day;
}







/*
  0	不合法
  1 合法
*/
u8_t date_validity(char *date)
{
	int i;
	u16_t year;
	u8_t month, day;
	u8_t mday[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}, 
						{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30,31}	};
	
	if(10 == strlen(date))
	{
		if('-' == date[4] && '-' == date[7])
		{
			for(i=0; i<4; i++)
			{
				if(date[i]<'0' || date[i]>'9')
				{
					return 0;
				}
			}
			if( (date[5]>='0' && date[5]<='1') &&
				(date[6]>='0' && date[6]<='9') &&
				(date[8]>='0' && date[8]<='3') &&
				(date[9]>='0' && date[9]<='9'))
			{
				year = (date[0]-'0')*1000 + (date[1]-'0')*100 + (date[2]-'0')*10 + (date[3]-'0');
				month = (date[5]-'0')*10 + (date[6]-'0');
				day = (date[8]-'0')*10 + (date[9]-'0');

				if(month < 1 || month > 12)
				{
					return 0;
				}
				
				if(IsLeapYear(year))
				{
					if(day < 1 || day > mday[1][month-1])
					{
						return 0;
					}
				}
				else
				{
					if(day < 1 || day > mday[0][month-1])
					{
						return 0;
					}
				}
				//合法
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}








/*
  0	不合法
  1 合法
*/
u8_t time_validity(char *time)
{
	u8_t hour, minute, second;
	
	if(8 == strlen(time))
	{
		if(':' == time[2] && ':' == time[5])
		{
			if( (time[0]>='0' && time[0]<='2') &&
				(time[1]>='0' && time[1]<='9') &&
				(time[3]>='0' && time[3]<='5') &&
				(time[4]>='0' && time[4]<='9') &&
				(time[6]>='0' && time[6]<='5') &&
				(time[7]>='0' && time[7]<='9') )
			{
				hour = (time[0]-'0')*10 + (time[1]-'0');
				minute = (time[3]-'0')*10 + (time[4]-'0');
				second = (time[6]-'0')*10 + (time[7]-'0');
				if(hour < 0 || hour > 23)
				{
					return 0;
				}
				
				if(minute < 0 || minute > 59)
				{
					return 0;
				}
				
				if(second < 0 || second > 59)
				{
					return 0;
				}
				
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}



/*
  0	失败
  1	成功
*/
u8_t GetUtcSysTime(struct timeinfo *ti)
{
	time_t timeTotal = 0;
	struct tm timev;
	struct tm *timep = NULL;

	memset(&timev, 0, sizeof(struct tm));
	/* returns  the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds. */
	timeTotal = time(NULL);
	timep = gmtime_r(&timeTotal, &timev);
	if( NULL == timep )
	{
		return 0;
	}

	ti->year = timev.tm_year+1900;
	ti->month = timev.tm_mon+1;
	ti->day = timev.tm_mday;
	ti->hour = timev.tm_hour;
	ti->minute = timev.tm_min;
	ti->second = timev.tm_sec;

	return 1;
}





/*
  0	失败
  1	成功
*/
u8_t GetSysTime(struct timeinfo *ti)
{
	time_t timeTotal = 0;
	struct tm timev;
	struct tm *timep = NULL;

	memset(&timev, 0, sizeof(struct tm));
	/* returns  the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds. */
	timeTotal = time(NULL);
	timep = localtime_r(&timeTotal, &timev);
	if( NULL == timep )
	{
		return 0;
	}

	ti->year = timev.tm_year+1900;
	ti->month = timev.tm_mon+1;
	ti->day = timev.tm_mday;
	ti->hour = timev.tm_hour;
	ti->minute = timev.tm_min;
	ti->second = timev.tm_sec;

	return 1;
}







/*
  0	失败
  1	成功
*/
u8_t SetSysTime(struct timeinfo *ti)
{
	u32_t tmp=0;
	struct timeval tv;	
	
	tmp = DateToSecond(1970, ti);
	tv.tv_sec = tmp;
	tv.tv_usec = 0;
	
	if(0 == settimeofday(&tv, NULL))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}




/*处理年月日格式时间*/


void AddHourYmd(struct timeinfo *ct, int add_h/*1~23*/)
{
	if(add_h >0 && add_h <24)
	{
		ct->hour += add_h;
		if(ct->hour>23)
		{
			ct->hour %= 24;
			(ct->day)++;
		}
		if((ct->day==29)&&(ct->month==2)&&(IsLeapYear(ct->year)!=1))
		{
			ct->day=1;
			(ct->month)++;
		}
		else if((ct->day==30)&&(ct->month==2)&&(IsLeapYear(ct->year)==1))
		{
			ct->day=1;
			ct->month++;
		}
		else if(ct->day==31)
		{
			if((ct->month==4)||(ct->month==6)||(ct->month==9)||(ct->month==11))
			{
				ct->day=1;
				(ct->month)++;
			}
		}
		else if(ct->day==32)
		{
			if((ct->month==1)||(ct->month==3)||(ct->month==5)||(ct->month==7)||(ct->month==8)||(ct->month==10))
			{
				ct->day=1;
				(ct->month)++;
			}
			else if(ct->month==12)
			{
				ct->day=1;
				ct->month=1;
				ct->year++;
			}
		}
	}
}




void SubHourYmd(struct timeinfo *ct, int sub_h/*1~23*/)
{
	if(sub_h > 0 && sub_h < 24)
	{
		if(ct->hour >= sub_h)
		{
			ct->hour -= sub_h;
		}
		else
		{
			ct->hour += 24;
			ct->hour -= sub_h;
					
			if(ct->day > 1)
			{
				ct->day -= 1;
			}
			else
			{	/*1,3,5,7,8,10,12*/
				if(ct->month == 1 ||ct->month == 2 ||ct->month == 4 ||ct->month == 6 || ct->month == 8 ||ct->month == 9 ||ct->month == 11)
				{
					ct->day = 31;
					ct->month -= 1;
					if(ct->month == 0)
					{
						ct->month = 12;
						ct->year -= 1;
					}
				}
				else if(ct->month == 5 ||ct->month == 7 ||ct->month == 10 ||ct->month ==12)
				{
					ct->day = 30;
					ct->month -= 1;
				}
				else if(ct->month == 3)
				{
					if(IsLeapYear(ct->year))
					{
						ct->day = 29;
						ct->month -=1;
					}
					else
					{
						ct->day = 28;
						ct->month -= 1;
					}
				}
			}
		}
	}
}





void AddSecondYmd(struct timeinfo *ct, int add_s/*1~99*/)
{
	if(add_s >0 && add_s <100)
	{
		ct->second += add_s;
		if(ct->second > 59)
		{
			ct->minute += (ct->second /60);
			ct->second %= 60;
			
			if(ct->minute > 59)
			{
				ct->minute %= 60;
				ct->hour += 1;

				if(ct->hour>23)
				{
					ct->hour %= 24;
					(ct->day)++;
				}
				if((ct->day==29)&&(ct->month==2)&&(IsLeapYear(ct->year)!=1))
				{
					ct->day=1;
					(ct->month)++;
				}
				else if((ct->day==30)&&(ct->month==2)&&(IsLeapYear(ct->year)==1))
				{
					ct->day=1;
					ct->month++;
				}
				else if(ct->day==31)
				{
					if((ct->month==4)||(ct->month==6)||(ct->month==9)||(ct->month==11))
					{
						ct->day=1;
						(ct->month)++;
					}
				}
				else if(ct->day==32)
				{
					if((ct->month==1)||(ct->month==3)||(ct->month==5)||(ct->month==7)||(ct->month==8)||(ct->month==10))
					{
						ct->day=1;
						(ct->month)++;
					}
					else if(ct->month==12)
					{
						ct->day=1;
						ct->month=1;
						ct->year++;
					}
				}
			}
		}
	}
}






void SubSecondYmd(struct timeinfo *ct, int sub_s/*1~99*/)
{
	int min, sec;
	
	if(sub_s > 0 && sub_s < 100)
	{
		min = sub_s /60;
		sec = sub_s %60;
		if(sec <= ct->second)
		{
			ct->second -= sec;
		}
		else
		{
			ct->second += 60;
			ct->second -= sec;

			if(ct->minute >= 1)
			{
				ct->minute -= 1;	
			}
			else
			{
				ct->minute += 60;
				ct->minute -= 1;
				
				if(ct->hour >= 1)
				{
					ct->hour -= 1;
				}
				else
				{
					ct->hour += 24;
					ct->hour -= 1;
					
					if(ct->day > 1)
					{
						ct->day -= 1;
					}
					else
					{	/*1,3,5,7,8,10,12*/
						if(ct->month == 1 ||ct->month == 2 ||ct->month == 4 ||ct->month == 6 || ct->month == 8 ||ct->month == 9 ||ct->month == 11)
						{
							ct->day = 31;
							ct->month -= 1;
							if(ct->month == 0)
							{
								ct->month = 12;
								ct->year -= 1;
							}
						}
						else if(ct->month == 5 ||ct->month == 7 ||ct->month == 10 ||ct->month ==12)
						{
							ct->day = 30;
							ct->month -= 1;
						}
						else if(ct->month == 3)
						{
							if(IsLeapYear(ct->year))
							{
								ct->day = 29;
								ct->month -=1;
							}
							else
							{
								ct->day = 28;
								ct->month -= 1;
							}
						}
					}
				}
			}
		}

		if(min > 0)
		{
			if(ct->minute >= min)
			{
				ct->minute -= min;
			}
			else
			{
				ct->minute += 60;
				ct->minute -= min;

				if(ct->hour >= 1)
				{
					ct->hour -= 1;
				}
				else
				{
					ct->hour += 24;
					ct->hour -= 1;
					
					if(ct->day > 1)
					{
						ct->day -= 1;
					}
					else
					{	/*1,3,5,7,8,10,12*/
						if(ct->month == 1 ||ct->month == 2 ||ct->month == 4 ||ct->month == 6 || ct->month == 8 ||ct->month == 9 ||ct->month == 11)
						{
							ct->day = 31;
							ct->month -= 1;
							if(ct->month == 0)
							{
								ct->month = 12;
								ct->year -= 1;
							}
						}
						else if(ct->month == 5 ||ct->month == 7 ||ct->month == 10 ||ct->month ==12)
						{
							ct->day = 30;
							ct->month -= 1;
						}
						else if(ct->month == 3)
						{
							if(IsLeapYear(ct->year))
							{
								ct->day = 29;
								ct->month -=1;
							}
							else
							{
								ct->day = 28;
								ct->month -= 1;
							}
						}
					}
				}
			}
		}
	}
}




/* 处理年日格式时间*/

void AddHourYd(struct timeinfo *ct2, int add_h/*1~23*/)
{
	if(add_h > 0 && add_h < 24)
	{
		ct2->hour += add_h;
		if(ct2->hour > 23)
		{
			ct2->hour = ct2->hour %24;
			ct2->days += 1;
			if( IsLeapYear(ct2->year) )
			{
				if( ct2->days > 366 )
				{
					ct2->days = 1;
					ct2->year += 1;
				}
			}
			else
			{
				if( ct2->days > 365 )
				{
					ct2->days = 1;
					ct2->year += 1;
				}
			}
		}
	}
}







void SubHourYd(struct timeinfo *ct2, int sub_h/*1~23*/)
{
	if(sub_h > 0 && sub_h < 24)
	{	
		if(ct2->hour >= sub_h)
		{
			ct2->hour -= sub_h;
		}
		else
		{
			ct2->hour += 24;
			ct2->hour -= sub_h;

			if(ct2->days > 1)
			{
				ct2->days -= 1;
			}
			else
			{
				if( IsLeapYear((ct2->year)-1) )
				{
					ct2->days = 366;
					ct2->year -= 1;
				}
				else
				{
					ct2->days = 365;
					ct2->year -= 1;
				}
			}
		}
	}
}







void AddSecondYd(struct timeinfo *ct2, int add_s/*1~99*/)
{
	if(add_s >0 && add_s <100)
	{
		ct2->second += add_s;
		if(ct2->second > 59)
		{
			ct2->minute += (ct2->second /60);
			ct2->second %= 60;
			
			if(ct2->minute > 59)
			{
				ct2->minute %= 60;
				ct2->hour += 1;
				if(ct2->hour > 23)
				{
					ct2->hour %= 24;
					ct2->days += 1;
					if( IsLeapYear(ct2->year) )
					{
						if( ct2->days > 366 )
						{
							ct2->days = 1;
							ct2->year += 1;
						}
					}
					else
					{
						if( ct2->days > 365 )
						{
							ct2->days = 1;
							ct2->year += 1;
						}
					}
				}
			}
		}
	}
}






void SubSecondYd(struct timeinfo *ct2, int sub_s/*1~99*/)
{
	int min,sec;
	
	if(sub_s > 0 && sub_s < 100)
	{
		min = sub_s /60;
		sec = sub_s %60;
		
		if(sec <= ct2->second)
		{
			ct2->second -= sec;
		}
		else
		{
			ct2->second += 60;
			ct2->second -= sec;

			if(ct2->minute >= 1)
			{
				ct2->minute -= 1; 
			}
			else
			{
				ct2->minute += 60;
				ct2->minute -= 1;

				if(ct2->hour >= 1)
				{
					ct2->hour -= 1;
				}
				else
				{
					ct2->hour += 24;
					ct2->hour -= 1;

					if(ct2->days > 1)
					{
						ct2->days -= 1;
					}
					else
					{
						if( IsLeapYear((ct2->year)-1) )
						{
							ct2->days = 366;
							ct2->year -= 1;
						}
						else
						{
							ct2->days = 365;
							ct2->year -= 1;
						}
					}
				}
			}
		}

		if(min > 0)
		{
			if(ct2->minute >= min)
			{
				ct2->minute -= min;
			}
			else
			{
				ct2->minute += 60;
				ct2->minute -= min;

				if(ct2->hour >= 1)
				{
					ct2->hour -= 1;
				}
				else
				{
					ct2->hour += 24;
					ct2->hour -= 1;

					if(ct2->days > 1)
					{
						ct2->days -= 1;
					}
					else
					{
						if( IsLeapYear((ct2->year)-1) )
						{
							ct2->days = 366;
							ct2->year -= 1;
						}
						else
						{
							ct2->days = 365;
							ct2->year -= 1;
						}
					}
				}
			}
		}
	}
}







float frac(float origin)
{
	return origin-(u32_t)(origin);
}






void GPSTimeToJulianDay (GPSTime *gpst, JULIANDAY *jld, JULIANDAY2 *jd)
{	
	jd->JD_integer =  gpst->gps_week*7 + 2444244 + (u32_t)((gpst->gps_tow.gps_sow + gpst->gps_tow.gps_ds)/(float)DAY_IN_SECOND + 0.5);
	jd->JD_decimal = frac((gpst->gps_tow.gps_sow + gpst->gps_tow.gps_ds)/(float)DAY_IN_SECOND + 0.5);
	jld->jd_day = jd->JD_integer;
	jld->jd_tod.jd_sod = (u32_t)( jd->JD_decimal * DAY_IN_SECOND +0.5);
	jld->jd_tod.jd_ds = gpst->gps_tow.gps_ds;
}






void JulianDayToCommonTime (JULIANDAY *jld, JULIANDAY2 *jd, struct timeinfo *ct)
{
	u32_t a,b,c,d,e;
	u32_t temp;
	
	a = (u32_t)(jd->JD_decimal+0.5) +jd->JD_integer;
	b = a + 1537;
	c = (u32_t)( (b -122.1)/365.25 );
	d = (u32_t)(365.25*c);
	e = (u32_t)( (b -d)/30.6001 );
	ct->day= (u8_t)(b -d -(u32_t)( 30.6001*e ) + frac(jd->JD_decimal+0.5));
	ct->month = e -1 -12*(u32_t)(e/14);
	ct->year = c -4715 -(u32_t)( (7 + ct->month)/10 );

	temp = (jld->jd_tod.jd_sod)%HOUR_IN_SECOND;
	ct->hour = (u8_t)((jld->jd_tod.jd_sod)/(float)HOUR_IN_SECOND) + 12;

	ct->hour %= 24;

	ct->minute = (u8_t)(temp/(float)MINUTE_IN_SECOND);
	ct->second = (u8_t)(temp%MINUTE_IN_SECOND + jld->jd_tod.jd_ds);
}







void GPSTimeToCommonTime(GPSTime *gpst, struct timeinfo *ct)
{
	JULIANDAY jld;
	JULIANDAY2 jd;
	GPSTimeToJulianDay(gpst, &jld, &jd);
	JulianDayToCommonTime (&jld, &jd, ct);
}








void GpsTime2CT(GPSTime *gps_time, struct timeinfo *ct2)
{
	long day_1981_11000;
	u32_t sec_within_day;
	u16_t year,days;
	u8_t hour,minute,second;
	u16_t temp;

	year = 1981;
	days = 1;
	hour = 0;
	minute = 0;
	second = 0;

	day_1981_11000 = gps_time->gps_week *7 + (long)(gps_time->gps_tow.gps_sow/(float)DAY_IN_SECOND);
	day_1981_11000 -= 360;
	sec_within_day = gps_time->gps_tow.gps_sow%DAY_IN_SECOND;
	while( day_1981_11000 >0 )
	{
		if( IsLeapYear(year) )
		{
			day_1981_11000 -= 366;
		}
		else
		{
			day_1981_11000 -= 365;
		}
		year += 1;
	}

	year -= 1;
	if( IsLeapYear(year) )
	{
		days = 366 + day_1981_11000;
	}
	else
	{
		days = 365 + day_1981_11000;
	}

	temp = sec_within_day %HOUR_IN_SECOND;
	hour = sec_within_day /HOUR_IN_SECOND;
	minute = temp /MINUTE_IN_SECOND;
	second = temp %MINUTE_IN_SECOND;

	ct2->year = year;
	ct2->days = days;
	ct2->hour = hour;
	ct2->minute = minute;
	ct2->second = second;
}










void CommonTimeToGPSTime(struct timeinfo *ct, GPSTime *gpst)
{
	u16_t dayofyear;
	u32_t ttlday, yr,in_yr;
	u8_t dayofweek;

	dayofyear = DateToDays(ct->year, ct->month, ct->day);
	in_yr = ct->year;

	/*fix the  bug if ct->year == 0*/
	if (in_yr < 1981)
	{
		in_yr = 1981;
	}
  	ttlday = 360;
  	for (yr=1981; yr<=(in_yr -1); yr++)
	{
		ttlday  += 365;
		if( ((yr%4 == 0) && (yr%100 != 0)) || (yr%400 == 0) )
		{
			ttlday  += 1;
		}
	}
	ttlday += dayofyear;
	gpst->gps_week = ttlday/7;
	dayofweek  = ttlday%7;
	gpst->gps_tow.gps_sow =  (ct->hour* 3600 + ct->minute* 60 + ct->second + dayofweek * 86400);
	gpst->gps_tow.gps_ds = 0;
}










void CommonTimeToJulianDay(struct timeinfo *ct, JULIANDAY2 *jd)
{
	u8_t m;
	u16_t y;
	float ut;

	ut = ct->hour +  ct->minute/(float)HOUR_IN_MINUTE  +  ct->second/(float)HOUR_IN_SECOND;

	if (ct->month <= 2)
	{
   		y = ct->year - 1;
   		m = ct->month + 12;
	}
	else
	{
   		y = ct->year;
   		m = ct->month;
	}

	jd->JD_integer = (u32_t)(365.25*y) + (u32_t)(30.6001*(m+1)) + ct->day + 1720981+ (u32_t)(ut/24 + 0.5);
	jd->JD_decimal = frac(ut/24 + 0.5);
}








u16_t CommonTimeToDOY(struct timeinfo *ct)
{
	struct timeinfo l_ct;
	JULIANDAY2 jd0,jd1;
	
	l_ct.year = ct->year;
	l_ct.month = 1;
	l_ct.day = 1;
	l_ct.hour = 0;
	l_ct.minute = 0;
	l_ct.second = 0;

	CommonTimeToJulianDay(ct, &jd1);
	CommonTimeToJulianDay(&l_ct, &jd0);

	if(jd1.JD_decimal < jd0.JD_decimal)
	{
		jd1.JD_integer -= 1;
		jd1.JD_decimal += 1;
	}
	jd1.JD_integer -= jd0.JD_integer;
	jd1.JD_decimal -= jd0.JD_decimal;
	
	return (jd1.JD_integer +1);
}







/*时间转换算法*/

long
DateToMjd (long Year, long Month, long Day)
{
    return
        367 * Year
        - 7 * (Year + (Month + 9) / 12) / 4
        - 3 * ((Year + (Month - 9) / 7) / 100 + 1) / 4
        + 275 * Month / 9
        + Day
        + 1721028
        - 2400000;
}

/*
 * Convert Modified Julian Day to calendar date.
 * - Assumes Gregorian calendar.
 * - Adapted from Fliegel/van Flandern ACM 11/#10 p 657 Oct 1968.
 */

void
MjdToDate (long Mjd, long *Year, long *Month, long *Day)
{
    long J, C, Y, M;

    J = Mjd + 2400001 + 68569;
    C = 4 * J / 146097;
    J = J - (146097 * C + 3) / 4;
    Y = 4000 * (J + 1) / 1461001;
    J = J - 1461 * Y / 4 + 31;
    M = 80 * J / 2447;
    *Day = J - 2447 * M / 80;
    J = M / 11;
    *Month = M + 2 - (12 * J);
    *Year = 100 * (C - 49) + Y + J;
}

/*
 * Convert GPS Week and Seconds to Modified Julian Day.
 * - Ignores UTC leap seconds.
 */

long
GpsToMjd (long GpsCycle, long GpsWeek, long GpsSeconds)
{
    long GpsDays;

    GpsDays = ((GpsCycle * 1024) + GpsWeek) * 7 + (GpsSeconds / 86400);
    return DateToMjd(1980, 1, 6) + GpsDays;
}




