#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/* Message Format (bits):
 * Start Marker:  8
 * Length:        5
 * Sequence:      6 
 * Type:          5
 * Data:          n bytes
 * CRC:           8
 */

// 01111110 in binary
#define FRAME_MARKER 0x7E 

// 5 bits for length (max 31) and 6 bits for sequence (max 63)
#define MAX_DATA_LEN 31
#define MAX_SEQ 63

#define NOT_MESSAGE -1
#define NOT_LENGTH  -2
#define NOT_CRC     -3

#define TIMEOUT_MS   1000 // Reduzido de 5s para 1s para melhor responsividade
#define MAX_RETRIES  10

/* Unused values: 8, 9, 14 */
typedef enum {
    MSG_ACK = 0,
    MSG_NACK = 1,
    MSG_VISION = 2,
    MSG_INIT = 3,
    MSG_DATA = 4,
    MSG_TXT = 5,
    MSG_JPG = 6,
    MSG_MP4 = 7,
    MSG_MOV_RIGHT = 10,
    MSG_MOV_LEFT = 11,
    MSG_MOV_UP = 12,
    MSG_MOV_DOWN = 13,
    MSG_ERROR = 15,
    MSG_END = 16
} MsgType;

typedef enum {
    ERR_SPACE = 1,
    ERR_WRITE = 2
} ErrorCode;

// Retorna o nome amigável do tipo de mensagem para os logs
const char* get_msg_type_name(MsgType type);

// Calculates the CRC (Polynomial Division)
unsigned char crc(unsigned char len, unsigned char seq, unsigned char type, const unsigned char *data);

// Assembles the frame for sending
int pack_frame(unsigned char seq, MsgType type, const unsigned char *data, unsigned char len, unsigned char *buf);

// Disassembles and validates a received frame
int unpack_frame(const unsigned char *buf, int length, unsigned char *out_seq, MsgType *out_type, unsigned char *out_data, unsigned char *out_len);

// --- UTILITÁRIOS COMPARTILHADOS ---
void log_message(const char *direction, unsigned char seq, MsgType type, int len, const char *log_prefix);
int corrupted_send(int sockfd, unsigned char *buf, int len, int flags, const char *log_prefix);
void send_once(int sockfd, unsigned char seq, MsgType type, const unsigned char *data, unsigned char len, const char *log_prefix);

// --- RECEPTOR CENTRALIZADO DE TIMEOUT E FILTRAGEM ---
int recv_frame_with_timeout(int sockfd, unsigned char *out_seq, MsgType *out_type, unsigned char *out_data, unsigned char *out_len, int timeout_ms, const char *log_prefix);

#endif