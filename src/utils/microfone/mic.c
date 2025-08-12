#include "mic.h"

uint16_t adc_buffer[SAMPLES];

void init_config_adc() // Inicializa o ADC (Conversor Analógico-Digital) para o microfone
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    adc_fifo_setup(true, true, 1, false, false);

    adc_set_clkdiv(ADC_CLOCK_DIV);
}

void sample_mic() // Amostra o microfone usando DMA
{
    // Garante que qualquer operação anterior seja completamente interrompida para evitar conflitos.
    dma_channel_abort(dma_channel);
    adc_run(false);

    // Drena o FIFO do ADC para remover quaisquer dados antigos ou corrompidos.
    adc_fifo_drain();

    // Se um som muito alto causou um erro, esta linha o reseta.
    adc_hw->fcs |= ADC_FCS_OVER_BITS;


    // Reconfigura o DMA para começar a escrever no início do nosso buffer.
    dma_channel_configure(
        dma_channel,
        &dma_cfg,
        adc_buffer,             // Ponteiro para o buffer de destino
        &adc_hw->fifo,          // Ponteiro para a fonte (ADC FIFO)
        SAMPLES,                // Quantidade de amostras a capturar
        true                    // Inicia a transferência imediatamente
    );

    // Liga a amostragem contínua do ADC
    adc_run(true);

    // Pausa o Núcleo 0 aqui até o DMA confirmar que preencheu todo o buffer.
    // Isso garante que nunca processaremos dados incompletos.
    dma_channel_wait_for_finish_blocking(dma_channel);
}

// O calculo de rms é baseado no microfone usado
#if USE_EXTERNAL_MIC == 1

// Obtém a tensão RMS (Root Mean Square) do microfone
// A tensão RMS é uma medida da tensão efetiva de uma onda AC (Corrente Alternada).
// A tensão RMS é calculada a partir das amostras digitais do ADC, subtraindo o offset DC (Corrente Continua) e calculando a média dos quadrados das amostras AC.
float get_voltage_rms()
{
    if (SAMPLES == 0) return 0.0f;

    float dc_offset_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++)
        dc_offset_digital += adc_buffer[i];
    dc_offset_digital /= SAMPLES;

    float sum_sq_ac_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++) {
        float ac_sample = (float)adc_buffer[i] - dc_offset_digital;
        sum_sq_ac_digital += ac_sample * ac_sample;
    }

    float rms_ac_digital = sqrtf(sum_sq_ac_digital / SAMPLES);
    float voltage_rms = rms_ac_digital * (3.3f / 4096.0f);
    
    return voltage_rms;
}

float get_db_simulated(float voltage_rms)
{
    float db_level_raw;

    if (voltage_rms < MIN_V_RMS_FOR_DB) {
        db_level_raw = MIN_DB_DISPLAY_LEVEL;
    } else {
        db_level_raw = 73.0f * log10f(voltage_rms / V_REF_DB) - 40.0f;
    }

    if (db_level_raw < MIN_DB_DISPLAY_LEVEL) {
        db_level_raw = MIN_DB_DISPLAY_LEVEL;
    }

    static float smoothed_db_level = MIN_DB_DISPLAY_LEVEL;
    
    if (db_level_raw > smoothed_db_level) {
        // ATAQUE RÁPIDO
        smoothed_db_level = 0.40f * smoothed_db_level + 0.60f * db_level_raw;
    } else {
        // DECAIMENTO LENTO
        smoothed_db_level = 0.96f * smoothed_db_level + 0.04f * db_level_raw;
    }

    float final_db_level = smoothed_db_level;
    if (final_db_level < MIN_DB_DISPLAY_LEVEL) {
        final_db_level = MIN_DB_DISPLAY_LEVEL;
    }
    if (final_db_level > 120.0f) {
        final_db_level = 120.0f;
    }

    return final_db_level;
}

#else

float get_voltage_rms()
{
    if (SAMPLES == 0)
        return 0.0f;

    float dc_offset_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++)
        dc_offset_digital += adc_buffer[i];
    dc_offset_digital /= SAMPLES;

    float sum_sq_ac_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++) {
        float ac_sample = (float)adc_buffer[i] - dc_offset_digital;
        sum_sq_ac_digital += ac_sample * ac_sample;
    }

    float rms_ac_digital = sqrtf(sum_sq_ac_digital / SAMPLES);

    // Filtro de ruído simples, eficaz para o microfone embutido
    if (rms_ac_digital < 5.0f)
        return 0.0f;

    float voltage_rms = rms_ac_digital * (3.3f / 4096.0f);

    return voltage_rms;
}

float get_db_simulated(float voltage_rms)
{
    float db_level_raw;

    if (voltage_rms < MIN_V_RMS_FOR_DB) {
        db_level_raw = MIN_DB_DISPLAY_LEVEL;
    } else {
        db_level_raw = 25.0f * log10f(voltage_rms / V_REF_DB) + 47.0f;
    }

    if (db_level_raw < MIN_DB_DISPLAY_LEVEL) {
        db_level_raw = MIN_DB_DISPLAY_LEVEL;
    }

    static float smoothed_db_level = MIN_DB_DISPLAY_LEVEL;
    
    if (db_level_raw > smoothed_db_level) {
        // ATAQUE RÁPIDO
        smoothed_db_level = 0.40f * smoothed_db_level + 0.60f * db_level_raw;
    } else {
        // DECAIMENTO LENTO
        smoothed_db_level = 0.96f * smoothed_db_level + 0.04f * db_level_raw;
    }

    float final_db_level = smoothed_db_level;
    if (final_db_level < MIN_DB_DISPLAY_LEVEL) {
        final_db_level = MIN_DB_DISPLAY_LEVEL;
    }
    if (final_db_level > 120.0f) {
        final_db_level = 120.0f;
    }
    printf("[DEBUG] RAW dB: %.2f | RMS: %.6f | V_REF_DB: %.6f\n", db_level_raw, voltage_rms, V_REF_DB);

    return final_db_level;
}

#endif // Fim da seleção de código (USE_EXTERNAL_MIC)

const char* classify_sound_level(float db_level)
{   
    const char* level_description;
    if (db_level < LIMIAR_DB_BAIXO_MODERADO)
    {
        level_description = "Baixo";
    }
    else if (db_level < LIMIAR_DB_MODERADO_ALTO)
    {
        level_description = "Moderado";
    }
    else
    {
        level_description = "Alto";
    }

    return level_description;
}