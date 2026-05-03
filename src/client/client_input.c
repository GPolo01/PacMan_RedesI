#include "client_input.h"
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