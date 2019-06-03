#ifndef	__DIN_ISSUE__
#define	__DIN_ISSUE__





#include "alloc.h"
#include "din_context.h"



#define NEW_ISSUECONFIG_INTERFACE
#ifdef NEW_ISSUECONFIG_INTERFACE

#else

int GPSBFIssueConfig(int fpga_fd, struct gpsbfinfo *cfg, int slot);
int GPSBEIssueConfig(int fpga_fd, struct gpsbeinfo *cfg, int slot);

int GBDBFIssueConfig(int fpga_fd, struct gbdbfinfo *cfg, int slot);
int GBDBEIssueConfig(int fpga_fd, struct gbdbeinfo *cfg, int slot);

int BDBFIssueConfig(int fpga_fd, struct bdbfinfo *cfg, int slot);
int BDBEIssueConfig(int fpga_fd, struct bdbeinfo *cfg, int slot);

int BEIIssueConfig(int fpga_fd, struct beiinfo *cfg, int slot);
int BFIIssueConfig(int fpga_fd, struct bfiinfo *cfg, int slot);

int BFEIIssueConfig(int fpga_fd, struct bfeiinfo *cfg, int slot);
int BFFIIssueConfig(int fpga_fd, struct bffiinfo *cfg, int slot);
#endif


int IssueConfig(struct inCtx *ctx, int slot, int bid, int set_which);

int IssueLeapForecast(struct leapinfo *lp);







#endif//__DIN_ISSUE__


