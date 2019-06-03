#ifndef	__DIN_CONFIG__
#define	__DIN_CONFIG__



#include "din_context.h"





int ReadBidFromTable(struct inCtx *pInCtx, char *pTbl);
int DeleteConfig(struct inCtx *pInCtx, char *pTbl, int iSlot );
int EmptyConfig(struct inCtx *pInCtx, char *pTbl, int iSlot );

int ReadConfigFromRunTimeTable(
	struct inCtx *ctx, 
	char *pTbl, 
	int iSlot,
	int iBid, 
	int iPort );

int WriteConfigToRunTimeTable(
struct inCtx *ctx,
struct inputinfo *pCfg,
char *pTbl);


int CopyConfig(
	struct inCtx *pInCtx, 
	char *pFromTbl, 
	char *pToTbl, 
	int iSlot, 
	int iBid, 
	int iPort );


#if 0
int GPSBFReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int GPSBFWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int GPSBEReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int GPSBEWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int GBDBFReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int GBDBFWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int GBDBEReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int GBDBEWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int BDBFReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int BDBFWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int BDBEReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int BDBEWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int BEIReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int BEIWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int BFIReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int BFIWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int BFEIReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int BFEIWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );



int BFFIReadConfig(
	struct inCtx *pInCtx, 
	char *pTbl, 
	int iSlot,
	int iPort );



int BFFIWriteConfig(
	struct inCtx *pInCtx, 
	struct inputinfo *pCfg, 
	char *pTbl );
#endif



int ReadSchema(struct inCtx *ctx, char *pTbl);
int WriteSchema(struct inCtx *ctx, char *pTbl);



int WriteLeapToShareMemory(struct inCtx *ctx);

int WriteDataToCurPhasePerf(struct inCtx *ctx,
	struct phase_perf_tbl_t *PhasePerfTbl,
	int PhasePerfLen);

int ReadPhaseUploadSwitchFromTable(struct inCtx *ctx, char *pTbl);
int WritePhaseUploadSwitchToTable(struct inCtx *ctx,  char *pTbl);

int WritePhaseUploadSwitchToShareMemory(struct inCtx *ctx);

#endif//__DIN_CONFIG__


