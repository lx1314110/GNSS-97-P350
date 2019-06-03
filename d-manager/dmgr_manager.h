#ifndef	__DMGR_MGR__
#define	__DMGR_MGR__





#include "dmgr_context.h"





bool_t ledRun(struct mgrCtx *ctx);
bool_t isRunning(struct mgrCtx *ctx);
void sysView(struct mgrCtx *ctx);

int readPid(struct pidinfo *pid);
int procStart(struct pidinfo *pid);
int procRestart(struct pidinfo *pid);
void procStop(struct pidinfo *pid);

int ReadFpgaVersion(struct mgrCtx *ctx);
int WriteVerToShareMemory(struct mgrCtx *ctx);

int DevMasterSlave(struct mgrCtx *ctx);

int ProcManager(struct mgrCtx *ctx);









#endif//__DMGR_MGR__


