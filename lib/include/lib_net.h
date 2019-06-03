#ifndef	__LIB_NET__
#define	__LIB_NET__

/*ethn_add() ret*/
enum {
	SET_OK= 1,
	SET_IP_ERR = -2,
	SET_MASK_ERR=-3,
	SET_GW_ERR = -4,
	SET_MAC_ERR = -5,
};

int __Sock_GetIp( char * networkcard, char * local_ip );
int __Sock_SetIp( char * networkcard, char * local_ip );

int __Sock_GetMask( char * networkcard, char * local_mask );
int __Sock_SetMask( char * networkcard, char * local_mask );

int __Sock_GetMac( char * networkcard, char * local_mac );
int __Sock_SetMac( char * networkcard, char * local_mac );
//int __Sock_SetMac( const char * networkcard, unsigned char * local_mac );

int __Sock_SetGateway( char * networkcard, char * old_gateway, char * new_gateway );
//int __Sock_SetGateway( char * local_gateway );
int __Sock_GetGateway( char * local_gateway );

int __Sock_GetBcast( char * networkcard, char * local_bcast );


int gateway_validity(char *gateway);
int mask_validity(char *mask);
int ip_validity(char *);

int mac_validity(char *mac);
int dns_validity(char *dns);
int net_validity(char *ip, char *netmask, char *gateway);


int ethn_down(char * networkcard);
int ethn_up(char * networkcard);
int ethn_add( char * networkcard, char * local_ip, char * local_mac, char * local_mask, char * local_gateway);
int ethn_update( char * networkcard, char * local_ip, char * local_mask );

void net_padding(char *imgd);

void dec_hex(int o_ip,char *pp);

void ip_to_hex(char *ip);





#endif//__LIB_NET__


