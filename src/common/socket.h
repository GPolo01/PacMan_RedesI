#ifndef SOCKET_H
#define SOCKET_H

// Cria e configura o raw socket em modo promíscuo
// Retorna o descritor do socket ou encerra o programa em caso de erro
int create_raw_socket(const char *iface);

// Retorna o tempo atual do sistema em ms
long long get_timestamp_ms(void);

#endif