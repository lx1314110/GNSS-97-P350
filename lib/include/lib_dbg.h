#ifndef	__LIB_DBG__
#define	__LIB_DBG__





#include <stdio.h>






/*
  debug class
*/
enum {
	DBG_EMERGENCY		= 0x01,	//system is unusable
	DBG_ALERT			= 0x02, //immediate action needed
	DBG_CRITICAL		= 0x03,	//critical conditions
	DBG_ERROR			= 0x04, //error conditions
	DBG_WARNING			= 0x05, //warning conditions
	DBG_NOTICE			= 0x06, //normal but significant condition
	DBG_INFORMATIONAL	= 0x07, //informational messages
	DBG_DEBUG 			= 0x08 	//debug level messages
};

#if 0
//form syslog.h
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */
#endif



//extern unsigned char gDbgLevel;
//extern unsigned short gDbgProType;
//extern unsigned char gSyslogEn;




/*
void print(unsigned short dbgClass, const char *format, ...);
*/



#if 0
#define print(dbgClass, ...) do {\
	if( dbgClass <= gDbgLevel )\
	{\
		if(gSyslogEn){ \
			syslog(gDbgLevel, "<%02d> [%s %s:%d] " , dbgClass, __FILE__, __func__, __LINE__);\
		}else{ \
			fprintf(stdout, "<%s><%02d> [%s %s:%d] ", gDaemonTbl[gDbgProType], dbgClass, __FILE__, __func__, __LINE__);\
			fprintf(stdout, __VA_ARGS__);\
			fprintf(stdout, "\n");\
		} \
	}\
}while(0)
#endif
#ifdef PRINT_DISABLE
#define print(dbgClass, format, args...) do {}while(0)
#else
#define print(dbgClass, format, args...) do {\
	unsigned short __processtype = get_print_processtype(); \
	print_prio(dbgClass, gDaemonTbl[__processtype], __FILE__, __func__, __LINE__, format, ##args); \
}while(0)


#endif


unsigned char set_print_level(unsigned char syslog_en, unsigned char dbg_level, unsigned short process_type);
unsigned short get_print_level(void);
unsigned short get_print_processtype(void);

void syslog_init(const char *ident);
void syslog_exit(void);


void print_prio(unsigned char dbgClass,
	unsigned char *proc_name,
	const char *file_name,
	const char *func_name,
	int	line_num, 
	const char *format, ...);






#endif//__LIB_DBG__


