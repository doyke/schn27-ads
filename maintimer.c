// maintimer.c

#include <common.h>
#include <stdint.h>
#include <maintimer.h>

#define FREQ	1000.0
#define TIMER2_RELOAD_VALUE	(65536L - (int)((FOSC / 12.0) / FREQ))

static uint8_t timevalue = 0;

void mainTimer_init(void)
{
	CKCON &= ~0x30;		// Timer2 clock defined by the T2XCLK bit in TMR2CN
	TMR2RLL = TIMER2_RELOAD_VALUE & 0xFF;
	TMR2RLH = TIMER2_RELOAD_VALUE / 256;
	ET2 = 1;			//	enable Timer2 interrupt
    mainTimer_enable(1);
}

uint8_t mainTimer_getTime(void)
{
	return timevalue;
}

void mainTimer_enable(uint8_t enable)
{
	TR2 = enable ? 1 : 0;
}

void timer2_isr(void) TIMER2_INTERRUPT_HANDLER
{
	TF2H = 0;	// clear interrupt flag
	++timevalue;
}
