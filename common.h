// common.h

#ifndef COMMON_H
#define COMMON_H

#include <c8051F410.h>

// high priority
#define ADC_INTERRUPT_HANDLER interrupt INTERRUPT_ADC0_EOC using 1
#define CALLED_BY_ADC_INTERRUPT_HANDLER using 1

// low priority
#define UART_INTERRUPT_HANDLER interrupt INTERRUPT_UART0 using 2
#define CALLED_BY_UART_INTERRUPT_HANDLER using 2
#define TIMER2_INTERRUPT_HANDLER interrupt INTERRUPT_TIMER2 using 2
#define CALLED_BY_TIMER2_INTERRUPT_HANDLER using 2
#define PCA_INTERRUPT_HANDLER interrupt INTERRUPT_PCA0 using 2
#define CALLED_BY_PCA_INTERRUPT_HANDLER using 2


// clock frequency
#define FOSC	49000000.0

// enable/disable rs485_transmitter
#define RS485TX_ENABLE() P1 |= 0x80
#define RS485TX_DISABLE() P1 &= ~0x80

// software reset
#define RESET() RSTSRC = 0x10

// interrupt enable/disable
#define INTERRUPT_ENABLE() EA = 1
#define INTERRUPT_DISABLE() EA = 0


#endif
