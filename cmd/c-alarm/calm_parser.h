#ifndef	__CALM_PARSER__
#define	__CALM_PARSER__





#include "calm_context.h"





int argv_parser(int argc, char *argv[], struct almCtx *ctx);
int almport_validity(char *port);
int alarm_id_validity(char *alarm_id);
int mask_sta_validity(char *mask_sta);
int page_validity(char *page);
int threshold_validity(char *threshold);

void helper(void);





#endif//__CALM_PARSER__


