#include "server_game.h"
#include <stdio.h>
#include <stdlib.h>

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