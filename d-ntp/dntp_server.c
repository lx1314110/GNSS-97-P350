#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "lib_md5.h"
#include "lib_dbg.h"
#include "lib_net.h"
#include "alloc.h"

#include "dntp_global.h"
#include "dntp_timestamp.h"
#include "dntp_macro.h"
#include "dntp_alloc.h"
#include "dntp_parser.h"
#include "dntp_config.h"
#include "dntp_server.h"
#include "lib_ip1725.h"

#include "ntp_gettime.h"
#include "dntp_socket.h"
#include "common.h"

//default:99
#define NTP_PRO_RR
//#define NTP_RR_90

u8_t li;
#define MD5_MESSAGE_BYTES 64
#define MD5_DIGEST_WORDS 4
static uint32_t net_secret[MD5_MESSAGE_BYTES / 4];
static int fpga_fd = 0;

bool_t isRunning(struct ntpCtx *ctx)
{
	return ctx->loop_flag;
}

#if 0
bool_t initializeSocket(struct ntpCtx *ctx)
{
	struct sockaddr_in sa;
	struct timeval tv;
	int so_reuseaddr = 1;
	int so_broadcast = 1;
	
	ctx->ntpSock = socket(AF_INET, SOCK_DGRAM, 0);
	if( ctx->ntpSock < 0 )
	{
		return false;
	}

	if( 0 != setsockopt( ctx->ntpSock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(int) ) )//允许地址重用
	{
		close(ctx->ntpSock);
		return false;
	}

	if(0 != setsockopt(ctx->ntpSock, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(int)))//允许发送广播信息
	{ 
		close(ctx->ntpSock);
		return false;
	}

	memset( &tv, 0, sizeof(struct timeval) );
	tv.tv_sec = 0;
	tv.tv_usec = 20000;
	
	if( 0 != setsockopt( ctx->ntpSock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval) ) )//设置发送超时
	{
		close(ctx->ntpSock);
		return false;
	}
	
	if( 0 != setsockopt( ctx->ntpSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval) ) )//设置接收超时
	{
		close(ctx->ntpSock);
		return false;
	}
	
	memset( &sa, 0, sizeof(struct sockaddr_in) );
	sa.sin_family = AF_INET;
	sa.sin_port = htons(NTP_SVR_PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind( ctx->ntpSock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in) ) < 0 )
	{
		close(ctx->ntpSock);
		return false;
	}

	return true;
}

#endif

bool_t initializeSocket(struct ntpCtx *ctx)
{

	int i;
	
	for(i = 0; i < ARRAY_SIZE(port); i++)
	{
		if(port[i].name[0])
		{
			create_raw_socket(&port[i]);
			
			port[i].ip = (port[i].ip_a[0] << 24) | (port[i].ip_a[1] << 16) 
						| (port[i].ip_a[2] << 8) | port[i].ip_a[3] ;
			
			port[i].mask = (port[i].mask_a[0] << 24) | (port[i].mask_a[1] << 16) 
						| (port[i].mask_a[2] << 8) | port[i].mask_a[3];
			
		}
		else
			port[i].sock = -1;
		
	}

	return true;
	
}



static uint32_t secure_ip_id(uint32_t daddr)
{
	uint32_t hash[MD5_DIGEST_WORDS];

	hash[0] = (uint32_t) daddr;
	hash[1] = net_secret[13];
	hash[2] = net_secret[14];
	hash[3] = net_secret[15];

	md5_transform(hash, (u8_t *)net_secret);

	return hash[0];
}


static void ip_select_fb_ident(u32_t daddr, u16_t *id)
{
	static u32_t ip_fallback_id;
	u32_t salt;

	salt = secure_ip_id((u32_t)ip_fallback_id ^ daddr);
	*id = htons(salt & 0xFFFF);
	ip_fallback_id = salt;
}





bool_t VnBroadcast(int soc, u8_t Vn, struct recvaddr_t *bcast_addr)
{
	int i, ret;
	struct ntpkt bcastPkt;
	struct timeval orgTime;
	l_fp tmpTime;
	
	memset(&bcastPkt, 0, sizeof(struct ntpkt));
	bcastPkt.LVM = PKT_LI_VN_MODE(li, Vn, 5);// LI：跳跃指示器,VN：版本号, Mode：工作模式
	bcastPkt.Stratum = 1;
	bcastPkt.Poll = 4;//连续信息间的最大间隔
	bcastPkt.Precision = -20;
	bcastPkt.RootDelay = 0;
	bcastPkt.RootDispersion = 0;
	memcpy(bcastPkt.RefId, "DCLS", 4);//参考时钟标识符
	
#ifdef TIME_USE_FPGA_TIME
	ntpgettimefpga(gNtpCtx.fpga_fd, gNtpCtx.lp.leapSecond, &orgTime,NULL);//从FPGA获取原始时间
#else
	if(GetSysTimeTV(&orgTime))
	{
		return;
	}
#endif

	TVTOTS(&orgTime, &tmpTime);//把一个时间结构体转换成时间戳
	HTONL_TS(&tmpTime, &(bcastPkt.RefStamp));//（参考时间戳）
	
#ifdef TIME_USE_FPGA_TIME
	ntpgettimefpga(gNtpCtx.fpga_fd, gNtpCtx.lp.leapSecond, &orgTime,NULL);
#else
	if(GetSysTimeTV(&orgTime))
	{
		return;
	}
#endif

	TVTOTS(&orgTime, &tmpTime);
	HTONL_TS(&tmpTime, &(bcastPkt.TranStamp));//（发送时间戳）
	ip_select_fb_ident(bcast_addr->ip, &bcast_addr->id);
	ret = send_packet((u8_t *)&bcastPkt, NTP_PKT_NOMAC, bcast_addr);
	if ( -1 == ret )
	{
		print(	DBG_ERROR, 
				"<%s>--Failed to send broadcast packet", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);
		
		return false;
	}
	if(DBG_DEBUG == get_print_level())
	{
		for( i=0; i<NTP_PKT_NOMAC; i++ )
		{
			printf("%02x", ((u8_t *)(&bcastPkt))[i] );//打印NTP协议
		}
		printf("\n\n");
	}
	
	return true;
}

u32_t getBroadIp(u32_t ip, u32_t netmask)
{
    u32_t n_ip;
    u32_t n_netmask;
    u32_t n_broadip;
    n_ip = htonl((u32_t)ip);
    n_netmask = htonl((u32_t)netmask);
    n_broadip = n_ip | (~n_netmask) ;
    return ntohl(n_broadip) ;
}





bool_t VaBroadcast(struct port_info *pInfo)
{
	
	struct recvaddr_t recvaddr;
	
 	memset(recvaddr.mac, 0xff, 6);
	uint32_t ip_a = 0,mask_a = 0;
    memcpy(&ip_a, pInfo->ip_a, 4);
    memcpy(&mask_a, pInfo->mask_a, 4);
    recvaddr.ip = getBroadIp(ip_a, mask_a);
	recvaddr.port = __cpu_to_be16(123);
    recvaddr.info = pInfo;
	
	//根据版本号广播
	if((gNtpCtx.ntpSta.vEn.VnEnable)&0x01)
	{
		if( !VnBroadcast(pInfo->sock, 2, &recvaddr) )
		{
			return false;
		}
	}

	if((gNtpCtx.ntpSta.vEn.VnEnable)&0x02)
	{
		if( !VnBroadcast(pInfo->sock, 3, &recvaddr) )
		{
			return false;
		}
	}

	if((gNtpCtx.ntpSta.vEn.VnEnable)&0x04)
	{
		if( !VnBroadcast(pInfo->sock, 4, &recvaddr) )
		{
			return false;
		}
	}
	
	return true;
}

#if 0
void SwitchConfig(int fpga_fd)
{
    u8_t i=0;
    u8_t addr = 0x0;
    u16_t vlanTag = 3; 
    u16_t regVal = 0;
	u16_t val;
	unsigned int vid;
	unsigned short reg;
	//阻止Spanning Tree Protocol风暴
	//SpanningTreeBlock(fpga_fd);
	
    //Set VLAN type 802.1Q based VLAN
    SwitchRegWrite(fpga_fd,0x82, 0x0880);   
    
    //Prot1 vlan Group
    //SwitchRegWrite(fpga_fd,0xAD,0xFFF1);    //把1号端口和4到23号口划为一个VLAN
    //SwitchRegWrite(fpga_fd,0xAE,0x00FF); 
	
    //SwitchRegWrite(fpga_fd,0xAF, 0x3);    //把管理端口2和1号口划为一个VLAN
	
	//read
	/*
	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (vid << 16) | reg;
		printf("entry %d: %08x\n", i, vid);
	}

	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (vid << 16) | reg;
		printf("entry %d: %08x\n", i, vid);
	}
	*/

	//write

	for(i = 0; i < 32; i++)
	{
		SwitchRegWrite(fpga_fd,0xFC, 1);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (1 << 6) | (0<< 5) | (i & 0x1f));
		SwitchRegWrite(fpga_fd,0xFC, 0);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (1 << 6) | (1<< 5) | (i & 0x1f));
	}


	//write
	for(i = 0; i < 32; i++)
	{
		SwitchRegWrite(fpga_fd,0xFC, i+1);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (0 << 6) | (0<< 5) | (i & 0x1f));
		SwitchRegWrite(fpga_fd,0xFC, i+1);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (0 << 6) | (1<< 5) | (i & 0x1f));
	}



	/*

	//read
	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (reg << 16) | vid;
		printf("entry %d: %08x\n", i, vid);
	}
	
	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (vid << 16) | reg;
		printf("entry %d: %08x\n", i, vid);
	}
	*/
    addr = 0xAD;
    //配置Vlan Table，转发表
    for (i = 0; i < 24; i++)
    { 

		if(i<16)
		{
			SwitchRegWrite(fpga_fd,addr, (1<<0)|(1<<i));
			//printf("addr: %x  %x\n",addr, (1<<0)|(1<<i));
			SwitchRegWrite(fpga_fd,addr+1, 0);
		}
		else
		{
			SwitchRegWrite(fpga_fd,addr, 1<<0); 
			SwitchRegWrite(fpga_fd,addr+1, 1<<(i-16));
		}
		
          //把每一个端口和1号端口划为一组 
		//SwitchRegWrite(fpga_fd,addr, (1<<0)|(1<<i));
        addr += 2;
    }

	//关闭MAC学习
	SwitchRegWrite(fpga_fd,0x8A, 0x0000);
    SwitchRegWrite(fpga_fd,0x8B, 0xFF00);
	
    //使能ADD VLAN
    //SwitchRegWrite(fpga_fd,0x97, 0xFFFF);
    //SwitchRegWrite(fpga_fd,0x98, 0xFFFF);

    //Remove VLAN
    //SwitchRegWrite(fpga_fd,0x99, 0xFFFE);
    //SwitchRegWrite(fpga_fd,0x9A, 0xFFFF);

    //使能VLAN Table
    //SwitchRegWrite(fpga_fd,0xFA, 0xFFFF);
    //SwitchRegWrite(fpga_fd,0xFC, 0xFFFF);

//配置PVID
	//Config port1-3 Vlan Tag Id,0x9B
	//regVal = (1<<0) | (2<<5) | (1<<10);//把各个端口进来的包隔离开来
	//SwitchRegWrite(fpga_fd, 0x9b, regVal);
	
    //Config Vlan Tag Id,0x9B~0xA3
    addr = 0x9b;
    vlanTag = 0;
    do
    {
			
			regVal = 0x00;
			regVal = (vlanTag<<0) | ((vlanTag+1)<<5) | ((vlanTag+2)<<10);//把各个端口进来的包隔离开来
			vlanTag += 3;
       
        SwitchRegWrite(fpga_fd,addr, regVal);
        addr++;
    }while(addr < 0xA3);
    
    //使能对应端口添加vlan tag功能
   // SwitchRegWrite(fpga_fd,0x47, 0xFFF2);
    //SwitchRegWrite(fpga_fd,0x48, 0x00FF); 
	u16_t ss = 0;
	SwitchRegRead(fpga_fd,0x6E,&ss);
	printf("1-------%d\n",ss);
	SwitchRegWrite(fpga_fd,0x6E,0x01);
	ss = 0xffff;
	SwitchRegRead(fpga_fd,0x6E,&ss);
	printf("2-------%d\n",ss);
	//usleep(10000);
	//SwitchRegWrite(fpga_fd,0x6E,0x0001);
	
	
}
#endif

int virtual_ethn_add(struct ntpCtx *ctx)
{
	int i;
	char networkcard[20];//eth0:x
	
	memset(networkcard, 0, sizeof(networkcard));
	
	for(i=0; i<20; i++)
	{
		sprintf(networkcard, "eth0.%d", i+5);
		#if MY_SWITCH
		int ret = SET_OK;
		ret = ethn_add(networkcard, (char *)ctx->ntpSta.vp[i].v_ip, (char *)ctx->ntpSta.vp[i].v_mac, (char *)ctx->ntpSta.vp[i].v_mask, (char *)ctx->ntpSta.vp[i].v_gateway);
		if(ret < 0){
			if(SET_IP_ERR == ret)
			{
				print(	DBG_ERROR, "%s[%d]", "Failed to add virtual ether.", ret);
				return 0;
			}else{
				print(	DBG_ERROR, "%s[%d]", "Failed to add virtual ether.", ret);
			}
		}
		#endif
		if(0 == __Sock_SetMac(networkcard, (char *)ctx->ntpSta.vp[i].v_mac))//??MAC
		{
			return 0;
		}
	}

	#ifdef DNTP_SET_WEB_NETWORK
	sprintf(networkcard, "%s", NETWEB_CARD);
	int ret = SET_OK;
	ret = ethn_add(networkcard, (char *)ctx->ntpSta.vp[20].v_ip, (char *)ctx->ntpSta.vp[20].v_mac, (char *)ctx->ntpSta.vp[20].v_mask, (char *)ctx->ntpSta.vp[20].v_gateway);
	if(ret < 0){
		if(SET_IP_ERR == ret)
		{
			print(	DBG_ERROR, "%s[%d]", "Failed to add virtual ether.", ret);
			return 0;
		}else{
			print(	DBG_ERROR, "%s[%d]", "Failed to add virtual ether.", ret);
		}
	}
	#endif
	
	return 1;
}










struct ntpreq * reqReceive(int sid,struct port_info *info)
{
	u16_t nRecv = 0;
	u8_t buf[256];
	socklen_t addrlen;
	struct sockaddr_in addr;
	struct timeval orgTime;
	struct ntpreq *req = NULL;
	int i;
	struct recvaddr_t recvaddr;
	
	memset(buf, 0, sizeof(buf));
	addrlen = sizeof(struct sockaddr_in);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	
	#if MY_SWITCH
	nRecv = recvfrom(sid, buf, NTP_PKT_MAC, 0, (struct sockaddr *)&addr, &addrlen);//接收NTP的发送的包
	#else
	nRecv = receive_packet(buf, 256,info,&recvaddr);
	#endif
	if(nRecv == 0)
	{
		return NULL;
	}
	
	memset(&orgTime, 0, sizeof(struct timeval));
	#ifdef  TIME_USE_FPGA_TIME
	ntpgettimefpga(gNtpCtx.fpga_fd,gNtpCtx.lp.leapSecond,&orgTime,NULL);//获取FPGA精确时间
	#else
	if(GetSysTimeTV(&orgTime))//获取系统当前精确时间
	{
		return NULL;
	}
	#endif
	
	//printf("rec:%d\n",nRecv);
	if(nRecv < NTP_PKT_NOMAC || nRecv > NTP_PKT_MAC)
	{
		print(	DBG_ERROR, 
				"<%s>--Invalid request packet", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);
		
		return NULL;
	}
	
	if((4 == PKT_MODE(buf[0])) || (5 == PKT_MODE(buf[0])))//工作模式
	{
		return NULL;
	}
	
	if(DBG_DEBUG == get_print_level())
	{
		for(i=0; i<nRecv; i++)
		{
			printf("%02x", buf[i] );
		}
		printf("\n\n");
	}

	req = (struct ntpreq *)malloc(sizeof(struct ntpreq));
	if(NULL == req)
	{
		return NULL;
	}
			
	memcpy(&(req->recvTime), &orgTime, sizeof(struct timeval));
	memcpy(&(req->recvAddr), &recvaddr, sizeof(struct recvaddr_t));
	memcpy(&(req->recvPkt), buf, nRecv);
		
    return req;
}








void reqResponse(void *args)
{
	struct ntpreq *req;
	u8_t version;
	int i;
	int ret;
	l_fp tmpTime;
	u32_t key_id;
	u8_t digest[16];
	md5_ctx md5;
	int send_len;
	struct timeval orgTime;

	req = (struct ntpreq *)args;
	version = PKT_VERSION(req->recvPkt.LVM);//获取版本信息
	//version field must be 2, 3, 4
	if(version < 2 || version > 4)
	{
		return;
	}
	else
	{	
		//version 2 is forbidden
		if( (2 == version) && (0 == ((gNtpCtx.ntpSta.vEn.VnEnable)&0x01)) )
		{
			return;
		}
		//version 3 is forbidden
		if( (3 == version) && (0 == ((gNtpCtx.ntpSta.vEn.VnEnable)&0x02)) )
		{
			return;
		}
		//version 4 is forbidden
		if( (4 == version) && (0 == ((gNtpCtx.ntpSta.vEn.VnEnable)&0x04)) )
		{
			return;
		}
	}

	if( (4 == version || 3 == version || 2 == version)  && (gNtpCtx.ntpSta.mEn.md5Enable) )
	{
		if(gNtpCtx.ntpSta.totalKey > 0 && gNtpCtx.ntpSta.totalKey <= MAX_MD5KEY_NUM)
		{
			key_id = ntohl(req->recvPkt.KeyId);
			if(0 == key_id)
			{
				return;
			}
			for(i=0; i<MAX_MD5KEY_NUM; i++)
			{
				if(key_id == gNtpCtx.ntpSta.mKey[i].md5Id)
				{
					break;
				}
			}
			if(MAX_MD5KEY_NUM == i)
			{
				print(	DBG_WARNING, 
						"<%s>--No matched MD5 key", 
						gDaemonTbl[DAEMON_CURSOR_NTP]);
				
				return;
			}
			else
			{
				md5_init(&md5);
				md5_update(&md5, gNtpCtx.ntpSta.mKey[i].md5Key, gNtpCtx.ntpSta.mKey[i].md5Len);
				md5_update(&md5, (u8_t *)&(req->recvPkt), NTP_PKT_NOMAC);
				md5_final(digest, &md5);//整理和填写输出结果
				if( 0 != memcmp(req->recvPkt.Digest, digest, 16) )//防止被篡改，比如发送一个电子文档，发送前，我先得到MD5的输出结果a。然后在对方收到电子文档后，对方也得到一个MD5的输出结果b。如果a与b一样就代表中途未被篡改。
				{
					print(	DBG_WARNING, 
							"<%s>--Failed to make an authentication", 
							gDaemonTbl[DAEMON_CURSOR_NTP]);
					
					return;
				}
			}
		}
		else
		{
			print(	DBG_WARNING, 
					"<%s>--No MD5 key", 
					gDaemonTbl[DAEMON_CURSOR_NTP]);
			return;
		}
	}
	
	req->recvPkt.LVM = PKT_LI_VN_MODE(li, version, 4);// LI：跳跃指示器，VN：NTP版本号，Mode：工作模式
	req->recvPkt.Stratum = 1;//Stratum按照距离外部UTC源的远近将所有服务器归入不同的Stratun（层）中
	req->recvPkt.Poll = 4;//连续信息间的最大间隔
	req->recvPkt.Precision = -20;//本地时钟精确度
	req->recvPkt.RootDelay = 0;//根时延
	req->recvPkt.RootDispersion = 0;//根离散
	memcpy(req->recvPkt.RefId, "DCLS", 4);//参考时钟标识符,DCLS直流电平携带码
		
	//time when client send request
	memcpy(&(req->recvPkt.OrgStamp), &(req->recvPkt.TranStamp), sizeof(l_fp));//原始时间戳

	//time when server receive request
	TVTOTS(&(req->recvTime), &tmpTime);
	HTONL_TS(&tmpTime, &(req->recvPkt.RecStamp));//接收时间戳

	//time when server last check
	TVTOTS(&(req->recvTime), &tmpTime);
	HTONL_TS(&tmpTime, &(req->recvPkt.RefStamp));//参考时间戳

	//time when server send respond
	memset(&orgTime, 0, sizeof(struct timeval));

	#ifdef TIME_USE_FPGA_TIME
	ntpgettimefpga(gNtpCtx.fpga_fd, gNtpCtx.lp.leapSecond, &orgTime,NULL);
	#else
	if(GetSysTimeTV(&orgTime))
	{
		return;
	}
	#endif
	TVTOTS(&orgTime, &tmpTime);
	HTONL_TS(&tmpTime, &(req->recvPkt.TranStamp));//发送时间戳

	send_len = NTP_PKT_NOMAC;
	if( (4 == version || 3 == version || 2 == version)  && (gNtpCtx.ntpSta.mEn.md5Enable) )//说明该版本是加密的
	{
		if(gNtpCtx.ntpSta.totalKey > 0 && gNtpCtx.ntpSta.totalKey <= MAX_MD5KEY_NUM)
		{
			key_id = ntohl(req->recvPkt.KeyId);
			if(0 == key_id)
			{
				return;
			}
			for(i=0; i<MAX_MD5KEY_NUM; i++)
			{
				if(key_id == gNtpCtx.ntpSta.mKey[i].md5Id)
				{
					break;
				}
			}
			if(MAX_MD5KEY_NUM == i)
			{
				print(	DBG_WARNING, 
						"<%s>--No matched MD5 key", 
						gDaemonTbl[DAEMON_CURSOR_NTP]);
				return;
			}
			else
			{
				md5_init(&md5);
				md5_update(&md5, gNtpCtx.ntpSta.mKey[i].md5Key, gNtpCtx.ntpSta.mKey[i].md5Len);
				md5_update(&md5, (u8_t *)&(req->recvPkt), NTP_PKT_NOMAC);
				md5_final(digest, &md5);
				memcpy(req->recvPkt.Digest, digest, 16);
				send_len = NTP_PKT_MAC;//KeyId占4个字节，Digest占16个字节，共20个字节
			}
		}
		else
		{
			print(	DBG_WARNING, 
					"<%s>--No MD5 key", 
					gDaemonTbl[DAEMON_CURSOR_NTP]);
			return;
		}
	}

	if(1 == SysCheckFpgaRunStatus(fpga_fd))//check fpga run is ok
	{
		#if MY_SWITCH
		ret = sendto(gNtpCtx.ntpSock, &(req->recvPkt), send_len, 0, (struct sockaddr *)&(req->recvAddr), sizeof(struct sockaddr));//发送NTP包
		#else
		ip_select_fb_ident(req->recvAddr.ip, &req->recvAddr.id);
		//fprintf(stderr, "ip:%d id:0x%x\n",req->recvAddr.ip,req->recvAddr.id);
		ret = send_packet((u8_t *)&(req->recvPkt), send_len, &(req->recvAddr));
		#endif
	}else{
		ret = 0;
		print(	DBG_ERROR, 
				"--fpga run status error, ntp package not send.");
	}

	//memcpy((struct ntpkt *)buf,&(req->recvPkt),send_len);
	
	if ( -1 == ret )
	{
		print(	DBG_ERROR, 
				"<%s>--Respond error", 
				gDaemonTbl[DAEMON_CURSOR_NTP]);
	}
	if(DBG_DEBUG == get_print_level())
	{
		for ( i=0; i<send_len; i++ )
		{
			printf("%02x", ((u8_t *)(&req->recvPkt))[i] );
		}
		printf("\n\n");
	}
}


static void
display_sched_attr(int policy, struct sched_param *param)
{
	print(DBG_INFORMATIONAL, "	policy=%s, priority=%d\n",
			(policy == SCHED_FIFO)	? "SCHED_FIFO" :
			(policy == SCHED_RR)	? "SCHED_RR" :
			(policy == SCHED_OTHER) ? "SCHED_OTHER" :
			"???",
			param->sched_priority);
}

void
display_thread_sched_attr(pthread_t pthread)
{
	int policy, s;
	pthread_t c_ph = 0;
	struct sched_param param;
	if(pthread != 0)
		c_ph = pthread;
	else
		c_ph = pthread_self();
	s = pthread_getschedparam(c_ph, &policy, &param);
	if (s != 0){
		fprintf(stderr,"pthread_getschedparam failure\n");
		return;
	}

	print(DBG_INFORMATIONAL, "cur_pthread:%d\n", c_ph);
	display_sched_attr(policy, &param);
}

void
display_process_sched_attr(pid_t pid)
{
	int policy = -1,s = -1;
	struct sched_param param;
	policy = sched_getscheduler(pid);
	if (policy < 0){
		fprintf(stderr, "sched_getscheduler failure\n");
		return;
	}
	s = sched_getparam(pid, &param);
	if (s != 0){
		fprintf(stderr, "sched_getparam failure\n");
		return;
	}
	print(DBG_INFORMATIONAL, "cur_pid:%d\n", pid);
	display_sched_attr(policy, &param);
}


int set_pro_rr(void)
{
	struct sched_param param;
	int maxpri;

	maxpri = sched_get_priority_max(SCHED_RR);//SCHED_RR实时调度策略,时间片轮转,sched_get_priority_max获取实时优先级的最大值

	if (maxpri == -1)
	{
		return -1;
	}
#ifdef NTP_RR_90
	param.sched_priority = maxpri-9;
	print(DBG_INFORMATIONAL, "p350_ntp rtpriority is %d \n",maxpri-9);
#else
	param.sched_priority = maxpri;
	print(DBG_INFORMATIONAL, "p350_ntp rtpriority is %d \n",maxpri);
#endif
	if (sched_setscheduler(getpid(),SCHED_RR,&param) == -1)//设置当前进程为最强的FIFO优先级
	{
		return -1;
	}
	display_process_sched_attr(getpid());
	return 0;
}



int set_pro_fifo(void)
{
	struct sched_param param;
	int maxpri;

	maxpri = sched_get_priority_max(SCHED_FIFO);//SCHED_FIFO实时调度策略,先到先服务,sched_get_priority_max获取实时优先级的最大值

	if (maxpri == -1)
	{
		return -1;
	}

	param.sched_priority = maxpri;
	print(DBG_INFORMATIONAL, "p350_ntp rtpriority is %d \n",maxpri);
	if (sched_setscheduler(getpid(),SCHED_FIFO,&param) == -1){
		return -1;
	}
	display_process_sched_attr(getpid());
	return 0;
}




int ProcNtp(struct ntpCtx *ctx)
{
	int ret = 0;
	fd_set fsr;
	struct timeval tv;
	struct ntpreq *req;
	
	//unsigned int portStatus = 0;
	u8_t i = 0;
	
	initializeContext(ctx);//初始化环境
	initializeBoardNameTable();	//初始化单盘名称索引
	set_print_level(true, DBG_INFORMATIONAL,DAEMON_CURSOR_NTP);
	syslog_init(NULL);

#ifdef NEW_ALARMID_COMM
	if(-1 == initializeAlarmIndexTable())
	{
		ret = __LINE__;
		goto exit;
	}
#endif

	if(0 == initializeDatabase(&ctx->pDb))//初始化数据库
	{
		ret = __LINE__;
		goto exit;
	}

	if(0 == initializeVethDatabase(&ctx->pVethDb)) //init mac ip veth database
	{
		ret = __LINE__;
		goto exit;
	}

	if(0 == md5_key_read(ctx, TBL_MD5_KEY))//从数据库中读md5 key	（MD5 验证，通过对接收的传输数据执行散列运算来检查数据的正确性）
	{
		ret = __LINE__;
		goto exit;
	}

	if(0 == version_enable_read(ctx, TBL_SYS))//从数据库中读版本使能
	{
		ret = __LINE__;
		goto exit;
	}

	if(0 == broadcast_enable_interval_read(ctx, TBL_SYS))//从数据库中读广播使能和时间间隔
	{
		ret = __LINE__;
		goto exit;
	}
	
	if(0 == md5_enable_read(ctx, TBL_SYS))//从数据库中读md5使能
	{
		ret = __LINE__;
		goto exit;
	}
	
	if(0 == read_virtual_eth(ctx, TBL_VETH))//从数据库中读虚拟网卡信息
	{
		ret = __LINE__;
		goto exit;
	}
	
#if 0
	if(-1 == initializePriority(DAEMON_PRIO_NTP))//初始化当前进程的优先级
	{
		ret = -1;
		goto exit;
	}
#endif 

#ifdef NTP_PRO_RR
	if (-1==set_pro_rr())////设置当前进程为最强的FIFO优先级
#else
	if (-1==set_pro_fifo())//设置当前进程为最强的RR优先级
#endif
	{
		ret = __LINE__;
		goto exit;
	}

	if(-1 == initializeSignal())//初始化退出信号
	{
		ret = __LINE__;
		goto exit;
	}
	
	if(-1 == initializeShareMemory(&(ctx->ipc)))//初始化共享内存
	{
		ret = __LINE__;
		goto exit;
	}

	if(-1 == initializeMessageQueue(&(ctx->ipc)))//初始化消息队列
	{
		ret = __LINE__;
		goto exit;
	}

	if(-1 == initializeSemaphoreSet(&(ctx->ipc)))//初始化信号量
	{
		ret = __LINE__;
		goto exit;
	}

	if(-1 == initializeFpga(ctx))//打开FPGA
	{
		ret = __LINE__;
		goto exit;
	}
	fpga_fd = ctx->fpga_fd;
	if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
	{
		ret = __LINE__;
		goto exit;
	}
	/*
	if(0 == ReadLeapsecond(ctx, TBL_SYS))//从系统配置表中读闰秒
	{
		ret = -1;
		goto exit;
	}
	*/
	
	if(-1 == writePid(&(ctx->ipc)))//pid拷贝到共享内存
	{
		ret = __LINE__;
		goto exit;
	}
	
	//SwitchConfig(ctx->fpga_fd); //配置1725交换芯片
	//only ntp,web to dmgr
	CreateNetWorkInterface();
	
	virtual_ethn_add(ctx);//添加虚拟网卡

	if( !initializeSocket(ctx) )//初始化套接字
	{
		ret = __LINE__;
		goto exit;
	}

	if(!tpCreate(&ctx->pool, 3, 1200))//创建3个线程
	{
		ret = __LINE__;
		goto exit;
	}

    if(0 == CreateThread(ctx->loop_flag,&ctx->thread_id))
    {
        ret = __LINE__;
        goto exit;    
    }

	if(0 == writeConfig(ctx))//写配置到共享内存
	{
		ret = __LINE__;
		goto exit;
	}
	#if 1
	if(ctx->ntpSta.bCast.bcEnable)//NTP广播使能
	{
		if(!broadcast_enable(ctx))//定时广播
		{
			ret = __LINE__;
			print(	DBG_ERROR, 
					"<%s>--Failed to enable broadcast.", 
					gDaemonTbl[DAEMON_CURSOR_NTP] );
			goto exit;
		}
	}
	#endif
	#ifdef TIME_USE_FPGA_TIME
	print(DBG_INFORMATIONAL, "--TIME_USE_FPGA_TIME is enable.\n");
	#endif
	int alm_ntp_exit_inx = -1;
	FIND_ALMINX_BY_SLOT_PORT_ID(SNMP_ALM_ID_NTP_EXIT, 1, SID_SYS_PORT, alm_ntp_exit_inx);
	if(alm_ntp_exit_inx == -1)
		return 0;
	ctx->alarm[alm_ntp_exit_inx].alm_sta = ALM_OFF;
	WriteAlarmToShareMemory(ctx);

	while(isRunning(ctx))
	{
		if(isNotify(ctx))//有通知
		{
			if(0 == ReadMessageQueue(ctx))//读取消息，并修改配置表
			{
				ret = __LINE__;
				print(	DBG_ERROR, 
				  	  	"<%s>--%s", 
				  	 	gDaemonTbl[DAEMON_CURSOR_NTP], 
				  	  	"ReadMessageQueue Error.");
				break;
			}
		}
		
		FD_ZERO(&fsr);
		int max_fd = -1;
		for(i = 0; i < ARRAY_SIZE(port); i++)
		{
			if(port[i].sock <= 0 && create_raw_socket(&port[i]) <= 0)
			{
				continue;	
			}
				
			
			if(port[i].sock > max_fd)
				max_fd = port[i].sock;	
			FD_SET(port[i].sock, &fsr);	
		}
		if(max_fd < 0)
			continue;	
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if(0 < select(max_fd +1, &fsr, NULL, NULL, &tv))
		{
			
			for(i = 0; i < ARRAY_SIZE(port); i++)
			{
				if(port[i].sock <= 0)
				{
					continue;
				}
				if(FD_ISSET(port[i].sock, &fsr))
				{
					//receive_packet(buffer, 1536, &port[i], &recvaddr);
					/*
					printf("%d-----socket:%d--%s--%02hhu.%02hhu.%02hhu.%02hhu %02hhu.%02hhu.%02hhu.%02hhu   "
					
					"%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx  %08x  %08x\n",i,port[i].sock,port[i].name,
					port[i].ip_a[0],
					port[i].ip_a[1],
					port[i].ip_a[2],
					port[i].ip_a[3],
					port[i].mask_a[0],
					port[i].mask_a[1],
					port[i].mask_a[2],
					port[i].mask_a[3],
					port[i].mac[0],
					port[i].mac[1],
					port[i].mac[2],
					port[i].mac[3],
					port[i].mac[4],
					port[i].mac[5],
					port[i].ip,
					port[i].mask);
					*/
					req = NULL;
					req = reqReceive(ctx->ntpSock,&port[i]);
					#if 1 //IF_FPGA_FAULT_STOP_NTP
					if(0 == SysCheckFpgaRunStatus(ctx->fpga_fd))//check fpga run is ok
					{
						ctx->alarm[alm_ntp_exit_inx].alm_sta = ALM_ON;
						WriteAlarmToShareMemory(ctx);
						ret = __LINE__;
						goto exit;
					}
					#endif

					if(NULL != req)
					{
						if (li != 0 || 1 != ntpchecktimefpga(ctx->fpga_fd)){
							free(req);
							req = NULL;
							continue;
						}
						if( !tpAddTask(&ctx->pool, reqResponse, req) )//将同步请求包、收包时间、客户端信息添加到任务队列，然后唤醒同步请求处理线程
						{
							free(req);
							req = NULL;
							print(	DBG_ERROR, 
									"--%s", 
									"tpAddTask Error.");
						}
					}
		
				}

			}
			
		}
		else
		{
			
		}
		
		
		
		
		
	}

exit:
	cleanAlarmIndexTable();
	tpDestroy(&ctx->pool);
	cleanDatabase(ctx->pDb);
	cleanVethDatabase(ctx->pVethDb);
	broadcast_disable();
	cleanSock();
	cleanFpga(ctx);

	if(ret == 0)
		print(DBG_WARNING, "process exit clearly![%d]\n",ret);
	else
		print(DBG_ERROR, "process exit clearly![%d]\n",ret);
	syslog_exit();
	
	return ret;
}






