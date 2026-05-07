#ifndef CLIENT_INPUT_H
#define CLIENT_INPUT_H

#include "../common/protocol.h"

// Blocks and waits for a valid movement key (W, A, S, D)
// Returns the corresponding MsgType command
MsgType get_user_movement(void);

#endif