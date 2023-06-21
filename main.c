#include "main.h"

#define PORT        8090
#define LEN         100
#define IPHLEN      sizeof(struct iphdr)
#define UDPHLEN     sizeof(struct udphdr)
#define SUMHLEN     IPHLEN + UDPHLEN + ETH_HLEN
#define BUFLEN      SUMHLEN + LEN

int main(int argc, char* argv[]){

    int client_fd;
    struct iphdr *ip;
    struct udphdr *udp;
    struct ethhdr *eth;
    struct sockaddr_ll ll;    

    char buffer[BUFLEN] = {0};
    eth = (struct ethhdr*)buffer;
    ip = (struct iphdr*)(buffer + ETH_HLEN);
    udp = (struct udphdr*)(buffer + IPHLEN + ETH_HLEN);

    //fill eth struct
    eth->h_source[0] = 0x18;
    eth->h_source[1] = 0xC0;
    eth->h_source[2] = 0x4D;
    eth->h_source[3] = 0x61;
    eth->h_source[4] = 0xEB;
    eth->h_source[5] = 0x39;

    eth->h_dest[0] = 0xB4; //B4-8C-9D-CA-BD-6C // D8-CB-8A-9B-BA-16
    eth->h_dest[1] = 0x8C;
    eth->h_dest[2] = 0x9D;
    eth->h_dest[3] = 0xCA;
    eth->h_dest[4] = 0xBD;
    eth->h_dest[5] = 0x6C;

    eth->h_proto = htons(ETH_P_IP);

    //fill ll struct for sendto
    ll.sll_family = AF_PACKET;
    ll.sll_halen = 6;
    if(!(ll.sll_ifindex = if_nametoindex("enp4s0"))){
        perror("netname");
        exit(EXIT_FAILURE);
    }

    //fill ip struct
    ip->check = 0;
    ip->ihl = 5;
    ip->version = 4;
    ip->protocol = IPPROTO_UDP;
    ip->ttl = 128;
    ip->tot_len = htons(BUFLEN - ETH_HLEN);
    ip->saddr = inet_addr("192.168.0.136");
    ip->daddr = inet_addr("192.168.0.134");

    int sum = 0;
    short* ptr = (short*)ip;
	for(int i = 0; i < 10;i ++){
        sum += htons(*(ptr+i));
    }
    sum = ((sum >> 16) + (sum & 0xFFFF));
    ip->check = htons(~sum & 0x0000FFFF);

    //fill udp struct
    udp->source = htons(8090);
    udp->dest = htons(8090);
    udp->len = htons(BUFLEN - IPHLEN - ETH_HLEN);
    udp->check = 0;    
  
    if((client_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    while(1){
        printf(":> ");
        for(int i = SUMHLEN; i < BUFLEN; i++){
            buffer[i] = 0;
        }
        scanf("%s", buffer + SUMHLEN) ;
        if(sendto(client_fd, buffer, BUFLEN, 0, (struct sockaddr*)&ll, sizeof(struct sockaddr_ll)) < 0){
            perror("send");
            exit(EXIT_FAILURE);
        }
    }

    return 0;

}