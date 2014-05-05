/*
 *  	Microprocessors for Robotics
 *
 *  	Lab 1
 *      Author: Biruh Tesfaye
 *
 *      Goal: to modify registers and enable PWM and LEDs
 */

#ifndef LABS_RangeFinder_
#define LABS_RangeFinder_


#include "inc/hw_types.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ethernet.h"
#include "driverlib/ethernet.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/rom.h"
#include "driverlib/udma.h"
#include "drivers/display96x16x1.h"
#include "drivers/io.h"
#include "drivers/sensors.h"
#include "drivers/sound.h"
#include "drivers/wav.h"
#include "driverlib/adc.h"

#include "inc/lm3s9b92.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"


#define DISTANCE_COUNT_TO_INCHES 300



// initialize the range finder
void initRangeFinder();
// return the current range measurements
unsigned long measureBoxDistance();
// copy the current range measurements into the global variable
void vBoxDistanceUpdate(unsigned long *currentMeasuredDistance);

#endif
