#include "client_ui.h"
#include <stdio.h>

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

void show_startup_message(const char *iface) {
    printf("Client started on interface %s. Connecting to server...\n", iface);
}

void show_error(const char *msg) {
    printf("ERROR: %s\n", msg);
}

void show_message(const char *msg) {
    printf("%s\n", msg);
}