#ifndef SOCKET_H
#define SOCKET_H

// Creates and configures the raw socket in promiscuous mode
// Returns the socket descriptor or exits the program on error
int create_raw_socket(const char *iface);

// Returns the current system time in ms
long long get_timestamp_ms(void);

#endif