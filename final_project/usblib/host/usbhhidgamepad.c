//*****************************************************************************
//
// usbhhidgamepad.c - This file holds the application interfaces for the
//					  Logitech F710 USB Wireless Game Controller
//					  The controller must be in "D - DirectControl" legacy
//					  mode.  This will enumerate as a USB HID Class device.
//
//*****************************************************************************

#include "inc/hw_types.h"
#include "usblib/usblib.h"
#include "usblib/host/usbhost.h"
#include "usblib/usbhid.h"
#include "usblib/host/usbhhid.h"
#include "usblib/host/usbhhidgamepad.h"


//*****************************************************************************
//
// Prototypes for local functions.
//
//*****************************************************************************
static unsigned long USBHGamepadCallback(void *pvCBData,
                                       unsigned long ulEvent,
                                       unsigned long ulMsgParam,
                                       void *pvMsgData);

//*****************************************************************************
//
// The size of a USB gamepad report.
//
//*****************************************************************************
#define USBHGP_REPORT_SIZE      8

//*****************************************************************************
//
// These are the flags for the tUSBHGamepad.ulHIDFlags member variable.
//
//*****************************************************************************
#define USBHGP_DEVICE_PRESENT   0x00000001

//*****************************************************************************
//
// This is the structure definition for a gamepad device instance.
//
//*****************************************************************************
typedef struct
{
    //
    // Global flags for an instance of a gamepad.
    //
    unsigned long ulHIDFlags;

    //
    // The applications registered callback.
    //
    tUSBCallback pfnCallback;

    //
    // The current state of the buttons.
    //
    unsigned char ucButtons;

    //
    // This is a local buffer to hold the current HID report that comes up
    // from the HID driver layer.
    //
    unsigned char pucBuffer[USBHGP_REPORT_SIZE];

    //
    // Heap data for the gamepad currently used to read the HID Report
    // Descriptor.
    //
    unsigned char *pucHeap;

    //
    // Size of the heap in bytes.
    //
    unsigned long ulHeapSize;

    //
    // This is the instance value for the HID device that will be used for the
    // gamepad.
    //
    unsigned long ulGamepadInstance;
}
tUSBHGamepad;




//*****************************************************************************
//
// This is the per instance information for a mouse device.
//
//*****************************************************************************
static tUSBHGamepad g_sUSBHGamepad =
{
    0
};

//*****************************************************************************
//
//! This function is used open an instance of a gamepad.
//!
//! \param pfnCallback is the callback function to call when new events occur
//! with the gamepad returned.
//! \param pucBuffer is the memory used by the driver to interact with the
//! USB gamepad.
//! \param ulSize is the size of the buffer provided by \e pucBuffer.
//!
//! This function is used to open an instance of the gamepad.  The value
//! returned from this function should be used as the instance identifier for
//! all other USBHGamepad calls.  The \e pucBuffer memory buffer is used to
//! access the gamepad.  The buffer size required is at least enough to hold
//! a normal report descriptor for the device.
//!
//! \return Returns the instance identifier for the gamepad that is attached.
//! If there is no gamepad present this will return 0.
//
//*****************************************************************************
unsigned long
USBHGamepadOpen(tUSBCallback pfnCallback, unsigned char *pucBuffer,
              unsigned long ulSize)
{
    //
    // Save the callback and data pointers.
    //
    g_sUSBHGamepad.pfnCallback = pfnCallback;

    //
    // Save the instance pointer for the HID device that was opened.
    //
    g_sUSBHGamepad.ulGamepadInstance = USBHHIDOpen(USBH_HID_DEV_NONE,
                                               USBHGamepadCallback,
                                               (unsigned long)&g_sUSBHGamepad);

    //
    // Save the heap buffer and size.
    //
    g_sUSBHGamepad.pucHeap = pucBuffer;
    g_sUSBHGamepad.ulHeapSize = ulSize;

    return((unsigned long)&g_sUSBHGamepad);
}

//*****************************************************************************
//
//! This function is used close an instance of a gamepad.
//!
//! \param ulInstance is the instance value for this gamepad.
//!
//! This function is used to close an instance of the gamepad that was opened
//! with a call to USBHGamepadOpen().  The \e ulInstance value is the value
//! that was returned when the application called USBHGamepadOpen().
//!
//! \return Returns 0.
//
//*****************************************************************************
unsigned long
USBHGamepadClose(unsigned long ulInstance)
{
    tUSBHGamepad *pUSBHGamepad;

    //
    // Recover the pointer to the instance data.
    //
    pUSBHGamepad = (tUSBHGamepad *)ulInstance;

    //
    // Reset the callback to null.
    //
    pUSBHGamepad->pfnCallback = 0;

    //
    // Call the HID driver layer to close out this instance.
    //
    USBHHIDClose(pUSBHGamepad->ulGamepadInstance);

    return(0);
}

//*****************************************************************************
//
//! This function is used to initialize a gamepad interface after a gamepad has
//! been detected.
//!
//! \param ulInstance is the instance value for this gamepad.
//!
//! This function should be called after receiving a \b USB_EVENT_CONNECTED
//! event in the callback function provided by USBHGamepadOpen(), however it
//! should only be called outside of the callback function.  This will
//! initialize the gamepad interface and determine how it reports events to the
//! USB host controller.  The \e ulInstance value is the value that was
//! returned when the application called USBHGamepadOpen().  This function only
//! needs to be called once per connection event but it should be called every
//! time a \b USB_EVENT_CONNECTED event occurs.
//!
//! \return Non-zero values should be assumed to indicate an error condition.
//
//*****************************************************************************
unsigned long
USBHGamepadInit(unsigned long ulInstance)
{
    tUSBHGamepad *pUSBHGamepad;

    //
    // Recover the pointer to the instance data.
    //
    pUSBHGamepad = (tUSBHGamepad *)ulInstance;

    //
    // Set the initial rate to only update on mouse state changes.
    //
    USBHHIDSetIdle(pUSBHGamepad->ulGamepadInstance, 0, 0);

    //
    // Read out the Report Descriptor from the mouse and parse it for
    // the format of the reports coming back from the mouse.
    //
    USBHHIDGetReportDescriptor(pUSBHGamepad->ulGamepadInstance,
                               pUSBHGamepad->pucHeap,
                               pUSBHGamepad->ulHeapSize);

    return(0);
}

//*****************************************************************************
//
// This function handles updating the state of the Gamepad
//
// \param pUSBHGamepad is the pointer to an instance of the gamepad data.
//
// This function simply passes the Gamepad report bytes to the application.
// It is up to the application to interpret these bytes as needed.
//
// \return None.
//
//*****************************************************************************
static void
UpdateGamepadState(tUSBHGamepad *pUSBHGamepad)
{
    pUSBHGamepad->pfnCallback(0,
                            USBH_EVENT_HID_GP_INPUT,
                            0,
                            pUSBHGamepad->pucBuffer);
}

//*****************************************************************************
//
//! This function handles event callbacks from the USB HID driver layer.
//!
//! \param pvCBData is the pointer that was passed in to the USBHHIDOpen()
//! call.
//! \param ulEvent is the event that has been passed up from the HID driver.
//! \param ulMsgParam has meaning related to the \e ulEvent that occurred.
//! \param pvMsgData has meaning related to the \e ulEvent that occurred.
//!
//! This function will receive all event updates from the HID driver layer.
//! The gamepad driver itself will mostly be concerned with report callbacks
//! from the HID driver layer and parsing them into control inputs for the
//! application that has registered for callbacks with the USBHGamepadOpen()
//! call.
//!
//! \return Non-zero values should be assumed to indicate an error condition.
//
//*****************************************************************************
unsigned long
USBHGamepadCallback(void *pvCBData, unsigned long ulEvent,
                  unsigned long ulMsgParam, void *pvMsgData)
{
    tUSBHGamepad *pUSBHGamepad;

    //
    // Recover the pointer to the instance data.
    //
    pUSBHGamepad = (tUSBHGamepad *)pvCBData;

    switch(ulEvent)
    {
        //
        // New gamepad has been connected so notify the application.
        //
        case USB_EVENT_CONNECTED:
        {
            //
            // Remember that a mouse is present.
            //
            pUSBHGamepad->ulHIDFlags |= USBHGP_DEVICE_PRESENT;

            //
            // Notify the application that a new mouse was connected.
            //
            pUSBHGamepad->pfnCallback(0, ulEvent, ulMsgParam, pvMsgData);

            break;
        }
        case USB_EVENT_DISCONNECTED:
        {
            //
            // No gamepad is present.
            //
            pUSBHGamepad->ulHIDFlags &= ~USBHGP_DEVICE_PRESENT;

            //
            // Notify the application that the mouse was disconnected.
            //
            pUSBHGamepad->pfnCallback(0, ulEvent, ulMsgParam, pvMsgData);

            break;
        }
        case USB_EVENT_RX_AVAILABLE:
        {
            //
            // New gamepad report structure was received.
            //
            USBHHIDGetReport(pUSBHGamepad->ulGamepadInstance, 0,
                             pUSBHGamepad->pucBuffer,
                             USBHGP_REPORT_SIZE);

            //
            // Update the current state of the mouse and notify the application
            // of any changes.
            //
            UpdateGamepadState(pUSBHGamepad);

            break;
        }
    }
    return(0);
}

