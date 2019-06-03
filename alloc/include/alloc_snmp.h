#include "lib_bit.h"

#define SMP_SID_SUB_NUM	(5)


//alm sub
#define SMP_SID_SUB_SYS 	(0x01)	//跟槽位端口无关 衍生1个 索引编号规则
#define SMP_SID_SUB_PWR 	(0x02) //跟槽位端口无关 衍生2个
#define SMP_SID_SUB_SAT 	(0x05)	//卫星   跟槽位号有关端口无关(其实可以跟端口有关，就像gbdbe,可以绑定在1端口，具体见sat_comm_almtbl)	INPUT_MAX_SLOT个
#define SMP_SID_SUB_IN		(0x06)	//输入		INPUT_MAX_SLOT*INPUT_MAX_PORT个
#define SMP_SID_SUB_OUT 	(0x08)	//输出     	MAX_SLOT*OUTPUT_MAX_PORT个

//SUB slot
#define SID_SYS_SLOT 1		//跟槽位号无关，但是函数initializeAlarmIndexTable为了统计，必须在系统内部设置为非0，只是显示给用户时设置为0

//alm sub port
#define SID_SYS_PORT 1
#define SID_SAT_PORT 1
#define SID_PWR_PORT 1

//alm id
#define SNMP_ALM_ID_ALL_REF_LOS		(SMP_SID_SUB_SYS<<25 | 1<<24 | 0x0003<<8 | 0x00)	/*ALL Reference LOS Alarm*//*所有输入丢失告警*/
#define SNMP_ALM_ID_XO_UNLOCK		(SMP_SID_SUB_SYS<<25 | 1<<24 | 0x0007<<8 | 0x01)	/*XO Unlock Alarm*//*晶体钟未锁定告警*/
#define SNMP_ALM_ID_RB_UNLOCK		(SMP_SID_SUB_SYS<<25 | 1<<24 | 0x0007<<8 | 0x02)	/*RB Unlock Alarm*//*铷钟未锁定告警*/
#define SNMP_ALM_ID_HW_FPGA_HALT	(SMP_SID_SUB_SYS<<25 | 1<<24 | 0x0007<<8 | 0x03)	/*HW FPGA Reg Alarm*//*CPU与HW FPGA通信异常告警*/
#define SNMP_ALM_ID_NTP_EXIT		(SMP_SID_SUB_SYS<<25 | 1<<24 | 0x0001<<8 | 0x00)		/*NTP Exit*//*NTP停止退出告警*/

#define SNMP_ALM_ID_EXT_PWR_INPUT	(SMP_SID_SUB_PWR<<25 | 1<<24 | 0x0003<<8 | 0x00)	/*External Power Input Alarm*//*电源输入故障告警*/
#define SNMP_ALM_ID_GPBD_SGN_LOS	(SMP_SID_SUB_SAT<<25 | 1<<24 | 0x0001<<8 | 0x00)	/*GPS/BD Signal LOS*//*GPS/BD卫星信号丢失告警*/
#define SNMP_ALM_ID_GPBD_SGN_DGD	(SMP_SID_SUB_SAT<<25 | 1<<24 | 0x0009<<8 | 0x00)	/*GPS/BD Signal Degrade*//*GPS/BD卫星信号劣化告警*/
#define SNMP_ALM_ID_ANT_OPEN		(SMP_SID_SUB_SAT<<25 | 1<<24 | 0x0003<<8 | 0x00)	/*Antenna Open*//*天馈线开路告警*/
#define SNMP_ALM_ID_ANT_SHORT		(SMP_SID_SUB_SAT<<25 | 1<<24 | 0x0004<<8 | 0x00)	/*Antenna Short*//*天馈线短路告警*/
#define SNMP_ALM_ID_GPBD_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x01)		/*GPS/BD Threshold Alarm*//*GPS/BD卫星输入信号性能超限*/
#define SNMP_ALM_ID_1PPS_TOD_THRSD	(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x02)		/*1PPS TOD Threshold Alarm*//*1PPS TOD输入信号性能超限*/	
#define SNMP_ALM_ID_1PPS_TOD_IN_LOS	(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x02)		/*1PPS TOD Input LOS*//*1PPS TOD输入信号丢失告警*/
#define SNMP_ALM_ID_1PPS_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x03)		/*1PPS Threshold Alarm*//*1PPS输入信号性能超限*/
#define SNMP_ALM_ID_1PPS_IN_LOS		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x03)		/*1PPS Input LOS*//*1PPS输入信号丢失告警*/
#define SNMP_ALM_ID_2MHZ_IN_LOS		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x06)		/*2048KHz Input LOS*//*2048KHz输入信号丢失告警*/
#define SNMP_ALM_ID_2MHZ_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x06)		/*2048KHz Threshold Alarm*//*2048KHz输入信号性能超限*/
#define SNMP_ALM_ID_10MHZ_IN_LOS	(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x07)		/*10MHz Input LOS*//*10MHz输入信号丢失告警*/
#define SNMP_ALM_ID_10MHZ_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x07)		/*10MHz Threshold Alarm*//*10MHz输入信号性能超限*/

#define SNMP_ALM_ID_2MB_IN_LOS		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x08)		/*2048Kbit/s Input LOS*//*2048Kbit/s输入信号丢失告警*/
#define SNMP_ALM_ID_2MB_AIS			(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0001<<8 | 0x00)		/*2048Kbit/s AIS*//*2048Kbit/s AIS*/
#define SNMP_ALM_ID_2MB_BPV			(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0002<<8 | 0x00)		/*2048Kbit/s BPV*//*2048Kbit/s BPV*/
#define SNMP_ALM_ID_2MB_CRC			(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0003<<8 | 0x00)		/*2048Kbit/s CRC*//*2048Kbit/s CRC*/
#define SNMP_ALM_ID_2MB_LOF			(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000B<<8 | 0x00)		/*2048Kbit/s LOF*//*2048Kbit/s LOF*/
#define SNMP_ALM_ID_2MB_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x08)		/*2048Kbit/s Threshold Alarm*//*2048Kbit/s输入信号性能超限*/
#define SNMP_ALM_ID_IRIGB_IN_LOS	(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x04)		/*IRIGB Input LOS*//*IRIGB输入信号丢失告警*/
#define SNMP_ALM_ID_IRIGB_IN_DGD	(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0011<<8 | 0x04)		/*IRIGB Degraded Alarm*//*B码输入信号降质告警*/
#define SNMP_ALM_ID_IRIGB_INVAID	(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0012<<8 | 0x04)		/*IRIGB Unavailable Alarm*//*B码输入信号不可用告警*/
#define SNMP_ALM_ID_IRIGB_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x04)		/*IRIGB Threshold Alarm*//*B 码输入信号性能超限*/
#define SNMP_ALM_ID_PTP_IN_LOS		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x05)		/*PTP Input LOS*//*PTP输入信号丢失告警*/
#define SNMP_ALM_ID_PTP_IN_DGD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0011<<8 | 0x05)		/*PTP Input Degraded*//*PTP信号劣化告警*/
#define SNMP_ALM_ID_PTP_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x05)		/*PTP Threshold Alarm*//*PTP输入信号性能超限告警*/	

//new add
#define SNMP_ALM_ID_NTP_IN_LOS		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0008<<8 | 0x09)		/*NTP Input LOS*//*NTP输入信号丢失告警*/
#define SNMP_ALM_ID_NTP_IN_DGD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x0011<<8 | 0x09)		/*NTP Input Degraded*//*NTP信号劣化告警 暂不支持*/
#define SNMP_ALM_ID_NTP_THRSD		(SMP_SID_SUB_IN<<25 | 1<<24 | 0x000C<<8 | 0x09)		/*NTP Threshold Alarm*//*NTP输入信号性能超限告警*/	
//end

#define SNMP_ALM_ID_LINK_DOWN		(SMP_SID_SUB_OUT<<25 | 1<<24 | 0x0006<<8 | 0x00)		/*Link Down*//*LinkDown告警*/


//one alarm info
struct snmp_alarm_info_t {
u8_t slot;
u8_t port;
u8_t bid;
u32_t alm_id;
u8_t *alm_msg;
};





/*************************************** EVENT **********************************************/

#define SMP_SID_SUB_CLK		(0x07)	//仅仅为事件,//跟槽位端口无关 衍生1个 索引编号规则

//sub slot
#define SID_CLK_SLOT 1		////跟槽位号无关，但是函数initializeAlarmIndexTable为了统计，必须在系统内部设置为非0，只是显示给用户时设置为0
//sub port
#define SID_CLK_PORT 1

enum {
	SMP_EVNT_GRP_NONE = 0,
	SMP_EVNT_GRP_SATE_MODSET = 1,
	SMP_EVNT_GRP_SATE_CLKSTA,
	SMP_EVNT_GRP_CLKSTA,
	SMP_EVNT_GRP_IN_CHSTA,
	SMP_EVNT_GRP_IN_MTSTA,
	SMP_EVNT_GRP_IN_CLK_QUAL_LVL,
	SMP_EVNT_GRP_OUT_CLK_QUAL_LVL,	//7
	SMP_EVNT_GRP_CURSOR_ARRAY_SIZE
};


//event id， num 45
#define SNMP_EVNT_ID_SYSTIME_SET	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0001<<8 | 0x00)	/*System Time Set*//*系统时间被设置no spt*/
#define SNMP_EVNT_ID_SYSTIME_UPDATE	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0002<<8 | 0x00)	/*System Time Update*//*系统时间被更新ok*/
#define SNMP_EVNT_ID_USR_CONF_LOSS	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0003<<8 | 0x00)	/*User Config Miss*//*用户配置数据文件丢失no spt*/
#define SNMP_EVNT_ID_SYSLEAP_SET	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0004<<8 | 0x00)	/*GPS Offset Value Set*//*系统闰秒（GPS OFFSET）被设置ok*/
#define SNMP_EVNT_ID_SYSLEAP_UPDATE	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0005<<8 | 0x00)		/*GPS Offset Value Update*//*系统闰秒（GPS OFFSET）被更新ok*/
#define SNMP_EVNT_ID_SYSIP_SET		(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0006<<8 | 0x00)	/*IP Address Set*//*本机IP地址被设置ok*/
#define SNMP_EVNT_ID_TRPSIP_SET		(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0007<<8 | 0x00)	/*rap Server IP Address Set*//*Trap服务器IP地址被设置*/
#define SNMP_EVNT_ID_TRPSIP_UPDATE	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0008<<8 | 0x00)	/*Trap Server IP Address Update*//*Trap服务器IP地址被更新*/
#define SNMP_EVNT_ID_ALM_GENERATE	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x0009<<8 | 0x00)	/*Alarm generated event*//*有告警产生事件*/
#define SNMP_EVNT_ID_ALM_DISAPPEAR	(SMP_SID_SUB_SYS<<25 | 0<<24 | 0x000A<<8 | 0x00)	/*Alarm disappeared event*//*有告警消失事件*/

#define SNMP_EVNT_ID_SAT_GPS		(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0001<<8 | 0x02)	/*GPS*//*单GPS模式被设置*//*start SMP_EVNT_GRP_SATE_MODSET ok*/
#define SNMP_EVNT_ID_SAT_BD			(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0001<<8 | 0x03)	/*BD*//*单北斗模式被设置*/
#define SNMP_EVNT_ID_SAT_MGPS		(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0001<<8 | 0x06)	/*MIX GPS*//*混合模式，主GPS被设置*/
#define SNMP_EVNT_ID_SAT_MBD		(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0001<<8 | 0x07)	/*MIX BD*//*混合模式，主BD被设置*//*end SMP_EVNT_GRP_SATE_MODSET*/	
#define SNMP_EVNT_ID_SAT_LNUM		(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0002<<8 | 0x00)	/*SAT_NUM LESS*//*可视卫星数量少于四颗ok*/
#define SNMP_EVNT_ID_SAT_LOCK		(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0004<<8 | 0x00)	/*SAT_LOCKED*//*卫星模块锁相环锁定*//*start SMP_EVNT_GRP_SATE_CLKSTA ok*/
#define SNMP_EVNT_ID_SAT_FREE		(SMP_SID_SUB_SAT<<25 | 0<<24 | 0x0005<<8 | 0x00)	/*SAT_FREE*//*卫星模块锁相环自由运行*//*end SMP_EVNT_GRP_SATE_CLKSTA*/

#define SNMP_EVNT_ID_CLK_BRIDGE		(SMP_SID_SUB_CLK<<25 | 0<<24 | 0x0001<<8 | 0x00)	/*Bridging*//*桥接（判源）*//*start SMP_EVNT_GRP_CLKSTA ok*/
#define SNMP_EVNT_ID_CLK_FAST		(SMP_SID_SUB_CLK<<25 | 0<<24 | 0x0002<<8 | 0x00)	/*Fast*//*快捕*/
#define SNMP_EVNT_ID_CLK_FREE		(SMP_SID_SUB_CLK<<25 | 0<<24 | 0x0003<<8 | 0x00)	/*Free*//*自由运行*/
#define SNMP_EVNT_ID_CLK_LOCK		(SMP_SID_SUB_CLK<<25 | 0<<24 | 0x0004<<8 | 0x00)	/*Locked*//*锁定*/	
#define SNMP_EVNT_ID_CLK_WARMUP		(SMP_SID_SUB_CLK<<25 | 0<<24 | 0x0005<<8 | 0x00)	/*Warm Up*//*预热*/
#define SNMP_EVNT_ID_CLK_HOLD		(SMP_SID_SUB_CLK<<25 | 0<<24 | 0x0006<<8 | 0x00)	/*Hold*//*保持*//*end SMP_EVNT_GRP_CLKSTA*/

#define SNMP_EVNT_ID_IN_CH_AVA		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0001<<8 | 0x00)		/*IN CH_AVA*//*通道状态可用*//*start SMP_EVNT_GRP_IN_CHSTA */
#define SNMP_EVNT_ID_IN_CH_MAST		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0005<<8 | 0x00)		/*IN CH_MAST*//*通道主用*/
#define SNMP_EVNT_ID_IN_CH_STBY		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0006<<8 | 0x00)		/*IN CH_STBY*//*通道备用*/
#define SNMP_EVNT_ID_IN_CH_UNAVA	(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0007<<8 | 0x00)		/*IN CH_UNAVA*//*通道状态不可用*//*end SMP_EVNT_GRP_IN_CHSTA*/
#define SNMP_EVNT_ID_IN_MT_AIS		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0008<<8 | 0x00)		/*IN MT_AIS*//*监测源告警指示*//*start SMP_EVNT_GRP_IN_MTSTA*/
#define SNMP_EVNT_ID_IN_MT_BPV		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0009<<8 | 0x00)		/*IN MT_BPV*//*监测源双极性破坏*/
#define SNMP_EVNT_ID_IN_MT_CRC		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x000A<<8 | 0x00)		/*IN MT_CRC*//*监测源循环冗余校验*/
#define SNMP_EVNT_ID_IN_MT_FREQ		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x000C<<8 | 0x00)		/*IN MT_FREQ*//*监测源频率越限*/
#define SNMP_EVNT_ID_IN_MT_LOS		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x000E<<8 | 0x00)		/*IN MT_LOS*//*监测源丢失*//*end SMP_EVNT_GRP_IN_MTSTA*/
#define SNMP_EVNT_ID_IN_REF_ACTIVE	(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0015<<8 | 0x00)		/*REF ACTIVE*//*参考源主用*/


#define SNMP_EVNT_ID_IN_RPC			(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0016<<8 | 0x01)		/*IN PRC*//*1级基准时钟质量等级*//*start SMP_EVNT_GRP_IN_CLK_QUAL_LVL*/
#define SNMP_EVNT_ID_IN_UNK			(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0016<<8 | 0x02)		/*IN UNK*//*质量等级未知*/
#define SNMP_EVNT_ID_IN_SSUT		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0016<<8 | 0x03)		/*IN SSUT*//*2级节点时钟质量等级*/
#define SNMP_EVNT_ID_IN_SSUL		(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0016<<8 | 0x04)		/*IN SSUL*//*3级节点时钟质量等级*/
#define SNMP_EVNT_ID_IN_SEC			(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0016<<8 | 0x05)		/*IN SEC*//*SDH设备时钟质量等级*/
#define SNMP_EVNT_ID_IN_DNU			(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0016<<8 | 0x06)		/*IN DNU*//*质量等级不可用*//*end SMP_EVNT_GRP_IN_CLK_QUAL_LVL*/
#define SNMP_EVNT_ID_IN_REF_SWAP	(SMP_SID_SUB_IN<<25 | 0<<24 | 0x0017<<8 | 0x00)		/*REF SWAP*//*参考源倒换*/

#define SNMP_EVNT_ID_OUT_RPC		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0001<<8 | 0x01)	/*OUT PRC*//*1级基准时钟质量等级*//*start SMP_EVNT_GRP_OUT_CLK_QUAL_LVL*/
#define SNMP_EVNT_ID_OUT_UNK		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0001<<8 | 0x02)	/*OUT UNK*//*质量等级未知*/
#define SNMP_EVNT_ID_OUT_SSUT		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0001<<8 | 0x03)	/*OUT SSUT*//*2级节点时钟质量等级*/
#define SNMP_EVNT_ID_OUT_SSUL		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0001<<8 | 0x04)	/*OUTSSUL*//*3级节点时钟质量等级*/
#define SNMP_EVNT_ID_OUT_SEC		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0001<<8 | 0x05)	/*OUTSEC*//*SDH设备时钟质量等级*/
#define SNMP_EVNT_ID_OUT_DNU		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0001<<8 | 0x06)	/*OUT DNU*//*质量等级不可用*//*end SMP_EVNT_GRP_OUT_CLK_QUAL_LVL*/

#define SNMP_EVNT_ID_LINK_DOWN		(SMP_SID_SUB_OUT<<25 | 0<<24 | 0x0002<<8 | 0x00)	/*Link Down*//*链路中断 ok*/


