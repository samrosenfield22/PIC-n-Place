/* 

 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <xc.h>
#include <stdbool.h>
#include "params.h"

  
// ******************************************************************************************* //
// ********************************* PREPROCESSOR DIRECTIVES ********************************* //
// ******************************************************************************************* //

//****************** system parameters ******************
//#define USTEP_PER_MM (21.220659 * USTEPS)
#define USTEP_PER_MM (6.267176 * USTEPS)
    //NEW #:
    //6.267176
    
    /*
    ustep/mm
    = 1 rev / 6pi mm
    * 360 deg / 1 rev
    * 1 step / 0.9 deg
    * n ustep / 1 step
    */
#define USTEP_PER_INCH (4312.03792484 * USTEPS)
    
#define DELAY_FACTOR (30000000 / USTEP_PER_MM)  //60*10^6 us in a minute / # of steps in a mm
                                                // / 2, since each delay is a half-delay
    
//****************** pin definitions ******************
//status LED pins
#define INIT_LED_DIR TRISAbits.RA0      //init and proc pins are switched for REV B
#define INIT_LED_PIN LATAbits.LATA0
#define PROC_LED_DIR TRISAbits.RA1 
#define PROC_LED_PIN LATAbits.LATA1

//delet this?
#define STEPPER_DRV_DIR TRISCbits.RC0   //does nothing in REVC
#define STEPPER_DRV_PIN LATCbits.LATC0
    
//stepper control pins
#define X_AXIS_EN_DIR TRISAbits.RA5
#define X_AXIS_EN_PIN LATAbits.LATA5
#define X_AXIS_DIREC_DIR TRISAbits.RA4
#define X_AXIS_DIREC_PIN LATAbits.LATA4
#define Y1_AXIS_EN_DIR TRISEbits.RE1
#define Y1_AXIS_EN_PIN LATEbits.LATE1
#define Y1_AXIS_DIREC_DIR TRISEbits.RE0
#define Y1_AXIS_DIREC_PIN LATEbits.LATE0
#define Y2_AXIS_EN_DIR TRISCbits.RC1
#define Y2_AXIS_EN_PIN LATCbits.LATC1
#define Y2_AXIS_DIREC_DIR TRISCbits.RC2
#define Y2_AXIS_DIREC_PIN LATCbits.LATC2
#define Z_AXIS_STEP_DIR TRISDbits.RD1
#define Z_AXIS_STEP_PIN LATDbits.LATD1
#define Z_AXIS_DIREC_DIR TRISDbits.RD0
#define Z_AXIS_DIREC_PIN LATDbits.LATD0
#define R_AXIS_EN_DIR TRISDbits.RD3
#define R_AXIS_EN_PIN LATDbits.LATD3
#define R_AXIS_DIREC_DIR TRISDbits.RD2
#define R_AXIS_DIREC_PIN LATDbits.LATD2
    
#define VACUUM_CTRL_DIR TRISDbits.RD4
#define VACUUM_CTRL_PIN LATDbits.LATD4
#define LIGHTBOX_CTRL_DIR TRISDbits.RD5
#define LIGHTBOX_CTRL_PIN LATDbits.LATD5
    
/*#define TX_DIR TRISCbits.RC6
#define TX_PIN LATCbits.LATC6
#define RX_DIR TRISCbits.RC7
#define RX_PIN LATCbits.LATC7*/


//must be set on interrupt-on-change pins (i.e. RB4 to RB7)
#define X_LIMIT_DIR TRISBbits.RB4
#define X_LIMIT_PIN PORTBbits.RB4
#define Y_LIMIT_DIR TRISBbits.RB5
#define Y_LIMIT_PIN PORTBbits.RB5
#define Z_LIMIT_DIR TRISBbits.RB6
#define Z_LIMIT_PIN PORTBbits.RB6
  
// ******************************************************************************************* //
// ************************************* GLOBAL VARIABLES ************************************ //
// ******************************************************************************************* //
//note: local definitions are found in main.c
extern double XPOS, YPOS, ZPOS, RPOS;
extern unsigned SYSTEM_TIME;
extern bool GCODE_PENDING;
extern char uart_str[40];
extern char *ctr;
extern double DEFAULT_FEEDRATE;

  
// ******************************************************************************************* //
// ********************************** FUNCTION DECLARATIONS ********************************** //
// ******************************************************************************************* //
void setupOscillator(void);
void setupGPIO(void);
void setupTimer(void);
void delay_ms(unsigned short);
void delay_us(unsigned short);
void blinkInitLed(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEM_H */

