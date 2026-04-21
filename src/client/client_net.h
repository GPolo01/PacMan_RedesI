#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "protocol.h"

// Tempo máximo de espera por uma resposta do servidor
// Deixei 1 segundo não sei qual o tempo certo
#define TIMEOUT_MS 1000

// Envia a mensagem de movimento e aguarda a resposta do servidor.
// Se der timeout ou receber NACK, retransmite a mensagem original.
// Retorna 1 se deu boa
int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type, unsigned char *out_data, unsigned char *out_len, MsgType *out_type);

// Função para lidar com a recepção em blocos de um arquivo (.txt, .jpg, .mp4)
int receive_file(int sockfd, unsigned char *seq_num, MsgType file_type, const unsigned char *initial_data, unsigned char initial_len);

#endif