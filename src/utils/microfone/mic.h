#ifndef MIC_H
#define MIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "dma.h"

// Altere para 1 se estiver usando o mic EXTERNO, ou deixe em 0 para o INTERNO.
#define USE_EXTERNAL_MIC 0

// Configurações Gerais
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define ADC_CLOCK_DIV 96.f
#define SAMPLES 2500
#define MIN_V_RMS_FOR_DB 0.000001f
#define MIN_DB_DISPLAY_LEVEL 30.0f
#define TEMPO_ALARME_SUSTENTADO_S 3

// Limetes para classificação do som 
#define LIMIAR_DB_BAIXO_MODERADO 70.0f
#define LIMIAR_DB_MODERADO_ALTO 94.0f

// Calibração Automática com base na seleção do microfone
#if USE_EXTERNAL_MIC == 1
    // Valor calibrado para o microfone externo 
    #define V_REF_DB 0.014f
#else
    // Valor original, adequado para o microfone embutido
    #define V_REF_DB 0.0130f
#endif


void init_config_adc();
void sample_mic();
float get_voltage_rms();
float get_db_simulated(float voltage_rms);
const char* classify_sound_level(float db_level);

#ifdef __cplusplus
}
#endif

#endif