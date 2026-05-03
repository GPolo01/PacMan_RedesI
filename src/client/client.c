#include "../common/socket.h"
#include "../common/protocol.h"
#include "client_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void render_map(unsigned char *data, int len) {
    printf("\033[H\033[J"); // Clear terminal screen
    printf("=== DARK PACMAN ===\n\n");

    // searching for the side of the matrix
    int side = 0;
    for (int i = 1; i * i <= len; i += 2) {
        if (i * i == len) {
            side = i;
            break;
        } 
    }

    if (side == 0) {
        printf("Error: Invalid map size received (%d bytes)", len);
        return;
    }

    // print the matrix
    int idx = 0;
    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            printf("%c ", data[idx]);
            idx++;
        }
        printf("\n");
    }
    printf("\nControls: W (Up), S (Down), A (Left), D (Right)\n");
    printf("Awaiting command...\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: sudo ./client <network_interface>\n");
        return -1;
    }

    int sock_client = create_raw_socket(argv[1]);
    unsigned char seq_num = 0;

    MsgType type_received;
    unsigned char len_received, data_received[MAX_DATA_LEN];

    printf("Client started on interface %s. Connecting to server...\n", argv[1]);

    printf("Sending INIT message...\n");
    int success = send_and_wait(sock_client, &seq_num, MSG_INIT, data_received, &len_received, &type_received);

    if (success && type_received == MSG_VISION) {
        render_map(data_received, len_received);
    } else {
        printf("Failed to initialize game with server.\n");
        close(sock_client);
        return -1;
    }


    while (1) {
        char key = getchar();
        MsgType movement_type;

        if (key == '\n') continue;

        switch (key) {
            case 'w': movement_type = MSG_MOV_UP; break;
            case 'a': movement_type = MSG_MOV_LEFT; break;
            case 's': movement_type = MSG_MOV_DOWN; break;
            case 'd': movement_type = MSG_MOV_RIGHT; break;
            default: continue;
        }

        printf("Sending movement command...\n");
        // This function blocks until a valid response is received, handling timeouts internally
        success = send_and_wait(sock_client, &seq_num, movement_type, data_received, &len_received, &type_received);
        
        if (success) {
            if (type_received == MSG_VISION) {
                printf("New visualization received! Vision size: %d bytes\n", len_received);
                render_map(data_received, len_received);
            }
            else if (type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4) {
                printf("\n>>> We found a dot! Receiving file...\n");
                receive_file(sock_client, &seq_num, type_received, data_received, len_received);
                printf("\nPress any movement key to continue...\n");
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