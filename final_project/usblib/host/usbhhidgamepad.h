//*****************************************************************************
//
// usbhhidgamepad.h - This file holds the application interfaces for USB
// gamepad devices.
// 
//*****************************************************************************

#ifndef __USBHHIDGAMEPAD_H__
#define __USBHHIDGAMEPAD_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

/* A structure type that contains all the data provided by the
 * controller concerning the state of its inputs
 */
typedef struct
{
	unsigned char deviceID;
	unsigned char leftX;
	unsigned char leftY;
	unsigned char rightX;
	unsigned char rightY;
	unsigned char dpad : 4;
	unsigned char buttonX : 1;
	unsigned char buttonA : 1;
	unsigned char buttonB : 1;
	unsigned char buttonY : 1;
	unsigned char leftBump : 1;
	unsigned char rightBump : 1;
	unsigned char leftTrig : 1;
	unsigned char rightTrig : 1;
	unsigned char back : 1;
	unsigned char start : 1;
	unsigned char lStick : 1;
	unsigned char rStick : 1;
	unsigned char mode;
}
tUSBHGamepadReport;

typedef enum
{
	UP,
	UP_RIGHT,
	RIGHT,
	DOWN_RIGHT,
	DOWN,
	DOWN_LEFT,
	LEFT,
	UP_LEFT,
	NONE
}
tUSBHGamepadDirectionPad;

extern unsigned long USBHGamepadOpen(tUSBCallback pfnCallback,
                                   unsigned char *pucBuffer,
                                   unsigned long ulBufferSize);
extern unsigned long USBHGamepadClose(unsigned long ulInstance);
extern unsigned long USBHGamepadInit(unsigned long ulInstance);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
