/*
 *  gamepad.c - Logitech Wireless USB Game Controller
 */

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhhid.h"
#include "usblib/host/usbhhidgamepad.h"
#include "drivers/display96x16x1.h"		// OLED display functions and #defines
#include "utils/ustdlib.h"				// usprintf()
#include "gamepad.h"


/* USB Host Controller Buffer Size */
#define HCD_MEMORY_SIZE         128

/* The size of the Gamepad's memory pool in bytes */
#define GAMEPAD_MEMORY_SIZE       128

/* The memory pool to provide to the Host controller driver */
unsigned char g_pHCDPool[HCD_MEMORY_SIZE];

/* The memory pool to provide to the Gamepad driver */
unsigned char g_pucBuffer[GAMEPAD_MEMORY_SIZE];

/* Declares the USB Events driver interface */
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);

/* Array of USB Drivers that will be used in this application
 * We need HID Class and Event Driver
 */
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
        &g_USBHIDClassDriver,
        &g_sUSBEventDriver
};

/* Number of elements in the g_ppHostClassDrivers array */
static const unsigned long g_ulNumHostClassDrivers =
        sizeof(g_ppHostClassDrivers) / sizeof(tUSBHostClassDriver *);

/* Global instance of the Gamepad */
static unsigned long g_ulGamepadInstance;


//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 t_u8 boundary.  In this application uDMA is only used for USB,
// so only the first 6 channels are needed.
//
//*****************************************************************************
tDMAControlTable g_sDMAControlTable[6];

//*****************************************************************************
//
// This is the generic callback from host stack.
//
// pvData is actually a pointer to a tEventInfo structure.
//
// This function will be called to inform the application when a USB event has
// occurred that is outside those related to the gamepad device.  At this
// point this is used to detect unsupported devices being inserted and removed.
// It is also used to inform the application when a power fault has occurred.
// This function is required when the g_USBGenericEventDriver is included in
// the host controller driver array that is passed in to the
// USBHCDRegisterDrivers() function.
//
//*****************************************************************************
void
USBHCDEvents(void *pvData)
{
    tEventInfo *pEventInfo;

    pEventInfo = (tEventInfo *)pvData;

    switch(pEventInfo->ulEvent)
    {
    /* New Device Detected */
    case USB_EVENT_CONNECTED:
    {
        /* Detects the initial enumeration of a basic USB Mouse.  This
         * is what the Logitech Unifying USB Adapter presents first.
         * When a paired device is found, the Mouse will be disconnected
         * and an HID Class "Other" Device will be presented.
         */
        if((USBHCDDevClass(pEventInfo->ulInstance, 0) == USB_CLASS_HID) &&
                (USBHCDDevProtocol(pEventInfo->ulInstance, 0) ==
                        USB_HID_PROTOCOL_MOUSE))
        {
            GamepadState = STATE_OTHER_DEVICE;
            break;
        }

        /* This is the Game Controller we are looking for */
        if((USBHCDDevClass(pEventInfo->ulInstance, 0) == USB_CLASS_HID) &&
                (USBHCDDevProtocol(pEventInfo->ulInstance, 0) ==
                        USB_HID_PROTOCOL_NONE))
        {
            GamepadState = STATE_GAMEPAD_INIT;
        }
        break;
    }

    /* Unsupported device detected.  This will occur if the D-X switch
     * on the Game Controller is in "X' mode (enumerates as a Class 0xFF).
     * This also occurs sometimes if no paired device is in range.
     */
    case USB_EVENT_UNKNOWN_CONNECTED:
    {
        GamepadState = STATE_UNKNOWN_DEVICE;
        break;
    }

    /* USB Device was disconnected */
    case USB_EVENT_DISCONNECTED:
    {
        GamepadState = STATE_NO_DEVICE;
        break;
    }

    /* Power Fault occurs */
    case USB_EVENT_POWER_FAULT:
    {
        GamepadState = STATE_NO_DEVICE;
        break;
    }

    default:
    {
        break;
    }
    }
}

//*****************************************************************************
//
// This is the callback from the USB HID gamepad handler.
//
// \param pvCBData is ignored by this function
// \param ulEvent is ignored by this function
// \param ulMsgParam is ignored by this function
// \param pvMsgData is a pointer to input report data
//
// This function will be called to inform the application when the gamepad
// has input data to provide and will update our global variables
//
// \return This function will return 0.
//
//*****************************************************************************
unsigned long
GamepadCallback(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgParam,
        void *pvMsgData)
{
    tUSBHGamepadReport *data;

    data = (tUSBHGamepadReport *)pvMsgData;

    GamepadButtons.buttonA = data->buttonA;
    GamepadButtons.buttonB = data->buttonB;
    GamepadButtons.buttonStart = data->start;
    GamepadButtons.dpad = data->dpad;

    GamepadUpdatedFlag = true;

    return(0);
}


/* Hardware initialization for Gamepad.  Call this with other init
 * functions before invoking the scheduler */
void vGamepadInit(void)
{
    //
    // Set the clocking to run from the PLL at 50 [MHz].
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //
    // Enable the GPIO pins Ethernet connector LEDs
    // Set the direction as output, and enable the GPIO pin for digital function.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3);

    //
    // Initially wait for device connection.
    //
    GamepadState = STATE_NO_DEVICE;

    //
    // Enable the uDMA controller and set up the control table base.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(g_sDMAControlTable);

    //
    // Set the USB pins to be controlled by the USB controller.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Enable Clocking to the USB controller.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);

    //
    // Set Host mode (USB MUX on evalbot PCB)
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_0);
    ROM_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_0,0);

    //
    // Set Power Enable sense pin
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA6_USB0EPEN);
    ROM_GPIOPinTypeUSBDigital(GPIO_PORTA_BASE, GPIO_PIN_6);

    //
    // Register the host class drivers.
    //
    USBHCDRegisterDrivers(0, g_ppHostClassDrivers, g_ulNumHostClassDrivers);

    //
    // Open an instance of the gamepad driver.  The mouse does not need
    // to be present at this time, this just saves a place for it and allows
    // the applications to be notified when a gamepad is present.
    g_ulGamepadInstance =
            USBHGamepadOpen(GamepadCallback, g_pucBuffer, GAMEPAD_MEMORY_SIZE);

    //
    // Initialize the power configuration.  This sets the power enable signal
    // to be active high and does not enable the power fault.
    //
    USBHCDPowerConfigInit(0, USBHCD_VBUS_AUTO_HIGH | USBHCD_VBUS_FILTER);

    /* Initializes USB controller for Host operation */
    USBHCDInit(0, g_pHCDPool, HCD_MEMORY_SIZE);

    /* Initializes OLED Display */
    Display96x16x1Init(true);
    Display96x16x1DisplayOn();
    Display96x16x1Clear();
}


/* Use scheduler to call this function.  Must be repeatedly called */
void vGamepadUpdate()
{
    char oledStr[15];

    /* Polls USB Device for any new reports */
    USBHCDMain();


    switch(GamepadState)
    {

    case STATE_NO_DEVICE:
    {
        Display96x16x1StringDrawCentered("Connect USB", 0, true);
        Display96x16x1StringDrawCentered("Adapter", 1, true);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_2 | GPIO_PIN_3);
        GamepadState = STATE_USB_IDLE;
        break;
    }

    case STATE_OTHER_DEVICE:
    {
        Display96x16x1StringDrawCentered("USB Adapter On", 0, true);
        Display96x16x1StringDrawCentered("Press a Button", 1, true);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_2);
        GamepadState = STATE_USB_IDLE;
        break;
    }

    case STATE_UNKNOWN_DEVICE:
    {
        Display96x16x1StringDrawCentered("Unknown Device", 0, true);
        Display96x16x1StringDrawCentered("Check DX Switch", 1, true);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_2);
        GamepadState = STATE_USB_IDLE;
        break;
    }

    case STATE_GAMEPAD_INIT:
    {
        USBHGamepadInit(g_ulGamepadInstance);
        GamepadState = STATE_GAMEPAD_CONNECTED;
        ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 1);
        Display96x16x1StringDrawCentered("Gamepad Found", 0, true);
        Display96x16x1StringDrawCentered("Press Start", 1, true);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0);
        break;
    }

    case STATE_GAMEPAD_CONNECTED:
    {
        if(GamepadButtons.buttonStart)
        {
            Display96x16x1Clear();
            GamepadState = STATE_GAMEPLAY;
        }
        break;
    }

    case STATE_GAMEPLAY:
    {
        if(GamepadUpdatedFlag)
        {
            /* Displays controller button status.  We can
             * comment this out when we're done developing */
            usprintf(oledStr, "%s%s %s %s %s",
                     (GamepadButtons.dpad==6)?"<":" ",
                     (GamepadButtons.dpad==2)?">":" ",
                     GamepadButtons.buttonStart?"START":"     ",
                     GamepadButtons.buttonA?"A":" ",
                     GamepadButtons.buttonB?"B":" ");
            Display96x16x1StringDrawCentered(oledStr, 1, false);
            GamepadUpdatedFlag = false;
        }
        break;
    }

    case STATE_POWER_FAULT:
    {
        Display96x16x1StringDrawCentered("USB POWER", 0, true);
        Display96x16x1StringDrawCentered("FAULT!", 1, true);
        break;
    }

    default:
    {
        break;
    }

    }
}
