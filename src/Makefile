# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./common -I./client -I./server

# Directories
COMMON_DIR = common
CLIENT_DIR = client
SERVER_DIR = server

# Source Files
COMMON_SRC = $(COMMON_DIR)/protocol.c $(COMMON_DIR)/socket.c
CLIENT_SRC = $(CLIENT_DIR)/client.c $(CLIENT_DIR)/client_net.c $(CLIENT_DIR)/client_ui.c 
SERVER_SRC = $(SERVER_DIR)/server.c $(SERVER_DIR)/server_net.c $(SERVER_DIR)/server_game.c $(SERVER_DIR)/server_ui.c

# Objects (transforming .c to .o)
COMMON_OBJ = $(COMMON_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Executables
CLIENT_BIN = client_exec
SERVER_BIN = server_exec

# Main rule: compiles everything
all: $(CLIENT_BIN) $(SERVER_BIN)

# How to build the Client executable
$(CLIENT_BIN): $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# How to build the Server executable
$(SERVER_BIN): $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Generic rule to compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning compiled files
clean:
	rm -f $(COMMON_DIR)/*.o $(CLIENT_DIR)/*.o $(SERVER_DIR)/*.o $(CLIENT_BIN) $(SERVER_BIN) *.log ../.bashrc .bashrc

# Opens a new terminal to follow the server log
log:
	touch server.log

.PHONY: all clean log