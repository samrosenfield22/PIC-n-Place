/* 
 * File:   usb.h
 * Author: swg
 *
 * Created on June 30, 2016, 5:50 PM
 */

#ifndef USB_H
#define	USB_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <xc.h>
    
#include "system.h"
#include "params.h"

char setupUSB(int);
void writeUSB(unsigned char);
void writeUSBstring(const char*);


#ifdef	__cplusplus
}
#endif

#endif	/* USB_H */

