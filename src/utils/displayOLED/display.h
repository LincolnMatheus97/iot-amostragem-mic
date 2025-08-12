#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define PORT_I2C i2c1
#define PIN_SDA 14
#define PIN_SCL 15
#define ADD_DISPLAY 0X3C

void init_barr_i2c();
void init_display();
void draw_display(uint32_t x_text, uint32_t y_text, uint32_t scale_text, const char* text);
void show_display();
void clear_display();
void display_update(float db_level, const char* sound_level);

#endif