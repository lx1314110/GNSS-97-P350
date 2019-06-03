#ifndef __CSYS_USER__
#define __CSYS_USER__







#include "lib_type.h"
#include "lib_sqlite.h"







//用户名、密码长度范围
#define	USERNAME_LEN_MIN	 5
#define USERNAME_LEN_MAX	18

#define PASSWORD_LEN_MIN	 5
#define PASSWORD_LEN_MAX	18


//用户数量范围
#define USER_NUM_MIN		 1
#define USER_NUM_MAX		20


struct userinfo {
	char username[USERNAME_LEN_MAX];
	char password[PASSWORD_LEN_MAX];
	u8_t permission;
};







s32_t username_validity(char *username);
s32_t password_validity(char *password);
s32_t permission_validity(u8_t user_permission);

s32_t username_existence(sqlite3 *pDb, char *tbl_user, char *username);
s32_t username_count(sqlite3 *pDb, char *tbl_user);






s32_t add_user(char *pDbPath, char *tbl_user, char *username, char *password, u8_t permission);
s32_t delete_user(char *pDbPath, char *tbl_user, char *username);
s32_t check_user(char *pDbPath, char *tbl_user, char *username, char *password);
s32_t modify_user( 
	char *pDbPath, 
	char *tbl_user, 
	char *original_username, 
	char *original_password, 
	char *new_password );
s32_t modify_user_mod( 
	char *pDbPath, 
	char *tbl_user, 
	char *username, 
	u8_t new_permission);

s32_t query_user(char *pDbPath, char *tbl_user, struct userinfo *users, u16_t *unum);



s32_t log_user(char *pDbPath, char *tbl_log, char *username);











#endif//__CSYS_USER__


