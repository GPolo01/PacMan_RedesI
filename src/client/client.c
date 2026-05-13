#include "../common/socket.h"
#include "../common/protocol.h"
#include "client_net.h"
#include "client_ui.h"
#include "client_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: sudo ./client <network_interface>\n");
        return -1;
    }

    int sock_client = create_raw_socket(argv[1]);
    unsigned char seq_num = 0;

    MsgType type_received;
    unsigned char len_received, data_received[MAX_DATA_LEN], full_vision[2000];
    int full_len = 0;

    show_startup_message(argv[1]);
    show_message("Sending INIT message...");

    int success = send_and_wait(sock_client, &seq_num, MSG_INIT, data_received, &len_received, &type_received);

    if (success && type_received == MSG_VISION) {
        recive_vision(sock_client, &seq_num, data_received, len_received, full_vision, &full_len);
        render_map(full_vision, full_len);

    } else {
        printf("Failed to initialize game with server.\n");
        close(sock_client);
        return -1;
    }


    while (1) {
        MsgType movement_type = get_user_movement();
        show_message("Sending movement command...");

        // This function blocks until a valid response is received, handling timeouts internally
        success = send_and_wait(sock_client, &seq_num, movement_type, data_received, &len_received, &type_received);
        
        if (success) {
            printf("type received: %d\n", type_received);
            if (type_received == MSG_VISION) {
                sleep(1); // Simulate processing time
                recive_vision(sock_client, &seq_num, data_received, len_received, full_vision, &full_len);
                render_map(full_vision, full_len);
            }
            else if (type_received == MSG_TXT || type_received == MSG_JPG || type_received == MSG_MP4) {
                show_message("\n>>> We found a dot! Receiving file...");
                receive_file(sock_client, &seq_num, type_received, data_received, len_received);
                show_message("\nPress any movement key to continue...");
            }
            else if (type_received == MSG_ERROR) {
                show_error("Server reported an error in transmission.");
            }
        }
    }
    close(sock_client);
    return 0;
}