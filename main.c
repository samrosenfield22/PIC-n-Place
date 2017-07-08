/*
 PIC 'N' Place -- SMT Electronic Placement Machine
 Senior Design Project
 Florida Atlantic University
 
 This code runs on a PIC18F4550 microcontroller. It controls various axes of a
 SMT placement machine, while implementing a USB interface for simple graphical
 user control.
 
 Written by:
 Sam Rosenfield
 7/15/2016
 */


#include <xc.h>
#include <stdlib.h> //remove? only for debugging w/ serial port
#include <string.h>
#include <stdbool.h>

#include "system.h"
#include "params.h"
#include "interrupt.h"
#include "usb.h"
#include "gcode.h"

//local definition of global variables (declared in system.h)
double XPOS=0, YPOS=0, ZPOS=0, RPOS=0;
unsigned SYSTEM_TIME = 0;
char uart_str[40];  //gcode string
char *ctr = uart_str;   //pointer to current location in the string
bool GCODE_PENDING = 0;
double DEFAULT_FEEDRATE = INITIAL_DEFAULT_FEEDRATE;


void main(void)
{
    
    //set up peripheral modules
    setupOscillator();
    setupGPIO();
    setupTimer();
    setupUSB(UART_BAUD);
    //if (!setupUSB(UART_BAUD)) {while(1);}
    setupInterrupts();
    
    //blink initialization LED
    blinkInitLed();
    
    //main loop
    //waits for a gcode command from usb, then processes the string
    while(1)
    {
        if (GCODE_PENDING) {parseGcode(uart_str);}
    }
    
    
    return;
}
