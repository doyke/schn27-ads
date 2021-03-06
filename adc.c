// adc.c - process analogue inputs

#include <common.h>
#include <stdint.h>
#include <arraysize.h>
#include <adc.h>


#define ADCCHN_PS		0x06
#define ADCCHN_PD		0x0B

sfr16 ADC0 = 0xBD;		// not present in c8051f410.h

static uint8_t data ready = 0;

static const uint8_t data chn[] = {ADCCHN_PS, ADCCHN_PD};
static uint32_t acc[] = {0, 0};

#define ACC_BITS	8

#define ADC_SAMPLE_RATE (100.0 * ARRAYSIZE(chn) * (1 << ACC_BITS))
#define RELOAD_VALUE (65536L - (uint16_t)(FOSC / ADC_SAMPLE_RATE))


// init ADC and timer3
void adc_init(void)
{
    REF0CN = 0x00;		// Vref as reference

	ADC0TK = 0xFF;
    ADC0CF = 0x80;		// SAR clock = 2.88 MHz
    ADC0CN = 0x81;		// enabled, conversion initiated on overflow of Timer 3
	ADC0MX = ADCCHN_PS;

	CKCON = (CKCON & ~0xC0) | 0x40;		// Timer3 clock = System clock
	TMR3CN = 0x04;		// Timer3 enabled (16 bit)
	TMR3RLH = RELOAD_VALUE >> 8;
	TMR3RLL = RELOAD_VALUE & 0xFF;
	EIE1 |= 0x08;		// enable ADC interrupt
}


// get current values (0..65535)
uint8_t adc_get(uint16_t *ps, uint16_t *pd)
{
	if (!ready)
		return 0;

#if ACC_BITS > 4
	*ps = (uint16_t)(acc[0] >> (ACC_BITS - 4));
	*pd = (uint16_t)(acc[1] >> (ACC_BITS - 4));
#elif ACC_BITS < 4
	*ps = (uint16_t)(acc[0] << (4 - ACC_BITS));
	*pd = (uint16_t)(acc[1] << (4 - ACC_BITS));
#else
	*ps = (uint16_t)acc[0];
	*pd = (uint16_t)acc[1];
#endif

	acc[0] = 0;
	acc[1] = 0;
	ready = 0;

	return 1;
}


// ADC interrupt handler
void adc_isr(void) ADC_INTERRUPT_HANDLER
{
	static uint16_t cnt = 0;
	static uint8_t i = 0;

	AD0INT = 0;		// clear interrupt flag

	if (ready)
		return;		// wait for adc_get

	acc[i] += ADC0;

	if (++cnt >= (1 << ACC_BITS))
	{
		cnt = 0;

		if (++i >= ARRAYSIZE(chn))
		{
			i = 0;
			ready = 1;	// all channels have been read
		}

		ADC0MX = chn[i];
	}
}
