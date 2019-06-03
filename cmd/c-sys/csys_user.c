#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_time.h"
#include "lib_log.h"

#include "alloc.h"
#include "csys_user.h"















/*
  -1	无效的用户名长度
  -2	用户名必须以字母开头
  -3	用户名必须由字母、数字、下划线组成
   0	有效的用户名
*/
s32_t username_validity(char *username)
{
	u16_t len1;
	s32_t i;

	len1 = strlen((const char *)username);
	//无效的用户名长度
	if( (len1 < USERNAME_LEN_MIN)||(len1 > USERNAME_LEN_MAX) )
	{
		return -1;
	}

	//用户名必须以字母开头
	if( ((username[0] < 'a')||(username[0] > 'z'))&&
		((username[0] < 'A')||(username[0] > 'Z')) )
	{
		return -2;
	}

	//由字母、数字、下划线组成
	for(i=1; i<len1; i++)
	{
		if( ((username[i]<'a') ||(username[i]>'z'))&&
			((username[i]<'A') ||(username[i]>'Z'))&&
			((username[i]<'0') ||(username[i]>'9'))&&
			('_' != username[i]) )
		{
			return -3;
		}
	}

	return 0;
}















/*
  -1	无效的密码长度
   0	有效的密码
*/
s32_t password_validity(char *password)
{
	int len;

	len = strlen(password);
	if( (len < PASSWORD_LEN_MIN)||(len > PASSWORD_LEN_MAX) )
	{
		//无效的密码长度
		return -1;
	}

	//有效的密码
	return 0;
}

/*
  -1	无效的用户权限
   0	有效的用户权限
*/
s32_t permission_validity(u8_t user_permission)
{
	if (user_permission > MINIMUM_PERMISSION || user_permission < MAXIMUM_PERMISSION) {
		//无效的用户权限
		return -1;
	}
	//有效的用户权限
	return 0;
}


/*
  -1	无法获取用户名信息
   0	用户名不存在
   1	用户名存在
*/
s32_t username_existence(sqlite3 *pDb, char *tbl_user, char *username)
{
	char sql[128];
	char **resultp;
	int nrow;
	int ncolumn;
	bool_t ret1;
	s32_t ret2;
	
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s where username=\'%s\';", tbl_user, username);
	db_lock(pDb);
	ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		//无法获取用户名信息
		ret2 = -1;
	}
	else
	{
		if(0 == atoi(resultp[1]))
		{
			//用户名不存在
			ret2 = 0;
		}
		else
		{
			//用户名存在
			ret2 = 1;
		}
		db_free_table(resultp);
	}

	db_unlock(pDb);
	return ret2;
}















/*
  -1		无法获取用户表的记录数量
  [1,20]	用户表的记录数量
*/
s32_t username_count(sqlite3 *pDb, char *tbl_user)
{
	char sql[128];
	char **resultp;
	int nrow;
	int ncolumn;
	bool_t ret1;
	s32_t ret2;
	
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select count(*) from %s;", tbl_user);
	db_lock(pDb);
	ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		//无法获取用户表的记录数量
		ret2 = -1;
	}
	else
	{
		ret2 = atoi(resultp[1]);
		db_free_table(resultp);
	}

	db_unlock(pDb);
	return ret2;
}

















/*
  -1	无法打开数据库
  -2	无法获取用户名信息
  -3	用户名已经存在
  -4	无法获取用户表的记录数量
  -5	用户表的记录数量大于等于20
  -6	添加用户失败
   0	添加用户成功
*/
s32_t add_user(char *pDbPath, char *tbl_user, char *username, char *password, u8_t permission)
{
	sqlite3 *pDb = NULL;
	char sql[128];
	bool_t ret1;
	s32_t ret2;
	
	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}
	else
	{
		ret2 = username_existence(pDb, tbl_user, username);
		if( -1 == ret2 )
		{
			//无法获取用户名信息
			db_close(pDb);
			return -2;
		}
		else if( 1 == ret2 )
		{
			//用户名已经存在
			db_close(pDb);
			return -3;
		}
		else
		{
			ret2 = username_count(pDb, tbl_user);
			if( -1 == ret2 )
			{
				//无法获取用户表的记录数量
				db_close(pDb);
				return -4;
			}
			if(ret2 >= USER_NUM_MAX)
			{
				//用户表的记录数量大于等于20
				db_close(pDb);
				return -5;
			}
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "insert into %s(username,password,permission) values(\'%s\',\'%s\',%d);",
				tbl_user, username, password, permission);
			db_lock(pDb);
			ret1 = db_put_table(pDb, sql);
			if(false == ret1)
			{
				db_unlock(pDb);
				db_close(pDb);
				//添加用户失败
				return -6;
			}
			else
			{
				db_unlock(pDb);
				db_close(pDb);
				//添加用户成功
				return 0;
			}
		}
	}
}














/*
  -1	无法打开数据库
  -2	无法获取用户名信息
  -3	用户名不存在
  -4	无法获取用户表的记录数量
  -5	用户表的记录数量小于等于1
  -6	删除用户失败
   0	删除用户成功
*/
s32_t delete_user(char *pDbPath, char *tbl_user, char *username)
{
	sqlite3 *pDb = NULL;
	char sql[128];
	bool_t ret1;
	s32_t ret2;

	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}
	else
	{
		ret2 = username_existence(pDb, tbl_user, username);
		if( -1 == ret2 )
		{
			//无法获取用户名信息
			db_close(pDb);
			return -2;
		}
		else if( 0 == ret2 )
		{
			//用户名不存在
			db_close(pDb);
			return -3;
		}
		else
		{
			ret2 = username_count(pDb, tbl_user);
			if( -1 == ret2 )
			{
				//无法获取用户表的记录数量
				db_close(pDb);
				return -4;
			}
			if(ret2 <= USER_NUM_MIN)
			{
				//用户表的记录数量小于等于1
				db_close(pDb);
				return -5;
			}
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "delete from %s where username=\'%s\';", tbl_user, username);
			db_lock(pDb);
			ret1 = db_put_table(pDb, sql);
			if(false == ret1)
			{
				db_unlock(pDb);
				db_close(pDb);
				//删除用户失败
				return -6;
			}
			else
			{
				db_unlock(pDb);
				db_close(pDb);
				//删除用户成功
				return 0;
			}
		}
	}
}














/*
  -1	无法打开数据库
  -2	无法获取用户名信息
  -3	用户名不存在
  -4	无法获取用户名、密码信息
  -5	验证用户失败
   0	验证用户成功
*/
s32_t check_user(char *pDbPath, char *tbl_user, char *username, char *password)
{
	sqlite3 *pDb = NULL;
	char sql[128];
	bool_t ret1;
	s32_t ret2;
	char **resultp;
	int nrow;
	int ncolumn;

	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}
	else
	{
		ret2 = username_existence(pDb, tbl_user, username);
		if( -1 == ret2 )
		{
			//无法获取用户名信息
			db_close(pDb);
			return -2;
		}
		else if( 0 == ret2 )
		{
			//用户名不存在
			db_close(pDb);
			return -3;
		}
		else
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "select count(*) from %s where username=\'%s\' and password=\'%s\';", tbl_user, username, password);
			db_lock(pDb);
			ret1 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
			if(false == ret1)
			{
				db_unlock(pDb);
				db_close(pDb);
				//无法获取用户名、密码信息
				return -4;
			}
			else
			{
				ret2 = atoi(resultp[1]);
				db_free_table(resultp);
				db_unlock(pDb);
				db_close(pDb);
				if(1 == ret2)
				{
					//验证用户成功
					return 0;
				}
				else
				{
					//验证用户失败
					return -5;
				}
			}
		}
	}
}















/*
  -1	无法打开数据库
  -2	无法获取用户名信息
  -3	用户名不存在
  -4	无法获取用户名、密码信息
  -5	验证用户失败
  -6	修改密码失败
   0	修改密码成功
*/
s32_t modify_user( 
	char *pDbPath, 
	char *tbl_user, 
	char *original_username, 
	char *original_password, 
	char *new_password )
{
	s32_t ret1;
	bool_t ret2;
	char sql[128];
	sqlite3 *pDb = NULL;
	
	ret1 = check_user(pDbPath, tbl_user, original_username, original_password);
	if(0 != ret1)
	{
		return ret1;
	}

	ret2 = db_open(pDbPath, &pDb);
	if( false == ret2 )
	{
		//无法打开数据库
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set password=\'%s\' where username=\'%s\';", tbl_user, new_password, original_username);
	db_lock(pDb);
	ret2 = db_put_table(pDb, sql);
	db_unlock(pDb);
	db_close(pDb);
	if(false == ret2)
	{
		//修改密码失败
		return -6;
	}
	else
	{
		//修改密码成功
		return 0;
	}
}

/*
  -1	无法打开数据库
  -2	无法获取用户名信息
  -3	用户名不存在
  -4	无法获取用户名、密码信息
  -5	验证用户失败
  -6	修改密码失败
   0	修改密码成功
*/
s32_t modify_user_mod( 
	char *pDbPath, 
	char *tbl_user, 
	char *username, 
	u8_t new_permission)
{
	bool_t ret1;
	s32_t ret2;
	char sql[128];
	sqlite3 *pDb = NULL;

	ret1 = db_open(pDbPath, &pDb);
	if( false == ret1 )
	{
		//无法打开数据库
		return -1;
	}

	ret2 = username_existence(pDb, tbl_user, username);
	if( -1 == ret2 )
	{
		//无法获取用户名信息
		db_close(pDb);
		return -2;
	}
	else if( 0 == ret2 )
	{
		//用户名不存在
		db_close(pDb);
		return -3;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set permission=\'%d\' where username=\'%s\';", tbl_user, new_permission, username);
	db_lock(pDb);
	ret2 = db_put_table(pDb, sql);
	db_unlock(pDb);
	db_close(pDb);
	if(false == ret2)
	{
		//修改权限失败
		return -4;
	}
	else
	{
		//修改权限成功
		return 0;
	}
}












/*
  -1	无法打开数据库
  -2	无法获取用户表记录
   0	成功获取用户表记录
*/
s32_t query_user(char *pDbPath, char *tbl_user, struct userinfo *users, u16_t *unum)
{
	bool_t ret2;
	char sql[128];
	sqlite3 *pDb = NULL;
	char **resultp;
	int nrow;
	int ncolumn;
	int i,j;

	ret2 = db_open(pDbPath, &pDb);
	if( false == ret2 )
	{
		//无法打开数据库
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select * from %s;", tbl_user);
	db_lock(pDb);
	ret2 = db_get_table(pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret2)
	{
		db_unlock(pDb);
		db_close(pDb);
		//无法获取用户表记录
		return -2;
	}
	else
	{
		for(i=1,j=0; i<(nrow+1); i++,j++)
		{
			strcpy((char *)users[j].username, (const char *)resultp[ncolumn * i]);
			strcpy((char *)users[j].password, (const char *)resultp[ncolumn * i +1]);
			users[j].permission =  (u8_t) atoi(resultp[ncolumn * i +2]);
		}
		db_free_table(resultp);
		db_unlock(pDb);
		db_close(pDb);
		*unum = nrow;
		return 0;
	}
}














/*
  -1	失败
   0	成功
*/
s32_t log_user(char *pDbPath, char *tbl_log, char *username)
{
	sqlite3 *pDb = NULL;
	char buf[256];

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1|%s", username);

	if(-1 == send_log_message(buf))
	{
		return -1;
	}
		
	if(!db_open(pDbPath, &pDb))
	{
		return -1;
	}
	else
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s %s", username, "login");
		if(0 == db_log_write(pDb, TBL_LOG, buf))
		{
			db_close(pDb);
			return -1;
		}
		else
		{
			db_close(pDb);
			return 0;
		}
	}
}











