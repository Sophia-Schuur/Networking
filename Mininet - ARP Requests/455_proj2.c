//Sophia Schuur
//11519303
//455 Networking
//10/14/2019
//simple program that can send ARP requests and process ARP replies.
//Run in mininet enviro.

#include <string.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <netinet/ether.h>

#define BUF_SIZ		65536	
#define SEND 0
#define RECV 1

struct arp_hdr {
	uint16_t ar_hrd; 		 
	uint16_t ar_pro; 		 
	unsigned char ar_hln; 	
	unsigned char ar_pln; 	 
	uint16_t ar_op; 		 
	unsigned char ar_sha[6]; 
	unsigned char ar_sip[4]; 
	unsigned char ar_tha[6]; 
	unsigned char ar_tip[4];
};

int make_eh_hdr(struct ether_header *buf, int sockfd, char *if_name){

	struct ether_header *eh;
	struct ifreq ifr;
	int i = 0;

	eh = buf;

	strncpy(ifr.ifr_name, if_name, IFNAMSIZ - 1);
	if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0){
		printf("  [!] ERROR - ioctl failed. Exiting\n");
		exit(0);
	}
	
	while(i < 6)
	{
		eh->ether_shost[i] = (uint8_t)ifr.ifr_hwaddr.sa_data[i];
		eh->ether_dhost[i] = 0xFF;
		i++;
	}

	eh->ether_type = htons(ETH_P_ARP);

	return 0;
}

int make_arp_hdr(struct arp_hdr *buf, int sockfd, char *if_name, char *dest_addr, uint8_t src_mac[6]){

	struct arp_hdr *ah;
	struct ifreq ifr;
	struct in_addr addr;	//store address
	unsigned int src_ip, dst_ip;

	ah = buf;
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ - 1);

	if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0){
		printf("  [!] ERROR - ioctl failed. Exiting\n");
		exit(0);
	}

	src_ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

	inet_aton(dest_addr, &addr); //convert dest IP
	dst_ip = addr.s_addr;

	ah->ar_hrd = htons(1);
	ah->ar_pro = htons(ETH_P_IP);
	ah->ar_hln = 6;
	ah->ar_pln = 4;
	ah->ar_op = htons(ARPOP_REQUEST);

	memset(&ah->ar_tha, 0, 6 * sizeof(uint8_t));
	memcpy(&ah->ar_sha, src_mac, 6 * sizeof(uint8_t));
	memcpy(&ah->ar_sip, &src_ip, 4 * sizeof(uint8_t));
	memcpy(&ah->ar_tip, &dst_ip, 4 * sizeof(uint8_t));
}

void send_arp(int sockfd, struct sockaddr_ll *sockaddr,  char *ip_addr, char *if_name){

	int nbytes;
	char buf[BUF_SIZ];
	struct arp_hdr ah;
	struct ether_header eh;
	size_t len;
	char *cp;
	cp = buf;

	make_eh_hdr(&eh, sockfd, if_name);
	
	printf("\n- - - - - - - - - - SEND - - - - - - - - - - \n");
	printf(" Source MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n", 
			(uint8_t)eh.ether_shost[0],
			(uint8_t)eh.ether_shost[1],
			(uint8_t)eh.ether_shost[2],
			(uint8_t)eh.ether_shost[3],
			(uint8_t)eh.ether_shost[4],
			(uint8_t)eh.ether_shost[5]);

	printf(" Dest MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n", 
			(uint8_t)eh.ether_dhost[0],
			(uint8_t)eh.ether_dhost[1],
			(uint8_t)eh.ether_dhost[2],
			(uint8_t)eh.ether_dhost[3],
			(uint8_t)eh.ether_dhost[4],
			(uint8_t)eh.ether_dhost[5]);

	printf(" Ether: %04x\n", (uint16_t)ntohs(eh.ether_type));
	printf("* * * * * * * * * * * * * * * * * * * *\n");

	make_arp_hdr(&ah, sockfd, if_name, ip_addr, eh.ether_shost);

	//make packet
	len = sizeof(struct ether_header) + sizeof(struct arp_hdr);
	//printf("\n %ld \n", len);

	memcpy(cp, &eh, sizeof(struct ether_header));
	cp += sizeof(struct ether_header);
	memcpy(cp, &ah, sizeof(struct arp_hdr));

	nbytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)sockaddr, sizeof(struct sockaddr_ll)); 
	if(nbytes < 0) {
		printf("  [!] ERROR - Cannot send. Exiting %s\n", strerror(errno));
		exit(0);
	}
}

void recv_arp(int sockfd, struct sockaddr_ll *sockaddr){

	char buf[BUF_SIZ];
	int nbytes;
	struct ether_header *eh = (struct ether_header *)buf;
	struct arp_hdr *ah = (struct arp_hdr *)buf + sizeof(struct ether_header);
	socklen_t sockaddr_size;

	sockaddr_size = sizeof(struct sockaddr);
	nbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, (struct sockaddr *)sockaddr, &sockaddr_size);

	
	printf("\n- - - - - - - - - - RECV - - - - - - - - - -\n");
	printf(" Source MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n", 
			(uint8_t)eh->ether_shost[0],
			(uint8_t)eh->ether_shost[1],
			(uint8_t)eh->ether_shost[2],
			(uint8_t)eh->ether_shost[3],
			(uint8_t)eh->ether_shost[4],
			(uint8_t)eh->ether_shost[5]);

	printf(" Dest MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n", 
			(uint8_t)eh->ether_dhost[0],
			(uint8_t)eh->ether_dhost[1],
			(uint8_t)eh->ether_dhost[2],
			(uint8_t)eh->ether_dhost[3],
			(uint8_t)eh->ether_dhost[4],
			(uint8_t)eh->ether_dhost[5]);

	printf(" Ether: %04x\n\n", (uint16_t)ntohs(eh->ether_type));
}


int main(int argc, char *argv[])
{
	struct sockaddr_ll sockaddr;
	struct ifreq ifr;
	char ip_addr[15];
	char interface_name[IFNAMSIZ];
	
	strcpy(interface_name, argv[1]);
	strcpy(ip_addr, argv[2]);

	printf("  Interface Name: %s\n", interface_name);
	printf("  IP Address: %s\n", ip_addr);

	int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(sockfd < 0) {
		printf("  [!] ERROR - Socket creation failed. Exiting\n");
		exit(0);
	}
	
	memset(&sockaddr, 0, sizeof(struct sockaddr_ll));

	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
	if(ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
		printf("  [!] ERROR - ioctl failed. Exiting\n");
		exit(0);
	}

	sockaddr.sll_ifindex = ifr.ifr_ifindex;
	sockaddr.sll_halen = ETH_ALEN;

	send_arp(sockfd, &sockaddr, ip_addr, interface_name);
	recv_arp(sockfd, &sockaddr);
	return 0;
}





















