#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "../common/protocol.h"

int server_send(int sockfd, unsigned char *seq_num, MsgType type, 
                unsigned char *data, unsigned char len, unsigned char expected_rx);

// Sends large payloads into 31-byte chunks and terminates with MSG_END
int server_send_vision(int sockfd, unsigned char *seq_num, MsgType type, 
    unsigned char *data, int total_len, unsigned char expected_rx, char action_after_vision);

int server_send_file(int sockfd, unsigned char *seq_num, MsgType file_type, const char *filepath, int action, unsigned char expected_rx);

#endif