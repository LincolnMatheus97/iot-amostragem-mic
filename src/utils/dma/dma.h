#ifndef DMA_H
#define DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/adc.h"

extern uint dma_channel;
extern dma_channel_config dma_cfg;

void init_config_dma();

#ifdef __cplusplus
}
#endif

#endif