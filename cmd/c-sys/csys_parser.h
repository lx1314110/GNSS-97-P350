#ifndef __CSYS_PARSER__
#define __CSYS_PARSER__
#include "csys_phase_upload.h"
#include "csys_event.h"





int vport_validity(char *vport);
int mid_validity(char *md5_id);
int mkey_validity(char *md5_key);
int interval_validity(char *interval);

int user_argv_parser(int argc, char *argv[]);
int net_argv_parser(int argc, char *argv[]);
int leap_argv_parser(int argc, char *argv[]);
int time_argv_parser(int argc, char *argv[]);
int update_argv_parser(int argc, char *argv[]);
int clock_argv_parser(int argc, char *argv[]);
int ver_argv_parser(int argc, char *argv[]);
int dev_type_argv_parser(int argc, char *argv[]);
int bid_argv_parser(int argc, char *argv[]);
int slotinfo_argv_parser(int argc, char *argv[]);

int reboot_argv_parser(int argc, char *argv[]);

int ntp_veth_argv_parser(int argc, char *argv[]);
int ntp_md5_argv_parser(int argc, char *argv[]);
int ntp_bei_argv_parser(int argc, char *argv[]);
int ntp_ve_argv_parser(int argc, char *argv[]);
int ntp_argv_parser(int argc, char *argv[]);

int argv_parser(int argc, char *argv[]);
void helper(void);





#endif//__CSYS_PARSER__


