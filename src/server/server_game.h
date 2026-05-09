#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../common/protocol.h"

#define SIZE 40

struct character {
    int x;
    int y;
    int life;
    int direction;
};

// Directions 0 - UP 1  RIGHT 2- DOWN 3 -  LEFT 4

extern unsigned char round_num;
extern int vision_range;

extern struct character pacman;
extern struct character red_ghost;
extern struct character blue_ghost;
extern struct character green_ghost;
extern struct character yellow_ghost;

extern char map[SIZE][SIZE];

void random_position(char matriz[SIZE][SIZE], char char_symbol, struct character *c);
void pacman_movement(MsgType mov_type);
void ghosts_movement(char matriz[SIZE][SIZE], struct character *c, int id);

#endif