/**/

#include "system.h"

/*******************************************************************************
 Function: setupOscillator
 Description: Configures the system oscillator
 Notes: Currently uses the 20MHz crystal oscillator. Can also use the 8MHz HS INTOSC.
 Inputs: 
 Outputs: 
*******************************************************************************/
void setupOscillator()
{
    OSCCONbits.SCS = 0b00;  //system clocked from primary osc
    //OSCCONbits.IRCF = 0b111;    //undivided intosc (8MHz)
    //OSCCONbits.SCS = 0b11;  //clock system from internal osc
}

/*******************************************************************************
 Function: setupGPIO
 Description: sets all GPIO pin directions, A/D directions, and initial states
 Notes: 
 Inputs: 
 Outputs: 
*******************************************************************************/
void setupGPIO()
{
    ADCON1bits.PCFG = 0b1111;   //configure all inputs as digital I/O
    
    //clear all ports
    PORTA; PORTB; PORTC; PORTD; PORTE;
    
    //set pin directions
    //TRISB = 0x0;    //all output, so when PORTB pullups are enabled, they will only be enabled on desired pins
    
    INIT_LED_DIR = 0;
    PROC_LED_DIR = 0;
    STEPPER_DRV_DIR = 0;
    VACUUM_CTRL_DIR = 0;
    LIGHTBOX_CTRL_DIR = 0;
    //TX_DIR = 1;
    //RX_DIR = 1;
    X_AXIS_DIREC_DIR = 0;
    X_AXIS_EN_DIR = 0;
    Y1_AXIS_DIREC_DIR = 0;
    Y1_AXIS_EN_DIR = 0;
    Y2_AXIS_DIREC_DIR = 0;
    Y2_AXIS_EN_DIR = 0;
    Z_AXIS_DIREC_DIR = 0;
    Z_AXIS_STEP_DIR = 0;
    R_AXIS_DIREC_DIR = 0;
    R_AXIS_EN_DIR = 0;
    
    //limit switches
    X_LIMIT_DIR = 1;
    Y_LIMIT_DIR = 1;
    //Z_LIMIT_DIR = 1;
    
    //enable PORTB pull ups
    INTCON2bits.RBPU = 0b0;
    LATBbits.LATB4 = 1;
    LATBbits.LATB5 = 1;
    
    
    //initialize output settings
    INIT_LED_PIN = 1;   //active low
    PROC_LED_PIN = 1;
    STEPPER_DRV_PIN = 0;
    VACUUM_CTRL_PIN = 1;
    LIGHTBOX_CTRL_PIN = 0;
    X_AXIS_EN_PIN = 1;
    Y1_AXIS_EN_PIN = 1;
    Y2_AXIS_EN_PIN = 1;
    Z_AXIS_STEP_PIN = 1;
    R_AXIS_EN_PIN = 1;
}

/*******************************************************************************
 Function: setupTimer
 Description: Initializes the TMR0 module for the main system timer (for delays)
 Notes: This should increment TMR0 every _____
 Inputs: 
 Outputs: 
*******************************************************************************/
void setupTimer()
{
    T0CONbits.T08BIT = 1;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;      //enable prescaler
    //T0CONbits.PSA = 1;  //no prescale
    T0CONbits.T0PS = 0b001; //1:4 prescale
    //T0CONbits.T0PS = 0b100; //higher prescale
    T0CONbits.TMR0ON = 1;
}

/*******************************************************************************
 Function: delay_us
 Description: Pauses for the given number of microseconds
 Notes: Max. delay is 65535 us
 Inputs: us (number of microseconds to delay)
 Outputs: 
*******************************************************************************/
void delay_us(unsigned short us)
{
	
    //implementation with global system timer:
    unsigned int current_time = SYSTEM_TIME;
    us >>= 6;   //64us per timer tick
    while(SYSTEM_TIME - current_time < us);
}

/*******************************************************************************
 Function: delay_ms
 Description: Pauses for the given number of milliseconds
 Notes: Max. delay is 65535 ms
 Inputs: ms (number of milliseconds to delay)
 Outputs: 
*******************************************************************************/
void delay_ms(unsigned short ms)
{
    //implementation with global system timer:
    unsigned int current_time = SYSTEM_TIME;
    unsigned int us = (unsigned int)ms << 4;    //multiply by 1000 (ms to us), divide by 64 (us per timer tick)
    //1000/64 = 15.625, or approx. 16
    while(SYSTEM_TIME - current_time < us);
}

/*******************************************************************************
 Function: blinkInitLed
 Description: Blinks the onboard INIT LED 5 times to indicate that the system has initialized
 Notes: 
 Inputs: 
 Outputs: 
*******************************************************************************/
void blinkInitLed()
{
    char i;
    
    for (i=0; i<5; ++i)
    //while(1)
    {
        INIT_LED_PIN = 0;
        delay_ms(150);
        INIT_LED_PIN = 1;
        delay_ms(150);
    }
}