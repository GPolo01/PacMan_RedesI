#include "client_net.h"
#include "../common/socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

const char* get_msg_type_name(MsgType type) {
    switch (type) {
        case MSG_INIT: return "MSG_INIT";
        case MSG_ACK: return "MSG_ACK";
        case MSG_NACK: return "MSG_NACK";
        case MSG_VISION: return "MSG_VISION";
        case MSG_MOV_UP: return "MSG_MOV_UP";
        case MSG_MOV_DOWN: return "MSG_MOV_DOWN";
        case MSG_MOV_RIGHT: return "MSG_MOV_RIGHT";
        case MSG_MOV_LEFT: return "MSG_MOV_LEFT";
        case MSG_TXT: return "MSG_TXT";
        case MSG_JPG: return "MSG_JPG";
        case MSG_MP4: return "MSG_MP4";
        case MSG_DATA: return "MSG_DATA";
        case MSG_END: return "MSG_END";
        case MSG_ERROR: return "MSG_ERROR";
        default: return "UNKNOWN";
    }
}

void log_message(const char *direction, unsigned char seq, MsgType type, int len) {
    FILE *log_file = fopen("client.log", "a");
    if (log_file) {
        fprintf(log_file, "[%lds] [%s] SEQ: %u | TYPE: %s | LEN: %d\n", (long)time(NULL), direction, seq, get_msg_type_name(type), len);
        fclose(log_file);
    }
}

int corrupted_send(int sockfd, unsigned char *buf, int len, int flags) {
    unsigned char temp_buf[256];
    if (len > 256) len = 256;
    memcpy(temp_buf, buf, len);

    int roll = rand() % 100;
    FILE *log_file = fopen("client.log", "a");

    // Simulate successful send but do nothing
    if (roll < 10) {
        if (log_file) {
            fprintf(log_file, "[%lds] [CORRUPTED] TYPE DROPPED\n", (long)time(NULL));
            fclose(log_file);
        }
        return len; 
    } else if (roll < 20) {
        if (log_file) {
            fprintf(log_file, "[%lds] [CORRUPTED] TYPE HEADER WITH ERRORS\n", (long)time(NULL));
            fclose(log_file);
        }
        temp_buf[1] ^= 0xFF;
        return send(sockfd, temp_buf, len, flags);    
    } else if (roll < 30) {
        if (log_file) {
            fprintf(log_file, "[%lds] [CORRUPTED] TYPE DATA WITH ERRORS\n", (long)time(NULL));
            fclose(log_file);
        }
        if (len > 4) {
            int random_byte = 3 + (rand() % (len - 4));
            temp_buf[random_byte] ^= 0xFF;
        } else {
            temp_buf[len -1] ^= 0xFF;
        }
        return send(sockfd, temp_buf, len, flags);
    }
    if (log_file) fclose(log_file);
    return send(sockfd, buf, len, flags);
}

void send_once(int sockfd, unsigned char seq, MsgType type, unsigned char *data, unsigned char len) {
    unsigned char frame[64];
    int size = pack_frame(seq, type, data, len, frame);
    log_message("SEND", seq, type, len);
    corrupted_send(sockfd, frame, size, 0);
}

int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type, 
    unsigned char *out_data, unsigned char *out_len, MsgType *out_type) {
    
    unsigned char sending_frame[64], buffer_rec[256];
    unsigned char rec_seq, rec_len;
    MsgType rec_type;
    int read_bytes, unpack_status;
    int frame_size = pack_frame(*seq_num, mov_type, NULL, 0, sending_frame);

    struct timeval timeout = { .tv_sec = TIMEOUT_MS / 1000, .tv_usec = (TIMEOUT_MS % 1000) * 1000};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    while(1) {
        log_message("SEND", *seq_num, mov_type, 0);
        corrupted_send(sockfd, sending_frame, frame_size, 0);
        unsigned long begin = get_timestamp_ms();

        do {
            read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);
            unpack_status = unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, NULL, &rec_len);

            if (read_bytes > 0 && unpack_status == 0) {
                log_message("RECV", rec_seq, rec_type, rec_len);
                // Don't proccess the msg u have sended (same type)
                if (rec_seq == *seq_num && rec_type != mov_type) {
                    if (rec_type == MSG_NACK) {
                        printf("NACK received. Beginning retransmission...\n");
                        break; // Break the DO-WHILE, triggers the outer WHILE to re-send
                    }
                    
                    // Sending ACK to server
                    send_once(sockfd, rec_seq, MSG_ACK, NULL, 0);
                    
                    *out_len = rec_len;
                    *out_type = rec_type;
                    *seq_num = (*seq_num + 1) % 64; // Increment sequence upon success
                    return 1;
                }
            }
            else if (read_bytes > 0 && unpack_status == -1) {
                send_once(sockfd, *seq_num, MSG_NACK, NULL, 0);
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

    unsigned char rec_seq, rec_len, buffer_rec[256], data_rec[MAX_DATA_LEN];
    MsgType rec_type;
    int read_bytes, unpack_status;

    // Aqui entramos em um loop recebendo MSG_DADOS e respondendo com MSG_ACK
    while (1) {
        int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);
        unpack_status = unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, data_rec, &rec_len);

        if (read_bytes > 0 && unpack_status == 0) {
            log_message("RECV", rec_seq, rec_type, rec_len);
            if (rec_seq == *seq_num) {
                if (rec_type == MSG_DATA) {
                    fwrite(data_rec, 1, rec_len, file);
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0);
                    *seq_num = (*seq_num + 1) % 64;
                } else if (rec_type == MSG_END) {
                    printf("Download concluded!\n");
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0);
                    *seq_num = (*seq_num + 1) % 64;
                    break;
                }
            }
        }
        else if (read_bytes > 0 && unpack_status == -1) {
            send_once(sockfd, *seq_num, MSG_NACK, NULL, 0);
        }
    }
    fclose(file);
    return 1;
}

int recive_vision(int sockfd, unsigned char *seq_num, const unsigned char *initial_data, 
                  unsigned char initial_len, unsigned char *full_vision, int *full_len) {
    memcpy(full_vision, initial_data, initial_len);
    *full_len = initial_len;

    unsigned char rec_seq, rec_len, buffer_rec[256], data_rec[MAX_DATA_LEN];
    MsgType rec_type;
    int read_bytes, unpack_status;

    while (1) {
        read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);
        unpack_status = unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, data_rec, &rec_len);
        if (read_bytes > 0 && unpack_status == 0) {
            log_message("RECV", rec_seq, rec_type, rec_len);
            if (rec_seq == *seq_num) {
                if (rec_type == MSG_VISION) {
                    memcpy(full_vision + *full_len, data_rec, rec_len);
                    *full_len += rec_len;
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0);
                    *seq_num = (*seq_num + 1) % 64;
                } else if (rec_type == MSG_END) {
                    send_once(sockfd, *seq_num, MSG_ACK, NULL, 0);
                    *seq_num = (*seq_num + 1) % 64;
                    break;
                }
            }
        }
        else if (read_bytes > 0 && unpack_status == -1) {
            send_once(sockfd, *seq_num, MSG_NACK, NULL, 0);
        }
    }
    return 1;
}
 