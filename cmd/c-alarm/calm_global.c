#include "calm_global.h"













struct almCtx gAlmCtx;











//#define THRESHOLD_PHVALUE_MAX

char *gHelpTbl[] = {
	"p350alm --help",
	"p350alm",
	"        query all supported alm",
	"        show alm_sta|mask|index|alm_id|slot|port|bid|msg",
	"p350alm threshold [VALUE]",
	"        VALUE (max:"get_marco_value(THRESHOLD_PHVALUE_MAX)")",
	"p350alm current [PAGE]",
	"        new interface [PAGE:1]",
	"        query current unshild and shiled(mask) alm",
	"        show alm_sta|mask|index|alm_id|slot|port|bid|msg|start_time|record_id(snmp)",
	"p350alm curnum",
	"p350alm upload [PAGE]",
	"        new interface [PAGE:1]",
	"        upload current unshild and shiled(mask) alm",
	"        show record_id(snmp)|alm_sta|mask|index|alm_id|slot|port|bid|msg|start_time",
	"        show the record nums of upload unshild and shiled alm",
	"p350alm uplnum",
	"p350alm history [PAGE]",
	"p350alm select PORT [ALMID]",
	"p350alm mask ALMINX [MASK]",
	"        PAGE is an integer.",
	"        PORT is from 1 to "get_marco_value(SLOT_MAX_PORT)".",
	"        MASK is from 0(do not mask) to 1(mask).",
	"        ALMINX is from 0 to "get_marco_value(ALM_CURSOR_ARRAY_SIZE)".",
	"p350alm bdalm",
	"        query the almnum of all board.",
	"p350alm almnum",
	"        show the record nums of history alm",
	NULL
};









