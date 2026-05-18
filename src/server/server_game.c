#include "server_game.h"
#include <stdio.h>
#include <stdlib.h>

unsigned char round_num = 0;
int vision_range = 1;
int pallets = 0;

struct character pacman;
struct character red_ghost;
struct character blue_ghost;
struct character green_ghost;
struct character yellow_ghost;

char matrix[SIZE][SIZE];

/* functions:
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
void random_position(struct character *c) {
    int i, j;
    do {
        i = rand() % SIZE;
        j = rand() % SIZE;
    } while (matrix[i][j] != '0');

    c->x = i;
    c->y = j;
    c->direction = 0; // All start facing UP 
    c->left_right_sense = '0'; //only important to Green Ghost
}

void spawn_pallet(char item) {
    int i, j;
    do {
        i = rand() % SIZE;
        j = rand() % SIZE;
    } while (matrix[i][j] != '0');
    
    matrix[i][j] = item;
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
        default: return;
    }

    // Bounds and wall checking
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return;
    if (matrix[x][y] == 'X') return;

    // Update localization
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

// Replacement of switch for 2 vetors
int dir_x[4] = {-1, 0, 1, 0};
int dir_y[4] = {0, 1, 0, -1};
void ghosts_movement(struct character *c, int id){
    int direction = c->direction;
    int x = c->x + dir_x[direction];
    int y = c->y + dir_y[direction];

    // If it okay to go(not a wall), just go
    if (x >= 0 && x < SIZE && y >= 0 && y < SIZE && matrix[x][y] != 'X') {
        c->x = x;
        c->y = y;
        return; 
    }

    // Otherwise, detected collision
    int new_direction = direction;

    // Red, blue, green and yellow
    if (id == 1) new_direction = (direction + 3) % 4;
    else if (id == 2) new_direction = (direction + 1) % 4;
    else if (id == 3) {
        if (c->left_right_sense == '0') {
            new_direction = (direction + 1 ) % 4;
            c->left_right_sense = '1'; 
        } else {
            new_direction = (direction + 3) % 4;
            c->left_right_sense = '0';
        }
    }
    else if (id == 4) new_direction = rand() % 4;

    c->direction = new_direction;
    int possible = 0;

    // Check all the directions so he doesn't just stand in front of a wall
    while (possible < 4) {
        x = c->x + dir_x[c->direction];
        y = c->y + dir_y[c->direction];

        if (x >= 0 && x < SIZE && y >= 0 && y < SIZE && matrix[x][y] != 'X') {
            c->x = x;
            c->y = y;
            return;
        }

        c->direction = (c->direction + 1) % 4;
        possible++;
    }
}

char check_collisions() {
    // With ghosts
    if((pacman.x == red_ghost.x && pacman.y == red_ghost.y) || 
        (pacman.x == blue_ghost.x && pacman.y == blue_ghost.y) ||
        (pacman.x == green_ghost.x && pacman.y == green_ghost.y) ||
        (pacman.x == yellow_ghost.x && pacman.y == yellow_ghost.y)) {
        pacman.life--;
        return 'M';
    }

    // verification to pallets
    char item = matrix[pacman.x][pacman.y];
    if (item >= '1' && item <= '6') {
        matrix[pacman.x][pacman.y] = '0';
        pallets++;
        return item;
    }
    return '0';
}