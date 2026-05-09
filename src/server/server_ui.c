#include "server_ui.h"
#include <stdio.h>

// Loads the maze from a CSV file or generates a fallback map if missing
void load_map(char matriz[SIZE][SIZE], const char *filename) {
    FILE *file = fopen(filename, "r");
    
    if (file == NULL) {
        printf("Error: File %s not found. Generating a blank test map...\n", filename);
        // TODO: Change for the UFPR map
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (i == 0 || i == SIZE - 1 || j == 0 || j == SIZE - 1) {
                    matriz[i][j] = 'X'; // Paredes nas bordas
                } else {
                    matriz[i][j] = '0'; // Caminho livre no meio
                }
            }
        }
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
            if (written_bytes >= MAX_DATA_LEN) {
                printf("estourou\n");
                return written_bytes;
            }

            if (i < 0 || i >= SIZE || j < 0 || j >= SIZE) {
                out_buffer[written_bytes] = 'X';
            } else {
                out_buffer[written_bytes] = map[i][j];
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
            printf("%c ", matriz[i][j]);
        }
        printf("\n");
    }
    printf("=========================\n\n");
}
