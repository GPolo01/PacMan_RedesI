#include "../common/socket.h"
#include "../common/protocol.h"
#include "client_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: sudo ./client <network_interface>\n");
        return -1;
    }

    int sock_client = create_raw_socket(argv[1]);
    unsigned char seq_num = 0;

    printf("Client started on interface %s. Controls: W/A/S/D.\n", argv[1]);

    // First message to Server to initialize game
    // TODO: Need to call send_and_wait here with MSG_INIT to get the first map view

    while (1) {
        char key = getchar();
        MsgType movement_type, type_received;
        unsigned char len_received, data_received[MAX_DATA_LEN];

        switch (key) {
            case 'w': movement_type = MSG_MOV_UP; break;
            case 'a': movement_type = MSG_MOV_LEFT; break;
            case 's': movement_type = MSG_MOV_DOWN; break;
            case 'd': movement_type = MSG_MOV_RIGHT; break;
            default: continue;
        }

        printf("Sending movement command...\n");
        // This function blocks until a valid response is received, handling timeouts internally
        int success = send_and_wait(sock_client, &seq_num, movement_type, data_received, &len_received, &type_received);
        
        if (success) {
            if (type_received == MSG_VISION) {
                printf("New visualization received! Vision size: %d bytes\n", len_received);
                // TODO: Render the map 'data_received' to the terminal
            }
            else if (type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4) {
                printf("We found a dot! Receiving file...\n");
                receive_file(sock_client, &seq_num, type_received, data_received, len_received);
            }
            else if (type_received == MSG_ERROR) {
                printf("Server reported an error in transmission.\n");
            }
            else if (type_received == MSG_END) {
                printf("Game Over! All dots collected or killed by ghosts.\n");
                break;
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