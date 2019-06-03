#ifndef	__DOUT_ISSUE__
#define	__DOUT_ISSUE__




#include "alloc.h"
#include "dout_context.h"

#define NEW_ISSUECONFIG_INTERFACE

#ifdef NEW_ISSUECONFIG_INTERFACE

#else
int RS232IssueConfig(int fpga_fd, struct rs232info *cfg, int slot);
int RS485IssueConfig(int fpga_fd, struct rs485info *cfg, int slot);
int BDCTIssueConfig(int fpga_fd, struct bdctinfo *cfg, int slot);
int BACIssueConfig(int fpga_fd, struct bacinfo *cfg, int slot);
int SF4IssueConfig(int fpga_fd, struct sf4info *cfg, int slot);
int BDC485IssueConfig(int fpga_fd, struct bdc485info *cfg, int slot);
int KJD8IssueConfig(int fpga_fd, struct kjd8info *cfg, int slot);
int TTL8IssueConfig(int fpga_fd, struct ttl8info *cfg, int slot);
int PPX485IssueConfig(int fpga_fd, struct ppx485info *cfg, int slot);
int PTPIssueConfig(int fpga_fd, struct ptpinfo *cfg, int slot);
int PTPFIssueConfig(int fpga_fd, struct ptpfinfo *cfg, int slot);
int OUTEIssueConfig(int fpga_fd, struct outeinfo *cfg, int slot);
int OUTHIssueConfig(int fpga_fd, struct outhinfo *cfg, int slot);

int KJD4IssueConfig(int fpga_fd, struct kjd4info *cfg, int slot);
int TP4IssueConfig(int fpga_fd, struct tp4info *cfg, int slot);
int SC4IssueConfig(int fpga_fd, struct sc4info *cfg, int slot);
#endif

int IssueConfig(struct outCtx *ctx, int slot, int bid);
int IssueDelay(struct outCtx *ctx);

int IssueOut2mbSA(struct outCtx *ctx);

int IssueOutBaudrate(struct outCtx *ctx);

int IssueIrigbAmplitude(struct outCtx *ctx);

int IssueIrigbVoltage(struct outCtx *ctx);

int SetOutBoardClockClass(struct outCtx *ctx);


int IssueIrigbZone(struct outCtx *ctx);
int IssueIrigbDST(struct outCtx *ctx);














#endif//__DOUT_ISSUE__


