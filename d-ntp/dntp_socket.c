
#include "dntp_socket.h"


/* calculate ip header check sum */
unsigned short checksum(void *buf, unsigned len)  
{  
	unsigned long sum = 0;
	unsigned short *word = buf;
	unsigned short last_word = 0;


	while(len >=2)
	{
		sum += *(word++);
		len -= 2;
	}
	if(len == 1)
	{
		*((unsigned char *)&last_word) = *(unsigned char *)word;
		sum += last_word;
	}

	sum = (sum >> 16) + (sum & 0xffff);  
	sum += (sum >> 16);  
  
	return (unsigned short)~sum;  
}


/* caclculate udp/tcp check sum */
unsigned long checksum_sum(void *buf, unsigned len, unsigned init)  
{  
	unsigned long sum = init;
	unsigned short *word = buf;
	unsigned short last_word = 0;


	while(len >=2)
	{
		sum += *(word++);
		len -= 2;
	}
	if(len == 1)
	{
		*((unsigned char *)&last_word) = *(unsigned char *)word;
		sum += last_word;
	}

	return sum;  
}  

unsigned short checksum_check(unsigned long sum)  
{
	sum = (sum >> 16) + (sum & 0xffff);  
	sum += (sum >> 16);  
  
	return (unsigned short)~sum;  
}  

int create_raw_socket(struct port_info *info)
{
	struct ifreq ifr;
	struct sockaddr_ll sll;
	int sock;
	int if_index;
	int so_reuseaddr = 1;
	
	/* create socket */
	if((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0 )
	{
		debug("Create raw socket error\n");
		return -1;
	}
	
	debug("Create raw socket %d\n", sock);
	
	/* get ifindex */
	strncpy(ifr.ifr_name, info->name, IF_NAMESIZE);
	if(ioctl(sock, SIOCGIFINDEX, &ifr) != 0)
	{
		debug("Get %s ifindex error\n", info->name);
		close(sock);
		return -1;
	}
	if_index = ifr.ifr_ifindex;
	
	/* get mac */
	if(ioctl(sock, SIOCGIFHWADDR, &ifr) != 0)
	{
		debug("Get %s mac error\n", info->name);
		close(sock);
		return -1;
	}
	memcpy(info->mac, ifr.ifr_hwaddr.sa_data, 6);
	
	#if 0
	/* PROMISC */
	if ((ioctl(sock, SIOCGIFFLAGS, &ifr)) != 0)
	{
		close(sock);
		return-1;
	}
	ifr.ifr_flags |= IFF_PROMISC;
	if ((ioctl(sock, SIOCSIFFLAGS, &ifr)) != 0)
	{
		close(sock);
		return -1;
	}
	#endif
	
	if( 0 != setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(int) ) )//set sock 
	{
		close(sock);
		return -1; 
	}
	
	/* bind */
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = PF_PACKET; 
	sll.sll_ifindex = if_index;
	sll.sll_protocol = htons(ETH_P_ALL); 
	if(bind(sock, (struct sockaddr *)&sll, sizeof(sll)) == -1 )
	{ 
		debug( "bind raw sock to %s error\n", info->name);
		close(sock);
		return -1; 
	}

	info->sock = sock;

	return sock;
}


int receive_packet(unsigned char *buffer, unsigned int buffer_size,
			 struct port_info *info, struct recvaddr_t *recvaddr)
{
    unsigned char buf[1536];
	struct ether_hdr *ehdr = (struct ether_hdr *)buf;
	struct arp_hdr *ahdr;
	struct ip_hdr *ihdr;
	struct icmp_hdr *ichdr;
	struct udp_hdr *uhdr;
//	struct pseudo_hdr pseudo;
	
	unsigned long ip_packet_len;//header + payload
	unsigned long ip_hdr_len;
	unsigned long ip_payload_len;
	
//	unsigned long sum;
	int len;

	len = recv(info->sock, buf, sizeof(buf), 0);
	if(len <= 0)
	 	return len;

	switch(__be16_to_cpu(ehdr->type))
	{
		case 0x0806:
			debug("\n----ARP----\n");
			ahdr = (struct arp_hdr *)(buf + sizeof(struct ether_hdr));
				/* only support ethernet IPv4 arp */
			if(__be16_to_cpu(ahdr->hard_type) != 1 ||
				__be16_to_cpu(ahdr->pro_type) != 0x0800 ||
				ahdr->hard_len != 6 ||
				ahdr->pro_len != 4)
			{
				debug("unsupported arp type\n");
				break;
			}
	
			switch(__be16_to_cpu(ahdr->op))
			{
				case 1: /* arp request */
					if(memcmp(arp_dst_ip(ahdr), info->ip_a, 4) == 0) //request my ip
					{
						memcpy(ehdr->dst, ehdr->src, 6);
						memcpy(ehdr->src, info->mac, 6);
						ahdr->op = __cpu_to_be16(2);
						memcpy(arp_dst_mac(ahdr), arp_src_mac(ahdr), 6);
						memcpy(arp_dst_ip(ahdr), arp_src_ip(ahdr), 4);
						memcpy(arp_src_mac(ahdr), ehdr->src, 6);
						memcpy(arp_src_ip(ahdr), info->ip_a, 4);

						send(info->sock, buf, (len < 60)?60:len, 0);
					}else{
						debug("other_arp\n");
					}

					/* update arp table */
					break;

				case 2: /* arp reply */
			
					break;
		
				default:
					debug("unknown ARP OP code %x\n", __be16_to_cpu(ahdr->op));
					break;
		
			}
			
			break;

			
		case 0x0800:
			debug("\n----IP----\n");
			ihdr = (struct ip_hdr *)(buf + sizeof(struct ether_hdr));
			if(ihdr->version != 4)
			{
				debug("only IPv4 supported\n");
				break;
			}
		
			ip_hdr_len = ihdr->iphdr_len * 4;
			if(ip_hdr_len < sizeof(struct ip_hdr))
			{
				debug("bad ip packet\n");
				break;
			}
			ip_packet_len = __be16_to_cpu(ihdr->total_len);
			ip_payload_len = ip_packet_len - ip_hdr_len;
		
			if(ihdr->daddr != __cpu_to_be32(info->ip)) //request my ip
			{
				break;
			}
			/*
			if((__be32_to_cpu(ihdr->saddr) & info->mask) != (info->ip & info->mask))
			{
				break;
			}
			*/
			debug("%hhu.%hhu.%hhu.%hhu -> %hhu.%hhu.%hhu.%hhu\n",
					ip_src_ip(ihdr)[0],
					ip_src_ip(ihdr)[1],
					ip_src_ip(ihdr)[2],
					ip_src_ip(ihdr)[3],
					ip_dst_ip(ihdr)[0],
					ip_dst_ip(ihdr)[1],
					ip_dst_ip(ihdr)[2],
					ip_dst_ip(ihdr)[3]);
			
			if(ihdr->protocol == 0x01)
			{/* icmp */
				debug("\n   ----ICMP----\n");
				ichdr = (struct icmp_hdr *)((unsigned char *)ihdr + ip_hdr_len);
				if((ichdr->type != 8) && (ichdr->code != 0))
				{/* not echo request */
					break;
				}
			
				memcpy(ehdr->dst, ehdr->src, 6);
				memcpy(ehdr->src, info->mac, 6);
			
				ihdr->daddr = ihdr->saddr;
				ihdr->saddr = __cpu_to_be32(info->ip);
				ihdr->checksum = 0;
				ihdr->checksum = checksum(ihdr, ip_hdr_len);
				
				ichdr->type = 0;
				ichdr->checksum = 0;
				ichdr->checksum = checksum(ichdr, ip_payload_len);
				send(info->sock, buf, (len < 60)?60:len, 0);
			
			}
			else if(ihdr->protocol == 0x11)
			{/* udp */
				debug("\n    ----UDP----\n");
				uhdr = (struct udp_hdr *)((unsigned char *)ihdr + ip_hdr_len);
				if(uhdr->dst_port == __cpu_to_be16(123)) //ntp
				{
					//printf("len:%d\n",__be16_to_cpu(uhdr->len));
					if( __be16_to_cpu(uhdr->len )> buffer_size)
						break;
					
					memcpy(buffer, (unsigned char *)uhdr + sizeof(struct udp_hdr), __be16_to_cpu(uhdr->len)-sizeof(struct udp_hdr));
					recvaddr->info = info;
					memcpy(recvaddr->mac, ehdr->src, 6);
					recvaddr->ip = ihdr->saddr;
					recvaddr->id = ihdr->id;
					recvaddr->port = uhdr->src_port;
					//printf("len:%d\n",__be16_to_cpu(uhdr->len));
					return __be16_to_cpu(uhdr->len)-sizeof(struct udp_hdr);
					
					
				}
			}
			else if(ihdr->protocol == 0x06)
			{/* tcp */
		
		
			}
			break;
			
		default:
			break;
	}

	return 0;
}
struct port_info port[20];



int send_packet(unsigned char *ntp_data, unsigned int len, struct recvaddr_t *recvaddr)
{
	unsigned char buf[1536];
	struct ether_hdr *ehdr = (struct ether_hdr *)buf;
	struct ip_hdr *ihdr;
	struct udp_hdr *uhdr;
	struct pseudo_hdr pseudo;
	unsigned char *data;
	int pkt_len;
	unsigned long sum;
	
	ihdr = (struct ip_hdr *)(buf + sizeof(struct ether_hdr));
	uhdr = (struct udp_hdr *)(buf + sizeof(struct ether_hdr) + sizeof(struct ip_hdr));
	data = buf + sizeof(struct ether_hdr) + sizeof(struct ip_hdr) + sizeof(struct udp_hdr);
	
	
	
	memcpy(ehdr->dst, recvaddr->mac, 6);
	memcpy(ehdr->src, recvaddr->info->mac, 6);
	ehdr->type = __cpu_to_be16(0x0800);
	
	ihdr->version = 4;
	ihdr->iphdr_len = sizeof(struct ip_hdr) / 4;
	ihdr->tos = 0;
	ihdr->total_len = __cpu_to_be16(len + sizeof(struct udp_hdr) + sizeof(struct ip_hdr));
	ihdr->id = recvaddr->id;
	ihdr->frag_off = 0;
	ihdr->ttl = 64;
	ihdr->protocol = 0x11;
	ihdr->checksum = 0;
	
	memcpy(&ihdr->saddr, recvaddr->info->ip_a, 4);
	ihdr->daddr = recvaddr->ip;
	ihdr->checksum = checksum(ihdr, sizeof(struct ip_hdr));
	
	uhdr->src_port = __cpu_to_be16(123);
	uhdr->dst_port = recvaddr->port;
	uhdr->len = __cpu_to_be16(len+sizeof(struct udp_hdr));
	uhdr->checksum = 0;
	pseudo.src_ip = ihdr->saddr;
	pseudo.dst_ip = ihdr->daddr;
	pseudo.zero = 0;
	pseudo.protocol = 0x11;
	pseudo.len = __cpu_to_be16(len + sizeof(struct udp_hdr));
	uhdr->checksum = 0;
	
	memcpy(data, ntp_data, len);
	sum = checksum_sum(&pseudo, sizeof(pseudo), 0);
	sum = checksum_sum(uhdr, len + sizeof(struct udp_hdr), sum);
	uhdr->checksum = checksum_check(sum);
	
	pkt_len = len + sizeof(struct ether_hdr) + sizeof(struct ip_hdr) + sizeof(struct udp_hdr);
	return send(recvaddr->info->sock, buf, (pkt_len < 60)?60:pkt_len, 0);

}



