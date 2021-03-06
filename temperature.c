// temperature.c

#include <common.h>
#include <stdint.h>
#include <properties.h>

#define PCA0_READ(val) do {val = PCA0L; val |= (uint16_t)PCA0H << 8;} while(0)
#define PCA0_WRITE(val) do {PCA0L = (uint8_t)(val & 0xFF); PCA0H = (uint8_t)(val >> 8);} while(0)
#define PCA0CP_READ(n, val) do {val = PCA0CPL##n; val |= (uint16_t)PCA0CPH##n << 8;} while(0)
#define PCA0CP_WRITE(n, val) do {PCA0CPL##n = (uint8_t)(val & 0xFF); PCA0CPH##n = (uint8_t)(val >> 8);} while(0)
#define PCA0INT_ENABLE() EIE1 |= 0x10
#define PCA0INT_DISABLE() EIE1 &= ~0x10

static uint16_t posvalue = 0;
static uint16_t negvalue = 0;
static uint8_t ready = 0;

void temperature_init(void)
{
	TMOD |= 0x02;	// 8bit timer
	CKCON |= 0x04;	// SYSCLK
	TH0 = 0;
	TR0 = 1;	// Timer0 enabled

	PCA0MD |= 0x04;	// Timer0 overlow as clock
	PCA0CPM0 = 0x31; // enable CCF0 interrupt
	PCA0CN |= 0x40;	// enable PCA

	PCA0INT_ENABLE();
}

void temperature_update(void)
{
	static uint8_t cnt = 10;
	float t = 0;

	if (ready != 3)
		return;

	if (negvalue != 0)
		t = 694.0f - 751.0f * ((float)posvalue / (float)negvalue);

	if (t > 200 && t < 400)
	{
		if (cnt == 0)
			t = 0.9f * properties.temp + 0.1f * t;	// lowpass
		else
			--cnt;
	}
	else
		t = 288;

	properties.temp = t;

	ready = 0;
}

void pca_isr(void) PCA_INTERRUPT_HANDLER
{
	PCA0_WRITE(0);
	PCA0CN &= ~0x01;	// clear interrupt flag
	
	if (ready == 3)
		return;

	if (P1 & 0x40)
	{
		PCA0CP_READ(0, negvalue);
		ready |= 1;
	}
	else
	{
		PCA0CP_READ(0, posvalue);
		ready |= 2;
	}
}
