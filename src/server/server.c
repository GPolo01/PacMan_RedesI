#include "socket.h"
#include "protocol.h"
#include "character.h"
#include "server_net.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define TAM 40
unsigned char round = 0;
int range_vision = 1;
struct character pacman;
struct character red_ghost;
struct character blue_ghost;
struct character green_ghost;
struct character yellow_ghost;
char map[TAM][TAM];

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

void load_map(int *matriz[TAM][TAM], char *nomeArquivo) {
    FILE *file = fopen(nomeArquivo, "r");
    if (file == NULL) {
        printf("Error trying to open file %s\n", nomeArquivo);
        // Otherwise we are going to use the default map UFPR
        return;
    }
    // If the user passes a map
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (!fscanf(file, "%d;", &matriz[i][j]))
                break;
        }
    }
    
    fclose(file);
}

void random_position(char matriz[TAM][TAM], char character, struct character *c) {
    int i, j;
    do {
        i = rand() % TAM;
        j = rand() % TAM;
    } while (matriz[i][j] != '0');

    c->x = i;
    c->y = j;
    c->direction = 0; // All start facing up 
    matriz[i][j] = character;
}

void pacman_moviment(MsgType mov_type) {
    int x = pacman.x;
    int y = pacman.y;

    switch (mov_type) {
        case MSG_MOV_CIMA:
            x--;
            break;
        case MSG_MOV_BAIXO:
            x++;
            break;
        case MSG_MOV_DIR:
            y++;
            break;
        case MSG_MOV_ESQ:
            y--;
            break;
        default:
            break;
    }

    // Verificamos se passou das bordas ou bateu na parede
    if (x < 0 || x >= TAM || y < 0 || y >= TAM) return;
    if (map[x][y] == 'X') return;
    // Se comeu pastilha, bateu em fastasmas
    // if (map[x][y]) return;

    map[x][y] = 'P';
    map[pacman.x][pacman.y] = '0';
    pacman.x = x;
    pacman.y = y;
}

/*movimenta os fantasmas:
    1 - R - regra da mão esquerda;        
    2 - B - regra da mão direita;
    3 - G - alterna entre direita e esquerda;
    4 - Y - aleatorio;
*/
void ghosts_moviment(int *matriz[TAM][TAM], struct character *c, int id){
    int x,y;

    x = c->x;
    y = c->y;

    switch (c->direction) {
    case 0:
        y++;
        break;
    case 1:
        x++;
        break;
    case 2:
        y--;
        break;
    case 3:
        x--;
        break;
    default:
        break;
    }
    // Verificamos se passou das bordas ou bateu na parede
    if (x < 0 || x >= TAM || y < 0 || y >= TAM || map[x][y] == 'X') {
        switch (id)
        {
        case 1:
            if (c->direction == 0) c->direction = 3;
            else c->direction--; 
            break;
        case 2:
            if (c->direction == 3) c->direction = 0;
            else c->direction++;
            break;
        case 3:
            // como vou fazer para ficar alternando se adicionar na struct
            break;
        case 4:
            c->direction = rand() % 4;
            break;
        default:
            break;
        }
    }

    if (id == 1) map[x][y] = 'R';
    else if (id == 2) map[x][y] = 'B';
    else if (id == 3) map[x][y] = 'G';
    else map[x][y] = 'Y';
    
    map[c->x][c->y] = '0';
    c->x = x;
    c->y = y;
}

int new_vision(unsigned char *out_buffer, int range) {
    int side_size = (range * 2) + 1;
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
            if (i < 0 || i >= TAM || j < 0 || j >= TAM) {
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
        printf("Uso: sudo ./server <interface_de_rede>\n");
        return -1;
    }

    int sock_server = create_raw_socket(argv[1]);
    srand(time(NULL));

    // Mapa e personagens
    load_map(map, "maze.csv");
    
    pacman.life = 3;
    random_position(map, "P", &pacman);
    random_position(map, "R", &red_ghost);
    random_position(map, "G", &green_ghost);
    random_position(map, "B", &blue_ghost);
    random_position(map, "Y", &yellow_ghost);

    unsigned char seq_rec, len_rec, server_seq;
    int read_bytes;
    MsgType type_rec;
    unsigned char buffer_rec[256], data_rec[MAX_DATA_LEN];
    
    printf("Server iniciated, waiting client");
    server_seq = 0;

    while (1) {
        read_bytes = recv(sock_server, buffer_rec, sizeof(buffer_rec), 0);
        if( read_bytes > 0 && unpack_frame(buffer_rec, read_bytes, &seq_rec, &type_rec, data_rec, &len_rec) == 0) {
            if (type_rec == MSG_INICIALIZACAO) {
                printf("Connect to client! sending map\n");
                round = 0;

                unsigned char fog_data[MAX_DATA_LEN];
                int vision_size = new_vision(fog_data, range_vision);

                server_send(sock_server, &server_seq, MSG_VISUALIZACAO, fog_data, vision_size);
            }
            else if(type_rec >= MSG_MOV_DIR && type_rec <= MSG_MOV_BAIXO) {
                round++;
                if (round % 5 == 0) range_vision++;

                // Atualizar posição PacMan
                pacman_moviment(type_rec);
                // Atualiza as posicoes dos fantasmas
                ghosts_moviment(map, &red_ghost, 1);
                ghosts_moviment(map, &blue_ghost, 2);
                ghosts_moviment(map, &green_ghost, 3);
                ghosts_moviment(map, &yellow_ghost, 4);

                //Verifica colisões nos fantasmas ou em pastilhas

                // Daí mandamos a matriz que o pacman enxerga
                unsigned char fog_data[MAX_DATA_LEN];
                int vision_size = new_vision(fog_data, range_vision);

                //enviamos nova visualizacaoo do mapa
                server_send(sock_server, &server_seq, MSG_VISUALIZACAO, fog_data, vision_size);
            }
        }
    }

    close(sock_server);
    return 0;
}