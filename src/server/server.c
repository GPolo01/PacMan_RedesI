#include "socket.h"
#include "protocol.h"
#include "server.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define TAM 40
unsigned char round = 0;
int range_vision = 1;
char map[TAM][TAM];

/* funções:
    cria e fica recarregando mapa(40x40) csv;
    P - PacMan; X - parede; 0 - posicao vazia;
    1 a 6 - os arquivos 2 txt, 2 jpg, 2 mp4; 
    sorteio de posição aleatória (pac-man, fantasmas, pastilhas)
    conecta com o cliente;
    envia visualização do mapa de pac-man;
    espera receber os movimentos;
    movimenta os fantasmas:
        G - alterna entre direita e esquerda;
        R - regra da mão esquerda;
        B - regra da mão direita;
        Y - aleatorio;
*/

void load_map(int *matriz[TAM][TAM], char *nomeArquivo) {
    FILE *file = fopen(nomeArquivo, "r");
    if (file == NULL) {
        printf("Error trying to open file %s\n", nomeArquivo);
        // Otherwise we are going to use the default map UFPR
        return;
    }
    // If the user passes a map
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (!fscanf(file, "%d;", &matriz[i][j]))
                break;
        }
    }
    
    fclose(file);
}

void random_position(char matriz[TAM][TAM], char character) {
    int i, j;
    do {
        i = rand() % TAM;
        j = rand() % TAM;
    } while (matriz[i][j] != '0');
    
    matriz[i][j] = character;
}

void moviment(int *matriz[TAM][TAM], char *character){}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Uso: sudo ./server <interface_de_rede>\n");
        return -1;
    }

    int sock_server = create_raw_socket(argv[1]);
    srand(time(NULL));

    load_map(map, "maze.csv");

    unsigned char seq_rec, len_rec;
    int read_bytes;
    MsgType type_rec;
    unsigned char data_rec[MAX_DATA_LEN];
    unsigned char buffer_rec[256];
    

    printf("Server iniciated, waiting client");

    while (1) {
        read_bytes = recv(sock_server, buffer_rec, sizeof(buffer_rec), 0);

        if( read_bytes > 0 && unpack_frame(buffer_rec, read_bytes, &seq_rec, &type_rec, data_rec, &len_rec) == 0) {
            if (type_rec == MSG_INICIALIZACAO) {
                printf("Connect to client! sending map\n");
                round = 0;
            }
            else if(type_rec >= MSG_MOV_DIR && type_rec <= MSG_MOV_BAIXO) {
                round++;

                // Atualizar posição PacMan

                // Atualiza as posicoes dos fantasmas

                //Verifica colisões nos fantasmas ou em pastilhas

                if (round % 5 == 0) range_vision++;

                // Daí mandamos a matriz que o pacman enxerga

                //enviamos nova visualizacaoo do mapa
            }
        }
    }

    close(sock_server);
    return 0;
}