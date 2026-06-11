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
        // --- Letter U (Columns 3 to 8)
        for (int i = 10; i <= 28; i++) {
            matrix[i][3] = 'X';  
            matrix[i][8] = 'X';  
        }
        for (int j = 4; j <= 7; j++) {
            matrix[29][j] = 'X'; 
        }

        // --- Letter F (Columns 12 to 17) ---
        for (int i = 10; i <= 29; i++) matrix[i][12] = 'X'; 
        for (int j = 13; j <= 17; j++) matrix[10][j] = 'X'; 
        for (int j = 13; j <= 16; j++) matrix[19][j] = 'X'; 

        // --- Letter P (Columns 21 to 26) ---
        for (int i = 10; i <= 29; i++) matrix[i][21] = 'X'; 
        for (int j = 22; j <= 25; j++) {
            matrix[10][j] = 'X'; 
            matrix[19][j] = 'X'; 
        }
        for (int i = 11; i <= 18; i++) matrix[i][26] = 'X';
        for (int i = 11; i <= 18; i++) {
            for (int j = 22; j <= 25; j++) {
            matrix[i][j] = 'X';
            }
        }

        // --- Letter R (Columns 30 to 35) ---
        for (int i = 10; i <= 29; i++) matrix[i][30] = 'X';
        for (int j = 31; j <= 34; j++) {
            matrix[10][j] = 'X'; 
            matrix[19][j] = 'X'; 
        }
        for (int i = 11; i <= 18; i++) matrix[i][35] = 'X'; 
        for (int i = 11; i <= 18; i++) {
            for (int j = 31; j <= 34; j++) {
                matrix[i][j] = 'X';
            }
        }

        for (int i = 20; i <= 29; i++) {
            int col = 31 + (i - 20) / 2; 
            if (col <= 35) {
                matrix[i][col] = 'X';
            }
        }

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
