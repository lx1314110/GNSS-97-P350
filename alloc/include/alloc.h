#ifndef __ALLOC__
#define __ALLOC__









#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "sqlite3.h"

#include "lib_type.h"
#include "lib_msgQ.h"
#include "lib_time.h"
#include "lib_bit.h"

#include "addr.h"

#define NEW_ALARMID_COMM	//must defined
#define NEW_3_4_IN_SLOT	//依赖于NEW_FPGA_ADDR_COMM

//中国移动tod
//#define CHINA_MOBILE_TOD_RULE
//中国联通tod
//#define CHINA_UNICOM_TOD_RULE
//工信部tod as same sa china_mobile
#define CHINA_MIIT_TOD_RULE

//------------------------------------------------------------------------------------------------------------
//第一次上电标志位
//------------------------------------------------------------------------------------------------------------
#define  FIRST   1
#define  OTHER   2

//------------------------------------------------------------------------------------------------------------
//卫星是否可用标志位
//------------------------------------------------------------------------------------
#define GET_SATELLITE   1
#define LOSS_SATELLITE  2
#define INIT_SATELLITE  3

//有输入源有效
#define REFSOURCE_VALID 1
#define REFSOURCE_INVALID 2
#define REFSOURCE_INIT 0	//FOR alarm mask

#define THRESHOLD_PHVALUE_MAX	10000000
#define THRESHOLD_PHVALUE_INIT 8388607


//user perimission management
#define MAXIMUM_PERMISSION 1
#define MEDIUM_PERMISSION 2
#define MINIMUM_PERMISSION 3

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
/*
  help document
*/
#define		HELP			"--help"


#define  		MY_SWITCH		0












//------------------------------------------------------------------------------------------------------------
//虚拟网卡
//------------------------------------------------------------------------------------------------------------
//for cmd help
#define	V_ETH_NTP	20
#define	V_ETH_MAX	21


struct veth {
	u8_t v_port;
	u8_t v_ip[16];
	u8_t v_mask[16];
	u8_t v_gateway[16];
	u8_t v_mac[18];
};











//------------------------------------------------------------------------------------------------------------
//eth0
//------------------------------------------------------------------------------------------------------------
#define	NETWORK_CARD	"eth0"
#define	NETWEB_CARD		"eth0.2"

struct netinfo {
	char ip[16];
	char mask[16];
	char gateway[16];
	char mac[18];
};



struct portinfo{
	u8_t port;
	u8_t enable;
	u8_t linkstatus;
};








//------------------------------------------------------------------------------------------------------------
//版本
//------------------------------------------------------------------------------------------------------------
struct verinfo {
	u8_t hwVer[7];
	u8_t swVer[7];
	u8_t fpgaVer[7];
};



//------------------------------------------------------------------------------------------------------------
//卫星接收机版本
//------------------------------------------------------------------------------------------------------------

#define	RCVRVER	"V2.0"
#define	RCVRTYPE	"GB_02"








//------------------------------------------------------------------------------------------------------------
//LED
//------------------------------------------------------------------------------------------------------------
enum {
	LED_OFF = 0,	//灯灭
	LED_ON = 1,		//灯亮
	LED_ALT = 3		//闪烁
};















//------------------------------------------------------------------------------------------------------------
//device
//------------------------------------------------------------------------------------------------------------
/* 
  FPGA device 
*/
#define	FPGA_DEV	"/dev/p350fpga"


/*
  PS device
*/
#define	PS_DEV		"/dev/p350ps"


/*
  INT device
*/
#define	INT0_DEV		"/dev/p350int0"// key interrupt
#define	INT1_DEV		"/dev/p350int1"// pps interrupt
#define	INT2_DEV		"/dev/p350int2"// pps interrupt

/*load device*/

#define LOAD_DEV		"/dev/p350load"














//------------------------------------------------------------------------------------------------------------
//www
//------------------------------------------------------------------------------------------------------------
#define WWW_PATH				"/usr/p350/www"
#define	WWW_INDEX_MASTER_PATH	"/usr/p350/www/index.master.html"
#define	WWW_INDEX_SLAVE_PATH	"/usr/p350/www/index.slave.html"
#define	WWW_INDEX_PATH			"/usr/p350/www/index.html"
#define WWW_VERSION_FILE		"/usr/p350/www/cgi-bin/sys_manage/ver"

//------------------------------------------------------------------------------------------------------------
//snmp(可选功能)
//------------------------------------------------------------------------------------------------------------
#define SNMP_PATH				"/usr/p350/snmp"
#define SNMP_MIB_VERSION_FILE		"/usr/p350/snmp/version"


//------------------------------------------------------------------------------------------------------------
//DB
//------------------------------------------------------------------------------------------------------------
//数据库
#define DB_PATH				"/data/db/p350.db"
#define	DB_ORGIN_PATH		"/usr/p350/db/p350.db"

#define USER_DATA_DB_DIR	"/data/db"
#define USER_DATA_MAC_DIR	"/data/mac"
//MAC数据库
//as same as p350.db
#define MAC_ORGIN_PATH		"/data/mac/mac_orgin.db"
// only mac/ip
#define	MAC_BAKUP_PATH		"/usr/p350/mac/mac.db"
#define	MAC_PATH			"/data/mac/mac.db"


#define	FPGA_PATH			"/usr/p350/fpga/fpga.rbf"

//系统配置表
#define TBL_USER			"TBL_USER"
#define	TBL_SYS				"TBL_SYS"
#define	TBL_MD5_KEY			"TBL_MD5_KEY"
#define	TBL_ALARM_HISTORY	"TBL_ALARM_HISTORY"
#define	TBL_ALARM_LIST		"TBL_ALARM_LIST"
#define	TBL_ALARM_SELECT	"TBL_ALARM_SELECT"
#define	TBL_ALARM_SHIELD	"TBL_ALARM_SHIELD"
#define	TBL_61850			"TBL_61850"
#define	TBL_EVENT_CURRENT	"TBL_EVENT_CURRENT"
#define	TBL_EVENT_HISTORY	"TBL_EVENT_HISTORY"
#define	TBL_PERF_CURRENT	"TBL_PERF_CURRENT"
#define	TBL_PERF_HISTORY	"TBL_PERF_HISTORY"


//输入输出盘的默认配置表
#define	TBL_IO_DEFAULT		"TBL_IO_DEFAULT"

//输入输出盘的实时配置表
#define	TBL_IO_RUNTIME		"TBL_IO_RUNTIME"

//虚拟网卡
#define	TBL_VETH			"TBL_VETH"

//日志
#define	TBL_LOG				"TBL_LOG"




//------------------------------------------------------------------------------------------------------------
//leap info
struct leapinfo {
	u8_t leapForecast;//闰秒预告, 在闰秒变化前59s置1, 闰秒变化后置0
	u8_t leapstate; //0-无操作, 1-正闰秒, 2-负闰秒
	//u8_t leaptime[20];
	struct timeinfo leaptime;
	s8_t leapSecond;
	u8_t leapMode;
};



//clock state info
struct clock_stainfo{
	u16_t state;
	int phase;
};







//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//debug info
struct dbginfo {
	u8_t dbg_level;//0 ~ 8
	u8_t syslog_en;
};











//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//reboot info of manager
struct rebootinfo {
	u8_t bReboot;// 1 for reboot
};


//update info of manager
struct updateinfo {
	u8_t bUpdate;// 1 for update
};












//------------------------------------------------------------------------------------------------------------
//NTP
//------------------------------------------------------------------------------------------------------------
#define	MIN_BCAST_ITVL		1
#define	MAX_BCAST_ITVL		1024


//广播
struct ntpbc {
	u8_t bcEnable;
	u16_t bcInterval;
};





//版本使能
struct ntpve {
	/*
	1	enable
	0	disable
	
	bit0	V2
	bit1	V3
	bit2	V4
	*/
	u8_t VnEnable;
};





#define	MAX_MD5KEY_LEN		20
#define	MAX_MD5KEY_NUM		10


//MD5
struct ntpmk {
	u32_t md5Id;
	u8_t md5Len;
	u8_t md5Key[MAX_MD5KEY_LEN +1];
};





struct ntpme {
	/*
	1	enable
	0	disable
	*/
	u8_t md5Enable;
};





struct ntpsta {
	struct ntpbc bCast;//广播
	struct ntpve vEn;//版本使能
	struct ntpme mEn;//MD5使能
	struct veth vp[V_ETH_MAX];//虚拟网卡
	u32_t totalKey;//MD5key总数
	struct ntpmk mKey[MAX_MD5KEY_NUM];//MD5
};













//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//cursor of slot
enum {
	SLOT_CURSOR_1 = 0,
	SLOT_CURSOR_2,
	SLOT_CURSOR_3,
	SLOT_CURSOR_4,
	SLOT_CURSOR_5,
	SLOT_CURSOR_6,
	SLOT_CURSOR_7,
	SLOT_CURSOR_8,
	SLOT_CURSOR_9,
	SLOT_CURSOR_10,
	SLOT_CURSOR_11,
	SLOT_CURSOR_12,
	SLOT_CURSOR_13,
	SLOT_CURSOR_14,
	SLOT_CURSOR_15,
	SLOT_CURSOR_16,
	SLOT_CURSOR_ARRAY_SIZE	//16
};
//FOR CMD HELP
#define	OUTPUT_MAX_SLOT 16


#define SLOT_CURSOR_NONE (-1)

enum{
	OTHER_SLOT_CURSOR_17 = 16,
	OTHER_SLOT_CURSOR_18 = 17,
	OTHER_SLOT_CURSOR_19 = 18,
	OTHER_SLOT_CURSOR_20 = 19,
};

enum {
	PWR_SLOT_CURSOR_1 = 0,
	PWR_SLOT_CURSOR_2,
	PWR_SLOT_CURSOR_ARRAY_SIZE /*PWR slot max size: 2*/
};

enum {
	INPUT_SLOT_CURSOR_1 = 0,
	INPUT_SLOT_CURSOR_2,
#ifdef NEW_3_4_IN_SLOT
	INPUT_SLOT_CURSOR_3,
	INPUT_SLOT_CURSOR_4,
#endif
	INPUT_SLOT_CURSOR_ARRAY_SIZE /*INPUT slot max size: 2/4*/
};
#ifdef NEW_3_4_IN_SLOT
// for CMD HELP MICRO   SHOULD  INPUT_SLOT_CURSOR_ARRAY_SIZE
#define	INPUT_MAX_SLOT 4
#else
#define	INPUT_MAX_SLOT 2
#endif


#define SLOT_NONE (SLOT_CURSOR_NONE+1)

#define	PWR_VOL_UPPER	197
#define	PWR_VOL_LOWER	165






//board identifier
enum {
	//none
	BID_NONE	= 0,//无盘
	
	//out
	BID_NTP		= 1,
	BID_NTPF	= 2,
	BID_RS232	= 3,
	BID_RS485	= 4,
	BID_BDCT	= 5,//此盘已经取消，预留
	BID_BAC		= 6, //板卡功能：直流b码转成交流b码
	BID_SF4		= 7,
	BID_BDC485	= 8, //此盘已经取消，预留
	BID_KJD8	= 9,
	BID_TTL8	= 10,
	BID_PPX485	= 11, //此盘已经取消，预留
	BID_PTP		= 12,
	BID_PTPF	= 13, //此盘取消，预留
	BID_OUTE	= 14, //此盘取消，预留
	BID_OUTH	= 15, //此盘取消，预留
	//61850 independent
	BID_61850	= 16,
	//new 20171220
	BID_KJD4	= 17,
	BID_TP4 	= 18,	//1pps+tod outboard new
	BID_SC4 	= 19,	//2MHz/2Mbit/10MHz outboard

	//in
	BID_GPSBF	= 60, //reserved satellite start
	BID_GPSBE	= 61,	//reserved
	BID_GBDBF	= 62,	//GPS BD Fiber(光口)
	BID_GBDBE	= 63,	//GPS BD Elec(电口)
	BID_BDBF	= 64,	//reserved
	BID_BDBE	= 65,	//reserved satellite end
	BID_BEI		= 66,	//only irgb1,irgb2  reserved
	BID_BFI		= 67,	//only irgb1,irgb2  reserved
	BID_BFEI	= 68,	////1路2Mhz/2Mbit 和两路IRIGB
	BID_BFFI	= 69,	////1路2Mhz/2Mbit 和两路IRIGB
	BID_PTP_IN  = 70,
	//new 20171220
	BID_RTF		=71, //1PPS+TOD、10MHz、2MHz REFER INBOARD
	BID_NTP_IN	=72,	//NTP IN BOARD NEW
	//EXT 扩展框驱动子卡(主框) 预留
	//SCC 扩展框管理子卡(扩展框) 预留
	//power
	BID_PWRM	= 90,
	BID_PWRD	= 91,
	BID_PWRL	= 109,//此盘取消，预留

	//alarm
	BID_ALM		= 110,
	
	BID_ARRAY_SIZE = 111
};



#define	BID_OUT_LOWER	BID_NTP
#define	BID_OUT_UPPER	BID_SC4	//61850 IS manage infoout board,not out board,by xpc

#define	BID_IN_LOWER	BID_GPSBF
#define	BID_IN_UPPER	BID_NTP_IN

#define	BID_INSATE_LOWER	BID_GPSBF
#define	BID_INSATE_UPPER	BID_BFFI

#define	BID_PWR_LOWER	BID_PWRM
#define	BID_PWR_UPPER	BID_PWRD

#define	BID_ALM_LOWER	BID_ALM
#define	BID_ALM_UPPER	BID_ALM



extern u8_t gBoardNameTbl[BID_ARRAY_SIZE][16];
void initializeBoardNameTable(void);


//enable
enum {
	DISABLE = 0,
	ENABLE	= 1
};






//signal type
#define		SIGNAL_TYPE_NONE		"none"
#define		SIGNAL_TYPE_NTP			"ntp"
#define		SIGNAL_TYPE_PTP			"ptp"
#define		SIGNAL_TYPE_2MH			"2mh"
#define		SIGNAL_TYPE_2MB			"2mb"
#define		SIGNAL_TYPE_IRIGB_DC	"irigb-dc"
#define		SIGNAL_TYPE_TODT		"todt"
#define		SIGNAL_TYPE_TODF		"todf"
#define		SIGNAL_TYPE_PPS			"pps"
#define		SIGNAL_TYPE_PPM			"ppm"
#define		SIGNAL_TYPE_PPH			"pph"
//#define		SIGNAL_TYPE_SATELLITE	"satellite"
#define		SIGNAL_TYPE_IRIGB_AC	"irigb-ac"
#define		SIGNAL_TYPE_EXT			"ext"
#define		SIGNAL_TYPE_10MH		"10mh"


//必须小于32,4个字节，因为struct boardattr_tbl_t gBoardAttrTbl用到了位结构
enum {
	SIGNAL_NONE			= 0,
	SIGNAL_NTP			= 1,
	SIGNAL_PTP			= 2,
	SIGNAL_2MH			= 3,
	SIGNAL_2MB			= 4,
	SIGNAL_IRIGB_DC		= 5,//b码直流
	SIGNAL_TODT			= 6,//电力tod
	SIGNAL_TODF			= 7,//1pps+tod
	SIGNAL_PPS			= 8,
	SIGNAL_PPM			= 9,
	SIGNAL_PPH			= 10,
	SIGNAL_IRIGB_AC		= 11,
	SIGNAL_EXT			= 12,
	SIGNAL_10MH			= 13,//new,but input signal,no use
	SIGNAL_ARRAY_SIZE
};


extern char gSignalTbl[SIGNAL_ARRAY_SIZE][10];












//波特率
#define		BAUDRATE_TYPE_1200		"1200"
#define		BAUDRATE_TYPE_2400		"2400"
#define		BAUDRATE_TYPE_4800		"4800"
#define		BAUDRATE_TYPE_9600		"9600"
#define		BAUDRATE_TYPE_19200		"19200"




enum {
	BAUDRATE_1200		= 0,	//1200
	BAUDRATE_2400		= 1,	//2400
	BAUDRATE_4800		= 2,	//4800
	BAUDRATE_9600		= 3,	//9600
	BAUDRATE_19200		= 4,	//19200
	BAUDRATE_ARRAY_SIZE
};



extern u8_t gBaudRateTbl[BAUDRATE_ARRAY_SIZE][8];





struct out2mbsainfo {
	//(#1-#4)/(#5-#8)/(#9-#12)/(#13-#16)
	u8_t mb_sa[4]; //current sys out ssm for the 2mb out.代表ssm信息在第几个sa发出
};

struct baudrate {
	//(#1-#4)/(#5-#8)/(#9-#12)/(#13-#16)
	u8_t br[5];
};









//time zone
#define		ZONE_TYPE_ZERO		"zero"
#define		ZONE_TYPE_E1		"e1"
#define		ZONE_TYPE_E2		"e2"
#define		ZONE_TYPE_E3		"e3"
#define		ZONE_TYPE_E4		"e4"
#define		ZONE_TYPE_E5		"e5"
#define		ZONE_TYPE_E6		"e6"
#define		ZONE_TYPE_E7		"e7"
#define		ZONE_TYPE_E8		"e8"
#define		ZONE_TYPE_E9		"e9"
#define		ZONE_TYPE_E10		"e10"
#define		ZONE_TYPE_E11		"e11"
#define		ZONE_TYPE_EW12		"ew12"
#define		ZONE_TYPE_W11		"w11"
#define		ZONE_TYPE_W10		"w10"
#define		ZONE_TYPE_W9		"w9"
#define		ZONE_TYPE_W8		"w8"
#define		ZONE_TYPE_W7		"w7"
#define		ZONE_TYPE_W6		"w6"
#define		ZONE_TYPE_W5		"w5"
#define		ZONE_TYPE_W4		"w4"
#define		ZONE_TYPE_W3		"w3"
#define		ZONE_TYPE_W2		"w2"
#define		ZONE_TYPE_W1		"w1"




enum {
	TZONE_ZERO	= '@',//零时区
	TZONE_E1	= 'A',//东一区
	TZONE_E2	= 'B',//东二区
	TZONE_E3	= 'C',//东三区
	TZONE_E4	= 'D',//东四区
	TZONE_E5	= 'E',//东五区
	TZONE_E6	= 'F',//东六区
	TZONE_E7	= 'G',//东七区
	TZONE_E8	= 'H',//东八区
	TZONE_E9	= 'I',//东九区
	TZONE_E10	= 'J',//东十区
	TZONE_E11	= 'K',//东十一区
	TZONE_EW12	= 'L',//东/西十二区
	TZONE_W11	= 'M',//西十一区
	TZONE_W10	= 'N',//西十区
	TZONE_W9	= 'O',//西九区
	TZONE_W8	= 'P',//西八区
	TZONE_W7	= 'Q',//西七区
	TZONE_W6	= 'R',//西六区
	TZONE_W5	= 'S',//西五区
	TZONE_W4	= 'T',//西四区
	TZONE_W3	= 'U',//西三区
	TZONE_W2	= 'V',//西二区
	TZONE_W1	= 'W' //西一区
};



extern u8_t gZoneTbl[24][5];








struct inzoneinfo {
	//(#1 irigb1)/(#1 irigb2)/(#2 irigb1)/(#2 irigb2)
	//volatile u8_t zone[5];
	u8_t zone[5];
};





//未来可能需要改为zone变量，因为统一成为系统1个了
struct outzoneinfo {
	//(#1-#4)/(#5-#8)/(#9-#12)/(#13-#16)
	u8_t zone[5];
};







#define OUTDELAY_SIGNALTYPE_LEN 4

enum {
	OUTDELAY_SIGNAL_TOD	= 1,
	OUTDELAY_SIGNAL_IRIGB	= 2,
	OUTDELAY_SIGNAL_PPX	= 3,
	OUTDELAY_SIGNAL_PTP	= 4
};

	struct outdelay_tbl_t {
		// signal type
		// 1 tod
		// 2 irigb
		// 3 ppx
		// 4 PTP OUT
		u8_t signal;
	
		//SIGNAL_NAME
		char * signal_name;
	};

#define FIND_SIGN_BY_SIGNME_OUTDELAY(signme, signme_len, sign) \
	FIND_OTHMEMBER_BY_STRMEMBER(signme, signme_len, sign, gOutdelayTbl, ARRAY_SIZE(gOutdelayTbl), signal_name, signal)

#define FIND_SIGNME_BY_SIGN_OUTDELAY(sign, signme) \
	FIND_OTHMEMBER_BY_MEMBER(sign, signme, gOutdelayTbl, ARRAY_SIZE(gOutdelayTbl), signal, signal_name)

extern struct outdelay_tbl_t gOutdelayTbl[OUTDELAY_SIGNALTYPE_LEN];

//------------------------------------------------------------------------------------------------------------
//根据输出信号类型管理输出时延
//------------------------------------------------------------------------------------------------------------
struct outdelay {
	// signal type
	// 1 tod
	// 2 irigb
	// 3 ppx
	// 4 PTP OUT
	u8_t signal;

	//delay
	int delay;
};











//------------------------------------------------------------------------------------------------------------
//irigb幅度比
//------------------------------------------------------------------------------------------------------------
#define	AMPLITUDE_RATIO_TYPE_3_1		"3:1"
#define	AMPLITUDE_RATIO_TYPE_4_1		"4:1"
#define	AMPLITUDE_RATIO_TYPE_5_1		"5:1"
#define	AMPLITUDE_RATIO_TYPE_6_1		"6:1"







enum {
	AMPLITUDE_RATIO_3_1	= 0,
	AMPLITUDE_RATIO_4_1,
	AMPLITUDE_RATIO_5_1,
	AMPLITUDE_RATIO_6_1,
	AMPLITUDE_RATIO_ARRAY_SIZE
};









extern u8_t gAmplitudeRatioTbl[AMPLITUDE_RATIO_ARRAY_SIZE][4];










//未来改为一个amp
struct amplitude {
	//(#1-#4)/(#5-#8)/(#9-#12)/(#13-#16)
	u8_t amp[5];
};











//------------------------------------------------------------------------------------------------------------
//irigb电压
//------------------------------------------------------------------------------------------------------------
#define	VOLTAGE_TYPE_3V		"3V"
#define	VOLTAGE_TYPE_4V		"4V"
#define	VOLTAGE_TYPE_5V		"5V"
#define	VOLTAGE_TYPE_6V		"6V"
#define	VOLTAGE_TYPE_7V		"7V"
#define	VOLTAGE_TYPE_8V		"8V"
#define	VOLTAGE_TYPE_9V		"9V"
#define	VOLTAGE_TYPE_10V	"10V"
#define	VOLTAGE_TYPE_11V	"11V"
#define	VOLTAGE_TYPE_12V	"12V"









enum {
	VOLTAGE_3V		= 0,
	VOLTAGE_4V,
	VOLTAGE_5V,
	VOLTAGE_6V,
	VOLTAGE_7V,
	VOLTAGE_8V,
	VOLTAGE_9V,
	VOLTAGE_10V,
	VOLTAGE_11V,
	VOLTAGE_12V,
	VOLTAGE_ARRAY_SIZE
};








extern u8_t gVoltageTbl[VOLTAGE_ARRAY_SIZE][4];












//未来改为一个系统b码输出vol
struct voltage {
	//(#1-#4)/(#5-#8)/(#9-#12)/(#13-#16)
	u8_t vol[5];
};














//ptp delay type
enum {
	//The port is configured to use the delay request-response mechanism
	DELAY_P2P = 0x00,
	//The port is configured to use the peer delay mechanism
    DELAY_E2E,
    PTP_DELAY_ARRAY_SIZE // 2
};



#define		PTP_DELAY_TYPE_P2P			"p2p"
#define		PTP_DELAY_TYPE_E2E			"e2e"

enum {
	//The port is configured to use the delay request-response mechanism
	PTP_MASTER = 0x00,
	//The port is configured to use the peer delay mechanism
    PTP_SLAVE,
    PTP_MODEL_ARRAY_SIZE // 2
};

#define		PTP_MODEL_TYPE_MASTER			"master"
#define		PTP_MODEL_TYPE_SLAVE			"slave"


//ptp multicast or unicast type
enum {
	UNICAST = 0x00,
	MULTICAST,
	PTP_MULTI_UNI_ARRAY_SIZE // 2
};



#define		PTP_MULTI_UNI_TYPE_UNI		"uni"
#define		PTP_MULTI_UNI_TYPE_MULTI	"multi"




//ptp layer type
enum {
	LAYER2 = 0x00,
	LAYER3,
	PTP_LAYER_ARRAY_SIZE // 2
};




#define		PTP_LAYER_TYPE_L2			"layer2"
#define		PTP_LAYER_TYPE_L3			"layer3"




//ptp step type
enum {
	ONE_STEP = 0x00,
	TWO_STEP,
	PTP_STEP_ARRAY_SIZE // 2
};




#define		PTP_STEP_TYPE_1STEP			"1step"
#define		PTP_STEP_TYPE_2STEP			"2step"



//ptp frequency
enum {
	FREQ_256_1 = 0x00,// 256 per second
	FREQ_128_1,
	FREQ_64_1,
	FREQ_32_1,
	FREQ_16_1,
	FREQ_8_1,
	FREQ_4_1,
	FREQ_2_1,
	FREQ_1_1,
	FREQ_1_2,// 1 per 2 second
	FREQ_1_4,
	FREQ_1_8,
	FREQ_1_16,
	FREQ_1_32,
	FREQ_1_64,
	FREQ_1_128,
	FREQ_1_256,
	PTP_FREQ_ARRAY_SIZE // 17
};

//相当于取反		define						show	exp					value	index	offset	true_value
#define		PTP_FREQ_256_1				"256"	// 256 per second	//8		0		-8		-8
#define		PTP_FREQ_128_1				"128"	// 128 per second	//7		1				-7
#define		PTP_FREQ_64_1				"64"	// 64 per second	//6		2				-6
#define		PTP_FREQ_32_1				"32"	// 32 per second	//5		3				-5
#define		PTP_FREQ_16_1				"16"	// 16 per second	//4		4				-4
#define		PTP_FREQ_8_1				"8"	// 8 per second			//3		5				-3
#define		PTP_FREQ_4_1				"4"	// 4 per second			//2		6				-2
#define		PTP_FREQ_2_1				"2"	// 2 per second			//1		7				-1
#define		PTP_FREQ_1_1				"1"	// 1 per second			//0		8				0
#define		PTP_FREQ_1_2				"1/2"	// 1 per 2 second	//-1	9				1
#define		PTP_FREQ_1_4				"1/4"	// 1 per 4 second	//-2	10				2
#define		PTP_FREQ_1_8				"1/8"	// 1 per 8 second	//-3	11
#define		PTP_FREQ_1_16				"1/16"	// 1 per 16 second	//-4	12
#define		PTP_FREQ_1_32				"1/32"	// 1 per 32 second	//-5	13
#define		PTP_FREQ_1_64				"1/64"	// 1 per 64 second	//-6	14
#define		PTP_FREQ_1_128				"1/128"	// 1 per 128 second	//-7	15
#define		PTP_FREQ_1_256				"1/256"	// 1 per 256 second	//-8	16				8

enum {
	ZERO = 0x00,
	ONE,
	PTP_TRACE_ARRAY_SIZE // 2
};

#define		PTP_ZERO		"0"
#define		PTP_ONE			"1"



extern u8_t gPdtTbl[PTP_DELAY_ARRAY_SIZE][4];
extern u8_t gPmodeTbl[PTP_MODEL_ARRAY_SIZE][7];

extern u8_t gPmutTbl[PTP_MULTI_UNI_ARRAY_SIZE][6];
extern u8_t gPltTbl[PTP_LAYER_ARRAY_SIZE][7];
extern u8_t gPstTbl[PTP_STEP_ARRAY_SIZE][6];
extern u8_t gFreqTbl[PTP_FREQ_ARRAY_SIZE][6];
//extern u8_t gtraceTbl[PTP_TRACE_ARRAY_SIZE][2];


struct outcommoninfo {
	int delay;//output delay
	u8_t signalType;//output signal type
};

#if 0
struct rs232info {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct rs485info {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct bdctinfo {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct bacinfo {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct sf4info {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct bdc485info {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct kjd8info {
	int delay;//output delay
	u8_t signalType;//output signal type
};

struct kjd4info {
	int delay;//output delay
	u8_t signalType;//output signal type
};

struct ttl8info {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct ppx485info {
	int delay;//output delay
	u8_t signalType;//output signal type
};
#endif
/*ntohl(inet_addr((char *)STR_IPV4)) //255.255.255.255返回-1; 转为主机序*/
#define STRIP_TO_UINTIP(STR_IPV4)	inet_network((char *)STR_IPV4) //255.255.255.255:ffffffff返回-1

#define UINTIP_TO_STRIP(UINT_IPV4, STR_IPV4)	 do { \
	struct in_addr __in;	\
	__in.s_addr = htonl((uint32_t) UINT_IPV4); \
	inet_ntop(AF_INET, &__in, STR_IPV4, MAX_IP_LEN); \
} while(0) 
#define	MAX_IP_NUM		16

#define	MAX_IP_LEN		16
#define	MAX_MAC_LEN		18
#define MAX_VAR_LEN	    6
#define	MAX_MASK_LEN	16
#define	MAX_GATEWAY_LEN	16
#define MAX_DNS_LEN		16
//0-国外模块（default），1-国内模块
#define PTP_TYPE_FRGN	0
#define PTP_TYPE_CN		1

//p50
/*
老外模块，和ptp老外输入模块一致。
PtpSendConfig和Ptp2SendConfig共享一个数据结构，共享资源，只是根据用户确定类型，选用哪种
配置函数
与ptp共享bid，共同增加ptpType，0-国外；1-国内

国外模块和国内模块区别：
增加ptpMask, ptpGateway
无：ptpMac, ptpSsmEnable
改：ptpPdelayReq：表示国外delay，国内delay。我暂时没加国外pdelay
  1	成功
  0	失败
*/
struct ptpinfo {
	u8_t ptpType; //0-国外模块（default），1-国内模块
	//int ptpDelay;//output delay, no use
	//u8_t ptpSignalType;  move to outcommoninfo
	u8_t ptpDelayType;//delay type
	
	u8_t ptpMulticast;//multicast or unicast
	u8_t ptpLayer;//layer 2 or layer 3
	u8_t ptpStep;//one-step or two-step
	u8_t ptpSync;//frequency of sync
	u8_t ptpAnnounce;//frequency of announce
	
	u8_t ptpPdelayReq;//frequency of pdelay_req//代表国内pdelay，代表国外模块delay
	u8_t ptpSsmEnable;//only 国内模块
	u8_t ptpIp[MAX_IP_LEN];//ip
	u8_t ptpMac[MAX_MAC_LEN];//mac //only for 国内模块
	u8_t ptpMask[MAX_MASK_LEN];//mask //仅仅为国外模块
	u8_t ptpGateway[MAX_GATEWAY_LEN];//gateway   //仅仅为国外模块

	//u8_t ptpSlaveIp[MAX_IP_LEN];//Slave ip, old

	u32_t oldptpSlaveIp[MAX_IP_NUM];//Master ip for layer 3 //backup last value 国外模块
	u8_t oldptpSlaveIp_num;
	//u8_t oldptpMasterMac[MAX_MAC_LEN];//Master MAC layer 2 //new  国外模块
	u32_t ptpSlaveIp[MAX_IP_NUM];//Master ip for layer 3:ip: HOST order: max 16 nums // 国外模块支持多个ip，国内模块支持一个（我们使用第一个）
	u8_t ptpSlaveIp_num;
	//u8_t ptpMasterMac[MAX_MAC_LEN];//Master MAC layer 2 //new

	u8_t ptpLevel[4];//the clock which we will set (1,2) 
	u8_t ptpPriority[4];//the clock of priority
	
	u8_t ptpRegion[4];
};



struct ptpfinfo {
	//int ptpDelay;//output delay, no use
	u8_t ptpSignalType;  //support set ntp/ptp/none ,set move to outcommoninfo

	u8_t ptpDelayType;//delay type
	u8_t ptpModelType;//model
	u8_t ptpMulticast;//multicast or unicast
	u8_t ptpLayer;//layer 2 or layer 3
	u8_t ptpStep;//one-step or two-step
	u8_t ptpSync;//frequency of sync
	u8_t ptpAnnounce;//frequency of announce
	u8_t ptpDelayReq;//frequency of delay_req
	u8_t ptpPdelayReq;//frequency of pdelay_req
	u8_t ptpSsmEnable;
	u8_t ptpIp[MAX_IP_LEN];//ip
	u8_t ptpMac[MAX_MAC_LEN];//mac
	u8_t ptpMask[MAX_MASK_LEN];//mask
	u8_t ptpGateway[MAX_GATEWAY_LEN];//gateway
	u8_t ptpDns1[MAX_DNS_LEN];// 1st dns
	u8_t ptpDns2[MAX_DNS_LEN];// 2nd dns
	u8_t ptpSlaveIp[MAX_IP_LEN];//Slave ip
};


#if 0
struct outeinfo {
	int delay;//output delay
	u8_t signalType;//output signal type
};


struct outhinfo {
	int delay;//output delay
	u8_t signalType;//output signal type
};

struct tp4info {
	int delay;//output delay
	u8_t signalType;//output signal type
};

struct sc4info {
	int delay;//output delay
	u8_t signalType;//output signal type
};
#endif

//FOR set_which
#define OUTPUTINFO_SET_OUTCOMM_INFO	1
#define OUTPUTINFO_SET_PRIV_INFO	2


//output board config
struct outputinfo {
	u8_t slot;//slot of equipment
	u8_t boardId;//board identifier
	u8_t nPort;//port of board

	u8_t set_which;	//incomminfo or private(ps:ptp) info
	struct outcommoninfo outcomminfo;
	union {
		#if 0
		struct rs232info rs232;
		struct rs485info rs485;
		struct bdctinfo bdct;
		struct bacinfo bac;
		struct sf4info sf4;
		struct bdc485info bdc485;
		struct kjd8info kjd8;
		struct kjd4info kjd4;
		struct ttl8info ttl8;
		struct ppx485info ppx485;
		#endif
		struct ptpinfo ptp;
		struct ptpfinfo ptpf;
		#if 0
		struct outeinfo oute;
		struct outhinfo outh;
		struct tp4info tp4;
		struct sc4info sc4;
		#endif
	};
};


/*
  输出板卡的端口数量
*/
#define	PORT_NTP		4
#define	PORT_NTPF		4
#define	PORT_RS232		2
#define	PORT_RS485		8
#define	PORT_BDCT		8
#define PORT_BAC		8
#define	PORT_SF4		4
#define	PORT_BDC485		8
#define	PORT_KJD8		8
#define	PORT_KJD4		4
#define	PORT_TTL8		8
#define	PORT_PPX485		8
#define	PORT_PTP		1
#define	PORT_PTPF		1
#define	PORT_OUTE		8
#define	PORT_OUTH		8
#define	PORT_TP4		4
#define	PORT_SC4		4

/*if gBoardAttrTbl port increase, please change it. it is used to set the common delay ,prio*/
#define INPUT_SLOT_MAX_PORT		5
#define OUTPUT_SLOT_MAX_PORT	8
#define SLOT_MAX_PORT	OUTPUT_SLOT_MAX_PORT

//output board status 136bytes
struct outputsta {
	u8_t boardId;//board identifier

	struct outcommoninfo outcomminfo[OUTPUT_SLOT_MAX_PORT];//current only use outinfo[0]
	union {
		#if 0
		struct rs232info rs232[PORT_RS232];
		struct rs485info rs485[PORT_RS485];
		struct bdctinfo bdct[PORT_BDCT];
		struct bacinfo bac[PORT_BAC];
		struct sf4info sf4[PORT_SF4];
		struct bdc485info bdc485[PORT_BDC485];
		struct kjd8info kjd8[PORT_KJD8];
		struct kjd4info kjd4[PORT_KJD4];
		struct ttl8info ttl8[PORT_TTL8];
		struct ppx485info ppx485[PORT_PPX485];
		#endif
		struct ptpinfo ptp[PORT_PTP];
		struct ptpfinfo ptpf[PORT_PTPF];
		#if 0
		struct outeinfo oute[PORT_OUTE];
		struct outhinfo outh[PORT_OUTH];
		struct tp4info tp4[PORT_TP4];
		struct sc4info sc4[PORT_SC4];
		#endif
	};
};




//------------sys-config-------------
struct sys_conf_t
{
	u8_t ph_upload_en; //phasePerformance upload switch；1-enable 0-disable
};







//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------

struct inph_thresold
{
	//int signal;
	int Threshold;
};

/*
  性能上报结构体
*/
struct phase_perf_tbl_t {
	int index;//1-60
	int	val;//phase_val
	u8_t slot;//恒定
	u8_t port;//恒定
	u8_t clock_status;//钟的锁相环状态，自由运行，快捕...
	char time[20];//当前鉴相值的时间
};
extern struct phase_perf_tbl_t gPhasePerfTbl[];//only cmd help need
extern int gPhasePerfTbl_len;


#define TBL_PHASE_PERF_CURRENT_ID_MAX_SEQ			(0x7FFFFFFF)	//2147483647
#define	READ_A_PHASE_PER_SECOND						(60) //每60s采集一个点：1~60
#define TBL_PHASE_PERF_CURRENT_ARRAY_SIZE			(1)	//当前性能缓存/表的长度：1~60
#define TBL_PHASE_PERF_HISTORY_ARRAY_SIZE			(60*24*2)		//历史记录保存：1~1*2天
#define CMD_READ_TBL_TABLE_PER_PAGE_MAX_LEN				(60) //每页的长度范围：任意值

#ifdef NEW_3_4_IN_SLOT
#define	ALM_CURSOR_ARRAY_SIZE	651
#else
#define	ALM_CURSOR_ARRAY_SIZE	343
#endif

/* 
  do not output via ALM board
*/
#define	ALM_NONE	0



/*
  屏蔽状态
*/
enum {
	MASK_OFF = 0,//不屏蔽
	MASK_ON//屏蔽
};



/*
  告警状态
*/
enum {
	ALM_OFF = 0,//正常
	ALM_ON		//异常
};





extern u8_t gAlarmStatusTbl[2][16];









/*
  告警板卡的端口数量
*/
#define	PORT_ALM	8







/*
  告警板卡的输出状态
*/
struct selectinfo {
	u8_t alm_board_port;
	u32_t alm_inx;
};


struct selectsta {
	u32_t alm_inx;
};

/*
  告警的屏蔽状态
*/
struct maskinfo {
	u32_t alm_inx;
	u8_t mask_sta;
};


struct masksta {
	u8_t mask_sta;
};




/*
  old告警的告警状态
*/
struct alarmsta {
	u8_t alm_sta;
};

/*
  告警的索引状态，用于共享
*/
#define MAX_ALM_INDEX	(100)	//小于此区域共享内存的大小
struct alm_inx_sta {
	u32_t index;	//new alarm index，max：900
	u8_t alm_sta;
};

/*
告警的所有id编号表,固态
*/
struct alm_id_tbl_t {
	u32_t alm_id;
	u8_t alm_type;
	char *alm_msg;
};
extern struct alm_id_tbl_t gAlmIdTbl[];//only cmd help need
extern int gAlmIdTbl_len;

/*
  告警的所有索引编号表,固态
*/
struct alm_inx_tbl_t {
	//u8_t alm_type;
	u8_t slot;
	u8_t port;
	u32_t alm_id;
	struct alm_id_tbl_t *alm_id_tbl;
};

extern struct alm_inx_tbl_t *pAlmInxTbl;
extern u32_t max_alminx_num;




/*
  告警的开始时间和结束时间
*/
struct tssta {
	u8_t alm_start_time[20];
	u8_t alm_end_time[20];
};


#define CUR_ALM_MAX_ARRAY_SIZE	(50)
#define UPLOAD_ALM_MAX_ARRAY_SIZE	(20)
/*1.为已经发生告警的告警信息传输, 2k
2.为snmp告警上报；两种结构体相同，处理方式不同而已*/
/*sizeof(struct alminfo_t) = 24*/
struct alminfo_t {
		u32_t id;//记录id，必须唯一，且从1开始
		u16_t alm_inx;	//new alarm index，max：900
		u8_t alm_sta;
		struct timeinfo ti;
};


/*
*==========================event=========================================
*/


//: no found,  balm_index should default -1.
#define FIND_EVNTINX_BY_SLOT_PORT_ID(bevnt_id, bevnt_slot, bevnt_port, bevnt_index) \
	FIND_INDEX_BY_THREE_MEMBER(bevnt_index, bevnt_id, bevnt_slot, bevnt_port, pEvntInxTbl, max_evntinx_num, evnt_id, slot, port)

#define INIT_EVNTFLAG_BY_SLOT_PORT_GRP(bevnt_grp, bevnt_slot, bevnt_port, init_val) \
	SET_MEMBER_VAL_BY_THREE_MEMBER(init_val, bevnt_grp, bevnt_slot, bevnt_port, pEvntInxTbl, max_evntinx_num, new_flag, evnt_group, slot, port, 1)

#define INIT_EVNTFLAG(init_val) \
		SET_MEMBER_VAL(init_val, pEvntInxTbl, max_evntinx_num, new_flag)

#define INIT_EVNTFLAG_BY_SLOT_PORT_ID(bevnt_id, bevnt_slot, bevnt_port, init_val) \
		SET_MEMBER_VAL_BY_THREE_MEMBER(init_val, bevnt_id, bevnt_slot, bevnt_port, pEvntInxTbl, max_evntinx_num, new_flag, evnt_id, slot, port, 0)

#define SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(bevnt_id, bevnt_slot, bevnt_port) \
		SET_EVNTFLAGVALID_BID_BY_SLOT_PORT_ID(bevnt_id, bevnt_slot, bevnt_port, BID_NONE)

#define SET_EVNTFLAGVALID_BID_BY_SLOT_PORT_ID(bevnt_id, bevnt_slot, bevnt_port, bevnt_bid) \
	do{ \
		int __i;	\
		struct timeinfo __ti; \
		for (__i = 0; __i < max_evntinx_num; ++__i) { \
			if(bevnt_id == pEvntInxTbl[__i].evnt_id \
			 && bevnt_slot == pEvntInxTbl[__i].slot \
			 && bevnt_port == pEvntInxTbl[__i].port){ \
			    pEvntInxTbl[__i].new_flag = 1; \
				if (pEvntInxTbl[__i].old_flag == 0 && pEvntInxTbl[__i].new_flag == 1){ \
					pEvntInxTbl[__i].valid = 1;	\
					pEvntInxTbl[__i].bid = bevnt_bid; \
					if(0 == GetSysTime(&__ti)){ \
						break; \
					} \
					sprintf((char *) pEvntInxTbl[__i].start_time, "%04d-%02d-%02d %02d:%02d:%02d", \
					__ti.year, __ti.month, __ti.day, __ti.hour, __ti.minute, __ti.second); \
					break; \
				} \
			} \
		} \
	} while(0)


/*
事件的所有id编号表,固态
*/
struct evnt_id_tbl_t {
	u32_t evnt_id;
	u8_t evnt_type;
	u8_t evnt_group;//事件组，共享同一flag
	char *evnt_msg;
};
extern struct evnt_id_tbl_t gEvntIdTbl[];//only cmd help need
extern int gEvntIdTbl_len;


/*
  事件的所有索引编号表,固态
*/
struct evnt_inx_tbl_t {
	int index;//恒定
	u8_t old_flag;//动态变化
	u8_t new_flag;//动态变化
	u8_t valid;////动态变化。事件是否有效; 如果old_flag==0&&new_flag==1, set valid =1;   if write record to database ok, set valid =0
	//u8_t evnt_type;
	u8_t slot;//恒定
	u8_t port;//恒定
	u8_t bid;//恒定
	u32_t evnt_id;//恒定
	u8_t evnt_group;//恒定
	char start_time[20];//动态变化
	struct evnt_id_tbl_t *evnt_id_tbl;//恒定
};

extern struct evnt_inx_tbl_t *pEvntInxTbl;
extern u32_t max_evntinx_num;


#define TBL_EVENT_CURRENT_ID_MAX_SEQ			(0x7FFFFFFF)	//2147483647
#define TBL_EVENT_CURRENT_ARRAY_SIZE			(20)
#define TBL_EVENT_HISTORY_ARRAY_SIZE			(10000)

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//ssm游标
enum {
	SSM_CURSOR_02 = 0,
	SSM_CURSOR_04,
	SSM_CURSOR_08,
	SSM_CURSOR_0B,
	SSM_CURSOR_0F,
	SSM_CURSOR_00,
	SSM_CURSOR_NE,
	SSM_CURSOR_ARRAY_SIZE
};

//ssm标准值定义
enum {
	SSM_VAL_02 = 0x02,
	SSM_VAL_04 = 0x04,
	SSM_VAL_08 = 0x08,
	SSM_VAL_0B = 0x0B,
	SSM_VAL_0F = 0x0F,
	SSM_VAL_00 = 0x00,
	SSM_VAL_NE = 0x01 //此处为自定义，表示该路无ssm值
};


//ssm定义
#define	SSM_02	"02"
#define	SSM_04	"04"
#define	SSM_08	"08"
#define	SSM_0B	"0B"
#define	SSM_0F	"0F"
#define	SSM_00	"00"
#define	SSM_NE	"--"


struct tbl_ssm_t {
	u8_t index;
	u8_t val;
	char *v_str;//[STR_SSM_LEN+1]
};

extern struct tbl_ssm_t gSsmTbl[];
extern int gSsmTbl_len;

#define FIND_STR_BY_SSM_VAL(ssm_val, ssm_str) \
		FIND_OTHMEMBER_BY_MEMBER(ssm_val, ssm_str, gSsmTbl, gSsmTbl_len, val, v_str)

#define FIND_VAL_BY_SSM_STR(ssm_str, ssm_val) \
	FIND_OTHMEMBER_BY_STRMEMBER(ssm_str, STR_SSM_LEN, ssm_val, gSsmTbl, gSsmTbl_len, v_str, val)

//sa游标
enum {
	SA_CURSOR_SA4 = 0,
	SA_CURSOR_SA5,
	SA_CURSOR_SA6,
	SA_CURSOR_SA7,
	SA_CURSOR_SA8,
	SA_CURSOR_ARRAY_SIZE
};

//sa定义
#define	SA_SA4	"SA4"
#define	SA_SA5	"SA5"
#define	SA_SA6	"SA6"
#define	SA_SA7	"SA7"
#define	SA_SA8	"SA8"

extern u8_t gSaTbl[SA_CURSOR_ARRAY_SIZE][4];

//signal
#define INSATE_BD_GPS_SGN_PORT1(signal)	(signal &BIT(4))
#define INSATE_2MH_SGN_PORT2(signal)	(signal &BIT(1))
#define INSATE_2MB_SGN_PORT3(signal)	(signal &BIT(0))
#define INSATE_IRIGB1_SGN_PORT4(signal)	(signal &BIT(2))
#define INSATE_IRIGB2_SGN_PORT5(signal)	(signal &BIT(3))

#define INPTP_PTP_SGN_PORT1(signal)	(signal &BIT(4))
#define INPTP_NTP_SGN_PORT1(signal)	(signal &BIT(4))

#define INRTF_1PPS_TOD_SGN_PORT1(signal)	(signal &BIT(4))
#define INRTF_1PPS_SGN_PORT2(signal)	(signal &BIT(5))
#define INRTF_10MH1_SGN_PORT3(signal)	(signal &BIT(6))
#define INRTF_10MH2_SGN_PORT4(signal)	(signal &BIT(7))


//接收模式游标
enum {
	MODE_CURSOR_GPS = 0,
	MODE_CURSOR_BD,
	MODE_CURSOR_MIX_GPS,
	MODE_CURSOR_MIX_BD,
	MODE_CURSOR_ARRAY_SIZE
};
//接收模式游标
enum {
	MODE_CURSOR_NONE_VAL = 0x00,
	MODE_CURSOR_GPS_VAL = 0x01,
	MODE_CURSOR_BD_VAL = 0x02,
	MODE_CURSOR_MIX_GPS_VAL = 0x03,
	MODE_CURSOR_MIX_BD_VAL = 0x04,
};


//接收模式定义
#define	MODE_GPS		"SG"
#define	MODE_BD			"SB"
#define	MODE_MIX_GPS	"MG"
#define	MODE_MIX_BD		"MB"

//接收机型号定义
//#define XHTFBG-22       1
//#define BX-T21BG		2
#define OLD_MODEL       1
#define NEW_MODEL       2
extern u8_t gModeTbl[MODE_CURSOR_ARRAY_SIZE][3];








enum {
	SATELLITE_ANTENNA_LOOP		= '1',//短路
	SATELLITE_ANTENNA_NORMAL	= '2',//正常
	SATELLITE_ANTENNA_OPEN		= '3' //开路
};








#define	MAX_GPS_STAR	24
#define	MAX_BD_STAR		24



struct starinfo {
	u8_t id;
	u8_t snr;
};

/*
输入板卡的端口数量
*/
#define PORT_SAT	5
#define PORT_PTP_IN	1
#define PORT_RTF	4
#define PORT_NTP_IN	1

//卫星板卡端口表示
#define SAT_PORT_SAT	1
#define SAT_PORT_2MH	2
#define SAT_PORT_2MB	3
#define SAT_PORT_IRIGB1	4
#define SAT_PORT_IRIGB2	5

//RTF端口表示
#define RTF_PORT_1PPS_TOD	1
#define RTF_PORT_1PPS	2
#define RTF_PORT_10MH1	3
#define RTF_PORT_10MH2	4


#define STR_SSM_LEN	2
struct incommoninfo {
	int delay[INPUT_SLOT_MAX_PORT];//0-satellite, 3-igirb1, 4-igirb2
	u8_t prio[INPUT_SLOT_MAX_PORT+1];
	u8_t ssm[INPUT_SLOT_MAX_PORT];//00 02 04 08 0b 0f --
};


struct incommonsta {
	struct incommoninfo incomminfo;
};

#if 1//def USE_SAT_COMMON_STA_TRUCT
#define FLAG_SAT_COMMINFO_MH_SSM	BIT(0)
#define FLAG_SAT_COMMINFO_MB_SA		BIT(1)
#define FLAG_SAT_COMMINFO_SYSMODE	BIT(2)
//#define FLAG_SAT_COMMINFO_DELAY		BIT(3)
#define FLAG_SAT_COMMINFO_ELEV		BIT(3)
//#define FLAG_SAT_COMMINFO_PRIO		BIT(5)
#define FLAG_SAT_COMMINFO_LON		BIT(4)
#define FLAG_SAT_COMMINFO_LAT		BIT(5)
#define FLAG_SAT_COMMINFO_MASK		(BIT(6)-1)

//除bei bfi, bfei bffi, 以外的所有卫星板卡
//HAVE gpsbf gpsbe, gbdbf gbdbe, bdbf bdbe, 6种
#define FLAG_SAT_ALL_COMMINFO	FLAG_SAT_COMMINFO_MASK

//HAVE bei bfi	2种
#define FLAG_SAT_BEI_BFI_COMMINFO	0
	//(FLAG_SAT_COMMINFO_DELAY | FLAG_SAT_COMMINFO_PRIO)

//HAVE bfei bffi   2种
#define FLAG_SAT_BFEI_BFFI_COMMINFO	(FLAG_SAT_COMMINFO_MH_SSM | FLAG_SAT_COMMINFO_MB_SA) 
	//| //FLAG_SAT_COMMINFO_DELAY | FLAG_SAT_COMMINFO_PRIO)

/*卫星通用信息*/
struct satcommoninfo {
	int info_flag;
	//u8_t mh_ssm[3];//for 2mhz
	u8_t mb_sa[4];//for 2mb, 然后fpga再调节2mb的mb_ssm
	u8_t sysMode;
	//int delay[PORT_SAT];//0-satellite, 3-igirb1, 4-igirb2 //move to incomm
	u8_t elev[8];
	//u8_t prio[PORT_SAT+1];// buf
	u8_t lon[13];
	u8_t lat[12];
};

#define FLAG_SAT_COMMSTA_LEAPSTATE	BIT(0)
#define FLAG_SAT_COMMSTA_TRACKMODE	BIT(1)
#define FLAG_SAT_COMMSTA_WORKMODE	BIT(2)
#define FLAG_SAT_COMMSTA_ELEVTHD	BIT(3)
#define FLAG_SAT_COMMSTA_FDRSTATUS	BIT(4)
#define FLAG_SAT_COMMSTA_RCVRVER	BIT(5)
#define FLAG_SAT_COMMSTA_RCVRTYPE	BIT(6)
#define FLAG_SAT_COMMSTA_UTC		BIT(7)
#define FLAG_SAT_COMMSTA_PST_STATUS	BIT(8)
#define FLAG_SAT_COMMSTA_TRACKNGB	BIT(9)
#define FLAG_SAT_COMMSTA_GBS		BIT(10)
#define FLAG_SAT_COMMSTA_NGPS		BIT(11)
#define FLAG_SAT_COMMSTA_VGPS		BIT(12)
#define FLAG_SAT_COMMSTA_NBD		BIT(13)
#define FLAG_SAT_COMMSTA_VBD		BIT(14)
#define FLAG_SAT_COMMSTA_PH			BIT(15)
#define FLAG_SAT_COMMSTA_MASK		(BIT(16)-1)

//除bei bfi, bfei bffi, 以外的所有卫星板卡
//HAVE gpsbf gpsbe, gbdbf gbdbe, bdbf bdbe, 6种
#define FLAG_SAT_ALL_COMMSTA	FLAG_SAT_COMMSTA_MASK

//HAVE bei bfi 2种
#define FLAG_SAT_BEI_BFI_COMMSTA	\
	(FLAG_SAT_COMMSTA_PH)

//HAVE bfei bffi   2种
#define FLAG_SAT_BFEI_BFFI_COMMSTA	\
	(FLAG_SAT_COMMSTA_PH)

struct satcommonsta {
	struct satcommoninfo satcomminfo;
	int sta_flag;

	u8_t leapstate; //0-无操作, 1-正闰秒, 2-负闰秒  new
	s8_t leapSecond;//闰秒值 new
	struct timeinfo leaptime;//闰秒发生的时间
	//u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];

	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs;
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[PORT_SAT];
};

#else
struct gpsbfinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	u8_t sysMode;
	int delay[5];//0-satellite, 3-igirb1, 4-igirb2
	u8_t elev[8];
	u8_t prio[6];
	u8_t lon[13];
	u8_t lat[12];
};


struct gpsbfsta {
	struct gpsbfinfo gpsbf;
	u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];

	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs;
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[5];
};



struct gpsbeinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	u8_t sysMode;
	int delay[5];
	u8_t elev[8];
	u8_t prio[6];
	u8_t lon[13];
	u8_t lat[12];
};


struct gpsbesta {
	struct gpsbeinfo gpsbe;
	u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];

	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs;
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[5];
};



struct gbdbfinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	u8_t sysMode;
	int delay[5];
	u8_t elev[8];
	u8_t prio[6];
	u8_t lon[13];
	u8_t lat[12];
};


struct gbdbfsta {
	struct gbdbfinfo gbdbf;
	u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];

	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs;
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[5];
};


struct gbdbeinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	u8_t sysMode;
	int delay[5];
	u8_t elev[8];
	u8_t prio[6];
	u8_t lon[13];
	u8_t lat[12];
};



struct gbdbesta {
	struct gbdbeinfo gbdbe;
	u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;//天线状态: '2'正常 '3'短路 '4'开路
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];


	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs; //告警：0x0定位不可用或者无效；1-告警；2-定位有效；6-正在估算位置
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[5];
};



struct bdbfinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	u8_t sysMode;
	int delay[5];
	u8_t elev[8];
	u8_t prio[6];
	u8_t lon[13];
	u8_t lat[12];
};


struct bdbfsta {
	struct bdbfinfo bdbf;
	u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];


	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs;
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[5];
};



struct bdbeinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	u8_t sysMode;
	int delay[5];
	u8_t elev[8];
	u8_t prio[6];
	u8_t lon[13];
	u8_t lat[12];
};


struct bdbesta {
	struct bdbeinfo bdbe;
	u8_t mb_ssm[3];
	u8_t trackMode;
	u8_t workMode;
	u8_t elevThd;
	u8_t fdrStatus;
	u8_t rcvrVer[7];
	u8_t rcvrType[6];
	u8_t utc[20];

	u8_t pst_status;//sta: 0-未定位；1-告警；2-2d模式；3-3d模式；4-位置保持
	u8_t trackngb;//sta:跟踪卫星数
	u8_t gbs;
	u8_t nGps;
	struct starinfo vGps[MAX_GPS_STAR];
	
	u8_t nBd;
	struct starinfo vBd[MAX_BD_STAR];
	
	int ph[5];
};



struct beiinfo {
	int delay[5];
	u8_t prio[6];
};


struct beista {
	struct beiinfo bei;
	int ph[5];
};



struct bfiinfo {
	int delay[5];
	u8_t prio[6];
};


struct bfista {
	struct bfiinfo bfi;
	int ph[5];
};



struct bfeiinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	int delay[5];
	u8_t prio[6];
};



struct bfeista {
	struct bfeiinfo bfei;
	u8_t mb_ssm[3];
	int ph[5];
};



struct bffiinfo {
	u8_t mh_ssm[3];
	u8_t mb_sa[4];
	int delay[5];
	u8_t prio[6];
};



struct bffista {
	struct bffiinfo bffi;
	u8_t mb_ssm[3];
	int ph[5];
};
#endif
//国外模块
struct ptpin_info {

	u8_t ptpDelayType;//delay type: p2p or e2e
	
	u8_t ptpMulticast;//multicast|unicast:'master', 'slave' or 'disable'
	u8_t ptpLayer;//layer 2(eth) or layer 3(udp)
	u8_t ptpStep;//one-step or two-step
	u8_t ptpSync;//frequency of sync  [-8,2] ptp2 u node add ip [ptpPriority] [ptpSync:-7,4] []ptpdelayReq:-7,4]
	u8_t ptpAnnounce;//frequency of announce [-4，5]

	u8_t ptpdelayReq;//frequency of delay_req [-7,7]	//new
	u8_t ptpPdelayReq;//frequency of pdelay_req [-7,7]
	//u8_t ptpSsmEnable;
	u8_t ptpIp[MAX_IP_LEN];//ip
	//u8_t ptpMac[MAX_MAC_LEN];//mac
	u8_t ptpMask[MAX_MASK_LEN];//mask //new
	u8_t ptpGateway[MAX_GATEWAY_LEN];//gateway  //new

	u32_t oldptpMasterIp[MAX_IP_NUM];//Master ip for layer 3 //backup last value
	u8_t oldptpMasterIp_num;
	u8_t oldptpMasterMac[MAX_MAC_LEN];//Master MAC layer 2 //new
	u32_t ptpMasterIp[MAX_IP_NUM];//Master ip for layer 3:ip: HOST order: max 16 nums
	u8_t ptpMasterIp_num;
	u8_t ptpMasterMac[MAX_MAC_LEN];//Master MAC layer 2 //new
	u8_t ptpLevel[4];//the clock which we will set (level1,2) 
	u8_t ptpPriority[4];//the clock of priority /the clock which we will set (level2) 
	
	u8_t ptpRegion[4];//add
};


struct ptpinsta {
	struct ptpin_info ptpinInfo;
	int sta_flag;
};


struct schemainfo {
	/*
	  1 for force
	  0 for free
	*/
	u8_t schema;
	
	/*
	  0, 1~10 which refsource index
	*/
	u8_t src_inx;
	/*INVALID_SOURCE_FLAG/TIME_SOURCE_FLAG/FREQ_SOURCE_FLAG/BOTH_SOURCE_FLAG*/
	u8_t flag;
	//refsource type
	u8_t type;
	//the slot of the input source
	u8_t slot;
	// the port of the input source
	u8_t port;
	//the boardid of the input source
	u8_t bid;
	u8_t refsrc_is_valid;
};

enum time_source_flag{
	INVALID_SOURCE_FLAG = 0,
	TIME_SOURCE_FLAG = 1,
	FREQ_SOURCE_FLAG = 2,
	BOTH_SOURCE_FLAG = 3,
	TIME_SRC_FLAG_ARRAY_LEN,
};
extern char gSrcFlagTbl[TIME_SRC_FLAG_ARRAY_LEN][8];

#define SIGNAL_TYPE_PTP_IN		"PTP" //INPUT PTP
#define SIGNAL_TYPE_RTF_1PPS_TOD	"1PPS+TOD"
#define TIME_SOURCE_LEN		(TIME_SOURCE_MAX_NOSOURCE+1)	//41
#define TIME_SOURCE_VALID_LEN	(TIME_SOURCE_MAX_NOSOURCE)	//40

#define PER_SLOT_SRCTYPE_NUM	(10)

/*time source ID*/
#define GPS_SOURCE_ID(slot)		(PER_SLOT_SRCTYPE_NUM * (slot-1) + 0)
#define S2MH_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 1)
#define S2MB_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 2)
#define IRIGB1_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 3)
#define IRIGB2_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 4)
#define TOD_SOURCE_ID(slot)		(PER_SLOT_SRCTYPE_NUM * (slot-1) + 5)
#define S1PPS_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 6)
#define S10MH1_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 7)
#define S10MH2_SOURCE_ID(slot)	(PER_SLOT_SRCTYPE_NUM * (slot-1) + 8)
#define NTP_SOURCE_ID(slot)		(PER_SLOT_SRCTYPE_NUM * (slot-1) + 9)

//40
#define TIME_SOURCE_MAX_NOSOURCE (PER_SLOT_SRCTYPE_NUM*INPUT_MAX_SLOT)

/*time source TYPE*/
#define GPS_SOURCE_TYPE			1
#define S2MH_SOURCE_TYPE		2
#define S2MB_SOURCE_TYPE		3
#define IRIGB1_SOURCE_TYPE		4
#define IRIGB2_SOURCE_TYPE		5
#define TOD_SOURCE_TYPE			6	//PTP/1PPS+TOD
#define S1PPS_SOURCE_TYPE		7
#define S10MH1_SOURCE_TYPE		8
#define S10MH2_SOURCE_TYPE		9
#define NTP_SOURCE_TYPE			10



#define NO_SOURCE_TYPE			0


/*fpga value  slot: 1-4*/
#define G_GPS_SOURCE_VAL(slot)		(0x1<<(4*(slot-1)))
#define G_S2MH_SOURCE_VAL(slot)		(0x2<<(4*(slot-1)))
#define G_S2MB_SOURCE_VAL(slot)		(0x3<<(4*(slot-1)))
#define G_IRIGB1_SOURCE_VAL(slot)	(0x4<<(4*(slot-1)))
#define G_IRIGB2_SOURCE_VAL(slot)	(0x5<<(4*(slot-1)))
#define G_TOD_SOURCE_VAL(slot)		(0x6<<(4*(slot-1)))
#define G_S1PPS_SOURCE_VAL(slot)	(0x8<<(4*(slot-1)))
#define G_S10MH1_SOURCE_VAL(slot)	(0x9<<(4*(slot-1)))
#define G_S10MH2_SOURCE_VAL(slot)	(0xa<<(4*(slot-1)))
#define G_NTP_SOURCE_VAL(slot)		(0x7<<(4*(slot-1)))



#define NO_SOURCE (0xFFFF)  	//the id of TIME_SOURCE_MAX_NOSOURCE is no_source 

struct timesource_tbl_t {
	int id;	//index
	int g_val;	//fpga val
	u8_t type;	//time source type
	int slot;
	u8_t flag;	//time, freq or both
	char * type_name;
	char * msg;
};

extern struct timesource_tbl_t gTimeSourceTbl[TIME_SOURCE_LEN];

//extern u8_t gTimeSourceTbl[12][40];
		//extern u8_t gTimeSourceTbl[12][40];
#define FIND_FLAG_BY_FPGA_VAL(tsrc_gval, tsrc_flag) \
		FIND_OTHMEMBER_BY_MEMBER(tsrc_gval, tsrc_flag, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), g_val, flag)

#define FIND_TYPE_BY_FPGA_VAL(tsrc_gval, tsrc_type) \
		FIND_OTHMEMBER_BY_MEMBER(tsrc_gval, tsrc_type, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), g_val, type)

#define FIND_SLOT_BY_FPGA_VAL(tsrc_gval, tsrc_slot) \
			FIND_OTHMEMBER_BY_MEMBER(tsrc_gval, tsrc_slot, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), g_val, slot)

#define FIND_TYPE_BY_ID(tsrc_id, tsrc_type) \
		FIND_OTHMEMBER_BY_MEMBER(tsrc_id, tsrc_type, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), id, type)

#define FIND_TYPE_NAME_BY_ID(tsrc_id, tsrc_type_name) \
	FIND_OTHMEMBER_BY_MEMBER(tsrc_id, tsrc_type_name, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), id, type_name)

#define FIND_SLOT_BY_ID(tsrc_id, tsrc_slot) \
	FIND_OTHMEMBER_BY_MEMBER(tsrc_id, tsrc_slot, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), id, slot)

#define FIND_FLAG_BY_ID(tsrc_id, tsrc_flag) \
		FIND_OTHMEMBER_BY_MEMBER(tsrc_id, tsrc_flag, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), id, flag)

//不唯一对应
#define FIND_FLAG_BY_TYPE(tsrc_type, tsrc_flag) \
		FIND_OTHMEMBER_BY_MEMBER(tsrc_type, tsrc_flag, gTimeSourceTbl, ARRAY_SIZE(gTimeSourceTbl), type, flag)

//only support array
#define FIND_OTHMEMBER_BY_MEMBER(find_condition, find_result, table, table_len, src_member, des_member) \
	do{ \
		int __i;	\
		for (__i = 0; __i < table_len; ++__i) { \
			if(find_condition == table[__i].src_member){ \
				find_result = table[__i].des_member;	\
				break; \
			} \
		} \
	} while(0)

//: no found,  balm_index should default -1.
#define FIND_ALMINX_BY_SLOT_PORT_ID(balm_id, balm_slot, balm_port, balm_index) \
		FIND_INDEX_BY_THREE_MEMBER(balm_index, balm_id, balm_slot, balm_port, pAlmInxTbl, max_alminx_num, alm_id, slot, port)

//only support array
#define FIND_OTHMEMBER_BY_STRMEMBER(find_condition, find_cndt_len, find_result, table, table_len, src_member, des_member) \
	do{ \
		int __i;	\
		find_result = -1; \
		for (__i = 0; __i < table_len; ++__i) { \
			if(memcmp(find_condition, table[__i].src_member, (size_t) find_cndt_len) == 0){ \
				find_result = table[__i].des_member;	\
				break; \
			} \
		} \
	} while(0)

//support point
#define FIND_INDEX_BY_THREE_MEMBER(find_result, find_con1, find_con2, find_con3, table, table_len, mbr_1st, mbr_2nd, mbr_3th) \
	do{ \
		int __i;	\
		for (__i = 0; __i < table_len; ++__i) { \
			if(find_con1 == table[__i].mbr_1st){ \
				if(find_con2 == table[__i].mbr_2nd){ \
					if(find_con3 == table[__i].mbr_3th){ \
						find_result = __i;	\
						break; \
					} \
				} \
			} \
		} \
	} while(0)

//support point, for a group init value; if there is a group,  no need break
#define SET_MEMBER_VAL_BY_THREE_MEMBER(set_val, find_con1, find_con2, find_con3, table, table_len, mbr_set, mbr_1st, mbr_2nd, mbr_3th, is_group) \
	do{ \
		int __i;	\
		for (__i = 0; __i < table_len; ++__i) { \
			if(find_con1 == table[__i].mbr_1st \
				&& find_con2 == table[__i].mbr_2nd \
				&& find_con3 == table[__i].mbr_3th){ \
					table[__i].mbr_set = set_val;	\
					if(!is_group) \
						break; \
			} \
		} \
	} while(0)

#define SET_MEMBER_VAL(set_val, table, table_len, mbr_set) \
	do{ \
		int __i;	\
		for (__i = 0; __i < table_len; ++__i) { \
			table[__i].mbr_set = set_val; \
		} \
	} while(0)

//----------

//FOR set_which
#define INPUTINFO_SET_ALL_INFO	0
#define INPUTINFO_SET_INCOMM_INFO	1
#define INPUTINFO_SET_PRIV_INFO	2

struct inputinfo {
	u8_t slot;//slot of equipment
	u8_t boardId;//board identifier
	u8_t nPort;//port of board

	u8_t set_which;	//incomminfo or private(satcomminfo)info
	struct incommoninfo incomminfo;
	union {
		struct satcommoninfo satcomminfo;
		//add ptp/rtf
		struct ptpin_info ptpinInfo;
	};
};


/*240 bytes*/
struct inputsta {
	u8_t boardId;//board identifier
	//ONLY satellite its use,and ledstatus, sate recever alarm, netmanage bd/gps show

	//which pboard: board = ctx->gpsbf( and so on)
	//void * pboard;
	u8_t satellite_valid;

	// satellite system mode
	//MODE_CURSOR_GPS 1	SG
	// MODE_CURSOR_BD 2	SB
	// MODE_CURSOR_MIX_GPS 3	MG
	// MODE_CURSOR_MIX_GPS 4	MB
	u8_t sys_mode;

	//satellite state: 1-bd/gps num>=5; 0-bd/gps num<5
	//bit0:gps bit1:bd
	//00 01 10 11
	u8_t satellite_state;

	//'1'	短路
	//'2'	正常
	//'3'	开路
	//antenna state
	u8_t antenna_state;

	//cur slot port num
	u8_t port_num;
	struct incommonsta incommsta;

	union {
		struct satcommonsta satcommon;
		struct ptpinsta ptpin;//1-port, add new
	};
};


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//message type of config
enum {
	MSG_CFG_DBG = 1,//debug
	MSG_CFG_UPDATE,//update
	MSG_CFG_REBOOT,//reboot
	MSG_CFG_GUARD,//guard
	MSG_CFG_SYS_CONF_SWI,//SYS CONF.INCLUDE:PhasePerf upload enable/disable set
	MSG_CFG_LPS,//leapsecond
	MSG_CFG_LPS_MODE_SET,//leapmode set
	MSG_CFG_OUT,//out
	MSG_CFG_IZONE,//input timezone
	MSG_CFG_OZONE,//output timezone
	MSG_CFG_IN,//in
	MSG_CFG_SCHEMA,//schema
	MSG_CFG_SELECT,//alarm select
	MSG_CFG_MASK,//alarm mask
	MSG_CFG_PORT_STATUS,
	MSG_CFG_OUT_DELAY,//out delay
	MSG_CFG_NTP_BC,//broadcast
	MSG_CFG_NTP_VE,//version enable
	MSG_CFG_NTP_MK_ADD, //add md5 key
	MSG_CFG_NTP_MK_DEL, //delete md5 key
	MSG_CFG_NTP_MK_MOD, //modify md5 key
	MSG_CFG_NTP_ME, //md5 enable
	MSG_CFG_NTP_VP,	//virtual port
	MSG_CFG_OBR,	//baudrate
	MSG_CFG_AMP,	//irigb amplitude ratio
	MSG_CFG_VOL,	//irigb voltage
	MSG_CFG_OBSA,	//out 2MB SA
	MSG_CFG_WG_LOG,
	MSG_CFG_WG_NET,
	MSG_CFG_WG_SERVER_IP,
	MSG_CFG_61850_IP,
	MSG_CFG_INPH_THRESHOLD //鉴相值阈值
};



//config structure
struct config {
	union {
		struct dbginfo dbg_config;
		struct updateinfo update_config;
		struct rebootinfo reboot_config;
		struct sys_conf_t sys_config;
		struct outputinfo out_config;
		struct inzoneinfo in_zone_config;
		struct outzoneinfo out_zone_config;
		struct inputinfo in_config;
		struct schemainfo schema_config;
		struct selectinfo select_config;
		struct maskinfo mask_config;
		struct inph_thresold thresold_config;
		struct leapinfo leap_config;
		struct outdelay out_delay_config;
		struct ntpbc bc_config;
		struct ntpve ve_config;
		struct ntpmk mk_config;
		struct ntpme me_config;
		struct veth vp_config;
		struct out2mbsainfo out_sa;
		struct baudrate out_br;
		struct amplitude irigb_amp;
		struct voltage irigb_vol;
		struct portinfo port_status;
		char buffer[MAX_BYTES];
	};
};






/*###########################################################################
*	new struct type 20180424
############################################################################*/
//板卡卫星的私有属性
struct sat_priv_data_t{
	int sta_flag;
	int info_flag;
};

//每个端口对应的信息
struct port_attr_t{
	//union{
	u8_t insrc_type;//输入盘，选源信号
	//u8_t outsignal_type;//输出盘信号源信号，may use, 暂时不用
	//};
};

#define NM_BOARD_TYPE_NONE		"none"
#define NM_BOARD_TYPE_INPUT		"in"
#define NM_BOARD_TYPE_OUTPUT	"out"
#define NM_BOARD_TYPE_PWR		"pwr"
#define NM_BOARD_TYPE_ALM		"alm"

enum {
	BOARD_TYPE_NONE	= 0,	//NO TYPE
	BOARD_TYPE_INPUT	= 1,	//输入板卡
	BOARD_TYPE_OUTPUT,			//输出板卡
	BOARD_TYPE_PWR,			//电源盘
	BOARD_TYPE_ALM,			//告警盘
};
extern char *gBdTypeTbl[10];

struct alm_attr_t
{
	u32_t alm_id;
	u32_t supported_port; //bits, max support 32 port
};



/*Board Card attributes, it is constant,板卡的固有属性，固定值，不变化*/
struct boardattr_tbl_t
{
	u8_t bid;
	u8_t max_port;
	u8_t board_type;	//input/output/pwr/alm_board(output)
	u8_t index; //in/out board index for the fpga addr
	char *board_name;
	union{
		u32_t supported_signal;	//支持的信号类型,先暂且考虑输出板卡的输出信号，最多支持32个信号类型（输出板卡专用）
		u32_t supported_src; //表示支持的输入信号选源（输入板卡专用）//暂时废弃
	};
	struct alm_attr_t * supported_alm;
	int 	spt_alm_len;	//supported_alm len

	struct port_attr_t *port_attr;//表示每个端口代表的输入选源信号（输入板卡专用）,长度为max_port
	void * pdata;//board private data
};

extern struct boardattr_tbl_t gBoardAttrTbl[];
extern int gBoardAttrTbl_len;

/*此处是动态变化，需定时检测,且在各个进程都需要检测 may use it*/
struct board_tbl_t
{
	u8_t cur_slot;
	u8_t cur_bid;
	struct boardattr_tbl_t *gBoardAttrTbl;
};

//extern struct board_tbl_t gBoardInfoTbl;
#define FIND_BDNAME_GBATBL_BY_BID(bd_id, bd_name) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_name, gBoardAttrTbl, gBoardAttrTbl_len, bid, board_name)

/*#define FIND_SUP_SRC_GBATBL_BY_BID(bd_id, bd_sptsrc) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_sptsrc, gBoardAttrTbl, gBoardAttrTbl_len, bid, supported_src)*/
//ONLY FOR OUTPUT BOARD
#define FIND_SUP_SIGNAL_GBATBL_BY_BID(bd_id, bd_sptsgn) \
				FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_sptsgn, gBoardAttrTbl, gBoardAttrTbl_len, bid, supported_signal)


#define FIND_SPT_ALM_GBATBL_BY_BID(bd_id, bd_sptalm) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_sptalm, gBoardAttrTbl, gBoardAttrTbl_len, bid, supported_alm)
#define FIND_SPT_LEN_ALM_GBATBL_BY_BID(bd_id, bd_sptalm_len) \
				FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_sptalm_len, gBoardAttrTbl, gBoardAttrTbl_len, bid, spt_alm_len)


#define FIND_MAXPORT_GBATBL_BY_BID(bd_id, bd_maxport) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_maxport, gBoardAttrTbl, gBoardAttrTbl_len, bid, max_port)

#define FIND_BDTYPE_GBATBL_BY_BID(bd_id, bd_type) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_type, gBoardAttrTbl, gBoardAttrTbl_len, bid, board_type)

#define FIND_PORTATTR_GBATBL_BY_BID(bd_id, bd_portattr) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_portattr, gBoardAttrTbl, gBoardAttrTbl_len, bid, port_attr)

#define FIND_PRIVDATA_GBATBL_BY_BID(bd_id, bd_pdata) \
		FIND_OTHMEMBER_BY_MEMBER(bd_id, bd_pdata, gBoardAttrTbl, gBoardAttrTbl_len, bid, pdata)

//-------------------------------------process-----------------------------------
//

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//exit signal and notify signal
enum {
	//notify signal
	NOTIFY_SIGNAL	= SIGUSR1,
	
	//exit signal
	EXIT_SIGNAL1	= SIGINT,
	EXIT_SIGNAL2	= SIGTERM,
	EXIT_SIGNAL3	= SIGHUP
};











//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//share memory path
#define	SHM_PATH	"/usr/p350/"

//semaphore path
#define	SEMA_PATH	"/usr/p350/"

//message queue path
#define	MSGQ_PATH	"/usr/p350/"


//share memory proj_id
enum {
	SHM_PROJ_ID_INPUT	= 'a',
	SHM_PROJ_ID_OUTPUT	= 'b',
	SHM_PROJ_ID_NTP		= 'c',
	SHM_PROJ_ID_ALARM	= 'd',
	SHM_PROJ_ID_CLOCK	= 'e',
	SHM_PROJ_ID_MANAGER	= 'f',
	SHM_PROJ_ID_KEYLCD	= 'g',
	SHM_PROJ_ID_WG      = 'h',
	SHM_PROJ_ID_61850   = 'i'
};

//semaphore proj_id, ALL sema id same
enum {
	SEMA_PROJ_ID	= 'A',
	SEMA_PROJ_ID_IP1725	= 'B'
};

//semaphore member
enum {
	SEMA_MEMBER_INPUT	= 0,
	SEMA_MEMBER_OUTPUT,
	SEMA_MEMBER_NTP,
	SEMA_MEMBER_ALARM,
	SEMA_MEMBER_CLOCK,
	SEMA_MEMBER_MANAGER,
	SEMA_MEMBER_KEYLCD,
	SEMA_MEMBER_WG,
	SEMA_MEMBER_61850,
	SEMA_MEMBER_ARRAY_SIZE
};

//message queue proj_id
enum {
	MSGQ_PROJ_ID_INPUT		= '1',
	MSGQ_PROJ_ID_OUTPUT		= '2',
	MSGQ_PROJ_ID_NTP		= '3',
	MSGQ_PROJ_ID_ALARM		= '4',
	MSGQ_PROJ_ID_CLOCK		= '5',
	MSGQ_PROJ_ID_MANAGER	= '6',
	MSGQ_PROJ_ID_KEYLCD		= '7',
	MSGQ_PROJ_ID_WG         = '8',
	MSGQ_PROJ_ID_61850		= '9'
};


//IPC cursor
enum {
	IPC_CURSOR_INPUT = 0,
	IPC_CURSOR_OUTPUT,
	IPC_CURSOR_NTP,
	IPC_CURSOR_ALARM,
	IPC_CURSOR_CLOCK,
	IPC_CURSOR_MANAGER,
	IPC_CURSOR_KEYLCD,
	IPC_CURSOR_WG,
	IPC_CURSOR_61850,
	IPC_CURSOR_ARRAY_SIZE
};



/*share memory offset public(input output alarm)
//(16K)
以下表示共享内容各个表示内容的起始偏移位置，请定义增加结构体内容时，一定检查共享内容size的大小
是否大于结构体的大小，否则会造成丢失数据
SHM_OFFSET_PID	: 0    存放自己进程的pid信息       start_offset:0 size:32
SHM_OFFSET_LPS		= 32,// 32~63  32  存放闰秒信息 start_offset:32 size:32
SHM_OFFSET_IZONE	= 64,// 64~95  32  存放输入时区信息 start_offset:64 size:32
SHM_OFFSET_OZONE	= 96,// 96~127	32  存放输出时区信息
SHM_OFFSET_OBR		= 128,// 128~159 32  存放输出波特率信息
SHM_OFFSET_AMP		= 160,// 160~191 32  存放irigb幅度比信息
SHM_OFFSET_VOL		= 192,// 192~223 32  存放irigb电压比信息
SHM_OFFSET_SCHEMA	= 224,// 224~255  32  存放选源信息
SHM_OFFSET_OBSA 	= 256,// 256-271 16      存放输出2MB的sa信息     size:16
SHM_OFFSET_ODELAY	= 272,// 272~511  240     存放输出板卡的延时补偿 size:240
SHM_OFFSET_ALM		= 512,// 512~4607	存放告警列表的状态,已占用ALM_CURSOR_ARRAY_SIZE*sizeof(char) =651个bytes，总共:4k
SHM_OFFSET_IPH_THRESHOLD = 4608, // 4608~4639 32   存放系统性能超限设置的值 32byte
SHM_OFFSET_PORT 	= 4640,// 4640~4831 192//存放ntp端口状态信息
SHM_OFFSET_FRESH	= 4832,// 4832~4863  32//all reserved
SHM_OFFSET_CLOCK	= 4864,// 4864~4895  32 存放选源的时钟状态信息
SHM_OFFSET_SYS_CONF_SWI = 4896,//4896~5151	256  存放系统设置项[如性能上报开关]信息
SHM_OFFSET_STA		= 5152 // 5152~16383 11k  //存放输入输出板卡所有配置信息及其进程的其他私有数据input: insta ; output:struct outsta; alarm: struct selectsta(8*32),struct masksta(4k)

*/
enum {
	SHM_OFFSET_PID		= 0,// 0~31 32
	SHM_OFFSET_LPS		= 32,// 32~63  32
	SHM_OFFSET_IZONE	= 64,// 64~95  32
	SHM_OFFSET_OZONE	= 96,// 96~127  32
	SHM_OFFSET_OBR		= 128,// 128~159 32
	SHM_OFFSET_AMP		= 160,// 160~191 32
	SHM_OFFSET_VOL		= 192,// 192~223 32
	SHM_OFFSET_SCHEMA	= 224,// 224~255  32
	SHM_OFFSET_OBSA		= 256,// 256-271 16
	SHM_OFFSET_ODELAY	= 272,// 272~511  240
	SHM_OFFSET_ALM		= 512,// 512~4607	4k,support 4096 index, reserve 30 alarm type size
	SHM_OFFSET_IPH_THRESHOLD = 4608, // 4608~4639 32
	SHM_OFFSET_PORT     = 4640,// 4640~4831	192//resoved 100 byte
	SHM_OFFSET_FRESH    = 4832,// 4832~4863  32//all reserved
	SHM_OFFSET_CLOCK    = 4864,// 4864~4895  32
	SHM_OFFSET_SYS_CONF_SWI = 4896,//4896~5151	256
	SHM_OFFSET_STA		= 5152 // 5152~16383 11k  //input: insta ; output:struct outsta; alarm: struct selectsta(8*32),struct masksta(4k)
};

/*
alarm_process:
SHM_OFFSET_STA
struct selectsta*PORT_ALM(8)
struct masksta*ALM_CURSOR_ARRAY_SIZE(3x)

*/
//alarm priv
enum {
	//by xpc
	SHM_OFFSET_ALMPRO_SELECTSTA = SHM_OFFSET_STA,
	SHM_OFFSET_ALMPRO_MASKSTA = (SHM_OFFSET_ALMPRO_SELECTSTA + sizeof(struct selectsta)*PORT_ALM),
	SHM_OFFSET_CURALM_INFO = (SHM_OFFSET_ALMPRO_MASKSTA + ALM_CURSOR_ARRAY_SIZE*sizeof(struct masksta)),//5835
	SHM_OFFSET_UPLOADALM_INFO = (SHM_OFFSET_CURALM_INFO + CUR_ALM_MAX_ARRAY_SIZE*sizeof(struct alminfo_t))//7035
};

//IPC info
struct ipcinfo {	
	int ipc_sem_id;//semaphore identifier //all process ipc_sem_id is same
	int ipc_msgq_id;//message queue identifier
	
	int ipc_shm_id;//share memory identifier
	void *ipc_base;//share memory base address
};











//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//cursor of daemon
enum {
	DAEMON_CURSOR_NONE	= 0,
	DAEMON_CURSOR_INPUT	= 1,
	DAEMON_CURSOR_OUTPUT,
	DAEMON_CURSOR_NTP,
	DAEMON_CURSOR_ALARM,
	DAEMON_CURSOR_CLOCK,
	DAEMON_CURSOR_MANAGER,
	DAEMON_CURSOR_KEYLCD,
	DAEMON_CURSOR_WG,
	DAEMON_CURSOR_IEC61850,
	DAEMON_CURSOR_TELNET,
	DAEMON_CURSOR_HTTP,
	DAEMON_CURSOR_ARRAY_SIZE
};


//priority of daemon
#define	DAEMON_PRIO_INPUT		0
#define	DAEMON_PRIO_OUTPUT		0
#define	DAEMON_PRIO_NTP			0
#define	DAEMON_PRIO_ALARM		0
#define	DAEMON_PRIO_CLOCK		0
#define	DAEMON_PRIO_MANAGER		0
#define	DAEMON_PRIO_KEYLCD		0
#define	DAEMON_PRIO_IEC61850	0
#define	DAEMON_PRIO_TELNET		0
#define	DAEMON_PRIO_HTTP		0
#define	DAEMON_PRIO_WG		    0


//进程信息
struct pidinfo {
	u16_t	p_cursor;//PID cursor
	pid_t	p_id;//PID
};


extern u8_t gDaemonTbl[DAEMON_CURSOR_ARRAY_SIZE][16];











//------------------------------------------------------------------------------------------------------------

#include "alloc_snmp.h"
int initializeAlarmIndexTable(void);
void cleanAlarmIndexTable(void);
int initializeEventIndexTable(void);
void cleanEventIndexTable(void);



#endif//__ALLOC__


