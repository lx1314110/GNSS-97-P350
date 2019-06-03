#ifndef __DLCD_MENU__
#define	__DLCD_MENU__


#define LCD_DOT_X       128
#define LCD_DOT_Y       64
#define LCD_FONT_X      6
#define LCD_FONT_Y      8
#define LCD_LINE        8

void TransferCommad(u16_t line, u16_t offset, u16_t data);
void TransferData(u16_t line, u16_t offset, u16_t data);
void OneLineBegin(char line);
void DrawOneString(char *str,int line) ;
void FullScreenEnd();
void FullScreenClear(void);
void ShowMenuChange(char line);


#endif//__DLCD_CMD__

