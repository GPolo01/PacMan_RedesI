#ifndef SERVER_UI_H
#define SERVER_UI_H

#include "server_game.h"

char get_character(int i, int j);
void load_map(char matriz[SIZE][SIZE], const char *filename);
int get_new_vision(unsigned char *out_buffer, int range);
void render_server_matrix(char matriz[SIZE][SIZE]);

#endif
