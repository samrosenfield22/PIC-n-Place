/* 
 * File:   interrupt.h
 * Author: swg
 *
 * Created on October 30, 2016, 3:26 PM
 */

#ifndef INTERRUPT_H
#define	INTERRUPT_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <xc.h>
#include "system.h"

void setupInterrupts(void);
void UART_interrupt(void);
void TMR0_interrupt(void);
void PORTB_interrupt(void);


#ifdef	__cplusplus
}
#endif

#endif	/* INTERRUPT_H */

