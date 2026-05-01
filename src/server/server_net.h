#ifndef SERVER_H
#define SERVER_H

#include "../common/protocol.h"

#define TIMEOUT_MS 1000

int server_send(int sockfd, unsigned char *seq_num, MsgType type, 
    unsigned char *data, unsigned char len);

#endif