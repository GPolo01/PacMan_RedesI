#include "../common/socket.h"
#include "../common/protocol.h"
#include "character.h"
#include "server_net.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define SIZE 40

unsigned char round_num = 0;
int vision_range = 1;

struct character pacman;
struct character red_ghost;
struct character blue_ghost;
struct character green_ghost;
struct character yellow_ghost;

char map[SIZE][SIZE];


/* funções:
    cria e fica recarregando mapa(40x40) csv;
    P - PacMan; X - parede; 0 - posicao vazia;
    1 a 6 - os arquivos 2 txt, 2 jpg, 2 mp4; 
    sorteio de posição aleatória (pac-man, fantasmas, pastilhas)
    conecta com o cliente;
    envia visualização do mapa de pac-man;
    espera receber os movimentos;
    movimenta os fantasmas:
        G - alterna entre direita e esquerda;
        R - regra da mão esquerda;
        B - regra da mão direita;
        Y - aleatorio;
*/

// Loads the maze from a CSV file or generates a fallback map if missing
void load_map(char matriz[SIZE][SIZE], const char *filename) {
    FILE *file = fopen(filename, "r");
    
    if (file == NULL) {
        printf("Error: File %s not found. Generating a blank test map...\n", filename);
        // TODO: Change for the UFPR map
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (i == 0 || i == SIZE - 1 || j == 0 || j == SIZE - 1) {
                    matriz[i][j] = 'X'; // Paredes nas bordas
                } else {
                    matriz[i][j] = '0'; // Caminho livre no meio
                }
            }
        }
        return;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (!fscanf(file, " %c;", &matriz[i][j])) break;
        }
    }
    fclose(file);
}

// Spawns a character at a random empty ('0') position
void random_position(char matriz[SIZE][SIZE], char char_symbol, struct character *c) {
    int i, j;
    do {
        i = rand() % SIZE;
        j = rand() % SIZE;
    } while (matriz[i][j] != '0');

    c->x = i;
    c->y = j;
    c->direction = 0; // All start facing UP 
    matriz[i][j] = char_symbol;
}

// Processes PacMan's movement requested by the client
void pacman_movement(MsgType mov_type) {
    int x = pacman.x;
    int y = pacman.y;

    switch (mov_type) {
        case MSG_MOV_UP:    x--; break;
        case MSG_MOV_DOWN:  x++; break;
        case MSG_MOV_RIGHT: y++; break;
        case MSG_MOV_LEFT:  y--; break;
        default: break;
    }

    // Bounds and wall checking
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return;
    if (map[x][y] == 'X') return;

    // Se comeu pastilha, bateu em fastasmas
    // if (map[x][y]) return;

    // Update map matrices
    map[x][y] = 'P';
    map[pacman.x][pacman.y] = '0';
    pacman.x = x;
    pacman.y = y;
}

/* Ghost movements:
    1 - R - left hand;        
    2 - B - right hand;
    3 - G - switching;
    4 - Y - random;
*/
// Handles the AI for ghosts
void ghosts_movement(char matriz[SIZE][SIZE], struct character *c, int id){
    int x = c->x;
    int y = c->y;

    switch (c->direction) {
        case 0: y++; break;
        case 1: x++; break;
        case 2: y--; break;
        case 3: x--; break;
        default: break;
    }

    // Boundary and collision logic
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || map[x][y] == 'X') {
        switch (id) {
            case 1: // Red Ghost - Left Hand Rule
                if (c->direction == 0) c->direction = 3;
                else c->direction--;
                break;
            case 2: // Blue Ghost - Right Hand Rule
                if (c->direction == 3) c->direction = 0;
                else c->direction++;
                break;
            case 3: // Green Ghost - Alternate Left/Right
                // TODO: Implement toggle state
                break;
            case 4: // Yellow Ghost - Random
                c->direction = rand() % 4;
                break;
        }
        return; // Did not move this turn due to collision
    }

    // Move Ghost on map
    if (id == 1) matriz[x][y] = 'R';
    else if (id == 2) matriz[x][y] = 'B';
    else if (id == 3) matriz[x][y] = 'G';
    else matriz[x][y] = 'Y';
    
    matriz[c->x][c->y] = '0';
    c->x = x;
    c->y = y;
}

// Generates the limited field of view for PacMan
int get_new_vision(unsigned char *out_buffer, int range) {
    int written_bytes = 0;

    /* range 1          range 2
    0   0  0        0   0  0   0  0
    0   P  0        0   0  0   0  0
    0   0  0        0   0  P   0  0
                    0   0  0   0  0
                    0   0  0   0  0
    */

    for (int i = pacman.x - range; i <= pacman.x + range; i++) {
        for (int j = pacman.y - range; j <= pacman.y + range; j++) {
            if (written_bytes >= MAX_DATA_LEN) {
                printf("estourou");
                return written_bytes;
            }

            if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
                out_buffer[written_bytes] = 'X';
            } else {
                out_buffer[written_bytes] = map[i][j];
            }
            written_bytes++;
        }
    }
    return written_bytes;
}

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