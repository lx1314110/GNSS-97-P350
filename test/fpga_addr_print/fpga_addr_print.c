#include "addr.h"
#ifdef NEW_FPGA_ADDR_COMM

#include <stdio.h>
#include "fpga_addr_print.h"
#define get_marco_name(x)   #x
#define Conn(x,y) x##y

//#define IN(marco,slot)   marco(1)
//#define slot  1
#define IN(marco, slot)   \
{marco(1), get_marco_name(marco(1))}, \
{marco(2), get_marco_name(marco(2))}, \
{marco(3), get_marco_name(marco(3))}, \
{marco(4), get_marco_name(marco(4))} \

#define OUT(marco, slot)   \
{marco(1), get_marco_name(marco(1))}, \
{marco(2), get_marco_name(marco(2))}, \
{marco(3), get_marco_name(marco(3))}, \
{marco(4), get_marco_name(marco(4))}, \
{marco(5), get_marco_name(marco(5))}, \
{marco(6), get_marco_name(marco(6))}, \
{marco(7), get_marco_name(marco(7))}, \
{marco(8), get_marco_name(marco(8))}, \
{marco(9), get_marco_name(marco(9))}, \
{marco(10), get_marco_name(marco(10))}, \
{marco(11), get_marco_name(marco(11))}, \
{marco(12), get_marco_name(marco(12))}, \
{marco(13), get_marco_name(marco(13))}, \
{marco(14), get_marco_name(marco(14))}, \
{marco(15), get_marco_name(marco(15))}, \
{marco(16), get_marco_name(marco(16))} \

#define BID_INDEX_8(marco, index)   \
{marco(0), get_marco_name(marco(0))}, \
{marco(1), get_marco_name(marco(1))}, \
{marco(2), get_marco_name(marco(2))}, \
{marco(3), get_marco_name(marco(3))}, \
{marco(4), get_marco_name(marco(4))}, \
{marco(5), get_marco_name(marco(5))}, \
{marco(6), get_marco_name(marco(6))}, \
{marco(7), get_marco_name(marco(7))}



/*
#ALL:  NUM
cat alloc/include/addr.h  | grep "^#define" | grep -v "^#define FPGA_BID_REGBASE" | grep -v "_OFFSETMASK" |grep -v '\\'  |wc -l

#SYS:
cat alloc/include/addr.h | grep "^#define" | grep -v "FPGA_BID_REGBASE" | grep -v "_OFFSETMASK" |grep -v '\\' | grep -v "(slot)"| awk '{print "{ "$2", ""get_marco_name""("$2")"" }," }'

#S01_S16(index):BID_INDEX_8
cat alloc/include/addr.h | grep "^#define" | grep "(index)"|  awk '{print $2 }' | awk -F\( '{print "BID_INDEX_8("$1", "$2","}'

#SLOT IN:
cat alloc/include/addr.h | grep "^#define" | grep "(slot)"| grep -v "FPGA_OUT" | grep -v "_OFFSETMASK" |  awk '{print $2 }' | awk -F\( '{print "IN("$1", "$2","}'

#SLOT OUT:
cat alloc/include/addr.h | grep "^#define" | grep "(slot)"| grep -v "FPGA_IN" |grep -v "_OFFSETMASK" | awk '{print $2 }' | awk -F\( '{print "OUT("$1", "$2","}'


*/
struct addr_info_t fpga_addr_i[] = 
{
	{ FPGA_DEVICE_TYPE, get_marco_name(FPGA_DEVICE_TYPE) },//sys2
	{ FPGA_VER, get_marco_name(FPGA_VER) },
	{ FPGA_LED_BD, get_marco_name(FPGA_LED_BD) },
	{ FPGA_LED_GPS, get_marco_name(FPGA_LED_GPS) },
	{ FPGA_LED_IRIGB, get_marco_name(FPGA_LED_IRIGB) },
	{ FPGA_LED_FREQ, get_marco_name(FPGA_LED_FREQ) },
	{ FPGA_LED_ALM, get_marco_name(FPGA_LED_ALM) },
	{ FPGA_LED_RUN, get_marco_name(FPGA_LED_RUN) },
	{ FPGA_SYS_OUT_TOD_DELAY_LOW, get_marco_name(FPGA_SYS_OUT_TOD_DELAY_LOW) },
	{ FPGA_SYS_OUT_TOD_DELAY_HIGH, get_marco_name(FPGA_SYS_OUT_TOD_DELAY_HIGH) },
	{ FPGA_SYS_OUT_IRIGB_DELAY_LOW, get_marco_name(FPGA_SYS_OUT_IRIGB_DELAY_LOW) },
	{ FPGA_SYS_OUT_IRIGB_DELAY_HIGH, get_marco_name(FPGA_SYS_OUT_IRIGB_DELAY_HIGH) },
	{ FPGA_SYS_OUT_PPX_DELAY_LOW, get_marco_name(FPGA_SYS_OUT_PPX_DELAY_LOW) },
	{ FPGA_SYS_OUT_PPX_DELAY_HIGH, get_marco_name(FPGA_SYS_OUT_PPX_DELAY_HIGH) },
	{ FPGA_SYS_OUT_PTP_DELAY_LOW, get_marco_name(FPGA_SYS_OUT_PTP_DELAY_LOW) },
	{ FPGA_SYS_OUT_PTP_DELAY_HIGH, get_marco_name(FPGA_SYS_OUT_PTP_DELAY_HIGH) },
	BID_INDEX_8(FPGA_BID_S01_TO_S16, index),
	{ FPGA_BID_S17, get_marco_name(FPGA_BID_S17) },
	{ FPGA_BID_S18_S19, get_marco_name(FPGA_BID_S18_S19) },
	{ FPGA_PWR_ALM, get_marco_name(FPGA_PWR_ALM) },
	{ FPGA_1ST_PWR_FREQ, get_marco_name(FPGA_1ST_PWR_FREQ) },
	{ FPGA_2ND_PWR_FREQ, get_marco_name(FPGA_2ND_PWR_FREQ) },
	{ FPGA_INT_PWR_VOL, get_marco_name(FPGA_INT_PWR_VOL) },
	{ FPGA_PWR_FREQ_INT, get_marco_name(FPGA_PWR_FREQ_INT) },
	{ FPGA_PWR_FREQ_FRA, get_marco_name(FPGA_PWR_FREQ_FRA) },
	{ FPGA_SYS_IP1725_ADDR, get_marco_name(FPGA_SYS_IP1725_ADDR) },
	{ FPGA_SYS_IP1725_VAL, get_marco_name(FPGA_SYS_IP1725_VAL) },
	{ FPGA_SYS_IP1725_REG_OP, get_marco_name(FPGA_SYS_IP1725_REG_OP) },
	{ FPGA_SYS_IP1725_READ, get_marco_name(FPGA_SYS_IP1725_READ) },
	{ FPGA_KEY_TABLE, get_marco_name(FPGA_KEY_TABLE) },
	{ FPGA_SYS_OUT_ALM_TAG, get_marco_name(FPGA_SYS_OUT_ALM_TAG) },
	{ FPGA_SYS_INSRC_SELECT, get_marco_name(FPGA_SYS_INSRC_SELECT) },
	{ FPGA_SYS_INSRC_STRATUM, get_marco_name(FPGA_SYS_INSRC_STRATUM) },
	{ FPGA_SYS_PH_LOW16, get_marco_name(FPGA_SYS_PH_LOW16) },
	{ FPGA_SYS_PH_HIGH12, get_marco_name(FPGA_SYS_PH_HIGH12) },
	{ FPGA_SYS_LOCK_TIME, get_marco_name(FPGA_SYS_LOCK_TIME) },
	{ FPGA_SYS_FRESH_TIME, get_marco_name(FPGA_SYS_FRESH_TIME) },
	{ FPGA_SYS_HIGH_SECOND, get_marco_name(FPGA_SYS_HIGH_SECOND) },
	{ FPGA_SYS_MED_SECOND, get_marco_name(FPGA_SYS_MED_SECOND) },
	{ FPGA_SYS_LOW_SECOND, get_marco_name(FPGA_SYS_LOW_SECOND) },
	{ FPGA_SYS_HIGH_MICO_SEC, get_marco_name(FPGA_SYS_HIGH_MICO_SEC) },
	{ FPGA_SYS_LOW_MICO_SEC, get_marco_name(FPGA_SYS_LOW_MICO_SEC) },
	{ FPGA_SYS_LPS, get_marco_name(FPGA_SYS_LPS) },
	{ FPGA_SYS_LEAP_FORECAST, get_marco_name(FPGA_SYS_LEAP_FORECAST) },
	{ FPGA_SYS_INSRC_VALID, get_marco_name(FPGA_SYS_INSRC_VALID) },
	{ FPGA_RBXO_TYP, get_marco_name(FPGA_RBXO_TYP) },
	{ FPGA_RBXO_STA, get_marco_name(FPGA_RBXO_STA) },
	{ FPGA_LCD_WE, get_marco_name(FPGA_LCD_WE) },
	{ FPGA_LCD_ADDR, get_marco_name(FPGA_LCD_ADDR) },
	{ FPGA_LCD_DATA, get_marco_name(FPGA_LCD_DATA) },
	{ FPGA_LCD_WREADY, get_marco_name(FPGA_LCD_WREADY) },
	{ FPGA_SYS_OUT_IRIGB_ZONE, get_marco_name(FPGA_SYS_OUT_IRIGB_ZONE) },
	{ FPGA_SYS_OUT_IRIGB_AR, get_marco_name(FPGA_SYS_OUT_IRIGB_AR) },
	{ FPGA_SYS_OUT_IRIGB_V, get_marco_name(FPGA_SYS_OUT_IRIGB_V) },
	{ FPGA_SYS_OUT_IRIGB_LEAP_FORECAST, get_marco_name(FPGA_SYS_OUT_IRIGB_LEAP_FORECAST) },
	{ FPGA_SYS_OUT_IRIGB_DST, get_marco_name(FPGA_SYS_OUT_IRIGB_DST) },
	{ FPGA_SYS_OUT_TOD_TAG, get_marco_name(FPGA_SYS_OUT_TOD_TAG) },
	{ FPGA_SYS_OUT_TOD_BR, get_marco_name(FPGA_SYS_OUT_TOD_BR) },
	{ FPGA_SYS_OUT_TOD_PPS_STATE, get_marco_name(FPGA_SYS_OUT_TOD_PPS_STATE) },
	{ FPGA_SYS_OUT_TOD_PPS_JITTER, get_marco_name(FPGA_SYS_OUT_TOD_PPS_JITTER) },
	{ FPGA_SYS_OUT_TOD_CLK_SRC_TYPE, get_marco_name(FPGA_SYS_OUT_TOD_CLK_SRC_TYPE) },
	{ FPGA_SYS_OUT_TOD_CLK_SRC_STATE, get_marco_name(FPGA_SYS_OUT_TOD_CLK_SRC_STATE) },
	{ FPGA_SYS_OUT_TOD_CLK_SRC_ALARM, get_marco_name(FPGA_SYS_OUT_TOD_CLK_SRC_ALARM) },
	{ FPGA_SYS_OUT_PTP_WEN, get_marco_name(FPGA_SYS_OUT_PTP_WEN) },
	{ FPGA_SYS_OUT_PTP_WADDR, get_marco_name(FPGA_SYS_OUT_PTP_WADDR) },
	{ FPGA_SYS_OUT_PTP_WVALUE, get_marco_name(FPGA_SYS_OUT_PTP_WVALUE) },
	{ FPGA_SYS_OUT_PTP_WREADY, get_marco_name(FPGA_SYS_OUT_PTP_WREADY) },
	{ FPGA_SYS_WDG_ENABLE, get_marco_name(FPGA_SYS_WDG_ENABLE) },
	{ FPGA_SYS_WDG_WDI, get_marco_name(FPGA_SYS_WDG_WDI) },
	{ FPGA_CHECK_RUN_STATUS, get_marco_name(FPGA_CHECK_RUN_STATUS) },
	{ FPGA_SYS_2MB_OUT_SA, get_marco_name(FPGA_SYS_2MB_OUT_SA) },
	{ FPGA_SYS_2MB_OUT_SSM, get_marco_name(FPGA_SYS_2MB_OUT_SSM) },
	{ FPGA_SYS_CUR_INSRC_SSM, get_marco_name(FPGA_SYS_CUR_INSRC_SSM) },
	IN(FPGA_IN_SIGNAL, slot),
	IN(FPGA_IN_TOD_DELAY_LOW16, slot),
	IN(FPGA_IN_TOD_DELAY_HIGH12, slot),
	IN(FPGA_IN_TOD_PH_LOW16, slot),
	IN(FPGA_IN_TOD_PH_HIGH12, slot),
	IN(FPGA_IN_GB_PH_LOW16, slot),
	IN(FPGA_IN_GB_PH_HIGH12, slot),
	IN(FPGA_IN_2MH_PH_LOW16, slot),
	IN(FPGA_IN_2MH_PH_HIGH12, slot),
	IN(FPGA_IN_2MB_PH_LOW16, slot),
	IN(FPGA_IN_2MB_PH_HIGH12, slot),
	IN(FPGA_IN_IRIGB1_PH_LOW16, slot),
	IN(FPGA_IN_IRIGB1_PH_HIGH12, slot),
	IN(FPGA_IN_IRIGB2_PH_LOW16, slot),
	IN(FPGA_IN_IRIGB2_PH_HIGH12, slot),
	IN(FPGA_IN_GB_DELAY_LOW16, slot),
	IN(FPGA_IN_GB_DELAY_HIGH12, slot),
	IN(FPGA_IN_2MH_DELAY_LOW16, slot),
	IN(FPGA_IN_2MH_DELAY_HIGH12, slot),
	IN(FPGA_IN_2MB_DELAY_LOW16, slot),
	IN(FPGA_IN_2MB_DELAY_HIGH12, slot),
	IN(FPGA_IN_IRIGB1_DELAY_LOW16, slot),
	IN(FPGA_IN_IRIGB1_DELAY_HIGH12, slot),
	IN(FPGA_IN_IRIGB1_LPS, slot),
	IN(FPGA_IN_IRIGB1_DSP, slot),
	IN(FPGA_IN_IRIGB1_QUALITY, slot),
	IN(FPGA_IN_IRIGB2_DELAY_LOW16, slot),
	IN(FPGA_IN_IRIGB2_DELAY_HIGH12, slot),
	IN(FPGA_IN_IRIGB2_LPS, slot),
	IN(FPGA_IN_IRIGB2_DSP, slot),
	IN(FPGA_IN_IRIGB2_QUALITY, slot),
	IN(FPGA_IN_GB_REN, slot),
	IN(FPGA_IN_GB_RADDR, slot),
	IN(FPGA_IN_GB_RVALUE, slot),
	IN(FPGA_IN_GB_WEN, slot),
	IN(FPGA_IN_GB_WADDR, slot),
	IN(FPGA_IN_GB_WVALUE, slot),
	IN(FPGA_IN_GB_WREADY, slot),
	IN(FPGA_IN_2MB_ALM, slot),
	IN(FPGA_IN_2MB_SA, slot),
	IN(FPGA_IN_2MB_SSM, slot),
	IN(FPGA_IN_TOD_LEAP, slot),
	IN(FPGA_IN_TOD_PPS_STATE, slot),
	IN(FPGA_IN_TOD_PPS_JITTER, slot),
	IN(FPGA_IN_TOD_CLK_SRC_TYPE, slot),
	IN(FPGA_IN_TOD_CLK_SRC_STATE, slot),
	IN(FPGA_IN_TOD_CLK_SRC_ALARM, slot),
	IN(FPGA_IN_PTP_REN, slot),
	IN(FPGA_IN_PTP_RADDR, slot),
	IN(FPGA_IN_PTP_RVALUE, slot),
	IN(FPGA_IN_PTP_WEN, slot),
	IN(FPGA_IN_PTP_WADDR, slot),
	IN(FPGA_IN_PTP_WVALUE, slot),
	IN(FPGA_IN_PTP_WREADY, slot),
	IN(FPGA_IN_1PPS_DELAY_LOW16, slot),
	IN(FPGA_IN_1PPS_DELAY_HIGH12, slot),
	IN(FPGA_IN_10MH1_DELAY_LOW16, slot),
	IN(FPGA_IN_10MH1_DELAY_HIGH12, slot),
	IN(FPGA_IN_10MH2_DELAY_LOW16, slot),
	IN(FPGA_IN_10MH2_DELAY_HIGH12, slot),
	IN(FPGA_IN_1PPS_PH_LOW16, slot),
	IN(FPGA_IN_1PPS_PH_HIGH12, slot),
	IN(FPGA_IN_10MH1_PH_LOW16, slot),
	IN(FPGA_IN_10MH1_PH_HIGH12, slot),
	IN(FPGA_IN_10MH2_PH_LOW16, slot),
	IN(FPGA_IN_10MH2_PH_HIGH12, slot),	
	IN(FPGA_IN_NTP_IN_DELAY_LOW16, slot),
	IN(FPGA_IN_NTP_IN_DELAY_HIGH12, slot),
	IN(FPGA_IN_NTP_IN_TIME_VAILD, slot),
	IN(FPGA_IN_NTP_IN_PH_LOW16, slot),
	IN(FPGA_IN_NTP_IN_PH_HIGH12, slot),
	OUT(FPGA_OUT_SIGNAL, slot),
};

int main(int argv, char **argc)
{
	int i;
	//int test = IN(FPGA_IN_SIGNAL(slot));
	for (i = 0; i < sizeof(fpga_addr_i)/sizeof(fpga_addr_i[0]); ++i){
		printf("%s\t\t0x%04x\n", fpga_addr_i[i].addr_name, fpga_addr_i[i].addr);
	}
	return 0;
}
#else
int main(int argv, char **argc){}
#endif

