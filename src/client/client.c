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
                char dummy_action = 0;
                recive_vision(sock_client, &client_seq_rx, data_received, len_received, full_vision, &full_len, &dummy_action);
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
                        char action_after_vision = 0;
                        recive_vision(sock_client, &client_seq_rx, data_received, len_received, full_vision, &full_len, &action_after_vision);
                        render_map(full_vision, full_len);
                        
                        if (action_after_vision != 0) {
                            // Check if a file is coming next (after eating a pallet or dying)
                            MsgType file_type;
                            unsigned char file_len, file_data[MAX_DATA_LEN];
                            int file_status = recv_frame(sock_client, &client_seq_rx, &file_type, file_data, &file_len, TIMEOUT_MS, "client");
                            
                            if (file_status == 0) {
                                if (file_type == MSG_TXT || file_type == MSG_JPG || file_type == MSG_MP4) {
                                    char number = file_data[0];

                                    if (number == 'D') {
                                        printf("\nPACMAN lost one life\n");
                                    } else if (number == 'G') {
                                        printf("\nGAME OVER\n");
                                    } else {
                                        printf("\nPallet found! Receiving file: %c \n", number);
                                    }

                                    receive_file(sock_client, &client_seq_rx, file_type, file_data, file_len);
                                    
                                    char filepath[128];
                                    
                                    if (file_type == MSG_TXT) sprintf(filepath, "../dots/%c.txt", number);
                                    else if (file_type == MSG_JPG) sprintf(filepath, "../dots/%c.jpg", number);
                                    else if (file_type == MSG_MP4) sprintf(filepath, "../dots/%c.mp4", number);

                                    char command[512];
                                    char chmod_cmd[256];

                                    char *sudo_user = getenv("SUDO_USER");
                                    sprintf(chmod_cmd, "chmod 777 %s", filepath);
                                    system(chmod_cmd);

                                    if (sudo_user != NULL) {
                                        if (file_type == MSG_MP4) {
                                            // O player de vídeo precisa ser forçado a achar o monitor e o áudio do usuário logado
                                            sprintf(command, "sudo -u %s env DISPLAY=:0 XDG_RUNTIME_DIR=/run/user/$(id -u %s) xdg-open %s > /dev/null 2>&1", sudo_user, sudo_user, filepath);
                                        } else {
                                            // Imagens e textos são mais simples e abrem normalmente
                                            sprintf(command, "sudo -u %s xdg-open %s > /dev/null 2>&1", sudo_user, filepath);
                                        }
                                    } else {
                                        sprintf(command, "xdg-open %s > /dev/null 2>&1", filepath);
                                    }
                                    
                                    system(command);

                                    printf("\nFile Open\n");
                                    printf("Press [ENTER] in the terminal when you want to go back\n");

                                    int key;
                                    while((key = getchar()) != '\n' && key != EOF);

                                    if(remove(filepath) == 0) printf("Removing File, move to continue\n");
                                    else printf("Warning, not possible to destroy file\n");
                                    
                                    // Check if MSG_END is sent next (in case it was the last pallet or game over)
                                    MsgType end_type;
                                    unsigned char end_len, end_data[MAX_DATA_LEN];
                                    int end_status = recv_frame(sock_client, &client_seq_rx, &end_type, end_data, &end_len, 150, "client");
                                    if (end_status == 0 && end_type == MSG_END) {
                                        printf("Game Over! Server has ended the game.\n");
                                        game_over = 1;
                                    }
                                } else if (file_type == MSG_END) {
                                    printf("Game Over! Server has ended the game.\n");
                                    game_over = 1;
                                }
                            }
                        }
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