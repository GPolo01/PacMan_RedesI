#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../common/protocol.h"

// Maximum wait thime for a server response
#define TIMEOUT_MS 1000

// Logs the sent and received messages
void log_message(const char *direction, unsigned char seq, MsgType type, int len);

int corrupted_send(int sockfd, unsigned char *buf, int len, int flags);

int server_send(int sockfd, unsigned char *seq_num, MsgType type, 
                unsigned char *data, unsigned char len);

// Sends large payloads into 31-byte chunks and terminates with MSG_END
int server_send_vision(int sockfd, unsigned char *seq_num, MsgType type, 
    unsigned char *data, int total_len);

int server_send_file(int sockfd, unsigned char *seq_num, MsgType file_type, const char *filepath, int action);

#endif