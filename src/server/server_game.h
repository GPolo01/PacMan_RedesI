#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include "../common/protocol.h"

#define SIZE 40

struct character {
    char left_right_sense; //Only for the green ghost (0-right,1-left)
    int x;
    int y;
    int old_x;
    int old_y;
    int life;
    int direction;
};

// Directions 0 - UP 1  RIGHT 2- DOWN 3 -  LEFT 4

extern unsigned char round_num;
extern int vision_range;
extern int pallets;

extern struct character pacman;
extern struct character red_ghost;
extern struct character blue_ghost;
extern struct character green_ghost;
extern struct character yellow_ghost;

extern char matrix[SIZE][SIZE];

// Spawns a character at a random empty ('0') position
void random_position(struct character *c);

// Spawns a pallet (item) at a random empty ('0') position
void spawn_pallet(char item);

// Processes PacMan's movement requested by the client
void pacman_movement(MsgType mov_type);

// Processes the movement of ghosts based on their AI and current direction 
void ghosts_movement(struct character *c, int id);

// Checks for collisions between PacMan and ghosts, as well as PacMan and pallets
char check_collisions();

#endif