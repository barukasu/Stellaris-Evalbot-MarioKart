/*
 * gamepad.h
 *
 *  Created on: May 3, 2014
 *      Author: ece
 */

#ifndef GAMEPAD_H_
#define GAMEPAD_H_

#include "inc/hw_types.h"

/* Struct that contains the Gamepad controls we're interested in */
typedef struct
{
    unsigned char dpad:4;
    unsigned char buttonA:1;
    unsigned char buttonB:1;
    unsigned char buttonStart:1;
}
tButtonState;

/* USB States for the Gamepad */
typedef enum
{
    /* Idle with no Gamepad */
    STATE_USB_IDLE,

    /* Device not connected */
    STATE_NO_DEVICE,

    /* Logitech "Mouse" detected, Gamepad not enumerated yet */
    STATE_OTHER_DEVICE,

    /* Gamepad detected, needs initialization */
    STATE_GAMEPAD_INIT,

    /* Gamepad connected and initialized, waiting for start button */
    STATE_GAMEPAD_CONNECTED,

    /* Start button pressed on Gamepad */
    STATE_GAMEPLAY,

    /* Unknown Device */
    STATE_UNKNOWN_DEVICE,

    /* Power Fault */
    STATE_POWER_FAULT
}
tUSBState;

/* Global that contains the current connection state of the gamepad */
tUSBState GamepadState;

/* Global that contains the current state of controller buttons. As
 * long as vGamepadUpdate() is scheduled to call repeatedly, this
 * variable can be used to access the current state of the
 * controller buttons. */
tButtonState GamepadButtons;

/* Global flag to inform application that button state has changed.
 * The last function that checks this is responsible for resetting */
tBoolean GamepadUpdatedFlag;

/* Hardware initialization for Gamepad.  Call this with other init
 * functions before invoking the scheduler */
void vGamepadInit();

/* Use scheduler to call this function.  Must be repeatedly called */
void vGamepadUpdate();

#endif /* GAMEPAD_H_ */
