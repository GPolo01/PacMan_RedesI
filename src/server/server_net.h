#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../common/protocol.h"

// Maximum wait thime for a server response
#define TIMEOUT_MS 1000

// Sends large payloads into 31-byte chunks and terminates with MSG_END
int server_send_stream(int sockfd, unsigned char *seq_num, MsgType type, 
    unsigned char *data, int total_len);

#endif