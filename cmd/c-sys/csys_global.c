#include "csys_global.h"







char *gHelpTbl[] = {
	"p350sys --help",
	"p350sys ver",
	"p350sys reboot",
	"p350sys guard on/off",
	"p350sys update",
	"p350sys clock",
	"p350sys btype",
	"p350sys slotinfo",
	"p350sys dtype",
	"p350sys user -q",
	"p350sys user -a USERNAME PASSWORD [PERMISSION]",
	"p350sys user -d USERNAME",
	"p350sys user -m ORIGINAL-USERNAME ORIGINAL-PASSWORD LATEST-PASSWORD",
	"p350sys user -s USERNAME NEW-PERMISSION",
	"p350sys user -c USERNAME PASSWORD",
	"        USERNAME is consist of letter/number/underscore",
	"        Length of USERNAME and PASSWORD is from 5 to 18",
	"        Value of PERMISSION is from 1 to 3, default:3",
	"              1:MAXIMUM_PERMISSION, 2:MEDIUM, 3:MINIMUM",
	"p350sys time [DATE TIME]",
	"        DATE is like XXXX-XX-XX",
	"        TIME is like XX:XX:XX",
	"p350sys leap [LEAP]",
	"        LEAP is from 0 to 99",
	"p350sys net [ip IP] [mac MAC] [mask MASK] [gateway GATEWAY]",
	"        IP is like XXX.XXX.XXX.XXX",
	"        MAC is like XX:XX:XX:XX:XX:XX",
	"        MASK is like XXX.XXX.XXX.XXX",
	"        GATEWAY is like XXX.XXX.XXX.XXX",
	"p350sys ntp m -q",
	"p350sys ntp m -a MD5-ID MD5-KEY",
	"p350sys ntp m -d MD5-ID",
	"p350sys ntp m -m MD5-ID MD5-KEY",
	"p350sys ntp m -e [MD5-EN]",
	"        MD5-ID is md5 identifier",
	"        MD5-KEY is md5 key",
	"        MD5-EN is 0(disable) or 1(enable)",
	"p350sys ntp b [B-EN B-INTERVAL]",
	"        B-EN is 0(disable) or 1(enable)",
	"        B-INTERVAL is from 64 to 1024",
	"p350sys ntp v [XXX]",
	"        XXX is V2/V3/V4",
	"        X is 0(disable) or 1(enable)",
	"p350sys ntp p [PORT]",
	"        query ntp netinfo",
	"p350sys ntp p PORT [ip IP] [mac MAC] [mask MASK] [gateway GATEWAY] [monitor ENABLE]",
	"        PORT is from 1 to "get_marco_value(V_ETH_NTP),
	"        IP is like XXX.XXX.XXX.XXX",
	"        MAC is like XX:XX:XX:XX:XX:XX",
	"        MASK is like XXX.XXX.XXX.XXX",
	"        GATEWAY is like XXX.XXX.XXX.XXX",
	"        ENABLE is 0(disable) or 1(enable)",
	"p350sys wg ip", 
	"        ip is like XXX.XXX.XXX.XXX",
	"p350sys event --help",
	"        show event help",
	"p350sys ph_upload --help",
	"        show phase perf upload help",
	NULL
	};





