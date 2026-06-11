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
    unsigned char client_seq_tx = 0; // Controls the sequence of what the client sends
    unsigned char client_seq_rx = 0; // Controls the sequence of what the client receives

    MsgType type_received;
    unsigned char len_received, data_received[MAX_DATA_LEN], full_vision[2000];
    int full_len = 0;

    FILE *log_file = fopen("client.log", "w");
    if (log_file) {
        fprintf(log_file, "--- CLIENT LOG STARTED ---\n");
        fclose(log_file);
    }

    printf("Client started on interface %s. Connecting to server...\n", argv[1]);
    printf("Sending INIT message...\n");

    int success = send_and_wait(sock_client, &client_seq_tx, MSG_INIT, client_seq_rx);
    if (success) {
        // Actively listens waiting for the first block of data of the server vision
        while (1) {
            int status = recv_frame(sock_client, &client_seq_rx, &type_received, data_received, &len_received, TIMEOUT_MS, "client");
            if (status == 0 && type_received == MSG_VISION) {
                recive_vision(sock_client, &client_seq_rx, data_received, len_received, full_vision, &full_len);
                render_map(full_vision, full_len);
                break;
            }
        }
    } else {
        printf("Failed to initialize game with server.\n");
        close(sock_client);
        return -1;
    }

    while (1) {
        MsgType movement_type = get_user_movement();

        // Sends the movement and waits for the server ACK of receipt
        success = send_and_wait(sock_client, &client_seq_tx, movement_type, client_seq_rx);
        
        if (success) {
            int game_over = 0;
            // Actively listens to know what action the server took (Vision, golden pallet, game over)
            while (1) {
                int status = recv_frame(sock_client, &client_seq_rx, &type_received, data_received, &len_received, TIMEOUT_MS, "client");
                
                if (status == 0) {
                    if (type_received == MSG_VISION) {
                        recive_vision(sock_client, &client_seq_rx, data_received, len_received, full_vision, &full_len);
                        render_map(full_vision, full_len);
                        break;
                    }
                    else if (type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4) {
                        printf("\n>>> We found a dot! Receiving file...\n");
                        receive_file(sock_client, &client_seq_rx, type_received, data_received, len_received);
                        printf("\nPress any movement key to continue...\n");
                        break;
                    }
                    else if (type_received == MSG_ERROR) {
                        printf("ERROR: Server reported an error in transmission.\n");
                        break;
                    }
                    else if (type_received == MSG_END) {
                        printf("Game Over! Server has ended the game.\n");
                        game_over = 1;
                        break;
                    }
                }
            }
            if (game_over) break;
        }
    }
    close(sock_client);
    return 0;
}