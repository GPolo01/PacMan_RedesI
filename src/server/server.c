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

    unsigned char seq_rec, len_rec, server_seq = 0;
    int read_bytes, unpack_status;
    MsgType type_rec;
    unsigned char buffer_rec[256], data_rec[2000];
    
    printf("Server initiated, waiting for client on interface %s...\n", argv[1]);

    while (1) {
        read_bytes = recv(sock_server, buffer_rec, sizeof(buffer_rec), 0);
        unpack_status = unpack_frame(buffer_rec, read_bytes, &seq_rec, &type_rec, data_rec, &len_rec);

        if( read_bytes > 0 && unpack_status == 0) {
            log_message("RECV", seq_rec, type_rec, len_rec);
            
            if (type_rec == MSG_INIT) {
                printf("Connected to client! Sending initial map vision.\n");
                round_num = 0;
                render_server_matrix(matrix);
                unsigned char fog_data[2000];
                int vision_size = get_new_vision(fog_data, vision_range);
                server_send_vision(sock_server, &server_seq, MSG_VISION, fog_data, vision_size);
            }
            else if(type_rec >= MSG_MOV_RIGHT && type_rec <= MSG_MOV_DOWN) {
                round_num++;
                
                if (round_num % 5 == 0 && vision_range < 19) vision_range++;

                pacman_movement(type_rec);
                ghosts_movement(&red_ghost, 1);
                ghosts_movement(&blue_ghost, 2);
                ghosts_movement(&green_ghost, 3);
                ghosts_movement(&yellow_ghost, 4);

                char action = check_collisions();

                render_server_matrix(matrix);

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
                    printf("Sending file %s to client...\n", filepath);
                    int success = server_send_file(sock_server, &server_seq, file_type, filepath, action);
                    if (success) printf("Transmission completed!\n");
                    if (pallets == 6) {
                        printf("ALL PALLETS COLLECTED! YOU WIN!\n");
                        server_send(sock_server, &server_seq, MSG_END, NULL, 0);
                    }
                    continue; // Temporário.
                } else if (action == 'M') {
                    printf("PACMAN LOST ONE LIFE!\n");
                    if (pacman.life == 0) {
                        printf("PACMAN HAS NO MORE LIVES! GAME OVER!\n");
                        server_send(sock_server, &server_seq, MSG_END, NULL, 0);
                        continue;
                    }
                }

                unsigned char fog_data[2000];
                int vision_size = get_new_vision(fog_data, vision_range);

                server_send_vision(sock_server, &server_seq, MSG_VISION, fog_data, vision_size);
            }
        }
        else if (read_bytes > 0 && unpack_status == -1) {
            send_once(sock_server, server_seq, MSG_NACK, NULL, 0);
        }
    }

    close(sock_server);
    return 0;
}