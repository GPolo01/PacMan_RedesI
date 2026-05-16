# Compilador e Flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./common -I./client -I./server

# Diretórios
COMMON_DIR = common
CLIENT_DIR = client
SERVER_DIR = server

# Arquivos Fonte
COMMON_SRC = $(COMMON_DIR)/protocol.c $(COMMON_DIR)/socket.c
CLIENT_SRC = $(CLIENT_DIR)/client.c $(CLIENT_DIR)/client_net.c $(CLIENT_DIR)/client_ui.c 
SERVER_SRC = $(SERVER_DIR)/server.c $(SERVER_DIR)/server_net.c $(SERVER_DIR)/server_game.c $(SERVER_DIR)/server_ui.c

# Objetos (transformando .c em .o)
COMMON_OBJ = $(COMMON_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Executáveis
CLIENT_BIN = client_exec
SERVER_BIN = server_exec

# Regra principal: compila tudo
all: $(CLIENT_BIN) $(SERVER_BIN)

# Como montar o executável do Cliente
$(CLIENT_BIN): $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Como montar o executável do Servidor
$(SERVER_BIN): $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Regra genérica para compilar os .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos compilados
clean:
	rm -f $(COMMON_DIR)/*.o $(CLIENT_DIR)/*.o $(SERVER_DIR)/*.o $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: all clean