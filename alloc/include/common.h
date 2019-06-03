#include "lib_dbg.h"
#include "lib_sqlite.h"
#include "lib_type.h"

typedef void (*sighandler_t)(int);

int SysReadClockType(int fpga_fd, u16_t *clock_type);
int SysReadPhase(int fpga_fd, int * sys_phase);
int SysReadClockStatus(int fpga_fd, u16_t *clock_sta);
int SysCheckFpgaRunStatus(int fpga_fd);
void SysWaitFpgaRunStatusOk(int fpga_fd);

int initializeCommExitSignal(sighandler_t handler);

int str_ip_group_to_uint32(char *ipv4_grp, u32_t *des_ipv4, u8_t * des_ip);
int uint32_ip_group_to_str_ip(u32_t *ipv4, u8_t ipv4_num, char *des_ipv4_grp);

#include "common_event.h"
#include "common_phase_perf.h"


