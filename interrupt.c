/**/

#include "interrupt.h"

/*******************************************************************************
 Function: setupInterrupts
 Description: Enables all desired interrupts and clears their flag bits
 Notes: The system uses 3 interrupts: UART receive, TMR0 overflow, and PORTB state change
 Inputs: 
 Outputs: 
*******************************************************************************/
void setupInterrupts()
{
    //clear interrupt flags
    RCIF = 0;   //UART receive flag
    RBIF = 0;   //PORTB change flag
    //TMR0IF = 0;
    
    //enable interrupts
    IPEN = 0;   //disable interrupt priority mode, maybe
    PEIE = 1;   //enables peripheral interrupts (is this necessary?))
    //RBIE = 1;   //enables interrupt on PORTB state change (RB4:RB7)(for limit switches)
    RBIE = 0;
    RCIE = 1;   //enables UART receive int
    TMR0IE = 1;
    GIE = 1;    //enables global interrupts
    
}


/*******************************************************************************
 Function: ISR
 Description: master interrupt service routine for ALL events
 Notes: 
 Inputs: 
 Outputs: 
*******************************************************************************/
void interrupt ISR()
//void interrupt ISR() __attribute__((interrupt))
{
    
    //if (RCIF && RCIE)   //UART receive interrupt
    if (RCIF)
    {
        UART_interrupt();
    }
    //if (TMR0IF && TMR0IE)
    if (TMR0IF)
    {
        TMR0_interrupt();
    }
    /*
    if (RBIF && RBIE)  //PORTB state change interrupt (limit switches) on RB4,5,6,7
    {
        PORTB_interrupt();
    }*/
}

/*******************************************************************************
 Function: UART_interrupt
 Description: UART receive interrupt for processing data from the USB
 Notes: The interrupt adds the byte to the UART buffer string, and checks to see
 if the byte is a newline. If it is, a global flag bit (GCODE_PENDING) is set to
 indicate that the Gcode string is complete and is ready to be processed.
 Inputs: 
 Outputs: 
*******************************************************************************/
void UART_interrupt()
{
    char byte = RCREG;  //also clears RCIF
    
    if (GCODE_PENDING == 1) {return;}
    
    if (byte == '\n')	//end of string
    {
        *ctr = '\0';
        ctr = uart_str;		//reset the pointer for next uart string
        GCODE_PENDING = 1;   //set flag to process the gcode command
        //RCIE = 0;
    }
    else
    {
        *ctr++ = byte;      //add the new byte to the string, then post-increment the address
    }
    
    //RCIF flag cleared in hardware when RCREG is read
}

/*******************************************************************************
 Function: TMR0_interrupt
 Description: Timer 0 overflow interrupt for systemwide timing
 Notes: TMR0 increments every 1/8th of a microsecond, so every us, the interrupt will
 occur. This increments the system time count.
 Inputs: 
 Outputs: 
*******************************************************************************/
void TMR0_interrupt()
{
    SYSTEM_TIME++;  //this is a global variable
    
    //reset the timer0 reg so it'll overflow
    //TMR0L = 0xFF;
    TMR0L = 0xBC;
    //TMR0 = (unsigned char)(-8);
    
    //clear the interrupt flag!
    TMR0IF = 0;
}

/*******************************************************************************
 Function: PORTB_interrupt
 Description: PORTB state change interrupt, which disables motors if a limit switch 
 was pressed
 Notes: This doesn't work -- if the Y axis hits the switch while X is already homed, 
 this will disable X instead of Y!
 Inputs: 
 Outputs: 
*******************************************************************************/
void PORTB_interrupt()
{
    //static char EMERGENCY_COUNT = 0;
    
    //check which switch was closed
    if (X_LIMIT_PIN == 0)
    {
        //disable the motor
        X_AXIS_EN_PIN = 1;  
        
        //zero position
        XPOS = 0;
    }
    else if (Y_LIMIT_PIN == 0)
    {
        //disable the motors
        Y1_AXIS_EN_PIN = 1;
        Y2_AXIS_EN_PIN = 1;
        
        //zero position
        YPOS = 0;
    }
    
    //delay(10);  //maybe not
    RBIF = 0;   //clear interrupt flag
}