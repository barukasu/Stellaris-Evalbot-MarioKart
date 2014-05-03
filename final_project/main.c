/*****************************************************************************
 Skeleton code for cooperative scheduler based application
 - Includes two example "threads"
 	 -- Function to Play a sound if bumper was hit
 	 -- WavPlayContinue() caller
 	 -- OLED Bouncy Ball thing
 *****************************************************************************/
#include "inc/hw_types.h"				// tBoolean type definition
#include "inc/hw_memmap.h"				// Memory mapped I/O address #defines
#include "driverlib/sysctl.h"			// SysCtlPeripheralEnable()
#include "drivers/display96x16x1.h"		// OLED display functions and #defines
#include "driverlib/gpio.h"				// GPIO functions and #defines
#include "drivers/sensors.h"			// BumperSensorsInit
#include "utils/scheduler.h"			// scheduler
#include "driverlib/interrupt.h"		// Interrupt functions
#include "driverlib/udma.h"				// DMA required for sound/USB
#include "driverlib/rom.h"				// ROM_SysStlClockSet()
#include "driverlib/timer.h"			// Timer functions and #defines
#include "drivers/sound.h"				// Sound functions
#include "drivers/wav.h"				// Wave functions
#include "powerup_wav.h"				// Wave encoded as a C array


/*****************************************************************************
 *  PREPROCESSOR DEFINES
 *****************************************************************************/
#define TICKS_PER_SECOND	100


/*****************************************************************************
 *  FUNCTION PROTOTYPES
 *****************************************************************************/
static void vInit(void);
static void vUpdateDisplay(void *pvParam);
static void vCheckBumperFlag(void *pvParam);
static void vContinueSound(void *pvParam);


/*****************************************************************************
 *  GLOBAL VARIABLES
 *****************************************************************************/
/* All functions that will need to be polled must be included here.  Functions
 * can be enabled/disabled at runtime, but must be listed here.  See
 * scheduler.h for info on fields.
 */
tSchedulerTask g_psSchedulerTable[] =
{
		{vContinueSound, 0, 5, 0, false},
		{vCheckBumperFlag, 0, 3, 0, true},
		{vUpdateDisplay, 0, 2, 0, true}
};

/* Number of tasks in g_psSchedulerTable */
unsigned long g_ulSchedulerNumTasks = sizeof(g_psSchedulerTable) /
		sizeof(tSchedulerTask);

/* Required for sound and USB ... might need to adjust this when both used */
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));

/* Global WAV header */
tWaveHeader xWaveHeader;

/* Global Flag for Bumper Pressed */
volatile tBoolean bBumperPressed = false;

/* Global Flag for WAV currently playing */
tBoolean bWavPlaying = false;


/*****************************************************************************
 *  FUNCTION IMPLEMENTATIONS
 *****************************************************************************/
int main(void)
{
	/* One-time initialization of hardware */
	vInit();

	SchedulerInit(TICKS_PER_SECOND);

	while(1)
	{
		SchedulerRun();
	}

	/* This should never happen */
	return 0;
}


/**
 * Performs one-time setup of peripherals
 */
static void vInit(void)
{
	/* Sets Clock Speed to 50MHz */
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);

	/* Initializes OLED Display */
	Display96x16x1Init(true);
	Display96x16x1DisplayOn();
	Display96x16x1Clear();

	/* Initializes Bumpers with Interrupt */
	BumpSensorsInit();
	GPIOPinIntEnable(GPIO_PORTE_BASE, (1 << 0)|(1 << 1));
	GPIOIntTypeSet(GPIO_PORTE_BASE, (1 << 0)|(1 << 1), GPIO_FALLING_EDGE);
	IntEnable(20);

	/* Initializes timer, required for scheduler */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER_UP);
	TimerEnable(TIMER0_BASE, TIMER_A);

	SoundInit();
	SoundVolumeSet(75);

	IntMasterEnable();
	return;
}

/* Bounces a little ball around the OLED screen.  Must be repeatedly
 * called by a scheduler to continue animation.  Good way to check
 * that the scheduler is not stuck
 */
static void vUpdateDisplay(void *pvParam)
{

#define MIN_X  	0
#define MIN_Y  	0
#define MAX_X  	94
#define MAX_Y  	14
#define UP		0
#define DOWN	1
#define LEFT	2
#define RIGHT	4

	/* Bitmap for OLED display */
	static unsigned char image[2][96] = { {0} };

	/* Position and direction state for "ball" */
	static unsigned char x_pos = 0;
	static unsigned char y_pos = 0;
	static unsigned char x_dir = RIGHT;
	static unsigned char y_dir = DOWN;

	/* Calculates the new position of the dot */
	if (x_dir == RIGHT)
	{
		x_pos++;
		if (x_pos > MAX_X)
		{
			x_pos = MAX_X - 1;
			x_dir = LEFT;
		}
	}
	else
	{
		x_pos--;
		if (x_pos == MIN_X)
		{
			x_pos = MIN_X + 1;
			x_dir = RIGHT;
		}
	}

	if (y_dir == DOWN)
	{
		y_pos++;
		if (y_pos > MAX_Y)
		{
			y_pos = MAX_Y - 1;
			y_dir = UP;
		}
	}
	else
	{
		y_pos--;
		if (y_pos == MIN_Y)
		{
			y_pos = MIN_Y + 1;
			y_dir = DOWN;
		}
	}

	/* Puts a dot in the image */
	image[(y_pos / 8)][x_pos] =
			(1 << (y_pos % 8) | (1 << ((y_pos % 8) + 1)));
	image[(y_pos / 8)][x_pos + 1] =
			(1 << (y_pos % 8) | (1 << ((y_pos % 8) + 1)));

	Display96x16x1ImageDraw((const unsigned char*)image,0,0,96,2);

	/* Erases the dot from the array (so we don't leave a trail)
	 * Comment these lines out if you want to draw lines. */
	image[(y_pos / 8)][x_pos] = 0;
	image[(y_pos / 8)][x_pos + 1] = 0;
}

/* If Bumper Flag is true, then start playing sound */
static void vCheckBumperFlag(void *pvParam)
{
	if (bBumperPressed)
	{
		WaveOpen((unsigned long *)g_ucPowerUpWav, &xWaveHeader);

		/* Start Playing WAV */
		WavePlayStart(&xWaveHeader);

		/* Re-enable vContinueSound() */
		SchedulerTaskEnable(0, false);

		/* Reset Bumper Flag */
		bBumperPressed = false;

		bWavPlaying = true;
	}
}

/* Required to keep the wave sound playing.  Does not complain
 * if called too often or after the WAV is complete. */
static void vContinueSound(void *pvParam)
{
	if (WavePlayContinue(&xWaveHeader))
	{
		/* If WavePlayContinue() returns true, then playback is
		 * complete...disable this task
		 */
		bWavPlaying = false;
		SchedulerTaskDisable(0);
	}
}


/*****************************************************************************
 *  INTERRUPT SERVICE ROUTINES
 *****************************************************************************/
void ISR_Bumper(void)
{
	/* Doesn't differentiate between L or R bumper */

	/* Clears Interrupt */
	GPIOPinIntClear(GPIO_PORTE_BASE, (1 << 0) | (1 << 1));

	if (!bWavPlaying) // Needs this or some debouncing, otherwise sound messes up
	{
		bBumperPressed = true;
	}

	return;
}
