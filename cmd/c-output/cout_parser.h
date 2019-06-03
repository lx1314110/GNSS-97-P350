#ifndef	__COUT_PARSER__
#define	__COUT_PARSER__







#include "cout_context.h"








int argv_parser(int argc, char *argv[], struct outCtx *ctx);

int ip_group_validity(char *ipv4_grp);
int slot_validity(char *slot);

int sa_validity(char *sa);

int zone_validity(char *zone);
int zone_group_validity(char *zone_grp);
int group4_validity(char *group);

int baudrate_validity(char *br);
int baudrate_group_validity(char *br_grp);

int amplitude_ratio_validity(char *ar);
int amplitude_ratio_group_validity(char *ar);

int voltage_validity(char *v);
int voltage_group_validity(char *v);

int delay_validity(char *delay);
int signal_validity(char *signal);
int ptp_type_validity(char *type);
int ptp_priority_validity(char *priority);
int ptp_delay_type_validity(char *pdt);
int ptp_multi_uni_type_validity(char *pmut);
int ptp_layer_type_validity(char *plt);
int ptp_step_type_validity(char *pst);
int ptp_freq_validity(char *pf);
int ptp_ssmEnable_validity(char *trace);
int ptp_class_validity(char *class);
int ptp_variance_validity(char *variance);
int ptp_region_validity(char *region);
void helper(void);










#endif//__COUT_PARSER__


