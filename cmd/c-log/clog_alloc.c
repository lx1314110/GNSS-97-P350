#include "lib_sqlite.h"

#include "clog_alloc.h"










/*
   0	成功
  -1	失败
*/
int initializeDatabase(struct logCtx *ctx)
{
	if(db_open(DB_PATH, &ctx->pDb))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}










/*
   0	成功
  -1	失败
*/
int cleanDatabase(struct logCtx *ctx)
{
	if(db_close(ctx->pDb))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}










