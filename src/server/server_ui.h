#ifndef SERVER_UI_H
#define SERVER_UI_H

#include "server_game.h"

// Retrieves the character at the specified coordinates in the game matrix
char get_character(int i, int j);

// Loads the map from a CSV file or generates a default map
void load_map(const char *filename);

// Generates the visible portion of the game matrix based on PacMan's position and the vision range
int get_new_vision(unsigned char *out_buffer, int range);

// Renders the game matrix on the terminal
void render_server_matrix(char matriz[SIZE][SIZE]);

#endif
