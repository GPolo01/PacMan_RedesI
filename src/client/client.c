#include "../common/socket.h"
#include "../common/protocol.h"
#include "client_net.h"
#include "client_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: sudo ./client <network_interface>\n");
        return -1;
    }

    int sock_client = create_raw_socket(argv[1]);
    unsigned char seq_num = 0;

    MsgType type_received;
    unsigned char len_received, data_received[MAX_DATA_LEN], full_vision[2000];
    int full_len = 0;

    printf("Client started on interface %s. Connecting to server...\n", argv[1]);
    printf("Sending INIT message...\n");

    int success = send_and_wait(sock_client, &seq_num, MSG_INIT, data_received, &len_received, &type_received);

    if (success && type_received == MSG_VISION) {
        recive_vision(sock_client, &seq_num, data_received, len_received, full_vision, &full_len);
        render_map(full_vision, full_len);

    } else {
        printf("Failed to initialize game with server.\n");
        close(sock_client);
        return -1;
    }


    while (1) {
        MsgType movement_type = get_user_movement();

        // This function blocks until a valid response is received, handling timeouts internally
        success = send_and_wait(sock_client, &seq_num, movement_type, data_received, &len_received, &type_received);
        
        if (success) {
            if (type_received == MSG_VISION) {
                recive_vision(sock_client, &seq_num, data_received, len_received, full_vision, &full_len);
                render_map(full_vision, full_len);
            }
            else if (type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4) {
                printf("\n>>> We found a dot! Receiving file...\n");
                receive_file(sock_client, &seq_num, type_received, data_received, len_received);
                printf("\nPress any movement key to continue...\n");
            }
            else if (type_received == MSG_ERROR) {
                printf("ERROR: Server reported an error in transmission.\n");
            }
            else if (type_received == MSG_END) {
                printf("Game Over! Server has ended the game.\n");
                break;
            }
        }
    }
    close(sock_client);
    return 0;
}