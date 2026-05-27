#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include "../common/protocol.h"

// Blocks and waits for a valid movement key (W, A, S, D)
// Returns the corresponding MsgType command
MsgType get_user_movement(void);

// Renders the square map matrix to the terminal
void render_map(unsigned char *data, int len);

// Opens the file, waits for user to close, and deletes it
void handle_file_viewing(const char *filepath);

#endif 