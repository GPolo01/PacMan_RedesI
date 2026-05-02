#include "socket.h"
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
 
int create_raw_socket(char* network_interface) {
    // Create socket for raw packets bypassing the OS network stack
 
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock == -1) {
        fprintf(stderr, "Error creating socket: Ensure you are running as root!\n");
        exit(-1);
    }
 
    int ifindex = if_nametoindex(network_interface);
    if (ifindex == 0) {
        fprintf(stderr, "Error: Network interface '%s' not found.\n", network_interface);
    }
 
    struct sockaddr_ll address = {0};
    address.sll_family = AF_PACKET;
    address.sll_protocol = htons(ETH_P_ALL);
    address.sll_ifindex = ifindex;

    // Bind the socket to the specific network interface
    if (bind(sock, (struct sockaddr*) &address, sizeof(address)) == -1) {
        fprintf(stderr, "Error binding socket to interface.\n");
        exit(-1);
    }
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    // Enable Promiscuous Mode: Allows the socket to intercept all traffic on the interface
    if (setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Error setting setsockopt: Promiscuous mode failed.\n");
        exit(-1);
    }
 
    return sock;
}

long long get_timestamp_ms(void) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (long long)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}