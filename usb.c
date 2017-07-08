/**/

#include "usb.h"

/*******************************************************************************
 Function: setupUSB
 Description: Sets up the EUSART module for UART communication, via the USB-UART chip (FT230X)
 Notes: Baud rate error check calculation may not work!
 Inputs: baudrate (called using UART_BAUD, which is set in system.h)
 Outputs: 0 if success, -1 if the baud rate error check failed
*******************************************************************************/
char setupUSB(int baudrate)
{
    long x;
    
    //note that these error-checking calculations don't actually work... yet
    BRGH = 0;
    x = (SYSTEM_FOSC>>6) / baudrate; --x;
    //x = (SYSTEM_FOSC/64) / baudrate; --x;
    if (x<3)                                      //If High Baud Rage Required
    {
        x = (SYSTEM_FOSC>>4) / baudrate; --x;
        //x = (SYSTEM_FOSC/16) / baudrate; --x;
        BRGH = 1;                                     //Setting High Baud Rate
    }
    if (x<3) {return -1;}
    
    SPBRG = x;              //Writing SPBRG Register
    BAUDCONbits.BRG16 = 0;  //8-bit BRG mode
    
    TXSTAbits.TX9 = 0;
    TXSTAbits.SYNC = 0;     //Asynchronous mode
    RCSTAbits.CREN = 1;     //Enables receiver -- note that this may not be needed
    RCSTAbits.SPEN = 1;     //Enables serial port
    TXSTAbits.TXEN = 1;     //Transmit enable -- must happen after the serial port is enabled!
    
    return 0;
}

/*******************************************************************************
 Function: writeUSB
 Description: Writes 1 byte to the USB bus
 Notes: 
 Inputs: byte (the char to be written)
 Outputs: 
*******************************************************************************/
void writeUSB(unsigned char byte)
{
    //since we are using both SPI and UART, we don't set these bits in the setup function
    while(!TRMT);   //wait for shift reg to clear
    TXREG = byte;
}

/*******************************************************************************
 Function: writeUSBstring
 Description: writes a string of bytes
 Notes: The string cannot contain more than 255 characters (including the null-termination)
 Inputs: string (pointer to the string being written)
 Outputs: 
*******************************************************************************/
void writeUSBstring(const char *string)
{
    unsigned char i;
    
    for (i=0; string[i] != '\0'; ++i)
        writeUSB(string[i]);
}

