// adc.h

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init(void);
uint8_t adc_get(uint16_t *ps, uint16_t *pd);

#endif
