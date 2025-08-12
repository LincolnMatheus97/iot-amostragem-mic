#ifndef CLIENTE_HTTP_H
#define CLIENTE_HTTP_H

// -- INCLUDES --
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"

// -- DEFINES --
#define PROXY_HOST "192.168.0.111"
#define PROXY_PORT 8080

// Estrutura para armazenar os dados da amostragem do microfone
typedef struct
{
    float nive_db;
    char nivel_som[20];
} StatusMicrofone;

void enviar_dados_para_nuvem(const StatusMicrofone *dados_a_enviar);

#endif
