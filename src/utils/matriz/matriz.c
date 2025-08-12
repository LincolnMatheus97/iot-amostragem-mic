#include "matriz.h"


void inicializar_matriz()
{
    ws2812b_init(CANAL_PIO, PINO_MATRIZ, QTD_LEDS);
    ws2812b_set_global_dimming(5);
}

void atualizar_ledbar(int qnt_leds)
{
    ws2812b_clear(); // Limpa a matriz antes de desenhar

    for (int i = 0; i < qnt_leds; i++)
    {
        if (i < LEDS_VERDES)
        {
            // Este LED está na faixa verde
            ws2812b_put(i, GRB_GREEN);
        }
        else if (i < LEDS_VERDES + LEDS_AMARELOS)
        {
            // Este LED está na faixa amarela
            ws2812b_put(i, GRB_YELLOW);
        }
        else
        {
            // Este LED está na faixa vermelha
            ws2812b_put(i, GRB_RED);
        }
    }
}

void renderizar()
{
    ws2812b_render();
}

void limpar_matriz()
{
    ws2812b_clear();
}