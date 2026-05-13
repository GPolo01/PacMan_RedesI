#include "server_ui.h"
#include <stdio.h>

// Auxiliary function to see if exist someone in that position of the matrix
char get_character(int i, int j) {
    if (pacman.x == i && pacman.y == j) return 'P';
    if (red_ghost.x == i && red_ghost.y == j) return 'R';
    if (blue_ghost.x == i && blue_ghost.y == j) return 'B';
    if (green_ghost.x == i && green_ghost.y == j) return 'G';
    if (yellow_ghost.x == i && yellow_ghost.y == j) return 'Y';
    return '\0';
}

// Loads the maze from a CSV file or generates a fallback map if missing
void load_map(char matriz[SIZE][SIZE], const char *filename) {
    FILE *file = fopen(filename, "r");
    
    if (file == NULL) {
        printf("Error: File %s not found. Generating a blank test map...\n", filename);
        // TODO: Change for the UFPR map
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (i == 0 || i == SIZE - 1 || j == 0 || j == SIZE - 1) {
                    matriz[i][j] = 'X'; // walls in the borders 
                } else {
                    matriz[i][j] = '0'; // Space empty
                }
            }
        }
        //Drawing the letters UFPR aligned 
        for (int i = 15; i <= 24; i++) {
            matriz[i][10] = 'X';
            matriz[i][13] = 'X';
        }
        for (int j = 11; j <= 12; j++) matriz[24][j] = 'X';

        for (int i = 15; i <= 24; i++) matriz[i][15] = 'X';
        for (int j = 15; j <= 18; j++) {
            matriz [15][j] = 'X';
            matriz [19][j] = 'X';
        }

        for (int i = 15; i <= 24; i++) matriz[i][20] = 'X';
        for (int i = 15; i <= 19; i++) matriz[20][26] = 'X';
        for (int j = 20; j <= 23; j++) {
            matriz[15][j] = 'X';
            matriz[19][j] = 'X';
        }

        for (int i = 15; i <= 24; i++) matriz[i][20] = 'X';
        for (int i = 15; i <= 19; i++) matriz [i][28] = 'X';
        for (int j = 25; j <= 28; j++) {
            matriz[15][j] = 'X';
            matriz[19][j] = 'X';
        }
        matriz[20][26] = 'X';
        matriz[21][26] = 'X';
        matriz[22][27] = 'X';
        matriz[23][28] = 'X';
        matriz[24][28] = 'X';

        return;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (!fscanf(file, " %c;", &matriz[i][j])) break;
        }
    }
    fclose(file);
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
            if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
                out_buffer[written_bytes] = 'X';
            } else {
                char character = get_character(i, j);
                if (character != '\0') {
                    out_buffer[written_bytes] = character;
                } else {
                    out_buffer[written_bytes] = map[i][j];
                }
            }
            written_bytes++;
        }
    }
    return written_bytes;
}

void render_server_matrix(char matriz[SIZE][SIZE]) {
    printf("\n=== SERVER MAP MATRIX ===\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char character = get_character(i, j);
            if (character != '\0') {
                printf("%c ", character);
            } else {
                printf("%c ", matriz[i][j]);
            }
        }
        printf("\n");
    }
    printf("=========================\n\n");
}
