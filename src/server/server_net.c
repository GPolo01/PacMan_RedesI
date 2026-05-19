#include "server_net.h"
#include "../common/socket.h"
#include "../common/protocol.h"
#include <sys/socket.h>
#include <stdio.h>
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
    FILE *log_file = fopen("server.log", "a");
    if (log_file) {
        fprintf(log_file, "[%lds] [%s] SEQ: %u | TYPE: %s | LEN: %d\n", (long)time(NULL), direction, seq, get_msg_type_name(type), len);
        fclose(log_file);
    }
}

int server_send(int sockfd, unsigned char *seq_num, MsgType type, 
                unsigned char *data, unsigned char len) {
    
    MsgType rec_type;
    unsigned char rec_seq, rec_len, sending_frame[64], buffer_rec[256];
    
    int frame_size = pack_frame(*seq_num, type, data, len, sending_frame);
    
    struct timeval timeout = { .tv_sec = TIMEOUT_MS / 1000, .tv_usec = (TIMEOUT_MS%1000) * 1000};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    while(1) {
        send(sockfd, sending_frame, frame_size, 0);
        log_message("SEND", *seq_num, type, len);

        // It goes to 18 quintillion
        unsigned long begin = get_timestamp_ms();

        do {
            int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);

            if (read_bytes > 0) {
                if (unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, NULL, &rec_len) == 0) {
                    log_message("RECV", rec_seq, rec_type, rec_len);
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

int server_send_vision(int sockfd, unsigned char *seq_num, MsgType type, 
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

int server_send_file(int sockfd, unsigned char *seq_num, MsgType type, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        printf("ERROR: Could not open file %s for sending.\n", filepath);
        return 0;
    }

    unsigned char buffer[MAX_DATA_LEN];
    int bytes_read;
    int is_first_chunk = 1;

    while ((bytes_read = fread(buffer, 1, MAX_DATA_LEN, file)) > 0) {
        
        MsgType real_type = is_first_chunk ? type : MSG_DATA;
        server_send(sockfd, seq_num, real_type, buffer, bytes_read);
        is_first_chunk = 0;
    }

    fclose(file);
    
    return server_send(sockfd, seq_num, MSG_END, NULL, 0);
}