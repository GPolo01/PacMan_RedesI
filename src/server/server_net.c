#include "server_net.h"
#include "../common/socket.h"
#include "../common/protocol.h"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

int server_send(int sockfd, unsigned char *seq_num, MsgType type, 
                unsigned char *data, unsigned char len) {
    
    MsgType rec_type;
    unsigned char rec_seq, rec_len, sending_frame[64], buffer_rec[256];
    
    int frame_size = pack_frame(*seq_num, type, data, len, sending_frame);
    
    struct timeval timeout = { .tv_sec = TIMEOUT_MS / 1000, .tv_usec = (TIMEOUT_MS%1000) * 1000};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    while(1) {
        send(sockfd, sending_frame, frame_size, 0);

        // It goes to 18 quintillion
        unsigned long begin = get_timestamp_ms();

        do {
            int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);

            if (read_bytes > 0) {
                if (unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, NULL, &rec_len) == 0) {
                    if (rec_seq == *seq_num) {
                        // Retransmission
                        if (rec_type == MSG_NACK) {
                            printf("NACK received. Begin retransmission");
                            break;
                        }

                        if (rec_type == MSG_ACK) {
                            *seq_num = (*seq_num + 1) % 64;
                            return 1;
                        }
                    }
                }
            }
        } while (get_timestamp_ms() - begin <= TIMEOUT_MS);

        printf("TIMEOUT! Begin retrasmission from Server.\n");
    }
}

int server_send_stream(int sockfd, unsigned char *seq_num, MsgType type, 
                       unsigned char *data, int total_len) {
    int bytes_sent = 0;

    while (bytes_sent < total_len) {
        int chunk_size = total_len - bytes_sent;
        if (chunk_size > MAX_DATA_LEN) chunk_size = MAX_DATA_LEN;

        int success = server_send(sockfd, seq_num, type, data + bytes_sent, chunk_size);
        if (!success) return 0;

        bytes_sent += chunk_size;
    }

    return server_send(sockfd, seq_num, MSG_END, NULL, 0);
}