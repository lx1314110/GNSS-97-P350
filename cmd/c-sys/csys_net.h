#ifndef __CSYS_NET__
#define	__CSYS_NET__



#include "lib_type.h"
#include "alloc.h"
#include "lib_net.h"









s32_t modify_net_param(char *pDbPath, char *tbl_sys, char * networkcard, struct netinfo *netp);
int write_eth(char *pDbPath, char *tbl_sys, struct netinfo *netp);
//int write_virtual_eth(char *pDbPath, char *tbl_sys, struct netinfo *netp);
s32_t query_net_param(char * networkcard, struct netinfo *netp);




#endif//__CSYS_NET__


