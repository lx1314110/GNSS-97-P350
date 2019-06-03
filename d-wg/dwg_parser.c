#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


#include "lib_msgQ.h"
#include "lib_shm.h"
#include "lib_sema.h"
#include "lib_dbg.h"
#include "lib_fpga.h"
#include "lib_net.h"
#include "lib_sqlite.h"
#include "lib_time.h"

#include "common.h"
#include "dwg_alloc.h"
#include "dwg_parser.h"

static struct UartBuffer* pTxBuffer = NULL;

static const u32_t crc32tab[] = {
 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
 0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
 0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
 0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d 
};


bool_t isRunning(struct wgCtx *ctx)
{
	return ctx->loop_flag;
}


struct UartBuffer *Buf_Init(u32_t buf_size)
{
	struct UartBuffer* pBuff = (struct UartBuffer* )malloc(sizeof(struct UartBuffer));
	pBuff->UartBuffSize = buf_size;
	pBuff->pUartBuff = (u8_t *)malloc(pBuff->UartBuffSize);
	pBuff->pInPos = pBuff->pUartBuff;
	pBuff->pOutPos = pBuff->pUartBuff;
	pBuff->Space = pBuff->UartBuffSize;
	pBuff->BuffState = 0;
	pBuff->nextBuff = NULL;
	pBuff->TcpControl = 0;
	return pBuff;

}


void Init_buf()
{
	pTxBuffer = Buf_Init(BUFFER_SIZE);
}

void Clean_buf()
{
	if(pTxBuffer){
		if(pTxBuffer->pUartBuff)
			FREE(pTxBuffer->pUartBuff);
		FREE(pTxBuffer);
	}
}



void uart_rx_intr_enable(u8_t uart_no)
{
#if 1
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
#else
    ETS_UART_INTR_ENABLE();
#endif
}

static void Buf_Cpy(struct UartBuffer* pCur, char* pdata , u16_t data_len)
{
    if(data_len == 0) return ;
    
    u16_t tail_len = pCur->pUartBuff + pCur->UartBuffSize - pCur->pInPos ;
    if(tail_len >= data_len){  //do not need to loop back  the queue
        memcpy(pCur->pInPos , pdata , data_len );
        pCur->pInPos += ( data_len );
        pCur->pInPos = (pCur->pUartBuff +  (pCur->pInPos - pCur->pUartBuff) % pCur->UartBuffSize );
        pCur->Space -=data_len;
    }else{
        memcpy(pCur->pInPos, pdata, tail_len);
        pCur->pInPos += ( tail_len );
        pCur->pInPos = (pCur->pUartBuff +  (pCur->pInPos - pCur->pUartBuff) % pCur->UartBuffSize );
        pCur->Space -=tail_len;
        memcpy(pCur->pInPos, pdata+tail_len , data_len-tail_len);
        pCur->pInPos += ( data_len-tail_len );
        pCur->pInPos = (pCur->pUartBuff +  (pCur->pInPos - pCur->pUartBuff) % pCur->UartBuffSize );
        pCur->Space -=( data_len-tail_len);
    }
    
}


//rx buffer dequeue
u16_t rx_buff_deq(char* pdata, u16_t data_len )
{
    u16_t buf_len =  (pTxBuffer->UartBuffSize- pTxBuffer->Space);
    u16_t tail_len = pTxBuffer->pUartBuff + pTxBuffer->UartBuffSize - pTxBuffer->pOutPos ;
    u16_t len_tmp = 0;
    len_tmp = ((data_len > buf_len)?buf_len:data_len);
	
    if(pTxBuffer->pOutPos <= pTxBuffer->pInPos){
        memcpy(pdata, pTxBuffer->pOutPos,len_tmp);
		
        pTxBuffer->pOutPos+= len_tmp;
        pTxBuffer->Space += len_tmp;
    }else{
        if(len_tmp>tail_len){
            memcpy(pdata, pTxBuffer->pOutPos, tail_len);
            pTxBuffer->pOutPos += tail_len;
            pTxBuffer->pOutPos = (pTxBuffer->pUartBuff +  (pTxBuffer->pOutPos- pTxBuffer->pUartBuff) % pTxBuffer->UartBuffSize );
            pTxBuffer->Space += tail_len;
            
            memcpy(pdata+tail_len , pTxBuffer->pOutPos, len_tmp-tail_len);
            pTxBuffer->pOutPos+= ( len_tmp-tail_len );
            pTxBuffer->pOutPos= (pTxBuffer->pUartBuff +  (pTxBuffer->pOutPos- pTxBuffer->pUartBuff) % pTxBuffer->UartBuffSize );
            pTxBuffer->Space +=( len_tmp-tail_len);                
        }else{
            //os_printf("case 3 in rx deq\n\r");
			
            memcpy(pdata, pTxBuffer->pOutPos, len_tmp);
            pTxBuffer->pOutPos += len_tmp;
            pTxBuffer->pOutPos = (pTxBuffer->pUartBuff +  (pTxBuffer->pOutPos- pTxBuffer->pUartBuff) % pTxBuffer->UartBuffSize );
            pTxBuffer->Space += len_tmp;
        }
    }
	
	/*
    if(pTxBuffer->Space >= UART_FIFO_LEN){
		printf("[%s]%d\n",__FILE__,__LINE__);
        uart_rx_intr_enable(UART0);
		printf("[%s]%d\n",__FILE__,__LINE__);
    }
	*/
    return len_tmp; 
}

void tx_buff_enq(char* pdata, u16_t data_len )
{
    if(pTxBuffer == NULL){

        pTxBuffer = Buf_Init(BUFFER_SIZE);
        if(pTxBuffer!= NULL){
            Buf_Cpy(pTxBuffer ,  pdata,  data_len );
        }else{
            print(DBG_WARNING, "Tx MALLOC no buf\n\r");
        }
    }else{
        if(data_len <= pTxBuffer->Space){
        Buf_Cpy(pTxBuffer ,  pdata,  data_len);
        }else{
            print(DBG_WARNING, "TX BUF FULL!!!!\n\r");
        }
    }
    #if 0
    if(pTxBuffer->Space <= URAT_TX_LOWER_SIZE){
	    set_tcp_block();        
    }
    #endif
}

int analyze(char *response,struct respond * respond)
{
	char *result = NULL;
	int i = 0;
    result = strtok( response, "|" );
	memcpy( respond->user, result, sizeof(respond->user));
	
	while( i < 3 &&result != NULL ) {
		
		result = strtok( NULL, "|" );
		if(i == 0)
		{
			memcpy( respond->password, result, sizeof(respond->password));
			
		}
		else if(i == 1)
		{
			
			memcpy( respond->pid , result, sizeof(respond->pid));
		}
		else 
		{
			memcpy( respond->answer, result, sizeof(respond->answer));
			
		}
		
		i++;
	}
	result = strtok( respond->answer, ";" );
	memcpy( respond->answer, result, sizeof(respond->answer));
	return 1;
}

void * thread_send(void *args)
{
	int ret=0;
	u16_t  readsize;
	char buf[100];
	static char sendbuf[100];
	struct wgCtx *ctx;
	//int n_pre;	
	ctx = (struct wgCtx *)args;
	struct respond res;
	int pre_pid = -1;
	//memcpy(res.pid,"-1",2);
	
	while(isRunning(ctx))
	{
		//if(n_pre != n_current)
		//{
			
			sleep(1);
			
		//}
		/*
		if(ctx->rec_pid == pre_pid)
		{
			
		}
		else
		{
			
			if(ctx->rec_pid == atoi(res.pid))
			{
				readsize = rx_buff_deq(buf,100);
				if(readsize > 0)
				{
					memcpy(sendbuf,buf,sizeof(buf));
					analyze(buf,&res);
					printf("buf:%s1111\n",sendbuf);
				}
			
			}
			*/
			readsize = rx_buff_deq(buf,100);
			if(readsize > 0)
			{
				memcpy(sendbuf,buf,sizeof(buf));
				analyze(buf,&res);
				//printf("buf:%s1111\n",sendbuf);
			}
			//printf("res.pid:%d   pid:%d\n",atoi(res.pid),pre_pid);
			if(atoi(res.pid) != pre_pid)
			{
				ret = sendto(ctx->sendSock,sendbuf,90,0,(struct sockaddr*)&(ctx->sa_send),sizeof(struct sockaddr_in));
			}
			
		//}
		pre_pid = atoi(res.pid); 
		//n_pre = n_current;
	}

	print(DBG_NOTICE, "Thread_send exit clearly!\n");
	pthread_exit(NULL);
}


void * thread_rec(void *args)
{
	int ret=0;
	char buf[32];
	struct wgCtx *ctx;
	struct respond res;
	
	ctx = (struct wgCtx *)args;

	while(isRunning(ctx))
	{
		sleep(1);
		ret = recvfrom(ctx->sendSock,buf,32,0,NULL, NULL);
		
		if(ret>0)
		{
			analyze(buf,&res);
		}
		ctx->rec_pid = atoi(res.pid);
		//printf("11%s %s %s %s11\n",res.user,res.password,res.pid,res.answer);
		
	}

	print(DBG_NOTICE, "Thread_rec exit clearly!\n");
	pthread_exit(NULL);
}



int CreateThread(struct wgCtx *ctx)
{
	int retval;

	retval = pthread_create(&(ctx->thread_1), NULL, thread_send, (void*)ctx);
	if(0 != retval)
	{
	
		return 0;
	}

	retval = pthread_create(&(ctx->thread_2), NULL, thread_rec, (void*)ctx);
	if(0 != retval)
	{
		
		return 0;
	}

	return 1;
}


int CloseThread(struct wgCtx *ctx)
{
	int retval;
	ctx->loop_flag = false;
	retval = pthread_join(ctx->thread_1, NULL);
	if( 0 != retval )
	{
		return 0;
	}
	
	retval = pthread_join(ctx->thread_2, NULL);
	if( 0 != retval )
	{
		return 0;
	}
	
	return 1;
}


/*
  true:		收到配置命令
  false:	没有收到配置命令
*/
bool_t isNotify(struct wgCtx *ctx)
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

/***********************************************
 * CRC32????
 **********************************************/
u32_t Crc32Check(const u8_t *buf, u32_t size)
{
    u32_t i, crc;
    crc = 0xFFFFFFFF;

	for (i=0; i<size; i++)
    {
        crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    
    return crc^0xFFFFFFFF;
}

/*
  1	成功
  0	失败
*/
int sendmessage(char *data, struct wgCtx *ctx,char *MessageType)
{
	char buf[100];
	u32_t Crc32Checkcal = 0;
	ctx->pid += 3;
	
	
	memset(buf,0,sizeof(buf));
//resend:	
	sprintf(buf,"%s|%s|%d|%s|%s|%s",USERNAME,PASSWORD,ctx->pid,MessageType,ctx->ip,data);
	
	Crc32Checkcal = Crc32Check((u8_t *)buf, strlen(buf));
		
	sprintf(buf,"%s|%u;",buf,Crc32Checkcal);
	
	tx_buff_enq(buf, sizeof(buf));
	
	return 1;
}



/*
  1	成功
  0	失败
*/
int sysLevel(char *data, struct wgCtx *ctx)
{
	struct dbginfo *dbg;

	dbg = (struct dbginfo *)data;
	if(0 == set_print_level(dbg->syslog_en, dbg->dbg_level, DAEMON_CURSOR_WG))
	{
		print(	DBG_ERROR, 
				"%s", "Failed to set print level.");

		return 0;
	}
	print(DBG_INFORMATIONAL, 
			  "--sys syslog:%d dbg_level:%d", dbg->syslog_en,
			  dbg->dbg_level);

	return 1;
}

int sysWgnet(char *data, struct wgCtx *ctx)
{
	struct veth *tmp;
	tmp = (struct veth *)data;
	char networkcard[20];
	int ret = SET_OK;
	//char networkword[40];
	//int i;
//	char buff[512];
	
	memset(networkcard, 0, sizeof(networkcard));
	sprintf(networkcard, "%s", NETWEB_CARD);
	/*
	sprintf(networkword, "ifconfig %s %s netmask %s up", networkcard,tmp->v_ip,tmp->v_mask);
	system(networkword);
	*/
	
	#if 0
	FILE* fd = NULL;
	int i;
	fd = fopen("/tmp/test.log","a+");
	if(fd == NULL)
	{
		return 0;
	}
	fprintf(fd,"###start####\n");
	fprintf(fd,"#######");
	fprintf(fd,"%s\n",tmp->v_ip);
	for(i=0; tmp->v_ip[i]!= '\0';i++){
		fprintf(fd,"%02x ",tmp->v_ip[i]);
	}
	
	fprintf(fd,"\n#######");
	fprintf(fd,"%s\n",tmp->v_mask);
	for(i=0; tmp->v_mask[i]!= '\0';i++){
		fprintf(fd,"%02x ",tmp->v_mask[i]);
	}
	
	fprintf(fd,"\n#######");
	fprintf(fd,"%s\n",tmp->v_gateway);
	for(i=0; tmp->v_gateway[i]!= '\0';i++){
		fprintf(fd,"%02x ",tmp->v_gateway[i]);
	}
	fprintf(fd,"\n###end####\n");
	fclose(fd);
	#endif
	/*
	if(0 == ethn_add("eth0.3", "192.168.33.22", NULL, "255.255.255.0", "192.168.33.1"))//添加网卡
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_NTP], 
				"Failed to modify virtual ether.");

		return 0;
	}
	*/
	ret = ethn_add(networkcard, (char *)tmp->v_ip, NULL, (char *)tmp->v_mask, (char *)tmp->v_gateway);//添加网卡
	if(ret < 0){
		if(SET_IP_ERR == ret)
		{
			print(	DBG_ERROR, "%s[%d]", "Failed to modify virtual ether.", ret);
			return 0;
		}else{
			print(	DBG_ERROR, "%s[%d]", "Failed to modify virtual ether.", ret);
		}
	}
	
	if(-1 == write_virtual_eth(ctx, TBL_VETH, tmp))//更新网卡信息
	{
		print(	DBG_ERROR, 
				"<%s>--%s", 
				gDaemonTbl[DAEMON_CURSOR_WG], 
				"Failed to modify virtual to sqlite.");
		return 0;
	}
	
	
	if(0 == __Sock_GetIp(NETWEB_CARD,ctx->ip))
	{
		return 0;
	}
	return 1;
}

int WriteWgIp(struct wgCtx *ctx, struct veth *tmp, char *pTbl)
{
	char sql[256];
	bool_t ret1;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s \
				 set wg_ip=\'%s\';", 
				 pTbl, 			
				 tmp->v_ip);
	
	db_lock(ctx->pDb);
	ret1 = db_put_table(ctx->pDb, sql);
	db_unlock(ctx->pDb);
	if(false == ret1)
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to write wg_ip.", 
				gDaemonTbl[DAEMON_CURSOR_WG] );
		
		return 0;
	}
	else
	{
		return 1;
	}
}

int ReadWgIp(struct wgCtx *ctx, char *pTbl ,char *ip)
{
	char sql[64];
	bool_t ret1;
	char **resultp;
	int nrow;
	int ncolumn;

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select wg_ip from %s;", pTbl);
	
	db_lock(ctx->pDb);
	ret1 = db_get_table(ctx->pDb, sql, &resultp, &nrow, &ncolumn);
	if(false == ret1)
	{
		db_unlock(ctx->pDb);
		
		print(	DBG_ERROR, 
				"<%s>--Failed to read wg_ip.", 
				gDaemonTbl[DAEMON_CURSOR_WG] );
		
		return 0;
	}
	
	memset(ip, 0, 16);
	memcpy(ip, resultp[1], strlen(resultp[1]));
	db_free_table(resultp);
	db_unlock(ctx->pDb);
	
	return 1;
}


int sysServer(char *data, struct wgCtx *ctx)
{
	struct veth *tmp;
	tmp = (struct veth *)data;
	
	ctx->sa_send.sin_addr.s_addr = inet_addr((char *)tmp->v_ip);
	
	
	if(0 == WriteWgIp(ctx, tmp, TBL_SYS))
	{
		print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_WG], 
					"Failed to WriteWgIp");
		return 0;
	}
	return 1;
}

/*
  1	成功
  0	失败
*/
int ReadMessageQueue(struct wgCtx *ctx)
{
	int ret;
	struct msgbuf msg;

	do
	{
		memset(&msg, 0, sizeof(struct msgbuf));
	
		ret = MsgQ_Rcv(ctx->ipc[IPC_CURSOR_WG].ipc_msgq_id, &msg);
		if(-1 == ret)
		{
			print(	DBG_ERROR, 
					"<%s>--%s", 
					gDaemonTbl[DAEMON_CURSOR_WG], 
					"Failed to read message queue");
			return 0;
		}
		
		if(ret > 0)
		{
			if(MSG_CFG_DBG == msg.mtype)
			{
				if(0 == sysLevel(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else if(MSG_CFG_WG_LOG == msg.mtype)//保存配置以及向对应的槽发送输出信号
			{
				
				if(0 == sendmessage(msg.mdata, ctx, "serverlog"))
				{
					/*
					print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_WG], 
						"loss log connection");
						*/
				}
			}
			else if(MSG_CFG_WG_NET == msg.mtype)//保存配置以及向对应的槽发送输出信号
			{
				if(0 == sysWgnet(msg.mdata, ctx))
				{
					return 0;
				}
				SET_EVNTFLAGVALID_BY_SLOT_PORT_ID(SNMP_EVNT_ID_SYSIP_SET, SID_SYS_SLOT, SID_SYS_PORT);
			}
			else if(MSG_CFG_WG_SERVER_IP == msg.mtype)//保存配置以及向对应的槽发送输出信号
			{
				if(0 == sysServer(msg.mdata, ctx))
				{
					return 0;
				}
			}
			else
			{
				print(	DBG_ERROR, 
						"<%s>--%s", 
						gDaemonTbl[DAEMON_CURSOR_WG], 
						"Invalid message type");
				return 0;
			}
		}
	}while(ret > 0);
	
	return 1;
}


int ReadAlmState(struct wgCtx *ctx)
{
	
	if(!FpgaRead(ctx->fpga_fd ,FPGA_LED_ALM,(u16_t *)&ctx->new_AlmState))
	{
		return 0;
	}
	
	if(ctx->new_AlmState != ctx->old_AlmState)
	{
		ctx->ne_changflag = 1;
	}
	return 1;
}

int ReadClockState(struct wgCtx *ctx)
{
	struct clock_stainfo clock_sta;
	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	shm_read(ctx->ipc[IPC_CURSOR_ALARM].ipc_base, 
			 SHM_OFFSET_CLOCK, 
			 sizeof(clock_sta), 
			 (char *)&(clock_sta), 
			 sizeof(clock_sta));
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_ALARM].ipc_sem_id, SEMA_MEMBER_ALARM))
	{
		return -1;
	}
	
	
	ctx->new_clkSta = clock_sta.state;
	if(ctx->new_clkSta > 4)
	{
		ctx->new_clkSta = 4;
	}
	
	if(ctx->new_clkSta != ctx->old_clkSta)
	{
		ctx->ne_changflag = 1;
	}
	return 1;
}


int ReadTimeSource(struct wgCtx *ctx)
{
	//u8_t fresh_state = 0;
	char * ref_src_name = NULL;
	struct inputsta sta;
	#if 1
		struct schemainfo schema;
	#endif

	if(1 != sema_lock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to lock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_ALARM]);

		return 0;
	}

	#if 0
	shm_read(ctx->ipc[IPC_CURSOR_INPUT].ipc_base, 
			  SHM_OFFSET_FRESH, 
			  sizeof(fresh_state), 
			  (char *)&fresh_state, 
			 sizeof(fresh_state));
	#else
	shm_read(ctx->ipc[IPC_CURSOR_INPUT].ipc_base, 
			 SHM_OFFSET_SCHEMA, 
			 sizeof(struct schemainfo), 
			 (char *)&schema, 
			 sizeof(struct schemainfo));
	#endif
	
	if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to unlock semaphore.", 
				gDaemonTbl[DAEMON_CURSOR_ALARM]);

		return 0;
	}

	
	//memset(ctx->new_TimeSource,0,sizeof(ctx->new_TimeSource));
	//printf("state:%s  mode:%d\n",fresh_state,sta.gbdbe.gbdbe.sysMode);
	if(REFSOURCE_VALID == schema.refsrc_is_valid && schema.slot > INPUT_SLOT_CURSOR_1)
	{
		if(schema.slot <= INPUT_SLOT_CURSOR_1 || schema.slot > INPUT_SLOT_CURSOR_ARRAY_SIZE)
		{
			print(	DBG_ERROR, 
					"<%s>--Get the schema slot error.", 
					gDaemonTbl[DAEMON_CURSOR_ALARM]);
			return 0;
		}
		
		if(1 != sema_lock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
		{
			return 0;
		}
		
		shm_read(ctx->ipc[IPC_CURSOR_INPUT].ipc_base,
				 SHM_OFFSET_STA + (schema.slot-1)*sizeof(struct inputsta), 
				 sizeof(struct inputsta), 
				 (char *)&sta, 
				 sizeof(struct inputsta));
		
		if(1 != sema_unlock(ctx->ipc[IPC_CURSOR_INPUT].ipc_sem_id, SEMA_MEMBER_INPUT))
		{
			return 0;
		}
	
		FIND_TYPE_NAME_BY_ID(schema.src_inx, ref_src_name);
		//ref_source_type = gTimeSourceTbl[schema.port].time_source_type;
		if(schema.type == GPS_SOURCE_TYPE){
			switch(sta.satcommon.satcomminfo.sysMode){
				case MODE_CURSOR_GPS_VAL:
				case MODE_CURSOR_MIX_GPS_VAL:
					memcpy(ctx->new_TimeSource,"GPS\0",4);
					break;
				case MODE_CURSOR_BD_VAL:
				case MODE_CURSOR_MIX_BD_VAL:
					memcpy(ctx->new_TimeSource,"BD\0",3);
					break;
				default:
					memcpy(ctx->new_TimeSource,"NONE\0",5);
			}
		} else {
				memcpy(ctx->new_TimeSource,ref_src_name,strlen(ref_src_name)+1);
		}
	}
	else
	{
		memcpy(ctx->new_TimeSource,"NONE\0",5);	
		
	}
	
	if(0 !=  memcmp(ctx->new_TimeSource,ctx->old_TimeSource,strlen(ctx->new_TimeSource)))
	{
		ctx->ne_changflag = 1;
	}
	return 1;
}




void ReNew_value(struct wgCtx *ctx)
{
	ctx->old_AlmState = ctx->new_AlmState;
	ctx->old_clkSta = ctx->new_clkSta;
	memcpy(ctx->old_TimeSource,ctx->new_TimeSource,10);
	ctx->ne_changflag = 0;
}
