#include "socket.h"
#include "protocol.h"
#include "client_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Uso: sudo ./client <interface_de_rede>\n");
        return -1;
    }

    int sock_client = create_raw_socket(argv[1]);

    unsigned char buffer_rec[256];
    unsigned char seq_num = 0;

    while (1) {
        // Ler movimento, enviar mensagem, receber mensagem
        char key = getchar();
        MsgType moviment_type, type_received;
        unsigned char len_received, data_received[MAX_DATA_LEN];

        //Vamos primeiro ler o teclado, movimento
        switch (key) {
            case 'w': moviment_type = MSG_MOV_CIMA; break;
            case 'a': moviment_type = MSG_MOV_ESQ; break;
            case 's': moviment_type = MSG_MOV_BAIXO; break;
            case 'd': moviment_type = MSG_MOV_DIR; break;
            default: continue;
        }

        
        // Enviar a mensagem
        printf("Sending movement\n");
        int success = send_and_wait(sock_client, &seq_num, moviment_type, data_received, &len_received, &type_received);
        
        if (success) {
            if (type_received == MSG_VISUALIZACAO) {
                printf("New visualization received! Vision size: %d vytes\n", len_received);
            }
            else if (type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4) {
                printf("We found a dot. Receiving file\n");
                receive_file(sock_client, &seq_num, type_received, data_received, len_received);
            }
            else if (type_received == MSG_ERROS) {
                printf("Server reported an error in transmission");
            }
            else if (type_received == MSG_FIM) {
                printf("Game Over! All dots collected or dead by ghosts.");
            }
        }

        /* Receber a mensagem
        recv(soc_server, buffer_rec, 256, 0);
        if (unpack_frame(buffer_rec, read_bytes, &seq_rec, &type_received, data_received, &len_received) == 0) {
            if (type_received == MSG_VISUALIZACAO) desenha mapa;
            else if(type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4)
            encontramos uma pastilha, vamos precisar receber o arquivo e salvá-lo numa pasta de tesouros
            else if(type_received == MSG_ERROS)
            acredito que aqui seria a retransmissão
            else if(type_received == MSG_FIM)
            acabou o jogo
        }

        */
    }
    close(sock_client);
    return 0;
}