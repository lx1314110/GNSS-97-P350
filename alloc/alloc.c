#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "lib_dbg.h"
#include "alloc.h" 

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define dbg_printf(fmt,args...)	printf (fmt ,##args)
#else
#define dbg_printf(args...) do {} while (0)
#endif

struct timesource_tbl_t gTimeSourceTbl[TIME_SOURCE_LEN] = {
	{ GPS_SOURCE_ID(1),	G_GPS_SOURCE_VAL(1),	GPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, BOTH_SOURCE_FLAG, "GPS/BD", 	"1# GPS/BD input"},
	{ S2MH_SOURCE_ID(1),G_S2MH_SOURCE_VAL(1),	S2MH_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, FREQ_SOURCE_FLAG, "2MH",		"1# 2MH input"},
	{ S2MB_SOURCE_ID(1),G_S2MB_SOURCE_VAL(1),	S2MB_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, FREQ_SOURCE_FLAG, "2MB",		"1# 2MB input"},
	{ IRIGB1_SOURCE_ID(1),G_IRIGB1_SOURCE_VAL(1),IRIGB1_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, BOTH_SOURCE_FLAG, "IRIGB",	"1# 1st IRIGB input"},
	{ IRIGB2_SOURCE_ID(1),G_IRIGB2_SOURCE_VAL(1),IRIGB2_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, BOTH_SOURCE_FLAG, "IRIGB",	"1# 2nd IRIGB input"},
	{ TOD_SOURCE_ID(1),	G_TOD_SOURCE_VAL(1),	TOD_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, BOTH_SOURCE_FLAG, "TOD",		"1# 1PPS+TOD/PTP input"},
	{ S1PPS_SOURCE_ID(1),	G_S1PPS_SOURCE_VAL(1),	S1PPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, FREQ_SOURCE_FLAG, "1PPS",	"1# 1PPS input"},
	{ S10MH1_SOURCE_ID(1),	G_S10MH1_SOURCE_VAL(1), S10MH1_SOURCE_TYPE, INPUT_SLOT_CURSOR_1+1, FREQ_SOURCE_FLAG, "10MH",	"1# 1st 10MH input"},
	{ S10MH2_SOURCE_ID(1),	G_S10MH2_SOURCE_VAL(1), S10MH2_SOURCE_TYPE, INPUT_SLOT_CURSOR_1+1, FREQ_SOURCE_FLAG, "10MH",	"1# 2nd 10MH input"},
	{ NTP_SOURCE_ID(1),	G_NTP_SOURCE_VAL(1),	NTP_SOURCE_TYPE,	INPUT_SLOT_CURSOR_1+1, TIME_SOURCE_FLAG, "NTP",		"1# NTP input"},


	{ GPS_SOURCE_ID(2),	G_GPS_SOURCE_VAL(2),	GPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, BOTH_SOURCE_FLAG, "GPS/BD",	"2# GPS/BD input"},
	{ S2MH_SOURCE_ID(2),G_S2MH_SOURCE_VAL(2),	S2MH_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, FREQ_SOURCE_FLAG, "2MH",		"2# 2MH input"},
	{ S2MB_SOURCE_ID(2),G_S2MB_SOURCE_VAL(2),	S2MB_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, FREQ_SOURCE_FLAG, "2MB",		"2# 2MB input"},
	{ IRIGB1_SOURCE_ID(2),G_IRIGB1_SOURCE_VAL(2),IRIGB1_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, BOTH_SOURCE_FLAG, "IRIGB",	"2# 1st IRIGB input"},
	{ IRIGB2_SOURCE_ID(2),G_IRIGB2_SOURCE_VAL(2),IRIGB2_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, BOTH_SOURCE_FLAG, "IRIGB",	"2# 2nd IRIGB input"},
	{ TOD_SOURCE_ID(2),	G_TOD_SOURCE_VAL(2),	TOD_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, BOTH_SOURCE_FLAG, "TOD",		"2# 1PPS+TOD/PTP input"},
	{ S1PPS_SOURCE_ID(2),	G_S1PPS_SOURCE_VAL(2),	S1PPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, FREQ_SOURCE_FLAG, "1PPS",	"2# 1PPS input"},
	{ S10MH1_SOURCE_ID(2),	G_S10MH1_SOURCE_VAL(2),	S10MH1_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, FREQ_SOURCE_FLAG, "10MH",	"2# 1st 10MH input"},
	{ S10MH2_SOURCE_ID(2),	G_S10MH2_SOURCE_VAL(2),	S10MH2_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, FREQ_SOURCE_FLAG, "10MH",	"2# 2nd 10MH input"},
	{ NTP_SOURCE_ID(2),	G_NTP_SOURCE_VAL(2),	NTP_SOURCE_TYPE,	INPUT_SLOT_CURSOR_2+1, TIME_SOURCE_FLAG, "NTP",		"2# NTP input"},
#ifdef NEW_3_4_IN_SLOT
	{ GPS_SOURCE_ID(3),	G_GPS_SOURCE_VAL(3),	GPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, BOTH_SOURCE_FLAG, "GPS/BD",	"3# GPS/BD input"},
	{ S2MH_SOURCE_ID(3),G_S2MH_SOURCE_VAL(3),	S2MH_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, FREQ_SOURCE_FLAG, "2MH",		"3# 2MH input"},
	{ S2MB_SOURCE_ID(3),G_S2MB_SOURCE_VAL(3),	S2MB_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, FREQ_SOURCE_FLAG, "2MB",		"3# 2MB input"},
	{ IRIGB1_SOURCE_ID(3),G_IRIGB1_SOURCE_VAL(3),IRIGB1_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, BOTH_SOURCE_FLAG, "IRIGB",	"3# 1st IRIGB input"},
	{ IRIGB2_SOURCE_ID(3),G_IRIGB2_SOURCE_VAL(3),IRIGB2_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, BOTH_SOURCE_FLAG, "IRIGB",	"3# 2nd IRIGB input"},
	{ TOD_SOURCE_ID(3),	G_TOD_SOURCE_VAL(3),	TOD_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, BOTH_SOURCE_FLAG, "TOD",		"3# 1PPS+TOD/PTP input"},//ptp_in show PTP
	{ S1PPS_SOURCE_ID(3),	G_S1PPS_SOURCE_VAL(3),	S1PPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, FREQ_SOURCE_FLAG, "1PPS",	"3# 1PPS input"},
	{ S10MH1_SOURCE_ID(3),	G_S10MH1_SOURCE_VAL(3), S10MH1_SOURCE_TYPE, INPUT_SLOT_CURSOR_3+1, FREQ_SOURCE_FLAG, "10MH",	"3# 1st 10MH input"},
	{ S10MH2_SOURCE_ID(3),	G_S10MH2_SOURCE_VAL(3), S10MH2_SOURCE_TYPE, INPUT_SLOT_CURSOR_3+1, FREQ_SOURCE_FLAG, "10MH",	"3# 2nd 10MH input"},
	{ NTP_SOURCE_ID(3),	G_NTP_SOURCE_VAL(3),	NTP_SOURCE_TYPE,	INPUT_SLOT_CURSOR_3+1, TIME_SOURCE_FLAG, "NTP",		"3# NTP input"},

	{ GPS_SOURCE_ID(4),	G_GPS_SOURCE_VAL(4),	GPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, BOTH_SOURCE_FLAG, "GPS/BD",	"4# GPS/BD input"},
	{ S2MH_SOURCE_ID(4),G_S2MH_SOURCE_VAL(4),	S2MH_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, FREQ_SOURCE_FLAG, "2MH",		"4# 2MH input"},
	{ S2MB_SOURCE_ID(4),G_S2MB_SOURCE_VAL(4),	S2MB_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, FREQ_SOURCE_FLAG, "2MB",		"4# 2MB input"},
	{ IRIGB1_SOURCE_ID(4),G_IRIGB1_SOURCE_VAL(4),IRIGB1_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, BOTH_SOURCE_FLAG, "IRIGB",	"4# 1st IRIGB input"},
	{ IRIGB2_SOURCE_ID(4),G_IRIGB2_SOURCE_VAL(4),IRIGB2_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, BOTH_SOURCE_FLAG, "IRIGB",	"4# 2nd IRIGB input"},
	{ TOD_SOURCE_ID(4),	G_TOD_SOURCE_VAL(4),	TOD_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, BOTH_SOURCE_FLAG, "TOD",		"4# 1PPS+TOD/PTP input"},
	{ S1PPS_SOURCE_ID(4),	G_S1PPS_SOURCE_VAL(4),	S1PPS_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, FREQ_SOURCE_FLAG, "1PPS",	"4# 1PPS input"},
	{ S10MH1_SOURCE_ID(4),	G_S10MH1_SOURCE_VAL(4), S10MH1_SOURCE_TYPE, INPUT_SLOT_CURSOR_4+1, FREQ_SOURCE_FLAG, "10MH",	"4# 1st 10MH input"},
	{ S10MH2_SOURCE_ID(4),	G_S10MH2_SOURCE_VAL(4), S10MH2_SOURCE_TYPE, INPUT_SLOT_CURSOR_4+1, FREQ_SOURCE_FLAG, "10MH",	"4# 2nd 10MH input"},
	{ NTP_SOURCE_ID(4),	G_NTP_SOURCE_VAL(4),	NTP_SOURCE_TYPE,	INPUT_SLOT_CURSOR_4+1, TIME_SOURCE_FLAG, "NTP",		"4# NTP input"},

#endif
	{ TIME_SOURCE_MAX_NOSOURCE, NO_SOURCE,	NO_SOURCE_TYPE, 0, INVALID_SOURCE_FLAG,	"NONE",	"NOSOURCE"}
};

char gSrcFlagTbl[TIME_SRC_FLAG_ARRAY_LEN][8] = {
	"none",
	"time",
	"freq",
	"both"
};


/* 
  进程索引
*/
u8_t gDaemonTbl[DAEMON_CURSOR_ARRAY_SIZE][16] = {	"none\0",
													"p350_input\0",
												 	"p350_output\0",
												 	"p350_ntp\0",
												 	"p350_alarm\0",
												 	"p350_clock\0",
												 	"p350_manager\0",
												 	"p350_keylcd\0",
													"p350_wg\0" ,
												 	"p350_61850\0",
												 	"p350_telnet\0",
												 	"p350_http\0" 
													};








char gSignalTbl[SIGNAL_ARRAY_SIZE][10] = {	SIGNAL_TYPE_NONE, 
											SIGNAL_TYPE_NTP,
											SIGNAL_TYPE_PTP,
											SIGNAL_TYPE_2MH,
											SIGNAL_TYPE_2MB,
											SIGNAL_TYPE_IRIGB_DC,
											SIGNAL_TYPE_TODT,
											SIGNAL_TYPE_TODF,
											SIGNAL_TYPE_PPS,
											SIGNAL_TYPE_PPM,
											SIGNAL_TYPE_PPH,
											SIGNAL_TYPE_IRIGB_AC,
											SIGNAL_TYPE_EXT,
											SIGNAL_TYPE_10MH};



struct outdelay_tbl_t gOutdelayTbl[OUTDELAY_SIGNALTYPE_LEN] =
{
	{OUTDELAY_SIGNAL_TOD, "tod"},
	{OUTDELAY_SIGNAL_IRIGB, "irigb"},
	{OUTDELAY_SIGNAL_PPX, "ppx"},
	{OUTDELAY_SIGNAL_PTP, "ptp"},
};






u8_t gPdtTbl[PTP_DELAY_ARRAY_SIZE][4] = { PTP_DELAY_TYPE_P2P, PTP_DELAY_TYPE_E2E };
u8_t gPmodeTbl[PTP_MODEL_ARRAY_SIZE][7] = { PTP_MODEL_TYPE_MASTER, PTP_MODEL_TYPE_SLAVE };
u8_t gPmutTbl[PTP_MULTI_UNI_ARRAY_SIZE][6] = { PTP_MULTI_UNI_TYPE_UNI, PTP_MULTI_UNI_TYPE_MULTI };
u8_t gPltTbl[PTP_LAYER_ARRAY_SIZE][7] = { PTP_LAYER_TYPE_L2, PTP_LAYER_TYPE_L3 };
u8_t gPstTbl[PTP_STEP_ARRAY_SIZE][6] = { PTP_STEP_TYPE_1STEP, PTP_STEP_TYPE_2STEP };
u8_t gFreqTbl[PTP_FREQ_ARRAY_SIZE][6] = {	PTP_FREQ_256_1,
											PTP_FREQ_128_1,
											PTP_FREQ_64_1,
											PTP_FREQ_32_1,
											PTP_FREQ_16_1,
											PTP_FREQ_8_1,
											PTP_FREQ_4_1,
											PTP_FREQ_2_1,
											PTP_FREQ_1_1,
											PTP_FREQ_1_2,
											PTP_FREQ_1_4,
											PTP_FREQ_1_8,
											PTP_FREQ_1_16,
											PTP_FREQ_1_32,
											PTP_FREQ_1_64,
											PTP_FREQ_1_128,
											PTP_FREQ_1_256	};
u8_t gtraceTbl[PTP_TRACE_ARRAY_SIZE][2] = { PTP_ZERO, PTP_ONE };




#if 1
#define LEN_SSM_TBL  ARRAY_SIZE(gSsmTbl)
struct tbl_ssm_t gSsmTbl[] = {
	{SSM_CURSOR_02, SSM_VAL_02, SSM_02},
	{SSM_CURSOR_04, SSM_VAL_04, SSM_04},
	{SSM_CURSOR_08, SSM_VAL_08, SSM_08},
	{SSM_CURSOR_0B, SSM_VAL_0B, SSM_0B},
	{SSM_CURSOR_0F, SSM_VAL_0F, SSM_0F},
	{SSM_CURSOR_00, SSM_VAL_00, SSM_00},
	{SSM_CURSOR_NE, SSM_VAL_NE, SSM_NE},
};
int gSsmTbl_len = LEN_SSM_TBL;
#else
u8_t gSsmTbl[SSM_CURSOR_ARRAY_SIZE][3] = {
	SSM_02,
	SSM_04,
	SSM_08,
	SSM_0B,
	SSM_0F,
	SSM_00
};
#endif








u8_t gSaTbl[SA_CURSOR_ARRAY_SIZE][4] = {
	SA_SA4,
	SA_SA5,
	SA_SA6,
	SA_SA7,
	SA_SA8
};









u8_t gModeTbl[MODE_CURSOR_ARRAY_SIZE][3] = {
	MODE_GPS,
	MODE_BD,
	MODE_MIX_GPS,
	MODE_MIX_BD
};








u8_t gBaudRateTbl[BAUDRATE_ARRAY_SIZE][8] = {
	BAUDRATE_TYPE_1200,
	BAUDRATE_TYPE_2400,
	BAUDRATE_TYPE_4800,
	BAUDRATE_TYPE_9600,
	BAUDRATE_TYPE_19200
};









u8_t gZoneTbl[24][5] = {
	ZONE_TYPE_ZERO,
	ZONE_TYPE_E1,
	ZONE_TYPE_E2,
	ZONE_TYPE_E3,
	ZONE_TYPE_E4,
	ZONE_TYPE_E5,
	ZONE_TYPE_E6,
	ZONE_TYPE_E7,
	ZONE_TYPE_E8,
	ZONE_TYPE_E9,
	ZONE_TYPE_E10,
	ZONE_TYPE_E11,
	ZONE_TYPE_EW12,
	ZONE_TYPE_W11,
	ZONE_TYPE_W10,
	ZONE_TYPE_W9,
	ZONE_TYPE_W8,
	ZONE_TYPE_W7,
	ZONE_TYPE_W6,
	ZONE_TYPE_W5,
	ZONE_TYPE_W4,
	ZONE_TYPE_W3,
	ZONE_TYPE_W2,
	ZONE_TYPE_W1
};

u8_t gAlarmStatusTbl[2][16] = {
	"normal", 
	"fault"
};









u8_t gAmplitudeRatioTbl[AMPLITUDE_RATIO_ARRAY_SIZE][4] = {
	AMPLITUDE_RATIO_TYPE_3_1,
	AMPLITUDE_RATIO_TYPE_4_1,
	AMPLITUDE_RATIO_TYPE_5_1,
	AMPLITUDE_RATIO_TYPE_6_1
};









u8_t gVoltageTbl[VOLTAGE_ARRAY_SIZE][4] = {
	VOLTAGE_TYPE_3V,
	VOLTAGE_TYPE_4V,
	VOLTAGE_TYPE_5V,
	VOLTAGE_TYPE_6V,
	VOLTAGE_TYPE_7V,
	VOLTAGE_TYPE_8V,
	VOLTAGE_TYPE_9V,
	VOLTAGE_TYPE_10V,
	VOLTAGE_TYPE_11V,
	VOLTAGE_TYPE_12V
};

char *gBdTypeTbl[10] = {	NM_BOARD_TYPE_NONE, 
							NM_BOARD_TYPE_INPUT,
							NM_BOARD_TYPE_OUTPUT,
							NM_BOARD_TYPE_PWR,
							NM_BOARD_TYPE_ALM,
							NULL};


struct phase_perf_tbl_t gPhasePerfTbl[TBL_PHASE_PERF_CURRENT_ARRAY_SIZE];
int gPhasePerfTbl_len = TBL_PHASE_PERF_CURRENT_ARRAY_SIZE;


/*###########################################################################
*	new struct type
############################################################################*/
/*PUBLIC DATA TYPE: 
ONLY SUPPORT SMP_SID_SUB_SYS
*/
//----------------------------

//private alm
// support SMP_SID_SUB_SAT SMP_SID_SUB_IN SMP_SID_SUB_OUT SMP_SID_SUB_PWR

//ntp/ntpf盘私有告警类型
#define LEN_ATTR_NTP_ALMTBL  ARRAY_SIZE(ntp_almtbl)
static struct alm_attr_t ntp_almtbl[] = {
	//ALL PORT, 1F
	{SNMP_ALM_ID_LINK_DOWN, ((1<<(PORT_NTP+1))-1)}
};

//卫星通用私有告警类型支持
//only all SMP_SID_SUB_SAT和some SMP_SID_SUB_IN
#define LEN_ATTR_SAT_COMM_ALMTBL  ARRAY_SIZE(sat_comm_almtbl)
static struct alm_attr_t sat_comm_almtbl[] = {
	//SMP_SID_SUB_SAT
	{SNMP_ALM_ID_GPBD_SGN_LOS, BIT(SAT_PORT_SAT)},
	{SNMP_ALM_ID_GPBD_SGN_DGD, BIT(SAT_PORT_SAT)},
	{SNMP_ALM_ID_ANT_OPEN, BIT(SAT_PORT_SAT)},
	{SNMP_ALM_ID_ANT_SHORT, BIT(SAT_PORT_SAT)},
	//SMP_SID_SUB_IN
	{SNMP_ALM_ID_GPBD_THRSD, BIT(SAT_PORT_SAT)},
	{SNMP_ALM_ID_2MHZ_IN_LOS, BIT(SAT_PORT_2MH)},
	{SNMP_ALM_ID_2MHZ_THRSD, BIT(SAT_PORT_2MH)},
		
	{SNMP_ALM_ID_2MB_IN_LOS, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_AIS, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_BPV, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_CRC, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_LOF, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_THRSD, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_IRIGB_IN_LOS, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_IN_DGD, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_INVAID, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_THRSD,  BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
};

#define LEN_ATTR_SAT_BEI_ALMTBL  ARRAY_SIZE(sat_bei_bfi_almtbl)
static struct alm_attr_t sat_bei_bfi_almtbl[] = {
	//SMP_SID_SUB_SAT
	//SMP_SID_SUB_IN
	{SNMP_ALM_ID_IRIGB_IN_LOS, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_IN_DGD, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_INVAID, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_THRSD,  BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
};

#define LEN_ATTR_SAT_BFEI_ALMTBL  ARRAY_SIZE(sat_bfei_bffi_almtbl)
static struct alm_attr_t sat_bfei_bffi_almtbl[] = {
	//SMP_SID_SUB_SAT
	//SMP_SID_SUB_IN
	{SNMP_ALM_ID_2MHZ_IN_LOS, BIT(SAT_PORT_2MH)},
	{SNMP_ALM_ID_2MHZ_THRSD, BIT(SAT_PORT_2MH)},
		
	{SNMP_ALM_ID_2MB_IN_LOS, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_AIS, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_BPV, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_CRC, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_LOF, BIT(SAT_PORT_2MB)}, 
	{SNMP_ALM_ID_2MB_THRSD, BIT(SAT_PORT_2MB)},

	{SNMP_ALM_ID_IRIGB_IN_LOS, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_IN_DGD, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_INVAID, BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
	{SNMP_ALM_ID_IRIGB_THRSD,  BIT(SAT_PORT_IRIGB1) | BIT(SAT_PORT_IRIGB2)},
};

#define LEN_ATTR_PTPIN_ALMTBL  ARRAY_SIZE(ptpin_almtbl)
static struct alm_attr_t ptpin_almtbl[] = {
	//SMP_SID_SUB_IN, PORT1
	{SNMP_ALM_ID_PTP_IN_LOS, BIT(1)},
	{SNMP_ALM_ID_PTP_IN_DGD, BIT(1)},
	{SNMP_ALM_ID_PTP_THRSD, BIT(1)},
};

#define LEN_ATTR_RTF_ALMTBL  ARRAY_SIZE(rtf_almtbl)
static struct alm_attr_t rtf_almtbl[] = {
	//SMP_SID_SUB_IN, port1-4
	{SNMP_ALM_ID_1PPS_TOD_THRSD, BIT(RTF_PORT_1PPS_TOD)},
	{SNMP_ALM_ID_1PPS_TOD_IN_LOS, BIT(RTF_PORT_1PPS_TOD)},
	{SNMP_ALM_ID_1PPS_THRSD, BIT(RTF_PORT_1PPS)},
	{SNMP_ALM_ID_1PPS_IN_LOS, BIT(RTF_PORT_1PPS)},
	{SNMP_ALM_ID_10MHZ_THRSD, BIT(RTF_PORT_10MH1) | BIT(RTF_PORT_10MH2)},
	{SNMP_ALM_ID_10MHZ_IN_LOS, BIT(RTF_PORT_10MH1) | BIT(RTF_PORT_10MH2)},
};

#define LEN_ATTR_NTPIN_ALMTBL  ARRAY_SIZE(ntpin_almtbl)
static struct alm_attr_t ntpin_almtbl[] = {
	//SMP_SID_SUB_IN, PORT1
	{SNMP_ALM_ID_NTP_IN_LOS, BIT(1)},
	{SNMP_ALM_ID_NTP_IN_DGD, BIT(1)},
	{SNMP_ALM_ID_NTP_THRSD, BIT(1)},
};


/*STA /INFO FLAG*/
//卫星通用私有数据
static struct sat_priv_data_t sat_comm_pdata = {
	.sta_flag = FLAG_SAT_ALL_COMMSTA,
	.info_flag = FLAG_SAT_ALL_COMMINFO
};

static struct sat_priv_data_t sat_bei_bfi_pdata = {
	.sta_flag = FLAG_SAT_BEI_BFI_COMMSTA,
	.info_flag = FLAG_SAT_BEI_BFI_COMMINFO
};

static struct sat_priv_data_t sat_bfei_bffi_pdata = {
	.sta_flag = FLAG_SAT_BFEI_BFFI_COMMSTA,
	.info_flag = FLAG_SAT_BFEI_BFFI_COMMINFO
};


//长度必须和它的端口一一对应
static struct port_attr_t gBoardNonePortAttr[] = {
	{ NO_SOURCE_TYPE },
};

//编译检查，避免端口数量和选源信号长度定义不一致
//卫星
#if PORT_SAT == 5
static struct port_attr_t gBoardSatPortAttr[PORT_SAT] = {
	{ GPS_SOURCE_TYPE },
	{ S2MH_SOURCE_TYPE },
	{ S2MB_SOURCE_TYPE },
	{ IRIGB1_SOURCE_TYPE },
	{ IRIGB2_SOURCE_TYPE }
};
#else
#error "defined MAX port or macro INSRC TYPE LEN  error!"
#endif

#if PORT_SAT == 5
//LEN:PORT_SAT, cur 5
static struct port_attr_t gBoardSat_bei_bfi_PortAttr[PORT_SAT] = {
	{ NO_SOURCE_TYPE },
	{ NO_SOURCE_TYPE },
	{ NO_SOURCE_TYPE },
	{ IRIGB1_SOURCE_TYPE },
	{ IRIGB2_SOURCE_TYPE }
};
#else
#error "defined MAX port or macro INSRC TYPE LEN  error!"
#endif

#if PORT_SAT == 5
//LEN:PORT_SAT, cur 5
static struct port_attr_t gBoardSat_bffi_bfei_PortAttr[PORT_SAT] = {
	{ NO_SOURCE_TYPE },
	{ S2MH_SOURCE_TYPE },
	{ S2MB_SOURCE_TYPE },
	{ IRIGB1_SOURCE_TYPE },
	{ IRIGB2_SOURCE_TYPE }
};
#else
#error "defined MAX port or macro INSRC TYPE LEN  error!"
#endif

//PTP_IN
#if PORT_PTP_IN == 1
static struct port_attr_t gBoardPtpinPortAttr[PORT_PTP_IN] = {
	{ TOD_SOURCE_TYPE } //ptp
};
#else
#error "defined MAX port or macro INSRC TYPE LEN  error!"
#endif

//RTF
#if PORT_RTF == 4
//PORT_RTF:3,不够用无源填充
static struct port_attr_t gBoardRtfPortAttr[PORT_RTF] = {
	{ TOD_SOURCE_TYPE },//1pps+tod
	{ S1PPS_SOURCE_TYPE },
	{ S10MH1_SOURCE_TYPE },
	{ S10MH2_SOURCE_TYPE },
	
};
#else
#error "defined MAX port or macro INSRC TYPE LEN  error!"
#endif

//NTP_IN
#if PORT_NTP_IN == 1
static struct port_attr_t gBoardNtpinPortAttr[PORT_NTP_IN] = {
	{ NTP_SOURCE_TYPE }
};
#else
#error "defined MAX port or macro INSRC TYPE LEN  error!"
#endif


#define BOARD_ATTR_TBL_LEN  ARRAY_SIZE(gBoardAttrTbl)
struct boardattr_tbl_t gBoardAttrTbl[] = {
	{BID_NONE, 0, BOARD_TYPE_NONE, IDX_BDTYPE_OUT_NONE, "none", {0}, NULL, 0, gBoardNonePortAttr, NULL},	
	//out
	{BID_NTP, PORT_NTP, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_NTP, "NTP4",
			{BIT(SIGNAL_NTP)}, ntp_almtbl, LEN_ATTR_NTP_ALMTBL, NULL, NULL},
	{BID_NTPF, PORT_NTPF, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_NTPF, "NTPF4",
			BIT(SIGNAL_NTP), ntp_almtbl, LEN_ATTR_NTP_ALMTBL, NULL, NULL},
 	{BID_RS232, PORT_RS232, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_RS232, "RS232",
 			BIT(SIGNAL_TODT), NULL, 0, NULL, NULL},
	{BID_RS485, PORT_RS485, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_RS485, "RS485",
			BIT(SIGNAL_TODT) | BIT(SIGNAL_PPS)| BIT(SIGNAL_PPM)| BIT(SIGNAL_PPH) |
			BIT(SIGNAL_IRIGB_DC), NULL, 0, NULL, NULL
	},
	{BID_BDCT, 0, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_BDCT, "BDCT", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},	//此盘取消，预留
	{BID_BAC, PORT_BAC, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_BAC, "BAC", BIT(SIGNAL_IRIGB_AC), NULL, 0, NULL, NULL}, //板卡功能：直流b码转成交流b码
	{BID_SF4, PORT_SF4, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_SF4, "SF4", BIT(SIGNAL_IRIGB_DC), NULL, 0, NULL, NULL},
	{BID_BDC485, 0, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_BDC485, "BDC485", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},	//此盘取消，预留
	{BID_KJD8, PORT_KJD8, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_KJD8, "KJD8",
			BIT(SIGNAL_PPS)| BIT(SIGNAL_PPM)| BIT(SIGNAL_PPH), NULL, 0, NULL, NULL
	},
	{BID_TTL8, PORT_TTL8, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_TTL8, "TTL8",
			BIT(SIGNAL_TODT) | BIT(SIGNAL_PPS)| BIT(SIGNAL_PPM)| BIT(SIGNAL_PPH) |
			BIT(SIGNAL_IRIGB_DC), NULL, 0, NULL, NULL
	},
	{BID_PPX485, 0, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_PPX485, "PPX485", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},//此盘取消，预留
	{BID_PTP, PORT_PTP, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_PTP, "PTP", BIT(SIGNAL_PTP), NULL, 0, NULL, NULL},
	{BID_PTPF, PORT_PTPF, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_PTPF, "PTPF", BIT(SIGNAL_NONE) | BIT(SIGNAL_PTP) | BIT(SIGNAL_NTP), NULL, 0, NULL, NULL}, //此盘取消，预留
	{BID_OUTE, 0, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_OUTE, "OUTE", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL}, //此盘取消，预留
	{BID_OUTH, 0, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_OUTH, "OUTH", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL}, //此盘取消，预留
	//61850
	{BID_61850, 1, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_61850, "61850", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},
	// out
	{BID_KJD4, PORT_KJD4, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_KJD4, "KJD4",
				BIT(SIGNAL_PPS)| BIT(SIGNAL_PPM)| BIT(SIGNAL_PPH), NULL, 0, NULL, NULL
	},

	{BID_TP4, PORT_TP4, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_TP4, "TP4", BIT(SIGNAL_TODF), NULL, 0, NULL, NULL},
	{BID_SC4, PORT_SC4, BOARD_TYPE_OUTPUT, IDX_BDTYPE_OUT_SC4, "SC4",
				BIT(SIGNAL_2MH)| BIT(SIGNAL_2MB)| BIT(SIGNAL_10MH), NULL, 0, NULL, NULL
	},
	//in
	{BID_GPSBF, 0, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_GPSBF, "GPSBF",
			{0}, sat_comm_almtbl, LEN_ATTR_SAT_COMM_ALMTBL, NULL, &sat_comm_pdata
	},//此盘取消，预留 单gps
	{BID_GPSBE, 0, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_GPSBE, "GPSBE",
			{0}, sat_comm_almtbl, LEN_ATTR_SAT_COMM_ALMTBL, NULL, &sat_comm_pdata
	},//此盘取消，预留 单gps
	{BID_GBDBF, PORT_SAT, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_GBDBF, "GBDBF",
			{0}, sat_comm_almtbl, LEN_ATTR_SAT_COMM_ALMTBL, gBoardSatPortAttr, &sat_comm_pdata
	},
	{BID_GBDBE, PORT_SAT, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_GBDBE, "GBDBE",
			{0}, sat_comm_almtbl, LEN_ATTR_SAT_COMM_ALMTBL, gBoardSatPortAttr, &sat_comm_pdata
	},
	{BID_BDBF, 0, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_BDBF, "BDBF", {0}, NULL, 0, NULL, NULL
	},//此盘取消，预留 单bd
	{BID_BDBE, 0, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_BDBE, "BDBE", {0}, NULL, 0, NULL, NULL
	},//此盘取消，预留 单bd
	{BID_BEI, 0, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_BEI, "BEI",
			{0}, sat_bei_bfi_almtbl, LEN_ATTR_SAT_BEI_ALMTBL, gBoardSat_bei_bfi_PortAttr, &sat_bei_bfi_pdata
	},//此盘取消，预留
	{BID_BFI, 0, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_BFI, "BFI",
			{0}, sat_bei_bfi_almtbl, LEN_ATTR_SAT_BEI_ALMTBL, gBoardSat_bei_bfi_PortAttr, &sat_bei_bfi_pdata
	},//此盘取消，预留
	{BID_BFEI, PORT_SAT, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_BFEI, "BFEI",
		{0}, sat_bfei_bffi_almtbl, LEN_ATTR_SAT_BFEI_ALMTBL, gBoardSat_bffi_bfei_PortAttr, &sat_bfei_bffi_pdata
	},//1路2Mhz/2Mbit 和两路IRIGB, 实际为4个端口，卫星盘通用做法用5，1-sat 2-2mhz 3-2mb 4-irigb1 5-irigb2
	{BID_BFFI, PORT_SAT, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_BFFI, "BFFI",
		{0}, sat_bfei_bffi_almtbl, LEN_ATTR_SAT_BFEI_ALMTBL, gBoardSat_bffi_bfei_PortAttr, &sat_bfei_bffi_pdata
	},//1路2Mhz/2Mbit 和两路IRIGB,MACRO_SAT_BFEI_BFFI_INSRC_TYPE的长度必须和max_port一致
	{BID_PTP_IN, PORT_PTP_IN, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_PTP_IN, "PTP_IN",
			{0}, ptpin_almtbl, LEN_ATTR_PTPIN_ALMTBL, gBoardPtpinPortAttr, NULL
	},
	{BID_RTF, PORT_RTF, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_RTF, "RTF", {0},
			rtf_almtbl, LEN_ATTR_RTF_ALMTBL, gBoardRtfPortAttr, NULL //| BIT(SIGNAL_TODF)| BIT(SIGNAL_10MH)//暂时没添加
	},
	{BID_NTP_IN, PORT_NTP_IN, BOARD_TYPE_INPUT, IDX_BDTYPE_IN_NTP_IN, "NTP_IN",
			{0}, ntpin_almtbl, LEN_ATTR_NTPIN_ALMTBL, gBoardNtpinPortAttr, NULL
	},
	//power
	{BID_PWRM, 1, BOARD_TYPE_PWR, 0, "PWRM", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},
	{BID_PWRD, 1, BOARD_TYPE_PWR, 0, "PWRD", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},
	{BID_PWRL, 0, BOARD_TYPE_PWR, 0, "PWRL", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},//此盘取消，预留

	//alarm
	{BID_ALM, 1, BOARD_TYPE_ALM, 0, "ALM", BIT(SIGNAL_NONE), NULL, 0, NULL, NULL},

};

int gBoardAttrTbl_len = BOARD_ATTR_TBL_LEN;

/*board max port num*/
//input board max port
//please update the INPUT_SLOT_MAX_PORT 5
//int gBoardInMaxPort = 0;
//please update the OUTPUT_SLOT_MAX_PORT 8
//int gBoardOutMaxPort = 0;


/*
  单盘名称索引
*/
u8_t gBoardNameTbl[BID_ARRAY_SIZE][16];

void initializeBoardNameTable(void)
{
	int i;
	u8_t board_type;
	u8_t temp = 0;
	u8_t in_maxport_num = 0, out_maxport_num = 0;
	struct boardattr_tbl_t *board_attr_tbl = gBoardAttrTbl;

	for(i=0; i<BID_ARRAY_SIZE; i++)
	{
		memset(gBoardNameTbl[i], 0, 16);
	}

	#if 1
	char *name = NULL;
	for(i=0; i<BID_ARRAY_SIZE; i++)
	{
		FIND_BDNAME_GBATBL_BY_BID(i, name);
		memcpy(gBoardNameTbl[i], name, strlen(name));
	}
	#else
	//none
	memcpy(gBoardNameTbl[BID_NONE], "none", 4);
	
	//out
	memcpy(gBoardNameTbl[BID_NTP], "NTP", 3);
	memcpy(gBoardNameTbl[BID_NTPF], "NTPF", 4);
	memcpy(gBoardNameTbl[BID_RS232], "RS232", 5);
	memcpy(gBoardNameTbl[BID_RS485], "RS485", 5);
	memcpy(gBoardNameTbl[BID_BDCT], "BDCT", 4);
	memcpy(gBoardNameTbl[BID_BAC], "BAC", 3);
	memcpy(gBoardNameTbl[BID_SF4], "SF4", 3);
	memcpy(gBoardNameTbl[BID_BDC485], "BDC485", 6);
	memcpy(gBoardNameTbl[BID_KJD8], "KJD8", 4);
	memcpy(gBoardNameTbl[BID_TTL8], "TTL8", 4);
	memcpy(gBoardNameTbl[BID_PPX485], "PPX485", 6);
	memcpy(gBoardNameTbl[BID_PTP], "PTP", 3);
	memcpy(gBoardNameTbl[BID_PTPF], "PTPF", 4);
	memcpy(gBoardNameTbl[BID_OUTE], "OUTE", 4);
	memcpy(gBoardNameTbl[BID_OUTH], "OUTH", 4);
	//61850
	memcpy(gBoardNameTbl[BID_61850], "61850", 5);
	// out
	memcpy(gBoardNameTbl[BID_KJD4], "KJD4", 4);
	memcpy(gBoardNameTbl[BID_TP4], "TP4", 4);
	memcpy(gBoardNameTbl[BID_SC4], "SC4", 4);
	//in
	memcpy(gBoardNameTbl[BID_GPSBF], "GPSBF", 5);
	memcpy(gBoardNameTbl[BID_GPSBE], "GPSBE", 5);
	memcpy(gBoardNameTbl[BID_GBDBF], "GBDBF", 5);
	memcpy(gBoardNameTbl[BID_GBDBE], "GBDBE", 5);
	memcpy(gBoardNameTbl[BID_BDBF], "BDBF", 4);
	memcpy(gBoardNameTbl[BID_BDBE], "BDBE", 4);
	memcpy(gBoardNameTbl[BID_BEI], "BEI", 3);
	memcpy(gBoardNameTbl[BID_BFI], "BFI", 3);
	memcpy(gBoardNameTbl[BID_BFEI], "BFEI", 4);
	memcpy(gBoardNameTbl[BID_BFFI], "BFFI", 4);
	memcpy(gBoardNameTbl[BID_PTP_IN], "PTP_IN",6);
	memcpy(gBoardNameTbl[BID_RTF], "RTF",6);

	//power
	memcpy(gBoardNameTbl[BID_PWRM], "PWRM", 4);
	memcpy(gBoardNameTbl[BID_PWRD], "PWRD", 4);
	memcpy(gBoardNameTbl[BID_PWRL], "PWRL", 4);

	//alarm
	memcpy(gBoardNameTbl[BID_ALM], "ALM", 3);
	#endif
	//find max port

	
	for (i = 0; i < ARRAY_SIZE(gBoardAttrTbl); ++i){	
		board_type = board_attr_tbl[i].board_type;

		if (BOARD_TYPE_INPUT == board_type){
			temp = board_attr_tbl[i].max_port;
			if(in_maxport_num < temp)
				in_maxport_num = temp;
		}

		if (BOARD_TYPE_OUTPUT == board_type){
			temp = board_attr_tbl[i].max_port;
			if(out_maxport_num < temp)
				out_maxport_num = temp;
		}
	}
	//gBoardInMaxPort = in_maxport_num;
	//gBoardOutMaxPort = out_maxport_num;

	/*check define*/
	if (in_maxport_num != INPUT_SLOT_MAX_PORT){
		fprintf(stderr, "input slot max port(%d) != macro define max slot(%d)\n, please update the INPUT_SLOT_MAX_PORT\n",
			in_maxport_num, ALM_CURSOR_ARRAY_SIZE);
		return;
	}
	if (out_maxport_num != OUTPUT_SLOT_MAX_PORT){
		fprintf(stderr, "output slot max port(%d) != macro define max slot(%d)\n, please update the OUTPUT_SLOT_MAX_PORT\n",
			out_maxport_num, ALM_CURSOR_ARRAY_SIZE);
		return;
	}
}

#define ALARM_ID_TBL_LEN  ARRAY_SIZE(gAlmIdTbl)
struct alm_id_tbl_t gAlmIdTbl[] = {
	{ SNMP_ALM_ID_ALL_REF_LOS, SMP_SID_SUB_SYS, "ALL Reference LOS Alarm" },
	{ SNMP_ALM_ID_XO_UNLOCK, SMP_SID_SUB_SYS, "XO Unlock Alarm" },
	{ SNMP_ALM_ID_RB_UNLOCK, SMP_SID_SUB_SYS, "RB Unlock Alarm" },
	{ SNMP_ALM_ID_HW_FPGA_HALT, SMP_SID_SUB_SYS, "HW FPGA Reg Alarm" },
	{ SNMP_ALM_ID_NTP_EXIT, SMP_SID_SUB_SYS, "NTP Exit" },
	{ SNMP_ALM_ID_EXT_PWR_INPUT, SMP_SID_SUB_PWR, "External Power Input Alarm" },
	{ SNMP_ALM_ID_GPBD_SGN_LOS, SMP_SID_SUB_SAT, "GPS/BD Signal LOS" },
	{ SNMP_ALM_ID_GPBD_SGN_DGD, SMP_SID_SUB_SAT, "GPS/BD Signal Degrade" },
	{ SNMP_ALM_ID_ANT_OPEN, SMP_SID_SUB_SAT, "Antenna Open" },
	{ SNMP_ALM_ID_ANT_SHORT, SMP_SID_SUB_SAT, "Antenna Short" },
	{ SNMP_ALM_ID_GPBD_THRSD, SMP_SID_SUB_IN, "GPS/BD Threshold Alarm" },
	{ SNMP_ALM_ID_1PPS_TOD_THRSD, SMP_SID_SUB_IN, "1PPS TOD Threshold Alarm" },
	{ SNMP_ALM_ID_1PPS_TOD_IN_LOS, SMP_SID_SUB_IN, "1PPS TOD Input LOS" },
	{ SNMP_ALM_ID_1PPS_THRSD, SMP_SID_SUB_IN, "1PPS Threshold Alarm" },
	{ SNMP_ALM_ID_1PPS_IN_LOS, SMP_SID_SUB_IN, "1PPS Input LOS" },
	{ SNMP_ALM_ID_2MHZ_IN_LOS, SMP_SID_SUB_IN, "2048KHz Input LOS" },
	{ SNMP_ALM_ID_2MHZ_THRSD, SMP_SID_SUB_IN, "2048KHz Threshold Alarm" },
	{ SNMP_ALM_ID_10MHZ_IN_LOS, SMP_SID_SUB_IN, "10MHz Input LOS" },
	{ SNMP_ALM_ID_10MHZ_THRSD, SMP_SID_SUB_IN, "10MHz Threshold Alarm" },
	{ SNMP_ALM_ID_2MB_IN_LOS, SMP_SID_SUB_IN, "2048Kbit/s Input LOS" },
	{ SNMP_ALM_ID_2MB_AIS, SMP_SID_SUB_IN, "2048Kbit/s AIS" },
	{ SNMP_ALM_ID_2MB_BPV, SMP_SID_SUB_IN, "2048Kbit/s BPV" },
	{ SNMP_ALM_ID_2MB_CRC, SMP_SID_SUB_IN, "2048Kbit/s CRC" },
	{ SNMP_ALM_ID_2MB_LOF, SMP_SID_SUB_IN, "2048Kbit/s LOF" },
	{ SNMP_ALM_ID_2MB_THRSD, SMP_SID_SUB_IN, "2048Kbit/s Threshold Alarm" },
	{ SNMP_ALM_ID_IRIGB_IN_LOS, SMP_SID_SUB_IN, "IRIGB Input LOS" },
	{ SNMP_ALM_ID_IRIGB_IN_DGD, SMP_SID_SUB_IN, "IRIGB Degraded Alarm" },
	{ SNMP_ALM_ID_IRIGB_INVAID, SMP_SID_SUB_IN, "IRIGB Unavailable Alarm" },
	{ SNMP_ALM_ID_IRIGB_THRSD, SMP_SID_SUB_IN, "IRIGB Threshold Alarm" },
	{ SNMP_ALM_ID_PTP_IN_LOS, SMP_SID_SUB_IN, "PTP Input LOS" },
	{ SNMP_ALM_ID_PTP_IN_DGD, SMP_SID_SUB_IN, "PTP Input Degraded" },
	{ SNMP_ALM_ID_PTP_THRSD, SMP_SID_SUB_IN, "PTP Threshold Alarm" },
	{ SNMP_ALM_ID_NTP_IN_LOS, SMP_SID_SUB_IN, "NTP Input LOS" },
	{ SNMP_ALM_ID_NTP_IN_DGD, SMP_SID_SUB_IN, "NTP Input Degraded" },
	{ SNMP_ALM_ID_NTP_THRSD, SMP_SID_SUB_IN, "NTP Threshold Alarm" },

	{ SNMP_ALM_ID_LINK_DOWN, SMP_SID_SUB_OUT, "Link Down" },
};
int gAlmIdTbl_len = ALARM_ID_TBL_LEN;

struct alm_inx_tbl_t *pAlmInxTbl;
u32_t max_alminx_num;


//support max slot: SLOT_CURSOR_ARRAY_SIZE 16
int initializeAlarmIndexTable(void)
{
#define ALM_SUB_NUM		5

#define ALM_SUB_SYS 	0
#define ALM_SUB_PWR 	1
#define ALM_SUB_SAT 	2
#define ALM_SUB_IN		3
#define ALM_SUB_OUT 	4

	int i,j,k,m,inx = 0;
	u8_t board_type;
	u8_t alarm_type;
	u8_t temp = 0;
	u8_t maxslot_num[ALM_SUB_NUM];
	u8_t maxport_num[ALM_SUB_NUM];
	u8_t maxalmtype_num[ALM_SUB_NUM];

	u8_t sid_sub[ALM_SUB_NUM];
	
	struct boardattr_tbl_t *board_attr_tbl = gBoardAttrTbl;
	struct alm_id_tbl_t * alm_id_tbl = gAlmIdTbl;

	sid_sub[ALM_SUB_SYS] = SMP_SID_SUB_SYS;
	sid_sub[ALM_SUB_PWR] = SMP_SID_SUB_PWR;
	sid_sub[ALM_SUB_SAT] = SMP_SID_SUB_SAT;
	sid_sub[ALM_SUB_IN] = SMP_SID_SUB_IN;
	sid_sub[ALM_SUB_OUT] = SMP_SID_SUB_OUT; 


	memset(maxslot_num, 0x0, ALM_SUB_NUM);
	memset(maxport_num, 0x0, ALM_SUB_NUM);
	memset(maxalmtype_num, 0x0, ALM_SUB_NUM);

	//MAX SLOT NUM
	maxslot_num[ALM_SUB_SYS] = 1;
	maxslot_num[ALM_SUB_PWR] = PWR_SLOT_CURSOR_ARRAY_SIZE;
	maxslot_num[ALM_SUB_SAT] = INPUT_SLOT_CURSOR_ARRAY_SIZE;
	maxslot_num[ALM_SUB_IN] = INPUT_SLOT_CURSOR_ARRAY_SIZE;
	maxslot_num[ALM_SUB_OUT] = SLOT_CURSOR_ARRAY_SIZE;

	//FOUND MAX PORT NUM
	maxport_num[ALM_SUB_SYS] = 1;
	maxport_num[ALM_SUB_SAT] = 1;
	for (i = 0; i < ARRAY_SIZE(gBoardAttrTbl); ++i){	
		board_type = board_attr_tbl[i].board_type;

		if (BOARD_TYPE_PWR == board_type){
			temp = board_attr_tbl[i].max_port;
			if(maxport_num[ALM_SUB_PWR] < temp)
				maxport_num[ALM_SUB_PWR] = temp;
		}
		if (BOARD_TYPE_INPUT == board_type){
			temp = board_attr_tbl[i].max_port;
			if(maxport_num[ALM_SUB_IN] < temp)
				maxport_num[ALM_SUB_IN] = temp;
		}

		if (BOARD_TYPE_OUTPUT == board_type){
			temp = board_attr_tbl[i].max_port;
			if(maxport_num[ALM_SUB_OUT] < temp)
				maxport_num[ALM_SUB_OUT] = temp;
		}

	}

	//FOUND MAX ALARM TYPE NUM
	for (i = 0; i < ARRAY_SIZE(gAlmIdTbl); ++i){
		alarm_type = alm_id_tbl[i].alm_type;

		if (SMP_SID_SUB_SYS == alarm_type)
			maxalmtype_num[ALM_SUB_SYS]++;
		if (SMP_SID_SUB_PWR == alarm_type)
			maxalmtype_num[ALM_SUB_PWR]++;
		if (SMP_SID_SUB_SAT == alarm_type)
			maxalmtype_num[ALM_SUB_SAT]++;
		if (SMP_SID_SUB_IN == alarm_type)
			maxalmtype_num[ALM_SUB_IN]++;
		if (SMP_SID_SUB_OUT == alarm_type)
			maxalmtype_num[ALM_SUB_OUT]++;
	}

	for (i = 0; i < ALM_SUB_NUM; ++i){
		max_alminx_num += (u32_t) maxslot_num[i] * maxport_num[i] * maxalmtype_num[i];
		dbg_printf("[%d]: found maxslot:%d maxport:%d maxalmtype:%d\n",
		i, maxslot_num[i],
		maxport_num[i], maxalmtype_num[i]);
	}
	dbg_printf("max alm index num: %d\n", max_alminx_num);

	/*MALLOC  //8**/
	pAlmInxTbl = (struct alm_inx_tbl_t *) MALLOC(max_alminx_num* sizeof(struct alm_inx_tbl_t));
	if (!pAlmInxTbl){
		fprintf(stderr, "malloc failure!\n");
		return -1;
	}

	inx = 0;
	for (m = 0; m < ALM_SUB_NUM; ++m){
		for (i = 0; i < ARRAY_SIZE(gAlmIdTbl); ++i){
			alarm_type = alm_id_tbl[i].alm_type;
			//m: 0-sys, 1-pwr, 2-sat,3-in,4-out
			//alm_type
			if (sid_sub[m] == alarm_type){
				for (j = 0; j < maxslot_num[m]; ++j){
					//slot
					for (k = 0; k < maxport_num[m]; ++k){
						//port
						pAlmInxTbl[inx].alm_id_tbl = &alm_id_tbl[i];
						pAlmInxTbl[inx].alm_id = alm_id_tbl[i].alm_id;
						pAlmInxTbl[inx].slot = j+1;
						pAlmInxTbl[inx].port = k+1;
						inx++;
					}
				}
				
			}	
		}
	}

#ifdef NEW_ALARMID_COMM
	if (inx != max_alminx_num || max_alminx_num != ALM_CURSOR_ARRAY_SIZE){
		fprintf(stderr, "max_alminx_num != inx or max_alminx_num(%d) != %d\n, please update the ALM_CURSOR_ARRAY_SIZE\n",
			max_alminx_num, ALM_CURSOR_ARRAY_SIZE);
		return -1;
	}
#endif

#ifdef DEBUG
	char *s_alm_type = NULL;
	for (i = 0; i < max_alminx_num; ++i){
		switch (pAlmInxTbl[i].alm_id_tbl->alm_type){
			case SMP_SID_SUB_SYS:
				s_alm_type = get_marco_name(SYS);
				break;
			case SMP_SID_SUB_PWR:
				s_alm_type = get_marco_name(PWR);
				break;
			case SMP_SID_SUB_SAT:
				s_alm_type = get_marco_name(SAT);
				break;
			case SMP_SID_SUB_IN:
				s_alm_type = get_marco_name(IN);
				break;
			case SMP_SID_SUB_OUT:
				s_alm_type = get_marco_name(OUT);
				break;
			default:
				s_alm_type = "invaid";
		}
		dbg_printf("[%3d] Alm Slot:%2d Port:%d ID:0x%08x AlmType:%-4s* AlmInfo:%s\n",
			i,
			pAlmInxTbl[i].slot,
			pAlmInxTbl[i].port,
			pAlmInxTbl[i].alm_id,
			s_alm_type,
			pAlmInxTbl[i].alm_id_tbl->alm_msg
			);
	}
#endif
	//TEST
	//int index = -1;
	//int alm_slot=2;
	//int alm_port=3;
	//int alm_id= SNMP_ALM_ID_2MHZ_THRSD;
	//FIND_ALMINX_BY_SLOT_PORT_ID(alm_id, alm_slot, alm_port, index);
	//dbg_printf("alm_index:%d\n",index);
	return 0;

}


void cleanAlarmIndexTable(void)
{
	if(pAlmInxTbl)
		FREE(pAlmInxTbl);
}




/******event*****/
#define EVENT_ID_TBL_LEN  ARRAY_SIZE(gEvntIdTbl)
struct evnt_id_tbl_t gEvntIdTbl[] = {
	{ SNMP_EVNT_ID_SYSTIME_SET, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "System Time Set" },
	{ SNMP_EVNT_ID_SYSTIME_UPDATE, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "System Time Update" },
	{ SNMP_EVNT_ID_USR_CONF_LOSS, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "User Config Miss" },
	{ SNMP_EVNT_ID_SYSLEAP_SET, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "GPS Offset Value Set" },
	{ SNMP_EVNT_ID_SYSLEAP_UPDATE, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "GPS Offset Value Update" },
	{ SNMP_EVNT_ID_SYSIP_SET, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "IP Address Set" },
	{ SNMP_EVNT_ID_TRPSIP_SET, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "Trap Server IP Address Set" },
	{ SNMP_EVNT_ID_TRPSIP_UPDATE, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "Trap Server IP Address Update" },
	{ SNMP_EVNT_ID_ALM_GENERATE, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "Alarm generated Event" },
	{ SNMP_EVNT_ID_ALM_DISAPPEAR, SMP_SID_SUB_SYS, SMP_EVNT_GRP_NONE, "Alarm disappeared Event" },

	{ SNMP_EVNT_ID_SAT_GPS, SMP_SID_SUB_SAT, SMP_EVNT_GRP_SATE_MODSET, "GPS" },
	{ SNMP_EVNT_ID_SAT_BD, SMP_SID_SUB_SAT, SMP_EVNT_GRP_SATE_MODSET, "BD" },
	{ SNMP_EVNT_ID_SAT_MGPS, SMP_SID_SUB_SAT, SMP_EVNT_GRP_SATE_MODSET, "MIX GPS" },
	{ SNMP_EVNT_ID_SAT_MBD, SMP_SID_SUB_SAT, SMP_EVNT_GRP_SATE_MODSET, "MIX BD" },
	{ SNMP_EVNT_ID_SAT_LNUM, SMP_SID_SUB_SAT, SMP_EVNT_GRP_NONE, "SAT_NUM LESS" },
	{ SNMP_EVNT_ID_SAT_LOCK, SMP_SID_SUB_SAT, SMP_EVNT_GRP_SATE_CLKSTA, "SAT_LOCKED" },
	{ SNMP_EVNT_ID_SAT_FREE, SMP_SID_SUB_SAT, SMP_EVNT_GRP_SATE_CLKSTA, "SAT_FREE" },

	{ SNMP_EVNT_ID_CLK_BRIDGE, SMP_SID_SUB_CLK, SMP_EVNT_GRP_CLKSTA, "Bridging" },
	{ SNMP_EVNT_ID_CLK_FAST, SMP_SID_SUB_CLK, SMP_EVNT_GRP_CLKSTA, "Fast" },
	{ SNMP_EVNT_ID_CLK_FREE, SMP_SID_SUB_CLK, SMP_EVNT_GRP_CLKSTA, "Free" },
	{ SNMP_EVNT_ID_CLK_LOCK, SMP_SID_SUB_CLK, SMP_EVNT_GRP_CLKSTA, "Locked" },
	{ SNMP_EVNT_ID_CLK_WARMUP, SMP_SID_SUB_CLK, SMP_EVNT_GRP_CLKSTA, "Warm Up" },
	{ SNMP_EVNT_ID_CLK_HOLD, SMP_SID_SUB_CLK, SMP_EVNT_GRP_CLKSTA, "Hold" },

	{ SNMP_EVNT_ID_IN_CH_AVA, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CHSTA, "IN CH_AVA" },
	{ SNMP_EVNT_ID_IN_CH_MAST, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CHSTA, "IN CH_MAST" },
	{ SNMP_EVNT_ID_IN_CH_STBY, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CHSTA, "IN CH_STBY" },
	{ SNMP_EVNT_ID_IN_CH_UNAVA, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CHSTA, "IN CH_UNAVA" },
	{ SNMP_EVNT_ID_IN_MT_AIS, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_MTSTA, "IN MT_AIS" },
	{ SNMP_EVNT_ID_IN_MT_BPV, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_MTSTA, "IN MT_BPV" },
	{ SNMP_EVNT_ID_IN_MT_CRC, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_MTSTA, "IN MT_CRC" },
	{ SNMP_EVNT_ID_IN_MT_FREQ, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_MTSTA, "IN MT_FREQ" },
	{ SNMP_EVNT_ID_IN_MT_LOS, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_MTSTA, "IN MT_LOS" },

	{ SNMP_EVNT_ID_IN_REF_ACTIVE, SMP_SID_SUB_IN, SMP_EVNT_GRP_NONE, "REF ACTIVE" },

	{ SNMP_EVNT_ID_IN_RPC, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CLK_QUAL_LVL, "IN PRC" },
	{ SNMP_EVNT_ID_IN_UNK, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CLK_QUAL_LVL, "IN UNK" },
	{ SNMP_EVNT_ID_IN_SSUT, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CLK_QUAL_LVL, "IN SSUT" },
	{ SNMP_EVNT_ID_IN_SSUL, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CLK_QUAL_LVL, "IN SSUL" },
	{ SNMP_EVNT_ID_IN_SEC, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CLK_QUAL_LVL, "IN SEC" },
	{ SNMP_EVNT_ID_IN_DNU, SMP_SID_SUB_IN, SMP_EVNT_GRP_IN_CLK_QUAL_LVL, "IN DNU" },

	{ SNMP_EVNT_ID_IN_REF_SWAP, SMP_SID_SUB_IN, SMP_EVNT_GRP_NONE, "REF SWAP" },

	{ SNMP_EVNT_ID_OUT_RPC, SMP_SID_SUB_OUT, SMP_EVNT_GRP_OUT_CLK_QUAL_LVL, "OUT PRC" },
	{ SNMP_EVNT_ID_OUT_UNK, SMP_SID_SUB_OUT, SMP_EVNT_GRP_OUT_CLK_QUAL_LVL,  "OUT UNK" },
	{ SNMP_EVNT_ID_OUT_SSUT, SMP_SID_SUB_OUT, SMP_EVNT_GRP_OUT_CLK_QUAL_LVL,  "OUT SSUT" },
	{ SNMP_EVNT_ID_OUT_SSUL, SMP_SID_SUB_OUT, SMP_EVNT_GRP_OUT_CLK_QUAL_LVL,  "OUTSSUL" },
	{ SNMP_EVNT_ID_OUT_SEC, SMP_SID_SUB_OUT, SMP_EVNT_GRP_OUT_CLK_QUAL_LVL,  "OUTSEC" },
	{ SNMP_EVNT_ID_OUT_DNU, SMP_SID_SUB_OUT, SMP_EVNT_GRP_OUT_CLK_QUAL_LVL,  "OUT DNU" },

	{ SNMP_EVNT_ID_LINK_DOWN, SMP_SID_SUB_OUT, SMP_EVNT_GRP_NONE, "Link Down" },
};
int gEvntIdTbl_len = EVENT_ID_TBL_LEN;

struct evnt_inx_tbl_t *pEvntInxTbl;
u32_t max_evntinx_num;

//#define CLK_SLOT_CURSOR_ARRAY_SIZE 1
//support max slot: SLOT_CURSOR_ARRAY_SIZE 16
int initializeEventIndexTable(void)
{
#define EVNT_SUB_NUM		5

#define EVNT_SUB_SYS 	0
#define EVNT_SUB_CLK 	1
#define EVNT_SUB_SAT 	2
#define EVNT_SUB_IN		3
#define EVNT_SUB_OUT 	4

	int i,j,k,m,inx = 0;
	u8_t board_type;
	u8_t event_type;
	u8_t temp = 0;
	u8_t maxslot_num[EVNT_SUB_NUM];
	u8_t maxport_num[EVNT_SUB_NUM];
	u8_t maxevntype_num[EVNT_SUB_NUM];

	u8_t sid_sub[EVNT_SUB_NUM];
	
	struct boardattr_tbl_t *board_attr_tbl = gBoardAttrTbl;
	struct evnt_id_tbl_t * evnt_id_tbl = gEvntIdTbl;

	sid_sub[EVNT_SUB_SYS] = SMP_SID_SUB_SYS;
	sid_sub[EVNT_SUB_CLK] = SMP_SID_SUB_CLK;
	sid_sub[EVNT_SUB_SAT] = SMP_SID_SUB_SAT;
	sid_sub[EVNT_SUB_IN] = SMP_SID_SUB_IN;
	sid_sub[EVNT_SUB_OUT] = SMP_SID_SUB_OUT; 


	memset(maxslot_num, 0x0, EVNT_SUB_NUM);
	memset(maxport_num, 0x0, EVNT_SUB_NUM);
	memset(maxevntype_num, 0x0, EVNT_SUB_NUM);

	//MAX SLOT NUM
	maxslot_num[EVNT_SUB_SYS] = SID_SYS_SLOT;
	maxslot_num[EVNT_SUB_CLK] = SID_CLK_SLOT; //CLK_SLOT_CURSOR_ARRAY_SIZE;
	maxslot_num[EVNT_SUB_SAT] = INPUT_SLOT_CURSOR_ARRAY_SIZE;
	maxslot_num[EVNT_SUB_IN] = INPUT_SLOT_CURSOR_ARRAY_SIZE;
	maxslot_num[EVNT_SUB_OUT] = SLOT_CURSOR_ARRAY_SIZE;

	//FOUND MAX PORT NUM
	maxport_num[EVNT_SUB_SYS] = SID_SYS_PORT;
	maxport_num[EVNT_SUB_SAT] = 1;
	maxport_num[EVNT_SUB_CLK] = SID_CLK_PORT;
	for (i = 0; i < ARRAY_SIZE(gBoardAttrTbl); ++i){	
		board_type = board_attr_tbl[i].board_type;

		if (BOARD_TYPE_INPUT == board_type){
			temp = board_attr_tbl[i].max_port;
			if(maxport_num[EVNT_SUB_IN] < temp)
				maxport_num[EVNT_SUB_IN] = temp;
		}

		if (BOARD_TYPE_OUTPUT == board_type){
			temp = board_attr_tbl[i].max_port;
			if(maxport_num[EVNT_SUB_OUT] < temp)
				maxport_num[EVNT_SUB_OUT] = temp;
		}

	}

	//FOUND MAX ALARM TYPE NUM
	for (i = 0; i < ARRAY_SIZE(gEvntIdTbl); ++i){
		event_type = evnt_id_tbl[i].evnt_type;

		if (SMP_SID_SUB_SYS == event_type)
			maxevntype_num[EVNT_SUB_SYS]++;
		if (SMP_SID_SUB_CLK == event_type)
			maxevntype_num[EVNT_SUB_CLK]++;
		if (SMP_SID_SUB_SAT == event_type)
			maxevntype_num[EVNT_SUB_SAT]++;
		if (SMP_SID_SUB_IN == event_type)
			maxevntype_num[EVNT_SUB_IN]++;
		if (SMP_SID_SUB_OUT == event_type)
			maxevntype_num[EVNT_SUB_OUT]++;
	}

	for (i = 0; i < EVNT_SUB_NUM; ++i){
		max_evntinx_num += (u32_t) maxslot_num[i] * maxport_num[i] * maxevntype_num[i];
		dbg_printf("[%d]: found maxslot:%d maxport:%d maxevnttype:%d\n",
		i, maxslot_num[i],
		maxport_num[i], maxevntype_num[i]);
	}
	dbg_printf("max evnt index num: %d\n", max_evntinx_num);

	/*MALLOC  //8**/
	pEvntInxTbl = (struct evnt_inx_tbl_t *) MALLOC(max_evntinx_num* sizeof(struct evnt_inx_tbl_t));
	if (!pEvntInxTbl){
		fprintf(stderr, "malloc failure!\n");
		return -1;
	}

	inx = 0;
	for (m = 0; m < EVNT_SUB_NUM; ++m){
		for (i = 0; i < ARRAY_SIZE(gEvntIdTbl); ++i){
			event_type = evnt_id_tbl[i].evnt_type;
			//m: 0-sys, 1-pwr, 2-sat,3-in,4-out
			//evnt_type
			if (sid_sub[m] == event_type){
				for (j = 0; j < maxslot_num[m]; ++j){
					//slot
					for (k = 0; k < maxport_num[m]; ++k){
						//port
						//default
						pEvntInxTbl[inx].index = inx;
						pEvntInxTbl[inx].old_flag = 0;
						pEvntInxTbl[inx].new_flag = 0;
						pEvntInxTbl[inx].valid = 0;
						pEvntInxTbl[inx].evnt_id_tbl = &evnt_id_tbl[i];
						pEvntInxTbl[inx].evnt_id = evnt_id_tbl[i].evnt_id;
						pEvntInxTbl[inx].evnt_group = evnt_id_tbl[i].evnt_group;
						memset(pEvntInxTbl[inx].start_time, 0x0, sizeof(pEvntInxTbl[inx].start_time));
						pEvntInxTbl[inx].slot = j+1;
						pEvntInxTbl[inx].port = k+1;
						inx++;
					}
				}
				
			}	
		}
	}

#ifdef NEW_ALARMID_COMM
	//#define EVNT_CURSOR_ARRAY_SIZE 1278
	if (inx != max_evntinx_num){
		fprintf(stderr, "max_evntinx_num(%d) != inx(%d)\n",
			max_evntinx_num, inx);
		return -1;
	}
#endif

#ifdef DEBUG
	char *s_evnt_type = NULL;
	for (i = 0; i < max_evntinx_num; ++i){
		switch (pEvntInxTbl[i].evnt_id_tbl->evnt_type){
			case SMP_SID_SUB_SYS:
				s_evnt_type = get_marco_name(SYS);
				break;
			case SMP_SID_SUB_CLK:
				s_evnt_type = get_marco_name(CLK);
				break;
			case SMP_SID_SUB_SAT:
				s_evnt_type = get_marco_name(SAT);
				break;
			case SMP_SID_SUB_IN:
				s_evnt_type = get_marco_name(IN);
				break;
			case SMP_SID_SUB_OUT:
				s_evnt_type = get_marco_name(OUT);
				break;
			default:
				s_evnt_type = "invaid";
		}
		dbg_printf("[%3d] Evnt Slot:%2d Port:%d ID:0x%08x EvntType:%-4s* EvntInfo:%s\n",
			i,
			pEvntInxTbl[i].slot,
			pEvntInxTbl[i].port,
			pEvntInxTbl[i].evnt_id,
			s_evnt_type,
			pEvntInxTbl[i].evnt_id_tbl->evnt_msg
			);
	}
#endif
	//TEST
	//int index = -1;
	//int evnt_slot=2;
	//int evnt_port=3;
	//int evnt_id= SNMP_EVNT_ID_2MHZ_THRSD;
	//FIND_EVNTINX_BY_SLOT_PORT_ID(evnt_id, evnt_slot, evnt_port, index);
	//dbg_printf("evnt_index:%d\n",index);
	return 0;

}


void cleanEventIndexTable(void)
{
	if(pEvntInxTbl)
		FREE(pEvntInxTbl);
}


