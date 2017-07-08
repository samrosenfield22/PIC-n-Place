/* 
 * File:   params.h
 * Author: swg
 *
 * Created on December 8, 2016, 8:55 PM
 */

#ifndef PARAMS_H
#define	PARAMS_H

#ifdef	__cplusplus
extern "C" {
#endif

//system parameters
#define SYSTEM_FOSC (20000000)
#define UART_BAUD (9600)

//machine dimensions
#define XDIM (340)  //all linear dimensions in mm and
#define YDIM (340)  //rotational dimensions in degrees
#define ZDIM (10)
#define RMIN (-180)
#define RMAX (180)
    
#define USTEPS (16)     //number of microsteps each stepper takes
                        //must agree with the settings of the hardware

#define INITIAL_DEFAULT_FEEDRATE (1000)     //default feedrate if none is specified, in mm/min
#define FIXED_Z_FEEDRATE (300)
//#define HOMESPEED_FAST (4000)
#define HOMESPEED_FAST (2000)    //for testing limits
#define HOMESPEED_SLOW (300)
    
#define SOFT_START (1)  //set to 0 to disable the soft start feature, 1 to enable
                        //causes the motor(s) to linearly accelerate/decelerate
#define GOOD_GUI (1)    //if sam's gui is being used, this should be set to 0.
                        //if justin's gui is being used, this should be set to 1


#ifdef	__cplusplus
}
#endif

#endif	/* PARAMS_H */

