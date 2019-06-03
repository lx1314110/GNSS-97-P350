#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_common.h"
#include "lib_system.h"
#include "alloc.h"
#include "addr.h"
#include "dmgr_global.h"
#include "dmgr_ps.h"
#include "dmgr_version.h"
#include "dmgr_parser.h"
#include "dmgr_alloc.h"
#include "lib_ip1725.h"
#include "dmgr_manager.h"
#include "common.h"

#if 0
void SwitchRegWrite(int fpga_fd,unsigned char reg, unsigned short val)
{   
    u16_t chipReg = reg;
    //等待总线空闲
   usleep(5000);

    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_ADDR, chipReg);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_VAL, val);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_REG_OP, 0x00);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_REG_OP, 0x01);

    //等待操作结束
    usleep(5000);
}

#endif

int system_run(const char * pathname, char * const arv[] );


bool_t ledRun(struct mgrCtx *ctx)
{
	u16_t tmp = 0;

	ctx->led_sta = ~(ctx->led_sta);
	tmp = (u16_t)(ctx->led_sta);
	
	if(!FpgaWrite(ctx->fpga_fd, FPGA_LED_RUN, tmp))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to write led" );

		return false;
	}

	return true;
}





/*
  -1	Ê§°Ü
   0	³É¹Š
*/
int readPid(struct pidinfo *pid)
{
	int i;
	int procPid;

	for(i=0; i<DAEMON_CURSOR_ARRAY_SIZE; i++)
	{
		if( (DAEMON_CURSOR_MANAGER != i) && 
			(DAEMON_CURSOR_TELNET != i) && 
			(DAEMON_CURSOR_CLOCK != i) )
		{
			pid[i].p_cursor = i;
			procPid = find_pid_by_name((char *)gDaemonTbl[i]);
			if(-1 == procPid)
			{
				#if 0
				print(	DBG_ERROR, 
						"<%s>--Failed to read pid of %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[i]);
				
				#endif
				return -1;
			}
			else
			{
				pid[i].p_id = procPid;
			}
		}
	}

	return 0;
}






/*
  -1	Ê§°Ü
   0	³É¹Š
*/
int procStart(struct pidinfo *pid)
{
	char cmd[1000];
	char * argv_no_use[]={cmd,NULL};
	pid_t pid_g;
	int rt;
	
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_INPUT]);//gDaemonTblÊÇœø³ÌË÷Òý
	if ((pid_g = system_run(cmd,argv_no_use))< 0){//ÔËÐÐInputœø³Ì
		print(	DBG_ERROR, 
						"<%s>--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[DAEMON_CURSOR_INPUT]);
				return -1;
	}else{
		
		pid[DAEMON_CURSOR_INPUT].p_id = pid_g;
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_OUTPUT]);
	if ((pid_g = system_run(cmd,argv_no_use)) < 0){//ÔËÐÐoutputœø³Ì
		print(	DBG_ERROR, 
						"<%s>--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[DAEMON_CURSOR_OUTPUT]);
				return -1;
	}else{
		
		pid[DAEMON_CURSOR_OUTPUT].p_id = pid_g;
	}
	
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_NTP]);
	if ((pid_g = system_run(cmd,argv_no_use)) < 0){//ÔËÐÐNTPœø³Ì
		print(	DBG_ERROR, 
						"<%s>--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[DAEMON_CURSOR_NTP]);
				return -1;
	}else{
		
		pid[DAEMON_CURSOR_NTP].p_id = pid_g;
	}


	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_ALARM]);
	if ((pid_g = system_run(cmd,argv_no_use)) < 0){//ÔËÐÐALARMœø³Ì
		print(	DBG_ERROR, 
						"<%s>--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[DAEMON_CURSOR_ALARM]);
				return -1;
	}else{
		
		pid[DAEMON_CURSOR_ALARM].p_id = pid_g;
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
	if ((pid_g = system_run(cmd,argv_no_use)) < 0){//ÔËÐÐKEYLCDœø³Ì
		print(	DBG_ERROR, 
						"<%s>--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
				return -1;
	}else{
		
		pid[DAEMON_CURSOR_KEYLCD].p_id = pid_g;
	}
	
	
	
	if ((rt = system("p350sys btype | grep '61850' >/dev/null")) == 0){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_IEC61850]);
		if ((pid_g = system_run(cmd,argv_no_use)) < 0){//ÔËÐÐIEC61850œø³Ì
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_IEC61850]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_IEC61850].p_id = pid_g;
		}
	}

	
		char * argv_http[]={"p350_http","-T","UTF-8","-C","/usr/p350/app/p350.conf",NULL};
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_HTTP]);
		if ((pid_g = system_run(cmd,argv_http)) < 0){//ÔËÐÐHTTPœø³Ì
		print(	DBG_ERROR, 
						"--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_HTTP]);
				return -1;
		}else{
			print(DBG_INFORMATIONAL, "http pid is %d \n",pid_g);
			pid[DAEMON_CURSOR_HTTP].p_id = pid_g;

		}
	
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_WG]);
	if ((pid_g = system_run(cmd,argv_no_use)) < 0){//ÔËÐÐWGœø³Ì
		
		print(	DBG_ERROR, 
						"<%s>--Can't start %s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						gDaemonTbl[DAEMON_CURSOR_WG]);
				return -1;
	}else{
		
		pid[DAEMON_CURSOR_WG].p_id = pid_g;
	}
	
	return 0;

}



/*
  -1	Ê§°Ü
   0	³É¹Š
*/
int procRestart(struct pidinfo *pid)
{
	char cmd[1000];
	char * argv_no_use[]={cmd,NULL};
	pid_t pid_g;
	int rt;

	if (pid[DAEMON_CURSOR_INPUT].p_id <= 0){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_INPUT]);
		if ((pid_g = system_run(cmd,argv_no_use))< 0){
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_INPUT]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_INPUT].p_id = pid_g;
		}
	}

	if(pid[DAEMON_CURSOR_OUTPUT].p_id <= 0){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_OUTPUT]);
		if ((pid_g = system_run(cmd,argv_no_use)) < 0){
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_OUTPUT]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_OUTPUT].p_id = pid_g;
		}
	}

	if (pid[DAEMON_CURSOR_NTP].p_id <= 0){
	memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_NTP]);
		if ((pid_g = system_run(cmd,argv_no_use)) < 0){
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_NTP]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_NTP].p_id = pid_g;
		}
	}

	if (pid[DAEMON_CURSOR_ALARM].p_id  <= 0){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_ALARM]);
		if ((pid_g = system_run(cmd,argv_no_use)) < 0){
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_ALARM]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_ALARM].p_id = pid_g;
		}
	}

	if (pid[DAEMON_CURSOR_KEYLCD].p_id <= 0){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
		if ((pid_g = system_run(cmd,argv_no_use)) < 0){
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_KEYLCD]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_KEYLCD].p_id = pid_g;
		}
	}

	if (pid[DAEMON_CURSOR_WG].p_id <= 0){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_WG]);
		if ((pid_g = system_run(cmd,argv_no_use)) < 0){
			print(	DBG_ERROR, 
							"<%s>--Can't start %s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							gDaemonTbl[DAEMON_CURSOR_WG]);
					return -1;
		}else{
			
			pid[DAEMON_CURSOR_WG].p_id = pid_g;
		}
	}
	
	if (((rt = system("p350sys btype | grep '61850' >/dev/null")) == 0)
		&&(pid[DAEMON_CURSOR_IEC61850].p_id <= 0)){
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "/usr/p350/app/proc/%s", gDaemonTbl[DAEMON_CURSOR_IEC61850]);
			if ((pid_g = system_run(cmd,argv_no_use)) < 0){
				print(	DBG_ERROR, 
								"<%s>--Can't start %s", 
								gDaemonTbl[DAEMON_CURSOR_MANAGER], 
								gDaemonTbl[DAEMON_CURSOR_IEC61850]);
						return -1;
			}else{
				
				pid[DAEMON_CURSOR_IEC61850].p_id = pid_g;

			}
		}
	return 0;

}







void procStop(struct pidinfo *pid)
{
	if(pid[DAEMON_CURSOR_INPUT].p_id > 0)
		kill(pid[DAEMON_CURSOR_INPUT].p_id, EXIT_SIGNAL2);
			
	if(pid[DAEMON_CURSOR_OUTPUT].p_id > 0)	
		kill(pid[DAEMON_CURSOR_OUTPUT].p_id, EXIT_SIGNAL2);

	if(pid[DAEMON_CURSOR_NTP].p_id > 0)
		kill(pid[DAEMON_CURSOR_NTP].p_id, EXIT_SIGNAL2);

	if(pid[DAEMON_CURSOR_ALARM].p_id > 0)
		kill(pid[DAEMON_CURSOR_ALARM].p_id, EXIT_SIGNAL2);

	if(pid[DAEMON_CURSOR_CLOCK].p_id > 0)
		kill(pid[DAEMON_CURSOR_CLOCK].p_id, EXIT_SIGNAL2);

	if(pid[DAEMON_CURSOR_KEYLCD].p_id > 0)
		kill(pid[DAEMON_CURSOR_KEYLCD].p_id, EXIT_SIGNAL2);
	
	if(pid[DAEMON_CURSOR_WG].p_id > 0)
		kill(pid[DAEMON_CURSOR_WG].p_id, EXIT_SIGNAL2);
	
	if(pid[DAEMON_CURSOR_TELNET].p_id > 0)
		kill(pid[DAEMON_CURSOR_TELNET].p_id, EXIT_SIGNAL2);
	if(pid[DAEMON_CURSOR_IEC61850].p_id > 0)
		kill(pid[DAEMON_CURSOR_IEC61850].p_id, EXIT_SIGNAL2);

	/*HTTP is demo */

	if (system("killall p350_http") != 0){
		print(	DBG_ERROR, 
				"<%s>--Can't stop %s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				gDaemonTbl[DAEMON_CURSOR_HTTP]);
	}
}






/*
  true:		³ÌÐòÔËÐÐ
  false:	³ÌÐòÍË³ö
*/
bool_t isRunning(struct mgrCtx *ctx)
{
	return ctx->loop_flag;
}






void sysView(struct mgrCtx *ctx)
{
	printf("\n");
	printf("\n====================================================================\n\n");
	printf("\tP350 integrated time-frequency synchronous equipment\n");
	printf("\tIP   : %s\n", ctx->net.ip);
	printf("\tMASK : %s\n", ctx->net.mask);
	printf("\tGW   : %s\n", ctx->net.gateway);
	printf("\tMAC  : %s\n", ctx->net.mac);
	printf("\n");
	printf("\tHW_VER  : %s\n", ctx->ver.hwVer);
	printf("\tSW_VER  : %s\n", ctx->ver.swVer);
	printf("\tFPGA_VER: %s\n", ctx->ver.fpgaVer);
	printf("\tBUILD_TIME: %s %s\n", __DATE__,__TIME__);
	printf("\n====================================================================\n\n");

//--log--
	print(DBG_INFORMATIONAL, "\n");
	print(DBG_INFORMATIONAL, "\n====================================================================\n\n");
	print(DBG_INFORMATIONAL, "\tP350 integrated time-frequency synchronous equipment\n");
	print(DBG_INFORMATIONAL, "\tIP   : %s\n", ctx->net.ip);
	print(DBG_INFORMATIONAL, "\tMASK : %s\n", ctx->net.mask);
	print(DBG_INFORMATIONAL, "\tGW   : %s\n", ctx->net.gateway);
	print(DBG_INFORMATIONAL, "\tMAC  : %s\n", ctx->net.mac);
	print(DBG_INFORMATIONAL, "\n");
	print(DBG_INFORMATIONAL, "\tHW_VER  : %s\n", ctx->ver.hwVer);
	print(DBG_INFORMATIONAL, "\tSW_VER  : %s\n", ctx->ver.swVer);
	print(DBG_INFORMATIONAL, "\tFPGA_VER: %s\n", ctx->ver.fpgaVer);
	print(DBG_INFORMATIONAL, "\tBUILD_TIME: %s %s\n", __DATE__,__TIME__);
	print(DBG_INFORMATIONAL, "\n====================================================================\n\n");
}









/*
  1	³É¹Š
  0	Ê§°Ü
*/
int ReadFpgaVersion(struct mgrCtx *ctx)
{
	u16_t tmp;
	
	if(!FpgaRead(ctx->fpga_fd, FPGA_VER, &tmp))
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to read FPGA version");
		
		return 0;
	}
	
	sprintf((char *)ctx->ver.fpgaVer, 
			"V%c%c.%c%c", 
			'0' +bcd2decimal((tmp >> 12) &0x000F),
			'0' +bcd2decimal((tmp >>  8) &0x000F),
			'0' +bcd2decimal((tmp >>  4) &0x000F),
			'0' +bcd2decimal((tmp      ) &0x000F));

	return 1;
}









/*
  1	³É¹Š
  0	Ê§°Ü
*/
int WriteVerToShareMemory(struct mgrCtx *ctx)
{
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_MANAGER].ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		print( DBG_ERROR, 
			   "<%s>--Failed to lock semaphore.", 
			   gDaemonTbl[DAEMON_CURSOR_MANAGER] );
		
		return 0;
	}
	
	shm_write(ctx->ipc[IPC_CURSOR_MANAGER].ipc_base, 
			  SHM_OFFSET_STA, 
			  sizeof(struct verinfo), 
			  (char *)&(ctx->ver), 
			  sizeof(struct verinfo));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_MANAGER].ipc_sem_id, SEMA_MEMBER_MANAGER))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER] );
		
		return 0;
	}

	return 1;
}









/*
  0	Ê§°Ü
  1	³É¹Š
*/
int DevMasterSlave(struct mgrCtx *ctx)//žùŸÝDEVµÄÀàÐÍÑ¡È¡htmlÊÇMaster»¹ÊÇslave
{
	u16_t devType;
	char cmd[256];

	if(!FpgaRead(ctx->fpga_fd, FPGA_DEVICE_TYPE, &devType))
	{
		return 0;
	}

	memset(cmd, 0, sizeof(cmd));
	if(0x02 == devType)
	{
		sprintf(cmd, "cp -f %s %s", WWW_INDEX_MASTER_PATH, WWW_INDEX_PATH);
	}
	else if(0x01 == devType)
	{
		sprintf(cmd, "cp -f %s %s", WWW_INDEX_SLAVE_PATH, WWW_INDEX_PATH);
	}
	else
	{
		sprintf(cmd, "cp -f %s %s", WWW_INDEX_MASTER_PATH, WWW_INDEX_PATH);
	}

	if(-1 == system(cmd))
	{
		return 0;
	}

	return 1;
}




static int load_fpga(void)
{
	int rt;
	char buff[100];
	sprintf(buff,"cat %s > %s",FPGA_PATH,LOAD_DEV);
	print(DBG_INFORMATIONAL, "Flushing the file:%s to fpga device...",FPGA_PATH);
	rt = system(buff);//°ÑFPGA_PATHÀïµÄÄÚÈÝÐŽÈëµœLOAD_DEVÎÄŒþ

	if (0 == WIFEXITED(rt))//Èç¹û×Óœø³ÌÕý³£œáÊøÔòÎª·Ç0Öµ
	{
		return -1;
	}
	else
	{
		if (WEXITSTATUS(rt) != 0)//È¡µÃ×Óœø³Ìexit()·µ»ØµÄœáÊøŽúÂë
		{
			return -1;
		}
	}
	print(DBG_INFORMATIONAL, "Flush fpga success.");
	return 0;
}



int ProcManager(struct mgrCtx *ctx)//Œà¿Øœø³Ì
{
	int ret = 0;

	set_print_level(true, DBG_INFORMATIONAL, DAEMON_CURSOR_MANAGER);
	syslog_init(NULL);
	initializeBoardNameTable();//³õÊŒ»¯µ¥ÅÌÃû³ÆË÷Òý

	//initialize context
	if(-1 == initializeContext(ctx))//³õÊŒ»¯iec61850 context
	{
		ret = __LINE__;
		goto exit;
	}
	if ( -1 == load_fpga())//°ÑFPGA_PATHÀïµÄÄÚÈÝÐŽÈëµœLOAD_DEVÎÄŒþ
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_MANAGER], 
				"Failed to download fpga");
		ret = __LINE__;
		goto exit;
	}

	sleep(2);

	if(-1 == initializeFpga(ctx))//Žò¿ªFPGA
	{
		ret = __LINE__;
		goto exit1;
	}

	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit2;
	}

	if(0 == DevMasterSlave(ctx))//žùŸÝDEVµÄÀàÐÍÑ¡È¡htmlÊÇMaster»¹ÊÇslave
	{
		ret = __LINE__;
		goto exit2;
	}

	if(0 == ReadFpgaVersion(ctx))//¶ÁÈ¡FPGAµÄ°æ±Ÿ
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeExitSignal())//³õÊŒ»¯ÍË³öÐÅºÅ
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeNotifySignal())//³õÊŒ»¯×Ô¶šÒåÐÅºÅ
	{
		ret = __LINE__;
		goto exit2;
	}
	if(-1 == initializeChildSignal())//³õÊŒ»¯×Óœø³ÌÐÅºÅ£¬»ØÊÕÍË³öœø³ÌµÄpid
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeShareMemory(ctx->ipc))//³õÊŒ»¯¹²ÏíÄÚŽæ
	{
		ret = __LINE__;
		goto exit2;
	}

	if(-1 == initializeMessageQueue(ctx->ipc))//³õÊŒ»¯ÏûÏ¢¶ÓÁÐ
	{
		ret = __LINE__;
		goto exit3;
	}

	if(-1 == initializeSemaphoreSet(ctx->ipc))//³õÊŒ»¯ÐÅºÅÁ¿
	{
		ret = __LINE__;
		goto exit4;
	}

	if(-1 == writePid(ctx->ipc))//°ÑipcÀïµÄpid¿œ±Žµœ¹²ÏíÄÚŽæ
	{
		ret = __LINE__;
		goto exit5;
	}

	//we will use new station:/data/db/p350.db
	SetDatabase();

	#if 0
	//initialize network
	if(0 == initializeDatabase(&ctx->pDb))//³õÊŒ»¯ÊýŸÝ¿â
	{
		ret = -1;
		goto exit;
	}
	#endif
	//if new veth databse not exist, choice old veth database
	SetVethDatabase();

	//initialize network

	
	if(0 == initializeDatabase(&ctx->pDb))
	{
		ret = __LINE__;
		goto exit5;
	}
	if(0 == initializeVethDatabase(&ctx->pVethDb))//³õÊŒ»¯ÊýŸÝ¿â
	{
		ret = __LINE__;
		goto exit5_1;
	}

	//del old cur-event table，move to history
	if(0 == initCurEventAndHistoryTable(ctx->pDb))
	{
		ret = __LINE__;
		goto exit6;
	}
	//del old cur-phasePerf table，move to history
	if(0 == initCurPhasePerfAndHistoryTable(ctx->pDb))
	{
		ret = __LINE__;
		goto exit6;
	}
	
	//if(0 == readNetwork(ctx->pDb, TBL_SYS, &ctx->net))//ŽÓÊýŸÝ¿âÖÐ¶ÁÈ¡ÍøÂçÐÅÏ¢£šip,mask..£©
	if(0 == readWebNetwork(ctx->pVethDb, TBL_VETH, &ctx->net))
	{
		ret = __LINE__;
		goto exit6;
	}

	if(0 == cleanVethDatabase(ctx->pVethDb))
	{
		ret = __LINE__;
		goto exit6;
	}

	if(0 == cleanDatabase(ctx->pDb))
	{
		ret = __LINE__;
		goto exit5_1;
	}

	sysView(ctx);
	if(0 == WriteVerToShareMemory(ctx))
	{
		ret = __LINE__;
		goto exit6;
	}

	if(-1 == initializePriority(DAEMON_PRIO_MANAGER))//³õÊŒ»¯µ±Ç°œø³ÌµÄÓÅÏÈŒ¶
	{
		ret = __LINE__;
		goto exit6;
	}
	
	SwitchConfig(ctx->fpga_fd);
	#ifdef ETH0_NET_CONF
	writeNetwork(&ctx->net);//ÉèÖÃÍøÂç²ÎÊý
	#endif
	if( 1 != setNetworkUp()){
		ret = __LINE__;
		goto exit6;
	}

	CreateWebNetWorkInterface();
	if( 1 != writeWebNetwork(&ctx->net)){
		ret = __LINE__;
		goto exit6;
	}

	//start proc
	if(-1 == procStart(ctx->pid))//¿ªÆôœø³Ì
	{
		ret = __LINE__;
		goto exit;
	}

	sleep(2);

	while(isRunning(ctx))
	{
		if(isNotify(ctx))//ÊÕµœÅäÖÃÃüÁî
		{
			ReadMessageQueue(ctx->pid, ctx->ipc);//ÅäÖÃ
		}
		
		if ( 1 == ctx->guard_flag )//Èç¹ûŒà¿Ø±êÖŸÎª1
			procRestart(ctx->pid);//ÖØÆôœø³Ì

		ledRun(ctx);//µÆÉÁ
		usleep(500000);//(500ms)
	}

exit:
	//stop proc
	procStop(ctx->pid);
		
	sleep(2);

exit6:
	cleanVethDatabase(ctx->pVethDb);
exit5_1:
	cleanDatabase(ctx->pDb);
exit5:
	cleanSemaphoreSet(ctx->ipc);
exit4:
	cleanMessageQueue(ctx->ipc);
exit3:
	cleanShareMemory(ctx->ipc);
exit2:
	cleanFpga(ctx);
exit1:
	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();
	
	return ret;
}



