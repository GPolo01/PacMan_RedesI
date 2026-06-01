#include "client_net.h"
#include "../common/socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type, 
    unsigned char *out_data, unsigned char *out_len, MsgType *out_type) {
    
    unsigned char sending_frame[64], buffer_rec[256];
    unsigned char rec_seq, rec_len;
    MsgType rec_type;

    int frame_size = pack_frame(*seq_num, mov_type, NULL, 0, sending_frame);

    struct timeval timeout = { .tv_sec = TIMEOUT_MS / 1000, .tv_usec = (TIMEOUT_MS % 1000) * 1000};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    while(1) {
        send(sockfd, sending_frame, frame_size, 0);
        unsigned long begin = get_timestamp_ms();

        do {
            int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);

            if (read_bytes > 0) {
                if (unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, out_data, &rec_len) == 0) {
                    // Don't proccess the msg u have sended (same type)
                    if (rec_seq == *seq_num && rec_type != mov_type) {
                        if (rec_type == MSG_NACK) {
                            printf("NACK received. Beginning retransmission...\n");
                            break; // Break the DO-WHILE, triggers the outer WHILE to re-send
                        }
                        
                        // Sending ACK to server
                        unsigned char ack_frame[64];
                        int ack_size = pack_frame(rec_seq, MSG_ACK, NULL, 0, ack_frame);
                        send(sockfd, ack_frame, ack_size, 0);
                        
                        *out_len = rec_len;
                        *out_type = rec_type;
                        *seq_num = (*seq_num + 1) % 64; // Increment sequence upon success
                        return 1;
                    }
                }
            }
        } while (get_timestamp_ms() - begin <= TIMEOUT_MS);

        printf("TIMEOUT! Retransmitting packet...\n");
    }
}

int receive_file(int sockfd, unsigned char *seq_num, MsgType file_type, 
                 const unsigned char *initial_data, unsigned char initial_len) {
    char filepath[128];
    char action = (initial_len > 0) ? initial_data[0] : '0';

    switch (file_type) {
        case MSG_TXT: sprintf(filepath, "../dots/%c.txt", action); break;
        case MSG_JPG: sprintf(filepath, "../dots/%c.jpg", action); break;
        case MSG_MP4: sprintf(filepath, "../dots/%c.mp4", action); break;
        default: return 0; break;
    }

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        printf("Error: Cannot create file %s\n", filepath);
        return 0;
    }

    printf("\nIniciating dowload of file %s\n", filepath);

    unsigned char rec_seq, rec_len, ack_frame[64], buffer_rec[256], data_rec[MAX_DATA_LEN];
    MsgType rec_type;

    // Aqui entramos em um loop recebendo MSG_DADOS e respondendo com MSG_ACK
    while (1) {
        int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);
        if (read_bytes > 0 && unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, data_rec, &rec_len) == 0) {
            // testar +1 ou sem +1
            if (rec_seq == *seq_num) {
                if (rec_type == MSG_DATA) {
                    fwrite(data_rec, 1, rec_len, file);

                    int ack_size = pack_frame(*seq_num, MSG_ACK, NULL, 0, ack_frame);
                    send(sockfd, ack_frame, ack_size, 0);
                    *seq_num = (*seq_num + 1) % 64;
                } else if (rec_type == MSG_END) {
                    printf("Download concluded!\n");
                    int ack_size = pack_frame(*seq_num, MSG_ACK, NULL, 0, ack_frame);
                    send(sockfd, ack_frame, ack_size, 0);
                    *seq_num = (*seq_num + 1) % 64;
                    break;
                }
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

    unsigned char rec_seq, rec_len, ack_frame[64], buffer_rec[256], data_rec[MAX_DATA_LEN];
    MsgType rec_type;

    while (1) {
        int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);
        if (read_bytes > 0 && unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, data_rec, &rec_len) == 0) {
            if (rec_seq == *seq_num) {
                if (rec_type == MSG_VISION) {
                    memcpy(full_vision + *full_len, data_rec, rec_len);
                    *full_len += rec_len;
                    
                    int ack_size = pack_frame(*seq_num, MSG_ACK, NULL, 0, ack_frame);
                    send(sockfd, ack_frame, ack_size, 0);
                    *seq_num = (*seq_num + 1) % 64;
                } else if (rec_type == MSG_END) {
                    int ack_size = pack_frame(*seq_num, MSG_ACK, NULL, 0, ack_frame);
                    send(sockfd, ack_frame, ack_size, 0);
                    *seq_num = (*seq_num + 1) % 64;
                    break;
                }
            }
        }
    }
    return 1;
}
 