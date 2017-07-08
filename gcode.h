/* 
 * File:   gcode.h
 * Author: swg
 *
 * Created on August 6, 2016, 3:12 PM
 */

#ifndef GCODE_H
#define	GCODE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <stdio.h>  //convert double to string w/ sprintf
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "system.h"
#include "params.h"
#include "usb.h"
    
typedef struct G_token_
{
    char flag;
    double arg;
} G_token;

typedef enum G_responses
{
    G_SUCCESS,
    G_INVALID_COMMAND,
    G_INVALID_ARGUMENT,
    G_OUT_OF_BOUNDS
} G_RESPONSE;

void parseGcode(char*);
//char execGcode(G_token*);
//char linearMove1Axis(char, double, unsigned short);
//char linearMove2Axis(double, double, unsigned short);
G_RESPONSE execGcode(G_token*);
G_RESPONSE linearMove1Axis(char, double, unsigned short);
G_RESPONSE linearMove2Axis(double, double, unsigned short);
unsigned short calculate_motordelay(unsigned short, unsigned int, unsigned int);
void homeAxis(char);
void moveToSwitch(char, int);


#ifdef	__cplusplus
}
#endif

#endif	/* GCODE_H */

