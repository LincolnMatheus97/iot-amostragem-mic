#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/timer.h"
#include "display.h"
#include "dma.h"
#include "mic.h"
#include "matriz.h"
#include "buzzer.h"

// --- Variáveis globais para comunicação entre os núcleos ---
volatile float g_nivel_db = 0.0f;
volatile int g_qnt_leds_acessos = 0;
volatile const char* g_str_nivel_som = "Iniciando";
volatile bool g_alarme_esta_ativo = false; // Indica se o alarme (sonoro e visual) está ativo
volatile bool g_pause_buzzer_para_renderizacao = false; // Flag para cooperação entre núcleos

// --- NÚCLEO 1: APENAS INTERFACE DE USUÁRIO ---
void core1_entry() {
    inicializar_matriz();
    init_barr_i2c();
    init_display();

    clear_display();
    draw_display(10, 32, 1, "INICIANDO SISTEMA");
    show_display();
    sleep_ms(2000);
    clear_display();
    draw_display(10, 32, 1, "SISTEMA ATIVO");
    show_display();
    sleep_ms(2000);

    char nivel_som_local[16];
    while (1) {
        // Copia os dados globais para variáveis locais para evitar condições de corrida
        float db_level_local = g_nivel_db;
        int leds_local = g_qnt_leds_acessos;
        strcpy(nivel_som_local, (const char *)g_str_nivel_som);

        // A matriz de LED SEMPRE reflete o nível de som atual, sem piscar
        atualizar_ledbar(leds_local);

        // --- LÓGICA DE COOPERAÇÃO ---
        // Se o alarme sonoro estiver ativo, peça ao Núcleo 0 para pausar o buzzer brevemente
        if (g_alarme_esta_ativo) {
            g_pause_buzzer_para_renderizacao = true;
            busy_wait_us_32(100); // Um delay mínimo para garantir que o Núcleo 0 veja a flag
        }

        renderizar();

        // Libera o buzzer imediatamente após a atualização da matriz
        if (g_alarme_esta_ativo) {
            g_pause_buzzer_para_renderizacao = false;
        }
        
        // O display OLED continua atualizando normalmente
        display_update(db_level_local, nivel_som_local);

        sleep_ms(30);
    }
}

// --- NÚCLEO 0: APENAS LÓGICA E PROCESSAMENTO ---
int main()
{
    stdio_init_all();
    
    inicializar_pwm_buzzer(PINO_BUZZER);
    init_config_adc();
    init_config_dma(); 

    printf("Lancando interface no Nucleo 1...\n");
    multicore_launch_core1(core1_entry);

    static float db_antes_buzzer = 0.0f;
    static bool buzzer_foi_ativado = false;
    
    sleep_ms(4000); 

    // Variáveis para a lógica de alarme sustentado
    uint64_t tempo_inic_de_nivel_alto = 0;
    bool temporizador_de_nivel_alto_em_execucao = false;
    const uint64_t ALARME_SUSTENTADO_US = TEMPO_ALARME_SUSTENTADO_S * 1000 * 1000;

    while (1)
    {
        sample_mic();
        float voltage_rms = get_voltage_rms();
        float nivel_db = get_db_simulated(voltage_rms);

        if (g_alarme_esta_ativo && nivel_db > db_antes_buzzer + 3.0f) {
            nivel_db = db_antes_buzzer;
        }

        const char* nivel_som = classify_sound_level(nivel_db);

        printf("RMS: %.6f V, V_REF_DB: %.6f, dB: %.2f\n", voltage_rms, V_REF_DB, nivel_db);

        // Atualiza as variáveis globais para o Núcleo 1
        g_nivel_db = nivel_db;
        g_str_nivel_som = nivel_som;
        
        int qnt_leds_acessos = 0;

        // Só calcula se o som estiver acima do piso mínimo
        if (nivel_db > MIN_DB_MAPA) {
            // Calcula o número de LEDs como um float primeiro
            float leds_float = ((nivel_db - MIN_DB_MAPA) / (MAX_DB_MAPA - MIN_DB_MAPA) * QTD_LEDS);
            qnt_leds_acessos = (int)leds_float;

            // Se o cálculo resultou em 0, mas estamos acima do mínimo, force 1 LED.
            if (qnt_leds_acessos == 0) {
                qnt_leds_acessos = 1;
            }
        }

        // Garante que o valor não ultrapasse o máximo de LEDs
        if (qnt_leds_acessos > QTD_LEDS) {
            qnt_leds_acessos = QTD_LEDS;
        }

        g_qnt_leds_acessos = qnt_leds_acessos;

        // --- LÓGICA DE ALARME SUSTENTADO ---
        if (strcmp(nivel_som, "Alto") == 0) {

            if (!buzzer_foi_ativado) {
                db_antes_buzzer = nivel_db;
                buzzer_foi_ativado = true;
            }
            // Se o som está alto e o timer não foi iniciado, inicie-o
            if (!temporizador_de_nivel_alto_em_execucao) {
                tempo_inic_de_nivel_alto = time_us_64();
                temporizador_de_nivel_alto_em_execucao = true;
                printf("Nivel alto detectado. Iniciando contagem para alarme...\n");
            }
            // Se o timer está rodando e o tempo foi atingido, ative o alarme
            else if (time_us_64() - tempo_inic_de_nivel_alto > ALARME_SUSTENTADO_US) {
                if (!g_alarme_esta_ativo) {
                    printf("ALARME ATIVADO: Nivel alto sustentado!\n");
                }
                g_alarme_esta_ativo = true;
            }
        } else {
            // Se o som não está mais alto, resete tudo
            temporizador_de_nivel_alto_em_execucao = false;
            if (g_alarme_esta_ativo) {
                printf("Alarme desativado. Nivel de ruido normalizado.\n");
            }
            g_alarme_esta_ativo = false;
            buzzer_foi_ativado = false;
        }

        // --- CONTROLE DO BUZZER ---
        if (g_alarme_esta_ativo && !g_pause_buzzer_para_renderizacao) {
            set_pwm_buzzer(PINO_BUZZER, 3000, 50); // Liga o buzzer
        } else {
            set_pwm_buzzer(PINO_BUZZER, 0, 0); // Desliga o buzzer (seja por pausa ou alarme inativo)
        }

        // printf("Debug Nucleo 0: dB: %.1f, Alarm Active: %d, Paused: %d\n", g_db_level, g_is_alarm_active, g_pause_buzzer_for_render);
        sleep_ms(50); 
    }

    return 0;
}