//Sophia Schuur
//455 Intro to Networking
//9/23/2019
//Can send and receive a message between two virtual hosts in Mininet.

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define BUF_SIZ		65536
#define SEND 0
#define RECV 1

void send_message(char *if_name, char *hw_addr, char *message){

	int sk_fd, sk_addr_size, send_len, numbytes;
	char buf[BUF_SIZ];
	struct sockaddr_ll sk_addr;
	struct ifreq if_idx, ifr_buf;
	struct ether_header eh;

	sk_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sk_fd < 0) {
		printf("  [!] ERROR - Socket creation failed. Exiting \n");
		exit(0);
	}

	//ioctl - general function that does a lot of different things
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ - 1); 
	if(ioctl(sk_fd, SIOCGIFINDEX, &if_idx) < 0) {
		printf("  [!] ERROR - ioctl failed. Exiting\n");
		exit(0);
	}

	memset(&ifr_buf, 0, sizeof(struct ifreq));
	strncpy(ifr_buf.ifr_name, if_name, IFNAMSIZ - 1); 
	if(ioctl(sk_fd, SIOCGIFHWADDR, &ifr_buf) < 0) {
		printf("  [!] ERROR - ioctl failed. Exiting\n");
		exit(0);
	}

	//header data
	int i = 0;
	while(i < 6)
	{
		eh.ether_shost[i] = (uint8_t)ifr_buf.ifr_hwaddr.sa_data[i];
		i++;
	}

	i = 0;
	while(i < 6)
	{
		eh.ether_dhost[i] = (uint8_t)hw_addr[i];
		i++;
	}

	eh.ether_type = htons(ETH_P_ALL);

	//length of buf should be the header plus size of message
	send_len = sizeof(struct ether_header) + strlen(message);

	//copy header to buf
	memcpy(buf, &eh, sizeof(struct ether_header));

	//copy message to buf
	strcpy(buf + sizeof(struct ether_header), message);

	//set last index of buf to empty
	buf[send_len] = 0;

	//set size of socket address
	sk_addr_size = sizeof(struct sockaddr_ll);

	memset(&sk_addr, 0, sk_addr_size); 
	sk_addr.sll_ifindex = if_idx.ifr_ifindex;
	sk_addr.sll_halen = ETH_ALEN;
	numbytes = sendto(sk_fd, buf, send_len, 0, (struct sockaddr*)&sk_addr, sk_addr_size); 
	if(numbytes < 0) {
		printf("  [!] ERROR - cannot send\n");
	}

	printf("  [SUCCESS] - - - Number of bytes sent: %d\n", numbytes);
	printf("  [NOTE] - Use 'h2 bg' to verify message received.\n\n");
}

void recv_message(char *if_name){

	int sk_fd, numbytes, sk_addr_size;
	char buf[BUF_SIZ];
	struct sockaddr_ll sk_addr;
	struct ifreq if_idx, ifr_buf;
	
	sk_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(sk_fd < 0) {
		printf("  [!] ERROR - Socket creation failed. Exiting \n");
		exit(0);
	}

	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ - 1);
	if(ioctl(sk_fd, SIOCGIFHWADDR, &if_idx) < 0) {
		printf("  [!] ERROR - ioctl failed. Exiting\n");
		exit(0);
	}

	printf("  [SUCCESS] - Received MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		(unsigned char) if_idx.ifr_hwaddr.sa_data[0],
		(unsigned char) if_idx.ifr_hwaddr.sa_data[1],
		(unsigned char) if_idx.ifr_hwaddr.sa_data[2],
		(unsigned char) if_idx.ifr_hwaddr.sa_data[3],
		(unsigned char) if_idx.ifr_hwaddr.sa_data[4],
		(unsigned char) if_idx.ifr_hwaddr.sa_data[5]);

	memset(&sk_addr, 0, sizeof(struct sockaddr_ll));
	sk_addr_size = sizeof(struct sockaddr_ll);
	numbytes = recvfrom(sk_fd, buf, BUF_SIZ, 0, (struct sockaddr *)&sk_addr, (socklen_t *)&sk_addr_size);

	printf("  [SUCCESS] - - - Number of bytes received: %d\n", numbytes);
	printf("  Message received: %s\n\n", buf + sizeof(struct ether_header));
}

int main(int argc, char *argv[])
{
	int mode;
	char hw_addr[6];
	char interfaceName[IFNAMSIZ];
	char buf[BUF_SIZ];
	memset(buf, 0, BUF_SIZ);

	int correct = 0;
	if (argc > 1){
		if(strncmp(argv[1],"Send", 4) == 0){
			if (argc == 5){
				mode = SEND; 
				sscanf(argv[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hw_addr[0], &hw_addr[1], &hw_addr[2], &hw_addr[3], &hw_addr[4], &hw_addr[5]);
				strncpy(buf, argv[4], BUF_SIZ);
				correct = 1;
				printf("  buf: %s\n", buf);
			}
		}
		else if(strncmp(argv[1],"Recv", 4) == 0){
			if (argc == 3){
				mode = RECV;
				correct = 1;
			}
		}
		strncpy(interfaceName, argv[2], IFNAMSIZ);
	}
	if(!correct){
		fprintf(stderr, "./455_proj2 Send <InterfaceName>  <DestHWAddr> <Message>\n");
		fprintf(stderr, "./455_proj2 Recv <InterfaceName>\n");
		exit(1);
	}

	//did not need to do anything else here? 

	if(mode == SEND){
		send_message(interfaceName, hw_addr, buf);
	}
	else if (mode == RECV){
		recv_message(interfaceName);
	}
	return 0;
}


