#ifndef	__DIN_P50__
#define	__DIN_P50__






#include <stdio.h>
#include <arpa/inet.h>
#include "lib_net.h"




/*
ptpin国外模块
 ptp2 help
A:/root> *** PTP2 HMI Help ***

              Format:
command
  subcommand1 <parameters>: description
  subcommand2 <parameters>: description
  ...

              Commands:
(c)clock
  (i)info: Print clock description.
  (u)user <string>: Change user description.
  (m)mode <oc|bc>: Change clock type mode.
  (t)twostep <on|off>: Change two-step mode.
  (q)quality <class> <accuracy> <variance>: Change clock quality.
  (p)priority 1|2 <value>: Change priority 1 or 2.
  (d)domain <number>: Change domain number.
  (s)slaveonly <on|off>: Change slave-only mode.
(p)port <port_number>
  (s)state: Print port state.
  (m)mechanism <e2e|p2p>: Change delay mechanism.
  (p)protocol <udp|eth|udp6>: Change network protocol.
  (c)compatibility <on|off|auto [timeout]>: Change V1 HW compatibility mode.
  (a)asymmetry <value>: Change delay asymmetry.
  (si)sync <value>: Change log sync interval.
  (di)delay <value>: Change min log delay request interval.
  (pi)pdelay <value>: Change min log pdelay request interval.
  (ai)announce <value>: Change log announce interval.
  (rt)receipt <value>: Change announce receipt timeout.
  (u)unicast <master|slave|disable>: Enable/disable unicast operation.
(u)unicast <port_number>
  (s)status: Print port unicast information.
  (n)negotiation <on|off>: Enable/disable unicast negotiation.
  node [add|del] <address> [clock_id] <priority> <si|x> <di|x>: Change unicast master discovery table.
  node [add|del] <address> <ai|x> <si|x> <di|x>: Change static unicast slave table.
  (q)query <interval>: Change unicast discovery query interval.
  (d)duration <value>: Change unicast transmission duration.
  (t)timeout <value>: Change cancel acknowledge receipt timeout.
  (l)limit <value>: Change total unicast messages rate limit.
  (f)filter (s)slave [add|del] <address|*> [clock_id] [x | [ai] [si] [di|x]]: Change acceptable unicast slave table.
  (f)filter (m)master [add|del] <address|*> [clock_id] [x | [priority] [si] [di|x]]: Change acceptable unicast master table.
(d)dataset
  (d)default: Print default dataset.
  (c)current: Print current dataset.
  (m)parent|master: Print parent dataset.
  (t)time: Print time properties dataset.
  (p)port: Print port dataset.
  (f)foreign: Print foreign master dataset.
  (u)unicast: Print unicast master or slave node dataset.
(t)time
  (i)info: Print time and synchronization information.
  (s)sync: Print synchronization status.
  arb <time>: Print or set time in +/-SSSSSSSSS.NNNNNNNNN format.
  ptp <time>: Print or set time in TAI YYYY-MM-DD HH:MM:SS.NNNNNNNNN format.
  utc <time>: Print or set time in UTC YYYY-MM-DD HH:MM:SS.NNNNNNNNN format.
  ntp <server>: Print or set time from NTP server specified by name or IP address.
  (t)timescale <ptp|arb>: Change timescale.
  (l)leap 59|61 <date>: Print UTC leap flags or schedule new leap second.
  (l)leap <save|load>: Print UTC leap table or save/load it into/from file.
  (o)offset <value>: Change UTC offset.
  (u)update <on|off>: Turn on/off automatic update of RTC time from PTP time.
  (h)holdover <value>: Change period of holdover state.
(g)gps
  (s)status: Print GPS interface information.
  (p)ppsin <enable|disable>: Enable/disable PPS input reception.
  (t)todin <enable|disable>: Enable/disable TOD input reception.
  (d)delay <value>: Change PPS-in cable delay compensation.
  (f)format <nmea|ascii|cm>: Change TOD-in message format.
  (v)void <ignore|syntonize|holdover>: Changes NMEA RMC void flag handling.
  (i)interface <default|com[n]> <baudrate>: Change TOD-in interface.
  (c)class <active> <holdover> <free>: Change clock class mapping.
  (m)message <string>: Send message to GPS receiver via TOD interface.
(pt)pulsetime
  (s)status: Print PPS-TOD status and configuration.
  (p)pulse <period> <width>: Change PPS-out pulse parameters.
  (m)mode <1|2|3>: Change PPS-TOD output mode (sync, holdover, always)
  (e)error <value>: Change error tolerance.
  (a)auto <on|off>: Turn on/off automatic restart.
  (t)todout <enable|disable>: Enable/disable TOD output.
  (i)interface <default|com[n]> <baudrate>: Change TOD-out interface.
  (d)delay <value>: Change TOD-out delay after PPS-out rising edge.
  (f)format <nmea [rmc|zda|both]|ascii [utc|local]|cm|custom>: Change TOD-out message format.
  (v)void <never|free|holdover>: Change NMEA RMC Active/Void flag behavior.
  (z)zone <none|tz|dst>: Change NMEA ZDA timezone/daylight field.
  start <time> <count>: Start/schedule PPS-TOD output.
  stop: Stop/cancel PPS-TOD output.
(nt)nettime
  (s)status: Print SNTP server status and configuration.
  (m)mode <1..4>: Change server mode (unicast, manycast, broadcast, mixed).
  (pi)poll <0..64>: Change poll/broadcast interval.
  start: Start SNTP server.
  stop: Stop SNTP server.
(e)engine
  (i)init default|nvm: Reinitialize clock.
  (p)port <port_number> <enable|disable>: Change port operational status.
  (e)esmc <auto|0..15|off>: Enable ESMC message transmission.
  (n)nvm save|reset: Change configuration in non-volatile storage.
  (d)debug <on|off>: Turn on/off debug output.
  (v)verbose <level>: Change verbose level for debug output.
  (m)monitor <on com[n]|off>: Enable output for PTP monitor program.
  (f)freqout <5|10|20|25>: Change frequency output (5, 10, 20, 25 MHz).
  (a)asymmetry <value>: Change PHY asymmetry compensation.

              Usage:
command:                  Prints list of subcommands.
command subcommand:       Prints current value of associated parameter.
command subcommand value: Sets associated parameter to new value.

              Examples:
'clock' or 'c'. Prints list of clock subcommands.
'clock priority 1' or 'c p 1'. Prints value of priority1.
'clock priority 1 130' or 'c p 1 130'. Sets value of priority1 to 130.
'port sync' or 'p si'. Prints value of log sync interval.
'port sync -2' or 'p si -2'. Sets value of log sync interval to -2 (4 sync per sec).
'dataset parent' or 'd m'. Prints content of parent dataset.
'dataset port' or 'd p'. Prints content of port dataset.
'engine verbose' or 'e v'. Prints list of debug output levels.
'engine monitor on 3' or 'e m on 3'. Enable clock monitor program on COM3.

*/





//enable ntp
#define	P50_CMD_NTP_ON		"ptp2 nt start\n"



//disable ntp
#define	P50_CMD_NTP_OFF		"ptp2 nt stop\n"


//enable ptp //DEFAULT SLAVE IN
#define	P50_CMD_PTP_ON		P50_CMD_SLAVE

//disable ptp
#define	P50_CMD_PTP_OFF		"ptp2 stop\n"



//receive china mobile tod
#define	P50_CMD_IN_CM_TOD		"ptp2 g f cm\n"



//9600
#define	P50_CMD_IN_B9600		"ptp2 g i default 9600\n"
//vco
#define P50_CMD_VCO             "ptp2 config -v 1000\n"
//clock mode
#define P50_CMD_CLOCK_SLAVE     "ptp2 c s on\n"

//ptp engine
#define	P50_CMD_STOP			"ptp2 stop\n"
#define	P50_CMD_MASTER			"ptp2 start 1\n"
#define	P50_CMD_SLAVE			"ptp2 start 0\n"


//output china mobile tod
#define P50_CMD_OUT_B9600		"ptp2 pt interface default 9600\n"
#define P50_CMD_OUT_CM_TOD		"ptp2 pt f cm\n"
#define P50_CMD_OUT_PT_M1       "ptp2 pt m 1\n"
#define	P50_CMD_OUT_1PPS_TOD	"ptp2 pt start\n"

//domain
#define P50_CMD_DOMAIN_24       "ptp2 c d 24\n"

//unicast discovery interval and durable interval
#define P50_CMD_DURATION_INTER  "ptp2 u d 60\n"
#define P50_CMD_DISCOVERY_INTER "ptp2 u q 10\n"

//layer
#define	P50_CMD_LAYER2		"ptp2 p p eth\n"
#define	P50_CMD_LAYER3		"ptp2 p p udp\n"



//delay type
#define	P50_CMD_E2E			"ptp2 p m e2e\n"
#define	P50_CMD_P2P			"ptp2 p m p2p\n"



//PTP ssm enable 
//#define	P50_CMD_SSMENABLE_ZERO			"ptp2 s en off\r\n"
//#define	P50_CMD_SSMENABLE_ONE			"ptp2 s en on\r\n"


//step
#define	P50_CMD_1STEP		"ptp2 c t off\n"
#define	P50_CMD_2STEP		"ptp2 c t on\n"

//单播
#define P50_CMD_UNICAST		    P50_CMD_UNICAST_SLAVE

#define P50_CMD_UNICAST_MASTER		"ptp2 p u master\n"
#define P50_CMD_UNICAST_SLAVE	    "ptp2 p u slave\n"

//多播
#define P50_CMD_MULTICAST		"ptp2 p u disable\n"

/*

国外模块[-4，5]
即16 - 1/32
announce [-8,8]
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




//sync [-8,2]
#define	P50_FILL_SYNC(STR, FREQ)	sprintf(STR, "ptp2 p si %d\n", FREQ)



//delay req [-7,7] by xpc
#define	P50_FILL_DELAY_REQ(STR, FREQ)	sprintf(STR, "ptp2 p di %d\n", FREQ)

//pdelay req [-7,7] by xpc
#define	P50_FILL_PDELAY_REQ(STR, FREQ)	sprintf(STR, "ptp2 p pi %d\n", FREQ)

//ptp region(0,255)
#define P50_FILL_REGION(STR,REGION)		sprintf(STR, "ptp2 c d %d\n", REGION)

//set ptp level, level must be 1 or 2, priority 0-255
#define	P50_FILL_LEVEL(STR, LEVEL,PRIORITY)	sprintf(STR, "ptp2 c p %d %d\n", LEVEL,PRIORITY)

//set ptp Clock properties
//#define P50_FILL_PROPERTIES(STR,CLASS,ACCURACY,VARIANCE) sprintf(STR, "ptp2 c q %s %s %s\n", CLASS,ACCURACY,VARIANCE)

//master ip and prio,sync,delay
#define	P50_FILL_MASTER_IP_PSD(STR, IP, PRIO, SYNC, DELAY)	do{\
	sprintf(STR, "ptp2 u node add %s %s %d %d\n", (char *)IP, PRIO, SYNC, DELAY);\
	}while(0);
#define	P50_FILL_MASTER_MAC_PSD(STR, MAC, PRIO, SYNC, DELAY)	do{\
	char __p50_mac__[MAX_MAC_LEN]; \
	memset(__p50_mac__, 0x0, MAX_MAC_LEN);\
	ptp_set_master_mac(__p50_mac__,MAC); \
	sprintf(STR, "ptp2 u node add %s %s %d %d\n", (char *) __p50_mac__, PRIO,SYNC,DELAY);\
	}while(0);

//master ip 
#define	P50_FILL_MASTER_IP(STR, IP)	do{\
	sprintf(STR, "ptp2 u node add %s\n", (char *)IP);\
	}while(0);
#define	P50_FILL_MASTER_MAC(STR, MAC)	do{\
	char __p50_mac__[MAX_MAC_LEN]; \
	memset(__p50_mac__, 0x0, MAX_MAC_LEN);\
	ptp_set_master_mac(__p50_mac__,MAC); \
	sprintf(STR, "ptp2 u node add %s\n", (char *) __p50_mac__);\
	}while(0);


//master ip 
#define	P50_DEL_MASTER_IP(STR, IP)	do{\
	sprintf(STR, "ptp2 u node del %s\n", (char *)IP);\
	}while(0);
#define	P50_DEL_MASTER_MAC(STR, MAC)	do{\
	char __p50_mac__[MAX_MAC_LEN]; \
	memset(__p50_mac__, 0x0, MAX_MAC_LEN);\
	ptp_set_master_mac(__p50_mac__,MAC); \
	sprintf(STR, "ptp2 u node del %s\n", (char *) __p50_mac__);\
	}while(0);


// ip 

#define	P50_FILL_IP(STR, IP)	do{\
	sprintf(STR, "ipconfig -a %s\n", (char *)IP);\
	}while(0);
	
// mask

#define	P50_FILL_MASK(STR, MASK) do {\
	sprintf(STR, "ipconfig -m %s\n", (char *)MASK);\
	}while(0);
	
// gateway 

#define	P50_FILL_GATEWAY(STR, GW) do {\
	sprintf(STR, "ipconfig -g %s\n", (char *)GW);\
	}while(0);
	



int p50_write(int fpga_fd, int slot, u8_t *buf, int len);

#if 0
void ptp_set_ssm( int fpga_fd,u8_t ssm_level);


void ptp_set_clkproperty(int fpga_fd,u8_t clkclass,u8_t accuracy,u16_t range);

void ptp_set_freqtrace(int fpga_fd,u8_t value);

void ptp_set_timetrace(int fpga_fd,u8_t value);

void ptp_set_timesource(int fpga_fd,u8_t value);
#endif







int p50_startup_ini(int fpga_fd, int slot, u8_t *ini, int len);




void ptp_set_master_mac(char *,u8_t *);







#endif//__DIN_P50__


