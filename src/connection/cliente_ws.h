#ifndef CLIENTE_WS_H
#define CLIENTE_WS_H

#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"

// -- DEFINES --
#define WS_SERVER_HOST "192.168.0.111" // Coloque o IP do seu servidor aqui
#define WS_SERVER_PORT 8080            // Coloque a Porta do seu servidor aqui

// Estrutura para os dados do microfone
typedef struct StatusMicrofone {
    float nive_db;
    char nivel_som[20];
} StatusMicrofone;

// Enum para gerenciar o estado da conexão
typedef enum {
    WS_STATE_DISCONNECTED,
    WS_STATE_CONNECTING,
    WS_STATE_HANDSHAKE_SENT,
    WS_STATE_CONNECTED,
    WS_STATE_CLOSING
} WebSocketState;

// Funções públicas
void ws_connect();
bool ws_is_connected();
void ws_send_data(const StatusMicrofone *dados_a_enviar);

#endif