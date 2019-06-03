#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "lib_bit.h"
#include "lib_fpga.h"
#include "lib_dbg.h"
#include "lib_net.h"
#include "lib_shm.h"
#include "lib_sema.h"

#include "addr.h"
#include "dout_p50.h"
#include "dout_context.h"
#include "dout_global.h"
#include "dout_issue.h"

#define DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot) 	\
	do { \
		struct outcommoninfo commoncfg;	\
										\
		commoncfg.delay = cfg->delay;	\
		commoncfg.signalType = cfg->signalType;	\
		return CommonIssueConfig((fpga_fd), (void *)&commoncfg, (slot));	\
	}while(0)

#ifdef NEW_ISSUECONFIG_INTERFACE
typedef int IssueConfigFun_t(struct outCtx *, int);

struct IssueConfig_t {
	u8_t  boardId;
	IssueConfigFun_t *IssueConfigFun;
};

static int PTPIssueConfig(struct outCtx *ctx, int slot);
static int PTPFIssueConfig(struct outCtx *ctx, int slot);

#if 0
static int RS232IssueConfig(struct outCtx *ctx, int slot);
static int RS485IssueConfig(struct outCtx *ctx, int slot);
static int BDCTIssueConfig(struct outCtx *ctx, int slot);
static int BACIssueConfig(struct outCtx *ctx, int slot);
static int SF4IssueConfig(struct outCtx *ctx, int slot);
static int BDC485IssueConfig(struct outCtx *ctx, int slot);
static int KJD8IssueConfig(struct outCtx *ctx, int slot);
static int TTL8IssueConfig(struct outCtx *ctx, int slot);
static int PPX485IssueConfig(struct outCtx *ctx, int slot);
static int OUTEIssueConfig(struct outCtx *ctx, int slot);
static int OUTHIssueConfig(struct outCtx *ctx, int slot);
static int KJD4IssueConfig(struct outCtx *ctx, int slot);
static int TP4IssueConfig(struct outCtx *ctx, int slot);
static int SC4IssueConfig(struct outCtx *ctx, int slot);
#endif

static struct IssueConfig_t gIssueConfig[] = {
	/*{BID_RS232,RS232IssueConfig},
	{BID_RS485,RS485IssueConfig},
	{BID_BDCT,BDCTIssueConfig},
	{BID_BAC,BACIssueConfig},
	{BID_SF4,SF4IssueConfig},
	{BID_BDC485,BDC485IssueConfig},
	{BID_KJD8,KJD8IssueConfig},
	{BID_TTL8,TTL8IssueConfig},
	{BID_PPX485,PPX485IssueConfig},*/
	{BID_PTP,PTPIssueConfig},
	{BID_PTPF,PTPFIssueConfig},
	/*
	{BID_OUTE,OUTEIssueConfig},
	{BID_OUTH,OUTHIssueConfig},
	{BID_KJD4,KJD4IssueConfig},//new
	{BID_TP4,TP4IssueConfig},
	{BID_SC4,SC4IssueConfig},*/
};

#define LEN_GISSUECONFIG (sizeof(gIssueConfig)/sizeof(struct IssueConfig_t))
#endif

static int CommonIssueConfig(int fpga_fd, void *commoncfg, int slot);
static int PtpSendConfig(int fpga_fd, int slot, struct ptpinfo *cfg);
static int Ptp2SendConfig(int fpga_fd, int slot, struct ptpinfo *cfg);

static int PtpfSendConfig(int fpga_fd, int slot, struct ptpfinfo *cfg);

/*
  1	成功
  0	失败
*/
static int CommonIssueConfig(int fpga_fd, void *commoncfg, int slot)
{
	struct outcommoninfo *cfg = (struct outcommoninfo *) commoncfg;

	if(!cfg)
		return 0;

	if(SLOT_NONE < slot && SLOT_CURSOR_ARRAY_SIZE >= slot){
		if(!FpgaWrite(fpga_fd, FPGA_OUT_SIGNAL(slot), cfg->signalType))
			return 0;
	}else{
		return 0;
	}

	return 1;
}

/*
  1	成功
  0	失败
*/
static int OUTCOMMIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct outcommoninfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].outcomminfo[0]);
	if(!cfg)
		return 0;
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}

#if 0
/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int RS232IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct rs232info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].rs232[0]);
	if(!cfg)
		return 0;
#else
int RS232IssueConfig(int fpga_fd, struct rs232info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}



/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int RS485IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct rs485info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].rs485[0]);
	if(!cfg)
		return 0;
#else
int RS485IssueConfig(int fpga_fd, struct rs485info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}




/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int BDCTIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct bdctinfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].bdct[0]);
	if(!cfg)
		return 0;
#else
int BDCTIssueConfig(int fpga_fd, struct bdctinfo *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}




/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int BACIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct bacinfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].bac[0]);
	if(!cfg)
		return 0;
#else
int BACIssueConfig(int fpga_fd, struct bacinfo *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}





/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int SF4IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct sf4info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].sf4[0]);
	if(!cfg)
		return 0;
#else
int SF4IssueConfig(int fpga_fd, struct sf4info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}




/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int BDC485IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct bdc485info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].bdc485[0]);
	if(!cfg)
		return 0;
#else
int BDC485IssueConfig(int fpga_fd, struct bdc485info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}





/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int KJD8IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct kjd8info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].kjd8[0]);
	if(!cfg)
		return 0;
#else
int KJD8IssueConfig(int fpga_fd, struct kjd8info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}







/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int TTL8IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct ttl8info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].ttl8[0]);
	if(!cfg)
		return 0;
#else
int TTL8IssueConfig(int fpga_fd, struct ttl8info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}







/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int PPX485IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct ppx485info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].ppx485[0]);
	if(!cfg)
		return 0;
#else
int PPX485IssueConfig(int fpga_fd, struct ppx485info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}
#endif



/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int PTPIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct ptpinfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].ptp[0]);
	if(!cfg)
		return 0;
#else
int PTPIssueConfig(int fpga_fd, struct ptpinfo *cfg, int slot)
{
#endif
	#if 0 //move to outcomm
	struct outcommoninfo commoncfg;

	commoncfg.delay = cfg->ptpDelay;
	commoncfg.signalType = cfg->ptpSignalType;
	//commoncfg.signalType = SIGNAL_PTP;
	if(1 != CommonIssueConfig(fpga_fd, (void *)&commoncfg, slot))
		return 0;
	#endif
	if(cfg->ptpType == PTP_TYPE_CN){
		//国内模块
		if(1 != PtpSendConfig(fpga_fd, slot, cfg)) {
			return 0;
		}
	}else{
		//default 国外模块
		if(1 != Ptp2SendConfig(fpga_fd, slot, cfg)) {
			return 0;
		}
	}

	return 1;
}




/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int PTPFIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct ptpfinfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].ptpf[0]);
	if(!cfg)
		return 0;
#else
int PTPFIssueConfig(int fpga_fd, struct ptpfinfo *cfg, int slot)
{
#endif
	#if 0 //move to outcomm
	struct outcommoninfo commoncfg;

	commoncfg.delay = cfg->ptpDelay;
	commoncfg.signalType = cfg->ptpSignalType;
	//commoncfg.signalType = SIGNAL_PTP;
	if(1 != CommonIssueConfig(fpga_fd, (void *)&commoncfg, slot))
		return 0;
	#endif
	ctx->outSta[slot-1].ptpf[0].ptpSignalType = ctx->outSta[slot-1].outcomminfo[0].signalType;
	if(1 != PtpfSendConfig(fpga_fd, slot, cfg)) {
		return 0;
	}

	return 1;
}

#if 0

/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int OUTEIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct outeinfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].oute[0]);
	if(!cfg)
		return 0;
#else
int OUTEIssueConfig(int fpga_fd, struct outeinfo *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}


/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int OUTHIssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct outhinfo *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].outh[0]);
	if(!cfg)
		return 0;
#else
int OUTHIssueConfig(int fpga_fd, struct outhinfo *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}


/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int KJD4IssueConfig(struct outCtx * ctx, int slot)
{
	int fpga_fd = -1;
	struct kjd4info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].kjd4[0]);
	if(!cfg)
		return 0;
#else
int KJD4IssueConfig(int fpga_fd, struct kjd4info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}


/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int TP4IssueConfig(struct outCtx * ctx, int slot)
{
	int fpga_fd = -1;
	struct tp4info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].tp4[0]);
	if(!cfg)
		return 0;
#else
int TP4IssueConfig(int fpga_fd, struct tp4info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}

/*
  1	成功
  0	失败
*/
#ifdef NEW_ISSUECONFIG_INTERFACE
static int SC4IssueConfig(struct outCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct sc4info *cfg = NULL;

	if(slot > SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->outSta[slot-1].sc4[0]);
	if(!cfg)
		return 0;
#else
int SC4IssueConfig(int fpga_fd, struct sc4info *cfg, int slot)
{
#endif
	DEFAULT_COMMONCONFIG(fpga_fd, cfg, slot);
}
#endif

#ifdef NEW_ISSUECONFIG_INTERFACE
/*
  1	成功
  0	失败
*/
int IssueConfig(struct outCtx *ctx, int slot, int bid)

{
	int i;
	switch (bid){
		//these output board no support set issue
		case BID_NTP:
		case BID_NTPF:
		case BID_61850:
			return 1;//OK
			break;
	}
	if (0 == OUTCOMMIssueConfig(ctx, slot))
		return 0;

	 for (i = 0; i < LEN_GISSUECONFIG; ++i){
		if(gIssueConfig[i].boardId == bid){

			switch (bid)
			{
				case BID_PTP:
				case BID_PTPF:

					sleep(5);
					break;
			}

			if(0 == gIssueConfig[i].IssueConfigFun(ctx, slot)){
				return 0;
			}
			break;
		}
	}
	return 1;

}

#else

/*
  1	成功
  0	失败
*/
int IssueConfig(struct outCtx *ctx, int slot, int bid)
{
	if(BID_RS232== bid)
	{
		if(0 == RS232IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].rs232[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_RS485 == bid)
	{
		if(0 == RS485IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].rs485[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_BDCT == bid)
	{
		if(0 == BDCTIssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].bdct[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_BAC == bid)
	{
		if(0 == BACIssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].bac[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_SF4 == bid)
	{
		if(0 == SF4IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].sf4[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_BDC485 == bid)
	{
		if(0 == BDC485IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].bdc485[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_KJD8 == bid)
	{
		if(0 == KJD8IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].kjd8[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_KJD4 == bid)
	{
		if(0 == KJD4IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].kjd4[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_TTL8 == bid)
	{
		if(0 == TTL8IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].ttl8[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_PPX485 == bid)
	{
		if(0 == PPX485IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].ppx485[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_PTP == bid)
	{
		sleep(5);
		if(0 == PTPIssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].ptp[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_PTPF == bid)
	{
		sleep(5);
		if(0 == PTPFIssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].ptpf[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_OUTH == bid)
	{
		if(0 == OUTHIssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].outh[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_OUTE == bid)
	{
		if(0 == OUTEIssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].oute[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_TP4 == bid)
	{
		if(0 == TP4IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].tp4[0]), slot))
		{
			return 0;
		}
	}
	else if(BID_SC4 == bid)
	{
		if(0 == SC4IssueConfig(ctx->fpga_fd, &(ctx->outSta[slot-1].sc4[0]), slot))
		{
			return 0;
		}
	}
	else
	{
		//do nothing
	}

	return 1;
}
#endif

#if 1
#define SIGN_BIT	27

#define SET_SIGNAL_OUTDELAY(fpga_fd, reg_delay_low, reg_delay_high, delay) \
do { \
	int __tmp = 0, __sign = 1; 	\
	u16_t __value = 0;		\
							\
	__tmp = delay;			\
	if(__tmp < 0){			\
		__sign = -1;			\
		__tmp = -__tmp;			\
	}						\
							\
	__tmp &= 0x07FFFFFF;		\
	if(-1 == __sign){			\
		__tmp |= BIT(SIGN_BIT);\
	}						\
							\
	__value = __tmp &0x0000FFFF;			\
	if(!FpgaWrite(fpga_fd, reg_delay_low, __value)){		\
		return 0;	\
	}				\
					\
	__value = (__tmp >> 16) &0x00000FFF;		\
	if(!FpgaWrite(fpga_fd, reg_delay_high, __value)){	\
		return 0;			\
	}	\
}while(0)
/*
  1	成功
  0	失败
*/
int IssueDelay(struct outCtx *ctx)
{
	SET_SIGNAL_OUTDELAY(ctx->fpga_fd, FPGA_SYS_OUT_TOD_DELAY_LOW, FPGA_SYS_OUT_TOD_DELAY_HIGH, 
						ctx->od[OUTDELAY_SIGNAL_TOD-1].delay);

	SET_SIGNAL_OUTDELAY(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_DELAY_LOW, FPGA_SYS_OUT_IRIGB_DELAY_HIGH, 
						ctx->od[OUTDELAY_SIGNAL_IRIGB-1].delay);

	SET_SIGNAL_OUTDELAY(ctx->fpga_fd, FPGA_SYS_OUT_PPX_DELAY_LOW, FPGA_SYS_OUT_PPX_DELAY_HIGH, 
						ctx->od[OUTDELAY_SIGNAL_PPX-1].delay);
	
	SET_SIGNAL_OUTDELAY(ctx->fpga_fd, FPGA_SYS_OUT_PTP_DELAY_LOW, FPGA_SYS_OUT_PTP_DELAY_HIGH, 
							ctx->od[OUTDELAY_SIGNAL_PTP-1].delay);

	return 1;
}

#else
int IssueDelay(struct outCtx *ctx)
{
	u16_t value;
	int sign;
	int tmp;

	tmp = ctx->od[OUTDELAY_SIGNAL_TOD-1].delay;
	if(tmp < 0)
	{
		sign = -1;
		tmp = -tmp;
	}
	else
	{
		sign = 1;
	}
	
	tmp &= 0x0FFFFFFF;
	if(-1 == sign)
	{
		tmp |= BIT(28);
	}
	
	value = tmp &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_DELAY_LOW, value))
	{
		return 0;
	}

	value = (tmp >> 16) &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_DELAY_HIGH, value))
	{
		return 0;
	}


	tmp = ctx->od[OUTDELAY_SIGNAL_IRIGB-1].delay;
	if(tmp < 0)
	{
		sign = -1;
		tmp = -tmp;
	}
	else
	{
		sign = 1;
	}
	
	tmp &= 0x0FFFFFFF;
	if(-1 == sign)
	{
		tmp |= BIT(28);
	}
	
	value = tmp &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_DELAY_LOW, value))
	{
		return 0;
	}

	value = (tmp >> 16) &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_DELAY_HIGH, value))
	{
		return 0;
	}


	tmp = ctx->od[OUTDELAY_SIGNAL_PPX-1].delay;
	if(tmp < 0)
	{
		sign = -1;
		tmp = -tmp;
	}
	else
	{
		sign = 1;
	}
	
	tmp &= 0x0FFFFFFF;
	if(-1 == sign)
	{
		tmp |= BIT(28);
	}
	
	value = tmp &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_PPX_DELAY_LOW, value))
	{
		return 0;
	}

	value = (tmp >> 16) &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_PPX_DELAY_HIGH, value))
	{
		return 0;
	}

	tmp = ctx->od[OUTDELAY_SIGNAL_PTP-1].delay;
	if(tmp < 0)
	{
		sign = -1;
		tmp = -tmp;
	}
	else
	{
		sign = 1;
	}
	
	tmp &= 0x0FFFFFFF;
	if(-1 == sign)
	{
		tmp |= BIT(28);
	}
	
	value = tmp &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_PTP_DELAY_LOW, value))
	{
		return 0;
	}

	value = (tmp >> 16) &0x0000FFFF;
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_PTP_DELAY_HIGH, value))
	{
		return 0;
	}


	return 1;
}
#endif



/*
  1	成功
  0	失败
*/
int IssueOut2mbSA(struct outCtx *ctx)
{
	u8_t * mb_sa = NULL;
	u16_t san = 0x00;

	mb_sa = ctx->out_sa.mb_sa;

	if(0 == memcmp(mb_sa, gSaTbl[SA_CURSOR_SA4], sizeof(gSaTbl[SA_CURSOR_SA4]))){
		san = 0x01;
	}else if(0 == memcmp(mb_sa, gSaTbl[SA_CURSOR_SA5], sizeof(gSaTbl[SA_CURSOR_SA5]))){
		san = 0x02;
	}else if(0 == memcmp(mb_sa, gSaTbl[SA_CURSOR_SA6], sizeof(gSaTbl[SA_CURSOR_SA6]))){
		san = 0x03;
	}else if(0 == memcmp(mb_sa, gSaTbl[SA_CURSOR_SA7], sizeof(gSaTbl[SA_CURSOR_SA7]))){
		san = 0x04;
	}else if(0 == memcmp(mb_sa, gSaTbl[SA_CURSOR_SA8], sizeof(gSaTbl[SA_CURSOR_SA8]))){
		san = 0x05;
	}else{
		print(	DBG_WARNING, "--Invalid SA, use default SA.");
		san = 0x04;
	}
		//设置SA字节
	// sa
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SA, san))
	{
		return 0;
	}

	return 1;
}




/*
  1	成功
  0	失败
*/
int IssueOutBaudrate(struct outCtx *ctx)
{
	u16_t temp;

	temp = ctx->out_br.br[0];
#ifdef NEW_FPGA_ADDR_COMM
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_TOD_BR, temp))
	{
		return 0;
	}
#else
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G01_OUT_TOD_BR, temp))
	{
		return 0;
	}

	temp = ctx->out_br.br[1];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G02_OUT_TOD_BR, temp))
	{
		return 0;
	}

	temp = ctx->out_br.br[2];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G03_OUT_TOD_BR, temp))
	{
		return 0;
	}
	
	temp = ctx->out_br.br[3];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G04_OUT_TOD_BR, temp))
	{
		return 0;
	}
#endif	

	return 1;
}










/*
  1	成功
  0	失败
*/
int IssueIrigbAmplitude(struct outCtx *ctx)
{
	u16_t temp;

	temp = ctx->irigb_amp.amp[0];
#ifdef NEW_FPGA_ADDR_COMM
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_AR, temp))
	{
		return 0;
	}
#else
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G01_OUT_IRIGB_AR, temp))
	{
		return 0;
	}

	temp = ctx->irigb_amp.amp[1];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G02_OUT_IRIGB_AR, temp))
	{
		return 0;
	}

	temp = ctx->irigb_amp.amp[2];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G03_OUT_IRIGB_AR, temp))
	{
		return 0;
	}
	
	temp = ctx->irigb_amp.amp[3];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G04_OUT_IRIGB_AR, temp))
	{
		return 0;
	}
#endif	

	return 1;
}











/*
  1	成功
  0	失败
*/
int IssueIrigbVoltage(struct outCtx *ctx)
{
	u16_t temp;

	temp = ctx->irigb_vol.vol[0];
#ifdef NEW_FPGA_ADDR_COMM
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_V, temp))
	{
		return 0;
	}
#else
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G01_OUT_IRIGB_V, temp))
	{
		return 0;
	}

	temp = ctx->irigb_vol.vol[1];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G02_OUT_IRIGB_V, temp))
	{
		return 0;
	}

	temp = ctx->irigb_vol.vol[2];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G03_OUT_IRIGB_V, temp))
	{
		return 0;
	}
	
	temp = ctx->irigb_vol.vol[3];
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G04_OUT_IRIGB_V, temp))
	{
		return 0;
	}
#endif	

	return 1;
}


#if 1
//以下函数根据锁相环的状态来进行设置
//暂时支持设置国内模块，国外模块的命令不支持
u8_t const num2hex[] = {
		0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x61,0x62,0x63,0x64,0x65,0x66
};

static void ptp_set_ssm(struct outCtx *ctx, u8_t ssm_level)
{
	int i;
	int fpga_fd = ctx->fpga_fd;

	for (i = SLOT_CURSOR_1; i < SLOT_CURSOR_ARRAY_SIZE; ++i){
		if(ctx->outSta[i].boardId == BID_PTP){
			if(ctx->outSta[i].ptp[0].ptpType == PTP_TYPE_CN){
				u8_t buffer[20] = "ptp2 s ssm ";

				buffer[11] = ssm_level + 0x30;
				buffer[12] = '\n';
				buffer[13] = '\0';
				p50_write(fpga_fd, i+1, buffer, strlen((char *)buffer)+1);
				
				usleep(100000);
			}
		}
	}
}


//set the clock property
static void ptp_set_clkproperty(struct outCtx *ctx, u8_t clkclass,u8_t accuracy,u16_t range)
{

	int i;
	int fpga_fd = ctx->fpga_fd;

	for (i = SLOT_CURSOR_1; i < SLOT_CURSOR_ARRAY_SIZE; ++i){
		if(ctx->outSta[i].boardId == BID_PTP){
			//国内国外模块都支持
			u8_t buffer[27] = "ptp2 c q ";
			if(ctx->outSta[i].ptp[0].ptpType == PTP_TYPE_CN){

				buffer[9]  = num2hex[clkclass/16];
				buffer[10] = num2hex[clkclass%16];
				buffer[11] = ' ';
				buffer[12] = num2hex[accuracy/16];
				buffer[13] = num2hex[accuracy%16];
				buffer[14] = ' ';
				buffer[15] = num2hex[range/4096];
				buffer[16] = num2hex[(range%4096)/256];
				buffer[17] = num2hex[(range%256)/16];
				buffer[18] = num2hex[range%16];
				buffer[19] = '\n';
				buffer[20] = '\0';
			}else if (ctx->outSta[i].ptp[0].ptpType == PTP_TYPE_FRGN){
				//国外模块
				buffer[9] = '0';
				buffer[10] = 'x';
				buffer[11]  = num2hex[clkclass/16];
				buffer[12] = num2hex[clkclass%16];
				buffer[13] = ' ';
				buffer[14] = '0';
				buffer[15] = 'x';
				buffer[16] = num2hex[accuracy/16];
				buffer[17] = num2hex[accuracy%16];
				buffer[18] = ' ';
				buffer[19] = '0';
				buffer[20] = 'x';
				buffer[21] = num2hex[range/4096];
				buffer[22] = num2hex[(range%4096)/256];
				buffer[23] = num2hex[(range%256)/16];
				buffer[24] = num2hex[range%16];
				buffer[25] = '\n';
				buffer[26] = '\0';

			}
			p50_write(fpga_fd, i+1, buffer, strlen((char *)buffer)+1);
			usleep(100000);
			
			//Uart_2_send_str(buffer); //观察发送的串
		}
	}
}


//set freq_trace
static void ptp_set_freqtrace(struct outCtx *ctx, u8_t value) //normal
{
	int i;
	int fpga_fd = ctx->fpga_fd;

	for (i = SLOT_CURSOR_1; i < SLOT_CURSOR_ARRAY_SIZE; ++i){
		if(ctx->outSta[i].boardId == BID_PTP){
			if(ctx->outSta[i].ptp[0].ptpType == PTP_TYPE_CN){
			    u8_t buffer[15] = "ptp2 p f ";

				buffer[9] = value + 0x30;
				buffer[10] = '\n';
				buffer[11] = '\0';

				p50_write(fpga_fd, i+1, buffer, strlen((char *)buffer)+1);
				usleep(100000);
			}
		}
	}
}

//set time_trace
static void ptp_set_timetrace(struct outCtx *ctx, u8_t value) //normal
{
	int i;
	int fpga_fd = ctx->fpga_fd;

	for (i = SLOT_CURSOR_1; i < SLOT_CURSOR_ARRAY_SIZE; ++i){
		if(ctx->outSta[i].boardId == BID_PTP){
			if(ctx->outSta[i].ptp[0].ptpType == PTP_TYPE_CN){
			    u8_t buffer[15] = "ptp2 p t ";

				buffer[9] = value + 0x30;
				buffer[10] = '\n';
				buffer[11] = '\0';

				p50_write(fpga_fd, i+1, buffer, strlen((char *)buffer)+1);
				usleep(100000);
			}
		}
	}
}

//set time source
static void ptp_set_timesource(struct outCtx *ctx, u8_t value)  //normal
{
	int i;
	int fpga_fd = ctx->fpga_fd;

	for (i = SLOT_CURSOR_1; i < SLOT_CURSOR_ARRAY_SIZE; ++i){
		if(ctx->outSta[i].boardId == BID_PTP){
			if(ctx->outSta[i].ptp[0].ptpType == PTP_TYPE_CN){
			    u8_t buffer[15] = "ptp2 p s ";

				buffer[9] = num2hex[value/16];
				buffer[10] = num2hex[value%16];
				buffer[11] = '\n';
				buffer[12] = '\0';

				p50_write(fpga_fd, i+1, buffer, strlen((char *)buffer)+1);
				usleep(100000);
			}
		}
	}
}

#endif


int SetOutBoardClockClass(struct outCtx *ctx)
{
	static u16_t clkSta,clkSta_pre=4;
	static u16_t clock_source,clock_source_pre;
	//1-RB 2-XO
	u16_t clock_type;
	//int i;
	//如果无选源时，以下保持上一次不变。
	static u16_t tsrc_type = NO_SOURCE_TYPE;
	static u16_t tsrc_flag = INVALID_SOURCE_FLAG;
	static u16_t tsrc_slot = 0; //为2mh/2mb频率源

#if 0
	u16_t in_mbssm = 0x0F;
	u16_t in_mhssm = 0x0F;
#endif
	static u16_t in_ssm = 0x00, in_ssm_pre = 0x09;//use invaild, make sure first exec
	/*
	  0x00		一级钟
	  0x01		二级钟
	  0x02		三级钟
	  0x03-0xFF	保留
	*/
	u16_t clock_stratum;
	struct clock_stainfo clock_sta;
	
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			 SHM_OFFSET_CLOCK, 
			 sizeof(clock_sta), 
			 (char *)&(clock_sta), 
			 sizeof(clock_sta));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_RBXO_TYP, &clock_type))
	{
		return -1;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_INSRC_STRATUM, &clock_stratum))
	{
		return -1;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_INSRC_SELECT, &clock_source))
	{
		return -1;
	}
	/*如果输入源为2mh/10mh/1pps等频率或者2mb且钟状态为FAST或者LOCK时，因为输出2mb ssm等级要跟随输入频率源.
	如果输入源ssm等级改变时，钟状态和选源并不会改变，所以把此操作放在下面if(clkSta != clkSta_pre || clock_source != clock_source_pre)
	函数内的话，ssm等级的改变不会被设置。故需要移至此处。*/
	if (tsrc_flag == FREQ_SOURCE_FLAG && (clkSta == 4 || clkSta == 3)){
						//频率源且钟状态是快捕，或者锁定
						
		if(tsrc_slot > INPUT_SLOT_CURSOR_1 && tsrc_slot <=INPUT_SLOT_CURSOR_ARRAY_SIZE ){
		
			if(tsrc_type == S2MB_SOURCE_TYPE){
				// 输入源为2mb时，ssm来自输入，其实2mb也可以用寄存器FPGA_SYS_CUR_INSRC_SSM
				if(!FpgaRead(ctx->fpga_fd, FPGA_IN_2MB_SSM(tsrc_slot), &in_ssm)){
					print(	DBG_ERROR, "--Failed to read in ssm of 2mb.");
					return 0;
				}
				
			}else{
				// 输入源为除2mb之外的其他源时，ssm来自用户配置
				if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_CUR_INSRC_SSM, &in_ssm)){
					print(	DBG_ERROR, "--Failed to read in ssm of 2mh from user.");
					return 0;
				}
			}
		}else{
			in_ssm = SSM_VAL_0F;
		}
		print(DBG_DEBUG, "tsrc_slot:%d tsrc_type:%d in_ssm:0x%x\n",tsrc_slot, tsrc_type, in_ssm);
		if(in_ssm != in_ssm_pre){
			FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, in_ssm);
			in_ssm_pre = in_ssm;
		}
	}
	/*
	钟状态：	 时间源输出SSM等级			  频率源
	FREE:	 0F 				   与时间源一致
	HOLD:	 RB:04/XO:08		   与时间源一致
	FAST:	   04				   输出2MB: =2MB信号SSM；输出2MHz等于网页输入源ssm配置
	LOCK:	   02				   输出2MB: =2MB信号SSM；输出2MHz等于网页输入源ssm配置
	*/	
	clkSta = clock_sta.state;
	/*钟状态或者输入源变化时，才执行以下段落*/
	if(clkSta != clkSta_pre || clock_source != clock_source_pre)
	{

		print(DBG_INFORMATIONAL, "Setting OutClockClass.");
		if(clock_source != NO_SOURCE){//无源时保持上次状态，不加则实时状态
			FIND_TYPE_BY_FPGA_VAL(clock_source, tsrc_type);
			FIND_FLAG_BY_FPGA_VAL(clock_source, tsrc_flag);
			FIND_SLOT_BY_FPGA_VAL(clock_source, tsrc_slot);
		}
		#if 0
		//cur or last source 2mb/2mh
		if (tsrc_flag == FREQ_SOURCE_FLAG){
			if(clock_source == NO_SOURCE){
				//以移动到下文状态一栏处理
				do{} while(0);
				//FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x0F);
			}else{
				//当前确实为频率源
				if(tsrc_type == S2MH_SOURCE_TYPE){
					// 输入源为2mh时，ssm来自用户配置
					if(tsrc_slot > INPUT_SLOT_CURSOR_1 && tsrc_slot <=INPUT_SLOT_CURSOR_ARRAY_SIZE ){
						if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_CUR_INSRC_SSM, &in_mhssm)){
							print(	DBG_ERROR, "--Failed to read in ssm of 2mh from user.");
							return 0;
						}
					}else{
						//no source and last source is freq
						in_mhssm=0x0F;
					}
					FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, in_mhssm);
				}else if(tsrc_type == S2MB_SOURCE_TYPE){
					if(tsrc_slot > INPUT_SLOT_CURSOR_1 && tsrc_slot <=INPUT_SLOT_CURSOR_ARRAY_SIZE ){
						if(!FpgaRead(ctx->fpga_fd, FPGA_IN_2MB_SSM(tsrc_slot), &in_mbssm)){
							print(	DBG_ERROR, "--Failed to read in ssm of 2mb.");
							return 0;
						}
					}else{
						//no source and last source is freq
						in_mbssm=0x0F;
					}
					FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, in_mbssm);
				}
			}
		}
		#endif
		switch ( clkSta )
		{
			case 4://锁定
				{				
					ptp_set_ssm(ctx,4);
				#if defined (CHINA_MOBILE_TOD_RULE) || defined (CHINA_MIIT_TOD_RULE)

					if(tsrc_flag == FREQ_SOURCE_FLAG)
						#ifdef CHINA_MOBILE_TOD_RULE
						ptp_set_clkproperty(ctx,7,0xFE,0xFFFF);//移动标准
						#else
						ptp_set_clkproperty(ctx,7,0x21,0xFFFF);//工信部标准
						#endif
					else
				#endif
					ptp_set_clkproperty(ctx,6,0x21,0x4E5D);

					ptp_set_freqtrace(ctx,1);
					ptp_set_timetrace(ctx,1);

					switch(tsrc_type){
						case GPS_SOURCE_TYPE:
							ptp_set_timesource(ctx,0x20);
							break;
						case TOD_SOURCE_TYPE:
							ptp_set_timesource(ctx,0x40);
							break;
						case IRIGB1_SOURCE_TYPE:
						case IRIGB2_SOURCE_TYPE:
							ptp_set_timesource(ctx,0x90);
							break;
						default:
							ptp_set_timesource(ctx,0xA0);
							break;
					}

					//TIME_SOURCE_FLAG NTP
					//GPS 1PPS+TOD
					//printf("#####tsrc_flag:%d\n",tsrc_flag);
					if(tsrc_flag == BOTH_SOURCE_FLAG){
						FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x02);//SSM_02
						in_ssm_pre = 0x02;
					}
					//频率源的ssm等级写入，因为是直通，所以移动到上面函数中
				}
				break;
			case 1:
			case 2://保持
				{
					ptp_set_ssm(ctx,8);
			#if defined (CHINA_MOBILE_TOD_RULE) || defined (CHINA_MIIT_TOD_RULE)

				#ifdef CHINA_MOBILE_TOD_RULE
					//保持, (晶体)钟,
					if((clock_type == 2) || 
						(tsrc_flag == FREQ_SOURCE_FLAG)
					)
						ptp_set_clkproperty(ctx,52,0xFE,0xFFFF);
					else if( clock_type == 1) //保持(铷)钟,无频率
						ptp_set_clkproperty(ctx,8,0xFE,0xFFFF);
						
				#else	//miit
					//保持, (晶体/三级)钟, 频率, (2mh/2mb 频率输入设备)
					if((clock_type == 2) || 
						(2 == clock_stratum &&
						(tsrc_flag == FREQ_SOURCE_FLAG))
					)
						ptp_set_clkproperty(ctx,52,0x21,0xFFFF);
					
						//保持, (铷/二级)钟, 频率, (2mh/2mb 频率输入设备)
					if(
						(clock_type == 1) || 
							(1 == clock_stratum && 
							(tsrc_flag == FREQ_SOURCE_FLAG))
					  )
						ptp_set_clkproperty(ctx,8,0x21,0xFFFF);
				#endif
			#else //联通
					ptp_set_clkproperty(ctx,160,0xFE,0xFFFF);
			#endif	
					ptp_set_freqtrace(ctx,0);
					ptp_set_timetrace(ctx,0);
					ptp_set_timesource(ctx,0xA0);
					//TIME_SOURCE_FLAG NTP
					//GPS 1PPS+TOD
					//add 2mb/2mh
					//if(tsrc_flag == BOTH_SOURCE_FLAG){
						if (clock_type == 1){//rb
							FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x04);//SSM_04
							in_ssm_pre = 0x04;
						}else{ // xo
							FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x08);//SSM_08
							in_ssm_pre = 0x08;
						}
					//}
				}

				
				break;
			case 3://快捕
				{
					ptp_set_ssm(ctx,8);
					#if defined (CHINA_MOBILE_TOD_RULE) || defined (CHINA_MIIT_TOD_RULE)
					#ifdef CHINA_MOBILE_TOD_RULE
					ptp_set_clkproperty(ctx,255,0xFE,0xFFFF);
					#else
					ptp_set_clkproperty(ctx,255,0x21,0xFFFF);
					#endif
					#else
					ptp_set_clkproperty(ctx,248,0xFE,0xFFFF);
					#endif
					ptp_set_freqtrace(ctx,0);
					ptp_set_timetrace(ctx,0);
					ptp_set_timesource(ctx,0xA0);
					//TIME_SOURCE_FLAG NTP
					//GPS 1PPS+TOD
					if(tsrc_flag == BOTH_SOURCE_FLAG){
						FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x04);//SSM_04
						in_ssm_pre = 0x04;
					}
					//频率源的ssm等级写入，因为是直通，所以移动到上面函数中
				}
				break;
			case 0://自由运行
				{
					ptp_set_ssm(ctx,8);
					#if defined (CHINA_MOBILE_TOD_RULE) || defined (CHINA_MIIT_TOD_RULE)
					#ifdef CHINA_MOBILE_TOD_RULE
					ptp_set_clkproperty(ctx,255,0xFE,0xFFFF);
					#else
					ptp_set_clkproperty(ctx,255,0x21,0xFFFF);
					#endif
					#else
					ptp_set_clkproperty(ctx,248,0xFE,0xFFFF);
					#endif
					ptp_set_freqtrace(ctx,0);
					ptp_set_timetrace(ctx,0);
					ptp_set_timesource(ctx,0xA0);
					//TIME_SOURCE_FLAG NTP
					//GPS 1PPS+TOD
					//当前或上次源为时间源,最新：自由运行状态下，都为0F
					//if(tsrc_flag == BOTH_SOURCE_FLAG){
						FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x0F);//SSM_0F
						in_ssm_pre = 0x0F;
					//}
				}
				break;
			default:
				{
					ptp_set_ssm(ctx,8);
					#if defined (CHINA_MOBILE_TOD_RULE) || defined (CHINA_MIIT_TOD_RULE)
					#ifdef CHINA_MOBILE_TOD_RULE
					ptp_set_clkproperty(ctx,255,0xFE,0xFFFF);
					#else
					ptp_set_clkproperty(ctx,255,0x21,0xFFFF);
					#endif
					#else
					ptp_set_clkproperty(ctx,248,0xFE,0xFFFF);
					#endif
					ptp_set_freqtrace(ctx,0);
					ptp_set_timetrace(ctx,0);
					ptp_set_timesource(ctx,0xA0);
					//TIME_SOURCE_FLAG NTP
					//GPS 1PPS+TOD
					//当前或上次源为时间源.最新：自由运行状态下，都为0F
					//if(tsrc_flag == BOTH_SOURCE_FLAG){
						FpgaWrite(ctx->fpga_fd, FPGA_SYS_2MB_OUT_SSM, 0x0F);//SSM_0F
						in_ssm_pre = 0x0F;
					//}
				}
				break;
		}
		clkSta_pre = clkSta;
		clock_source_pre = clock_source;		
	}

	return 0;	
}






/*适配P502.5.5.3*/


#define PTP_DELAY_TIME	100000


/*
  1	成功
  0	失败
*/
static int PtpSendConfig(int fpga_fd, int slot, struct ptpinfo *cfg)
{
	u16_t len = 0;
	u16_t tmp = 0;
	char buf[64];
	char ptpCmd[2048];
	char str_ipv4[MAX_IP_LEN];

	memset(ptpCmd, 0, sizeof(ptpCmd));
	print(DBG_INFORMATIONAL, "slot:%d setting starting...\n", slot);

	//delay type
	if(DELAY_E2E == cfg->ptpDelayType)
	{
		tmp = strlen(P50_CMD_E2E)+1;
		memcpy(ptpCmd +len, P50_CMD_E2E, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(DELAY_P2P == cfg->ptpDelayType)
	{
		tmp = strlen(P50_CMD_P2P)+1;
		memcpy(ptpCmd +len, P50_CMD_P2P, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else
	{
		return 0;
	}
	
	//layer
	if(LAYER2 == cfg->ptpLayer)
	{
		tmp = strlen(P50_CMD_LAYER2)+1;
		memcpy(ptpCmd +len, P50_CMD_LAYER2, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else if(LAYER3 == cfg->ptpLayer)
	{
		tmp = strlen(P50_CMD_LAYER3)+1;
		memcpy(ptpCmd +len, P50_CMD_LAYER3, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else
	{
		return 0;
	}

	//step
	if(ONE_STEP == cfg->ptpStep)
	{
		tmp = strlen(P50_CMD_1STEP)+1;
		memcpy(ptpCmd +len, P50_CMD_1STEP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else if(TWO_STEP == cfg->ptpStep)
	{
		tmp = strlen(P50_CMD_2STEP)+1;
		memcpy(ptpCmd +len, P50_CMD_2STEP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else
	{
		return 0;
	}
	//PTP ssm_enable
	if(ZERO == cfg->ptpSsmEnable)
	{
		tmp = strlen(P50_CMD_SSMENABLE_ZERO)+1;
		memcpy(ptpCmd +len, P50_CMD_SSMENABLE_ZERO, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else if(ONE == cfg->ptpSsmEnable)
	{
		tmp = strlen(P50_CMD_SSMENABLE_ONE)+1;
		memcpy(ptpCmd +len, P50_CMD_SSMENABLE_ONE, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else
	{
		return 0;
	}
	
	//unicast

	if (UNICAST == cfg->ptpMulticast)
	{
		//单播设置
		print(DBG_INFORMATIONAL, "UNICAST Setting.\n");
		tmp = strlen(P50_CMD_UNICAST)+1;
		memcpy(ptpCmd +len, P50_CMD_UNICAST, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;

		if(cfg->ptpSlaveIp_num > 0){
			memset(buf, 0, sizeof(buf));
			//ip_to_hex((char *)cfg->ptpSlaveIp);
			memset(str_ipv4, 0x0, sizeof(str_ipv4));
			UINTIP_TO_STRIP(cfg->ptpSlaveIp[0], str_ipv4);//因为本国内模块只支持一个ip，我们只取第一个
			P50_FILL_SLAVE_IP(buf, str_ipv4);
			tmp = strlen(buf)+1;
			memcpy(ptpCmd +len, buf, tmp);
			if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
			{
					return 0;
			}
			usleep(PTP_DELAY_TIME);

			len += tmp;
		}
	}
	else if(MULTICAST == cfg->ptpMulticast)
	{
		//组播设置
		print(DBG_INFORMATIONAL, "MULTICAST Setting.\n");
		tmp = strlen(P50_CMD_MULTICAST)+1;
		memcpy(ptpCmd +len, P50_CMD_MULTICAST, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	//sync
	memset(buf, 0, sizeof(buf));
	P50_FILL_SYNC(buf, -8 +cfg->ptpSync);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//announce
	memset(buf, 0, sizeof(buf));
	P50_FILL_ANNOUNCE(buf, -8 +cfg->ptpAnnounce);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//pdelay req
	memset(buf, 0, sizeof(buf));
	P50_FILL_PDELAY_REQ(buf, -8 +cfg->ptpPdelayReq);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	
	//ptp region
	memset(buf, 0, sizeof(buf));
	P50_FILL_REGION(buf, atoi((char *)cfg->ptpRegion));
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	/*ip地址必须向设置，如果P50没有设置ip地址情况下不能正常启动*/
	//ip
	memset(buf, 0, sizeof(buf));
	//ip_to_hex((char *)cfg->ptpIp);
	P50_FILL_IP(buf, cfg->ptpIp);
	//printf("%s\n",buf);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//Mac
	memset(buf, 0, sizeof(buf));
	ptp_set_mac(buf,cfg->ptpMac);
	//printf("%s\n",buf);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	
	

	//Priority1
	memset(buf, 0, sizeof(buf));
	P50_FILL_LEVEL(buf, 1,atoi((char *)cfg->ptpLevel));
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	//Priority2
	memset(buf, 0, sizeof(buf));
	P50_FILL_LEVEL(buf, 2,atoi((char *)cfg->ptpPriority));
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	print(DBG_INFORMATIONAL, "setting end. cur len:%d.\n", len);
	if(len >= 2048)
		return 0;//error


	return 1;
}


/*
老外模块，和ptp老外输入模块一致。
PtpSendConfig和Ptp2SendConfig共享一个数据结构，共享资源，只是根据用户确定类型，选用哪种
配置函数
与ptp共享bid，共同增加ptpType，0-国外；1-国内

国外模块和国内模块区别：
增加ptpMask, ptpGateway
无：ptpMac, ptpSsmEnable
改：ptpPdelayReq：表示国外delay，国内delay。我暂时没加国外pdelay
  1	成功
  0	失败
*/

#define PTP2_LOGIN_USER	"root"
#define PTP2_LOGIN_PASSWD	"root"

static int Ptp2SendConfig(int fpga_fd, int slot, struct ptpinfo *cfg)
{
	int i;
	u16_t len = 0;
	u16_t tmp = 0;
	char buf[128];
	char ptpCmd[2048];
	char str_ipv4[MAX_IP_LEN];

	static u8_t old_ptpIp[SLOT_CURSOR_ARRAY_SIZE][MAX_IP_LEN];//ip 此处三处，最好改为二维数组加上槽位，我这个是临时做法。如果可以，就删除这个吧
	//u8_t old_ptpMac[MAX_MAC_LEN];//mac //only for 国内模块
	static u8_t old_ptpMask[SLOT_CURSOR_ARRAY_SIZE][MAX_MASK_LEN];//mask //仅仅为国外模块
	static u8_t old_ptpGateway[SLOT_CURSOR_ARRAY_SIZE][MAX_GATEWAY_LEN];//gateway   //仅仅为国外模块

	memset(ptpCmd, 0, sizeof(ptpCmd));
	print(DBG_INFORMATIONAL, "slot:%d setting starting...\n", slot);
	/* 登陆*/
	#if 0
	tmp = strlen("\rroot\rroot\r");
	memset(buf, 0, sizeof(buf));
	memcpy(buf, "\rroot\rroot\r", tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	#else
	memset(buf, 0, sizeof(buf));
	memcpy(buf, "\n", 1);
	tmp = strlen(buf)+1;
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s\n", PTP2_LOGIN_USER);
	tmp = strlen(buf)+1;
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s\n", PTP2_LOGIN_PASSWD);
	tmp = strlen(buf)+1;
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	#endif

	/*ip地址必须向设置，如果P50没有设置ip地址情况下不能正常启动*/
	//ip
	if(memcmp(cfg->ptpIp, old_ptpIp[slot-1], sizeof(cfg->ptpIp)) != 0){
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_2_FILL_IP(buf, cfg->ptpIp);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		memcpy(old_ptpIp[slot-1], cfg->ptpIp, sizeof(cfg->ptpIp));
	}

	//mask
	if(memcmp(cfg->ptpMask, old_ptpMask[slot-1], sizeof(cfg->ptpMask)) != 0){
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_2_FILL_MASK(buf, cfg->ptpMask);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		memcpy(old_ptpMask[slot-1], cfg->ptpMask, sizeof(cfg->ptpMask));
	}

	//gateway
	if(memcmp(cfg->ptpGateway, old_ptpGateway[slot-1], sizeof(cfg->ptpGateway)) != 0){
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_2_FILL_GATEWAY(buf, cfg->ptpGateway);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		memcpy(old_ptpGateway[slot-1], cfg->ptpGateway, sizeof(cfg->ptpGateway));
	}
	
	//delay type
	if(DELAY_E2E == cfg->ptpDelayType)
	{
		tmp = strlen(P50_CMD_E2E)+1;
		memcpy(ptpCmd +len, P50_CMD_E2E, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(DELAY_P2P == cfg->ptpDelayType)
	{
		tmp = strlen(P50_CMD_P2P)+1;
		memcpy(ptpCmd +len, P50_CMD_P2P, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else
	{
		return 0;
	}
	
	//layer
	if(LAYER2 == cfg->ptpLayer)
	{
		tmp = strlen(P50_CMD_LAYER2)+1;
		memcpy(ptpCmd +len, P50_CMD_LAYER2, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else if(LAYER3 == cfg->ptpLayer)
	{
		tmp = strlen(P50_CMD_LAYER3)+1;
		memcpy(ptpCmd +len, P50_CMD_LAYER3, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else
	{
		return 0;
	}

	//step
	if(ONE_STEP == cfg->ptpStep)
	{
		tmp = strlen(P50_CMD_1STEP)+1;
		memcpy(ptpCmd +len, P50_CMD_1STEP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else if(TWO_STEP == cfg->ptpStep)
	{
		tmp = strlen(P50_CMD_2STEP)+1;
		memcpy(ptpCmd +len, P50_CMD_2STEP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else
	{
		return 0;
	}

	#if 0
	//PTP ssm_enable
	if(ZERO == cfg->ptpSsmEnable)
	{
		tmp = strlen(P50_CMD_SSMENABLE_ZERO)+1;
		memcpy(ptpCmd +len, P50_CMD_SSMENABLE_ZERO, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else if(ONE == cfg->ptpSsmEnable)
	{
		tmp = strlen(P50_CMD_SSMENABLE_ONE)+1;
		memcpy(ptpCmd +len, P50_CMD_SSMENABLE_ONE, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	else
	{
		return 0;
	}
	#endif
	//unicast
#define SUPPORT_ONLY_ONE_GROUP_NODE

	if (UNICAST == cfg->ptpMulticast)
	{
		//单播设置
		print(DBG_INFORMATIONAL, "UNICAST Setting.\n");
		tmp = strlen(P50_2_CMD_UNICAST)+1;
		memcpy(ptpCmd +len, P50_2_CMD_UNICAST, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;

		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpSlaveIp);
		if(LAYER3 == cfg->ptpLayer){
			#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
			for(i = 0; i < cfg->oldptpSlaveIp_num; i++)	
			#else
			//support set mul master point
			for(i = 0; i < cfg->ptpSlaveIp_num; i++)
			#endif
			{
				memset(str_ipv4, 0x0, sizeof(str_ipv4));
				#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
				UINTIP_TO_STRIP(cfg->oldptpSlaveIp[i], str_ipv4);
				#else
				UINTIP_TO_STRIP(cfg->ptpSlaveIp[i], str_ipv4);
				#endif
				P50_2_DEL_SLAVE_IP(buf, str_ipv4);

				tmp = strlen(buf)+1;
				memcpy(ptpCmd +len, buf, tmp);
				if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
				{
						return 0;
				}
				usleep(PTP_DELAY_TIME);
				len += tmp;
				memset(buf, 0, sizeof(buf));
			}

		}else{ //LAYER2
			#if 0
			#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
			P50_DEL_MASTER_MAC(buf, cfg->oldptpSlaveMac);
			#else
			P50_DEL_MASTER_MAC(buf, cfg->ptpSlaveMac);
			#endif
			tmp = strlen(buf)+1;
			memcpy(ptpCmd +len, buf, tmp);
			if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
			{
					return 0;
			}
			usleep(PTP_DELAY_TIME);
			len += tmp;
			memset(buf, 0, sizeof(buf));
			#endif
		}

		//ip_to_hex((char *)cfg->ptpSlaveIp);
		if(LAYER3 == cfg->ptpLayer){
			for(i = 0; i < cfg->ptpSlaveIp_num; i++){
				memset(str_ipv4, 0x0, sizeof(str_ipv4));
				UINTIP_TO_STRIP(cfg->ptpSlaveIp[i], str_ipv4);
				#ifdef SUPPORT_ADD_NODE_PRIO_SYNC_DELAY
				P50_2_FILL_SLAVE_IP_PSD(buf, str_ipv4, -8 + cfg->ptpAnnounce, -8 + cfg->ptpSync, -8 + cfg->ptpPdelayReq);
				#else
				P50_2_FILL_SLAVE_IP(buf, str_ipv4);
				#endif

				tmp = strlen(buf)+1;
				memcpy(ptpCmd +len, buf, tmp);
				if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
				{
						return 0;
				}
				usleep(PTP_DELAY_TIME);
				len += tmp;
			}
			#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
			memset(cfg->oldptpSlaveIp, 0x0, sizeof(cfg->oldptpSlaveIp));
			memcpy(cfg->oldptpSlaveIp, cfg->ptpSlaveIp, sizeof(cfg->ptpSlaveIp));
			cfg->oldptpSlaveIp_num = cfg->ptpSlaveIp_num;
			#endif
		}else{ //LAYER2
			//暂不支持二层单播
			#if 0
			#ifdef SUPPORT_ADD_NODE_PRIO_SYNC_DELAY
			P50_FILL_MASTER_MAC_PSD(buf, cfg->ptpSlaveMac, cfg->ptpPriority, -8 + cfg->ptpSync,-8 + cfg->ptpdelayReq);
			#else
			P50_FILL_MASTER_MAC(buf, cfg->ptpSlaveMac);
			#endif
			#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
			memset(cfg->oldptpSlaveMac, 0x0, MAX_MAC_LEN);
			memcpy(cfg->oldptpSlaveMac, cfg->ptpSlaveMac, MAX_MAC_LEN);
			#endif
			tmp = strlen(buf)+1;
			memcpy(ptpCmd +len, buf, tmp);
			if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
			{
					return 0;
			}
			usleep(PTP_DELAY_TIME);
			len += tmp;
			#endif
		}
	}
	else if(MULTICAST == cfg->ptpMulticast)
	{
		//组播设置
		print(DBG_INFORMATIONAL, "MULTICAST Setting.\n");
		tmp = strlen(P50_2_CMD_MULTICAST)+1;
		memcpy(ptpCmd +len, P50_2_CMD_MULTICAST, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);

		len += tmp;
	}
	//sync
	memset(buf, 0, sizeof(buf));
	P50_FILL_SYNC(buf, -8 +cfg->ptpSync);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//announce
	memset(buf, 0, sizeof(buf));
	P50_FILL_ANNOUNCE(buf, -8 +cfg->ptpAnnounce);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//delay req
	memset(buf, 0, sizeof(buf));
	//P50_FILL_DELAY_REQ(buf, -8 +cfg->ptpDelayReq);
	P50_FILL_PDELAY_REQ(buf, -8 +cfg->ptpPdelayReq);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	#if 0
	//pdelay req
	memset(buf, 0, sizeof(buf));
	P50_FILL_PDELAY_REQ(buf, -8 +cfg->ptpPdelayReq);
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	#endif
	#if 1
	//ptp region
	memset(buf, 0, sizeof(buf));
	P50_2_FILL_REGION(buf, atoi((char *)cfg->ptpRegion));
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	#endif

	//Priority1
	memset(buf, 0, sizeof(buf));
	P50_FILL_LEVEL(buf, 1,atoi((char *)cfg->ptpLevel));
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	//Priority2
	memset(buf, 0, sizeof(buf));
	P50_FILL_LEVEL(buf, 2,atoi((char *)cfg->ptpPriority));
	tmp = strlen(buf)+1;
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
			return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;
	print(DBG_INFORMATIONAL, "setting end. cur len:%d.\n", len);
	//print(DBG_INFORMATIONAL, "%s\n", ptpCmd);
	if(len >= 2048)
		return 0;//error


	return 1;
}










/*
  1	成功
  0	失败
*/
static int PtpfSendConfig(int fpga_fd, int slot, struct ptpfinfo *cfg)
{
	u16_t len = 0;
	u16_t tmp = 0;
	char buf[64];
	char ptpCmd[1024];

	memset(ptpCmd, 0, sizeof(ptpCmd));


	/* 登陆*/
	tmp = strlen("\rroot\rroot\r");
	memset(buf, 0, sizeof(buf));
	memcpy(buf, "\rroot\rroot\r", tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
		usleep(PTP_DELAY_TIME);
	//signal type
	if(SIGNAL_NONE == cfg->ptpSignalType)
	{
		tmp = strlen(P50_CMD_NTP_OFF);
		memcpy(ptpCmd +len, P50_CMD_NTP_OFF, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		tmp = strlen(P50_CMD_PTP_OFF);
		memcpy(ptpCmd +len, P50_CMD_PTP_OFF, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(SIGNAL_NTP == cfg->ptpSignalType)
	{
		tmp = strlen(P50_CMD_NTP_ON);
		memcpy(ptpCmd +len, P50_CMD_NTP_ON, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		tmp = strlen(P50_CMD_PTP_OFF);
		memcpy(ptpCmd +len, P50_CMD_PTP_OFF, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(SIGNAL_PTP == cfg->ptpSignalType)
	{

		
	}
	else
	{
		return 0;
	}

	//delay type
	if(DELAY_E2E == cfg->ptpDelayType)
	{
		tmp = strlen(P50_CMD_E2E);
		memcpy(ptpCmd +len, P50_CMD_E2E, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(DELAY_P2P == cfg->ptpDelayType)
	{
		tmp = strlen(P50_CMD_P2P);
		memcpy(ptpCmd +len, P50_CMD_P2P, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else
	{
		return 0;
	}


	//layer
	if(LAYER2 == cfg->ptpLayer)
	{
		tmp = strlen(P50_CMD_LAYER2);
		memcpy(ptpCmd +len, P50_CMD_LAYER2, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(LAYER3 == cfg->ptpLayer)
	{
		tmp = strlen(P50_CMD_LAYER3);
		memcpy(ptpCmd +len, P50_CMD_LAYER3, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else
	{
		return 0;
	}

	//step
	if(ONE_STEP == cfg->ptpStep)
	{
		tmp = strlen(P50_CMD_1STEP);
		memcpy(ptpCmd +len, P50_CMD_1STEP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else if(TWO_STEP == cfg->ptpStep)
	{
		tmp = strlen(P50_CMD_2STEP);
		memcpy(ptpCmd +len, P50_CMD_2STEP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
			return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else
	{
		return 0;
	}
	
	//sync
	memset(buf, 0, sizeof(buf));
	P50_FILL_SYNC(buf, -8 +cfg->ptpSync);
	tmp = strlen(buf);
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//announce
	memset(buf, 0, sizeof(buf));
	P50_FILL_ANNOUNCE(buf, -8 +cfg->ptpAnnounce);
	tmp = strlen(buf);
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	//pdelay req
	memset(buf, 0, sizeof(buf));
	P50_FILL_PDELAY_REQ(buf, -8 +cfg->ptpPdelayReq);
	tmp = strlen(buf);
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	/*ip地址必须向设置，如果P50没有设置ip地址情况下不能正常启动*/
		//ip
	memset(buf, 0, sizeof(buf));
	P50_FILL_IP(buf, cfg->ptpIp);
	tmp = strlen(buf);
	memcpy(ptpCmd +len, buf, tmp);
	if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);
	len += tmp;

	return 1;
}









/*
  1	成功
  0	失败
*/
int IssueIrigbZone(struct outCtx *ctx)
{
	int value;
	
	value = gTzTbl[ctx->out_zone.zone[0] -'@'];
	if(value < 0)
	{
		value = -value;
		value |= BIT(4);
	}
#ifdef NEW_FPGA_ADDR_COMM
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_ZONE, value))
	{
		return 0;
	}

#else
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G01_OUT_IRIGB_ZONE, value))
	{
		return 0;
	}


	value = gTzTbl[ctx->out_zone.zone[1] -'@'];
	if(value < 0)
	{
		value = -value;
		value |= BIT(4);
	}
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G02_OUT_IRIGB_ZONE, value))
	{
		return 0;
	}


	value = gTzTbl[ctx->out_zone.zone[2] -'@'];
	if(value < 0)
	{
		value = -value;
		value |= BIT(4);
	}
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G03_OUT_IRIGB_ZONE, value))
	{
		return 0;
	}


	value = gTzTbl[ctx->out_zone.zone[3] -'@'];
	if(value < 0)
	{
		value = -value;
		value |= BIT(4);
	}
	if(!FpgaWrite(ctx->fpga_fd, FPGA_G04_OUT_IRIGB_ZONE, value))
	{
		return 0;
	}
#endif

	return 1;
}












/*
  1	成功
  0	失败
*/
int IssueIrigbDST(struct outCtx *ctx)
{
	if(!FpgaWrite(ctx->fpga_fd, FPGA_SYS_OUT_IRIGB_DST, 0))
	{
		return 0;
	}
	return 1;
}













