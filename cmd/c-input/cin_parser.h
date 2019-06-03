#ifndef	__CIN_PARSER__
#define	__CIN_PARSER__





#include "cin_context.h"




int argv_parser(int argc, char *argv[], struct inCtx *ctx);

int port5_validity(char *port);
int schema_validity(char *src_num);
int slot_validity(char *slot);
int prio_validity(char *prio);
int delay_validity(char *delay);
int ssm_validity(char *ssm);
int sa_validity(char *sa);
int mode_validity(char *mode);
int elev_validity(char *elev);
int lon_validity(char *lon);
int lat_validity(char *lat);
int ip_group_validity(char *);


//ptp
int ptp_priority_validity(char *priority);
int ptp_delay_type_validity(char *pdt);
int ptp_multi_uni_type_validity(char *pmut);
int ptp_layer_type_validity(char *plt);
int ptp_step_type_validity(char *pst);
int ptp_freq_validity(char *pf);
//int ptp_ssmEnable_validity(char *trace);
int ptp_class_validity(char *class);
int ptp_variance_validity(char *variance);
int ptp_region_validity(char *region);

void helper(void);




#endif//__CIN_PARSER__


