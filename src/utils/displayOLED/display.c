#include "display.h"

ssd1306_t display;

void init_barr_i2c()
{
    i2c_init(PORT_I2C, 400 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
}

void init_display()
{
    ssd1306_init(&display, 128, 64, ADD_DISPLAY, PORT_I2C);
}

void draw_display(uint32_t x_text, uint32_t y_text, uint32_t scale_text, const char* text)
{
    ssd1306_draw_string(&display, x_text, y_text, scale_text, text);
}

void show_display()
{
    ssd1306_show(&display);
}

void clear_display()
{
    ssd1306_clear(&display);
}

void display_update(float db_level, const char* sound_level) {
    clear_display();

    draw_display(10, 0, 1, "Intensidade Sonora");

    char db_val[30];
    snprintf(db_val, sizeof(db_val), "Nivel: %.1f dB", db_level); 
    draw_display(0, 16, 1, db_val);
    
    char nivel_desc[30];
    char aviso_extra[30] = "";

    if (strcmp(sound_level, "Baixo") == 0) {
        snprintf(nivel_desc, sizeof(nivel_desc), "Classif. Baixo");
        snprintf(aviso_extra, sizeof(aviso_extra), "Nivel Seguro!");
    } else if (strcmp(sound_level, "Moderado") == 0) {
        snprintf(nivel_desc, sizeof(nivel_desc), "Classif. Moderado");
        snprintf(aviso_extra, sizeof(aviso_extra), "Cuidado com o tempo!");
    } else {
        snprintf(nivel_desc, sizeof(nivel_desc), "Classif. Alto");
        snprintf(aviso_extra, sizeof(aviso_extra), "Nivel Prejudicial!");
    }

    draw_display(0, 32, 1, nivel_desc);

    if (aviso_extra[0] != '\0') {
        draw_display(0, 48, 1, aviso_extra);
    }

    show_display();
}