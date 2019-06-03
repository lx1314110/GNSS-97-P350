#include "cout_global.h"




struct outCtx gOutCtx;












char *gHelpTbl[] = {
	"p350out --help",
	"p350out",
	"p350out SLOT portinfo",
	"       SLOT is 1 to "get_marco_value(OUTPUT_MAX_SLOT),
	"       show current out-signal",
	"p350out SLOT signal",
	"       SLOT is 1 to "get_marco_value(OUTPUT_MAX_SLOT),
	"       show current out-signal up/down/unknown status for ntp",
	"p350out delay [[TYPE] [DELAY]]",
	"        TYPE is tod/irigb/ppx",
	"        DELAY is an integer",
	"p350out [SLOT] [delay DELAY] //reserved",
	"p350out [SLOT] [signal SIGNAL] ",
	"p350out SLOT sgnlist",
	"       SLOT is 1 to "get_marco_value(OUTPUT_MAX_SLOT),
	"       show current supported out-signal list table",
	"       SIGNAL is none/ntp/ptp/2mh/2mb/todt/todf/pps/ppm/pph/irigb-dc/irigb-ac/ext",
	"p350out [ptp [SLOT] [[pdt PDT] ",
	"                 [pmut PMUT] ",
	"                 [plt PLT] ",
	"                 [pst PST] ",
	"                 [psf PSF] ",
	"                 [paf PAF] ",
	"                 [ppdrf PPDRF] ",
	"                 [pssm SSM_ENABLE] ",
	"                 [pip PIP] ",
	"                 [pmac PMAC] ",
	"                 [psip PSIP] ",
	"                 [level VALUE1]",
	"                 [priority VALUE2]",
	"                 [preg PREG]",
	"                 [pmask PMASK]",
	"                 [pgw PGW]",
	"                 [ptyp PTYPE]]]",
	"        PDT is p2p/e2e",
	"        PMUT is multi/uni",
	"        PLT is layer2/layer3",
	"        PST is 1step/2step",
	"        PSF PAF PDRF PPDRF is 256 128 64 32 16 8 4 2 1",
	"                              1/2 1/4 1/8 1/16 1/32 1/64 1/128 1/256",
	"        SSM_ENABLE is 0 or 1 (only support chinese model)",
	"        PIP PMASK PGW is like XXX.XXX.XXX.XXX (PMASK/PGW only support foreign model)",
	"        PMAC is like XX:XX:XX:XX:XX:XX (only support chinese model)",
	"        PSIP is like XXX.XXX.XXX.XXX",
	"        level priority VALUE is 0 to 255",
	"        PREG is 0 to 255",
	"        PTYPE is 0-Foreign model, 1-Chinese model",
	"p350out SLOT",
	"        query ptp:",
	"        delay(no use)|ptp/ntp(no use)|pdt|pmut|plt|pst|psf|paf|ppdrf|pssm|pip|pmac|psip|prio1|prio2|preg|pmask|pgw|ptyp",
	//"        PMODE is master/slave",
	"p350out obsa [OBSA]",
	"        OBSA is SA4 SA5 SA6 SA7 SA8", 
	"p350out obssm",
	"        query out 2mb ssm,value:02 04 08 0B 0F 00",
	"p350out iar [GROUP_NUM] [IAR[|IAR2|IAR3|IAR4]]",
	"p350out iv	[GROUP_NUM] [IV[|IV2|IV3|IV4]]",
	"p350out obr [GROUP_NUM] [BR[:BR2:BR3:BR4]]",
	"p350out [IOZ] [GROUP_NUM] [ZONE[:ZONE2:ZONE3:ZONE4]]",
	"        IAR is 3:1/4:1/5:1/6:1",
	"        IV is 3V/4V/5V/6V/7V/8V/9V/10V/11V/12V",
	"        BR is 1200/2400/4800/9600/19200",
	"        IOZ is izone/ozone",
	"        GROUP_NUM is 1 to 4, if no group_num, set all",
	"        SLOT is from 1 to 16",
	"        ZONE(1-4) is zero/e1/e2/e3/e4/e5/e6/e7/e8/e9/e10/e11/ew12/w11/w10/w9/w8/w7/w6/w5/w4/w3/w2/w1",
	NULL
	};







