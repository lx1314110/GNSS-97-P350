#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "lib_dbg.h"
#include "d61850_alloc.h"
#include "d61850_protocol.h"
#include "lib_cpu.h"
#include "lib_fpga.h"
#include "lib_bit.h"
#include "addr.h"
#include "common.h"

#define DEVICE_61850 "MONT/"
#define LLN0 "LLN0/"
#define GGI01 "GGIO1/"



int send_version_info(struct iec61850_Ctx *ctx);
int send_net_info(struct iec61850_Ctx *ctx);
int send_time_source_info(struct iec61850_Ctx *ctx);
int send_out_source_info(struct iec61850_Ctx *ctx);
int send_alm_info(struct iec61850_Ctx *ctx);
int send_cpu_info(struct iec61850_Ctx *ctx);






/*
  true:		程序运行
  false:	程序退出
*/
bool_t isRunning(struct iec61850_Ctx *ctx)
{
	return ctx->loop_flag;
}










/*
   0	成功
  -1	失败
*/
int ProcIec61850(struct iec61850_Ctx *ctx)
{
	int ret = 0;
	
	initializeContext(ctx);
	set_print_level(true, DBG_INFORMATIONAL, DAEMON_CURSOR_IEC61850);
	syslog_init(NULL);
	
	if(-1 == initializeDatabase(ctx))//初始化数据库
	{
		ret = __LINE__;
		goto exit1;
	}
	
	if(-1 == initializeFpga(ctx))
	{
		ret = __LINE__;
		goto exit2;
	}

	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit3;
	}

	if(-1 == initializeExitSignal())
	{
		ret = __LINE__;
		goto exit3;
	}
	
	/*init serial*/

	if (-1 == initializeSerial(ctx))
	{
		ret = __LINE__;
		goto exit3;
	}
	
	if(-1 == initializeNotifySignal())//NOTIFY_SIGNAL ctx->notify_flag = ture
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(-1 == initializeShareMemory(ctx->ipc))
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(-1 == initializeMessageQueue(ctx->ipc))
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(-1 == initializeSemaphoreSet(ctx->ipc))
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(-1 == writePid(ctx->ipc))
	{
		ret = __LINE__;
		goto exit4;
	}
	
	if(-1 == set61850_network(ctx, TBL_61850))//设置ip,暂未完成
	{
		ret = __LINE__;
		goto exit4;
	}
	
	/*send  once*/

	/*fixed*/
	if (0 != iec61850_send(ctx->serial_fd, DEVICE_61850 LLN0 "DevType=GNSS-97-P350")){//设备
		print(	DBG_ERROR, 
					"--Failed to send DevType.");
	}

	if (0 != iec61850_send(ctx->serial_fd, DEVICE_61850 LLN0 "DevDescr=Time synchronization")){//备注
		print(	DBG_ERROR, 
					"--Failed to send DevDescr.");
	}
	
	if (0 != iec61850_send(ctx->serial_fd, DEVICE_61850 LLN0 "Company=DTT")){//大唐
		print(	DBG_ERROR, 
					"--Failed to send Company.");
	}	

	if (0 != iec61850_send(ctx->serial_fd,DEVICE_61850 LLN0 "PortNum=10")){//端口号
		print(	DBG_ERROR, 
					"--Failed to send PortNum.");
	}	
	/*
	if (0 != iec61850_send(ctx->serial_fd,DEVICE_61850 LLN0 "Board61850IP=192.168.1.210")){//端口号
		print(	DBG_ERROR, 
					"--Failed to send PortNum.");
	}
	*/
	/*form board*/
	
	send_version_info(ctx);//发送版本信息

	while(isRunning(ctx))
	{
		if(isNotify(ctx))//是否有通知
		{
			if(0 == ReadMessageQueue(ctx))//读取设置命令
			{
				ret = __LINE__;
				print(DBG_ERROR,
				  	  "ReadMessageQueue Error.");
				break;
			}
		}	
		int rt;
		if ((rt = system("p350sys btype | grep '61850' >/dev/null")) != 0){
				ret = __LINE__;
				goto exit4;
		}
		SysWaitFpgaRunStatusOk(ctx->fpga_fd);
		send_net_info(ctx);//发送网络信息
		send_cpu_info(ctx);//发送CPU信息
		send_time_source_info(ctx);
		send_alm_info(ctx);//发送告警信息
		send_out_source_info(ctx);//发送输出源信息
		//usleep(100000);//(100ms)
		sleep(1);
	}

exit4:
	//cleanShareMemory(ctx->ipc);
	cleanSerial(ctx);

exit3:
	cleanFpga(ctx);
exit2:
	cleanDatabase(ctx);
exit1:
	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();

	return ret;
}



#define CMD_MSG_LEN 100

int send_version_info(struct iec61850_Ctx *ctx)
{
	FILE * fp;
	char line_buf[CMD_MSG_LEN];
	char hw_ver[CMD_MSG_LEN];
	char sw_ver[CMD_MSG_LEN];
	char fpga_ver[CMD_MSG_LEN];
	if(NULL == (fp = popen("p350sys ver", "r" ))){
		print(	DBG_ERROR, 
					"--popen version.");	
		return 0;
	}
	
	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--ver fgets err.");
		return 0;
	}

	sscanf(line_buf,"HW_VER | %s",hw_ver);

	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--HW_VER ver fgets err.");
		return 0;
	}

	sscanf(line_buf,"SW_VER | %s",sw_ver);


	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--SW_VER ver fgets err.");
		return 0;
	}

	sscanf(line_buf,"FPGA_VER | %s",fpga_ver);

	pclose(fp);
	//printf("hw_ver is %s ,sw_ver is %s, fpga_ver is %s \n",hw_ver,sw_ver,fpga_ver);

	sprintf(line_buf,DEVICE_61850 LLN0 "HWVersion=%s",hw_ver);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"--Failed to send HWVersion.");
		return 0;
	}
//	printf("%s\n",line_buf);
	sprintf(line_buf,DEVICE_61850 LLN0 "FWVersion=%s",fpga_ver);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"--Failed to send FWVersion.");
		return 0;
	}
	
	sprintf(line_buf,DEVICE_61850 LLN0 "SWVersion=%s",sw_ver);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"--Failed to send SWVersion.");
		return 0;
	}

	
	return 0;
}

int send_net_info(struct iec61850_Ctx *ctx)
{
	FILE * fp;
	char line_buf[CMD_MSG_LEN];
	char ip[CMD_MSG_LEN];
	char mac[CMD_MSG_LEN];
	char mask[CMD_MSG_LEN];
	char gateway[CMD_MSG_LEN];
	if(NULL == (fp = popen("p350sys net", "r" ))){
		print(	DBG_ERROR, 
					"--popen net.");	
		return 0;
	}

	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--net fgets err.");
		return 0;
	}
	sscanf(line_buf,"IP=%s",ip);
	
	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--net fgets err.");
		return 0;
	}
	sscanf(line_buf,"MAC=%s",mac);
	
	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--net fgets err.");
		return 0;
	}
	sscanf(line_buf,"MASK=%s",mask);
	
	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR, 
					"--net fgets err.");
		return 0;
	}
	sscanf(line_buf,"GATEWAY=%s",gateway);
	pclose(fp);

	//printf("ip is %s, mask is %s,mac is %s, gateway is %s \n",ip,mask,mac,gateway);


	sprintf(line_buf,DEVICE_61850 LLN0 "IPAddr=%s",ip);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"--Failed to send IPAddr.");
		return 0;
	}

	sprintf(line_buf,DEVICE_61850 LLN0 "MACAddr=%s",mac);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"--Failed to send MACAddr.");
		return 0;
	}
	return 0;
}

int send_cpu_info(struct iec61850_Ctx *ctx)
{	
	int cpu;
	char cpustr[CMD_MSG_LEN];
	char line_buf[CMD_MSG_LEN];
	
	cpu = cpu_utilization();
	sprintf(cpustr,"CpuUsage=%d",cpu);
	//printf("%s\n",cpustr);

	sprintf(line_buf,DEVICE_61850 LLN0 "%s",cpustr);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR,
					"--Failed to send MACAddr.");
		return 0;
	}
	return 0;
	
}

int send_time_source_info(struct iec61850_Ctx *ctx)
{
	int i = 0, found = 0;
	int bid = 0;
	FILE * fp = NULL;
	char line_buf[CMD_MSG_LEN];
	char send_buf[CMD_MSG_LEN];

	for (i = 0; i < INPUT_SLOT_CURSOR_ARRAY_SIZE; ++i){
		memset(line_buf,0x0,sizeof(line_buf));
		sprintf(line_buf, "p350sys btype | head -%d | tail -1 | awk -F: \'{printf $1}\' | tr -d \' \'",i+1);
		if(NULL == (fp = popen(line_buf, "r" ))){
			print(	DBG_ERROR,
					"--popen p350sys.");		
			return 0;
		}

		memset(line_buf,0x0,sizeof(line_buf));
		if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
			print(	DBG_ERROR,
					"--mode fgets err.");
			return 0;
		}
		pclose(fp);
		fp = NULL;
		bid = atoi(line_buf);
	
		if( BID_GPSBF == bid ||
		BID_GPSBE == bid ||
		BID_GBDBF == bid ||
		BID_GBDBE == bid ||
		BID_BDBF == bid ||
		BID_BDBE == bid ){
			found = 1;
			break;
		}
	}

	memset(line_buf,0x0,sizeof(line_buf));
	if (found)
		sprintf(line_buf,"p350in %d mode",i+1);
	else
		sprintf(line_buf,"p350in %d mode",1);//default
	if(NULL == (fp = popen(line_buf, "r" ))){
		print(	DBG_ERROR,
					"--popen mode.");
		return 0;
	}
	memset(line_buf,0x0,sizeof(line_buf));
	if(NULL == fgets(line_buf, CMD_MSG_LEN, fp)){
		print(	DBG_ERROR,
					"--mode fgets err.");
		return 0;
	}
	pclose(fp);
	fp = NULL;

	memset(send_buf,0x0,sizeof(send_buf));
	if (strncmp("SG",line_buf,2) == 0){
		sprintf(send_buf,DEVICE_61850 LLN0 "InType=%s","GPS");
	}else if (strncmp("SB",line_buf,2) == 0){
		sprintf(send_buf,DEVICE_61850 LLN0 "InType=%s","北斗");
	}else if (strncmp("MG",line_buf,2) == 0){
		sprintf(send_buf,DEVICE_61850 LLN0 "InType=%s","混合模式、主用GPS");
	}else if (strncmp("MB",line_buf,2) == 0){
		sprintf(send_buf,DEVICE_61850 LLN0 "InType=%s","混合模式、主用北斗");
	}else{
		sprintf(send_buf,DEVICE_61850 LLN0 "InType=%s","无卫星模块");
	}
	//sprintf(send_buf,DEVICE_61850 LLN0 "InType=%s",line_buf);
	//printf("time source is %s \n",send_buf);
	if (0 != iec61850_send(ctx->serial_fd,send_buf)){
		print(	DBG_ERROR, 
					"--Failed to send InType.");
		return 0;
	}
	return 0;
}

int send_out_source_info(struct iec61850_Ctx *ctx)
{
	char line_buf[CMD_MSG_LEN];
	sprintf(line_buf,DEVICE_61850 LLN0 "OutType=%s","1588、PTP、SNTP、IRIGB、PPS");
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"--Failed to send OutType.");
		return 0;
	}
	return 0;
}




int get_line(int line, char * buf, FILE * fp)
{
	int i;
	for (i=0;i<line;i++)
	{
		if(NULL == fgets(buf, CMD_MSG_LEN, fp)){
			print(	DBG_ERROR, 
						"--mode fgets err.");
			return -1;
		}

	}
	
	return 0;
}

#if 0
int send_alm_info(struct iec61850_Ctx *ctx)
{
	 u16_t ext_pwr_alm;
	// u16_t in_alm;
	 char power_stat1,power_stat2;
	 int alm2;
	 char line_buf[CMD_MSG_LEN];
	 char send_buf[CMD_MSG_LEN];
	 FILE * fp;
	 

	memset(line_buf,0,CMD_MSG_LEN);
	memset(send_buf,0,CMD_MSG_LEN);
	if(!FpgaRead(ctx->fpga_fd, FPGA_PWR_ALM, &ext_pwr_alm))
	{
		print(DBG_ERROR, "<%s>--Failed to read alarm of external power.", 
						 gDaemonTbl[DAEMON_CURSOR_IEC61850]);
		
		return 0;
	}

	power_stat2 = ext_pwr_alm &BIT(0);
	power_stat1 = (ext_pwr_alm &BIT(1)) >> 1;

	//printf("power1 is %d ,power2 is %d \n",power_stat1,power_stat2);

	sprintf(line_buf,DEVICE_61850 GGI01 "Alm=%d",power_stat1);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"<%s>--Failed to send Alm.", 
					gDaemonTbl[DAEMON_CURSOR_IEC61850]);
		return 0;
	}

	sprintf(line_buf,DEVICE_61850 GGI01 "Alm1=%d",power_stat2);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, 
					"<%s>--Failed to send Alm.", 
					gDaemonTbl[DAEMON_CURSOR_IEC61850]);
		return 0;
	}


	#if 0
	if(!FpgaRead(ctx->fpga_fd, FPGA_S01_IN_SIGNAL, &in_alm)){
			return 0;
	}
	#endif


	if(NULL == (fp = popen("p350alm", "r" ))){
		print(	DBG_ERROR, 
					"<%s>--popen mode.", 
					gDaemonTbl[DAEMON_CURSOR_IEC61850]);		
		return 0;
	}


	if (get_line(6,line_buf,fp)!=0){
		pclose(fp);
		print(	DBG_ERROR, 
						"<%s>--mode get_line err.", 
						gDaemonTbl[DAEMON_CURSOR_IEC61850]);
		return 0;
	}
	//printf("send %s \n",line_buf);
	sscanf(line_buf,"%d |",&alm2);
	//printf("alm2 is %d\n",alm2);
	pclose(fp);
	
	//alm2 = (in_alm &BIT(4)) >> 4;
	sprintf(send_buf,DEVICE_61850 GGI01 "Alm2=%d",alm2);
	if (0 != iec61850_send(ctx->serial_fd,send_buf)){
		print(	DBG_ERROR, 
					"<%s>--Failed to send Alm2.", 
					gDaemonTbl[DAEMON_CURSOR_IEC61850]);
		return 0;
	}
	//printf("%s\n",line_buf);
	return 0;
}
#else
int send_alm_info(struct iec61850_Ctx *ctx)
{
	u16_t ext_pwr_alm = 0;
	// u16_t in_alm;
	int bidpwr1 = 0x00FF, bidpwr2 = 0x00FF;
	u16_t temp = 0, insrc_valid_alm = 0;
	char power_stat1 = ALM_OFF,power_stat2 = ALM_OFF;
	int alm2 = ALM_OFF;
	char line_buf[CMD_MSG_LEN];

	if(!FpgaRead(ctx->fpga_fd, FPGA_PWR_ALM, &ext_pwr_alm))
	{
		print(DBG_ERROR, "Failed to read alarm of external power.");
		
		return -1;
	}


	if(!FpgaRead(ctx->fpga_fd, FPGA_BID_S18_S19, &temp)){
		print(	DBG_ERROR, "Failed to read FPGA_BID_S18_S19.");
		return -1;
	}
	bidpwr1 = (temp>>8)&0x00FF;//18
	bidpwr2 = temp&0x00FF;//19 slot
	//二号外部电源输入告警状态
	if(!(ext_pwr_alm &BIT(0)) || bidpwr2 == 0x00FF){
		power_stat2 = ALM_OFF;
	} else {
		power_stat2 = ALM_ON;
	}
	//一号外部电源输入告警状态
	if(!(ext_pwr_alm &BIT(1)) || bidpwr1 == 0x00FF ){
		power_stat1 = ALM_OFF;
	} else {
		power_stat1 = ALM_ON;
	}

	if(!FpgaRead(ctx->fpga_fd, FPGA_SYS_INSRC_VALID, &insrc_valid_alm))
	{
		print(DBG_ERROR, "Failed to read FPGA_SYS_INSRC_VALID.");
		
		return -1;
	}

	if(insrc_valid_alm == 0x0)
		alm2 = ALM_OFF;
	else
		alm2 = ALM_ON;

	memset(line_buf, 0x0, sizeof(line_buf));
	sprintf(line_buf,DEVICE_61850 GGI01 "Alm=%d",power_stat1);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR,"Failed to send Alm.");
		return -1;
	}

	memset(line_buf, 0x0, sizeof(line_buf));
	sprintf(line_buf,DEVICE_61850 GGI01 "Alm1=%d",power_stat2);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, "Failed to send Alm.");
		return -1;
	}

	memset(line_buf, 0x0, sizeof(line_buf));
	sprintf(line_buf,DEVICE_61850 GGI01 "Alm2=%d",alm2);
	if (0 != iec61850_send(ctx->serial_fd,line_buf)){
		print(	DBG_ERROR, "Failed to send Alm2.");
		return -1;
	}
	//printf("%s\n",line_buf);
	return 0;
}

#endif
