 #define NEW_FPGA_ADDR_COMM

#include "addr_base.h"

#ifdef NEW_FPGA_ADDR_COMM
/*-----------------------------------BASE DEFINE-------------------------------*/
/*
_FPGA_ADDR(slot,iotype,bdtype,offset)
取值范围：
slot: 0-31(0x1F)
iotype:0-ouput 1-input
bdtype:0-31(0x1f)
offset:0-31(0x1f)

high --------------------- low(16bit)
slot   iotype bdtype  offset
5(bit)  1       5       5



[1]. slot 0: 总共2048个寄存器地址，表示对槽位号不敏感的寄存器
   	  io: input or output
      bdtype 
	    [1.1]	0		common 共64个寄存器，表示小部分的公用寄存器, fpga test
		[1.2]   1-31 	共1984个寄存器，代表仅以板卡类型区分且对槽位号不敏感的寄存器，比如ptp相关寄存器
				        此处寄存器多个相同类型的板卡仅共享一个配置。

[2]. slot 1-20: 总共40960个寄存器地址，表示板卡寄存器
	bdtype：
		[2.1]    0		总共1280个寄存器，表示所有板卡都共有的通用寄存器，对板卡类型不敏感，比如输出信号类型寄存器OUT_SIGNAL
		[2.2]    1-31	总共39680个寄存器，表示各个板卡私有的寄存器。该部分支持多个板卡且每个板卡拥有不同的配置信息

[3]. slot 21-25 (reserved)： 总共：10240个寄存器，预留
[4]. slot 26-31： 总共：12288个寄存器，表示系统寄存器
	slot
		[4.1]	26		共2048个寄存器
		[4.2]	27		共2048个寄存器
				...
*/

#define FPGA_SYS2_COMMON_OFFSETMASK ((1 << (16-_FPGA_ADDR_SLOTBITS))-1) //2047
//[1.1] sys_offset:0-31(0x1F) 最多支持31个寄存器
#define FPGA_SYS1_INCOMMON(sys_offset) _FPGA_IN_ADDR(0, 0,((sys_offset) & _FPGA_ADDR_OFFSETMASK))
//[1.1] sys_offset:0-31 最多支持31个寄存器
#define FPGA_SYS1_OUTCOMMON(sys_offset) _FPGA_OUT_ADDR(0, 0,((sys_offset) & _FPGA_ADDR_OFFSETMASK))

//[4.1] sys_offset:0-2047(0x07FF) 最多支持2048个寄存器
#define FPGA_SYS2_COMMON(sys_offset) _FPGA_ADDR(26, 0, 0,((sys_offset) & FPGA_SYS2_COMMON_OFFSETMASK))

#if 0
//[4.2] 暂时够，不用再扩展
//sys_offset:0-2047
#define FPGA_SYS3_COMMON_OFFSETMASK ((1 << (16-_FPGA_ADDR_SLOTBITS))-1) //2047
#define FPGA_SYS3_COMMON(sys_offset) _FPGA_ADDR(27, 0, 0,(sys_offset & FPGA_SYS3_COMMON_OFFSETMASK))
#endif

//[1.2] in--- bidtype:1-31 offset:0-31 最多支持992个寄存器，
#define FPGA_IN_SLOT_COMMON(bdtype,offset) _FPGA_IN_ADDR(0,(bdtype) & _FPGA_ADDR_BDTYPEMASK,(offset) & _FPGA_ADDR_OFFSETMASK)
//[1.2] out--- bidtype:1-31 offset:0-31 最多支持992个寄存器
#define FPGA_OUT_SLOT_COMMON(bdtype,offset) _FPGA_OUT_ADDR(0,(bdtype) & _FPGA_ADDR_BDTYPEMASK,(offset) & _FPGA_ADDR_OFFSETMASK)

//[2.1] in--- slot:1-20(0x14) offset:0-31 最多支持640个寄存器
//目前in-slot取值范围为1-4，所以5-20可以作为SYS扩展公用
#define FPGA_IN_BDTYPE_COMMON(slot,offset) _FPGA_IN_ADDR((slot) & _FPGA_ADDR_SLOTMASK,0,(offset) & _FPGA_ADDR_OFFSETMASK)
//[2.1] out--- slot:1-20(0x14) offset:0-31 最多支持640个寄存器
//目前out-slot取值范围为1-16，所以17-20可以作为SYS扩展公用
#define FPGA_OUT_BDTYPE_COMMON(slot,offset) _FPGA_OUT_ADDR((slot) & _FPGA_ADDR_SLOTMASK,0,(offset) & _FPGA_ADDR_OFFSETMASK)

//[2.2] in--- slot:1-20(0x14) bdtype:1-31(0x1F) offset:0-31(0x1F) 最多支持19840个私有private寄存器
//目前in-slot取值范围为1-4，所以slot：5-20可以作为SYS扩展公用
//(注意：我们这里因为卫星板卡寄存器大于32个，所以卫星bdtype统一一起使用，范围为IDX_BDTYPE_IN_GPSBF-IDX_BDTYPE_IN_BFFI
//即bdtype：1-10)
#define FPGA_IN_PRIV_ADDR(slot, bdtype, offset) \
	_FPGA_IN_ADDR((slot) & _FPGA_ADDR_SLOTMASK, (bdtype) & _FPGA_ADDR_BDTYPEMASK, (offset) & _FPGA_ADDR_OFFSETMASK)
//[2.2] out--- slot:1-20(0x14) bdtype:1-31(0x1F)  offset:0-31(0x1F) 最多支持19840个私有private寄存器
//	目前out-slot取值范围为1-16，所以17-20可以作为SYS扩展公用
#define FPGA_OUT_PRIV_ADDR(slot, bdtype, offset) \
	_FPGA_OUT_ADDR((slot) & _FPGA_ADDR_SLOTMASK, (bdtype) & _FPGA_ADDR_BDTYPEMASK, (offset) & _FPGA_ADDR_OFFSETMASK)
/*-----------------------------------BASE DEFINE END-------------------------------*/

/*-----------------------------------[1.1]SYS1-------------------------------*/
//fpga test.
/*---------------------------------[1.1]SYS1 END----------------------------*/

/*-----------------------------------[4.1]SYS2-------------------------------*/
/* 设备类型 */
/*
  0x01	扩展框
  0x02	主框
*/
#define	FPGA_DEVICE_TYPE	FPGA_SYS2_COMMON(0x0001)
//弃用
//#define	FPGA_RESET_LCD	FPGA_SYS2_COMMON(0x0002)
/*-----------------*/
/* FPGA版本 */
/*-----------------*/
/*
  FPGA版本
  VAB.CD
  bit0~bit3表示A
  bit4~bit7表示B
  bit8~bit11表示C
  bit12~bit15表示D
*/
#define	FPGA_VER	FPGA_SYS2_COMMON(0x0002)

/* LED灯 */
/*
  BD : bit0-bit1
  00 灯灭 BD不可用
  01 灯亮 BD可用
  11 闪烁 选用BD
*/
#define	FPGA_LED_BD		FPGA_SYS2_COMMON(0x0003)


/*
  GPS : bit2-bit3
  00 灯灭 GPS不可用
  01 灯亮 GPS可用
  11 闪烁 选用GPS
*/
#define	FPGA_LED_GPS	FPGA_SYS2_COMMON(0x0004)

/*
  IRIGB : bit4-bit5
  00 灯灭 IRIGB不可用
  01 灯亮 IRIGB可用
  11 闪烁 选用IRIGB
*/
#define	FPGA_LED_IRIGB	FPGA_SYS2_COMMON(0x0005)

/*
  FREQ : bit6-bit7
  00 灯灭 FREQ不可用
  01 灯亮 FREQ可用
  11 闪烁 选用FREQ
*/
#define	FPGA_LED_FREQ	FPGA_SYS2_COMMON(0x0006)

/*
  ALARM : bit8-bit9
  00 灯灭 无告警
  01 灯亮 有告警
  11 保留
*/
#define	FPGA_LED_ALM	FPGA_SYS2_COMMON(0x0007)

/*
  RUN : bit10-bit11
  交替写00,01 闪烁
  11 保留
*/
#define	FPGA_LED_RUN	FPGA_SYS2_COMMON(0x0008)

/* 根据输出信号类型设置输出时延 */
#define	FPGA_SYS_OUT_TOD_DELAY_LOW		FPGA_SYS2_COMMON(0x0009)
#define	FPGA_SYS_OUT_TOD_DELAY_HIGH		FPGA_SYS2_COMMON(0x000A)

#define	FPGA_SYS_OUT_IRIGB_DELAY_LOW	FPGA_SYS2_COMMON(0x000B)
#define	FPGA_SYS_OUT_IRIGB_DELAY_HIGH	FPGA_SYS2_COMMON(0x000C)

#define	FPGA_SYS_OUT_PPX_DELAY_LOW		FPGA_SYS2_COMMON(0x000D)
#define	FPGA_SYS_OUT_PPX_DELAY_HIGH		FPGA_SYS2_COMMON(0x000E)

#define	FPGA_SYS_OUT_PTP_DELAY_LOW		FPGA_SYS2_COMMON(0x000F)
#define	FPGA_SYS_OUT_PTP_DELAY_HIGH		FPGA_SYS2_COMMON(0x0010)


/* 板卡ID */

/*
  寄存器表示槽位1-16的板卡ID（index：0-7）
  bit8~bit15表示槽位2*index+1的板卡ID
  bit0~bit7表示槽位2*index+2的板卡ID
*/

 #define FPGA_BID_REGBASE	(0x11)
//index：0-7，即0x0011-0x0018
#define	FPGA_BID_S01_TO_S16(index)	FPGA_SYS2_COMMON((FPGA_BID_REGBASE+index)) //0x0011-0x0018

/*
  槽位17的板卡ID
  bit0-bit7表示槽位17的板卡ID. alm board
*/
#define	FPGA_BID_S17		FPGA_SYS2_COMMON(0x0019)

/*
  槽位18、槽位19的板卡ID
  bit0-bit7表示槽位19的板卡ID
  bit8-bit15表示槽位18的板卡ID: power board
*/
#define	FPGA_BID_S18_S19	FPGA_SYS2_COMMON(0x001A)

/*
  槽位18[1]、槽位19[0]的电源板卡告警
  bit0表示槽位19的电源板卡告警
  bit1表示槽位18的电源板卡告警
  1	告警
  0	正常
*/
#define	FPGA_PWR_ALM		FPGA_SYS2_COMMON(0x001B)

/*
  槽位18的电源板卡频率
*/
#define	FPGA_1ST_PWR_FREQ		FPGA_SYS2_COMMON(0x001C)

/*
  槽位19的电源板卡频率
*/
#define	FPGA_2ND_PWR_FREQ		FPGA_SYS2_COMMON(0x001D)

/*
  蓄电池
*/
#define	FPGA_INT_PWR_VOL		FPGA_SYS2_COMMON(0x001E)

/*
  电源频率的整数部分
*/
#define	FPGA_PWR_FREQ_INT		FPGA_SYS2_COMMON(0x001F)

/*
  电源频率的小数部分
*/
#define	FPGA_PWR_FREQ_FRA		FPGA_SYS2_COMMON(0x0020)


/*--------------------*/
/* IP1725 */
/*--------------------*/
#define	FPGA_SYS_IP1725_ADDR	FPGA_SYS2_COMMON(0x0021)
#define	FPGA_SYS_IP1725_VAL	FPGA_SYS2_COMMON(0x0022)
#define	FPGA_SYS_IP1725_REG_OP	FPGA_SYS2_COMMON(0x0023)
#define	FPGA_SYS_IP1725_READ	FPGA_SYS2_COMMON(0x0024)

#define	FPGA_KEY_TABLE	FPGA_SYS2_COMMON(0x0025)

/*----------*/
/* 告警板卡 */
/*---------*/
/*
  bit0~bit7
*/
#define	FPGA_SYS_OUT_ALM_TAG	FPGA_SYS2_COMMON(0x0026)

/*--------------*/
/* 输入选源 */
/*--------------*/
/*
  输入选源，bit0-bit16
  
(0x1<<0)  0x01	#1 卫星输入
(0x2<<0)  0x02	#1 2mh输入
(0x3<<0)  0x03	#1 2mb输入
(0x4<<0)  0x04	#1 irigb1输入
(0x5<<0)  0x05	#1 irigb2输入
(0x6<<0)  0x06	#1 ptp  1pps+tod输入
(0x7<<0)  0x07	#1 ntp输入
(0x8<<0)  0x08	#1 1pps输入
(0x9<<0)  0x09	#1 10mh1输入
(0xA<<0)  0x0A	#1 10mh2输入

(0x1<<4)  0x10	#2 卫星输入
(0x2<<4)  0x20	#2 2mh输入
(0x3<<4)  0x30	#2 2mb输入
(0x4<<4)  0x40	#2 irigb1输入
(0x5<<4)  0x50	#2 irigb2输入
(0x6<<4)  0x60	#2 ptp  1pps+tod输入
(0x7<<4)  0x70	#2 ntp输入
(0x8<<4)  0x80	#2 1pps输入
(0x9<<4)  0x90	#2 10mh1输入
(0xA<<4)  0xA0	#2 10mh2输入

(0x1<<8)  0x100	#3 卫星输入
(0x2<<8)  0x200	#3 2mh输入
(0x3<<8)  0x300	#3 2mb输入
(0x4<<8)  0x400	#3 irigb1输入
(0x5<<8)  0x500	#3 irigb2输入
(0x6<<8)  0x600	#3 ptp  1pps+tod输入
(0x7<<8)  0x700	#3 ntp输入
(0x8<<8)  0x800	#3 1pps输入
(0x9<<8)  0x900	#3 10mh1输入
(0xA<<8)  0xA00	#3 10mh2输入

(0x1<<12)  0x1000	#4 卫星输入
(0x2<<12)  0x2000	#4 2mh输入
(0x3<<12)  0x3000	#4 2mb输入
(0x4<<12)  0x4000	#4 irigb1输入
(0x5<<12)  0x5000	#4 irigb2输入
(0x6<<12)  0x6000   #4 ptp 1pps+tod输入
(0x7<<12)  0x7000	#4 ntp输入
(0x8<<12)  0x8000	#4 1pps输入
(0x9<<12)  0x9000	#4 10mh1输入
(0xA<<12)  0xA000	#4 10mh2输入
  (~0x0)  0xFFFF	输入不可用、无输入
*/
#define	FPGA_SYS_INSRC_SELECT	FPGA_SYS2_COMMON(0x0027)

/*
  时钟源等级，bit0-bit7
  
  0x00			一级钟
  
  0x01			二级钟
  
  0x02			三级钟
  
  0x03-0xFF		保留
*/
#define	FPGA_SYS_INSRC_STRATUM	FPGA_SYS2_COMMON(0x0028)

/*---------------*/
/* 选源后系统鉴相值*/
/*---------------*/
/*
  bit0~bit15表示输入源或者系统鉴相值的bit0~bit15
*/
#define	FPGA_SYS_PH_LOW16	FPGA_SYS2_COMMON(0x0029)

/*
  bit0~bit11表示输入源或者系统鉴相值的bit16~bit23，输入源或者系统鉴相值的bit23为符号位，1表示负，0表示正
*/
#define	FPGA_SYS_PH_HIGH12	FPGA_SYS2_COMMON(0x002A)

/*
  时间锁存寄存器，bit0-bit15
*/
#define	FPGA_SYS_LOCK_TIME	FPGA_SYS2_COMMON(0x002B)

/*
  时间刷新寄存器，bit0-bit15
*/
#define	FPGA_SYS_FRESH_TIME	FPGA_SYS2_COMMON(0x002C)

/*
  1900-现在的秒，bit0-bit15     
*/
#define	FPGA_SYS_HIGH_SECOND	FPGA_SYS2_COMMON(0x002D)

#define	FPGA_SYS_MED_SECOND	FPGA_SYS2_COMMON(0x002E)

#define	FPGA_SYS_LOW_SECOND	FPGA_SYS2_COMMON(0x002F)

/*fpga us寄存器 */
#define	FPGA_SYS_HIGH_MICO_SEC	FPGA_SYS2_COMMON(0x0030)
#define	FPGA_SYS_LOW_MICO_SEC	FPGA_SYS2_COMMON(0x0031)

/*
  闰秒寄存器，bit0-bit15
*/
#define	FPGA_SYS_LPS	FPGA_SYS2_COMMON(0x0032)

/*
  旧接口-闰秒预告，bit0 1路irigb bit1 2路irigb，fpga未使用
*/
#define	FPGA_SYS_LEAP_FORECAST	FPGA_SYS2_COMMON(0x0033)


//输入源是否可用 0-ok, 1-not
#define FPGA_SYS_INSRC_VALID	FPGA_SYS2_COMMON(0x0034)

/*
  钟类型
  1		RB
  2		XO
*/
#define	FPGA_RBXO_TYP	FPGA_SYS2_COMMON(0x0035)

/*
  钟状态
  0		自由运行
  1,2	保持
  3		快捕
  4+	锁定
*/
#define	FPGA_RBXO_STA	FPGA_SYS2_COMMON(0x0036)

/*
*----------
* LCD
*----------
*/
#define	FPGA_LCD_WE		FPGA_SYS2_COMMON(0x0037)
#define	FPGA_LCD_ADDR 	FPGA_SYS2_COMMON(0x0038)
#define	FPGA_LCD_DATA	FPGA_SYS2_COMMON(0x0039)
#define	FPGA_LCD_WREADY	FPGA_SYS2_COMMON(0x003A)

/*
*----------
* IRIGB 输出相关
*----------
*/

/*
   sys zone for the out irigb
  bit4, 0表示'+', 1表示'-'
  bit0-bit3表示时区
*/
#define	FPGA_SYS_OUT_IRIGB_ZONE	FPGA_SYS2_COMMON(0x003B)

/*
  bit0-bit2表示幅度比
  0		3:1
  1		4:1
  2		5:1
  3		6:1
*/
#define	FPGA_SYS_OUT_IRIGB_AR	FPGA_SYS2_COMMON(0x003C)

/*
  bit0-bit3表示电压
  0		3V
  1		4V
  2		5V
  3		6V
  4		7V
  5		8V
  6		9V
  7		10V
  8		11V
  9		12V
*/
#define	FPGA_SYS_OUT_IRIGB_V	FPGA_SYS2_COMMON(0x003D)

/*
  irigb闰秒预告
  (bit0)	闰秒预告，在闰秒变化前59s置1，闰秒变化后置0
  (bit1)	闰秒标志，0表示正闰秒，1表示负闰秒
*/
#define	FPGA_SYS_OUT_IRIGB_LEAP_FORECAST	FPGA_SYS2_COMMON(0x003E)
/*
  夏令时, bit0-bit1
*/
#define	FPGA_SYS_OUT_IRIGB_DST	FPGA_SYS2_COMMON(0x003F)

/*
*----------
* 电力tod信号输出相关
*----------
*/
/*
  电力TOD状态标志, bit0-bit15
  (bit0)	闰秒标志，0表示正闰秒，1表示负闰秒
  (bit1)	闰秒预告，在闰秒变化前59s置1，闰秒变化后置0
  (bit2)	0
  (bit3)	0
  (bit4)	时区偏移值符号位，0表示+，1表示-
  (bit5)	半小时时区偏移，0表示不增加，1表示时间偏移值额外增加0.5
  (bit6)	夏令时标志，夏令时期间置1
  (bit7)	夏令时预告，夏令时切换前59s置1，夏令时切换后置0
  (bit8-bit11)	时区偏移值(串口报文时间与UTC时间的差值)
  (bit12-bit15)	时间质量
  				0x0	正常工作状态，时钟同步正常			(钟跟踪)
  				0x1	时钟同步异常，时间准确度优于1ns
  				0x2	时钟同步异常，时间准确度优于10ns
  				0x3	时钟同步异常，时间准确度优于100ns
  				0x4	时钟同步异常，时间准确度优于1μs
  				0x5	时钟同步异常，时间准确度优于10μs	(钟保持 <24小时)
  				0x6	时钟同步异常，时间准确度优于100μs
  				0x7	时钟同步异常，时间准确度优于1ms
  				0x8	时钟同步异常，时间准确度优于10ms	(钟保持 >24小时)
  				0x9	时钟同步异常，时间准确度优于100ms
  				0xA	时钟同步异常，时间准确度优于1s		(钟快捕)
  				0xB	时钟同步异常，时间准确度优于10s
  				0xF	时钟严重故障，时间信息不可信		(钟自由)
*/
#define	FPGA_SYS_OUT_TOD_TAG	FPGA_SYS2_COMMON(0x0040)

/*
  电力TOD波特率, bit0-bit3
  0x00	1200
  0x01	2400
  0x02	4800
  0x03	9600
  0x04	19200
*/
#define	FPGA_SYS_OUT_TOD_BR	FPGA_SYS2_COMMON(0x0041)

/*
  PPS状态，bit0-bit7
  0x00 正常
  0x01 时间同步设备(原子钟)保持
  0x02 不可用
  0x03 时间同步设备(高稳晶振)保持
  0x04 传输承载设备保持
  保留
*/
#define	FPGA_SYS_OUT_TOD_PPS_STATE	FPGA_SYS2_COMMON(0x0042)

/*
  PPS抖动量级，bit0-bit7，0x00-0xFF
  0x00 – 0ns
  0x01 - 15ns
  0x02 - 30ns
  ...
  0xFF - 无意义
*/
#define	FPGA_SYS_OUT_TOD_PPS_JITTER	FPGA_SYS2_COMMON(0x0043)

//TOD/PTP
/*
  时钟源类型，bit0-bit7
  0x00	北斗
  0x01	GPS
  0x02	1588/ptp
  0x03	其他
*/
#define	FPGA_SYS_OUT_TOD_CLK_SRC_TYPE	FPGA_SYS2_COMMON(0x0044)

/*
  时钟源状态，bit0-bit15
  0x00 = no fix 未锁定
  0x01 = dead reckoning only 已知点模式
  0x02 = 2D-fix 二维模式
  0x03 = 3D-fix 三维模式
  0x04 = GPS + dead reckoning combined 卫星+已知点混合模式
  0x05 = Time only fix 保留
  0x06..0xff = reserved
*/
#define	FPGA_SYS_OUT_TOD_CLK_SRC_STATE	FPGA_SYS2_COMMON(0x0045)

/*TOD/PTP out need satellite status
  时钟源告警，bit0-bit15
  Bit0	: not used
  Bit1	: Antenna open
  Bit2	: Antenna shorted
  Bit3	: Not tracking satellites
  Bit4	: not used
  Bit5	: Survey-in progress GPS单星授时模式
  Bit6	: no stored position 没有存储到卫星信息
  Bit7	: Leap second pending 闰秒等待状态
  Bit8	: In test mode
  Bit9	: Position is questionable 存储的位置信息可疑
  Bit10	: not used
  Bit11	: Almanac not complete 星历不完整
  Bit12	: PPS was generated 保留
*/
#define	FPGA_SYS_OUT_TOD_CLK_SRC_ALARM	FPGA_SYS2_COMMON(0x0046)

/*----------------*/
/* 槽位1-16的PTP、PTPF板卡 */
/*----------------*/
/*
  写使能，bit0，1表示写
*/
#define	FPGA_SYS_OUT_PTP_WEN		FPGA_SYS2_COMMON(0x0047)

/*
  写地址，bit0-bit7
*/
#define	FPGA_SYS_OUT_PTP_WADDR		FPGA_SYS2_COMMON(0x0048)

/*
  写数据，bit0-bit7
*/
#define	FPGA_SYS_OUT_PTP_WVALUE		FPGA_SYS2_COMMON(0x0049)

/*
  PTP写完成数据状态，bit0，1表示写数据完成，0表示正在写数据
*/
#define	FPGA_SYS_OUT_PTP_WREADY		FPGA_SYS2_COMMON(0x004A)
/*看门狗功能暂未使用*/
/*看门狗使能*/
#define	FPGA_SYS_WDG_ENABLE	FPGA_SYS2_COMMON(0x004B)
/*看门狗喂狗*/
#define	FPGA_SYS_WDG_WDI	FPGA_SYS2_COMMON(0x004C)

/*检测fpga运行状态*/
#define FPGA_CHECK_RUN_STATUS	FPGA_SYS2_COMMON(0x004D)

/*
  out 2mb SA，bit0-bit7
*/
#define	FPGA_SYS_2MB_OUT_SA	FPGA_SYS2_COMMON(0x004E)

/*
  out 2mb SSM，bit0-bit7
*/
#define	FPGA_SYS_2MB_OUT_SSM	FPGA_SYS2_COMMON(0x004F)

/*
  in 2mh/10mh/1pps等无 SSM的频率源的用户ssm设置值，bit0-bit7 用于当前输入源的ssm等级查询和写入，主要用于频率源.此寄存器软件自用
*/
#define FPGA_SYS_CUR_INSRC_SSM	FPGA_SYS2_COMMON(0x0050)


/*---------------------------THE [4.1]SYS2 END-------------------------------*/



/*-----------------------------------[1.2]SLOT(0)_COMMON-------------------------------*/
//////////
/*---------------------------THE [1.2]SLOT(0)_COMMON END-------------------------------*/




/*-----------------------------------[2.1]BOARDTYPE(0)_COMMON-------------------------------*/

/*
必须要判断板卡，因为某些板卡无此信号时，寄存器值是随机值。
  输入信号状态，bit0~bit4，0表示有输入，1表示无输入
  bit0表示2mb，bit1表示2mh，bit2表示irigb1，bit3表示irigb2，bit4表示参考输入（卫星，ptp，ntp, rtf:1pps+tod）
  bit5表示1pps，bit6表示10mh1,bit7表示10mh2;
*/
//slot:1-4
#define	FPGA_IN_SIGNAL(slot)	FPGA_IN_BDTYPE_COMMON(slot, 0x00)

/*
  bit0-bit15表示1PPS+TOD/PTP_IN延时补偿的bit0-bit15
*/
#define	FPGA_IN_TOD_DELAY_LOW16(slot)	FPGA_IN_BDTYPE_COMMON(slot, 0x01)

/*
  bit0-bit11表示1PPS+TOD/PTP_IN延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_TOD_DELAY_HIGH12(slot)	FPGA_IN_BDTYPE_COMMON(slot, 0x02)

/*鉴相值暂时不用*/
/*
  bit0~bit15表示1PPS+TOD/PTP_IN鉴相值的bit0~bit15
*/
#define	FPGA_IN_TOD_PH_LOW16(slot)	FPGA_IN_BDTYPE_COMMON(slot, 0x03)

/*
  bit0~bit11表示1PPS+TOD/PTP_IN鉴相值的bit16~bit23，bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_TOD_PH_HIGH12(slot)	FPGA_IN_BDTYPE_COMMON(slot, 0x04)

/*-----------------------------------*/
/* 槽位1-16输出板卡 */
/*-----------------------------------*/
/*
  输出信号类型，bit0~bit3表示输出信号类型
  0000表示不输出
  0001表示NTP
  0010表示PTP
  0011表示2MH
  0100表示2MB
  0101表示IRIGB-DC
  0110表示TODT
  0111表示TODF
  1000表示PPS
  1001表示PPM
  1010表示PPH
  1011表示IRIGB-AC
  1100表示EXT(extern IRIGB-AC)
  1101表示10MHZ NEW
*/
//	#define	FPGA_S01_OUT_SIGNAL 	FpgaAddr(BaseAddr, 0x0080)
//fpgatype[]  slot: 1-16
#define	FPGA_OUT_SIGNAL(slot)	FPGA_OUT_BDTYPE_COMMON(slot, 0x00)




/*---------------------------THE [2.1]BOARDTYPE(0)_COMMON END-------------------------------*/



/*-----------------------------------[2.2]PRIV_ADDR-------------------------------*/
//我们这里，因为卫星板卡寄存器大于32个，所以卫星bdtype统一一起使用，范围为IDX_BDTYPE_IN_GPSBF-IDX_BDTYPE_IN_BFFI
//即bdtype：1-10

//卫星板卡的所有信号的鉴相值暂时保留，网页上给用户读取值作隐藏处理
/*-------------*/
/* 槽位1-4输入板卡 */
/*-------------*/

/*-------slot:1-4  bdtype:IDX_BDTYPE_IN_GPSBF:1   offset:0x00-0x1F------*/

/*
  bit0~bit15表示GPS/BD鉴相值的bit0~bit15
*/
//slot:1-4  bdtype:1   offset:0x00
#define	FPGA_IN_GB_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x00)

/*
  bit0~bit11表示GPS/BD鉴相值的bit16~bit23，GPS/BD鉴相值的bit23为符号位，1表示负，0表示正
*/
//slot:1-4  bdtype:1   offset:0x01
#define	FPGA_IN_GB_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x01)

/*
  bit0~bit15表示2MHz鉴相值的bit0~bit15
*/
//slot:1-4  bdtype:1   offset:0x02
#define	FPGA_IN_2MH_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x02)

/*
  bit0~bit11表示2MHz鉴相值的bit16~bit23，2MHz鉴相值的bit23为符号位，1表示负，0表示正
*/
//slot:1-4  bdtype:1   offset:0x03
#define	FPGA_IN_2MH_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x03)

/*
  bit0~bit15表示2Mbit/s鉴相值的bit0~bit15
*/
//slot:1-4  bdtype:1   offset:0x04
#define	FPGA_IN_2MB_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x04)
/*
  bit0~bit11表示2Mbit/s鉴相值的bit16~bit23，2Mbit/s鉴相值的bit23为符号位，1表示负，0表示正
*/
//slot:1-4  bdtype:1   offset:0x05
#define	FPGA_IN_2MB_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x05)

/*
  bit0~bit15表示第一路IRIGB鉴相值的bit0~bit15
*/
#define	FPGA_IN_IRIGB1_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x06)

/*
  bit0~bit11表示第一路IRIGB鉴相值的bit16~bit23，第一路IRIGB鉴相值的bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_IRIGB1_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x07)

/*
  bit0~bit15表示第二路IRIGB鉴相值的bit0~bit15
*/
#define	FPGA_IN_IRIGB2_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x08)

/*
  bit0~bit11表示第二路IRIGB鉴相值的bit16~bit23，第二路IRIGB鉴相值的bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_IRIGB2_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x09)


/*
  bit0-bit15表示接收机延时补偿的bit0-bit15
*/
#define	FPGA_IN_GB_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x0A)

/*
  bit0-bit11表示接收机延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_GB_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x0B)

/*
  bit0-bit15表示2MH补偿的bit0-bit15
*/
#define	FPGA_IN_2MH_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x0C)
/*
  bit0-bit11表示2MH延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_2MH_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x0D)

/*
  bit0-bit15表示2MB补偿的bit0-bit15
*/
#define	FPGA_IN_2MB_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x0E)
/*
  bit0-bit11表示2MB延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_2MB_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x0F)

/*
  bit0-bit15表示irigb1延时补偿的bit0-bit15
*/
#define	FPGA_IN_IRIGB1_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x10)

/*
  bit0-bit11表示irigb1延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_IRIGB1_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x11)

//INCREASE
/*
  irigb闰秒预告
  (bit0)	闰秒预告，在闰秒变化前59s置1，闰秒变化后置0
  (bit1)	闰秒标志，0表示正闰秒，1表示负闰秒
*/
#define	FPGA_IN_IRIGB1_LPS(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x12)

/*
  夏令时, bit0-bit1
*/
#define	FPGA_IN_IRIGB1_DSP(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x13)

/*(bit0-bit3) 时间质量
			  0x0 正常工作状态，时钟同步正常		  (钟跟踪)
			  0x1 时钟同步异常，时间准确度优于1ns
			  0x2 时钟同步异常，时间准确度优于10ns
			  0x3 时钟同步异常，时间准确度优于100ns
			  0x4 时钟同步异常，时间准确度优于1μs
			  0x5 时钟同步异常，时间准确度优于10μs (钟保持 <24小时)
			  0x6 时钟同步异常，时间准确度优于100μs
			  0x7 时钟同步异常，时间准确度优于1ms
			  0x8 时钟同步异常，时间准确度优于10ms  (钟保持 >24小时)
			  0x9 时钟同步异常，时间准确度优于100ms
			  0xA 时钟同步异常，时间准确度优于1s		  (钟快捕)
			  0xB 时钟同步异常，时间准确度优于10s
			  0xF 时钟严重故障，时间信息不可信	  (钟自由)
*/
#define	FPGA_IN_IRIGB1_QUALITY(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x14)
//INCREASE END

/*
  bit0-bit15表示irigb2延时补偿的bit0-bit15
*/
#define	FPGA_IN_IRIGB2_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x15)

/*
  bit0-bit11表示irigb2延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_IRIGB2_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x16)

/*
  irigb闰秒预告
  (bit0)	闰秒预告，在闰秒变化前59s置1，闰秒变化后置0
  (bit1)	闰秒标志，0表示正闰秒，1表示负闰秒
*/
#define	FPGA_IN_IRIGB2_LPS(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x17)

/*
  夏令时, bit0-bit1
*/
#define	FPGA_IN_IRIGB2_DSP(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x18)

/*(bit0-bit3) 时间质量
			  0x0 正常工作状态，时钟同步正常		  (钟跟踪)
			  0x1 时钟同步异常，时间准确度优于1ns
			  0x2 时钟同步异常，时间准确度优于10ns
			  0x3 时钟同步异常，时间准确度优于100ns
			  0x4 时钟同步异常，时间准确度优于1μs
			  0x5 时钟同步异常，时间准确度优于10μs (钟保持 <24小时)
			  0x6 时钟同步异常，时间准确度优于100μs
			  0x7 时钟同步异常，时间准确度优于1ms
			  0x8 时钟同步异常，时间准确度优于10ms  (钟保持 >24小时)
			  0x9 时钟同步异常，时间准确度优于100ms
			  0xA 时钟同步异常，时间准确度优于1s		  (钟快捕)
			  0xB 时钟同步异常，时间准确度优于10s
			  0xF 时钟严重故障，时间信息不可信	  (钟自由)
*/
#define	FPGA_IN_IRIGB2_QUALITY(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x19)


/*
  接收机读使能，bit0，1表示读
*/
#define	FPGA_IN_GB_REN(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x1A)

/*
  接收机读地址，bit0~bit7
*/
#define	FPGA_IN_GB_RADDR(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x1B)

/*
  接收机读数据，bit0~bit7
*/
#define	FPGA_IN_GB_RVALUE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x1C)

/*
  接收机写使能，bit0，1表示写
*/
#define	FPGA_IN_GB_WEN(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x1D)

/*
  接收机写地址，bit0~bit7
*/
#define	FPGA_IN_GB_WADDR(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x1E)

/*
  接收机写数据，bit0~bit7
*/
#define	FPGA_IN_GB_WVALUE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBF, 0x1F)

/*-------------slot:1-4  bdtype:IDX_BDTYPE_IN_GPSBF:1   offset:0x00-0x1F END-----*/
/*-------------slot:1-4  bdtype:IDX_BDTYPE_IN_GPSBE:2   offset:0x00-0x1F-----*/
/*
  接收机数据写完成
*/
#define	FPGA_IN_GB_WREADY(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBE, 0x00)


/*
  2mb告警，bit0-bit3，1表示有告警，0表示无告警
  bit0表示BPV告警
  bit1表示AIS告警
  bit2表示SYNC告警
  bit3表示CRC告警
*/
#define	FPGA_IN_2MB_ALM(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBE, 0x01)

/*
  2mb SA，bit0-bit7
*/
#define	FPGA_IN_2MB_SA(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBE, 0x02)

/*
  2mb SSM，bit0-bit7
*/
#define	FPGA_IN_2MB_SSM(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_GPSBE, 0x03)

/*-------------slot:1-4  bdtype:IDX_BDTYPE_IN_GPSBE:2   offset:0x00-0x1F END-----*/

/*-------slot:1-4  bdtype: IDX_BDTYPE_IN_PTP_IN:11 offset:0x00-0x1F---------- */
/*
  PTP闰秒，bit0-bit8
*/
#define	FPGA_IN_TOD_LEAP(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x00)

//INCREASE
/*
  PPS状态，bit0-bit7
  0x00 正常
  0x01 时间同步设备(原子钟)保持
  0x02 不可用
  0x03 时间同步设备(高稳晶振)保持
  0x04 传输承载设备保持
  保留
*/
#define	FPGA_IN_TOD_PPS_STATE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x01)

/*
  PPS抖动量级，bit0-bit7，0x00-0xFF
  0x00 – 0ns
  0x01 - 15ns
  0x02 - 30ns
  ...
  0xFF - 无意义
*/
#define	FPGA_IN_TOD_PPS_JITTER(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x02)

//TOD/PTP
/*
  时钟源类型，bit0-bit7
  0x00	北斗
  0x01	GPS
  0x02	1588/ptp
  0x03	其他
*/
#define	FPGA_IN_TOD_CLK_SRC_TYPE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x03)

/*
  时钟源状态，bit0-bit15
  0x00 = no fix 未锁定
  0x01 = dead reckoning only 已知点模式
  0x02 = 2D-fix 二维模式
  0x03 = 3D-fix 三维模式
  0x04 = GPS + dead reckoning combined 卫星+已知点混合模式
  0x05 = Time only fix 保留
  0x06..0xff = reserved
*/
#define	FPGA_IN_TOD_CLK_SRC_STATE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x04)

/*TOD/PTP out need satellite status
  时钟源告警，bit0-bit15
  Bit0	: not used
  Bit1	: Antenna open
  Bit2	: Antenna shorted
  Bit3	: Not tracking satellites
  Bit4	: not used
  Bit5	: Survey-in progress GPS单星授时模式
  Bit6	: no stored position 没有存储到卫星信息
  Bit7	: Leap second pending 闰秒等待状态
  Bit8	: In test mode
  Bit9	: Position is questionable 存储的位置信息可疑
  Bit10	: not used
  Bit11	: Almanac not complete 星历不完整
  Bit12	: PPS was generated 保留
*/
#define	FPGA_IN_TOD_CLK_SRC_ALARM(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x05)

/*
  读使能，bit0，1表示读
*/
#define	FPGA_IN_PTP_REN(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x06)

/*
  读地址，bit0~bit7
*/
#define	FPGA_IN_PTP_RADDR(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x07)

/*
  读数据，bit0~bit7
*/
#define	FPGA_IN_PTP_RVALUE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x08)
/*
  写使能，bit0，1表示写
*/
#define	FPGA_IN_PTP_WEN(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x09)

/*
  写地址，bit0-bit7
*/
#define	FPGA_IN_PTP_WADDR(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x0A)

/*
  写数据，bit0-bit7
*/
#define	FPGA_IN_PTP_WVALUE(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x0B)

/*
  接收机数据写完成
*/
#define	FPGA_IN_PTP_WREADY(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x0C)
//INCREASE END

/*
1# PTP输入告警
0x00 =正常
0x01 =时间同步设备（原子钟）保持
0x02 =不可用
0x03 =时间同步设备（高稳晶振）保持
0x04 =传输承载设备保持
其它保留
*/
//	#define	FPGA_S01_PTP_IN(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_PTP_IN, 0x00)

/*-------slot:1-4  bdtype: IDX_BDTYPE_IN_PTP_IN(11) offset:0x00-0x1F END-------*/

/*-------slot:1-4  bdtype: IDX_BDTYPE_IN_RTF:12 offset:0x00-0x1F---------- */
/*TOD在公用部分，延时补偿和信号类型有关，所以以后仍然可以公用，只要区分板卡即可*/
/*
  bit0-bit15表示1PPS延时补偿的bit0-bit15
*/
#define	FPGA_IN_1PPS_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x00)

/*
  bit0-bit11表示1PPS延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_1PPS_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x01)

/*
  bit0-bit15表示10MH1延时补偿的bit0-bit15
*/
#define	FPGA_IN_10MH1_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x02)

/*
  bit0-bit11表示10MH1延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_10MH1_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x03)

/*
  bit0-bit15表示10MH2延时补偿的bit0-bit15
*/
#define	FPGA_IN_10MH2_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x04)

/*
  bit0-bit11表示10MH2延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_10MH2_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x05)


/*鉴相值暂时不用*/
/*
  bit0~bit15表示1PPS鉴相值的bit0~bit15
*/
#define	FPGA_IN_1PPS_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x06)

/*
  bit0~bit11表示1PPS鉴相值的bit16~bit23，bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_1PPS_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x07)

/*
  bit0~bit15表示10MH1鉴相值的bit0~bit15
*/
#define	FPGA_IN_10MH1_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x08)

/*
  bit0~bit11表示10MH1鉴相值的bit16~bit23，bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_10MH1_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x09)

/*
  bit0~bit15表示10MH2鉴相值的bit0~bit15
*/
#define	FPGA_IN_10MH2_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x0A)

/*
  bit0~bit11表示10MH2鉴相值的bit16~bit23，bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_10MH2_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_RTF, 0x0B)


/*-------slot:1-4  bdtype: IDX_BDTYPE_IN_RTF(12) offset:0x00-0x1F END-------*/


/*-------slot:1-4  bdtype: IDX_BDTYPE_IN_NTP_IN:13 offset:0x00-0x1F---------- */

/*
  bit0-bit15表示ntp_in延时补偿的bit0-bit15
*/
#define	FPGA_IN_NTP_IN_DELAY_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_NTP_IN, 0x00)

/*
  bit0-bit11表示ntp_in延时补偿的bit16-bit28，bit28为符号位，1表示负，0表示正
*/
#define	FPGA_IN_NTP_IN_DELAY_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_NTP_IN, 0x01)


/*
  表示NTP_IN时间值是否有效,1-valid 0-invalid
*/
#define	FPGA_IN_NTP_IN_TIME_VAILD(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_NTP_IN, 0x02)

/*鉴相值暂时不用*/
/*
  bit0~bit15表示NTP_IN鉴相值的bit0~bit15
*/
#define	FPGA_IN_NTP_IN_PH_LOW16(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_NTP_IN, 0x03)

/*
  bit0~bit11表示NTP_IN鉴相值的bit16~bit23，bit23为符号位，1表示负，0表示正
*/
#define	FPGA_IN_NTP_IN_PH_HIGH12(slot)	FPGA_IN_PRIV_ADDR(slot, IDX_BDTYPE_IN_NTP_IN, 0x04)


/*-------slot:1-4  bdtype: IDX_BDTYPE_IN_NTP_IN(11) offset:0x00-0x1F END-------*/

/*---------------------------THE [2.2]PRIV_ADDR END-------------------------------*/


#endif

