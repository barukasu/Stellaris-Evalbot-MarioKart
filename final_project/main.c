/*****************************************************************************
 Skeleton code for cooperative scheduler based application
 - Includes four example "threads"
 -- Function to initiate a sound if bumper was hit
 -- Function to continue playing sound
 -- OLED Bouncy Pixel to monitor scheduling
 -- USB Gamepad Polling and Update
 *****************************************************************************/
#include "inc/hw_types.h"               // tBoolean type definition
#include "inc/hw_memmap.h"              // Memory mapped I/O address #defines
#include "driverlib/sysctl.h"           // SysCtlPeripheralEnable()
#include "driverlib/gpio.h"             // GPIO functions and #defines
#include "drivers/sensors.h"            // BumperSensorsInit
#include "utils/scheduler.h"            // scheduler
#include "driverlib/interrupt.h"        // Interrupt functions
#include "driverlib/udma.h"             // DMA required for sound/USB
#include "driverlib/rom.h"              // ROM_SysStlClockSet()
#include "driverlib/timer.h"            // Timer functions and #defines
#include "drivers/sound.h"              // Sound functions
#include "drivers/wav.h"                // Wave functions
#include "powerup_wav.h"                // Wave encoded as a C array

#include "gamepad.h"                    // Gamepad Functions
#include "bouncy.h"                     // Bouncy Thing

/*****************************************************************************
 *  PREPROCESSOR DEFINES
 *****************************************************************************/
#define TICKS_PER_SECOND    100

/*****************************************************************************
 *  FUNCTION PROTOTYPES
 *****************************************************************************/
static void vInit(void);
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
        { vContinueSound, 0, 5, 0, false },
        { vCheckBumperFlag, 0, 3, 0, true },
        { vBouncyUpdate, 0, 2, 0, true },
        { vGamepadUpdate, 0, 1, 0, true }
};

/* Number of tasks in g_psSchedulerTable */
unsigned long g_ulSchedulerNumTasks = sizeof(g_psSchedulerTable)
                / sizeof(tSchedulerTask);

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

    while (1)
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
    ROM_SysCtlClockSet(
            SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
            | SYSCTL_XTAL_16MHZ);

    /* Initializes Bumpers with Interrupt */
    BumpSensorsInit();
    GPIOPinIntEnable(GPIO_PORTE_BASE, (1 << 0) | (1 << 1));
    GPIOIntTypeSet(GPIO_PORTE_BASE, (1 << 0) | (1 << 1), GPIO_FALLING_EDGE);
    IntEnable(20);

    /* Initializes timer, required for scheduler */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER_UP);
    TimerEnable(TIMER0_BASE, TIMER_A);

    SoundInit();
    SoundVolumeSet(75);

    IntMasterEnable();

    /* Hardware initialization for Bouncy related peripherals */
    vBouncyInit();

    /* Hardware initialization for Gamepad related peripherals */
    vGamepadInit();

    return;
}


/* If Bumper Flag is true, then start playing sound */
static void vCheckBumperFlag(void *pvParam)
{
    if ((bBumperPressed) && (!bWavPlaying))
    {
        WaveOpen((unsigned long *) g_ucPowerUpWav, &xWaveHeader);

        /* Start Playing WAV */
        WavePlayStart(&xWaveHeader);

        /* Re-enable vContinueSound() */
        SchedulerTaskEnable(0, false);

        /* Reset Bumper Flag */
        bBumperPressed = false;

        bWavPlaying = true;
    }
    else
    {
        /* Need this to prevent an extra bumper press from
         * being queued while WAV is playing.  We can get rid of this
         * if we add debounce. */
        bBumperPressed = false;
    }
}

/* Required to keep the wave sound playing.  Does not complain
 * if called too often or after the WAV is complete. */
static void vContinueSound(void *pvParam)
{
    if (WavePlayContinue(&xWaveHeader))
    {
        /* If WavePlayContinue() returns true, then playback is
         * complete...disable this task and clear flag
         */
        SchedulerTaskDisable(0);
        bWavPlaying = false;
    }
}

/*****************************************************************************
 *  INTERRUPT SERVICE ROUTINES
 *****************************************************************************/
void ISR_Bumper(void)
{
    // TODO: Doesn't differentiate between L or R bumper
    // TODO: We should probably add debouncing

    /* Clears Interrupt */
    GPIOPinIntClear(GPIO_PORTE_BASE, (1 << 0) | (1 << 1));

    bBumperPressed = true;

    return;
}
