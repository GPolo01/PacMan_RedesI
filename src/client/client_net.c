#include "client_net.h"
#include "socket.h"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

// Envia a mensagem de movimento e aguarda a resposta do servidor.
// Se der timeout ou NACK, retransmite a mensagem original.
// Retorna a mensagem válida recebida do servidor.
int send_and_wait(int sockfd, unsigned char *seq_num, MsgType mov_type, 
    unsigned char *out_data, unsigned char *out_len, MsgType *out_type) {
    
    unsigned char sending_frame[64];
    unsigned char buffer_rec[256];
    unsigned char rec_seq, rec_len;
    int frame_size = pack_frame(*seq_num, mov_type, NULL, 0, sending_frame);
    MsgType rec_type;

    struct timeval timeout = { .tv_sec = TIMEOUT_MS / 1000, .tv_sec = (TIMEOUT_MS%1000) * 1000};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    while(1) {
        send(sockfd, sending_frame, frame_size, 0);

        // It goes to 18 quintillion
        unsigned long begin = get_timestamp_ms();

        do {
            int read_bytes = recv(sockfd, buffer_rec, sizeof(buffer_rec), 0);

            if (read_bytes > 0) {
                if (unpack_frame(buffer_rec, read_bytes, &rec_seq, &rec_type, out_data, &rec_len) == 0) {
                    if (rec_seq == *seq_num) {
                        // Retransmission
                        if (rec_type == MSG_NACK) {
                            printf("NACK received. Begin retransmission");
                            break;
                        }

                        *out_len = rec_len;
                        *out_data = rec_type;
                        *seq_num = (*seq_num + 1) % 64;
                        return 1;
                    }
                }
            }
        } while (get_timestamp_ms() - begin <= TIMEOUT_MS);

        printf("TIMEOUT! Begin retrasmission.\n");
    }
}

// Função para lidar com a recepção em blocos de um arquivo (.txt, .jpg, .mp4)
// Recebe a mensagem inicial que avisou sobre a pastilha e gerencia a criação do arquivo numa pasta
int receive_file(int sockfd, unsigned char *seq_num, MsgType file_type, 
    const unsigned char *initial_data, unsigned char initial_len) {
    
    char filepath[128];
    
    // dots é como se chama as pastilhas do jogo em ingles
    //Vamos fazer tudo em Inglês mano? se não tem que mudar
    if (file_type == MSG_TXT) strcpy(filepath, "dots/file.txt");
    else if (file_type == MSG_JPG) strcpy(filepath, "dots/file.jpg");
    else if (file_type == MSG_MP4) strcpy(filepath, "dots/file.mp4");
    else return 0;

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        printf("Error, not possible to create file %s\n", filepath);
        return 0;
    }
    printf("\n Iniciate dowload of file %s\n", filepath);

    if (initial_len > 0) fwrite(initial_data, 1, initial_len, file);

    // Aqui entramos em um loop recebendo MSG_DADOS e respondendo com MSG_ACK
    // Vamos fazer direto janela deslizante?
    
    fclose(file);
    return 1;
}
