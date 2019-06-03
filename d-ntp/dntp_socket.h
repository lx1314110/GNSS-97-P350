#ifndef	__DNTP_SOCKET__
#define	__DNTP_SOCKET__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/select.h>
#include <linux/filter.h>
#include <stdint.h>
#include <asm/byteorder.h>


//#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
//#define DEBUG

#ifdef DEBUG
#define debug(fmt, arg...) printf(fmt, ##arg)
#else
#define debug(fmt, arg...)
#endif



/* *********************************************8********* */

struct ether_hdr {
	uint8_t dst[6];
	uint8_t src[6];
	uint16_t type;//arp: 0x0806, ip: 0x0800
}__attribute__((packed));

struct arp_hdr {
	uint16_t hard_type;//ether: 0x0001
	uint16_t pro_type;//ip: 0x0800
	uint8_t hard_len;//ether: 6
	uint8_t pro_len;//ip: 4
	uint16_t op;//request:1, reply: 2
	/*
		sender mac;
		sender ip;
		dst mac;
		dst ip;
	*/
}__attribute__((packed));
#define arp_src_mac(ahdr) ((unsigned char *)((struct arp_hdr *)(ahdr) + 1) + 0)
#define arp_src_ip(ahdr) ((unsigned char *)((struct arp_hdr *)(ahdr) + 1) + 6)
#define arp_dst_mac(ahdr) ((unsigned char *)((struct arp_hdr *)(ahdr) + 1) + 10)
#define arp_dst_ip(ahdr) ((unsigned char *)((struct arp_hdr *)(ahdr) + 1) + 16)


struct ip_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		uint8_t	iphdr_len:4,
				version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
		uint8_t	version:4,
				iphdr_len:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
		uint8_t tos;
		uint16_t total_len;
		uint16_t id;
		uint16_t frag_off;
		uint8_t	ttl;
		uint8_t	protocol;
		uint16_t checksum;
		uint32_t saddr;
		uint32_t daddr;
}__attribute__((packed));

#define ip_src_ip(ihdr) ((unsigned char *)&(((struct ip_hdr*)(ihdr))->saddr))
#define ip_dst_ip(ihdr) ((unsigned char *)&(((struct ip_hdr*)(ihdr))->daddr))

struct icmp_hdr {
	uint8_t	type;
	uint8_t	code;
	uint16_t checksum;
}__attribute__((packed));


struct udp_hdr {
	uint16_t	src_port;
	uint16_t	dst_port;
	uint16_t	len;
	uint16_t	checksum;
}__attribute__((packed));

struct pseudo_hdr {
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t zero;
	uint8_t protocol;
	uint16_t len;
}__attribute__((packed));

/*********************************************************************/

/*********************************************************************/
struct port_info {
	int sock;
	char name[IF_NAMESIZE];
	unsigned char mac[6];
	unsigned char ip_a[4];
	unsigned char mask_a[4];
	uint32_t ip;
	uint32_t mask;
	
};


struct recvaddr_t {

	uint16_t port;
	uint32_t ip;
	unsigned char mac[6];
	uint16_t id;
	struct port_info *info;
};


extern struct port_info port[20];
int receive_packet(unsigned char *buffer, unsigned int buffer_size,struct port_info *info, struct recvaddr_t *recvaddr);
int send_packet(unsigned char *ntp_data, unsigned int len, struct recvaddr_t *recvaddr);

int create_raw_socket(struct port_info *info);

#endif