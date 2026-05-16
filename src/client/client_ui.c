#include "client_ui.h"
#include <stdio.h>

MsgType get_user_movement(void) {
    char key;
    while (1) {
        key = getchar();
        
        // Ignore Enter key artifacts left in the standard input buffer
        if (key == '\n') continue;

        switch (key) {
            case 'w': case 'W': return MSG_MOV_UP;
            case 'a': case 'A': return MSG_MOV_LEFT;
            case 's': case 'S': return MSG_MOV_DOWN;
            case 'd': case 'D': return MSG_MOV_RIGHT;
            default: continue; // Ignore invalid keys and keep waiting
        }
    }
}

void render_map(unsigned char *data, int len) {
    printf("\033[H\033[J"); // Clear terminal screen
    printf("=== DARK PACMAN ===\n\n");

    // Searching for the side of the matrix
    int side = 0;
    for (int i = 1; i * i <= len; i += 2) {
        if (i * i == len) {
            side = i;
            break;
        }
    }

    if (side == 0) {
        printf("Error: Invalid map size received (%d bytes)\n", len);
        return;
    }

    // Print the matrix
    int idx = 0;
    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            printf("%c ", data[idx]);
            idx++;
        }
        printf("\n");
    }
    printf("\nControls: W (Up), S (Down), A (Left), D (Right)\n");
    printf("Awaiting command...\n");
}