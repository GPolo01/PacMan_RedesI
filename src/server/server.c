#include "../common/socket.h"
#include "../common/protocol.h"
#include "server_game.h"
#include "server_ui.h"
#include "server_net.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: sudo ./server <network_interface>\n");
        return -1;
    }

    int sock_server = create_raw_socket(argv[1]);
    srand(time(NULL));

    load_map("maze.csv");
    pacman.life = 3;

    random_position(&pacman);
    random_position(&red_ghost);
    random_position(&green_ghost);
    random_position(&blue_ghost);
    random_position(&yellow_ghost);

    // The pallets position randomly
    spawn_pallet('1');
    spawn_pallet('2');
    spawn_pallet('3');
    spawn_pallet('4');
    spawn_pallet('5');
    spawn_pallet('6');
    
    FILE *log_file = fopen("server.log", "w");
    if (log_file) {
        fprintf(log_file, "--- SERVER LOG STARTED ---\n");
        fclose(log_file);
    }

    unsigned char seq_rec, len_rec;
    unsigned char server_seq_tx = 0;
    unsigned char server_seq_rx = 0;
    MsgType type_rec;
    unsigned char data_rec[2000];
    
    printf("Server initiated, waiting for client on interface %s...\n", argv[1]);

    while (1) {
        int status = recv_frame(sock_server, &seq_rec, &type_rec, data_rec, &len_rec, 3600000, "server");

        if (status == 0) {
            if (seq_rec == server_seq_rx) {
                if (type_rec == MSG_INIT) {
                    printf("Connected to client! Sending initial map vision.\n");
                    round_num = 0;
                    render_server_matrix(matrix);
                    unsigned char fog_data[2000];
                    int vision_size = get_new_vision(fog_data, vision_range);
                    
                    send_frame(sock_server, seq_rec, MSG_ACK, NULL, 0, "server");
                    server_seq_rx = (server_seq_rx + 1) % 64;

                    server_send_vision(sock_server, &server_seq_tx, MSG_VISION, fog_data, vision_size, server_seq_rx, 0);
                }
                else if (type_rec >= MSG_MOV_RIGHT && type_rec <= MSG_MOV_DOWN) {
                    round_num++;
                    
                    if (round_num % 5 == 0 && vision_range < 19) vision_range++;

                    pacman_movement(type_rec);
                    ghosts_movement(&red_ghost, 1);
                    ghosts_movement(&blue_ghost, 2);
                    ghosts_movement(&green_ghost, 3);
                    ghosts_movement(&yellow_ghost, 4);

                    char action = check_collisions();

                    render_server_matrix(matrix);

                    send_frame(sock_server, seq_rec, MSG_ACK, NULL, 0, "server");
                    server_seq_rx = (server_seq_rx + 1) % 64;

                    if (action >= '1' && action <= '6') {
                        printf("PACMAN ATE PALLET %c!\n", action);

                        MsgType file_type;
                        char filepath[256];

                        switch (action) {
                            case '1': file_type = MSG_TXT; strcpy(filepath, "../pallets/1.txt"); break;
                            case '2': file_type = MSG_TXT; strcpy(filepath, "../pallets/2.txt"); break;
                            case '3': file_type = MSG_JPG; strcpy(filepath, "../pallets/3.jpg"); break;
                            case '4': file_type = MSG_JPG; strcpy(filepath, "../pallets/4.jpg"); break;
                            case '5': file_type = MSG_MP4; strcpy(filepath, "../pallets/5.mp4"); break;
                            case '6': file_type = MSG_MP4; strcpy(filepath, "../pallets/6.mp4"); break;
                        }

                        // Send the vision first
                        unsigned char fog_data[2000];
                        int vision_size = get_new_vision(fog_data, vision_range);
                        server_send_vision(sock_server, &server_seq_tx, MSG_VISION, fog_data, vision_size, server_seq_rx, action);

                        // Then send the file itself
                        printf("Sending file %s to client...\n", filepath);
                        int success = server_send_file(sock_server, &server_seq_tx, file_type, filepath, action, server_seq_rx);
                        if (success) printf("Transmission completed!\n");

                        if (pallets == 6) {
                            printf("ALL PALLETS COLLECTED! YOU WIN!\n");
                            server_send(sock_server, &server_seq_tx, MSG_END, NULL, 0, server_seq_rx);
                            continue; 
                        }
                    } else if (action == 'M') {
                        // Send the vision first so the client can update the map showing Pacman's death / new position
                        unsigned char fog_data[2000];
                        int vision_size = get_new_vision(fog_data, vision_range);
                        server_send_vision(sock_server, &server_seq_tx, MSG_VISION, fog_data, vision_size, server_seq_rx, action);

                        if (pacman.life > 0) {
                            printf("PACMAN LOST ONE LIFE! (%d remaining)\n", pacman.life);
                            server_send_file(sock_server, &server_seq_tx, MSG_JPG, "../pallets/dead.jpg", 'D', server_seq_rx);
                        } else {
                            printf("PACMAN HAS NO MORE LIVES! GAME OVER!\n");
                            server_send_file(sock_server, &server_seq_tx, MSG_JPG, "../pallets/game_over.jpg", 'G', server_seq_rx);
                            server_send(sock_server, &server_seq_tx, MSG_END, NULL, 0, server_seq_rx);
                        }
                    } else {
                        // Regular turn vision send
                        unsigned char fog_data[2000];
                        int vision_size = get_new_vision(fog_data, vision_range);
                        server_send_vision(sock_server, &server_seq_tx, MSG_VISION, fog_data, vision_size, server_seq_rx, 0);
                    }
                }
            } else if (seq_rec == (server_seq_rx + 63) % 64) {
                // Duplicate command (lost previous ACK) -> resend simple ACK
                send_frame(sock_server, seq_rec, MSG_ACK, NULL, 0, "server");
            }
        }
    }

    close(sock_server);
    return 0;
}