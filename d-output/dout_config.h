#ifndef __DOUT_CONFIG__
#define __DOUT_CONFIG__



#include "dout_context.h"




int ReadBidFromFpga(struct outCtx * ctx);
int ReadBidFromTable(struct outCtx *ctx, char *pTbl);

int DeleteConfig(struct outCtx *ctx, char *pTbl, int iSlot);
int EmptyConfig(struct outCtx *ctx, char *pTbl, int iSlot);
int CopyConfig(struct outCtx *ctx, char *pFromTbl, char *pToTbl, int iSlot, int iBid, int iPort);

#if 0
int RS232ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int RS232WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int RS485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int RS485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int BDCTReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int BDCTWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int BACReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int BACWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int SF4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot,int iPort);
int SF4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int BDC485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int BDC485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int KJD8ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int KJD8WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);
int KJD4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int KJD4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int TTL8ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int TTL8WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int PPX485ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int PPX485WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int PTPReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int PTPWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int PTPFReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int PTPFWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int OUTEReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int OUTEWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int OUTHReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int OUTHWriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int TP4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int TP4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);

int SC4ReadConfig(struct outCtx *ctx, char *pTbl, int iSlot, int iPort);
int SC4WriteConfig(struct outCtx *ctx, struct outputinfo *pCfg, char *pTbl);
#endif

int ReadOutTimezone(struct outCtx *ctx, char *pTbl);
int WriteOutTimezone(struct outCtx *ctx, char *pTbl);

int ReadInTimezone(struct outCtx *ctx, char *pTbl);
int WriteInTimezone(struct outCtx *ctx, char *pTbl);

int ReadLeapsecond(struct outCtx *ctx, char *pTbl);
int WriteLeapsecond(struct outCtx *ctx, char *pTbl);

int ReadOut2mbSA(struct outCtx *ctx, char *pTbl);
int WriteOut2mbSA(struct outCtx *ctx, char *pTbl);

int ReadDelay(struct outCtx *ctx, char *pTbl);
int WriteDelay(struct outCtx *ctx, char *pTbl);

int ReadOutBaudrate(struct outCtx *ctx, char *pTbl);
int WriteOutRaudrate(struct outCtx *ctx, char *pTbl);


int ReadIrigbAmplitude(struct outCtx *ctx, char *pTbl);
int WriteIrigbAmplitude(struct outCtx *ctx, char *pTbl);


int ReadIrigbVoltage(struct outCtx *ctx, char *pTbl);
int WriteIrigbVoltage(struct outCtx *ctx, char *pTbl);


//int ReadConfigFromTable(struct outCtx *ctx, char *pTbl, int iSlot, int iBid, int iPort);
int ReadConfigFromRunTimeTable(
	struct outCtx *ctx, 
	char *pTbl, 
	int iSlot,
	int iBid, 
	int iPort );

int WriteConfigToRunTimeTable(
struct outCtx *ctx,
struct outputinfo *pCfg,
char *pTbl);


int WriteConfigToShareMemory(struct outCtx *ctx, int iSlot);
int WriteOutZoneToShareMemory(struct outCtx *ctx);
int WriteInZoneToShareMemory(struct outCtx *ctx);
int WriteLeapToShareMemory(struct outCtx *ctx);
int WriteDelayToShareMemory(struct outCtx *ctx);
int WriteOut2mbSAToShareMemory(struct outCtx *ctx);
int WriteOutBaudrateToShareMemory(struct outCtx *ctx);


int WriteIrigbAmplitudeToShareMemory(struct outCtx *ctx);
int WriteIrigbVoltageToShareMemory(struct outCtx *ctx);


int WriteAlarmToShareMemory(struct outCtx *ctx);










#endif//__DOUT_CONFIG__


