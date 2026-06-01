#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../common/protocol.h"

// Maximum wait thime for a server response
#define TIMEOUT_MS 1000

// Logs the sent and received messages
void log_message(const char *direction, unsigned char seq, MsgType type, int len);

int corrupted_send(int sockfd, unsigned char *buf, int len, int flags);

// Sends a movement message and waits for the server's response.
// If a timeout or NACK occurs, it retransmits the original message.
int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type, unsigned char *out_data, unsigned char *out_len, MsgType *out_type);

// Handles receiving blocks of files (.txt, .jpg, .mp4) using a sliding window or Stop-and-Wait
int receive_file(int sockfd, unsigned char *seq_num, MsgType file_type, const unsigned char *initial_data, unsigned char initial_len);

// Handles receiving blocks of the vision using a sliding window or Stop-and-Wait
int recive_vision(int sockfd, unsigned char *seq_num, const unsigned char *initial_data, 
                  unsigned char initial_len, unsigned char *full_vision, int *full_len);

#endif