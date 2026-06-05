#include "client_net.h"
#include "../common/socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type) {
    int retries = 0;
    int current_timeout = TIMEOUT_MS;

    while (retries < MAX_RETRIES) {
        send_once(sockfd, *seq_num, mov_type, NULL, 0, "client");
        
        long long begin = get_timestamp_ms();
        int ack_received = 0;
        int nack_received = 0;

        while (get_timestamp_ms() - begin < current_timeout) {
            int remain = current_timeout - (int)(get_timestamp_ms() - begin);
            if (remain <= 0) break;

            unsigned char rec_seq, rec_len;
            MsgType rec_type;
            unsigned char data_rec[MAX_DATA_LEN];
            
            int status = recv_frame_with_timeout(sockfd, &rec_seq, &rec_type, data_rec, &rec_len, remain, "client");
            
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
            *seq_num = (*seq_num + 1) % 64; // Confirmação recebida com sucesso!
            return 1; 
        }

        if (nack_received) {
            printf("NACK recebido do Servidor. Retransmitindo...\n");
        }

        retries++;
        current_timeout *= 2;
    }
    printf("TIMEOUT limite atingido no cliente!\n");
    return 0;
}

int receive_file(int sockfd, unsigned char *seq_num, MsgType file_type, const unsigned char *initial_data, unsigned char initial_len) {
    char filepath[128];
    char action = (initial_len > 0) ? initial_data[0] : '0';

    switch (file_type) {
        case MSG_TXT: sprintf(filepath, "../dots/%c.txt", action); break;
        case MSG_JPG: sprintf(filepath, "../dots/%c.jpg", action); break;
        case MSG_MP4: sprintf(filepath, "../dots/%c.mp4", action); break;
        default: return 0;
    }

    FILE *file = fopen(filepath, "wb");
    if (!file) return 0;
    printf("\nBaixando arquivo em %s...\n", filepath);
    
    // Confirma recebimento dos metadados iniciais
    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0, "client");
    *seq_num = (*seq_num + 1) % 64;

    unsigned char rec_seq, rec_len, data_rec[MAX_DATA_LEN];
    MsgType rec_type;

    while (1) {
        int status = recv_frame_with_timeout(sockfd, &rec_seq, &rec_type, data_rec, &rec_len, TIMEOUT_MS, "client");
        if (status == 0) {
            if (rec_seq == *seq_num) {
                if (rec_type == MSG_DATA) {
                    fwrite(data_rec, 1, rec_len, file);
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0, "client");
                    *seq_num = (*seq_num + 1) % 64;
                } else if (rec_type == MSG_END) {
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0, "client");
                    *seq_num = (*seq_num + 1) % 64;
                    break;
                }
            } else if (rec_seq == (*seq_num + 63) % 64) {
                // Duplicata de bloco (ACK anterior se perdeu) -> reenvia o ACK do bloco
                send_once(sockfd, rec_seq, MSG_ACK, NULL, 0, "client");
            }
        }
    }
    fclose(file);
    return 1;
}

int recive_vision(int sockfd, unsigned char *seq_num, const unsigned char *initial_data, 
                  unsigned char initial_len, unsigned char *full_vision, int *full_len) {
    memcpy(full_vision, initial_data, initial_len);
    *full_len = initial_len;

    // Confirma primeira linha da visão
    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0, "client");
    *seq_num = (*seq_num + 1) % 64;

    unsigned char rec_seq, rec_len, data_rec[MAX_DATA_LEN];
    MsgType rec_type;

    while (1) {
        int status = recv_frame_with_timeout(sockfd, &rec_seq, &rec_type, data_rec, &rec_len, TIMEOUT_MS, "client");
        if (status == 0) {
            if (rec_seq == *seq_num) {
                if (rec_type == MSG_VISION) {
                    memcpy(full_vision + *full_len, data_rec, rec_len);
                    *full_len += rec_len;
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0, "client");
                    *seq_num = (*seq_num + 1) % 64;
                } else if (rec_type == MSG_END) {
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0, "client");
                    *seq_num = (*seq_num + 1) % 64;
                    break;
                }
            } else if (rec_seq == (*seq_num + 63) % 64) {
                send_once(sockfd, rec_seq, MSG_ACK, NULL, 0, "client");
            }
        }
    }
    return 1;
}