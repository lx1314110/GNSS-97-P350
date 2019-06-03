#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_sqlite.h"
#include "csys_net.h"




/*
  -1	无法打开数据库
  -2	无法设置IP
  -3	无法设置MASK
  -4	无法设置GATEWAY
  -5	无法设置MAC
  -6	无法获取GATEWAY
   0	成功设置网络参数
*/
s32_t modify_net_param(char *pDbPath, char *tbl_sys, char * networkcard, struct netinfo *netp)
{
	sqlite3 *pDb = NULL;
	char sql[256];
	char tmp[64];
	bool_t ret1;
	char old_gateway[16];

	if('\0' != netp->ip[0])
	{
		if(0 == __Sock_SetIp(networkcard, netp->ip))
		{
			return -2;
		}
	}

	if('\0' != netp->mask[0])
	{
		if(0 == __Sock_SetMask(networkcard, netp->mask))
		{
			return -3;
		}
	}

	if('\0' != netp->mac[0])
	{
		if(0 == __Sock_SetMac(networkcard, netp->mac))
		{
			return -4;
		}
	}

	if('\0' != netp->gateway[0])
	{
		if(0 == __Sock_GetGateway(old_gateway))
		{
			if(0 == __Sock_SetGateway(networkcard, netp->gateway, netp->gateway))
			{
				return -3;
			}
		}
		else
		{
			if(0 == __Sock_SetGateway(networkcard, old_gateway, netp->gateway))
			{
				return -3;
			}
		}
	}

	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}
	
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "update %s set", tbl_sys);
	db_lock(pDb);
	
	if('\0' != netp->ip[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s ip=\'%s\';", tmp, netp->ip);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置IP
			return -2;
		}
	}

	if('\0' != netp->mask[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s mask=\'%s\';", tmp, netp->mask);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置MASK
			return -3;
		}
	}

	if('\0' != netp->gateway[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s gateway=\'%s\';", tmp, netp->gateway);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置GATEWAY
			return -4;
		}
	}

	if('\0' != netp->mac[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s mac=\'%s\';", tmp, netp->mac);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置MAC
			return -5;
		}
	}

	db_unlock(pDb);
	db_close(pDb);

	//成功设置网络参数
	return 0;
}
#if 0
int write_virtual_eth(char *pDbPath, char *tbl_sys, struct netinfo *netp)
{
	sqlite3 *pDb = NULL;
	char sql[256];
	bool_t ret1;
	
	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "update %s \
			  set v_ip=\'%s\', v_mask=\'%s\', v_gateway=\'%s\' where v_port=%d;", 
			 tbl_sys, 
			 netp->ip, 
			 netp->mask,
			 netp->gateway,
			 21 );
	
	db_lock(pDb);
	ret1 = db_put_table(pDb, sql);
	db_unlock(pDb);
	if(false == ret1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
#endif
int write_eth(char *pDbPath, char *tbl_sys, struct netinfo *netp)
{
	sqlite3 *pDb = NULL;
	char sql[256];
	char tmp[64];
	bool_t ret1;
	
	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}
	
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "update %s set", tbl_sys);
	db_lock(pDb);
	
	if('\0' != netp->ip[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s ip=\'%s\';", tmp, netp->ip);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置IP
			return -2;
		}
	}
	
	if('\0' != netp->mask[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s mask=\'%s\';", tmp, netp->mask);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置MASK
			return -3;
		}
	}

	if('\0' != netp->gateway[0])
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "%s gateway=\'%s\';", tmp, netp->gateway);
		ret1 = db_put_table(pDb, sql);
		if(false == ret1)
		{
			db_rollback(pDb);
			db_unlock(pDb);
			db_close(pDb);
			//无法设置GATEWAY
			return -4;
		}
	}
	
	db_unlock(pDb);
	db_close(pDb);

	//成功设置网络参数
	return 0;
}

/*
  -1	获取IP失败
  -2	获取MASK失败
  -3	获取GATEWAY失败
  -4	获取MAC失败
   0	成功
*/
s32_t query_net_param(char * networkcard, struct netinfo *netp)
{
	int ret = 0;
	if( 0 == __Sock_GetIp(networkcard, netp->ip) )
	{
		ret = -1;
	}

	if( 0 == __Sock_GetMask(networkcard, netp->mask) )
	{
		ret = -2;
	}

	if( 0 == __Sock_GetGateway(netp->gateway) )
	{
		ret = -3;
	}

	if( 0 == __Sock_GetMac(networkcard, netp->mac) )
	{
		ret = -4;
	}

	return ret;
}





