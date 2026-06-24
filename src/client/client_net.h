#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../common/protocol.h"

// Sends a message and waits for the server's response
// If a timeout or NACK occurs, it retransmits the original message
int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type, unsigned char expected_rx);

// Handles receiving blocks of files (.txt, .jpg, .mp4) using Stop-and-Wait
int receive_file(int sockfd, unsigned char *seq_num, MsgType file_type, const unsigned char *initial_data, unsigned char initial_len);

// Handles receiving blocks of the vision using Stop-and-Wait
int receive_vision(int sockfd, unsigned char *seq_num, const unsigned char *initial_data, 
                  unsigned char initial_len, unsigned char *full_vision, int *full_len, char *out_action_after_vision);

#endif