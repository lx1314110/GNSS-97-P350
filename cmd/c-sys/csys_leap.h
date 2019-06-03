#ifndef __CSYS_LEAP__
#define __CSYS_LEAP__



#include "alloc.h"





int leap_validity(char *leap);
s32_t modify_leap(char *pDbPath, char *tbl_sys, struct leapinfo *pLeap);
s32_t query_leap(char *pDbPath, char *tbl_sys, struct leapinfo *pLeap);







#endif//__CSYS_LEAP__


