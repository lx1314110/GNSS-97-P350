#include <stdio.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_msgQ.h"

#include "addr.h"
#include "dlcd_global.h"
#include "dlcd_ctrl.h"
#include "dlcd_menu.h"
#include "common.h"

#define KEY_MENU        0
#define KEY_UP          1
#define KEY_DOWN        2
#define KEY_CONFIRM     3
#define KEY_RETURN      4

#define GENERIC_READ_CMD_OPTION         1
#define GENERIC_WRITE_CMD_OPTION        2
#define GENERIC_MENU_OPTION             3

#define CMD_MSG_LEN     128
// 21
#define LCD_MSG_LEN     32

#define LCD_LINE0       0
#define LCD_LINE1       1
#define LCD_LINE2       2
#define LCD_LINE3       3
#define LCD_LINE4       4
#define LCD_LINE5       5
#define LCD_LINE6       6
#define LCD_LINE7       7

int user_input_slot = 0;
int lcd_timeout = 0;


typedef void (*func_timeout) (void);


char main_menu[LCD_LINE][LCD_MSG_LEN] = {
	 "*********************",
	 "**     WELCOME     **",
	 "**   DTT -- P350   **",
	 "**                 **",
	 "**                 **",
	 "**                 **",
	 "**                 **",
	 "*********************"
	 };


const cmdMenu cmd_menu[] =
{
	{"  View Version",       (*LCD_ViewVersion), 
	GENERIC_READ_CMD_OPTION,  0, 0, 0},
	{"  View Baudrate",      (*LCD_ViewHelp), 
	GENERIC_READ_CMD_OPTION,  0, 0, 1},
	{"  System Config",      (*LCD_SystemConfig), 
	GENERIC_MENU_OPTION,      0, 0, 2},
	{"  View Alarm",         (*LCD_AlarmConfig), 
	GENERIC_MENU_OPTION,      0, 0, 3},
	{"  Input Boards",       (*LCD_InputConfig), 
	GENERIC_MENU_OPTION,      0, 0, 4},
	{"  Output Boards",      (*LCD_OutputConfig), 
	GENERIC_MENU_OPTION,      0, 0, 5},

	

	{"  View Current Time",  (*LCD_View_Time), 
	GENERIC_READ_CMD_OPTION,  1, 2, 0},		
	{"  View Leap Second",   (*LCD_View_Leap), 
	GENERIC_READ_CMD_OPTION,  1, 2, 1},	
	{"  View Net Config",    (*LCD_View_Net), 
	GENERIC_READ_CMD_OPTION,  1, 2, 2},	
	{"  View Clock Type",    (*LCD_View_Clock), 
	GENERIC_READ_CMD_OPTION,  1, 2, 3},
	{"  View Board Type",    (*LCD_View_Btype), 
	GENERIC_READ_CMD_OPTION,  1, 2, 4},

	{"  View Current Alarm", (*LCD_View_Alarm), 
	GENERIC_READ_CMD_OPTION,  1, 3, 0},
	{"  View History Alarm ", (*LCD_View_AlarmHistory), 
	GENERIC_READ_CMD_OPTION,  1, 3, 1},

	{"  View GPS/BD Status", (*LCD_View_Status),  
	GENERIC_READ_CMD_OPTION,  1, 4, 0},	
	{"  View Priority",      (*LCD_View_Priority), 
	GENERIC_READ_CMD_OPTION,  1, 4, 1},	
//	{"  View Delay",         (*LCD_View_Delay), 
//	GENERIC_READ_CMD_OPTION,  1, 4, 2},
	{"  View SSM/SA",        (*LCD_View_SSM_SA), 
	GENERIC_READ_CMD_OPTION,  1, 4, 2},
	{"  View Signal",        (*LCD_View_Signal), 
	GENERIC_READ_CMD_OPTION,  1, 4, 3},
	{"  View Mode",          (*LCD_View_Mode), 
	GENERIC_READ_CMD_OPTION,  1, 4, 4},
	{"  View Elevation",     (*LCD_View_Elevation), 
	GENERIC_READ_CMD_OPTION,  1, 4, 5},
	{"  View Lo/La",         (*LCD_View_Lola), 
	GENERIC_READ_CMD_OPTION,  1, 4, 6},
	
	{"  View Output",        (*LCD_View_Output), 
	GENERIC_READ_CMD_OPTION,  1, 5, 0},	
	{ 0, 0, 0, 0, 0 }
};


static void initializeContext(struct lcdCtx *ctx);
static void exitHandler( int signum );
static bool_t isRunning(struct lcdCtx *ctx);

/*
  true:		程序运行
  false:	程序退出
*/
static bool_t isRunning(struct lcdCtx *ctx)
{
	return ctx->loop_flag;
}

/**************************************************
author:   Tim
date:      2013.07.02
function: Init Menu
retrun:  	NULL
modify:  
***************************************************/
void InitMenu(char level, char group)
{
	int line = 0;
	const cmdMenu *pcmd_menu = cmd_menu; 
	FullScreenClear();

	while (pcmd_menu->cmd_func != NULL) 
	{
		if(pcmd_menu->level == level && pcmd_menu->group== group)
		{
			DrawOneString(pcmd_menu->shortDesc,line);
			line++;
		}
		pcmd_menu++;
	}
	FullScreenEnd();
}

/**************************************************
author:   Tim
date:      2013.07.02
function: Common function to show 
retrun:  	NULL
modify:  
***************************************************/
int LCD_View_Common(char *command)
{
	char buf[CMD_MSG_LEN];
	int len=0;
	int line=0;
	FILE *readfp;
	
	if(command==NULL)
		return -1;
	
	if((readfp = popen(command, "r" ))==NULL)
		return -1;
	
	FullScreenClear();	
	while((line<LCD_LINE)&&(fgets(buf, CMD_MSG_LEN, readfp)!=NULL))
	{
		len =strlen(buf);
		buf[len-1] = 0;
		DrawOneString(buf,line++);
		memset( buf,0,sizeof(buf) );
	}
	FullScreenEnd();

	pclose( readfp );
	return 1;

}


/**************************************************
author:   Tim
date:      2013.07.02
function: LCD Get current max order number
retrun:  	maxorder
modify:  
***************************************************/
char GetMaxOrder(unsigned char index)
{
	const cmdMenu *pcmd_menu = &cmd_menu[index]; 
	char maxorder = 0;
	char level = pcmd_menu->level;
	char group = pcmd_menu->group;
	pcmd_menu = cmd_menu; 

	while (pcmd_menu->cmd_func != NULL) 
	{
		if ((pcmd_menu->level == level) &&(pcmd_menu->group == group)) 
		{
			maxorder++;
		}
		pcmd_menu++;
	}
	
	return (maxorder-1);

}

/**************************************************
author:   Tim
date:      2013.07.02
function: LCD Get Index By Group
retrun:  	index
modify:  
***************************************************/
char GetIndexByGroup(char level, char group)
{
	const cmdMenu *pcmd_menu = cmd_menu; 
	char index = 0;

	while (pcmd_menu->cmd_func != NULL) 
	{
		if ((pcmd_menu->level == level) &&(pcmd_menu->group == group)) 
		{
			break;
		}
		pcmd_menu++;
		index++;
	}
	
	return index;
}

/**************************************************
author:   Tim
date:      2013.07.02
function: LCD Get Index By Order
retrun:  	index
modify:  
***************************************************/
char GetIndexByOrder(char level, char order)
{
	const cmdMenu *pcmd_menu = cmd_menu; 
	char index = 0;

	while (pcmd_menu->cmd_func != NULL) 
	{
		if ((pcmd_menu->level == level) &&(pcmd_menu->order == order)) 
		{
			break;
		}
		pcmd_menu++;
		index++;
	}
	
	return index;

}

/**************************************************
author:   Tim
date:      2013.07.03
function: get p350 init
retrun:  	1	init  ready
  		-1	init not ready
modify:  add 300s timeout 
***************************************************/
int GetP350Init()
{
	int ret;
	//char current_inits[4];	
	fd_set	read_fds;
	struct  timeval tv;
	FD_ZERO(&read_fds);
	FD_SET(gLcdCtx.int_fd, &read_fds);
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	if((ret = select(gLcdCtx.int_fd+1, &read_fds, NULL, NULL, &tv))<0)
	{
		print(DBG_DEBUG, "<%s>--select error.", 
								 gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
		return -1;
	}
	else if(0 == ret)//超时
	{
		print(DBG_DEBUG, "<%s>--select timeout.", 
								 gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
		return 0;
	}
	else 
	{
		if (FD_ISSET(gLcdCtx.int_fd, &read_fds))
		{
			/*if (read(gLcdCtx.int_fd, current_inits, sizeof current_inits) != sizeof current_inits) 
			{
				print(DBG_DEBUG, "<%s>--Failed to read int device.", 
									 gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
				return -1;
			
			}*/

			return 1;
		}else
		{
			return -1;
		}
	}

	//return 1;
}


/**************************************************
author:   Tim
date:      2013.07.02
function: LCD get key value
retrun:  	1	ok
  		-1	error
modify:  
***************************************************/
static int KeyValue()
{
	u16_t key_tmp;

	if(!FpgaRead(gLcdCtx.fpga_fd, FPGA_KEY_TABLE, &key_tmp))
	{
		print(DBG_ERROR, "<%s>--Failed to read  from FPGA.", 
					   gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
		return -1;
	}

	//printf("key %d\n",key_tmp);
	
	switch(key_tmp){  
		case(0x01):
			return(KEY_MENU);
		case(0x02):
			return(KEY_UP);
		case(0x04):
			return(KEY_DOWN);
		case(0x08):
			return(KEY_CONFIRM);
		case(0x10):
			return(KEY_RETURN);
		default:
			return -2;
			
	}
	
	return 1;
}

/**************************************************
author:   Tim
date:      2013.07.02
function: LCD sub menu operate
retrun:  	1	ok
  		-1	error
modify:  
***************************************************/
static int SubMenuGetValue(int key, int *num, int min, int max)
{
	char str[LCD_MSG_LEN] = {0};
	
	switch (key) {
		case KEY_UP:
			(*num)++;
			if(*num>max)
				*num = max;
			sprintf(str, "  %06d", *num);

			DrawOneString(str, LCD_LINE4) ;
			FullScreenEnd();
			break;
		case KEY_DOWN:
			(*num)--;
			if(*num<min)
				*num = min;
			sprintf(str, "  %06d", *num);
			DrawOneString(str, LCD_LINE4) ;
			FullScreenEnd();		
			break;
		case KEY_RETURN:
			break;
		case KEY_MENU:
			break;
		case KEY_CONFIRM:
			return 1;
			break;
		default:
			return -1;

	}

	return 0;
}

/**************************************************
author:   Tim
date:      2013.07.02
function: LCD get user input value
retrun:  	1	ok
  		-1	error
modify:  
***************************************************/
int GetInputValue(int *num, int min, int max)
{

	unsigned int key = -1;
	int* p = num;
	*p = min;
	
	while(1) 
	{
		if (1 == GetP350Init()) 
		{
			key = KeyValue();
			if (-1 == key || -2 == key) 
			{
				if (key == -2)
					print(DBG_INFORMATIONAL, "--Failed to read key device.");
				//continue;
			}

			if(SubMenuGetValue(key, p, min, max)==1)
				break;
		}

	}
	return 1;

}


/**************************************************
author:   Tim
date:      2013.07.02
function: LCD keytable operatioon
retrun:  	1	ok
  		-1	error
modify:  
***************************************************/
static int MainMenuOperate(int key)
{
	static unsigned char  FuncIndex=0;	                      //for menu contrl
	static char cmdOptionType = GENERIC_MENU_OPTION;  //for menu contrl
	static char maxOrder = 5;                         //for menu contrl
	void (*FuncPtr)()=NULL;
	char level=0,group=0;
	
	switch (key) {
		case KEY_UP:
			if(cmdOptionType == GENERIC_MENU_OPTION)
			{
				FuncIndex = (cmd_menu[FuncIndex].order==0) ? (FuncIndex) : (FuncIndex-1);
				ShowMenuChange(cmd_menu[FuncIndex].order);
			}
			break;
			
		case KEY_DOWN:
			if(cmdOptionType == GENERIC_MENU_OPTION)
			{
				FuncIndex = (cmd_menu[FuncIndex].order==maxOrder) ? (FuncIndex) : (FuncIndex+1);
				ShowMenuChange(cmd_menu[FuncIndex].order);		
			}
			break;
			
		case KEY_RETURN://返回
			if(cmdOptionType == GENERIC_MENU_OPTION)
			{
				level = (cmd_menu[FuncIndex].level==0) ? 0:(cmd_menu[FuncIndex].level-1);
				FuncIndex = GetIndexByOrder(level,0);	
			}
			InitMenu(cmd_menu[FuncIndex].level, cmd_menu[FuncIndex].group);
			ShowMenuChange(cmd_menu[FuncIndex].order);	
			cmdOptionType = GENERIC_MENU_OPTION;
			maxOrder = GetMaxOrder(FuncIndex);

			break;
			
		case KEY_MENU:
			FuncIndex=0;
			cmdOptionType=GENERIC_MENU_OPTION;
			maxOrder = GetMaxOrder(FuncIndex);
			InitMenu(0,0);
			ShowMenuChange(cmd_menu[FuncIndex].order);	
			break;
			
		case KEY_CONFIRM://确认
			FuncPtr=cmd_menu[FuncIndex].cmd_func;
			(*FuncPtr)();
			cmdOptionType = cmd_menu[FuncIndex].cmdOptionType;
			if(cmd_menu[FuncIndex].cmdOptionType==GENERIC_MENU_OPTION)
			{
				level = cmd_menu[FuncIndex].level+1;
				group = cmd_menu[FuncIndex].order;
				FuncIndex = GetIndexByGroup(level,group);
				maxOrder = GetMaxOrder(FuncIndex);
				ShowMenuChange(cmd_menu[FuncIndex].order);	
			}
			break;
			
		default:
			return -1;
	}

	return 1;
}

/**************************************************
author:   Tim
date:      2013.07.22
function: initialize prosess Priority
retrun:  	1	ok
  		-1	error
modify:  
***************************************************/
static int initializePriority(int prio)
{
	pid_t pid;
	
	pid = getpid();
	if ( 0 != setpriority(PRIO_PROCESS, pid, prio) )
	{			
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_KEYLCD], 
				"Can't set priority.");
		return(-1);
	}
	return 0;
}


/**************************************************
author:   Tim
date:      2013.08.15
function: if Lcd Timeout-------- show real time
retrun:  NULL
modify:  
***************************************************/

void LcdTimeoutShowTime()
{
    time_t lt;
	char *str = NULL;
	char line4[LCD_MSG_LEN] = "**                 **";
	char line5[LCD_MSG_LEN] = "**                 **";

	lt = time(NULL);
	str = ctime(&lt);
	memcpy(&line5[4], str+3, 7);
	memcpy(&line5[12], str+20, 4);
	memcpy(&line4[7], str+11, 9);
	DrawOneString(line4,LCD_LINE4);
	DrawOneString(line5,LCD_LINE5);
	FullScreenEnd();
}


/**************************************************
author:   Tim
date:      2013.08.15
function: if Lcd Timeout-------- show Satellite status
retrun:  NULL
modify:  
***************************************************/
void LcdTimeoutShowSatellite()
{
	FILE *readfp;
	char buf[CMD_MSG_LEN] = {0};
	int line = LCD_LINE3;
	int len = 0;
	int i = 0;
	int show_offset = 0;
	int valid_line_num = 0;
	//show max 4 slot
	char sate_num_info[INPUT_SLOT_CURSOR_ARRAY_SIZE][LCD_MSG_LEN];
	//max show 21 col
	char line3_6[4][LCD_MSG_LEN] = {
			"**                 **",
			"**                 **",
			"**                 **",
			"**                 **"
			                 
	};

	for (i = 0; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		memset(sate_num_info[i], '\0', sizeof(sate_num_info[i]));
	}

	if((readfp = popen("p350in gb", "r" ))==NULL)
		return;

	i = 0;
	while((i<INPUT_SLOT_CURSOR_ARRAY_SIZE)&&(fgets(buf, CMD_MSG_LEN, readfp)!=NULL))
	{
		len = strlen(buf);
		if(len > 0){
			buf[len-1] = ' ';//"\n"->" "
			memcpy(sate_num_info[i++], buf, len);
		}
		memset(buf,0,sizeof(buf) );
	}

	for (i = 0; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		if (sate_num_info[i][0] != '\0'){
			valid_line_num++;
		}
	}

	if (valid_line_num < 4){
		show_offset = 1;
	}

	i = 0;
	// so i+show_offset<4
	while((line<LCD_LINE7))
	{
		len = strlen(sate_num_info[i]);
		if (len > 0){
			memcpy(&line3_6[i+show_offset][3], sate_num_info[i], len);
		}
		DrawOneString(&line3_6[i++][0],line++);
		memset(buf,0,sizeof(buf) );
	}

	FullScreenEnd();
	pclose( readfp );

	return;

}

/**************************************************
author:   Tim
date:      2013.08.15
function: if Lcd Timeout-------- show lola Elevation
retrun:  NULL
modify:  
***************************************************/
void LcdTimeoutShowLocation()
{
	FILE *readfp;
	char buf[CMD_MSG_LEN] = {0};
	char str[LCD_MSG_LEN] = {0};
	int line = 0;

	if((readfp = popen("zhanghui", "r" ))==NULL)
		return;
		
	while(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		strncpy(str, buf, 10);
		DrawOneString(str,line++);
		memset(str,0,sizeof(str) );
	}
	pclose( readfp );

}




/**************************************************
author:   Tim
date:      2013.07.31
function: if Lcd Timeout, show real time, gps status...
retrun:  NULL
modify:  
***************************************************/
void LcdTimeoutShow()
{
	int i     = 0;
	int j     = 0;
	int count = 0;
	func_timeout funclist[2] = {
							LcdTimeoutShowTime,
							LcdTimeoutShowSatellite
							} ;

	pthread_detach(pthread_self());
	//print(DBG_NOTICE, "sub pid:%d", pthread_self());
	FullScreenClear();

	for(j=0;j<LCD_LINE;j++)
	{
		DrawOneString(main_menu[j],j);
	}
#if 1
	FullScreenEnd();
#endif
	while(lcd_timeout && isRunning(&gLcdCtx))
	{
		funclist[i]();
		sleep(1);
		if((++count)%10 == 0)
		{
			count = 0;
			i++;
			if(i>=2)
				i=0;
			
			FullScreenClear();	
			
			for(j=0;j<LCD_LINE;j++)
			{
				DrawOneString(main_menu[j],j);
			}
			
		}
		#if 1
		FullScreenEnd();
		#endif
	}
	print(DBG_DEBUG, "pthread exit clearly!\n");
	return;
}

#if 0
void StratReset(struct lcdCtx *ctx)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_RESET_LCD, 0x0000))//LCD reset
	{
		return 0;
	}
	if(!FpgaWrite(ctx->fpga_fd, FPGA_RESET_LCD, 0x0001))
	{
		return 0;
	}
}

static void Reset_Lcd(int sig)
{    
	if( SIGALRM == sig )
	{
		StratReset(&gLcdCtx);
	}
}

//弃用
int LcdReset()
{
	struct itimerval timer;
	
	if( SIG_ERR == signal(SIGALRM, SIG_IGN) )
	{
		return 0;
	}

	if( SIG_ERR == signal(SIGALRM, Reset_Lcd) )
	{
		return 0;
	}

	timer.it_value.tv_sec = 60;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 60;
	timer.it_interval.tv_usec = 0;
	if( -1 == setitimer(ITIMER_REAL, &timer, 0) )//实现精度较高的定时功能，ITIMER_REAL: 以系统真实的时间来计算，它送出SIGALRM信号
	{
		return 0;
	}

	return 1;
}
#endif

static void initializeContext(struct lcdCtx *ctx)
{
	ctx->loop_flag = true;
	ctx->notify_flag = false;
	ctx->power_flag = true;
	ctx->pDb = NULL;
	ctx->fpga_fd = -1;
	ctx->int_fd = -1;
}

static void exitHandler( int signum )
{
	if( (EXIT_SIGNAL1 == signum) ||
		(EXIT_SIGNAL2 == signum) ||
		(EXIT_SIGNAL3 == signum) ){
		gLcdCtx.loop_flag = false;
		print(DBG_NOTICE, "pid:%d received signo:%d", pthread_self(), signum);
	}
}


/**************************************************
author:   Tim
date:      2013.07.31
function: LCD main 
retrun:  	1	ok
  		-1	error
modify:  add lcd timeout pthread
***************************************************/
int Prockeylcd(struct lcdCtx *ctx)
{
    int key = 0;
	int reslt = 0;
	int ret = 0;
	pthread_t id = 0; 
	
	set_print_level(true, DBG_INFORMATIONAL, DAEMON_CURSOR_KEYLCD);
	syslog_init(NULL);
	initializeContext(ctx);
	if(-1 == initializeCommExitSignal(&exitHandler))
	{
		ret = __LINE__;
		goto exit1;
	}

	ctx->fpga_fd = FpgaOpen(FPGA_DEV);
	if(-1 == ctx->fpga_fd)
	{
		print(DBG_ERROR, "--Failed to open FPGA device.");
		ret = __LINE__;
		goto exit1;
	}
	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit2;
	}

	ctx->int_fd = FpgaOpen(INT0_DEV);//按键中断
	if(-1 == ctx->int_fd)
	{
		print(DBG_ERROR, "--Failed to open int device.");
		ret = __LINE__;
		goto exit2;
	}
#if 0
	ctx->ps_fd = FpgaOpen(PS_DEV);
	if(-1 == ctx->ps_fd)
	{
		print(DBG_ERROR, "<%s>--Failed to open ps device.", 
							 gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
		return -1;
	}

	Fpga_DownLoad();

	
#endif

	initializePriority(DAEMON_PRIO_KEYLCD);//初始化当前进程的优先级

	InitMenu(0,0);//初始化菜单

	#if 0
	//已使用写完成的方式，原复位方式为临时解决方案，现弃用
	if(0 == LcdReset(ctx->fpga_fd))
	{
		return -1;
	}
	#endif

	while(isRunning(ctx)) 
	{
		SysWaitFpgaRunStatusOk(ctx->fpga_fd);
		reslt = GetP350Init();//获取P350的初始化准备状态
		
		if(1 == reslt) //有按键中断
			
		{
			if(1 == lcd_timeout)
			{
				lcd_timeout = 0;
				key = KEY_MENU;
			}
			else
			{
				key = KeyValue();//获取按键信息
				if (-1 == key || -2 == key) 
				{
					if(-2 == key)
						print(DBG_INFORMATIONAL,
							"--Failed to read key device.");
					continue;
				}
			}
			
			MainMenuOperate(key);//主菜单控制
		}
		else if(0 == reslt)
		{
			if(lcd_timeout == 0)
			{
				lcd_timeout = 1;
				ret = pthread_create(&id, NULL, (void *)LcdTimeoutShow, NULL); //超时就显示时间、GPS状态

				if(ret!=0)
				{ 
					print(DBG_ERROR, "--pthread_create error."); 
				}

				print(DBG_DEBUG, "--Timeout.");
			}
		}
		
	}

//exit3:	
	FpgaClose(ctx->int_fd);

exit2:
	FpgaClose(ctx->fpga_fd);

exit1:
	sleep(2);
	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();

	return ret;
}


/*****************                command function                    *****************/
void LCD_ViewVersion(void)
{
	LCD_View_Common("p350sys ver");
	return;
}

void LCD_ViewHelp(void)
{
	FullScreenClear();
	DrawOneString("  Baudrate: 115200", LCD_LINE4);
	FullScreenEnd();
	return;
}

void LCD_SystemConfig(void)
{
	InitMenu(1, 2);
}

void LCD_AlarmConfig(void)
{
	InitMenu(1, 3);
}

void LCD_InputConfig(void)
{
	char buf[32];
	memset(buf, 0x0, sizeof(buf));
	sprintf(buf, "Input your slot(1-%d):", INPUT_SLOT_CURSOR_ARRAY_SIZE);
	FullScreenClear();
	DrawOneString(buf,LCD_LINE0);
	DrawOneString("  000001", LCD_LINE4) ;
	FullScreenEnd();

	GetInputValue(&user_input_slot,1,INPUT_SLOT_CURSOR_ARRAY_SIZE);
	InitMenu(1, 4);
}
/**************************************************
author:   Tim
date:      2013.07.30
function: LCD output menu
retrun:    NULL
modify:   LCD has a bug,so clear twice !
***************************************************/
void LCD_OutputConfig(void)
{
	char buf[32];
	memset(buf, 0x0, sizeof(buf));
	sprintf(buf, "Input your slot(1-%d):", SLOT_CURSOR_ARRAY_SIZE);
	FullScreenClear();
	FullScreenClear();
	DrawOneString(buf,LCD_LINE0);
	DrawOneString("  000001", LCD_LINE4) ;
	FullScreenEnd();

	GetInputValue(&user_input_slot,1,SLOT_CURSOR_ARRAY_SIZE);
	InitMenu(1, 5);
}

/**************************************************
author:   Tim
date:      2013.08.02
function: LCD view time
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Time(void)
{
	LCD_View_Common("p350sys time"); 
	return;
}

/**************************************************
author:   Tim
date:      2013.07.29
function: LCD view leap
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Leap(void)
{
	char buf[CMD_MSG_LEN];
	char str[LCD_MSG_LEN] = " Leap Time :";
	int len=0;
	FILE *readfp;
	
	if((readfp = popen("p350sys leap", "r" ))==NULL)
		return;
	
	FullScreenClear();
	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		strcat(str, buf);
		len = strlen(str);
		str[len-1] = 's';
		str[len] = 0;
		DrawOneString(str,LCD_LINE4);
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}


void LCD_View_Net(void)
{

	LCD_View_Common("p350sys net");	
	return;
}

void LCD_View_Clock(void)
{
	LCD_View_Common("p350sys clock");	
	return;
}
/**************************************************
author:   Tim
date:      2013.07.03
function: LCD view board type
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Btype(void)
{
	char buf[CMD_MSG_LEN];
	char str[LCD_MSG_LEN];
	int len=0;
	int line=0;
	int slot = 1;
	FILE *readfp;
	
	if((readfp = popen("p350sys btype", "r" ))==NULL)
		return;
	
	FullScreenClear();	
	while((line<LCD_LINE)&&(fgets(buf, CMD_MSG_LEN, readfp)!=NULL))
	{
		len = strlen(buf);
		buf[len-1] = 0;
		
		if(!strstr(buf, "none"))
		{
			sprintf(str, "Slot %d: %s", slot,buf);
			DrawOneString(str,line++);
			memset(str,0,sizeof(str) );
		}
		
		slot++;
		memset(buf,0,sizeof(buf) );
		
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;

}

/**************************************************
author:   Tim
date:      2013.07.03
function: LCD view current alarm
retrun:    NULL
modify:  need more line
***************************************************/
void LCD_View_Alarm(void)
{
	char buf[CMD_MSG_LEN];
	char str[LCD_MSG_LEN] = " Current Alarm : ";
	int len=0;
	int line=0;
	int num =0;
	FILE *readfp;
	
	if((readfp = popen("p350alm", "r" ))==NULL)
		return;

	FullScreenClear();	
	while((line<LCD_LINE)&&(fgets(buf, CMD_MSG_LEN, readfp)!=NULL))
	{
		if(buf[0]=='1' && buf[4]=='0')
		{
			num ++;
	//		len =strlen(buf);
	//		buf[len-1] = 0;
	//		DrawOneString(&buf[12],line++);
	//		memset( buf,0,sizeof(buf) );
		}
	}

	sprintf(buf, "%d", num);
	strcat(str, buf);
	len = strlen(str);
	str[len] = 0;
	DrawOneString(str,LCD_LINE4);

	FullScreenEnd();

	pclose( readfp );
	
	return;
}


/**************************************************
author:   Tim
date:      2013.07.03
function: LCD view history alarm
retrun:    NULL
modify:  2013.10.30
***************************************************/
void LCD_View_AlarmHistory(void)
{
	char buf[CMD_MSG_LEN];
	char str[LCD_MSG_LEN] = " History Alarm : ";
	int len = 0;
	int num = 0;
	FILE *readfp;
	
	if((readfp = popen("p350alm almnum", "r" ))==NULL)
		return;
	
	FullScreenClear();	
	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		num = atoi(buf);
		sprintf(buf,"%d",num);
		strcat(str, buf);
		len = strlen(str);
		str[len] = 0;
		DrawOneString(str,LCD_LINE4);
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}


/**************************************************
author:   Tim
date:      2013.08.01
function: is standby or active
retrun:    0  standby
             1  active
             -1 free mode
modify:  
***************************************************/
int LCD_Get_Schema(int slot)
{
	char buf[CMD_MSG_LEN];
	FILE *readfp;

	if((readfp = popen("p350in schema", "r" ))==NULL)
		return -1;

	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		if(strstr(buf,"free"))
			return -1;
		else if (strstr(buf,"force"))
		{
			//if(((slot==1) && (strstr(buf,"1"))) || ((slot==2) && (strstr(buf,"6"))))
				return 1;
		}
	}
	
	pclose( readfp );

	return -1;

}


/**************************************************
author:   Tim
date:      2013.08.01
function: LCD view GPS/BD status
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Status(void)
{
	char buf[CMD_MSG_LEN];
	int line = 0;
	int i    = 0;
	int flag = 0;
	FILE *readfp;
	char str[LCD_MSG_LEN] = {0};

	flag = LCD_Get_Schema(user_input_slot);
	
	sprintf(str, "p350in %d 1", user_input_slot);

	if((readfp = popen(str, "r" ))==NULL)
		return;
	
	FullScreenClear();	

	switch(flag)
	{
		case (0):
			DrawOneString("Schema Mode: Standby",line++);
			break;
		case (1):
			DrawOneString("Schema Mode: Active",line++);
			break;
		case (-1):
			DrawOneString("Schema Mode: Free",line++);
			break;
		default :
			break;
	}
	
	while(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		i++;
		if(i==12)
		{	
			if(strstr(buf, "GPS"))
				strncpy(str, buf, 10);
			else	
				strncpy(str, "GPS | 0 |", 10);

			DrawOneString(str,line++);
			memset(str,0,sizeof(str) );
		}
		else if(i==13)
		{	
			if(strstr(buf, "BD"))
				strncpy(str, buf, 9);
			else	
				strncpy(str, "BD | 0 |", 9);

			DrawOneString(str,line++);
			memset(str,0,sizeof(str) );
		}	
		
		memset(buf,0,sizeof(buf) );
	}

	FullScreenEnd();

	pclose( readfp );
	
	return;
}


/**************************************************
author:   Tim
date:      2013.07.30
function: LCD view priority
retrun:    NULL
modify:  priority    (low 0-9 high)
***************************************************/
void LCD_View_Priority(void)
{
	char buf[CMD_MSG_LEN];
	int len=0;
	int line=0;
	FILE *readfp;
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350in %d prio", user_input_slot);

	if((readfp = popen(str, "r" ))==NULL)
		return;
	
	FullScreenClear();
	memset( str,0x0,sizeof(str) );
	if((fgets(buf, CMD_MSG_LEN, readfp)!=NULL) && (strstr(buf,"Failure")==NULL))
	{
		len = strlen(buf);//len=valid data+'\n'
		buf[len-1] = 0;//'\n'
		while(line<(len-1) && line<LCD_LINE)
		{
			sprintf(str, "Signal %d: %c", line+1, buf[line]);
			DrawOneString(str,line++); 
			memset( str,0,sizeof(str) );
		}
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}

/**************************************************
author:   Tim
date:      2013.07.30
function: LCD view delay
retrun:    NULL
modify:  delay unit : 10ns
***************************************************/
void LCD_View_Delay(void)
{
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350in %d delay", user_input_slot);
	LCD_View_Common(str); 

	return;
}

/**************************************************
author:   Tim
date:      2013.08.02
function: LCD view SSM/SA
retrun:    NULL
modify:  
***************************************************/
void LCD_View_SSM_SA(void)
{
	int line = LCD_LINE2;
	char buf[CMD_MSG_LEN];
	int len=0;
	FILE *readfp;
	char str[LCD_MSG_LEN] = {0};
		
	FullScreenClear();	

	//2mh
	sprintf(str, "p350in %d ssm", user_input_slot);
	
	if((readfp = popen(str, "r" ))==NULL)
		return;

	memset(buf, 0, sizeof(buf) );
	while(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		len =strlen(buf);
		buf[len-1] = 0;
		//sprintf(str, "2MH SSM : %s", buf);
		sprintf(str, "%s", buf);
		DrawOneString(str,line++);
		memset(buf, 0, sizeof(buf) );
	}
	pclose( readfp );

	sprintf(str, "p350in %d sa", user_input_slot);

	if((readfp = popen(str, "r" ))==NULL)
		return;

	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		len =strlen(buf);
		buf[len-1] = 0;
		//sprintf(str, "2MB SA : %s", buf);
		sprintf(str, "%s", buf);
		DrawOneString(str, line++);
		memset(buf, 0, sizeof(buf) );
	}
	pclose( readfp );

	FullScreenEnd();

	return;

}


/**************************************************
author:   Tim
date:      2013.07.29
function: LCD view Signal
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Signal(void)
{
	char buf[CMD_MSG_LEN];
	int len=0;
	int line=0;
	FILE *readfp;
	char *tmp = NULL;
	const char *separator = ":";
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350in %d signal", user_input_slot);

	
	if((readfp = popen(str, "r" ))==NULL)
		return;
	
	FullScreenClear();	
	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		len = strlen(buf);
		buf[len-1] = 0;
		tmp = strtok(buf,separator);
		while((line<LCD_LINE)&&(tmp!=NULL))
		{
			sprintf(str, "Signal %d: %s", line+1, tmp);
			DrawOneString(str,line++);
			tmp = strtok(NULL,separator);  
			memset( str,0,sizeof(str) );
		}
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}

/**************************************************
author:   Tim
date:      2013.07.29
function: LCD view Mode
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Mode(void)
{
	char buf[CMD_MSG_LEN];
	int line=0;
	FILE *readfp;
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350in %d mode", user_input_slot);

	
	if((readfp = popen(str, "r" ))==NULL)
		return;

	
	FullScreenClear();	
	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		if(strstr(buf,"SG"))
		{
			DrawOneString("Mode : Single",line++); 
			DrawOneString("Satellite : GPS",line++); 
		}
		else if(strstr(buf,"SB"))
		{
			DrawOneString("Mode : Single",line++); 
			DrawOneString("Satellite : BD",line++); 
		}
		else if(strstr(buf,"MG"))
		{
			DrawOneString("Mode : Mix",line++); 
			DrawOneString("Satellite : GPS",line++); 
		}
		else if(strstr(buf,"MB"))
		{
			DrawOneString("Mode : Mix",line++); 
			DrawOneString("Satellite : BD",line++); 
		} 

	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}



/**************************************************
author:   Tim
date:      2013.07.29
function: LCD view Elevation
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Elevation(void)
{
	char buf[CMD_MSG_LEN];
	int len=0;
	FILE *readfp;
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350in %d elev", user_input_slot);
	
	if((readfp = popen(str, "r" ))==NULL)
		return;

	
	FullScreenClear();	
	if(fgets(buf, CMD_MSG_LEN, readfp)!=NULL)
	{
		len = strlen(buf);
		if(len < CMD_MSG_LEN-1){
			buf[len-1] = ' ';
			buf[len]   = 'M';
			buf[len+1] = 0;
			DrawOneString(&buf[0], LCD_LINE4);
		}
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}

void LCD_View_Lola(void)
{
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350in %d lola", user_input_slot);
	LCD_View_Common(str); 

	return;
}
/**************************************************
author:   Tim
date:      2013.07.30
function: LCD view output
retrun:    NULL
modify:  
***************************************************/
void LCD_View_Output(void)
{
	char buf[CMD_MSG_LEN];
	int len   = 0;
	int line  = 0;
	int count = 0;
	FILE *readfp;
	char *tmp = NULL;
	const char *separator = "|";
	char str[LCD_MSG_LEN] = {0};
	sprintf(str, "p350out %d", user_input_slot);

	
	if((readfp = popen(str, "r" ))==NULL)
		return;
	
	FullScreenClear();	
	if((fgets(buf, CMD_MSG_LEN, readfp)!=NULL) && (strstr(buf,separator)))
	{
		len = strlen(buf);
		buf[len-1] = 0;
		tmp = strtok(buf, separator);

		while(tmp)
		{
			switch (count) 
			{
				case 0:              //delay
					sprintf(str, "Delay     : %s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;	
				case 2:              //delay type
					sprintf(str, "Delay type: %s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;
				case 3:              //multi
					sprintf(str, "Multi type: %s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;
				case 4: 		     //layer
					sprintf(str, "Layer     : %s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;	
				case 5: 		     //step
					sprintf(str, "Step     : %s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;
				case 10:             //ip
					sprintf(str, "IP:%s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;	
				case 11:            //mac
					sprintf(str, "MAC:%s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;
				case 12:            //server ip
					sprintf(str, "SVR-IP:%s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;
				case 15:            //region
					sprintf(str, "Region:%s", tmp);
					DrawOneString(str, line++);
					memset( str, 0, sizeof(str) );
					break;

					
				default:
					break;

			}
			
			tmp = strtok(NULL, separator); 
			count++;
		}
		
		 
	}
	FullScreenEnd();

	pclose( readfp );
	
	return;
}



