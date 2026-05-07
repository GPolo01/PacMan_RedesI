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

    load_map(map, "maze.csv");
    pacman.life = 3;

    random_position(map, 'P', &pacman);
    random_position(map, 'R', &red_ghost);
    random_position(map, 'G', &green_ghost);
    random_position(map, 'B', &blue_ghost);
    random_position(map, 'Y', &yellow_ghost);

    unsigned char seq_rec, len_rec, server_seq = 0;
    int read_bytes;
    MsgType type_rec;
    unsigned char buffer_rec[256], data_rec[MAX_DATA_LEN];
    
    printf("Server initiated, waiting for client on interface %s...\n", argv[1]);

    while (1) {
        read_bytes = recv(sock_server, buffer_rec, sizeof(buffer_rec), 0);

        if( read_bytes > 0 && unpack_frame(buffer_rec, read_bytes, &seq_rec, &type_rec, data_rec, &len_rec) == 0) {
            printf("type received: %d\n", type_rec);
            
            if (type_rec == MSG_INIT) {
                printf("Connected to client! Sending initial map vision.\n");
                round_num = 0;
                unsigned char fog_data[MAX_DATA_LEN];
                int vision_size = get_new_vision(fog_data, vision_range);
                server_send(sock_server, &server_seq, MSG_VISION, fog_data, vision_size);
            }
            else if(type_rec >= MSG_MOV_RIGHT && type_rec <= MSG_MOV_DOWN) {
                round_num++;
                printf("Round %d\n", round_num);
                
                if (round_num % 5 == 0) vision_range++;

                pacman_movement(type_rec);
                ghosts_movement(map, &red_ghost, 1);
                ghosts_movement(map, &blue_ghost, 2);
                ghosts_movement(map, &green_ghost, 3);
                ghosts_movement(map, &yellow_ghost, 4);

                render_server_matrix(map);

                unsigned char fog_data[MAX_DATA_LEN];
                int vision_size = get_new_vision(fog_data, vision_range);

                //enviamos nova visualizacaoo do mapa
                server_send(sock_server, &server_seq, MSG_VISION, fog_data, vision_size);
                printf("Map sended\n");
            }
        }
    }

    close(sock_server);
    return 0;
}