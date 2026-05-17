#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../common/protocol.h"

// Maximum wait thime for a server response
#define TIMEOUT_MS 1000

// Logs the sent and received messages
void log_message(const char *direction, unsigned char seq, MsgType type, int len);

// Sends large payloads into 31-byte chunks and terminates with MSG_END
int server_send_stream(int sockfd, unsigned char *seq_num, MsgType type, 
    unsigned char *data, int total_len);

#endif