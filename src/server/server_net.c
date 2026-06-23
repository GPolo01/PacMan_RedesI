#include "server_net.h"
#include "../common/socket.h"
#include "../common/protocol.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int server_send(int sockfd, unsigned char *seq_num, MsgType type, unsigned char *data, unsigned char len, unsigned char expected_rx) {
    while (1) {
        send_frame(sockfd, *seq_num, type, data, len, "server");

        long long begin = get_timestamp_ms();
        int ack_received = 0;
        int nack_received = 0;

        while (get_timestamp_ms() - begin < TIMEOUT_MS) {
            int remain = TIMEOUT_MS - (int)(get_timestamp_ms() - begin);
            if (remain <= 0) break;

            unsigned char rec_seq, rec_len;
            MsgType rec_type;
            
            int status = recv_frame(sockfd, &rec_seq, &rec_type, NULL, &rec_len, remain, "server");
            
            if (status == 0) {
                if (rec_type == MSG_INIT || (rec_type >= MSG_MOV_RIGHT && rec_type <= MSG_MOV_DOWN)) {
                    if (rec_seq == (expected_rx + 63) % 64) {
                        // The client retransmitted the previous command (probably because it lost the previous ACK).
                        // Resend the corresponding ACK to unblock the client.
                        send_frame(sockfd, rec_seq, MSG_ACK, NULL, 0, "server");
                    } else if (rec_seq == expected_rx) {
                        // The client sent a NEW command. This means it successfully received
                        // the packet (implicit ACK).
                        // server returns to the main loop
                        // and handles this new command in the correct flow.
                        return 0;
                    }
                } else if (rec_seq == *seq_num) {
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
            printf("NACK received from Client. Retransmitting...\n");
        }
    }
}

int server_send_vision(int sockfd, unsigned char *seq_num, MsgType type, unsigned char *data, int total_len, unsigned char expected_rx, char action_after_vision) {
    int bytes_sent = 0;
    while (bytes_sent < total_len) {
        int chunk_size = total_len - bytes_sent;
        if (chunk_size > MAX_DATA_LEN) chunk_size = MAX_DATA_LEN;

        if (!server_send(sockfd, seq_num, type, data + bytes_sent, chunk_size, expected_rx)) return 0;
        bytes_sent += chunk_size;
    }
    if (action_after_vision != 0) {
        unsigned char action_byte = (unsigned char)action_after_vision;
        return server_send(sockfd, seq_num, MSG_END, &action_byte, 1, expected_rx);
    }
    return server_send(sockfd, seq_num, MSG_END, NULL, 0, expected_rx);
}

int server_send_file(int sockfd, unsigned char *seq_num, MsgType type, const char *filepath, int action, unsigned char expected_rx) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return 0;

    unsigned char buffer[MAX_DATA_LEN];
    int bytes_read;
    unsigned char action_byte = (unsigned char)action;
    
    if (!server_send(sockfd, seq_num, type, &action_byte, 1, expected_rx)) {
        fclose(file);
        return 0;
    }

    while ((bytes_read = fread(buffer, 1, MAX_DATA_LEN, file)) > 0) {
        if (!server_send(sockfd, seq_num, MSG_DATA, buffer, bytes_read, expected_rx)) {
            fclose(file);
            return 0;
        }
    }
    fclose(file);
    return server_send(sockfd, seq_num, MSG_END, NULL, 0, expected_rx);
}