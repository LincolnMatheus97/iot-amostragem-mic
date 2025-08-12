#include "buzzer.h"

// Definição de uma função para inicializar o PWM no pino do buzzer
void inicializar_pwm_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_enabled(slice_num, false); // Começa desligado
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125); // Um divisor padrão
    pwm_init(slice_num, &config, true);
}

void set_pwm_buzzer(uint pin, uint freq, uint duty) {
    uint slice_num = pwm_gpio_to_slice_num(pin);

    if (freq == 0 || duty == 0) {
        // Frequência ou duty 0 significa desligar o som
        pwm_set_gpio_level(pin, 0);
        return;
    }

    // Calcula o valor de "wrap" (topo da contagem) para a frequência desejada
    // Usando o clock do sistema (tipicamente 125MHz)
    // Formula: F_pwm = 125,000,000 / (clkdiv * wrap)
    // Com clkdiv=125, temos F_pwm = 1,000,000 / wrap
    uint32_t wrap = 1000000 / freq;
    pwm_set_wrap(slice_num, wrap);

    // Calcula o nível do PWM para o duty cycle desejado
    uint32_t level = (wrap * duty) / 100;
    pwm_set_gpio_level(pin, level);
}