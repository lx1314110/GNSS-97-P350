#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
//#include <linux/kernel.h>
#include "lib_container.h"
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_msgQ.h"
#include "lib_dbg.h"
#include "lib_file.h"

#include "dmgr_alloc.h"
#include "dmgr_parser.h"
#include "dmgr_global.h"





/*
  true:		收到配置命令
  false:	没有收到配置命令
*/
bool_t isNotify(struct mgrCtx *ctx)
{
	if(ctx->notify_flag)
	{
 		ctx->notify_flag = false;
 		return true;
	}
	else
	{
		return false;
	}
}






/*
  -1	失败
   0	成功
*/
int ReadMessageQueue(struct pidinfo * ppid, struct ipcinfo *ipc)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
		ret = MsgQ_Rcv(ipc[IPC_CURSOR_MANAGER].ipc_msgq_id, &msg);//接收消息
		if(ret < 0)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Failed to read message queue");
			return -1;
		}
	//	printf("msg.mtype is %d\n",msg.mtype);
		if(ret > 0)//有消息
		{
			if(MSG_CFG_DBG == msg.mtype)
			{
				if(0 == sysLevel(msg.mdata, ipc))//设置打印级别
				{
					return -1;
				}
			}
			else if(MSG_CFG_REBOOT == msg.mtype)
			{
				if(0 == sysReboot(msg.mdata, ppid, ipc))//重启P350
				{
					return -1;
				}
			}
			else if(MSG_CFG_GUARD == msg.mtype)//
			{
				if (strcmp(msg.mdata,"on") == 0){//设置监控标志为1
					gMgrCtx.guard_flag = 1;

				}else{
					gMgrCtx.guard_flag = 0;
				}
			}
			else if(MSG_CFG_UPDATE == msg.mtype)
			{
				if(0 == sysUpdate(msg.mdata, ppid, ipc))//升级P350
				{
					return -1;
				}
			}else if (MSG_CFG_GUARD== msg.mtype)
			{
				
			}
			else
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Invalid message type");
				return -1;
			}
		}
	}while(ret > 0);

	return 0;
}






/*
  1	成功
  0	失败
*/
int sysLevel(char *data, struct ipcinfo *ipc)
{
	struct dbginfo *dbg;

	dbg = (struct dbginfo *)data;
	if(0 == set_print_level(dbg->syslog_en, dbg->dbg_level,DAEMON_CURSOR_MANAGER))
	{
		print(	DBG_ERROR, 
				"%s","Failed to set print level.");
		
		return 0;
	}
	print(DBG_INFORMATIONAL, 
		  "--sys syslog:%d dbg_level:%d", dbg->syslog_en,
		  dbg->dbg_level);
	return 1;
}


/*
  true:		重启设备
  false:	不重启设备
*/
bool_t isReboot(struct rebootinfo *reboot)
{
	return((1 == reboot->bReboot)? true : false);
}



/*
  1	成功
  0	失败
*/
int sysReboot(char *data, struct pidinfo *pid, struct ipcinfo *ipc)
{
	struct rebootinfo *reboot;

	reboot = (struct rebootinfo *)data;
	if(isReboot(reboot))
	{
		system("p350reboot&");	
	}

	return 1;
}


/*
  true:		更新设备程序
  false:	不更新设备程序
*/
bool_t isUpdate(struct updateinfo *update)
{
	return((1 == update->bUpdate)? true : false);
}




/*
  1	成功
  0	失败
*/
int sysUpdate(char *data, struct pidinfo *pid, struct ipcinfo *ipc)
{
	//DIR *dir;
	//char fileApp[64];
	//char fileWww[64];
	//char fileFpga[64];
	//char fileDb[64];
	//char path[128];
	//struct dirent *entry;
	struct updateinfo *update;

	update = (struct updateinfo *)data;
	if(isUpdate(update))
	{
		if(-1 == system("/usr/p350/md5sum.sh"))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Failed to execute md5sum.sh");

			return 0;
		}
		#if 0
		if(-1 == chdir("/usr/p350/"))
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Failed to change directory." );
			
			return 0;
		}
				
		dir = opendir("/usr/p350/tmp");
		if(NULL == dir)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Failed to open directory." );
			
			return 0;
		}

		/*update integrate */

		while(1)
		{
			errno = 0;
			if((entry = readdir(dir)) == NULL)
			{
				closedir(dir);
				if(0 == errno)
				{
					break;
				}
				else
				{
					print(	DBG_ERROR, 
							"<%s>--%s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							"Failed to read directory." );
					
					return 0;
				}
			}
			
			if (0 == memcmp(entry->d_name, "p350update", 10) ) 
			{
			//	memset(path, 0, 128);
				sprintf(path, "tar jxf tmp/%s -C tmp", entry->d_name);
				if(-1 == system(path))
				{
					print(	DBG_ERROR, 
							"<%s>--Failed to execute \"%s\"", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							path);

					return 0;
				}
			}
		}
	
		memset(fileApp, 0, 64);
		memset(fileWww, 0, 64);
		memset(fileFpga, 0, 64);
		memset(fileDb, 0, 64);

		dir = opendir("/usr/p350/tmp");
		if(NULL == dir)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_MANAGER], 
					"Failed to open directory." );
			
			return 0;
		}
		while(1)
		{
			errno = 0;
			if((entry = readdir(dir)) == NULL)
			{
				closedir(dir);
				if(0 == errno)
				{
					break;
				}
				else
				{
					print(	DBG_ERROR, 
							"<%s>--%s", 
							gDaemonTbl[DAEMON_CURSOR_MANAGER], 
							"Failed to read directory." );
					
					return 0;
				}
			}
			else if(0 == memcmp(entry->d_name, "app", 3))
			{
				memcpy(fileApp, entry->d_name, strlen(entry->d_name));
			}
			else if(0 == memcmp(entry->d_name, "fpga", 4))
			{
				memcpy(fileFpga, entry->d_name, strlen(entry->d_name));
			}
			else if(0 == memcmp(entry->d_name, "www", 3))
			{
				memcpy(fileWww, entry->d_name, strlen(entry->d_name));
			}
			else if(0 == memcmp(entry->d_name, "db", 2))
			{
				memcpy(fileDb, entry->d_name, strlen(entry->d_name));
			}
			else
			{
				//do nothing
			}
		}

		if(0 != strlen(fileApp))
		{
			if(-1 == system("rm -fR bak/app.tar.bz2"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR bak/app.tar.bz2\"");
				
				return 0;
			}

			if(-1 == system("tar jcf app.tar.bz2 app"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"tar jcf app.tar.bz2 app\"");

				return 0;
			}
				
			if(-1 == system("mv app.tar.bz2 bak"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"mv app.tar.bz2 bak\"");

				return 0;
			}
				
			if(-1 == system("rm -fR app"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR app\"");

				return 0;
			}

			memset(path, 0, 128);
			sprintf(path, "tar jxf tmp/%s -C .", fileApp);
			if(-1 == system(path))
			{
				print(	DBG_ERROR, 
						"<%s>--Failed to execute \"%s\"", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						path);

				return 0;
			}

			if(-1 == system("chmod -R 777 app"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"chmod -R 777 app\"");

				return 0;
			}

			if(-1 == system("rm -f /bin/p350*"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -f /bin/p350*\"");

				return 0;
			}

			if(-1 == system("cp app/cmd/* /bin/"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"cp app/cmd/* /bin/\"");

				return 0;
			}
		}

		if(0 != strlen(fileWww))
		{
			if(-1 == system("rm -fR bak/www.tar.bz2"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR bak/www.tar.bz2\"");
				
				return 0;
			}
				
			if(-1 == system("tar jcf www.tar.bz2 www"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"tar jcf www.tar.bz2 www\"");

				return 0;
			}
				
			if(-1 == system("mv www.tar.bz2 bak"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"mv www.tar.bz2 bak\"");

				return 0;
			}
				
			if(-1 == system("rm -fR www"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR www\"");

				return 0;
			}

			memset(path, 0, 128);
			sprintf(path, "tar jxf tmp/%s -C .", fileWww);
			if(-1 == system(path))
			{
				print(	DBG_ERROR, 
						"<%s>--Failed to execute \"%s\"", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						path);

				return 0;
			}

			if(-1 == system("chmod -R 777 www"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"chmod -R 777 www\"");

				return 0;
			}
		}

		if(0 != strlen(fileFpga))
		{
			if(-1 == system("rm -fR bak/fpga.rbf"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR bak/fpga.rbf\"");
				
				return 0;
			}
				
			if(-1 == system("mv fpga/fpga.rbf bak"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"mv fpga/fpga.rbf bak\"");

				return 0;
			}

			if(-1 == system("cp tmp/fpga*.rbf fpga"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"cp tmp/fpga*.rbf fpga\"");

				return 0;
			}

			if(-1 == system("mv fpga/fpga* fpga/fpga.rbf"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"mv fpga/fpga* fpga/fpga.rbf\"");

				return 0;
			}

			if(-1 == system("chmod -R 777 fpga"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"chmod -R 777 fpga\"");

				return 0;
			}
		}

		if(0 != strlen(fileDb))
		{
			if(-1 == system("rm -fR bak/db.tar.bz2"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR bak/db.tar.bz2\"");
				
				return 0;
			}

			if(-1 == system("tar jcf db.tar.bz2 db"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"tar jcf db.tar.bz2 db\"");

				return 0;
			}
				
			if(-1 == system("mv db.tar.bz2 bak"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"mv db.tar.bz2 bak\"");

				return 0;
			}
				
			if(-1 == system("rm -fR db"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"rm -fR db\"");

				return 0;
			}

			memset(path, 0, 128);
			sprintf(path, "tar jxf tmp/%s -C .", fileDb);
			if(-1 == system(path))
			{
				print(	DBG_ERROR, 
						"<%s>--Failed to execute \"%s\"", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						path);

				return 0;
			}

			if(-1 == system("chmod -R 777 db"))
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_MANAGER], 
						"Failed to execute \"chmod -R 777 db\"");

				return 0;
			}
		}

		if(	(0 != strlen(fileApp)) || 
			(0 != strlen(fileFpga)) || 
			(0 != strlen(fileWww)) || 
			(0 != strlen(fileDb)) )
		{
			//run  p350 reboot script
			system("p350reboot&");
		}
		else
		{
			return 0;
		}
	#endif
	}
	
	return 1;
}
















