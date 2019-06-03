#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_dbg.h"
#include "lib_bit.h"
#include "lib_sqlite.h"

#include "dntp_alloc.h"
#include "dntp_config.h"
#include "dntp_socket.h"








/*
  @ctx		ntp context
  @tbl		md5 key table	

  1	成功
  0	失败

  read md5 key
*/
int md5_key_read(struct ntpCtx *ctx, char *tbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int i,j;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select md5_id, md5_key from %s;", tbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read md5 key.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}

	ctx->ntpSta.totalKey = nrow;
	
	for(i=1,j=0; i<(nrow+1); i++,j++)
	{
		ctx->ntpSta.mKey[j].md5Id = atoi(resultp[ncolumn * i]);
		ctx->ntpSta.mKey[j].md5Len = strlen(resultp[ncolumn * i +1]);
		memcpy(ctx->ntpSta.mKey[j].md5Key, resultp[ncolumn * i +1], ctx->ntpSta.mKey[j].md5Len);
		print(	DBG_INFORMATIONAL, 
				"<%s>--%d %d %s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				ctx->ntpSta.mKey[j].md5Id,
				ctx->ntpSta.mKey[j].md5Len,
				ctx->ntpSta.mKey[j].md5Key );
	}
		
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}










/*
  @ctx		ntp context
  @mk		md5 key
  @tbl		md5 key table

  1	成功
  0	失败

  add md5 key
*/
int md5_key_add(struct ntpCtx *ctx, struct ntpmk *mk, char *tbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "insert into %s(md5_id, md5_key) values(%d, \'%s\');", 
			 tbl, 
			 mk->md5Id, 
			 mk->md5Key );
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to add md5 key.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx		ntp context
  @mk		md5 key
  @tbl		md5 key table

  1	成功
  0	失败

  delete md5 key
*/
int md5_key_del(struct ntpCtx *ctx, struct ntpmk *mk, char *tbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "delete from %s where md5_id=%d;", 
			 tbl, 
			 mk->md5Id );
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to delete md5 key.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx		ntp context
  @mk		md5 key
  @tbl		md5 key table

  1	成功
  0	失败

  modify md5 key
*/
int md5_key_mod(struct ntpCtx *ctx, struct ntpmk *mk, char *tbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "update %s set md5_key=\'%s\' where md5_id=%d;", 
			 tbl, 
			 mk->md5Key, 
			 mk->md5Id );
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to modify md5 key.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx		ntp context
  @tbl		system table	

  1	成功
  0	失败

  read version enable
*/
int version_enable_read(struct ntpCtx *ctx, char *tbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select v2_enable, v3_enable, v4_enable from %s;", tbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read version enable.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	
	if('1' == resultp[3][0])
	{
		ctx->ntpSta.vEn.VnEnable |= BIT(0);
	}
	if('1' == resultp[4][0])
	{
		ctx->ntpSta.vEn.VnEnable |= BIT(1);
	}
	if('1' == resultp[5][0])
	{
		ctx->ntpSta.vEn.VnEnable |= BIT(2);
	}
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%02X", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			ctx->ntpSta.vEn.VnEnable );
		
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}










/*
  @ctx		ntp context
  @ve		version enable
  @tbl		system table	

  1	成功
  0	失败

  write version enable
*/
int version_enable_write(struct ntpCtx *ctx, struct ntpve *ve, char *tbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "update %s set v2_enable=\'%c\', v3_enable=\'%c\', v4_enable=\'%c\';", 
			 tbl, 
			 (((ve->VnEnable)&BIT(0))? '1' : '0'), 
			 (((ve->VnEnable)&BIT(1))? '1' : '0'), 
			 (((ve->VnEnable)&BIT(2))? '1' : '0') );
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write version enable.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx		ntp context
  @tbl		system table	

  1	成功
  0	失败

  read broadcast enable and interval
*/
int broadcast_enable_interval_read(struct ntpCtx *ctx, char *tbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select broadcast_enable, broadcast_interval from %s;", tbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read broadcast enable and interval.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}

	ctx->ntpSta.bCast.bcEnable = atoi(resultp[2]);
	ctx->ntpSta.bCast.bcInterval = atoi(resultp[3]);
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%d %d", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			ctx->ntpSta.bCast.bcEnable, 
			ctx->ntpSta.bCast.bcInterval );
		
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}










/*
  @ctx		ntp context
  @bc		broadcast enable and interval
  @tbl		system table	

  1	成功
  0	失败

  write broadcast enable and interval
*/
int broadcast_enable_interval_write(struct ntpCtx *ctx, struct ntpbc *bc, char *tbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "update %s set broadcast_enable=\'%c\', broadcast_interval=%d;", 
			 tbl, 
			 bc->bcEnable + '0', 
			 bc->bcInterval );
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write broadcast enable and interval.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx		ntp context
  @tbl		system table	

  1	成功
  0	失败

  read md5 enable
*/
int md5_enable_read(struct ntpCtx *ctx, char *tbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select md5_enable from %s;", tbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read md5 enable.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}

	ctx->ntpSta.mEn.md5Enable = atoi(resultp[1]);
	
	print(	DBG_INFORMATIONAL, 
			"<%s>--%d", 
			gDaemonTbl[DAEMON_CURSOR_NTP], 
			ctx->ntpSta.mEn.md5Enable );
		
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}










/*
  @ctx		ntp context
  @bc		md5 enable
  @tbl		system table	

  1	成功
  0	失败

  write md5 enable
*/
int md5_enable_write(struct ntpCtx *ctx, struct ntpme *me, char *tbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf( sql,
			 "update %s set md5_enable=%d;", 
			 tbl, 
			 me->md5Enable );
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write md5 enable.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}









/*
  @ctx		ntp context
  @tbl		virtual ether table	

  1	成功
  0	失败

  read virtual ether
*/
int read_virtual_eth(struct ntpCtx *ctx, char *tbl)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	int i,j,k;
	char networkcard[20];
	int im[4];
	u8_t mac[6];
	
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select v_ip, v_mac, v_mask, v_gateway from %s;", tbl);
	
	db_lock(ctx->pVethDb);
	ret1 = db_get_table(ctx->pVethDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pVethDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read virtual ether.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	
	for(i=1,j=0; i<(nrow+1); i++,j++)
	{
		ctx->ntpSta.vp[j].v_port = j+1;
		memcpy(ctx->ntpSta.vp[j].v_ip, resultp[ncolumn * i], strlen(resultp[ncolumn * i]));
		memcpy(ctx->ntpSta.vp[j].v_mac, resultp[ncolumn * i +1], strlen(resultp[ncolumn * i +1]));
		memcpy(ctx->ntpSta.vp[j].v_mask, resultp[ncolumn * i +2], strlen(resultp[ncolumn * i +2]));
		memcpy(ctx->ntpSta.vp[j].v_gateway, resultp[ncolumn * i +3], strlen(resultp[ncolumn * i +3]));
		
		/*1-20 ntp port, 21 network manager port, we not process it*/
		if(j < nrow -1){
			memset(networkcard, 0, sizeof(networkcard));
			sprintf(networkcard, "eth0.%d",j+5);
			//printf("eth[%d]:%s\n",j+5,networkcard);
			strncpy(port[j].name, networkcard, IF_NAMESIZE);
			 if(sscanf((char *)ctx->ntpSta.vp[j].v_ip, "%d.%d.%d.%d", &im[0], &im[1], &im[2], &im[3]) != 4)
			 {
			      return 0;
			 }
			
			for(k=0;k<4;k++)
			{
				port[j].ip_a[k] = im[k];
				//printf(" ip:   %d",im[k]);
			}
			//printf("\n");
			
			port[j].ip = (port[j].ip_a[0] << 24) | (port[j].ip_a[1] << 16) | (port[j].ip_a[2] << 8) | port[j].ip_a[3] ;	
			//printf("j:%d  ip--%08x\n",j,port[j].ip);			

			 if(sscanf((char *)ctx->ntpSta.vp[j].v_mask, "%d.%d.%d.%d", &im[0], &im[1], &im[2], &im[3]) != 4)
			 {
			      return 0;
			 }
			
			for(k=0;k<4;k++)
			{
				port[j].mask_a[k] = im[k];
				//printf("mask:  %d",im[k]);
			}
			port[j].mask = (port[j].mask_a[0] << 24) | (port[j].mask_a[1] << 16) | (port[j].mask_a[2] << 8) |port[j].mask_a[3];
			
			//printf("\n");
			if(6 != sscanf((char *)ctx->ntpSta.vp[j].v_mac,"%hhx%*c%hhx%*c%hhx%*c%hhx%*c%hhx%*c%hhx", 
	                                    &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
			{
				return 0;
			}

			for(k=0;k<6;k++)
			{
				port[j].mac[k] = mac[k];
				//printf(" mac:%d",mac[k]);
			}
		//printf("\n");
		}
		print(	DBG_INFORMATIONAL, 
				"<%s>--%d %s %s %s %s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				ctx->ntpSta.vp[j].v_port, 
				ctx->ntpSta.vp[j].v_ip, 
				ctx->ntpSta.vp[j].v_mac, 
				ctx->ntpSta.vp[j].v_mask,
				ctx->ntpSta.vp[j].v_gateway );
	}
	db_free_table(resultp);
	db_unlock(ctx->pVethDb);
	return 1;
}

int read_eth(struct ntpCtx *ctx, char *tbl,u8_t v_port)
{
	char sql[256];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;
	//int i,j;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select v_mac from %s where v_port=%d;", tbl,v_port);
	
	db_lock(ctx->pVethDb);
	ret1 = db_get_table(ctx->pVethDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pVethDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read virtual ether.", 
				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	memcpy(ctx->ntpSta.vp[v_port-1].v_mac, resultp[1], strlen(resultp[1]));
	
	db_free_table(resultp);
	db_unlock(ctx->pVethDb);
	return 1;	
}







/*
  @ctx		ntp context
  @vp		virtual ether
  @tbl		virtual ether table

  1	成功
  0	失败

  modify virtual ether
*/
int write_virtual_eth(struct ntpCtx *ctx, struct veth *vp, char *tbl)
{
	char sql[512];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	if (vp->v_ip[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_ip=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl, 
			 vp->v_ip,
			 vp->v_port );
	}
	if (vp->v_mask[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_mask=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl,  
			 vp->v_mask,
			 vp->v_port );
	}
	if(vp->v_gateway[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_gateway=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl,
			 vp->v_gateway,
			 vp->v_port );
	}
	if(vp->v_mac[0] != '\0'){
		sprintf( sql, "%.*s update %s \
			 set v_mac=\'%s\' where v_port=%d; ", 
			 strlen(sql),sql, tbl, 
			 vp->v_mac,
			 vp->v_port );
	}

	//printf("%s\n", sql);return 0;

	if(sql[0] == '\0')
		return 0;
	
	db_lock(ctx->pVethDb);
	ret1 = db_put_table(ctx->pVethDb, sql);
	db_unlock(ctx->pVethDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to modify virtual ether.", 
 				gDaemonTbl[DAEMON_CURSOR_NTP] );

		return 0;
	}
	else
	{
		return 1;
	}
}


/*
  @ctx			output board context
  @pTbl			system table
  
  1  成功
  0  失败
  
  read leapsecond from system table
*/
int ReadLeapsecond(struct ntpCtx *ctx, char *pTbl)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select leapsecond, leapmode from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read leapsecond.", 
				gDaemonTbl[DAEMON_CURSOR_OUTPUT] );
		
		return 0;
	}
	
	ctx->lp.leapSecond = atoi(resultp[2]);
	ctx->lp.leapMode = atoi(resultp[3]);
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}







