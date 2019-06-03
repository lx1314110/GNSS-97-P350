#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#include "lib_sema.h"
#include "lib_shm.h"
#include "lib_fpga.h"
#include "lib_bit.h"
#include "lib_common.h"
#include "lib_log.h"
#include "lib_sqlite.h"

#include "alloc.h"
#include "csys_common.h"
#include "csys_phase_upload.h"

#define	HISTORY				"history"
#define	CURRENT				"current"

#define	HISTORYNUM		 	"hstynum"
#define	CURRENTNUM		 	"curnum"

#define	RECORD_PER_PAGE		CMD_READ_TBL_TABLE_PER_PAGE_MAX_LEN


#undef DEBUG
//#define DEBUG

void ph_upload_helper(void);
int ph_upload_page_validity(char *, int);
int ph_upload_pages(sqlite3 *pDb, char *pTbl);
int ph_upload_nums(sqlite3 *pDb, char *pTbl);
int current_ph_upload_query(sqlite3 * pDb, char * pTbl, int page);
int history_ph_upload_query(sqlite3 * pDb, char * pTbl, int page);

//========================================================================
static int phase_upload_en_argv_parser(int argc, char *argv[]);

/*
  1	合法
  0	非法
*/
int ph_upload_page_validity(char *page, int record_num)
{
	int len;
	int ret = 0;

	len = strlen(page);
#if 0
	if(1 == len)
	{
		if((page[0] >= '1') && (page[0] <= '9'))
		{
			ret = 1;
		}
	}
	else if(2 == len)
	{
		if( ((page[0] >= '1') && (page[0] <= '9')) &&
			((page[1] >= '0') && (page[1] <= '9')) &&
			((atoi(page) >= 1) && (atoi(page) <= MAX_ALM/RECORD_PER_PAGE)) )
		{
			ret = 1;
		}
	}
	else
	{
		//do nothing
	}
#else
	int i = 0;
	ret = 1;
	for (i = 0; i < len; i++){
		if(isdigit(page[i]) == 0)
			ret = 0;
	}
	
	int max_page = 0;
	if(record_num % RECORD_PER_PAGE == 0)
		max_page = record_num/RECORD_PER_PAGE;
	else
		max_page = record_num/RECORD_PER_PAGE +1;
	if( atoi(page) < 1 || atoi(page) > max_page){
		ret = 0;
	}
#endif
	return ret;
}


/*
  >0	成功
  -1	失败
*/
int ph_upload_pages(sqlite3 *pDb, char *pTbl)
{
	char sql[64];
	char **resultp;
	int nrow;
	int ncolumn;
	int ret = 0;
	int page = 0;
	if((NULL == pDb) || (NULL == pTbl))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录数量
		return -1;
	}
	else
	{
		page = atoi(resultp[1]);
		if(page%RECORD_PER_PAGE == 0)
			ret = page/RECORD_PER_PAGE;
		else
			ret = page/RECORD_PER_PAGE +1;
		db_free_table(resultp);
		db_unlock(pDb);
		
		return ret;
	}
}



/*
  >0	成功
  -1	失败
*/
int ph_upload_nums(sqlite3 *pDb, char *pTbl)
{
	char sql[64];
	char **resultp;
	int nrow;
	int ncolumn;
	int ret;

	if((NULL == pDb) || (NULL == pTbl))
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", pTbl);
	
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录数量
		return -1;
	}
	else
	{
		ret = atoi(resultp[1]);
		db_free_table(resultp);
		db_unlock(pDb);
		
		return ret;
	}
}

/*
   0	成功
  -1	失败
*/
int current_ph_upload_query(sqlite3 * pDb, char * pTbl, int page)
{
	char sql[512];
	char **resultp;
	int nrow;
	int ncolumn;
	int i;
	int ph_index;

	if(NULL == pDb || NULL == pTbl || page < 1)
	{
		return -1;
	}

	memset(sql, 0, sizeof(sql));
#if 1
	#if 1
	//倒序排列	
	sprintf(sql, "select ID, phase_inx, phase_val, slot, port, clock_status, phase_time, is_read \
				  from %s order by rowid desc limit %d offset %d; \
				  update %s set is_read=1 where is_read=0;",
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE,
				  pTbl);
	#else
		//顺序排列
	sprintf(sql, "select ID, phase_inx, phase_val, slot, port, clock_status, phase_time, is_read \
				  from %s limit %d offset %d; \
				  update %s set is_read=1 where is_read=0;",
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE,
				  pTbl);
	#endif
#else
	no is_read
	sprintf(sql, "select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
				  from %s where is_read=0 order by rowid desc limit %d offset %d; \
				  update %s set is_read=1 where is_read=0;", 
				  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE,
				  pTbl);
#endif
	db_lock(pDb);
	if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
	{
		db_unlock(pDb);
		//无法获取记录
		return -1;
	}

	for(i=1; i<(nrow+1); i++)
	{

		//new: ID(KEY) | phase_inx | phase_val | slot | port | clock_status | phase_time |  is_read
		ph_index = atoi(resultp[ncolumn *i +1]);
		if (ph_index < 0 || ph_index > gPhasePerfTbl_len){//1-60
			db_free_table(resultp);
			db_unlock(pDb);
			return -1;
		}
		//clock status
		char *clksta = NULL;
		switch (atoi(resultp[ncolumn *i +5])){
			case 0:
				clksta = "Free";
				break;
			case 1:
			case 2:
				clksta = "Hold";
				break;
			case 3:
				clksta = "Fast";
				break;
			case 4:
				clksta = "Lock";
				break;
			default:
				clksta = "Free";
			}
		#if 0
		//no is_read
		//new: ID(KEY) | phase_inx | phase_val | slot | port | clock_status | phase_time
		printf( "%s | %s | %s | %s | %s | %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					resultp[ncolumn *i +2],
					resultp[ncolumn *i +3],
					resultp[ncolumn *i +4],
					clksta,
					resultp[ncolumn *i +6]);
		#else
		//new: ID(KEY) | phase_inx | phase_val | slot | port | clock_status | phase_time |  is_read 
		printf( "%s | %s | %s | %s | %s | %s | %s | %s\n", 
					resultp[ncolumn *i +0],
					resultp[ncolumn *i +1],
					resultp[ncolumn *i +2],
					resultp[ncolumn *i +3],
					resultp[ncolumn *i +4],
					clksta,
					resultp[ncolumn *i +6],
					resultp[ncolumn *i +7]);

		#endif
	}
	db_free_table(resultp);
	db_unlock(pDb);
	return 0;

}


/*
   0	成功
  -1	失败
*/
int history_ph_upload_query(sqlite3 * pDb, char * pTbl, int page)
{
		char sql[512];
		char **resultp;
		int nrow;
		int ncolumn;
		int i;
		//int ph_index;
	
		if(NULL == pDb || NULL == pTbl || page < 1)
		{
			return -1;
		}
	
		memset(sql, 0, sizeof(sql));

	#if 1
	//倒序排列
		sprintf(sql, "select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
			  from %s order by rowid desc limit %d offset %d;", 
			  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE);
	#else
	//顺序排列
		sprintf(sql, "select ID, phase_inx, phase_val, slot, port, clock_status, phase_time \
			  from %s limit %d offset %d;", 
			  pTbl, RECORD_PER_PAGE, (page -1)*RECORD_PER_PAGE);
	#endif

		db_lock(pDb);
		if(!db_get_table(pDb, sql, &resultp, &nrow, &ncolumn))
		{
			db_unlock(pDb);
			//无法获取记录
			return -1;
		}
	
		for(i=1; i<(nrow+1); i++)
		{
	
			//new: ID(KEY) | phase_inx | phase_val | slot | port | clock_status | phase_time |is_read
			#if 0
			ph_index = atoi(resultp[ncolumn *i +1]);
			if (ph_index < 0 || ph_index > gPhasePerfTbl_len){//1-60
				db_free_table(resultp);
				db_unlock(pDb);
				return -1;
			}
			#endif
			//clock status
			char *clksta = NULL;
			switch (atoi(resultp[ncolumn *i +5])){
				case 0:
					clksta = "Free";
					break;
				case 1:
				case 2:
					clksta = "Hold";
					break;
				case 3:
					clksta = "Fast";
					break;
				case 4:
					clksta = "Lock";
					break;
				default:
					clksta = "Free";
			}
		#if 1
			//no is_read
			//new: ID(KEY) | phase_inx | phase_val | slot | port | clock_status | phase_time
			printf( "%s | %s | %s | %s | %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						resultp[ncolumn *i +1],
						resultp[ncolumn *i +2],
						resultp[ncolumn *i +3],
						resultp[ncolumn *i +4],
						clksta,
						resultp[ncolumn *i +6]);
		#else
			//new: ID(KEY) | phase_inx | phase_val | slot | port | clock_status | phase_time |	is_read 
			printf( "%s | %s | %s | %s | %s | %s | %s | %s\n", 
						resultp[ncolumn *i +0],
						resultp[ncolumn *i +1],
						resultp[ncolumn *i +2],
						resultp[ncolumn *i +3],
						resultp[ncolumn *i +4],
						clksta,
						resultp[ncolumn *i +6],
						resultp[ncolumn *i +7]);
	
		#endif
		}
		db_free_table(resultp);
		db_unlock(pDb);
		return 0;

}

/*
  -1	失败
   0	成功
*/
static int phase_upload_en_argv_parser(int argc, char *argv[])
{
	struct sys_conf_t sys_conf;
	struct ipcinfo ipc;
	struct config update;
	struct msgbuf msg;
	struct pidinfo pid;

	ipc.ipc_msgq_id = MsgQ_Init(MSGQ_PATH, MSGQ_PROJ_ID_INPUT);
	if(-1 == ipc.ipc_msgq_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_shm_id = shm_create(SHM_PATH, SHM_PROJ_ID_INPUT);
	if(-1 == ipc.ipc_shm_id)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_base = shm_attach(ipc.ipc_shm_id);
	if(((void *)-1) == ipc.ipc_base)
	{
		printf("Failure.\n");
		return -1;
	}

	ipc.ipc_sem_id = sema_open(SEMA_PATH, SEMA_PROJ_ID);
	if(-1 == ipc.ipc_sem_id)
	{
		printf("Failure.\n");
		return -1;
	}
	
	if(2 == argc)//从共享内存读系统配置	
	{
		if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
		{
			printf("Failure.\n");
			return -1;
		}
		shm_read(ipc.ipc_base, 
				 SHM_OFFSET_SYS_CONF_SWI, 
				 sizeof(struct sys_conf_t), 
				 (char *)&sys_conf, 
				 sizeof(struct sys_conf_t));
		if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
		{
			printf("Failure.\n");
			return -1;
		}

		if(sys_conf.ph_upload_en)
			printf("%s\n","on");
		else
			printf("%s\n","off");
			
		SHM_DETACH(ipc.ipc_base);
		return 0;
	}
	else if(3 == argc)//enable/disalb
	{
		if((switch_validity(argv[2])))
		{
			if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			shm_read(ipc.ipc_base, 
					 SHM_OFFSET_SYS_CONF_SWI, 
					 sizeof(struct sys_conf_t), 
					 (char *)&sys_conf, 
					 sizeof(struct sys_conf_t));
			if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}

			memset(&update, 0, sizeof(struct config));
			//backup other
			memcpy(&update.sys_config, &sys_conf, sizeof(struct sys_conf_t));
			//set
			if (memcmp(argv[2], "on", strlen("on")) == 0){
				update.sys_config.ph_upload_en = 1;
			}
			if (memcmp(argv[2], "off", strlen("off")) == 0){
				update.sys_config.ph_upload_en = 0;
			}
			
			memset(&msg, 0, sizeof(struct msgbuf));
			msg.mtype = MSG_CFG_SYS_CONF_SWI;//系统配置
			memcpy(msg.mdata, update.buffer, sizeof(struct sys_conf_t));//更新配置

			if(1 != sema_lock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			shm_read(ipc.ipc_base, 
					 SHM_OFFSET_PID, 
					 sizeof(struct pidinfo), 
					 (char *)&pid, 
					 sizeof(struct pidinfo));//从共享内存读PID
			if(1 != sema_unlock(ipc.ipc_sem_id, SEMA_MEMBER_INPUT))
			{
				printf("Failure.\n");
				return -1;
			}
			
			if(MsgQ_Send(ipc.ipc_msgq_id, &msg, sizeof(struct sys_conf_t)))//发送配置消息
			{
				if(notify(pid.p_id, NOTIFY_SIGNAL))//发送通知信号
				{
					SHM_DETACH(ipc.ipc_base);
					printf("Success.\n");
					return 0;
				}
				else
				{
					printf("Failure.\n");
					return -1;
				}
			}
			else
			{
				printf("Failure.\n");
				return -1;
			}
		}
		else
		{
			printf("Failure.\n");
			return -1;
		}
	}
	else
	{
		ph_upload_helper();
		return -1;
	}
}

/*
  -1	失败
   0	成功
*/
int ph_upload_argv_parser(int argc, char *argv[])
{
	int ret = 0;
	int tmp;
	sqlite3 *pDb = NULL;

	//HELP
	if(3 == argc){
		if(0 == memcmp(argv[2], HELP, strlen(HELP)) )
		{
			ph_upload_helper();
			return -1;
		}
	}

	initializeBoardNameTable();

	//initialize
	if(1 != initializeDatabase(&pDb))
	{
		ret = -1;
		goto exit0;
	}

	if(2 == argc){
		return phase_upload_en_argv_parser(argc, argv);
	}
	else if(3 == argc)
	{
		if(switch_validity(argv[2])){
			return phase_upload_en_argv_parser(argc, argv);
		}
		else if(0 == memcmp(argv[2], CURRENT, strlen(CURRENT)))
		{
			tmp = ph_upload_pages(pDb, TBL_PERF_CURRENT);//获取记录的页数
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[2], HISTORY, strlen(HISTORY)))
		{
			tmp = ph_upload_pages(pDb, TBL_PERF_HISTORY);//获取记录的页数
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[2], CURRENTNUM, strlen(CURRENTNUM)))
		{
			tmp = ph_upload_nums(pDb, TBL_PERF_CURRENT);//获取记录数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else if(0 == memcmp(argv[2], HISTORYNUM, strlen(HISTORYNUM)))
		{
			tmp = ph_upload_nums(pDb, TBL_PERF_HISTORY);//获取记录数量
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				printf("%d\n", tmp);
			}
		}
		else
		{
			ret = -1;
		}
	}
	else if(4 == argc)
	{
		if( (0 == memcmp(argv[2], CURRENT, strlen(CURRENT))) &&
				(ph_upload_page_validity(argv[3], TBL_PHASE_PERF_CURRENT_ARRAY_SIZE)) )
		{
			tmp = ph_upload_pages(pDb, TBL_PERF_CURRENT);
			if(-1 == tmp){
				ret = -1;
			}else{
				if(atoi(argv[3]) <= tmp){
					if(-1 == current_ph_upload_query(pDb, TBL_PERF_CURRENT, atoi(argv[3]))){//记录查询
						ret = -1;
					}else{
						ret = 0;
					}
				}else{
					ret = -1;
				}
			}
		}
		else if( (0 == memcmp(argv[2], HISTORY, strlen(HISTORY))) &&
				(ph_upload_page_validity(argv[3],TBL_PHASE_PERF_HISTORY_ARRAY_SIZE)) )
		{
			tmp = ph_upload_pages(pDb, TBL_PERF_HISTORY);
			if(-1 == tmp){
				ret = -1;
			}else{
				if(atoi(argv[3]) <= tmp){
					if(-1 == history_ph_upload_query(pDb, TBL_PERF_HISTORY, atoi(argv[3]))){//告警记录查询
						ret = -1;
					}else{
						ret = 0;
					}
				}else{
					ret = -1;
				}
			}
		}
		else if(0 == memcmp(argv[2], HISTORY, strlen(HISTORY)) &&
			0 == memcmp(argv[3], "delete", strlen("delete")))
		{
			char sql[128];
			bool_t ret1;

			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s;", TBL_PERF_HISTORY);
			db_lock(pDb);
			ret1 = db_put_table(pDb, sql);
			db_unlock(pDb);
			if(false == ret1)
			{
				printf("fail to delete log\n");
				ret = -1;
			}
			else
			{
				ret = 0;
			}
		}
		else
		{
			ret = -1;
		}
	}
	else
	{
		ret = -1;
	}

//exit1:
	//clean
	cleanDatabase(pDb);
exit0:
	if(-1 == ret)
	{
		printf("Failure.\n");
		return -1;
	}
	else
	{
		return 0;
	}
}



static char *gPhUploadHelpTbl[] = {
	"p350sys ph_upload --help",
	"p350sys ph_upload",
	"        query phase upload on/off",
	"p350sys ph_upload on/off",
	"        set phase upload on/off",
	"p350sys ph_upload current",
	"        query the cur-ph_upload page",
	"p350sys ph_upload current [PAGE]",
	"        query the cur-ph_upload list of cur PAGE",
	"p350sys ph_upload curnum",
	"        query the cur-ph_upload record num",
	"p350sys ph_upload history",
	"        query the history-ph_upload page",
	"p350sys ph_upload history [PAGE]",
	"        query the history-ph_upload list of cur PAGE",
	"p350sys ph_upload hstynum",
	"        query the history-ph_upload record num",
	"p350sys ph_upload history delete",
	"        delete all history-ph_upload",
	NULL
	};


void ph_upload_helper(void)
{
	int i;
	
	for(i=0; gPhUploadHelpTbl[i]; i++)
	{
		printf("%s\n", gPhUploadHelpTbl[i]);
	}
}

