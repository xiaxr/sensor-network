#pragma once
/*
Interrupts functions extruded from wiringPi library by Oitzu.

wiringPi Copyright (c) 2012 Gordon Henderson
https://projects.drogon.net/raspberry-pi/wiringpi
wiringPi is free software: GNU Lesser General Public License
see <http://www.gnu.org/licenses/>
*/

#include "RF24_arch_config.h"

#define INT_EDGE_SETUP 0
#define INT_EDGE_FALLING 1
#define INT_EDGE_RISING 2
#define INT_EDGE_BOTH 3

/*
 * interruptHandler:
 *      This is a thread and gets started to wait for the interrupt we're
 *      hoping to catch. It will call the user-function when the interrupt
 *      fires.
 *********************************************************************************
 */
void *interruptHandler(void *arg);

/*
 * waitForInterrupt:
 *      Pi Specific.
 *      Wait for Interrupt on a GPIO pin.
 *      This is actually done via the /sys/class/gpio interface regardless of
 *      the wiringPi access mode in-use. Maybe sometime it might get a better
 *      way for a bit more efficiency.
 *********************************************************************************
 */
int waitForInterrupt(int pin, int mS);

/*
 * piHiPri:
 *      Attempt to set a high priority schedulling for the running program
 *********************************************************************************
 */
int piHiPri(const int pri);

/*
 * attachInterrupt (Original: wiringPiISR):
 *      Pi Specific.
 *      Take the details and create an interrupt handler that will do a call-
 *      back to the user supplied function.
 *********************************************************************************
 */
int attachInterrupt(int pin, int mode, void (*function)(void));

/*
 * detachInterrupt:
 *      Pi Specific detachInterrupt.
 *      Will cancel the interrupt thread, close the filehandle and
 *		setting wiringPi back to 'none' mode.
 *********************************************************************************
 */
int detachInterrupt(int pin);

void rfNoInterrupts();

void rfInterrupts();
