#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

#include "lib_dbg.h"
#include "alloc.h"




/*
	syslog enable : true1 (default)
	syslog disalbe: false0
*/
static unsigned char gSyslogEn = true;

/*
  debug level
*/
static unsigned char gDbgLevel = DBG_DEBUG;

/*
  which process
*/
static unsigned short gDbgProType = DAEMON_CURSOR_NONE;


static int syslog_init_flag = 0;



/*
  @dbgClass debug class
*/
/*
void print(unsigned short dbgClass, const char *format, ...)
{
	if( dbgClass <= gDbgLevel )
	{
		va_list ap;
		char buf[512];

		va_start(ap,format);
		vsprintf(buf,format,ap);
		va_end(ap);

		printf("<%02d>%s\r\n", dbgClass, buf);
	}
}
*/
#if 0
#define print(dbgClass, ...) do {\
	if( dbgClass <= gDbgLevel )\
	{\
		fprintf(stdout, "<%s><%02d> [%s %s:%d] ", gDaemonTbl[gDbgProType], dbgClass, __FILE__, __func__, __LINE__);\
		fprintf(stdout, __VA_ARGS__);\
		fprintf(stdout, "\r\n");\
	}\
}while(0)
#endif

/*c89:
void print_prio(unsigned char dbgClass,
	unsigned char *proc_name,
	unsigned char *file_name,
	unsigned char *func_name,
	int	line_num, 
	const char *format, ...)

*/
#if 1
void print_prio(unsigned char dbgClass,
	unsigned char *proc_name,
	const char *file_name,
	const char *func_name,
	int	line_num, 
	const char *format, ...)
{
    if(dbgClass <= gDbgLevel)
    {
    	va_list ap;
        char buf[512];

        va_start(ap,format);
        vsprintf(buf,format,ap);
        va_end(ap);
		if(gSyslogEn){
			//syslog(DBG_DEBUG,"%s", buf);
			syslog(dbgClass-1, "<%02d> [%s %s:%d] %s", dbgClass, 
				file_name, func_name, line_num, buf);
		}else{
			fprintf(stdout, "<%s><%02d> [%s %s:%d] %s\n", proc_name, dbgClass, 
				file_name, func_name, line_num,buf);
		}
    }
}
#endif





/*
  set print level
  
  @dbg_level	debug level

  on success, 1 will be returned.
  on error, 0 will be returned.
*/
unsigned char set_print_level(unsigned char syslog_en, unsigned char dbg_level, unsigned short process_type )
{
	if (syslog_en < 0 || syslog_en > 1){
		return 0;
	}

	gSyslogEn = syslog_en;

	if(dbg_level < 0 || dbg_level > 8)
	{
		return 0;
	}

	gDbgLevel = dbg_level;

	if(process_type < DAEMON_CURSOR_NONE || process_type >= DAEMON_CURSOR_ARRAY_SIZE)
	{
		return 0;
	}
	gDbgProType = process_type;

	syslog_init(NULL);
	return 1;
}



/*
  get print process type
*/
unsigned short get_print_processtype(void)
{
	return(gDbgProType);
}





/*
  get print level
*/
unsigned short get_print_level(void)
{
	return(gDbgLevel);
}


void syslog_init(const char *ident)
{
	if(gSyslogEn){
		if (syslog_init_flag == 0){
			openlog(ident, LOG_PID|LOG_CONS, LOG_LOCAL0);
			syslog_init_flag = 1;
		}
		
	}
}

void syslog_exit(void)
{
	if(gSyslogEn != 0 && syslog_init_flag == 1){
		closelog();
		syslog_init_flag = 0;
	}
}


