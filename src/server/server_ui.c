#include "server_ui.h"
#include <stdio.h>

// COLORS
#define COLOR_RESET   "\x1b[0m"
#define COLOR_WALL    "\x1b[34m" // Blue
#define COLOR_PACMAN  "\x1b[93m" // Bright Yellow
#define COLOR_GHOST_R "\x1b[31m" // Red
#define COLOR_GHOST_G "\x1b[32m" // Green
#define COLOR_GHOST_B "\x1b[36m" // Cyan 
#define COLOR_GHOST_Y "\x1b[33m" // Yellow
#define COLOR_EMPTY   "\x1b[90m" // Dark Gray
#define COLOR_DOT     "\x1b[37m" // Magenta 

// Prints a single character with its corresponding color
void print_colored_char(unsigned char c) {
    switch (c) {
        case 'P': 
            printf("%s%c " COLOR_RESET, COLOR_PACMAN, c); break;
        case 'X': 
            printf("%s%c " COLOR_RESET, COLOR_WALL, c); break;
        case 'R': 
            printf("%s%c " COLOR_RESET, COLOR_GHOST_R, c); break;
        case 'G': 
            printf("%s%c " COLOR_RESET, COLOR_GHOST_G, c); break;
        case 'B': 
            printf("%s%c " COLOR_RESET, COLOR_GHOST_B, c); break;
        case 'Y': 
            printf("%s%c " COLOR_RESET, COLOR_GHOST_Y, c); break;
        case '0': 
            printf("%s%c " COLOR_RESET, COLOR_EMPTY, c); break; 
        case '1': case '2': case '3': case '4': case '5': case '6': 
            printf("%s%c " COLOR_RESET, COLOR_DOT, c); break;
        default:  
            printf("%c ", c); break;
    }
}

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
void load_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    
    if (file == NULL) {
        printf("Error: File %s not found. Generating a blank test map...\n", filename);
        // TODO: Change for the UFPR map
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (i == 0 || i == SIZE - 1 || j == 0 || j == SIZE - 1) {
                    matrix[i][j] = 'X'; // walls in the borders 
                } else {
                    matrix[i][j] = '0'; // Space empty
                }
            }
        }
        //Drawing the letters UFPR aligned 
        for (int i = 15; i <= 24; i++) {
            matrix[i][10] = 'X';
            matrix[i][13] = 'X';
        }
        for (int j = 11; j <= 12; j++) matrix[24][j] = 'X';

        for (int i = 15; i <= 24; i++) matrix[i][15] = 'X';
        for (int j = 15; j <= 18; j++) {
            matrix [15][j] = 'X';
            matrix [19][j] = 'X';
        }

        for (int i = 15; i <= 24; i++) matrix[i][20] = 'X';
        for (int i = 15; i <= 19; i++) matrix[20][26] = 'X';
        for (int j = 20; j <= 23; j++) {
            matrix[15][j] = 'X';
            matrix[19][j] = 'X';
        }

        for (int i = 15; i <= 24; i++) matrix[i][20] = 'X';
        for (int i = 15; i <= 19; i++) matrix [i][28] = 'X';
        for (int j = 25; j <= 28; j++) {
            matrix[15][j] = 'X';
            matrix[19][j] = 'X';
        }
        matrix[20][26] = 'X';
        matrix[21][26] = 'X';
        matrix[22][27] = 'X';
        matrix[23][28] = 'X';
        matrix[24][28] = 'X';

        return;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (!fscanf(file, " %c;", &matrix[i][j])) break;
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
                    out_buffer[written_bytes] = matrix[i][j];
                }
            }
            written_bytes++;
        }
    }
    return written_bytes;
}

void render_server_matrix(char matrix[SIZE][SIZE]) {
    printf("\033[H\033[J"); // Clear terminal screen
    printf("\n=== SERVER MAP MATRIX ===\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char character = get_character(i, j);
            if (character != '\0') {
                print_colored_char(character);
            } else {
                print_colored_char(matrix[i][j]);
            }
        }
        printf("\n");
    }
    printf("=========================\n\n");
}
