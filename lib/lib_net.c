//  Interface request structure used for socket ioctl's.
//  All interface ioctl's must have parameter definitions which begin with ifr_name.
//	The remainder may be interface specific.
/*	
struct ifreq
{
	#define IFHWADDRLEN 6
	#define IFNAMSIZ IF_NAMESIZE
    union
    {
        char ifrn_name[IFNAMSIZ]; //Interface name, e.g. "eth0".
    } ifr_ifrn;

    union
    {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_dstaddr;
        struct sockaddr ifru_broadaddr;
        struct sockaddr ifru_netmask;
        struct sockaddr ifru_hwaddr;
        short int ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        struct ifmap ifru_map;
        char ifru_slave[IFNAMSIZ]; //Just fits the size
        char ifru_newname[IFNAMSIZ];
        __caddr_t ifru_data;
    } ifr_ifru;
};

#define	ifr_name	ifr_ifrn.ifrn_name 		//interface name
#define ifr_hwaddr	ifr_ifru.ifru_hwaddr 	//MAC address
#define ifr_addr	ifr_ifru.ifru_addr 		//address
#define ifr_dstaddr ifr_ifru.ifru_dstaddr 	//other end of p-p lnk
#define ifr_broadaddr	ifr_ifru.ifru_broadaddr //broadcast address
#define ifr_netmask		ifr_ifru.ifru_netmask 	//interface net mask
#define ifr_flags	ifr_ifru.ifru_flags 	//flags
#define ifr_metric	ifr_ifru.ifru_ivalue 	//metric
#define ifr_mtu		ifr_ifru.ifru_mtu 	//mtu
#define ifr_map		ifr_ifru.ifru_map 	//device map
#define ifr_slave	ifr_ifru.ifru_slave //slave device
#define ifr_data	ifr_ifru.ifru_data 	//for use by interface
#define ifr_ifindex ifr_ifru.ifru_ivalue 	//interface index
#define ifr_bandwidth ifr_ifru.ifru_ivalue 	//link bandwidth
#define ifr_qlen	ifr_ifru.ifru_ivalue 	//queue length
#define ifr_newname ifr_ifru.ifru_newname 	//New name

#define _IOT_ifreq			_IOT( _IOTS(char), IFNAMSIZ, _IOTS(char), 16, 0, 0 )
#define _IOT_ifreq_short 	_IOT( _IOTS(char), IFNAMSIZ, _IOTS(short), 1, 0, 0 )
#define _IOT_ifreq_int 		_IOT( _IOTS(char), IFNAMSIZ, _IOTS(int), 1, 0, 0 )
*/

// Structure used in SIOCGIFCONF request. Used to retrieve interface
// configuration for machine (useful for programs which must know all
// networks accessible).
/*
struct ifconf

{
    int ifc_len; //Size of buffer.
    union
    {

        __caddr_t ifcu_buf;
        struct ifreq *ifcu_req;
    } ifc_ifcu;
};
#define ifc_buf 	ifc_ifcu.ifcu_buf //Buffer address.

#define ifc_req 	ifc_ifcu.ifcu_req //Array of structures.
#define	_IOT_ifconf _IOT( _IOTS(struct ifconf), 1, 0, 0, 0, 0 ) //not right
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>

#include "lib_net.h"


int __Sock_GetIp( char * networkcard, char * local_ip ) 
{
	int sock_fd;
	struct ifreq ifr;
	struct sockaddr_in * ptr = NULL;
	
	if ( ( sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
	 	close(sock_fd);
		return (0);
	}

	memset( &ifr, 0, sizeof(struct ifreq) );
	//printf("net:%s\n",networkcard);
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if ( ioctl( sock_fd, SIOCGIFADDR, &ifr, sizeof(struct ifreq) ) < 0 )
	{
	  	close(sock_fd);
		return (0);
	}
	
	ptr = (struct sockaddr_in *) &( ifr.ifr_addr );	
	strncpy( local_ip, inet_ntoa( ptr->sin_addr ), 16 );
	
	close( sock_fd );
	return (1);
}



int __Sock_SetIp( char * networkcard, char * local_ip )
{
	struct sockaddr_in sin;
	struct ifreq ifr;
	int sock_fd;
	
	if ( ( sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
	 	close(sock_fd);
		return (0);
	}

	//close the interface
	memset(&ifr, 0, sizeof(ifr));
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) //传入网络接口名称，获得标志
	{
		close(sock_fd);
		return (0);
	}
	 
	ifr.ifr_flags &= (~IFF_UP);//关闭网卡
  	if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0) 
  	{
  		close(sock_fd);
		return (0);
	}

	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );	
	memset( &sin, 0, sizeof(sin) );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(local_ip);
	
	memmove( &ifr.ifr_addr, &sin, sizeof(sin) );//拷贝函数，设置网卡信息	
	if ( ioctl( sock_fd, SIOCSIFADDR, &ifr ) < 0 )
	{
		close(sock_fd);
		return (0);
	}
	
	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		close(sock_fd);
		return (0);
	}
	
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);//启动网卡
	if ( ioctl ( sock_fd, SIOCSIFFLAGS, &ifr) < 0 )
	{
		close(sock_fd);
		return (0);
	}
	
	close(sock_fd);
  	return (1);
}



int __Sock_GetMask( char * networkcard, char * local_mask )
{
	int sock_fd;
	struct ifreq ifr;
	struct sockaddr_in *ptr = NULL;
	
	if ( (sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		close(sock_fd);
		return (0);
	}

	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );

	if ( ioctl( sock_fd, SIOCGIFNETMASK, &ifr ) < 0 )
	{
		close(sock_fd);
		return (0);
	}

	ptr = (struct sockaddr_in *)&ifr.ifr_netmask;
	strncpy( local_mask, inet_ntoa(ptr->sin_addr), 16 );
    
	close(sock_fd);
	return (1);
}



int __Sock_SetMask( char *networkcard, char * local_mask )
{
	int sock_fd;
	struct ifreq ifr;
	struct sockaddr_in netmask_addr;

	if( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		close(sock_fd);
		return (0);
	}

	//close the interface
	memset(&ifr, 0, sizeof(ifr));
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		close(sock_fd);
		return (0);
	}
	 
	ifr.ifr_flags &= (~IFF_UP);
  	if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0) 
  	{
  		close(sock_fd);
		return (0);
	}

	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	memset( &netmask_addr, 0, sizeof(struct sockaddr_in) );
	netmask_addr.sin_family = AF_INET;
	netmask_addr.sin_addr.s_addr = inet_addr(local_mask);

	memmove( &ifr.ifr_netmask, &netmask_addr, sizeof(struct sockaddr_in) );

	if ( ioctl( sock_fd, SIOCSIFNETMASK, &ifr ) < 0 )//设置MASK
	{
		close(sock_fd);
		return (0);
	}
	
	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		close(sock_fd);
		return (0);
	}
	
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if ( ioctl ( sock_fd, SIOCSIFFLAGS, &ifr) < 0 )
	{
		close(sock_fd);
		return (0);
	}
	
	close(sock_fd);
	return (1);
}



int __Sock_GetMac( char * networkcard, char * local_mac )
{
	int sock_fd;
	struct ifreq ifr;
	unsigned char * ptr = NULL;
	unsigned char buf2[18];

	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		close(sock_fd);
		return(0);
	}

	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if ( ioctl( sock_fd, SIOCGIFHWADDR, &ifr ) < 0 )
	{
		close(sock_fd);
		return(0);
	}
	
	ptr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	memset( buf2, 0, sizeof(buf2) );
	
	sprintf( (char *)buf2,"%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5) );
	memcpy( local_mac, buf2, 18 );

	close(sock_fd);	
	return (1);
}



/*
int __Sock_SetMac( const char * networkcard, unsigned char * local_mac )
{
	char mac_add[64];
	
	//mac such as 000fb0defc00
	memset( mac_add, '\0', sizeof(mac_add) );
	sprintf( mac_add, "ifconfig %s hw ether %s", networkcard, local_mac );
	
	if ( -1 == system( mac_add ) )
	{
		return (0);
	}
	return (1);
}*/



//mac such as 00:0f:b0:de:fc:00
int __Sock_SetMac( char * networkcard, char * local_mac )
{
	char MAC[6];
	char *hwptr = NULL;
	char tmp[18];
	int sock_fd = -1;
	struct ifreq ifr;
	struct sockaddr *hwaddr = NULL;
	int i;

	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		close(sock_fd);
		return(0);
	}
	
	memset(MAC, 0, sizeof(MAC));
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, local_mac, 17);
	hwptr = tmp;
	
	for(i = 0; i < 6; i++)
	{
		hwptr = strtok((char *)hwptr,":");
		MAC[i] = strtol((char *)hwptr,NULL,16);			
		hwptr = NULL;
	}

	//close the interface
	memset(&ifr, 0, sizeof(ifr));
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		close(sock_fd);
		return (0);
	}
	 
	ifr.ifr_flags &= (~IFF_UP);
  	if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0) 
  	{
  		close(sock_fd);
		return (0);
	}
  	
  	//set the MAC address
  	memset(&ifr, 0, sizeof(ifr));
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	hwaddr = (struct sockaddr *)&ifr.ifr_hwaddr;
	hwaddr->sa_family = ARPHRD_ETHER;
	hwptr = hwaddr->sa_data;

	for(i=0; i<6; i++)
	{
		*hwptr++ = (unsigned char)(MAC[i] & 0xFF);
	}
		
	if (ioctl(sock_fd, SIOCSIFHWADDR, &ifr) < 0)
	{
		close(sock_fd);
		return (0);
	}
	
	//open the interface
	memset(&ifr, 0, sizeof(ifr));
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		close(sock_fd);
		return (0);
	}
	 
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
  	if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0) 
  	{
  		close(sock_fd);
		return (0);
	}
	
	close(sock_fd);
	return (1);
}



int __Sock_GetGateway( char *local_gateway )
{
	char buf[1024];
	int pl=0,ph=0;
	int i;
	FILE *readfp;
	
	memset( buf,0,sizeof(buf) );
	readfp = popen( "/sbin/route -n", "r" );
	if ( NULL == readfp )
	{
		return(0);
	}

	if ( 0 == fread( buf, sizeof(char), sizeof(buf),  readfp) )
	{
		pclose( readfp );
		return (0);
	}
	pclose( readfp );

   	for( i=0; i<1024; i++ )
   	{
   		if ( buf[i]=='U' && buf[i+1]=='G' && (i+1)<1024 )
   		{
   				pl=i;
   				break;
   		}
   	}
	if( pl == 0 )
	{
		return(0);
	}
   	
   	while ( buf[pl]>'9'|| buf[pl]<'0' )
   	{
   		pl--;
   	}
   	while ( (buf[pl]>='0' && buf[pl]<='9') || buf[pl]=='.')
   	{
   		pl--;
   	}
   	while ( buf[pl]>'9' || buf[pl]<'0' )
   	{
   		pl--;
   	}
	buf[pl+1]='\0';

   	for ( i=pl; ((buf[i]>='0' && buf[i]<='9') || buf[i]=='.'); i--);
   	ph = i+1;
	strcpy(local_gateway, &buf[ph]);

	return (1);
}



/*
int __Sock_SetGateway( char * local_gateway )
{
	char cmd[64] = "route add default gw ";

	if ( -1 == system("route del default") )
	{
		return (0);
	}
	
	strcat( cmd, local_gateway );
	if (-1 == system( cmd ) )
	{
		return(0);
	}
	return (1);
}*/



int __Sock_SetGateway( char * networkcard, char * old_gateway, char * new_gateway )
{
	int sock_fd = -1;
	struct rtentry rt;
	struct sockaddr rtsockaddr;
	struct sockaddr_in	*sin = NULL;
	
	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		close(sock_fd);
		return(0);
	}
	
	memset(&rt, 0, sizeof(struct rtentry));
	memset(&rtsockaddr, 0, sizeof(struct sockaddr));
	
	sin = (struct sockaddr_in *)&rtsockaddr;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	
	rt.rt_dst = rtsockaddr;		//set the destination address to '0.0.0.0'
	rt.rt_genmask = rtsockaddr;	//set the netmask to '0.0.0.0'
	
	//Fill in the other fields.
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 0;
	rt.rt_dev = networkcard;
	
	//delete the current default gateway. 
	sin->sin_addr.s_addr = inet_addr(old_gateway);
	rt.rt_gateway = rtsockaddr;

	errno = 0;
	if (ioctl(sock_fd, SIOCDELRT, &rt) < 0) 
	{
		if( ESRCH != errno )
		{
			close(sock_fd);
	   		return(0);
		}
	}
	//set the new default gateway. 
	sin->sin_addr.s_addr = inet_addr(new_gateway);
	rt.rt_gateway = rtsockaddr;
	    
	if (ioctl(sock_fd, SIOCADDRT, &rt) < 0)
	{
		close(sock_fd);
		return(0);
	}
	
	close(sock_fd);
	return(1);
}










/*
  0	失败
  1	成功
*/
int __Sock_GetBcast( char * networkcard, char * local_bcast )
{
	int sock_fd;
	struct ifreq ifr;
	struct sockaddr_in * ptr = NULL;

	if( ( sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
	 	close(sock_fd);
		return (0);
	}
	
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, networkcard, strlen(networkcard)); 
	if( -1 == ioctl(sock_fd, SIOCGIFBRDADDR, &ifr) )
	{
		close(sock_fd);
		return 0;
	}

	ptr = (struct sockaddr_in *)&ifr.ifr_broadaddr;
	memcpy(local_bcast, inet_ntoa(ptr->sin_addr), 16 );

	close(sock_fd);
	return 1;
}







/*
  0	不合法
  1 合法
*/
int ip_validity(char *ipv4)
{
	#if 0
	if( INADDR_NONE == inet_addr((const char *)ip) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
	#endif

	struct in_addr addr;
    if(ipv4 == NULL)
        return 0;
    if(inet_pton(AF_INET, ipv4, (void *)&addr) == 1)
        return 1;
    return 0;
}


/*
  0	不合法
  1 合法
*/
int mask_validity(char *mask)
{
	if( INADDR_NONE == inet_addr((const char *)mask) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


/*
  0	不合法
  1 合法
*/
int gateway_validity(char *gateway)
{
	if( INADDR_NONE == inet_addr((const char *)gateway) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


/*
  0	不合法
  1 合法
*/
int mac_validity(char *mac)
{
	int i,j;
	int len;
	int count;
	char tmp[6][3];
	char *endptr;
	long retval;

	len = strlen((const char *)mac);
	if(17 != len)
	{
		//长度无效
		return 0;
	}

	count = 0;
	for(i=0; i<17; i++)
	{
		if(':' == mac[i])
		{
			count++;
		}
	}
	if(5 != count)
	{
		//冒号数量不等于5
		return 0;
	}
	
	if( (':' != mac[2]) || (':' != mac[5]) ||
		(':' != mac[8]) || (':' != mac[11]) || (':' != mac[14]) )
	{
		//冒号位置无效
		return 0;
	}

	for(i=0,j=0; i<6&&j<16; i++,j+=3)
	{
		strncpy(tmp[i], &mac[j], 2);
		tmp[i][2] = '\0';
		errno = 0;
		retval = strtol(tmp[i], &endptr, 16);
		if(0 != errno || endptr == tmp[i])
		{
			//转换失败
			return 0;
		}
	}
	
	return 1;
}








/*
  0	不合法
  1 合法
*/
int dns_validity(char *dns)
{
	if( INADDR_NONE == inet_addr((const char *)dns) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}








/*
  0	不合法
  1 合法
*/
int net_validity(char *ip, char *netmask, char *gateway)
{
	unsigned int hl_ip, hl_mask, hl_gw;

	hl_ip = ntohl(inet_addr(ip));
	hl_mask = ntohl(inet_addr(netmask));
	hl_gw = ntohl(inet_addr(gateway));

	if(	(hl_gw < ((hl_ip & hl_mask) | (~hl_mask))) && 
		(hl_gw > (hl_ip & hl_mask)) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}





int ethn_down(char * networkcard)
{
	int sock_fd;
	struct ifreq ifr;
		
	if ( ( sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
	 	close(sock_fd);
		return (0);
	}
	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) //读取设备的活动标志
	{
		close(sock_fd);
		return (0);
	}
	
	ifr.ifr_flags &= (~IFF_UP);
	if ( ioctl ( sock_fd, SIOCSIFFLAGS, &ifr) < 0 )//设置设备的活动标志
	{
		close(sock_fd);
		return (0);
	}
	
	close(sock_fd);
  	return (1);
}



int ethn_up(char * networkcard)
{
	int sock_fd;
	struct ifreq ifr;
		
	if ( ( sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
	 	close(sock_fd);
		return (0);
	}
	memset( &ifr, 0, sizeof(struct ifreq) );
	strncpy( ifr.ifr_name, networkcard, IFNAMSIZ );
	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) 
	{
		close(sock_fd);
		return (0);
	}
	
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if ( ioctl ( sock_fd, SIOCSIFFLAGS, &ifr) < 0 )
	{
		close(sock_fd);
		return (0);
	}
	
	close(sock_fd);
  	return (1);
}





int ethn_add( char * networkcard, char * local_ip, char * local_mac, char * local_mask, char * local_gateway)
{
	char old_gateway[16];

	if('\0' != local_ip[0])
	{
		if(0 == __Sock_SetIp(networkcard, local_ip))//设置ip
		{
			//致命性错误，退出
			return SET_IP_ERR;
		}
	}

	if('\0' != local_mask[0])
	{
		if(0 == __Sock_SetMask(networkcard, local_mask))//设子网掩码
		{
			ethn_up(networkcard);//非致命性错误，恢复网口
			return SET_MASK_ERR;
		}
	}

	if( NULL != local_mac&&'\0' != local_mac[0])
	{
		if(0 == __Sock_SetMac(networkcard, local_mac))//设MAC
		{
			ethn_up(networkcard);//非致命性错误，恢复网口
			return SET_MAC_ERR;
		}
	}

	if('\0' != local_gateway[0])
	{
		if(0 == __Sock_GetGateway(old_gateway))//获取网关失败
		{
			if(0 == __Sock_SetGateway(networkcard, local_gateway, local_gateway))//设置新网关
			{	
				ethn_up(networkcard);//非致命性错误，恢复网口
				return SET_GW_ERR;
			}
		}
		else
		{
			//del old gateway and add new gateway
			if(0 == __Sock_SetGateway(networkcard, old_gateway, local_gateway))//获取成功则设置网关
			{
				ethn_up(networkcard);//非致命性错误，恢复网口
				return SET_GW_ERR;
			}
		}
	}

	return 1;
}









//ip mask gw dns
//172.18.1.6 --> 172.018.001.006
void net_padding(char *imgd)
{
	unsigned int hl;
	
	hl = ntohl(inet_addr(imgd));
	memset(imgd, 0, strlen(imgd));
	sprintf(imgd, "%03d.%03d.%03d.%03d", (hl>>24)&0x000000FF, (hl>>16)&0x000000FF, (hl>>8)&0x000000FF, hl&0x000000FF);
}


void dec_hex(int o_ip,char *pp)
{	
	char phex[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	char hight,low;
	hight = o_ip / 16;
	low = o_ip % 16;
	sprintf(pp,"%c%c\0",phex[hight],phex[low]);
}
	   
void ip_to_hex(char *ip)
{
	char *result = NULL;
	int o_ip[4] = {0};
	char pp[4][3] = {"0","0","0","0"};
	int i = 0;
    result = strtok( ip, "." );
	while( i < 4 && result != NULL ) {
		o_ip[i] = atoi(result);
		dec_hex(o_ip[i],pp[i]);
		i++;
		result = strtok( NULL, "." );
	}
	sprintf(ip,"%s%s%s%s",pp[0],pp[1],pp[2],pp[3]);
}







