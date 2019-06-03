#ifndef __DLCD_CTRL__
#define __DLCD_CTRL__



#include "dlcd_context.h"


typedef struct  _cmdMenu
{
	char		*shortDesc;		    /* short description */
	void        (*cmd_func)();		/* Command function */
	char		cmdOptionType;	   /* command option type */
	char 		level;				/* menu level */
	char		group;				/* group */
	char		order;				/* function order */	

} cmdMenu;



int GetP350Init();
void LcdTimeoutShow();

int GetInputValue(int* num, int min, int max);
int Prockeylcd(struct lcdCtx *ctx);



void LCD_ViewVersion(void);
void LCD_ViewHelp(void);
void LCD_SystemConfig(void);
void LCD_AlarmConfig(void);
void LCD_InputConfig(void);
void LCD_OutputConfig(void);
int LCD_Get_Schema(int slot);
void LCD_View_Status(void);
void LCD_View_Time(void);
void LCD_View_Leap(void);
void LCD_View_Net(void);
void LCD_View_Clock(void);
void LCD_View_Btype(void);
void LCD_View_Alarm(void);
void LCD_View_AlarmHistory(void);
void LCD_View_Priority(void);
void LCD_View_Delay(void);
void LCD_View_SSM_SA(void);
void LCD_View_Signal(void);
void LCD_View_Mode(void);
void LCD_View_Elevation(void);
void LCD_View_Lola(void);
void LCD_View_Output(void);

#endif//__DLCD_CTRL__


