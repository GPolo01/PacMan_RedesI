#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/*Formato da mensagem (por/bits):
    Marcador de Início 8
    Tamanho 5
    Sequência 6 
    Tipo 5
    Dados n bytes
    CRC 8
*/

// o 01111110
#define FRAME_MARKER 0x7E 

// 5 bits tamanho e 6 para sequência
#define MAX_DATA_LEN 31
#define MAX_SEQ 63

/* Sem usos definidos: 8,9,14*/
typedef enum {
    MSG_ACK = 0,
    MSG_NACK = 1,
    MSG_VISUALIZACAO = 2,
    MSG_INICIALIZACAO = 3,
    MSG_DADOS = 4,
    MSG_TXT = 5,
    MSG_JPG = 6,
    MSG_MP4 = 7,
    MSG_MOV_DIR = 10,
    MSG_MOV_ESQ = 11,
    MSG_MOV_CIMA = 12,
    MSG_MOV_BAIXO = 13,
    MSG_ERROS = 15,
    MSG_FIM = 16
} MsgType;

typedef enum {
    ERR_ESPACO = 1,
    ERR_ESCRITA = 2
} ErrorCode;

// Calcula o CRC (divisão polinomial)
unsigned char crc(unsigned char len, unsigned char seq, unsigned char type, const unsigned char *data);

// Monta o frame para envio e desmonta/valida frame
int pack_frame(unsigned char seq, MsgType type, const unsigned char *data, unsigned char len, unsigned char *buf);
int unpack_frame(const unsigned char *buf, int length, unsigned char *out_seq, MsgType *out_type, unsigned char *out_data, unsigned char *out_len);

#endif