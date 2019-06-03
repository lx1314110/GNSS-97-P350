#ifndef	__COUT_CONTEXT__
#define	__COUT_CONTEXT__




#include "alloc.h"





struct outCtx {
	int fpga_fd;
	u8_t bid[SLOT_CURSOR_ARRAY_SIZE];
	struct ipcinfo ipc;
	struct pidinfo pid;
};



struct outsetinfo {
	int slot;

	u8_t set_ptptype_flag;
	int ptptype;//PTP_TYPE_CN / PTP_TYPE_FRGN(DEFAULT)

	u8_t set_delay_flag;
	int delay;
	
	u8_t set_signal_type_flag;
	char signal[10];

	u8_t set_ptp_delay_type_flag;
	u8_t ptpDelayType[4];//delay type
	
	u8_t set_ptp_model_type_flag;
	u8_t ptpModelType[7];//delay type
	
	u8_t set_ptp_multi_uni_type_flag;
	u8_t ptpMulticast[6];//multicast or unicast

	u8_t set_ptp_layer_type_flag;
	u8_t ptpLayer[7];//layer 2 or layer 3

	u8_t set_ptp_step_type_flag;
	u8_t ptpStep[6];//one-step or two-step

	u8_t set_ptp_sync_freq_flag;
	u8_t ptpSync[6];//frequency of sync

	u8_t set_ptp_announce_freq_flag;
	u8_t ptpAnnounce[6];//frequency of announce
	
	u8_t set_ptp_delay_req_freq_flag;
	u8_t ptpDelayReq[6];//frequency of delay_req

	u8_t set_ptp_pdelay_req_freq_flag;
	u8_t ptpPdelayReq[6];//frequency of pdelay_req

	u8_t set_ptp_ssm_flag;
	u8_t ptpssm[2];//freq_trace
	
	u8_t set_ptp_ip_flag;
	u8_t ptpIp[MAX_IP_LEN];//ip

	u8_t set_ptp_mac_flag;
	u8_t ptpMac[MAX_MAC_LEN];//mac
	
	u8_t set_ptp_mask_flag;
	u8_t ptpMask[MAX_MASK_LEN];//mask

	u8_t set_ptp_gw_flag;
	u8_t ptpGateway[MAX_GATEWAY_LEN];//gateway

	u8_t set_ptp_dns1_flag;
	u8_t ptpDns1[MAX_DNS_LEN];// 1st dns

	u8_t set_ptp_dns2_flag;
	u8_t ptpDns2[MAX_DNS_LEN];// 2nd dns

	u8_t set_ptp_slave_ip_flag;
	//u8_t ptp_Slave_Ip[MAX_IP_LEN];//slave ip
	u8_t ptp_Slave_Ip[MAX_IP_LEN*MAX_IP_NUM+1];//peer slave ip

	u8_t set_ptp_level_flag;
	u8_t ptp_level[4];//ptpe level

	u8_t set_ptp_priority_flag;
	u8_t ptp_priority[4];//ptp priority
	
	u8_t set_ptp_region_flag;
	u8_t ptp_region[4];//region
};





#endif//__COUT_CONTEXT__


