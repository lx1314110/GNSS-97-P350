#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "lib_log.h"

#include "alloc.h"

#include "clog_alloc.h"
#include "clog_macro.h"
#include "clog_global.h"
#include "clog_query.h"
#include "clog_parser.h"












int argv_parser(int argc, char *argv[], struct logCtx *ctx)
{
	int ret = 0;
	int tmp;

	if(-1 == initializeDatabase(ctx))
	{
		printf("Failure.\n");
		return -1;
	}
	
	
	if(1 == argc)
	{
		tmp = log_pages(ctx->pDb, TBL_LOG);//日志页数
		if(-1 == tmp)
		{
			ret = -1;
		}
		else
		{
			printf("%d\n", tmp);
		}
	}
	else if(2 == argc)
	{
		if(0 == memcmp(argv[1], HELP, strlen(HELP)))
		{
			helper();
		}
		else if(0 == memcmp(argv[1], "delete", strlen("delete")))
		{
			char sql[128];
			bool_t ret1;

			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s;", TBL_LOG);
			db_lock(ctx->pDb);
			ret1 = db_put_table(ctx->pDb, sql);
			db_unlock(ctx->pDb);
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
		else if(page_validity(argv[1]))
		{
			tmp = log_pages(ctx->pDb, TBL_LOG);
			if(-1 == tmp)
			{
				ret = -1;
			}
			else
			{
				if(atoi(argv[1]) <= tmp)
				{
					if(-1 == log_query(ctx->pDb, TBL_LOG, atoi(argv[1])))//日志查询
					{
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

	//clean
	cleanDatabase(ctx);
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










/*
  1	合法
  0	非法
*/
int page_validity(char *page)
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
			((atoi(page) >= 1) && (atoi(page) <= MAX_LOG/RECORD_PER_PAGE)) )
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
		int record_num = MAX_LOG;
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









void helper(void)
{
	int i;

	for(i=0; gHelpTbl[i]; i++)
	{
		printf("%s\n", gHelpTbl[i]);
	}
}









