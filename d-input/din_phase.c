#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_bit.h"

#include "addr.h"
#include "din_context.h"
#include "din_phase.h"

typedef int ReadPhaseFun_t(struct inCtx *, int);

struct ReadPhase_t {
	u8_t  boardId;
	ReadPhaseFun_t *ReadPhaseFun;
};

static int SATCOMMReadPhase(struct inCtx *ctx, int slot);
//support 10 sat mode all.
static struct ReadPhase_t gReadPhase[] = {
	{BID_GPSBF,SATCOMMReadPhase},
	{BID_GPSBE,SATCOMMReadPhase},
	{BID_GBDBF,SATCOMMReadPhase},
	{BID_GBDBE,SATCOMMReadPhase},
	{BID_BDBF,SATCOMMReadPhase},
	{BID_BDBE,SATCOMMReadPhase},
	{BID_BEI,SATCOMMReadPhase},
	{BID_BFI,SATCOMMReadPhase},
	{BID_BFEI,SATCOMMReadPhase},
	{BID_BFFI,SATCOMMReadPhase},
};
#define LEN_GREADPHASE (sizeof(gReadPhase)/sizeof(struct ReadPhase_t))

/*
  1	成功
  0	失败
*/
static int ReadPhaseFromFPGACommon(int fpga_fd, int slot, int *ph)
{
	int i;
	int sign;
	u16_t ph_low, ph_high;

	int *phase = NULL;
	phase = ph;

	if (!phase){
		print(	DBG_ERROR, 
				"--Null phase.");
		return 0;
	}
	if (SLOT_NONE >= slot || INPUT_SLOT_CURSOR_ARRAY_SIZE < slot)
	{
		print(	DBG_ERROR, 
				"--Invalid slot.");
		return 0;
	}
	// gps/bd
	if(!FpgaRead(fpga_fd, FPGA_IN_GB_PH_LOW16(slot), &ph_low))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of satellite.");
		return 0;
	}
	if(!FpgaRead(fpga_fd, FPGA_IN_GB_PH_HIGH12(slot), &ph_high))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of satellite.");
		return 0;
	}
	phase[0] = (ph_high << 16)| ph_low;


	// 2mh         2MHz
	if(!FpgaRead(fpga_fd, FPGA_IN_2MH_PH_LOW16(slot), &ph_low))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of 2mh.");
		return 0;
	}
	if(!FpgaRead(fpga_fd, FPGA_IN_2MH_PH_HIGH12(slot), &ph_high))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of 2mh.");
		return 0;
	}
	phase[1] = (ph_high << 16)| ph_low;
	

	// 2mb			2Mbit/s
	if(!FpgaRead(fpga_fd, FPGA_IN_2MB_PH_LOW16(slot), &ph_low))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of 2mb.");
		return 0;
	}
	if(!FpgaRead(fpga_fd, FPGA_IN_2MB_PH_HIGH12(slot), &ph_high))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of 2mb.");
		return 0;
	}
	phase[2] = (ph_high << 16)| ph_low;


	// irigb1
	if(!FpgaRead(fpga_fd, FPGA_IN_IRIGB1_PH_LOW16(slot), &ph_low))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of irigb1.");
		return 0;
	}
	if(!FpgaRead(fpga_fd, FPGA_IN_IRIGB1_PH_HIGH12(slot), &ph_high))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of irigb1.");
		return 0;
	}
	phase[3] = (ph_high << 16)| ph_low;

	
	// irigb2
	if(!FpgaRead(fpga_fd, FPGA_IN_IRIGB2_PH_LOW16(slot), &ph_low))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of irigb2.");
		return 0;
	}
	if(!FpgaRead(fpga_fd, FPGA_IN_IRIGB2_PH_HIGH12(slot), &ph_high))
	{
		print(	DBG_ERROR, 
				"--Failed to read phase of irigb2.");
		return 0;
	}
	phase[4] = (ph_high << 16)| ph_low;

	for(i=0; i<5; i++){
		if(phase[i] &BIT(23)){
			sign = -1;
		}else{
			sign = 1;
		}
		phase[i] &= (~BIT(23));
		phase[i] *= sign;
	}

	return 1;
}

static int SATCOMMReadPhase(struct inCtx *ctx, int slot)
{
	int phase[PORT_SAT];

	//support 10 sat type, all
	if (!(ctx->inSta[slot-1].satcommon.sta_flag & FLAG_SAT_COMMSTA_PH)){
		return 1;//return true
	}
	if(!ReadPhaseFromFPGACommon(ctx->fpga_fd, slot, phase)){
		return 0;
	}

	memcpy(ctx->inSta[slot -1].satcommon.ph, phase, PORT_SAT);
	return 1;	
}

/*
  1	成功
  0	失败
*/
int ReadPhase(struct inCtx *ctx)
{
	int i,j;

	for(i=INPUT_SLOT_CURSOR_1; i< INPUT_SLOT_CURSOR_ARRAY_SIZE; i++){
		 for (j = 0; j < LEN_GREADPHASE; ++j){
			if(gReadPhase[j].boardId == ctx->inSta[i].boardId){
				if(0 == gReadPhase[j].ReadPhaseFun(ctx, i+1)){
					return 0;
				}
				break;
			}
		}
	}

	return 1;
}


