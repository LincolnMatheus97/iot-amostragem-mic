#include <string.h>
#include <stdlib.h>
#include "cliente_ws.h"
#include "ws.h"

// --- Variáveis de Estado ---
static struct tcp_pcb *ws_pcb = NULL;
static ip_addr_t ws_remote_addr;
static volatile WebSocketState ws_state = WS_STATE_DISCONNECTED;
static uint8_t ws_buffer[2048]; // Buffer para construir os pacotes
static int ws_buffer_len = 0;

// --- Callbacks do LwIP ---
static void ws_close_connection() {
    if (ws_pcb != NULL) {
        tcp_arg(ws_pcb, NULL);
        tcp_poll(ws_pcb, NULL, 0);
        tcp_sent(ws_pcb, NULL);
        tcp_recv(ws_pcb, NULL);
        tcp_err(ws_pcb, NULL);
        tcp_close(ws_pcb);
        ws_pcb = NULL;
    }
    ws_state = WS_STATE_DISCONNECTED;
    printf("WebSocket: Conexão fechada.\n");
}

static void callback_erro(void *arg, err_t err) {
    printf("WebSocket: Erro de conexão %d\n", err);
    ws_close_connection();
}

static err_t callback_dados_recebidos(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (!p) {
        printf("WebSocket: Conexão fechada pelo servidor.\n");
        ws_close_connection();
        return ERR_OK;
    }

    // Se estamos esperando a resposta do handshake
    if (ws_state == WS_STATE_HANDSHAKE_SENT) {
        char* response = (char*)p->payload;
        if (p->len > 12 && strstr(response, " 101 ") != NULL) {
            printf("WebSocket: Handshake bem-sucedido! Conexão estabelecida.\n");
            ws_state = WS_STATE_CONNECTED;
        } else {
            printf("WebSocket: Falha no handshake. Resposta:\n%.*s\n", p->len, response);
            ws_close_connection();
        }
    }
    // Se já estamos conectados, podemos processar os frames WebSocket
    else if (ws_state == WS_STATE_CONNECTED) {
        // Aqui você pode usar a função WS::ParsePacket para interpretar dados do servidor
        WebsocketPacketHeader_t header;
        uint8_t payload_buffer[256];

        WS::ParsePacket(&header, (char*)p->payload, p->len);
        
        // Copia o payload (desmascarado) para um buffer local
        memcpy(payload_buffer, (uint8_t *)p->payload + header.start, header.length);
        
        printf("WebSocket: Dados recebidos do servidor: %.*s\n", (int)header.length, payload_buffer);
    }

    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

static err_t callback_conectado(void *arg, struct tcp_pcb *pcb, err_t err) {
    if (err != ERR_OK) {
        printf("WebSocket: Falha ao conectar TCP: %d\n", err);
        ws_close_connection();
        return err;
    }

    printf("WebSocket: Conexão TCP estabelecida. Enviando handshake...\n");

    // Configura os callbacks para receber dados e erros
    tcp_recv(pcb, callback_dados_recebidos);
    tcp_err(pcb, callback_erro);

    // Envia a requisição de Upgrade para WebSocket
    ws_buffer_len = sprintf((char*)ws_buffer,
                            "GET / HTTP/1.1\r\n"
                            "Host: %s:%d\r\n"
                            "Upgrade: websocket\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                            "Sec-WebSocket-Version: 13\r\n"
                            "\r\n",
                            WS_SERVER_HOST, WS_SERVER_PORT);

    cyw43_arch_lwip_begin();
    err_t write_err = tcp_write(pcb, ws_buffer, ws_buffer_len, TCP_WRITE_FLAG_COPY);
    if (write_err == ERR_OK) {
        tcp_output(pcb);
        ws_state = WS_STATE_HANDSHAKE_SENT;
    } else {
        printf("WebSocket: Erro ao enviar handshake: %d\n", write_err);
        ws_close_connection();
    }
    cyw43_arch_lwip_end();

    return ERR_OK;
}

void ws_connect() {
    if (ws_state != WS_STATE_DISCONNECTED) return;

    printf("Iniciando conexão com o servidor WebSocket em %s:%d\n", WS_SERVER_HOST, WS_SERVER_PORT);

    cyw43_arch_lwip_begin();
    
    // Resolve o IP do host
    ip4addr_aton(WS_SERVER_HOST, &ws_remote_addr);

    ws_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!ws_pcb) {
        printf("WebSocket: Erro ao criar PCB.\n");
        cyw43_arch_lwip_end();
        return;
    }
    
    tcp_arg(ws_pcb, NULL);

    ws_state = WS_STATE_CONNECTING;
    err_t err = tcp_connect(ws_pcb, &ws_remote_addr, WS_SERVER_PORT, callback_conectado);
    
    cyw43_arch_lwip_end();

    if (err != ERR_OK) {
        printf("WebSocket: Falha ao iniciar conexão TCP: %d\n", err);
        ws_state = WS_STATE_DISCONNECTED;
    }
}

bool ws_is_connected() {
    return ws_state == WS_STATE_CONNECTED;
}

void ws_send_data(const StatusMicrofone *dados_a_enviar) {
    if (!ws_is_connected()) {
        // printf("WebSocket: Não conectado. Impossível enviar dados.\n");
        return;
    }

    // 1. Serializa os dados para JSON
    char corpo_json[128];
    snprintf(corpo_json, sizeof(corpo_json),
             "{\"niveldB\": %.2f, \"nivelSom\": \"%s\"}",
             dados_a_enviar->nive_db, dados_a_enviar->nivel_som);

    // 2. Constrói o pacote WebSocket usando a classe WS
    //    O '1' no final indica que o payload deve ser mascarado (obrigatório para clientes)
    ws_buffer_len = WS::BuildPacket((char*)ws_buffer, sizeof(ws_buffer), WEBSOCKET_OPCODE_TEXT, corpo_json, strlen(corpo_json), 1);

    // 3. Envia o pacote pela conexão TCP
    cyw43_arch_lwip_begin();
    err_t err = tcp_write(ws_pcb, ws_buffer, ws_buffer_len, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        // printf("WebSocket: Dados enviados.\n");
        tcp_output(ws_pcb);
    } else {
        printf("WebSocket: Erro ao enviar dados: %d\n", err);
        // Se o erro for grave, fecha a conexão para tentar reconectar depois
        if (err == ERR_MEM || err == ERR_CONN) {
            ws_close_connection();
        }
    }
    cyw43_arch_lwip_end();
}