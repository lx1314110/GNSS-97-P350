#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "lib_msgQ.h"
#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_common.h"

#include "alloc.h"
#include "cout_global.h"
#include "cout_set.h"


extern u8_t gtraceTbl[PTP_TRACE_ARRAY_SIZE][2];






/*
  -1	失败
   0	成功
*/
int in_zone_set(struct outCtx *ctx, char *zone_grp, int group)
{
	int i,j = 0;
	int len = 0;
	int found = 0;
	char * tmp = NULL;
	char buf[ZONE_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = ":";
	struct config cfg;
	struct msgbuf msg;
	struct inzoneinfo tz;

	if(strlen(zone_grp)>=ZONE_BUF_LEN || strlen(zone_grp)<= 0)
		return -1;
	if(group < 0 || group > 4)
		return -1;
	memset(buf,0x0,  sizeof(buf));
	memset(&cfg, 0, sizeof(struct config));
		
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_IZONE, 
			 sizeof(struct inzoneinfo), 
			 (char *)&tz, 
			 sizeof(struct inzoneinfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	//re-read backup
	memcpy(cfg.in_zone_config.zone, tz.zone, 4);

	if(group == 0){
		//all set
		memcpy(buf, zone_grp, strlen(zone_grp));
		tmp = strtok_r(buf, separator, &saveptr);
		if(!tmp)
			return -1;
		j=0;
		while (NULL != tmp){
			found = 0;
			for(i=0; i<ARRAY_SIZE(gZoneTbl); i++)
			{	
				len = (strlen(tmp) > strlen((const char *)gZoneTbl[i])) ? strlen(tmp) : strlen((const char *)gZoneTbl[i]);
				if(0 == memcmp(tmp, gZoneTbl[i], len))
				{
					found = 1;
					break;
				}
			}
			if(found)
				cfg.in_zone_config.zone[j++] = i+'@';

			tmp = strtok_r(NULL, separator, &saveptr);
		}
		if(!found)
			return -1;

	}else{
		found = 0;
		for(i=0; i<ARRAY_SIZE(gZoneTbl); i++)
		{	
			len = (strlen(zone_grp) > strlen((const char *)gZoneTbl[i])) ? strlen(zone_grp) : strlen((const char *)gZoneTbl[i]);
			if(0 == memcmp(zone_grp, gZoneTbl[i], len))
			{
				found = 1;
				break;
			}
		}	
		if(found)
			cfg.in_zone_config.zone[group-1] = i+'@';
		else
			return -1;

	}
	
	cfg.in_zone_config.zone[4] = '\0';
	
	msg.mtype = MSG_CFG_IZONE;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct inzoneinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct inzoneinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}






/*
  -1	失败
   0	成功
   四组
*/
int out_zone_set(struct outCtx *ctx, char *zone_grp, int group)
{
	int i,j = 0;
	int len = 0;
	int found = 0;
	char * tmp = NULL;
	char buf[ZONE_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = ":";
	struct config cfg;
	struct msgbuf msg;
	struct outzoneinfo tz;

	if(strlen(zone_grp)>=ZONE_BUF_LEN || strlen(zone_grp)<= 0)
		return -1;
	if(group < 0 || group > 4)
		return -1;
	memset(buf,0x0,  sizeof(buf));
	memset(&cfg, 0, sizeof(struct config));
		
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_OZONE, 
			 sizeof(struct outzoneinfo), 
			 (char *)&tz, 
			 sizeof(struct outzoneinfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	//re-read backup
	memcpy(cfg.out_zone_config.zone, tz.zone, 4);


	if(group == 0){
		//all set
		memcpy(buf, zone_grp, strlen(zone_grp));
		tmp = strtok_r(buf, separator, &saveptr);
		if(!tmp)
			return -1;
		j=0;
		while (NULL != tmp){
			found = 0;
			for(i=0; i<ARRAY_SIZE(gZoneTbl); i++)
			{	
				len = (strlen(tmp) > strlen((const char *)gZoneTbl[i])) ? strlen(tmp) : strlen((const char *)gZoneTbl[i]);
				if(0 == memcmp(tmp, gZoneTbl[i], len))
				{
					found = 1;
					break;
				}
			}
			if(found)
				cfg.out_zone_config.zone[j++] = i+'@';

			tmp = strtok_r(NULL, separator, &saveptr);
		}
		if(!found)
			return -1;

	}else{
		found = 0;
		for(i=0; i<ARRAY_SIZE(gZoneTbl); i++)
		{	
			len = (strlen(zone_grp) > strlen((const char *)gZoneTbl[i])) ? strlen(zone_grp) : strlen((const char *)gZoneTbl[i]);
			if(0 == memcmp(zone_grp, gZoneTbl[i], len))
			{
				found = 1;
				break;
			}
		}	
		if(found)
			cfg.out_zone_config.zone[group-1] = i+'@';
		else
			return -1;

	}
	
	cfg.out_zone_config.zone[4] = '\0';
	
	msg.mtype = MSG_CFG_OZONE;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct outzoneinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct outzoneinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}



/*
  -1	失败
   0	成功
*/
int out_2mb_sa_set(struct outCtx *ctx, u8_t *sa)
{
	struct config cfg;
	struct msgbuf msg;
	//struct out2mbsainfo obsa;

	if(!sa)
		return -1;
	memset(&cfg, 0, sizeof(struct config));

	#if 0
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_OBSA, 
			 sizeof(struct out2mbsainfo), 
			 (char *)&obsa, 
			 sizeof(struct out2mbsainfo));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	//re-read backup
	#endif
	memcpy(cfg.out_sa.mb_sa, sa, strlen((const char *)sa));

	msg.mtype = MSG_CFG_OBSA;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct out2mbsainfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct out2mbsainfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}









/*
  -1	失败
   0	成功
*/
int out_baudrate_set(struct outCtx *ctx, char *br_grp, int group)
{
	int i,j = 0;
	int len = 0;
	int found = 0;
	char * tmp = NULL;
	char buf[BAUDRATE_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = ":";
	struct config cfg;
	struct msgbuf msg;
	struct baudrate baud;

	if(strlen(br_grp)>=BAUDRATE_BUF_LEN || strlen(br_grp)<= 0)
			return -1;
	if(group < 0 || group > 4)
		return -1;
	memset(buf,0x0,  sizeof(buf));
	memset(&cfg, 0, sizeof(struct config));
		
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_OBR, 
			 sizeof(struct baudrate), 
			 (char *)&baud, 
			 sizeof(struct baudrate));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	//re-read backup
	memcpy(cfg.out_br.br, baud.br, 4);


	if(group == 0){
		//all set
		memcpy(buf, br_grp, strlen(br_grp));
		tmp = strtok_r(buf, separator, &saveptr);
		if(!tmp)
			return -1;
		j=0;
		while (NULL != tmp){
			found = 0;
			for(i=0; i<ARRAY_SIZE(gBaudRateTbl); i++)
			{	
				len = (strlen(tmp) > strlen((const char *)gBaudRateTbl[i])) ? strlen(tmp) : strlen((const char *)gBaudRateTbl[i]);
				if(0 == memcmp(tmp, gBaudRateTbl[i], len))
				{
					found = 1;
					break;
				}
			}
			if(found)
				cfg.out_br.br[j++] = i;

			tmp = strtok_r(NULL, separator, &saveptr);
		}
		if(!found)
			return -1;

	}else{
		found = 0;
		for(i=0; i<ARRAY_SIZE(gBaudRateTbl); i++)
		{	
			len = (strlen(br_grp) > strlen((const char *)gBaudRateTbl[i])) ? strlen(br_grp) : strlen((const char *)gBaudRateTbl[i]);
			if(0 == memcmp(br_grp, gBaudRateTbl[i], len))
			{
				found = 1;
				break;
			}
		}
		if(found)
			cfg.out_br.br[group-1] = i;
		else
			return -1;

	}

	cfg.out_br.br[4] = '\0';

	msg.mtype = MSG_CFG_OBR;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct baudrate));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct baudrate)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}













/*
  -1	失败
   0	成功
*/
int irigb_amplitude_ratio_set(struct outCtx *ctx, char *iar_grp, int group)
{
	int i,j = 0;
	int len = 0;
	int found = 0;
	char * tmp = NULL;
	char buf[IAR_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = "|";
	struct config cfg;
	struct msgbuf msg;
	struct amplitude ampltd;

	if(strlen(iar_grp)>=IAR_BUF_LEN || strlen(iar_grp)<= 0)
		return -1;
	if(group < 0 || group > 4)
		return -1;
	memset(buf,0x0,  sizeof(buf));
	memset(&cfg, 0, sizeof(struct config));

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_AMP, 
			 sizeof(struct amplitude), 
			 (char *)&ampltd, 
			 sizeof(struct amplitude));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	//re-read backup
	memcpy(cfg.irigb_amp.amp, ampltd.amp, 4);


	if(group == 0){
		//all set
		memcpy(buf, iar_grp, strlen(iar_grp));
		tmp = strtok_r(buf, separator, &saveptr);
		if(!tmp)
			return -1;
		j=0;
		while (NULL != tmp){
			found = 0;
			for(i=0; i<ARRAY_SIZE(gAmplitudeRatioTbl); i++)
			{	
				len = (strlen(tmp) > strlen((const char *)gAmplitudeRatioTbl[i])) ? strlen(tmp) : strlen((const char *)gAmplitudeRatioTbl[i]);
				if(0 == memcmp(tmp, gAmplitudeRatioTbl[i], len))
				{
					found = 1;
					break;
				}
			}
			if(found)
				cfg.irigb_amp.amp[j++] = i + 3; //fgpa start point 3

			tmp = strtok_r(NULL, separator, &saveptr);
		}
		if(!found)
			return -1;

	}else{
		found = 0;
		for(i=0; i<ARRAY_SIZE(gAmplitudeRatioTbl); i++)
		{	
			len = (strlen(iar_grp) > strlen((const char *)gAmplitudeRatioTbl[i])) ? strlen(iar_grp) : strlen((const char *)gAmplitudeRatioTbl[i]);
			if(0 == memcmp(iar_grp, gAmplitudeRatioTbl[i], len))
			{
				found = 1;
				break;
			}
		}
		if(found)
			cfg.irigb_amp.amp[group-1] = i + 3; //fgpa start point 3
		else
			return -1;

	}

	cfg.irigb_amp.amp[4] = '\0';


	msg.mtype = MSG_CFG_AMP;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct amplitude));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct amplitude)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}














/*
  -1	失败
   0	成功
*/
int irigb_voltage_set(struct outCtx *ctx, char *iv_grp, int group)
{
	int i,j = 0;
	int len = 0;
	int found = 0;
	char * tmp = NULL;
	char buf[IV_BUF_LEN];
	char *saveptr = NULL;
	const char *separator = "|";
	struct config cfg;
	struct msgbuf msg;
	struct voltage voltg;

	if(strlen(iv_grp)>=IV_BUF_LEN || strlen(iv_grp)<= 0)
			return -1;
	if(group < 0 || group > 4)
		return -1;
	memset(buf,0x0,  sizeof(buf));
	memset(&cfg, 0, sizeof(struct config));

	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}
	
	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_VOL, 
			 sizeof(struct voltage), 
			 (char *)&voltg, 
			 sizeof(struct voltage));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	//re-read backup
	memcpy(cfg.irigb_vol.vol, voltg.vol, 4);


	if(group == 0){
		//all set
		memcpy(buf, iv_grp, strlen(iv_grp));
		tmp = strtok_r(buf, separator, &saveptr);
		if(!tmp)
			return -1;
		j=0;
		while (NULL != tmp){
			found = 0;
			for(i=0; i<ARRAY_SIZE(gVoltageTbl); i++)
			{	
				len = (strlen(tmp) > strlen((const char *)gVoltageTbl[i])) ? strlen(tmp) : strlen((const char *)gVoltageTbl[i]);
				if(0 == memcmp(tmp, gVoltageTbl[i], len))
				{
					found = 1;
					break;
				}
			}
			if(found)
				cfg.irigb_vol.vol[j++] = i;

			tmp = strtok_r(NULL, separator, &saveptr);
		}
		if(!found)
			return -1;

	}else{
		found = 0;
		for(i=0; i<ARRAY_SIZE(gVoltageTbl); i++)
		{	
			len = (strlen(iv_grp) > strlen((const char *)gVoltageTbl[i])) ? strlen(iv_grp) : strlen((const char *)gVoltageTbl[i]);
			if(0 == memcmp(iv_grp, gVoltageTbl[i], len))
			{
				found = 1;
				break;
			}
		}
		if(found)
			cfg.irigb_vol.vol[group-1] = i;
		else
			return -1;

	}

	cfg.irigb_vol.vol[4] = '\0';


	msg.mtype = MSG_CFG_VOL;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct voltage));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct voltage)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}








/*
ip1,ip2,ip3...ip16
192.168.1.45,192.168.2.55,...;
  0	不合法
  1 合法
*/
static int str_ip_group_to_uint32(char *ipv4_grp, u32_t *des_ipv4, u8_t * des_ipv4_num)
{
	#if 0
	if( INADDR_NONE == inet_addr((const char *)ip) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
	#endif
	int i,j;
	//int found = 0;
	char *ptr = NULL;
	char *saveptr = NULL;
	char t_ipv4_grp[MAX_IP_LEN*MAX_IP_NUM+1];
	u32_t t_des_ipv4[MAX_IP_NUM];
	struct in_addr addr;
    if(ipv4_grp == NULL || strlen(ipv4_grp)<7 || strlen(ipv4_grp) > MAX_IP_LEN*MAX_IP_NUM)
        return 0;
	memset(t_ipv4_grp, 0x0, sizeof(t_ipv4_grp));

	//del " ","\n", "\r"
	for (i = 0, j = 0; i < strlen(ipv4_grp); ++i){
		if(ipv4_grp[i] == ' ' || ipv4_grp[i] == '\n'
			|| ipv4_grp[i] == '\r' || ipv4_grp[i] == '\t')
			continue;
		t_ipv4_grp[j++] = ipv4_grp[i];
	}

	ptr = strtok_r(t_ipv4_grp, ",", &saveptr);
	if(!ptr)
		return 0;

	i = 0;
	memset(t_des_ipv4, 0x0, sizeof(t_des_ipv4));
	do {
		//found = 0;
		//printf("ptr:%s\n", ptr);
		if(inet_pton(AF_INET, ptr, (void *)&addr) != 1){
   			return 0;
		}
		t_des_ipv4[i] = ntohl(addr.s_addr);
		//printf("ptr:%s\tpton:%08x\taddr:%08x\thost:%08x\thost2:%08x\n", ptr, 
		//addr.s_addr,inet_addr(ptr),inet_network(ptr), ntohl(inet_addr(ptr)));
		i++;
	}while((ptr = strtok_r(NULL, ",", &saveptr)) != NULL);
	if(i > MAX_IP_NUM)
		return 0;
	*des_ipv4_num = i;
	memcpy(des_ipv4, t_des_ipv4, (*des_ipv4_num) * sizeof(u32_t));
	//printf("num:%d\n", *des_ipv4_num);
	return 1;

}

//#test##############
#if 0
static int uint32_ip_group_to_str_ip(u32_t *ipv4, u8_t ipv4_num, char *des_ipv4_grp)
{
	int i;
	char str_ip[MAX_IP_LEN];
	char str_ip_group[MAX_IP_LEN*MAX_IP_NUM+1];//注意栈溢出问题，必要时用static

	if(ipv4 == NULL || des_ipv4_grp == NULL || ipv4_num > MAX_IP_NUM)
        return 0;
	memset(str_ip_group, 0x0, sizeof(str_ip_group));
	for (i = 0; i < ipv4_num; ++i){
		//if(sta.ptpin.ptpinInfo.ptpMasterIp[i] == 0)
			//continue;
		memset(str_ip, 0x0, MAX_IP_LEN);
		UINTIP_TO_STRIP(ipv4[i], str_ip);
		sprintf(str_ip_group, "%s%s,", str_ip_group, str_ip);
	}
	if(strlen(str_ip_group)>0)
		str_ip_group[strlen(str_ip_group)-1] = '\0';
	memcpy(des_ipv4_grp, str_ip_group, strlen(str_ip_group));
	return 1;
}
#endif
//####################




/*
  -1	失败
   0	成功
*/
int output_board_set(struct outCtx *ctx, struct outsetinfo *ds)
{
	int board_type = BOARD_TYPE_NONE;
	int supported_signal = BIT(SIGNAL_NONE);
	struct outputsta sta;
	struct config tmp;
	struct msgbuf msg;
	
	u8_t id_pdt = 0;
	u8_t id_pmut = 0;
	u8_t id_signal = 0;
	//u8_t id_pmodel = 0;
	u8_t id_plt = 0;
	u8_t id_pst = 0;
	u8_t id_psf = 0;
	u8_t id_paf = 0;
	//u8_t id_pdrf = 0;
	u8_t id_ppdrf = 0;
	u8_t id_pssm = 0;
	int len = 0;

	memset(&tmp, 0, sizeof(struct config));
	//default priv
	tmp.out_config.set_which = OUTPUTINFO_SET_PRIV_INFO;
//ptp all config not have delay and signal, ps: p350out 11 delay 10 ;p350out 11 signal ptp
	if(ds->set_delay_flag)
	{
		tmp.out_config.set_which = OUTPUTINFO_SET_OUTCOMM_INFO;
	}

	if(ds->set_signal_type_flag)
	{
		tmp.out_config.set_which = OUTPUTINFO_SET_OUTCOMM_INFO;
		for(id_signal=0; id_signal<SIGNAL_ARRAY_SIZE; id_signal++)
		{
			if(0 == memcmp(ds->signal, gSignalTbl[id_signal], strlen((const char *)gSignalTbl[id_signal])))
			{
				break;
			}
		}
	}
	
	if(ds->set_ptp_delay_type_flag)
	{
		for(id_pdt=0; id_pdt<PTP_DELAY_ARRAY_SIZE; id_pdt++)
		{
			if(0 == memcmp(ds->ptpDelayType, gPdtTbl[id_pdt], strlen((const char *)gPdtTbl[id_pdt])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_multi_uni_type_flag)
	{
		for(id_pmut=0; id_pmut<PTP_MULTI_UNI_ARRAY_SIZE; id_pmut++)
		{
			if(0 == memcmp(ds->ptpMulticast, gPmutTbl[id_pmut], strlen((const char *)gPmutTbl[id_pmut])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_layer_type_flag)
	{
		for(id_plt=0; id_plt<PTP_LAYER_ARRAY_SIZE; id_plt++)
		{
			if(0 == memcmp(ds->ptpLayer, gPltTbl[id_plt], strlen((const char *)gPltTbl[id_plt])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_step_type_flag)
	{
		for(id_pst=0; id_pst<PTP_STEP_ARRAY_SIZE; id_pst++)
		{
			if(0 == memcmp(ds->ptpStep, gPstTbl[id_pst], strlen((const char *)gPstTbl[id_pst])))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_sync_freq_flag)
	{
		for(id_psf=0; id_psf<ARRAY_SIZE(gFreqTbl); id_psf++)
		{
			len = (strlen((const char *)ds->ptpSync) > strlen((const char *)gFreqTbl[id_psf])) ? strlen((const char *)ds->ptpSync) : strlen((const char *)gFreqTbl[id_psf]);
			if(0 == memcmp(ds->ptpSync, gFreqTbl[id_psf], len))
			{
				break;
			}
		}
	}

	if(ds->set_ptp_announce_freq_flag)
	{
		for(id_paf=0; id_paf<ARRAY_SIZE(gFreqTbl); id_paf++)
		{
			len = (strlen((const char *)ds->ptpAnnounce) > strlen((const char *)gFreqTbl[id_paf])) ? strlen((const char *)ds->ptpAnnounce) : strlen((const char *)gFreqTbl[id_paf]);
			if(0 == memcmp(ds->ptpAnnounce, gFreqTbl[id_paf], len))
			{
				break;
			}
		}
	}

	
	if(ds->set_ptp_pdelay_req_freq_flag)
	{
		for(id_ppdrf=0; id_ppdrf<ARRAY_SIZE(gFreqTbl); id_ppdrf++)
		{
			len = (strlen((const char *)ds->ptpPdelayReq) > strlen((const char *)gFreqTbl[id_ppdrf])) ? strlen((const char *)ds->ptpPdelayReq) : strlen((const char *)gFreqTbl[id_ppdrf]);
			if(0 == memcmp(ds->ptpPdelayReq, gFreqTbl[id_ppdrf], len))
			{
				break;
			}
		}
	}
	
	if(ds->set_ptp_ssm_flag)
	{
		for(id_pssm=0; id_pssm<PTP_TRACE_ARRAY_SIZE; id_pssm++)
		{
			if(0 == memcmp(ds->ptpssm, gtraceTbl[id_pssm], strlen((const char *)gtraceTbl[id_pssm])))
			{
				break;
			}
		}
	}
	
	
	if(1 != sema_lock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	shm_read(ctx->ipc.ipc_base, 
			 SHM_OFFSET_STA +(ds->slot-1)*sizeof(struct outputsta), 
			 sizeof(struct outputsta), 
			 (char *)&sta, 
			 sizeof(struct outputsta));
	
	if(1 != sema_unlock(ctx->ipc.ipc_sem_id, SEMA_MEMBER_OUTPUT))
	{
		return -1;
	}

	tmp.out_config.slot = ds->slot;
	tmp.out_config.boardId = sta.boardId;
	tmp.out_config.nPort = 1;


	switch (sta.boardId){
		//these output board no support set outcommon
		//case BID_NTP:
		//case BID_NTPF://by supported_signal filter
		case BID_61850:
			return -1;
			break;
	}

	if(tmp.out_config.set_which == OUTPUTINFO_SET_OUTCOMM_INFO){//outcommon config
		FIND_BDTYPE_GBATBL_BY_BID(sta.boardId, board_type);
		
		if(board_type == BOARD_TYPE_OUTPUT){
			//backup the orgin config
			memcpy(&tmp.out_config.outcomminfo, &sta.outcomminfo[0], sizeof(struct outcommoninfo));
			if(ds->set_delay_flag)
				tmp.out_config.outcomminfo.delay = ds->delay;//should no use it,  current use delay of signal
			if(ds->set_signal_type_flag){
				FIND_SUP_SIGNAL_GBATBL_BY_BID(sta.boardId, supported_signal);
				if(supported_signal & BIT(id_signal))
					tmp.out_config.outcomminfo.signalType = id_signal;
				else
					return -1;//no support current signal.
			}
		}else{
			return -1;
		}
	}else{ //priv configure

		if(BID_PTP == sta.boardId)
		{
			memcpy(&tmp.out_config.ptp, &sta.ptp[0], sizeof(struct ptpinfo));

			if(ds->set_ptptype_flag)
				tmp.out_config.ptp.ptpType = ds->ptptype;
			#if 0
			if(ds->set_delay_flag)
				tmp.out_config.ptp.ptpDelay = ds->delay;
			#endif
			if(ds->set_ptp_delay_type_flag)
				tmp.out_config.ptp.ptpDelayType = id_pdt;
				
			if(ds->set_ptp_multi_uni_type_flag)
				tmp.out_config.ptp.ptpMulticast = id_pmut;
				
			if(ds->set_ptp_layer_type_flag)
				tmp.out_config.ptp.ptpLayer = id_plt;
				
			if(ds->set_ptp_step_type_flag)
				tmp.out_config.ptp.ptpStep = id_pst;
				
			if(ds->set_ptp_sync_freq_flag)
				tmp.out_config.ptp.ptpSync = id_psf;
				
			if(ds->set_ptp_announce_freq_flag)
				tmp.out_config.ptp.ptpAnnounce = id_paf;
		 		
		 	if(ds->set_ptp_pdelay_req_freq_flag)
				tmp.out_config.ptp.ptpPdelayReq = id_ppdrf;
			
			if(ds->set_ptp_ssm_flag)
				tmp.out_config.ptp.ptpSsmEnable = id_pssm;		
			
			if(ds->set_ptp_ip_flag)
				memcpy(tmp.out_config.ptp.ptpIp, ds->ptpIp, MAX_IP_LEN);
				
			if(ds->set_ptp_mac_flag)
				memcpy(tmp.out_config.ptp.ptpMac, ds->ptpMac, MAX_MAC_LEN);

			if(ds->set_ptp_mask_flag)
				memcpy(tmp.out_config.ptp.ptpMask, ds->ptpMask, MAX_MASK_LEN);
			if(ds->set_ptp_gw_flag)
				memcpy(tmp.out_config.ptp.ptpGateway, ds->ptpGateway, MAX_GATEWAY_LEN);

			if(ds->set_ptp_slave_ip_flag){
				//printf("test_start\n");
				if(!str_ip_group_to_uint32((char *)ds->ptp_Slave_Ip, tmp.out_config.ptp.ptpSlaveIp, &tmp.out_config.ptp.ptpSlaveIp_num))
					return -1;
				#if 0
				int i;
				for (i = 0; i < tmp.out_config.ptp.ptpSlaveIp_num; ++i){
					printf("%d####:%08x\n", i+1,tmp.out_config.ptp.ptpSlaveIp[i]);
				}
				char des_ipv4_grp[MAX_IP_LEN*MAX_IP_NUM+1];
				memset(des_ipv4_grp, 0x0, sizeof(des_ipv4_grp));
				if(!uint32_ip_group_to_str_ip(tmp.out_config.ptp.ptpSlaveIp,tmp.out_config.ptp.ptpSlaveIp_num, des_ipv4_grp))
					return -1;
				printf("test:%s\n", des_ipv4_grp);
				return 0;
				#endif
			}
			//memcpy(tmp.out_config.ptp.ptpSlaveIp, ds->ptp_Slave_Ip, MAX_IP_LEN);
			
			if(ds->set_ptp_level_flag)
				
				memcpy(tmp.out_config.ptp.ptpLevel, ds->ptp_level, 4);
			if(ds->set_ptp_priority_flag)
				
				memcpy(tmp.out_config.ptp.ptpPriority, ds->ptp_priority, 4);
			
			if(ds->set_ptp_region_flag)
				
				memcpy(tmp.out_config.ptp.ptpRegion, ds->ptp_region, 4);
			
		}
		else if(BID_PTPF == sta.boardId)
		{
			memcpy(&tmp.out_config.ptpf, &sta.ptpf[0], sizeof(struct ptpfinfo));

			#if 1//support set ptp/ntp/none
			//if(ds->set_signal_type_flag)
			//	tmp.out_config.ptpf.ptpSignalType = id_signal;//move to outcommoninfo
			#else
			//if(ds->set_signal_type_flag)
			//	return -1;
			#endif

			//if(ds->set_delay_flag)
			//	tmp.out_config.ptpf.ptpDelay = ds->delay;

			if(ds->set_ptp_delay_type_flag)
				tmp.out_config.ptpf.ptpDelayType = id_pdt;

			
				
			if(ds->set_ptp_multi_uni_type_flag)
				tmp.out_config.ptpf.ptpMulticast = id_pmut;
				
			if(ds->set_ptp_layer_type_flag)
				tmp.out_config.ptpf.ptpLayer = id_plt;
				
			if(ds->set_ptp_step_type_flag)
				tmp.out_config.ptpf.ptpStep = id_pst;
				
			if(ds->set_ptp_sync_freq_flag)
				tmp.out_config.ptpf.ptpSync = id_psf;
				
			if(ds->set_ptp_announce_freq_flag)
				tmp.out_config.ptpf.ptpAnnounce = id_paf;
				
		
		 		
		 	if(ds->set_ptp_pdelay_req_freq_flag)
				tmp.out_config.ptpf.ptpPdelayReq = id_ppdrf;
				
			if(ds->set_ptp_ip_flag)
				memcpy(tmp.out_config.ptpf.ptpIp, ds->ptpIp, MAX_IP_LEN);
				
			if(ds->set_ptp_mac_flag)
				memcpy(tmp.out_config.ptpf.ptpMac, ds->ptpMac, MAX_MAC_LEN);
				
			if(ds->set_ptp_mask_flag)
				memcpy(tmp.out_config.ptpf.ptpMask, ds->ptpMask, MAX_MASK_LEN);
				
			if(ds->set_ptp_gw_flag)
				memcpy(tmp.out_config.ptpf.ptpGateway, ds->ptpGateway, MAX_GATEWAY_LEN);
				
			if(ds->set_ptp_dns1_flag)
				memcpy(tmp.out_config.ptpf.ptpDns1, ds->ptpDns1, MAX_DNS_LEN);
				
			if(ds->set_ptp_dns2_flag)
				memcpy(tmp.out_config.ptpf.ptpDns2, ds->ptpDns2, MAX_DNS_LEN);
			if(ds->set_ptp_slave_ip_flag)
				memcpy(tmp.out_config.ptpf.ptpSlaveIp, ds->ptp_Slave_Ip, MAX_IP_LEN);
		}else{
			return -1;
		}
	}
	msg.mtype = MSG_CFG_OUT;
	memcpy(msg.mdata, tmp.buffer, sizeof(struct outputinfo));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct outputinfo)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}






/*
  -1	失败
   0	成功
*/
int type_delay_set(struct outCtx *ctx, struct outdelay *od)
{
	struct config cfg;
	struct msgbuf msg;

	od->delay /= 10;//because fpga x10

	memset(&cfg, 0, sizeof(struct config));
	memcpy(&(cfg.out_delay_config), od, sizeof(struct outdelay));
	
	msg.mtype = MSG_CFG_OUT_DELAY;
	memcpy(msg.mdata, cfg.buffer, sizeof(struct outdelay));
	if(MsgQ_Send(ctx->ipc.ipc_msgq_id, &msg, sizeof(struct outdelay)))
	{
		if(notify(ctx->pid.p_id, NOTIFY_SIGNAL))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}






