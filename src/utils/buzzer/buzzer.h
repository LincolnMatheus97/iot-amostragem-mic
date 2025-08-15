#ifndef BUZZER_H
#define BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define PINO_BUZZER 21

// Função para inicializar o PWM do buzzer
void inicializar_pwm_buzzer(uint pin);

// Função não-bloqueante para controlar o buzzer
void set_pwm_buzzer(uint pin, uint freq, uint duty);

#ifdef __cplusplus
}
#endif

#endif