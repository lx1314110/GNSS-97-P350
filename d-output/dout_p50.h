#ifndef	__DOUT_P50__
#define	__DOUT_P50__






#include <stdio.h>
#include <arpa/inet.h>
#include "lib_net.h"










//enable ntp
#define	P50_CMD_NTP_ON		"ptp2 nt start\n"



//disable ntp
#define	P50_CMD_NTP_OFF		"ptp2 nt stop\n"



//enable ptp
#define	P50_CMD_PTP_ON		"ptp2 start 1\n"



//disable ptp
#define	P50_CMD_PTP_OFF		"ptp2 stop\n"



//receive china mobile tod
#define	P50_CMD_IN_CM_TOD		"ptp2 g f cm\n"



//9600
#define	P50_CMD_IN_B9600		"ptp2 g i default 9600\n"


//output china mobile tod
#define P50_CMD_OUT_B9600		"ptp2 pt interface default 9600\n"
#define P50_CMD_OUT_CM_TOD		"ptp2 pt f cm\n"
#define	P50_CMD_OUT_1PPS_TOD	"ptp2 pt start\n"


//layer
#define	P50_CMD_LAYER2		"ptp2 p p eth\n"
#define	P50_CMD_LAYER3		"ptp2 p p udp\n"



//delay type
#define	P50_CMD_E2E			"ptp2 p m e2e\n"
#define	P50_CMD_P2P			"ptp2 p m p2p\n"

//model
#define	P50_CMD_MASTER			"ptp2 start 1\n"
#define	P50_CMD_SLAVE			"ptp2 start 0\n"

//PTP ssm enable
#define	P50_CMD_SSMENABLE_ZERO			"ptp2 s en off\n"
#define	P50_CMD_SSMENABLE_ONE			"ptp2 s en on\n"


//step
#define	P50_CMD_1STEP		"ptp2 c t off\n"
#define	P50_CMD_2STEP		"ptp2 c t on\n"


#define P50_CMD_UNICAST		    "ptp2 p u uni\n"
#define P50_CMD_MULTICAST		"ptp2 p u mul\n"

/*announce [-8,8]
  -8	256 announce per second
  -7	128 announce per second
  -6	64 announce per second
  -5	32 announce per second
  -4	16 announce per second
  -3	8 announce per second
  -2	4 announce per second
  -1	2 announce per second
   0	1 announce per second
   1	1 announce per 2 second
   2	1 announce per 4 second
   3	1 announce per 8 second
   4	1 announce per 16 second
   5	1 announce per 32 second
   6	1 announce per 64 second
   7	1 announce per 128 second
   8	1 announce per 256 second
*/
#define	P50_FILL_ANNOUNCE(STR, FREQ)	sprintf(STR, "ptp2 p ai %d\n", FREQ)




//sync [-8,8]
#define	P50_FILL_SYNC(STR, FREQ)	sprintf(STR, "ptp2 p si %d\n", FREQ)





//pdelay req [-8,8]
#define	P50_FILL_PDELAY_REQ(STR, FREQ)	sprintf(STR, "ptp2 p di %d\n", FREQ)

//ptp region(0,127)国内，国外ptp2 c d
#define P50_FILL_REGION(STR,REGION)		sprintf(STR, "ptp2 p d %d\n", REGION)

//set ptp level
#define	P50_FILL_LEVEL(STR, LEVEL,PRIORITY)	sprintf(STR, "ptp2 c p %d %d\n", LEVEL,PRIORITY)

//set ptp Clock properties
//#define P50_FILL_PROPERTIES(STR,CLASS,ACCURACY,VARIANCE) sprintf(STR, "ptp2 c q %s %s %s\n", CLASS,ACCURACY,VARIANCE)

//slave ip 

#define	P50_FILL_SLAVE_IP(STR, IP) do {\
	sprintf(STR, "ipconfig -u %x\n", ntohl(inet_addr((char *)IP)));\
	}while(0);


// ip 

#define	P50_FILL_IP(STR, IP) do {\
	sprintf(STR, "ipconfig -a %x\n", ntohl(inet_addr((char *)IP)));\
	}while(0);
	


/*国外ptp start*/
//单播
#define P50_2_CMD_UNICAST		    P50_2_CMD_UNICAST_MASTER
#define P50_2_CMD_UNICAST_MASTER		"ptp2 p u master\n"
#define P50_2_CMD_UNICAST_SLAVE	    "ptp2 p u slave\n"
//多播
#define P50_2_CMD_MULTICAST		"ptp2 p u disable\n"

//ptp region(0,255)
#define P50_2_FILL_REGION(STR,REGION)		sprintf(STR, "ptp2 c d %d\n", REGION)

// ip 

#define	P50_2_FILL_IP(STR, IP)	do{\
	sprintf(STR, "ipconfig -a %s\n", (char *)IP);\
	}while(0);
	
// mask

#define	P50_2_FILL_MASK(STR, MASK) do {\
	sprintf(STR, "ipconfig -m %s\n", (char *)MASK);\
	}while(0);
	
// gateway 

#define	P50_2_FILL_GATEWAY(STR, GW) do {\
	sprintf(STR, "ipconfig -g %s\n", (char *)GW);\
	}while(0);
	


//slave ip 
#if 0
#define	P50_2_FILL_SLAVE_IP(STR, IP)	do{\
	sprintf(STR, "ptp2 u filter slave add %s\n", (char *)IP);\
	}while(0);
#else
/*最后三个参数是Annouce、Sync、Delay报文速率,
因为M64的默认边界值是1,0,0,没有调通。所以改成M51相同的值-4,-7,-7*/
#define P50_2_FILL_SLAVE_IP(STR, IP)	P50_2_FILL_SLAVE_IP_PSD(STR, IP, -4, -7, -7)
#endif
/*
#define	P50_2_FILL_SLAVE_MAC(STR, MAC)	do{\
	char __p50_mac__[MAX_MAC_LEN]; \
	memset(__p50_mac__, 0x0, MAX_MAC_LEN);\
	ptp_set_master_mac(__p50_mac__,MAC); \
	sprintf(STR, "ptp2 u filter slave add %s\n", (char *) __p50_mac__);\
	}while(0);
*/

//slave ip 
#define	P50_2_DEL_SLAVE_IP(STR, IP)	do{\
	sprintf(STR, "ptp2 u filter slave del %s\n", (char *)IP);\
	}while(0);
/*
#define	P50_2_DEL_SLAVE_MAC(STR, MAC)	do{\
	char __p50_mac__[MAX_MAC_LEN]; \
	memset(__p50_mac__, 0x0, MAX_MAC_LEN);\
	ptp_set_master_mac(__p50_mac__,MAC); \
	sprintf(STR, "ptp2 u filter slave del %s\n", (char *) __p50_mac__);\
	}while(0);
*/
//master ip and prio,sync,delay
#define	P50_2_FILL_SLAVE_IP_PSD(STR, IP, ANNOUCE, SYNC, DELAY)	do{\
	sprintf(STR, "ptp2 u filter slave add %s %d %d %d\n", (char *)IP, ANNOUCE, SYNC, DELAY);\
	}while(0);
/*
#define	P50_2_FILL_SLAVE_MAC_PSD(STR, MAC, ANNOUCE, SYNC, DELAY)	do{\
	char __p50_mac__[MAX_MAC_LEN]; \
	memset(__p50_mac__, 0x0, MAX_MAC_LEN);\
	ptp_set_master_mac(__p50_mac__,MAC); \
	sprintf(STR, "ptp2 u filter slave add %s %d %d %d\n", (char *) __p50_mac__, PRIO,ANNOUCE,DELAY);\
	}while(0);
	*/

/*国外ptp end*/

int p50_write(int fpga_fd, int slot, u8_t *buf, int len);

#if 0
void ptp_set_ssm( int fpga_fd,u8_t ssm_level);


void ptp_set_clkproperty(int fpga_fd,u8_t clkclass,u8_t accuracy,u16_t range);

void ptp_set_freqtrace(int fpga_fd,u8_t value);

void ptp_set_timetrace(int fpga_fd,u8_t value);

void ptp_set_timesource(int fpga_fd,u8_t value);
#endif







int p50_startup_ini(int fpga_fd, int slot, u8_t *ini, int len);




void ptp_set_mac(char *buf,u8_t value[]);







#endif//__DOUT_P50__


