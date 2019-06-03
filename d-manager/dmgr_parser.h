#ifndef __DMGR_PARSER__
#define __DMGR_PARSER__




#include "dmgr_context.h"




bool_t isNotify(struct mgrCtx *ctx);
int ReadMessageQueue(struct pidinfo * pid, struct ipcinfo *ipc);

int sysLevel(char *data, struct ipcinfo *ipc);
int sysReboot(char *data, struct pidinfo *pid, struct ipcinfo *ipc);
int sysUpdate(char *data, struct pidinfo *pid, struct ipcinfo *ipc);

bool_t isReboot(struct rebootinfo *reboot);
bool_t isUpdate(struct updateinfo *update);






#endif//__DMGR_PARSER__


