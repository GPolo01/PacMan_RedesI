#include "client_ui.h"
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

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

MsgType get_user_movement(void) {
    char key;
    struct termios old, new;

    tcgetattr(STDIN_FILENO, &old);
    
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    while (1) {
        key = getchar();
        MsgType moviment;
        
        // Ignore Enter key artifacts left in the standard input buffer
        if (key == '\n') continue;

        switch (key) {
            case 'w': case 'W': moviment = MSG_MOV_UP; break;
            case 'a': case 'A': moviment = MSG_MOV_LEFT; break;
            case 's': case 'S': moviment = MSG_MOV_DOWN; break;
            case 'd': case 'D': moviment = MSG_MOV_RIGHT; break;
            default: continue; // Ignore invalid keys and keep waiting
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        return moviment;
    }
}

void render_map(unsigned char *data, int len) {
    printf("\033[H\033[J"); // Clear terminal screen
    printf("=== DARK PACMAN ===\n\n");

    // Searching for the side of the matrix
    int side = 0;
    for (int i = 1; i * i <= len; i++) {
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
            print_colored_char(data[idx]);
            idx++;
        }
        printf("\n");
    }
    printf("\nControls: W (Up), S (Down), A (Left), D (Right)\n");
    printf("Awaiting command...\n");
}