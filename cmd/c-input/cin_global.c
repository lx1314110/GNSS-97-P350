#include "cin_global.h"





struct inCtx gInCtx;





char *gHelpTbl[] = {
	"p350in --help",
	"p350in gb",
	"p350in SLOT portinfo",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"p350in SLOT portstype",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"p350in SLOT [PORT] prio [PRIO]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       PORT is from 1 to "get_marco_value(INPUT_SLOT_MAX_PORT),
	"       PRIO is - or from 0 to 9",
	"p350in SLOT PORT delay [DELAY]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       PORT is from 1 to "get_marco_value(INPUT_SLOT_MAX_PORT),
	"       DELAY is an integer",
	"p350in schema [SCHEMA] [PORT]",
	"       SCHEMA is force or free",
	"       PORT is from 0 to "get_marco_value((TIME_SOURCE_VALID_LEN-1)),
	"p350in schema list",
	"       show all supported schema list",
	"p350in schema vlist",
	"       show cur supported valid-schema list",
	"p350in SLOT [PORT] ssm [SSM]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       PORT is from 1 to "get_marco_value(INPUT_SLOT_MAX_PORT),
	"       Only support freq src port setting",
	"       SSM is 02 04 08 0B 0F 00",
	"       if 2mb signal is loss, 2MB SSM is --",
	"p350in SLOT [PORT] sa [SA]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       PORT is from 1 to "get_marco_value(INPUT_SLOT_MAX_PORT),
	"       SA is SA4 SA5 SA6 SA7 SA8",
	"p350in SLOT signal",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"p350in SLOT mode [MODE]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       MODE is SG SB MG MB",
	"p350in SLOT elev [ELEV]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       ELEV is a floating number",
	"p350in SLOT lola [LOT LOV LAT LAV]",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       LOT is E or W",
	"       LOV is like dddmm.mmmmm",
	"       LAT is N or S",
	"       LAV is like ddmm.mmmmm",
	"p350in SLOT PORT",
	"       SLOT is 1 to "get_marco_value(INPUT_MAX_SLOT),
	"       PORT is from 1 to "get_marco_value(INPUT_SLOT_MAX_PORT),
	"p350in ptp SLOT",
	"p350in [ptp SLOT [[pdt PDT] ",
	"                 [pmut PMUT] ",
	"                 [plt PLT] ",
	"                 [pst PST] ",
	"                 [psf PSF] ",
	"                 [paf PAF] ",
	"                 [pdrf PDRF] ",
	"                 [ppdrf PPDRF] ",
	"                 [pip PIP] ",
	"                 [pmask PMASK] ",
	"                 [pgw PGW] ",
	"                 [pmip PMIP] ",
	"                 [pmmac PMMAC] ",
	"                 [level VALUE1]",
	"                 [priority VALUE2]",
	"                 [preg PREG]]]",
	"        PDT is p2p/e2e",
	"        PMUT is multi/uni",
	"        PLT is layer2/layer3",
	"        PST is 1step/2step",
	"        PSF PAF PDRF PPDRF is 256 128 64 32 16 8 4 2 1",
	"                              1/2 1/4 1/8 1/16 1/32 1/64 1/128 1/256",
	"        PIP PMASK PGW is like XXX.XXX.XXX.XXX",
	"        PMIP is like XXX.XXX.XXX.XXX",
	"        PMMAC is like XX:XX:XX:XX:XX:XX",
	"        level priority VALUE is 0 to 255",
	"        PREG is 0 to 255",
	NULL
};





