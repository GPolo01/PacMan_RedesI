#include "protocol.h"
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

// tabela para otimização do crc
static const unsigned char crc8_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

unsigned char crc(unsigned char len, unsigned char seq, 
    unsigned char type, const unsigned char *data) {
    
    unsigned char crc = 0x00;

    // Calculate CRC over the header bytes first
    crc = crc8_table[crc ^ (unsigned char)(((len & 0x1F) << 3) | ((seq >> 3) & 0x07))];
    crc = crc8_table[crc ^ (unsigned char)(((seq & 0x07) << 5) | (type & 0x1F))];

    // Calculate CRC over the payload
    for (int i = 0; i < len; i++) crc = crc8_table[crc ^ data[i]];

    return crc;
}


int pack_frame(unsigned char seq, MsgType type, const unsigned char *data, 
    unsigned char len, unsigned char *buf) {
    
    // Validation to prevent buffer overflow or malformed packets   
    if (len > MAX_DATA_LEN || type > 16 || seq > MAX_SEQ) return 0;

    unsigned char temp_buf[64];

    // Byte 0: Frame Marker
    temp_buf[0] = FRAME_MARKER;

    // Byte 1: 5 bits for Length, 3 for Sequence
    temp_buf[1] = (unsigned char)(((len & 0x1F) << 3) | ((seq >> 3) & 0x07));
    
    // Byte 2: 3 bits for Sequence, 5 for Type
    temp_buf[2] = (unsigned char)(((seq & 0x07) << 5) | (type & 0x1F));

    // Payload
    if (len > 0 && data != NULL) memcpy(temp_buf + 3, data, len);

    // Byte N: CRC
    temp_buf[3 + len] = crc(len, seq, (unsigned char)type, data);
    
    int total_frame_size = 4 + len; // Total frame size

    int i = 0,j = 0;
    for (; i < total_frame_size; i++, j++) {
        buf[j] = temp_buf[i];

        if (temp_buf[i] == 0x88 || temp_buf[i] == 0x81) {
            buf[j + 1] = 0xff;
            j++;
        }
    }
    total_frame_size = j;
    
    

    int MIN_FRAME_SIZE = 14; 
    if (total_frame_size < MIN_FRAME_SIZE) {
        // Fill with zeros to 14 bytes
        memset(buf + total_frame_size, 0, MIN_FRAME_SIZE - total_frame_size);
        total_frame_size = MIN_FRAME_SIZE; 
    }
    

    return total_frame_size;
}


int unpack_frame(const unsigned char *buf, int length, unsigned char *out_seq,
    MsgType *out_type, unsigned char *out_data, unsigned char *out_len) {
    // At least 4 bytes (Marker + Header + Size + CRC)
    if (length < 4 || buf[0] != FRAME_MARKER) return NOT_MESSAGE;
    
    unsigned char temp_buf[64];

    int i = 0,j = 0;
    for (; i < length; i++, j++) {
        temp_buf[j] = buf[i];

        if (buf[i] == 0x81 || buf[i] == 0x88) {
            i++;
        }
    }
    length = j;
    
    unsigned char b1 = temp_buf[1];
    unsigned char b2 = temp_buf[2];

    // Extracting bits
    unsigned char len = (b1 >> 3) & 0x1F;
    unsigned char seq = ((b1 & 0x07) << 3) | ((b2 >> 5) & 0x07);
    unsigned char type = b2 & 0x1F;

    // Ensure the buffer contains the full length
    if (length < 4 + len) return NOT_LENGTH;

    unsigned char received_crc = temp_buf[3 + len];
    unsigned char calculated_crc = crc(len, seq, type, temp_buf + 3);

    if (calculated_crc != received_crc) return NOT_CRC; // Data corrupted

    *out_len = len;
    *out_seq = seq;
    *out_type = (MsgType)type;
    if (len > 0 && out_data != NULL) memcpy(out_data, temp_buf + 3, len);

    return 0; // Success
}

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

void log_message(const char *direction, unsigned char seq, MsgType type, int len, const char *log_prefix) {
    char filename[64];
    sprintf(filename, "%s.log", log_prefix);
    FILE *log_file = fopen(filename, "a");
    if (log_file) {
        fprintf(log_file, "[%lds] [%s] SEQ: %u | TYPE: %s | LEN: %d\n", (long)time(NULL), direction, seq, get_msg_type_name(type), len);
        fclose(log_file);
    }
}

int corrupted_send(int sockfd, unsigned char *buf, int len, int flags, const char *log_prefix) {
    unsigned char temp_buf[256];
    if (len > 256) len = 256;
    memcpy(temp_buf, buf, len);

    int roll = rand() % 100;
    char filename[64];
    sprintf(filename, "%s.log", log_prefix);
    FILE *log_file = fopen(filename, "a");

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
            temp_buf[len - 1] ^= 0xFF;
        }
        return send(sockfd, temp_buf, len, flags);
    }
    if (log_file) fclose(log_file);
    return send(sockfd, buf, len, flags);
}

void send_once(int sockfd, unsigned char seq, MsgType type, const unsigned char *data, unsigned char len, const char *log_prefix) {
    unsigned char frame[64];
    int size = pack_frame(seq, type, data, len, frame);
    log_message("SEND", seq, type, len, log_prefix);
    corrupted_send(sockfd, frame, size, 0, log_prefix);
}

int recv_frame_with_timeout(int sockfd, unsigned char *out_seq, MsgType *out_type, unsigned char *out_data, unsigned char *out_len, int timeout_ms, const char *log_prefix) {
    long long begin = get_timestamp_ms();
    unsigned char buffer_rec[256];
    
    struct timeval tv = { .tv_sec = timeout_ms / 1000, .tv_usec = (timeout_ms % 1000) * 1000 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    
    while (get_timestamp_ms() - begin < timeout_ms) {
        int remain = timeout_ms - (int)(get_timestamp_ms() - begin);
        if (remain <= 0) break;
        
        tv.tv_sec = remain / 1000;
        tv.tv_usec = (remain % 1000) * 1000;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

        int bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);
        if (bytes > 0) {
            int status = unpack_frame(buffer_rec, bytes, out_seq, out_type, out_data, out_len);
            if (status == 0) {
                log_message("RECV", *out_seq, *out_type, *out_len, log_prefix);
                return 0; // Sucesso
            } else if (status == NOT_CRC || status == NOT_LENGTH) {
                unsigned char bad_seq = ((buffer_rec[1] & 0x07) << 3) | ((buffer_rec[2] >> 5) & 0x07);
                send_once(sockfd, bad_seq, MSG_NACK, NULL, 0, log_prefix);
                return -2; // Corrompido
            }
        }
    }
    return -1; // Timeout
}
