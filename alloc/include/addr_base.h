#ifndef __ADDR__
#define	__ADDR__

#include "asm/addr_base.h"


/*#############################################new define####################*/
//the index of board identifier for the fpga
//OUT_INDEX 输出板卡类型索引【1-19】
enum {
	//none
	IDX_BDTYPE_OUT_NONE	= 0,//无盘 COMMON
	
	//out
	IDX_BDTYPE_OUT_NTP		= 1,
	IDX_BDTYPE_OUT_NTPF,
	IDX_BDTYPE_OUT_RS232,
	IDX_BDTYPE_OUT_RS485,
	IDX_BDTYPE_OUT_BDCT,//此盘已经取消，预留
	IDX_BDTYPE_OUT_BAC, //板卡功能：直流b码转成交流b码
	IDX_BDTYPE_OUT_SF4,
	IDX_BDTYPE_OUT_BDC485, //此盘已经取消，预留
	IDX_BDTYPE_OUT_KJD8,
	IDX_BDTYPE_OUT_TTL8,	//10
	IDX_BDTYPE_OUT_PPX485, //此盘已经取消，预留
	IDX_BDTYPE_OUT_PTP,
	IDX_BDTYPE_OUT_PTPF, //此盘取消，预留
	IDX_BDTYPE_OUT_OUTE, //此盘取消，预留
	IDX_BDTYPE_OUT_OUTH, //此盘取消，预留
	//61850 independent
	IDX_BDTYPE_OUT_61850,
	//new 20171220
	IDX_BDTYPE_OUT_KJD4,
	IDX_BDTYPE_OUT_TP4,	//1pps+tod outboard new
	IDX_BDTYPE_OUT_SC4,	//2MHz/2Mbit/10MHz outboard //19

	//alarm
	//FPGA_BDTYPE_OUT_ALM,	//20
	
	//FPGA_BDTYPE_OUT_ARRAY_SIZE	//20
};
#define FPGA_BDTYPE_OUT_ARRAY_SIZE IDX_BDTYPE_OUT_SC4

//IN_INDEX 输入板卡类型索引【1-13】, 不能超过31
enum {
	//none
	IDX_BDTYPE_IN_NONE	= 0,//无盘 COMMON

	//in
	IDX_BDTYPE_IN_GPSBF = 1, //reserved satellite start //1
	IDX_BDTYPE_IN_GPSBE,	//reserved
	IDX_BDTYPE_IN_GBDBF,	//GPS BD Fiber(光口)
	IDX_BDTYPE_IN_GBDBE,	//GPS BD Elec(电口)
	IDX_BDTYPE_IN_BDBF,	//reserved
	IDX_BDTYPE_IN_BDBE,	//reserved satellite end
	IDX_BDTYPE_IN_BEI,	//reserved
	IDX_BDTYPE_IN_BFI,	//reserved
	IDX_BDTYPE_IN_BFEI,	////1路2Mhz/2Mbit 和两路IRIGB
	IDX_BDTYPE_IN_BFFI,	////1路2Mhz/2Mbit 和两路IRIGB //10
	IDX_BDTYPE_IN_PTP_IN,
	//new 20171220
	IDX_BDTYPE_IN_RTF, //1PPS、10MHz、2MHz REFER INBOARD //12
	IDX_BDTYPE_IN_NTP_IN,
	//power
	//FPGA_BDTYPE_IN_PWRM,
	//FPGA_BDTYPE_IN_PWRD,
	//FPGA_BDTYPE_IN_PWRL,//此盘取消，预留   //15
	
	//FPGA_BDTYPE_IN_ARRAY_SIZE	//14
};
#define FPGA_BDTYPE_IN_ARRAY_SIZE	IDX_BDTYPE_IN_NTP_IN


/*============================new define end================================*/


/*----------------------------------------------------------------------------------------------------------*/
/* 寄存器地址为4字节，以2字节为单位读/写寄存器 */
/*----------------------------------------------------------------------------------------------------------*/
#define	BaseAddr	0x00000000

#define	FpgaAddr(BASE, OFFSET)	(BASE + OFFSET)

#endif//__ADDR__


