#include "client_input.h"
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

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