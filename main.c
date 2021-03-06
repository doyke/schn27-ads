// main.c

/**
 * Air data system (ADS)
 * @author schn27
 */


#include <common.h>
#include <adc.h>
#include <interface.h>
#include <maintimer.h>
#include <config.h>
#include <temperature.h>


void mainLoop(void);

static void init(void);
static void initResetSources(void);
static void initOscillator(void);
static void initPortIO(void);


void main(void)
{
	init();
	mainLoop();
}


static void init(void)
{
	initResetSources();
	initPortIO();

#if 0	// allready done in bootloader
	initOscillator();
#endif

	config_init();
	adc_init();
	interface_init();
	mainTimer_init();
	temperature_init();

	INTERRUPT_ENABLE();
}


static void initResetSources(void)
{
    int i = 0;

	VDM0CN = 0x80;		// enabled, level low (level HIGH is recommended in datasheet!)
    for (i = 0; i < 20; i++) {}	// Wait 5us for Vdd Monitor stabilization
	RSTSRC = 0x02;		// Vdd Monitor as reset source
}


#if 0
// FOSC = 24.5 MHz * 2 = 49 MHz
static void initOscillator(void)
{
    int i = 0;

    PFE0CN &= ~0x20;
    FLSCL = 0x10;
    PFE0CN |= 0x20;

    CLKMUL = 0x80;
    for (i = 0; i < 20; i++) {}	// Wait 5us for initialization

    CLKMUL |= 0xC0;
    while ((CLKMUL & 0x20) == 0) {}

    CLKSEL = 0x02;

    OSCICN = 0x87;
}
#endif


static void initPortIO(void)
{
    // P0.0  -  Skipped,     Open-Drain, Digital
    // P0.1  -  Skipped,     Open-Drain, Digital
    // P0.2  -  Skipped,     Open-Drain, Digital
    // P0.3  -  Skipped,     Open-Drain, Digital
    // P0.4  -  TX0 (UART0), Push-Pull,  Digital
    // P0.5  -  RX0 (UART0), Push-Pull,  Digital
    // P0.6  -  Skipped,     Open-Drain, Analog
    // P0.7  -  Skipped,     Push-Pull,  Digital

    // P1.0  -  Skipped,     Open-Drain, Digital
    // P1.1  -  Skipped,     Open-Drain, Digital
    // P1.2  -  Skipped,     Open-Drain, Analog
    // P1.3  -  Skipped,     Open-Drain, Analog
    // P1.4  -  Skipped,     Open-Drain, Digital
    // P1.5  -  Skipped,     Open-Drain, Digital
    // P1.6  -  CEX0 (PCA),  Open-Drain, Digital
    // P1.7  -  Skipped,     Push-Pull,  Digital

    // P2.0  -  Skipped,     Open-Drain, Digital
    // P2.1  -  Skipped,     Open-Drain, Digital
    // P2.2  -  Skipped,     Open-Drain, Analog
    // P2.3  -  Skipped,     Open-Drain, Digital
    // P2.4  -  Skipped,     Open-Drain, Digital
    // P2.5  -  Skipped,     Open-Drain, Digital
    // P2.6  -  Skipped,     Open-Drain, Digital
    // P2.7  -  Skipped,     Open-Drain, Digital

    P0MDIN    = 0xBF;
    P1MDIN    = 0xF3;
    P2MDIN    = 0xFB;
    P0MDOUT   = 0xB0;
    P1MDOUT   = 0x80;
    P0SKIP    = 0xCF;
    P1SKIP    = 0xBF;
    P2SKIP    = 0xFF;
    XBR0      = 0x01;
    XBR1      = 0x41;

	P0 |= 0x40;
	P1 |= 0x0C;
	P2 |= 0x04;
}


