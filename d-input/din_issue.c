#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_fpga.h"
#include "lib_bit.h"
#include "lib_dbg.h"
#include "lib_common.h"

#include "addr.h"

#include "din_satellite.h"
#include "din_p50.h"
#include "din_issue.h"

typedef int IssueConfigFun_t(struct inCtx *, int);

struct IssueConfig_t {
	u8_t  boardId;
	IssueConfigFun_t *IssueConfigFun;
};

static int SATCOMMIssueConfig(struct inCtx *ctx, int slot);
static int PTPINIssueConfig(struct inCtx *ctx, int slot);


static struct IssueConfig_t gIssueConfig[] = {
	{BID_GPSBF,SATCOMMIssueConfig},
	{BID_GPSBE,SATCOMMIssueConfig},
	{BID_GBDBF,SATCOMMIssueConfig},
	{BID_GBDBE,SATCOMMIssueConfig},
	{BID_BDBF,SATCOMMIssueConfig},
	{BID_BDBE,SATCOMMIssueConfig},
	{BID_BEI,SATCOMMIssueConfig},
	{BID_BFI,SATCOMMIssueConfig},
	{BID_BFEI,SATCOMMIssueConfig},
	{BID_BFFI,SATCOMMIssueConfig},
	//PTP OTHER
	{BID_PTP_IN,PTPINIssueConfig},
};
#define LEN_GISSUECONFIG (sizeof(gIssueConfig)/sizeof(struct IssueConfig_t))

#if 1
#define SIGN_BIT	27

#define SET_SIGNAL_INDELAY(fpga_fd, reg_delay_low, reg_delay_high, delay) \
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
#else
#define SIGN_BIT	11

#define SET_SIGNAL_INDELAY(fpga_fd, reg_delay_low, reg_delay_high, delay) \
do{	\
	u16_t __tmp = 0;	\
	int __delay = 0;	\
	/*__delay = delay + 7;*/	\
	__delay = delay;	\
	if(__delay < 0){		\
		__delay = -__delay;	\
	}						\
	__tmp = __delay &0x0000FFFF;		\
	if(!FpgaWrite(fpga_fd, reg_delay_low, __tmp)){	\
		return 0;	\
	}				\
					\
	__tmp = (__delay >> 16) &0x0000FFFF;	\
	__tmp &= 0x07FF;						\
	if(delay < 0){							\
		__tmp |= BIT(SIGN_BIT);				\
	}										\
	if(!FpgaWrite(fpga_fd, reg_delay_high, __tmp)){	\
		return 0;									\
	}												\
}while(0)
#endif

/*
  1	成功
  0	失败
*/
static int SATCOMMIssueConfig(struct inCtx *ctx, int slot)
{
	int fpga_fd = -1;
	int event_id = 0;
	struct satcommoninfo *cfg = NULL;

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->inSta[slot-1].satcommon.satcomminfo);
	if(!cfg)
		return 0;

	u16_t san = 0;

	if ((ctx->inSta[slot-1].satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_LON) && \
		(ctx->inSta[slot-1].satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_LAT) && \
		(ctx->inSta[slot-1].satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_ELEV)
		){
		//support 6, no support bei bfi bfei bffi
		if(0 == lola_elev_set(fpga_fd, slot, cfg->lon, cfg->lat, cfg->elev)){//海拔、经纬设置
			print(	DBG_ERROR, 
					"--Failed to set lola elev.");

			return 0;
		}
	}

	if (ctx->inSta[slot-1].satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_SYSMODE){
		//support 6, no support bei bfi bfei bffi
		if(0 == mode_set(fpga_fd, slot, cfg->sysMode)){//接收机工作模式设置
			print(	DBG_ERROR, 
					"--Failed to set mode.");

			return 0;
		}
		//move to proc
		//INIT_EVNTFLAG_BY_SLOT_PORT_GRP(SMP_EVNT_GRP_SATE_MODSET, slot, SAT_PORT_SAT, 0);
		switch (cfg->sysMode){
			case MODE_CURSOR_GPS_VAL:
				event_id = SNMP_EVNT_ID_SAT_GPS;
				break;
			case MODE_CURSOR_BD_VAL:
				event_id = SNMP_EVNT_ID_SAT_BD;
				break;
			case MODE_CURSOR_MIX_GPS_VAL:
				event_id = SNMP_EVNT_ID_SAT_MGPS;
				break;
			case MODE_CURSOR_MIX_BD_VAL:
				event_id = SNMP_EVNT_ID_SAT_MBD;
				break;
		}
		//event
		//SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(event_id, slot, SAT_PORT_SAT);
		SET_EVNTFLAGVALID_BID_BY_SLOT_PORT_ID(event_id, slot, SAT_PORT_SAT, ctx->inSta[slot-1].boardId);
	}

	if (ctx->inSta[slot-1].satcommon.satcomminfo.info_flag & FLAG_SAT_COMMINFO_MB_SA){
		//设置SA字节
		//no support bei bfi
		if(0 == memcmp(cfg->mb_sa, gSaTbl[SA_CURSOR_SA4], sizeof(gSaTbl[SA_CURSOR_SA4])))
		{
			san = 0x01;
		}
		else if(0 == memcmp(cfg->mb_sa, gSaTbl[SA_CURSOR_SA5], sizeof(gSaTbl[SA_CURSOR_SA5])))
		{
			san = 0x02;
		}
		else if(0 == memcmp(cfg->mb_sa, gSaTbl[SA_CURSOR_SA6], sizeof(gSaTbl[SA_CURSOR_SA6])))
		{
			san = 0x03;
		}
		else if(0 == memcmp(cfg->mb_sa, gSaTbl[SA_CURSOR_SA7], sizeof(gSaTbl[SA_CURSOR_SA7])))
		{
			san = 0x04;
		}
		else if(0 == memcmp(cfg->mb_sa, gSaTbl[SA_CURSOR_SA8], sizeof(gSaTbl[SA_CURSOR_SA8])))
		{
			san = 0x05;
		}
		else
		{
			print(	DBG_ERROR, 
					"<%s>--Invalid SA.", 
					gDaemonTbl[DAEMON_CURSOR_INPUT]);

			return 0;
		}
			//设置SA字节
		// sa
		if(!FpgaWrite(fpga_fd, FPGA_IN_2MB_SA(slot), san))
		{
			return 0;
		}
	}

	return 1;
}

#if 1
/*适配国外PTP模块*/
#define PTP_DELAY_TIME	100000


#define PTPIN_LOGIN_USER	"root"
#define PTPIN_LOGIN_PASSWD	"root"

/*
  1	成功
  0	失败
*/
static int PtpSendConfig(int fpga_fd, int slot, struct ptpin_info *cfg)
{
	int i;
	u16_t len = 0;
	u16_t tmp = 0;
	char buf[128];
	char ptpCmd[2048];
	char str_ipv4[MAX_IP_LEN];

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
	sprintf(buf, "%s\n", PTPIN_LOGIN_USER);
	tmp = strlen(buf)+1;
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s\n", PTPIN_LOGIN_PASSWD);
	tmp = strlen(buf)+1;
	if(0 == p50_write(fpga_fd, slot, (u8_t *)buf, tmp))
	{
		return 0;
	}
	usleep(PTP_DELAY_TIME);


	#endif

	
	
    if(LAYER3 == cfg->ptpLayer && UNICAST == cfg->ptpMulticast)
    {
        //ptp2 stop engine
      	tmp = strlen(P50_CMD_STOP)+1;
		memcpy(ptpCmd +len, P50_CMD_STOP, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		//usleep(PTP_DELAY_TIME);
		usleep(PTP_DELAY_TIME);
		len += tmp;

		
		//
		//! the ptp ip change result as ptp communicatin error(layer3 unicast).
		//
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
	
		//mask
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_FILL_MASK(buf, cfg->ptpMask);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	
		//gateway
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_FILL_GATEWAY(buf, cfg->ptpGateway);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(8*PTP_DELAY_TIME);
		len += tmp;
      
		
		//ptp2 slave
      	tmp = strlen(P50_CMD_SLAVE)+1;
		memcpy(ptpCmd +len, P50_CMD_SLAVE, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
		
        //ptp2 vco config
      	tmp = strlen(P50_CMD_VCO)+1;
		memcpy(ptpCmd +len, P50_CMD_VCO, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		//usleep(PTP_DELAY_TIME);
		usleep(PTP_DELAY_TIME);
		len += tmp;
		
		//ptp2 clock mode slave
      	tmp = strlen(P50_CMD_CLOCK_SLAVE)+1;
		memcpy(ptpCmd +len, P50_CMD_CLOCK_SLAVE, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//ptp2 clock domain
		tmp = strlen(P50_CMD_DOMAIN_24)+1;
		memcpy(ptpCmd +len, P50_CMD_DOMAIN_24, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//ptp2 tod format
		tmp = strlen(P50_CMD_OUT_CM_TOD)+1;
		memcpy(ptpCmd +len, P50_CMD_OUT_CM_TOD, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//pps out mode 1 sync
		tmp = strlen(P50_CMD_OUT_PT_M1)+1;
		memcpy(ptpCmd +len, P50_CMD_OUT_PT_M1, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//pps out tod baudrate
		tmp = strlen(P50_CMD_OUT_B9600)+1;
		memcpy(ptpCmd +len, P50_CMD_OUT_B9600, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//pps start
		tmp = strlen(P50_CMD_OUT_1PPS_TOD)+1;
		memcpy(ptpCmd +len, P50_CMD_OUT_1PPS_TOD, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//slave discovery durable time
		tmp = strlen(P50_CMD_DURATION_INTER)+1;
		memcpy(ptpCmd +len, P50_CMD_DURATION_INTER, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//slave discovery inter time
		tmp = strlen(P50_CMD_DISCOVERY_INTER)+1;
		memcpy(ptpCmd +len, P50_CMD_DISCOVERY_INTER, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	}
	else
	{
        //
		//! the ptp ip change result as ptp communicatin error(layer3 unicast).
		//
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
	
		//mask
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_FILL_MASK(buf, cfg->ptpMask);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
	
		//gateway
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_FILL_GATEWAY(buf, cfg->ptpGateway);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;
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
		tmp = strlen(P50_CMD_UNICAST)+1;
		memcpy(ptpCmd +len, P50_CMD_UNICAST, tmp);
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
			for(i = 0; i < cfg->oldptpMasterIp_num; i++)	
			#else
			//support set mul master point
			for(i = 0; i < cfg->ptpMasterIp_num; i++)
			#endif
			{
				memset(str_ipv4, 0x0, sizeof(str_ipv4));
				#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
				UINTIP_TO_STRIP(cfg->oldptpMasterIp[i], str_ipv4);
				#else
				UINTIP_TO_STRIP(cfg->ptpMasterIp[i], str_ipv4);
				#endif
				P50_DEL_MASTER_IP(buf, str_ipv4);

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
			#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
			P50_DEL_MASTER_MAC(buf, cfg->oldptpMasterMac);
			#else
			P50_DEL_MASTER_MAC(buf, cfg->ptpMasterMac);
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
		}

		//ip_to_hex((char *)cfg->ptpSlaveIp);
		if(LAYER3 == cfg->ptpLayer){
			for(i = 0; i < cfg->ptpMasterIp_num; i++){
				memset(str_ipv4, 0x0, sizeof(str_ipv4));
				UINTIP_TO_STRIP(cfg->ptpMasterIp[i], str_ipv4);
				#ifdef SUPPORT_ADD_NODE_PRIO_SYNC_DELAY
				P50_FILL_MASTER_IP_PSD(buf, str_ipv4, cfg->ptpPriority, -8 + cfg->ptpSync, -8 + cfg->ptpdelayReq);
				#else
				P50_FILL_MASTER_IP(buf, str_ipv4);
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
			memset(cfg->oldptpMasterIp, 0x0, sizeof(cfg->oldptpMasterIp));
			memcpy(cfg->oldptpMasterIp, cfg->ptpMasterIp, sizeof(cfg->ptpMasterIp));
			cfg->oldptpMasterIp_num = cfg->ptpMasterIp_num;
			#endif
		}else{ //LAYER2
			#ifdef SUPPORT_ADD_NODE_PRIO_SYNC_DELAY
			P50_FILL_MASTER_MAC_PSD(buf, cfg->ptpMasterMac, cfg->ptpPriority, -8 + cfg->ptpSync,-8 + cfg->ptpdelayReq);
			#else
			P50_FILL_MASTER_MAC(buf, cfg->ptpMasterMac);
			#endif
			#ifdef SUPPORT_ONLY_ONE_GROUP_NODE
			memset(cfg->oldptpMasterMac, 0x0, MAX_MAC_LEN);
			memcpy(cfg->oldptpMasterMac, cfg->ptpMasterMac, MAX_MAC_LEN);
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

	//delay req
	memset(buf, 0, sizeof(buf));
	P50_FILL_DELAY_REQ(buf, -8 +cfg->ptpdelayReq);
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

	#if 1
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
	#endif

	/*ip地址必须向设置，如果P50没有设置ip地址情况下不能正常启动*/
	//ip
    //2019-05-21
	/*
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

		//mask
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_FILL_MASK(buf, cfg->ptpMask);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;

		//gateway
		memset(buf, 0, sizeof(buf));
		//ip_to_hex((char *)cfg->ptpIp);
		P50_FILL_GATEWAY(buf, cfg->ptpGateway);
		//printf("%s\n",buf);
		tmp = strlen(buf)+1;
		memcpy(ptpCmd +len, buf, tmp);
		if(0 == p50_write(fpga_fd, slot, (u8_t *)(ptpCmd +len), tmp))
		{
				return 0;
		}
		usleep(PTP_DELAY_TIME);
		len += tmp;*/
	

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
static int PTPINIssueConfig(struct inCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct ptpin_info *cfg = NULL;

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE || slot <= 0)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->inSta[slot-1].ptpin.ptpinInfo);
	if(!cfg)
		return 0;

	if(1 != PtpSendConfig(fpga_fd, slot, cfg)) {
		return 0;
	}

	return 1;
}
#endif

/*
COMMON
  1	成功
  0	失败
*/
static int INCOMMIssueConfig(struct inCtx *ctx, int slot)
{
	int fpga_fd = -1;
	struct incommoninfo *cfg = NULL;

	if(slot > INPUT_SLOT_CURSOR_ARRAY_SIZE)
		return 0;

	fpga_fd = ctx->fpga_fd;
	cfg = &(ctx->inSta[slot-1].incommsta.incomminfo);
	if(!cfg)
		return 0;

	// 设置延时补偿
	int max_port = 0;
	int port = 0;
	struct port_attr_t *port_attr = NULL;
	FIND_PORTATTR_GBATBL_BY_BID(ctx->inSta[slot-1].boardId, port_attr);
	if(!port_attr)
		return 1;
	FIND_MAXPORT_GBATBL_BY_BID(ctx->inSta[slot-1].boardId, max_port);
	
	for (port = 0; port < max_port; ++port){
		switch (port_attr[port].insrc_type){
			case GPS_SOURCE_TYPE:
				// port 1
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_GB_DELAY_LOW16(slot), FPGA_IN_GB_DELAY_HIGH12(slot),
						ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
			break;
			case S2MH_SOURCE_TYPE:
				// port 2
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_2MH_DELAY_LOW16(slot), FPGA_IN_2MH_DELAY_HIGH12(slot),
						ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case S2MB_SOURCE_TYPE:
				//port 3
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_2MB_DELAY_LOW16(slot), FPGA_IN_2MB_DELAY_HIGH12(slot),
						ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case IRIGB1_SOURCE_TYPE:
				// port 4
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_IRIGB1_DELAY_LOW16(slot), FPGA_IN_IRIGB1_DELAY_HIGH12(slot),
						ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case IRIGB2_SOURCE_TYPE:
				// port 5
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_IRIGB2_DELAY_LOW16(slot), FPGA_IN_IRIGB2_DELAY_HIGH12(slot),
						ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case TOD_SOURCE_TYPE:
				//support rtf port1, ptp_in port
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_TOD_DELAY_LOW16(slot), FPGA_IN_TOD_DELAY_HIGH12(slot),
				ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case NTP_SOURCE_TYPE:
				//support NTP port1
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_NTP_IN_DELAY_LOW16(slot), FPGA_IN_NTP_IN_DELAY_HIGH12(slot),
				ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case S10MH1_SOURCE_TYPE:
				//support rtf port2
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_10MH1_DELAY_LOW16(slot), FPGA_IN_10MH1_DELAY_HIGH12(slot),
				ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case S10MH2_SOURCE_TYPE:
				//support rtf port3
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_10MH2_DELAY_LOW16(slot), FPGA_IN_10MH2_DELAY_HIGH12(slot),
				ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
			case S1PPS_SOURCE_TYPE:
				//support rtf port4
				SET_SIGNAL_INDELAY(fpga_fd, FPGA_IN_1PPS_DELAY_LOW16(slot), FPGA_IN_1PPS_DELAY_HIGH12(slot),
				ctx->inSta[slot-1].incommsta.incomminfo.delay[port]);
				break;
		}
	}

	return 1;
}


int IssueConfig(struct inCtx *ctx, int slot, int bid, int set_which)

{

	int i;
	print(DBG_INFORMATIONAL, "issueConfig set_which: %d", set_which);
	if(set_which == INPUTINFO_SET_PRIV_INFO){
		goto priv;
	}

//incomm:
	if (0 == INCOMMIssueConfig(ctx, slot))
		return 0;

	if(set_which == INPUTINFO_SET_INCOMM_INFO){
		goto exit_sc;
	}

priv:
	 for (i = 0; i < LEN_GISSUECONFIG; ++i){
		if(gIssueConfig[i].boardId == bid){
			switch (bid)
			{
				case BID_PTP_IN:

					sleep(5);
					break;
			}
			if(0 == gIssueConfig[i].IssueConfigFun(ctx, slot)){
				return 0;
			}
			switch (bid)
			{
				case BID_INSATE_LOWER ... BID_INSATE_UPPER:
					//wait sate config success...
					sleep(3);
					break;
			}
			break;
		}
	}
exit_sc:
	return 1;

}

/*
  1	成功
  0	失败
*/
int IssueLeapForecast(struct leapinfo *lp)
{
	int ret = 0;
	struct ipcinfo ipc;
	struct config update;
	struct msgbuf msg;
	struct pidinfo pid;

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_OUTPUT);
	if(-1 == ipc.ipc_msgq_id)
	{
		return 0;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_OUTPUT);
	if(-1 == ipc.ipc_shm_id)
	{
		return 0;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		return 0;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		goto quit;
	}
	memset(&update, 0, sizeof(struct config));
	memcpy(&update.leap_config, lp, sizeof(struct leapinfo));//闰秒和闰秒预告

	memset(&msg, 0, sizeof(struct msgbuf));
	msg.mtype = MSG_CFG_LPS;//闰秒
	memcpy(msg.mdata, update.buffer, sizeof(struct leapinfo));

	if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		goto quit;
	}
	shm_read(ipc.ipc_base, 
			 SHM_OFFSET_PID, 
			 sizeof(struct pidinfo), 
			 (char *)&pid, 
			 sizeof(struct pidinfo));
	if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		goto quit;
	}
	
	if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct leapinfo)))//发送闰秒和闰秒预告
	{
		if(notify(pid.p_id, NOTIFY_SIGNAL))//发送NOTIFY_SIGNAL信号
		{
			ret = 1;
			goto quit;
		}
		else
		{
			goto quit;
		}
	}
	else
	{
		goto quit;
	}

quit:
	if( 1 == shm_detach(ipc.ipc_base) )
		ipc.ipc_base = (void *)-1;
	return ret;
}

