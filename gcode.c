/*
 
 */

#include "gcode.h"

/*******************************************************************************
 Function: parseGcode
 Description: Parses and tokenizes a string of Gcode commands, and calls execGcode()
 
 Notes: The Gcode string is broken up into tokens, where each token consists of a 
 character and a number. The tokens are stored in the G_token structure. Once the whole
 string has been split, the tokens are passed to execGcode().
 After the command has completed execution, process data is sent to the user application 
 over USB.
 
 Inputs: Gcode (pointer to the string to be tokenized)
 Outputs: 
*******************************************************************************/
void parseGcode(char *Gcode)
{
    char i=0;
    char *buf=NULL, *comment_delim=NULL;
    G_token Gparsed[4];
	
    char finalposition[40];
    char gcode_process;
    
    //turn on process LED
    PROC_LED_PIN = 0;
    
    //initialize all token data to 0
    memset( Gparsed, 0, sizeof(Gparsed) );
    
    //remove comment
    comment_delim = strchr(Gcode, ';');
    if (comment_delim != NULL) {*comment_delim = '\0';}
    
    buf = strtok(Gcode, " ");
    while(buf != NULL)
    {
        //process stored token
        Gparsed[i].flag = buf[0];   //store flag char
        buf++;
        Gparsed[i].arg = atof(buf); //store argument value

        //grab the next token
        buf = strtok(NULL, " ");
        ++i;
    }
    
    //execute Gcode command
    gcode_process = execGcode(Gparsed);
    
    //prepare for subsequent command
    GCODE_PENDING = 0;    //allow new commands to be accepted
    //RCIE = 1;
    memset(uart_str, 0, sizeof(uart_str));
    PROC_LED_PIN = 1;
	
    
    //send process data back to the GUI
    switch (gcode_process)
    {
        case G_SUCCESS:
            if (GOOD_GUI) {sprintf(finalposition, "X%.3f Y%.3f Z%.3f\n", XPOS, YPOS, ZPOS);}
            else {sprintf(finalposition, "%.3f %.3f %.3f\n", XPOS, YPOS, ZPOS);}
            writeUSBstring(finalposition);
            break;
        case G_INVALID_COMMAND:
            writeUSBstring("INVALID COMMAND\n");
            break;
        case G_INVALID_ARGUMENT:
            writeUSBstring("INVALID ARGUMENT\n");
            break;    
        case G_OUT_OF_BOUNDS:
            writeUSBstring("OUT OF BOUNDS\n");
            break;  
        default:
            //this should never happen
            writeUSBstring("UNKNOWN STATE\n");
    }
	
}

/*******************************************************************************
 Function: execGcode
 Description: Selects the correct action from the given Gcode tokens
 Notes: 
 Inputs: Gparsed (pointer to an array of Gcode tokens)
 Outputs: Process data about the Gcode's execution
*******************************************************************************/
G_RESPONSE execGcode(G_token *Gparsed)
{
    int i;
    
    char axis = 0;
    unsigned short feedrate = DEFAULT_FEEDRATE;
    double arg1, arg2;  //for single or dual-axis linear motion
    
    if (Gparsed[0].flag == 'G')
    {
        
        //************************* G COMMANDS *************************//
        switch((char)Gparsed[0].arg)
        {
            case 1:             //linear move
                if (Gparsed[1].flag == 'F')
                {
                    DEFAULT_FEEDRATE = Gparsed[1].arg;
                    return G_SUCCESS;
                }
                
                axis = Gparsed[1].flag;
                arg1 = Gparsed[1].arg;
                if ( ((axis<'X') || (axis>'Z')) && (axis!='R')) {return G_INVALID_ARGUMENT;}//return false;
                
                //check if next token contains a second axis, or a feedrate
                if (Gparsed[2].flag == 0)   //no 2nd argument token
                {
                    return linearMove1Axis(axis, arg1, feedrate);  //initialized to DEFAULT_FEEDRATE
                }
                else if (Gparsed[2].flag == 'F')
                {
                    feedrate = Gparsed[2].arg;
                    return linearMove1Axis(axis, arg1, feedrate);
                }
                else if (Gparsed[2].flag == 'Y')        //double axis move
                {
                    arg2 = Gparsed[2].arg;
                    if (Gparsed[3].flag == 0)
                    {
                        return linearMove2Axis(arg1, arg2, feedrate);
                    }
                    else if (Gparsed[3].flag == 'F')
                    {
                        feedrate = Gparsed[3].arg;
                        return linearMove2Axis(arg1, arg2, feedrate);
                    }
                    else {return G_INVALID_ARGUMENT;}//return false;}
                }
                else {return G_INVALID_ARGUMENT;}//return false;}
                
                break;

            case 4:             //dwell (delay n msec or sec)
                i = Gparsed[1].arg;
                
                if (Gparsed[1].flag == 'S') {delay_ms(i);}
                else if (Gparsed[1].flag == 'P') {delay_us(i);}
                else {return G_INVALID_ARGUMENT;}
                return G_SUCCESS;
                break;

            case 28:            //home
                axis = Gparsed[1].flag;
                
                if (axis == 0)   //no argument
                {
                    //home axes, one by one, in order
                    homeAxis('X');
                    homeAxis('Y');
                }
                else if ( (axis == 'X') || (axis == 'Y') || (axis == 'Z') ) {homeAxis(axis);}
                else {return G_INVALID_ARGUMENT;}
                break;

            default:            //unrecognized command
                return G_INVALID_COMMAND;
        }   
    }       //end of 'G' commands
    
    else if (Gparsed[0].flag == 'M')
    {
        //************************* M COMMANDS *************************//
        switch((char)Gparsed[0].arg)
        {
            case 10:    //vacuum on
                VACUUM_CTRL_PIN = 0;
                break;
                
            case 11:    //vacuum off
                VACUUM_CTRL_PIN = 1;
                break;
            
            case 12:    //lightbox on
                LIGHTBOX_CTRL_PIN = 1;
                break;
                
            case 13:    //lightbox off
                LIGHTBOX_CTRL_PIN = 0;
                break;
                
            default:    //unrecognized command
                return G_INVALID_COMMAND;
        }
    }
    
    else    //Gcode string doesn't begin with 'G' or 'M' -- invalid command
    {
        return G_INVALID_COMMAND;
    }
    
    return G_SUCCESS;
}

/*******************************************************************************
 Function: linearMove1Axis
 Description: Linearly moves 1 machine axis to the specified location
 Notes: Or, if the given axis is R (Z-rotation), it turns to the specified angle
 Inputs:    axis (Which axis is moving -- must be 'X', 'Y', 'Z', or 'R')
            arg (Position coordinate the given axis must move to, in mm -- must be in bounds)
            feedrate (axis's travel speed, in mm/min)
 Outputs: Process data about the Gcode's execution
*******************************************************************************/
G_RESPONSE linearMove1Axis(char axis, double arg, unsigned short feedrate)
{
    unsigned int i, steps;
    double del;
    unsigned short calc_delay, halfdelay;
    
    //disable all axes (active low)
    //X_AXIS_EN_PIN = 1;
    //Y1_AXIS_EN_PIN = 1;
    //Y2_AXIS_EN_PIN = 1;
    //R_AXIS_EN_PIN = 1;
    
    //check lower bound
    if ( (arg < 0) && (axis != 'R') ) {return G_OUT_OF_BOUNDS;}  //minimum for all axes
    
    /*
    //enable the correct axis, and calculate the travel distance for that axis
    if (axis == 'X')
    {
        if (arg > XDIM) {return G_OUT_OF_BOUNDS;}    //check bounds
        
        R_AXIS_EN_PIN = 1;  //only disable rotation motor for X/Y motion
        
        X_AXIS_EN_PIN = 0;
        del = arg - XPOS;
        XPOS = arg;
        
        //if (X_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    else if (axis == 'Y')
    {
        if (arg > YDIM) {return G_OUT_OF_BOUNDS;}
        
        R_AXIS_EN_PIN = 1;  //only disable rotation motor for X/Y motion
        
        Y1_AXIS_EN_PIN = 0; Y2_AXIS_EN_PIN = 0;
        del = arg - YPOS;
        YPOS = arg;
        
        //if (Y_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    else if (axis == 'Z')
    {
        if (arg > ZDIM) {return G_OUT_OF_BOUNDS;}
        
        //Z_AXIS_EN_PIN = 0;
        del = arg - ZPOS;
        ZPOS = arg;
        
        //if (Z_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    else if (axis == 'R')
    {
        //convert to degrees
        //arg *= (63/360);
        arg *= 0.175;
        
        if ( (arg < RMIN) || (arg > RMAX) ) {return G_OUT_OF_BOUNDS;}
        
        R_AXIS_EN_PIN = 0;
        del = arg - RPOS;
        RPOS = arg;
        
        //if (Z_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    
    
    //set motor direction (sets all direction pins, since other motors are disabled)
    if (del > 0)
    {
        //set all direction pins high
        X_AXIS_DIREC_PIN = 1;
        Y1_AXIS_DIREC_PIN = 1;
        Y2_AXIS_DIREC_PIN = 1;
        Z_AXIS_DIREC_PIN = 1;
        R_AXIS_DIREC_PIN = 1;
    }
    else
    {
        X_AXIS_DIREC_PIN = 0;
        Y1_AXIS_DIREC_PIN = 0;
        Y2_AXIS_DIREC_PIN = 0;
        Z_AXIS_DIREC_PIN = 0;
        R_AXIS_DIREC_PIN = 0;
        del *= -1;
    }
    
    */
    
    //calculate the travel distance for that axis
    if (axis == 'X')
    {
        if (arg > XDIM) {return G_OUT_OF_BOUNDS;}    //check bounds
        del = arg - XPOS;
        XPOS = arg;
        
    }
    else if (axis == 'Y')
    {
        if (arg > YDIM) {return G_OUT_OF_BOUNDS;}
        del = arg - YPOS;
        YPOS = arg;
        
        //if (Y_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    else if (axis == 'Z')
    {
        if (arg > ZDIM) {return G_OUT_OF_BOUNDS;}
        del = arg - ZPOS;
        ZPOS = arg;
        
        //if (Z_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    else if (axis == 'R')
    {
        //convert to degrees
        //arg *= (63/360);
        arg *= 0.175;
        
        if ( (arg < RMIN) || (arg > RMAX) ) {return G_OUT_OF_BOUNDS;}
        del = arg - RPOS;
        RPOS = arg;
        
        //if (Z_LIMIT_PIN == 0) {RBIE = 0;}   //disable PORTB interrupt
    }
    
    if (del > 0)
    {
        //set all direction pins high
        X_AXIS_DIREC_PIN = 1;
        Y1_AXIS_DIREC_PIN = 1;
        Y2_AXIS_DIREC_PIN = 1;
        Z_AXIS_DIREC_PIN = 1;
        R_AXIS_DIREC_PIN = 1;
    }
    else
    {
        X_AXIS_DIREC_PIN = 0;
        Y1_AXIS_DIREC_PIN = 0;
        Y2_AXIS_DIREC_PIN = 0;
        Z_AXIS_DIREC_PIN = 0;
        R_AXIS_DIREC_PIN = 0;
        del *= -1;
    }
    
    
    
    delay_ms(50);
    
    
    //calculate the number of steps required for the given distance
    steps = del*USTEP_PER_MM;       //or, ustep/inch
    
    /************ move the motor ****************/
    if (axis == 'X')
    {
        //z axis will be at a fixed rate specified in system.h
        calc_delay = DELAY_FACTOR/feedrate;
        for (i=0; i<steps; ++i)
        {
            //calc_delay = calculate_motordelay(300, i, steps);

            X_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            X_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
    }
    else if (axis == 'Y')
    {
        //z axis will be at a fixed rate specified in system.h
        calc_delay = DELAY_FACTOR/feedrate;
        for (i=0; i<steps; ++i)
        {
            //calc_delay = calculate_motordelay(300, i, steps);

            Y1_AXIS_EN_PIN = 1;
            Y2_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            Y1_AXIS_EN_PIN = 0;
            Y2_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
    }
    else if (axis == 'Z')
    {
        //z axis will be at a fixed rate specified in system.h
        calc_delay = DELAY_FACTOR/FIXED_Z_FEEDRATE;
        for (i=0; i<steps; ++i)
        {
            //calc_delay = calculate_motordelay(300, i, steps);

            Z_AXIS_STEP_PIN = 1;
            delay_us(calc_delay);
            Z_AXIS_STEP_PIN = 0;
            delay_us(calc_delay);
        }
    }
    else if (axis == 'R')
    {
        //z axis will be at a fixed rate specified in system.h
        calc_delay = DELAY_FACTOR/FIXED_Z_FEEDRATE;
        for (i=0; i<steps; ++i)
        {
            //calc_delay = calculate_motordelay(300, i, steps);

            R_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            R_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
    }
    
    
    //re-enable the PORTB interrupt, in case it was disabled
    //RBIE = 1;
    
    return G_SUCCESS;
}


/*******************************************************************************
 Function: linearMove2Axis
 Description: Moves both X and Y axes to the given coordinates
 Notes: The dual-axis motion is completed in 2 steps. First, the X and Y axes move
 simultaneously (the head moves at a 45deg angle) until all of one of the axes' motion
 is complete. The remaining single-axis motion is done by calling linearMove1Axis().
 Inputs:    x_target (Desired X-axis position, in mm)
            y_target (Desired Y-axis position, in mm)
            feedrate (axis's travel speed, in mm/min)
 Outputs: Process data about the Gcode's execution
*******************************************************************************/
G_RESPONSE linearMove2Axis(double x_target, double y_target, unsigned short feedrate)
{
    unsigned int i, steps;
    double del_x, del_y, del;    //total x and y motion required (del stores the shorter value)
    unsigned short halfdelay, calc_delay;
    
    char singleaxis;        //axis for single-axis motion
    double singlearg;       //argument for single-axis motion
    
    // INITIAL CALCULATIONS AND SETTINGS
    
    //check bounds
    if ((x_target < 0) || (x_target > XDIM)) {return G_OUT_OF_BOUNDS;}
    if ((y_target < 0) || (y_target > YDIM)) {return G_OUT_OF_BOUNDS;}
    
    
    //disable interrupt if one axis starts at its home position
    //NOTE that if the other axis crashes, it will not stop!!
    //if ( (X_LIMIT_PIN == 0) || (Y_LIMIT_PIN == 0) ) {RBIE = 0;}   //disable PORTB interrupt
    
    //calculate X and Y travel distances
    del_x = x_target - XPOS;
    if (del_x > 0) {X_AXIS_DIREC_PIN = 1;}
    else {del_x *= -1; X_AXIS_DIREC_PIN = 0;}
    del_y = y_target - YPOS;
    if (del_y > 0) {Y1_AXIS_DIREC_PIN = 1; Y2_AXIS_DIREC_PIN = 1;}
    else {del_y *= -1; Y1_AXIS_DIREC_PIN = 0; Y2_AXIS_DIREC_PIN = 0;}
    
    //determine which axis will be left over, after the 45deg motion
    if (del_x > del_y)
    {
        del = del_y;
        
        singleaxis = 'X';
        singlearg = x_target;
    }
    else
    {
        del = del_x;
        
        singleaxis = 'Y';
        singlearg = y_target;
    }
    
    //update machine position
    if (X_AXIS_DIREC_PIN == 1) {XPOS += del;}
    else {XPOS -= del;}
    if (Y1_AXIS_DIREC_PIN == 1) {YPOS += del;}
    else {YPOS -= del;}
    delay_ms(30);
    
    
    //move the motors
    steps = del*USTEP_PER_MM;
    
    if (steps < (feedrate/10))    //travel dist is too short for a full slowstart
    {
        for (i=0; i<steps; ++i)    //or, ustep/inch
        {
            calc_delay = calculate_motordelay(feedrate, i, steps) * 1.4142136;

            //STEPPER_DRV_PIN = 1;
            X_AXIS_EN_PIN = 1;
            Y1_AXIS_EN_PIN = 1;
            Y2_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            X_AXIS_EN_PIN = 0;
            Y1_AXIS_EN_PIN = 0;
            Y2_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
    }
    else
    {
        for (i=0; i<(feedrate/20); ++i)
        {
            calc_delay = calculate_motordelay(feedrate, i, steps) * 1.4142136;

            X_AXIS_EN_PIN = 1;
            Y1_AXIS_EN_PIN = 1;
            Y2_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            X_AXIS_EN_PIN = 0;
            Y1_AXIS_EN_PIN = 0;
            Y2_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
        calc_delay = DELAY_FACTOR/feedrate;
        calc_delay *= 1.4142136;
        for (; i<=steps-(feedrate/20); ++i)
        {
            X_AXIS_EN_PIN = 1;
            Y1_AXIS_EN_PIN = 1;
            Y2_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            X_AXIS_EN_PIN = 0;
            Y1_AXIS_EN_PIN = 0;
            Y2_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
        for (; i<steps; ++i)
        {
            calc_delay = calculate_motordelay(feedrate, i, steps) * 1.4142136;

            X_AXIS_EN_PIN = 1;
            Y1_AXIS_EN_PIN = 1;
            Y2_AXIS_EN_PIN = 1;
            delay_us(calc_delay);
            X_AXIS_EN_PIN = 0;
            Y1_AXIS_EN_PIN = 0;
            Y2_AXIS_EN_PIN = 0;
            delay_us(calc_delay);
        }
    }
    
    
    //move the remaining single-axis motion
    linearMove1Axis(singleaxis, singlearg, feedrate);
    
    //re-enable the interrupt
    //RBIE = 1;
    
    return G_SUCCESS;
    //note: this assumes that linearMove1Axis() above will return success as well,
    //since it will only fails if the new coordinates are out of bounds (which will make linearMove2Axis() fail)
}


/*******************************************************************************
 Function: calculate_motordelay
 Description: Calculates the delay for a given step in an axis's motion
 
 Notes: instead of starting a motor at full speed, steppers are linearly accelerated
 and decelerated to/from the given speed to reduce jerk. This is done by calculating the
 delay for each step in the motor's path. If the motor is in the first/last n steps in
 its path, the delay is calculated from the given acceleration to allow a smooth speed
 change. This n, the depth of the slow start/stop, is a function of the desired feedrate
 and the hardcoded initial feedrate.
 
 To calculate the delay, we assume that in the first n steps, the motor will linearly 
 accelerate until n/2 steps and then decelerate until n steps, at which point full speed
 should be acheived. By integrating the acceleration ramp over the number of steps taken,
 we solve for the appropriate feedrate, and calculate the delay.
 
 Note that this functionality can be disabled by setting the SOFT_START parameter
 (in system.h) to 0.
 
 Inputs:    feedrate (Desired instantaneous speed, in mm/min)
            i (Number of steps the motor has already taken)
            steps (Total number of steps needed to complete the move)
 Outputs: The delay time needed to get the desired feedrate
*******************************************************************************/
unsigned short calculate_motordelay(unsigned short feedrate, unsigned int i, unsigned int steps)
{
    unsigned short halfdelay;
    unsigned short x;        //how "deep" into the slow-feedrate zone the move is
    
    unsigned short depth = feedrate / 20;
    unsigned short init_feedrate = 500;
    
    unsigned short acc_max;
    
    
    //if (feedrate < init_feedrate) {return 30000/feedrate;}
    if (feedrate < init_feedrate) {return DELAY_FACTOR/feedrate;}
    
    if (SOFT_START == 0)
    {
        //return (30000/feedrate);
        return (DELAY_FACTOR/feedrate);
    }
    else
    {
        //calculate delay
        //if (steps < depth*2)            //total move length is too short for full warmup/cooldown
        if (steps < (depth<<1))            //total move length is too short for full warmup/cooldown
        {
            //if (i < steps/2) {x = i;}
            if (i < (steps>>1)) {x = i;}
            else {x = steps-i;}
        }
        else if (i < depth)            //warmup delay
        {
            x = i;
        }
        else if (i > steps-depth)      //cooldown delay
        {
            x = steps - i;
        }
        //else {return (30000/feedrate);}           //normal speed
        else {return (DELAY_FACTOR/feedrate);}           //normal speed
    }
    
    //determine the max acceleration
    acc_max = (feedrate - init_feedrate);
    //acc_max /= (depth/2);
    acc_max /= (depth>>1);
    
    //using all computed values and given parameters, calculate the feedrate
    //if (i <= depth/2)
    if (i <= (depth>>1))
    {
        feedrate = acc_max*x*x / depth;
    }
    else
    {
        //feedrate = acc_max*depth/2;
        feedrate = acc_max*depth>>1;
        feedrate -= ((acc_max/depth) * (100-x)*(100-x));
    }
    feedrate += init_feedrate;
    
    
    //calculate the desired halfdelay
    //halfdelay = 30000 / feedrate;
    halfdelay = DELAY_FACTOR / feedrate;
                
    return halfdelay;
}


/*******************************************************************************
 Function: homeAxis
 Description: Homes the given axis
 Notes: Moves the axis home rapidly, backs off, re-homes slowly.
 Fast/slow home speeds are defined in system.h
 Inputs: axis (the axis to be homed)
 Outputs: 
*******************************************************************************/
void homeAxis(char axis)
{
    moveToSwitch(axis, HOMESPEED_FAST);
    linearMove1Axis(axis, 5.0, HOMESPEED_SLOW);   //back away 5mm from the home position
    moveToSwitch(axis, HOMESPEED_SLOW);
}

/*******************************************************************************
 Function: moveToSwitch
 Description: Moves the given axis until it closes the limit switch
 Notes: May or may not end up using the PORTB interrupt
 Inputs: axis (Desired axis to home), feedrate (Travel speed, in mm/min)
 Outputs: 
*******************************************************************************/
void moveToSwitch(char axis, int speed)
{
    unsigned short delay;
    unsigned short limit;
    
    //disable all axes (active low)
    
    
    //enable the correct axis, and set its pos to zero
    if (axis == 'X')
    {
        //X_AXIS_EN_PIN = 0;
        //limit = X_LIMIT_PIN;
        limit = 1<<4;   //for now
        XPOS = 0;
    }
    else if (axis == 'Y')
    {
        //Y1_AXIS_EN_PIN = 0; Y2_AXIS_EN_PIN = 0;
        //limit = Y_LIMIT_PIN;
        limit = 1<<5;   //for now
        YPOS = 0;
    }
    /*else if (axis == 'Z')
    {
        //Z_AXIS_EN_PIN = 0;
        //limit = Z_LIMIT_PIN;
        limit = 1<<6;   //for now
    }*/
    
    //set all motors in reverse
    X_AXIS_DIREC_PIN = 0;
    Y1_AXIS_DIREC_PIN = 0;
    Y2_AXIS_DIREC_PIN = 0;
    Z_AXIS_DIREC_PIN = 0;
    
    delay_ms(1);
    
    //calculate delay for the given mode
    //if (mode == 0) {halfdelay = (60000/((unsigned short)(2*HOMESPEED_FAST)));}
    //else {halfdelay = (60000/((unsigned short)(2*HOMESPEED_SLOW)));}
    //halfdelay = (60000/((unsigned short)(2*speed)));
    delay = DELAY_FACTOR/speed;
    
    while(1)
    {
        //STEPPER_DRV_PIN = 1;
        if (axis == 'X')
        {
            X_AXIS_EN_PIN = 1;
            delay_us(delay);
            X_AXIS_EN_PIN = 0;
            delay_us(delay);
        }
        else if (axis == 'Y')
        {
            Y1_AXIS_EN_PIN = 1;
            Y2_AXIS_EN_PIN = 1;
            delay_us(delay);
            Y1_AXIS_EN_PIN = 0;
            Y2_AXIS_EN_PIN = 0;
            delay_us(delay);
        }
        
        if ((PORTB & limit) == 0) {break;}
    }
    
}