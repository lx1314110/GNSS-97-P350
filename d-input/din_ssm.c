#include <string.h>

#include "lib_dbg.h"
#include "lib_fpga.h"

#include "addr.h"
#include "alloc.h"
#include "din_context.h"
#include "din_ssm.h"

typedef int ReadSSMFun_t(struct inCtx *, int);

struct ReadSSM_t {
	u8_t  boardId;
	ReadSSMFun_t *ReadSSMFun;
};

//support gpsbe,gpsbf, gbdbe,gbdbf,bdbf,bdbd,bfei,bffi; not bei,bfi. only for 2mb
static int SATCOMMReadSSM(struct inCtx *ctx, int slot);//SSM 时钟等级

static struct ReadSSM_t gReadSSM[] = {
	{BID_GPSBF,SATCOMMReadSSM},
	{BID_GPSBE,SATCOMMReadSSM},
	{BID_GBDBF,SATCOMMReadSSM},
	{BID_GBDBE,SATCOMMReadSSM},
	{BID_BDBF,SATCOMMReadSSM},
	{BID_BDBE,SATCOMMReadSSM},
	//{BID_BEI,BEIReadSSM},
	//{BID_BFI,BFIReadSSM},
	{BID_BFEI,SATCOMMReadSSM},
	{BID_BFFI,SATCOMMReadSSM},
};
#define LEN_GREADSSM (sizeof(gReadSSM)/sizeof(struct ReadSSM_t))

#if 1
/*
	only for 2mb
  1	成功
  0	失败
*/
static int SATCOMMReadSSM(struct inCtx *ctx, int slot)//SSM 时钟等级
{
	int i;
	u16_t ssm;
	if (SLOT_NONE >= slot || INPUT_SLOT_CURSOR_ARRAY_SIZE < slot){
		print(	DBG_ERROR, 
				"--Invalid slot.");
		return 0;
	}
	#if 0
	if (!(ctx->inSta[slot-1].satcommon.sta_flag & FLAG_SAT_COMMSTA_MB_SSM)){
		return 1;//return true
	}
	#endif
	if(!FpgaRead(ctx->fpga_fd, FPGA_IN_2MB_SSM(slot), &ssm))
	{
		print(	DBG_ERROR, 
				"--Failed to read ssm of 2mb.");
		return 0;
	}

	ssm &= 0x000F;

	for (i = 0; i < gSsmTbl_len; ++i){
		if(ssm == gSsmTbl[i].val)
			ctx->inSta[slot-1].incommsta.incomminfo.ssm[SAT_PORT_2MB-1] = ssm;
	}
	return 1;
}

#else
/*
  1	成功
  0	失败
*/
static int SATCOMMReadSSM(struct inCtx *ctx, int slot)//SSM 时钟等级
{
	u16_t ssm;
	if (SLOT_NONE >= slot || INPUT_SLOT_CURSOR_ARRAY_SIZE < slot){
		print(	DBG_ERROR, 
				"--Invalid slot.");
		return 0;
	}

	if (!(ctx->inSta[slot-1].satcommon.sta_flag & FLAG_SAT_COMMSTA_MB_SSM)){
		return 1;//return true
	}
	if(!FpgaRead(ctx->fpga_fd, FPGA_IN_2MB_SSM(slot), &ssm))
	{
		print(	DBG_ERROR, 
				"--Failed to read ssm of 2mb.");
		return 0;
	}

	ssm &= 0x000F;
	switch(ssm)
	{
		case 0x02:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_02, 2);
			break;
				
		case 0x04:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_04, 2);
			break;
				
		case 0x08:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_08, 2);
			break;

		case 0x0B:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_0B, 2);
			break;

		case 0x0F:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_0F, 2);
			break;

		case 0x00:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_00, 2);
			break;

		default:
			memcpy(ctx->inSta[slot-1].satcommon.mb_ssm, SSM_00, 2);
	}

	return 1;
}
#endif

/*
  1	成功
  0	失败
*/
int ReadSSM(struct inCtx *ctx)
{
	int i,j;

	for(i=INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		 for (j = 0; j < LEN_GREADSSM; ++j){
			if(gReadSSM[j].boardId == ctx->inSta[i].boardId){
				if(0 == gReadSSM[j].ReadSSMFun(ctx, i+1)){
					return 0;
				}
				break;
			}
		}
	}

	return 1;
}


