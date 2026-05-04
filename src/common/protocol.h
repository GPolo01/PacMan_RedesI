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

// Calculates the CRC (Polynomial Division)
unsigned char crc(unsigned char len, unsigned char seq, unsigned char type, const unsigned char *data);

// Assembles the frame for sending
int pack_frame(unsigned char seq, MsgType type, const unsigned char *data, unsigned char len, unsigned char *buf);

// Disassembles and validates a received frame
int unpack_frame(const unsigned char *buf, int length, unsigned char *out_seq, MsgType *out_type, unsigned char *out_data, unsigned char *out_len);

#endif