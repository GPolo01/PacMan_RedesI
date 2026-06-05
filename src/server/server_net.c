#include "server_net.h"
#include "../common/socket.h"
#include "../common/protocol.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int server_send(int sockfd, unsigned char *seq_num, MsgType type, unsigned char *data, unsigned char len) {
    int retries = 0;
    int current_timeout = TIMEOUT_MS;

    while (retries < MAX_RETRIES) {
        send_once(sockfd, *seq_num, type, data, len, "server");

        long long begin = get_timestamp_ms();
        int ack_received = 0;
        int nack_received = 0;

        while (get_timestamp_ms() - begin < current_timeout) {
            int remain = current_timeout - (int)(get_timestamp_ms() - begin);
            if (remain <= 0) break;

            unsigned char rec_seq, rec_len;
            MsgType rec_type;
            
            int status = recv_frame_with_timeout(sockfd, &rec_seq, &rec_type, NULL, &rec_len, remain, "server");
            
            if (status == 0) {
                if (rec_seq == *seq_num) {
                    if (rec_type == MSG_NACK) {
                        nack_received = 1;
                        break;
                    }
                    if (rec_type == MSG_ACK) {
                        ack_received = 1;
                        break;
                    }
                }
            } else if (status == -1) {
                break;
            }
        }

        if (ack_received) {
            *seq_num = (*seq_num + 1) % 64;
            return 1;
        }

        if (nack_received) {
            printf("NACK recebido do Cliente. Retransmitindo...\n");
        }

        retries++;
        current_timeout *= 2;
    }
    return 0;
}

int server_send_vision(int sockfd, unsigned char *seq_num, MsgType type, unsigned char *data, int total_len) {
    int bytes_sent = 0;
    while (bytes_sent < total_len) {
        int chunk_size = total_len - bytes_sent;
        if (chunk_size > MAX_DATA_LEN) chunk_size = MAX_DATA_LEN;

        if (!server_send(sockfd, seq_num, type, data + bytes_sent, chunk_size)) return 0;
        bytes_sent += chunk_size;
    }
    return server_send(sockfd, seq_num, MSG_END, NULL, 0);
}

int server_send_file(int sockfd, unsigned char *seq_num, MsgType type, const char *filepath, int action) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return 0;

    unsigned char buffer[MAX_DATA_LEN];
    int bytes_read;
    unsigned char action_byte = (unsigned char)action;
    
    server_send(sockfd, seq_num, type, &action_byte, 1);

    while ((bytes_read = fread(buffer, 1, MAX_DATA_LEN, file)) > 0) {
        server_send(sockfd, seq_num, MSG_DATA, buffer, bytes_read);
    }
    fclose(file);
    return server_send(sockfd, seq_num, MSG_END, NULL, 0);
}