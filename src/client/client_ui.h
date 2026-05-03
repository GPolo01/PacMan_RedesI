#ifndef CLIENT_UI_H
#define CLIENT_UI_H

// Renders the square map matrix to the terminal
void render_map(unsigned char *data, int len);

// Standardized print functions

// Prints a startup message with the name of interface 
void show_startup_message(const char *iface);

// Prints error message
void show_error(const char *msg);

// Prints a message
void show_message(const char *msg);

#endif 