//****************************************************************************
//
// main.c - Hello World example for EvalBot
//
//****************************************************************************

#include "inc/lm3s9b92.h"


//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
    UARTprintf("DriverLib assertion error in %s@%d\n", pcFilename, ulLine);
    for(;;)
    {
    }
}
#endif

/**
 * Main function
 * 		This startup code expects you to add a main function.  This is the
 * 		entry point of your program
 */



#include "drivers/io.h"
#include "drivers/motor.h"
#include "rangefinder.h"
//#include "utils/ustdlib.c"

int main ()
{
    Display96x16x1Init(1);
    Display96x16x1DisplayOn();

	// initialize the range finder
    initRangeFinder();

    char statusStr[96];

	// make measurements
	while(1)
	{
		usprintf(statusStr,"AIN0 = %4d\r",measureBoxDistance());
	    Display96x16x1StringDrawCentered( "Mario Kart \0",0,1);
	    Display96x16x1StringDrawCentered( statusStr,1,1);

	    // create a delay
	    SysCtlDelay(SysCtlClockGet() / 6);
	}
	//MotorStop(LEFT_SIDE); MotorStop(RIGHT_SIDE);
    return 0;
}

