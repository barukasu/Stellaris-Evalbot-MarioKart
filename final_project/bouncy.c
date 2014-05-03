/*
 * bouncy.c
 *
 *  Created on: May 3, 2014
 *      Author: ece
 */

#include "inc/hw_types.h"               // tBoolean type definition
#include "drivers/display96x16x1.h"     // OLED display functions and #defines
#include "bouncy.h"

/* Hardware initialization for Bouncy.  Call this with other init
 * functions before invoking the scheduler */
void vBouncyInit()
{
    /* Initializes OLED Display */
    Display96x16x1Init(true);
    Display96x16x1DisplayOn();
    Display96x16x1Clear();
}

/* Bounces a pixel on left side of OLED display.  Must be repeatedly
 * called by a scheduler to continue animation.  Good way to check
 * that the scheduler is not stuck
 */
void vBouncyUpdate()
{

#define MIN_Y   0
#define MAX_Y   15
#define UP      0
#define DOWN    1

    /* Bitmap for OLED display */
    static unsigned char image[2] = { 0 };

    /* Position and direction state for pixel */
    static unsigned char y_pos = 0;
    static unsigned char y_dir = DOWN;

    /* Calculates the new position of the dot */
    if (y_dir == DOWN)
    {
        y_pos++;
        if (y_pos > MAX_Y)
        {
            y_pos = MAX_Y - 1;
            y_dir = UP;
        }
    } else
    {
        y_pos--;
        if (y_pos == MIN_Y)
        {
            y_pos = MIN_Y + 1;
            y_dir = DOWN;
        }
    }

    /* Puts a dot in the image */
    image[(y_pos / 8)] = (1 << (y_pos % 8) | (1 << ((y_pos % 8) + 1)));

    Display96x16x1ImageDraw((const unsigned char*) image, 0, 0, 1, 2);

    /* Erases the dot from the array (so we don't leave a trail) */
    image[(y_pos / 8)] = 0;
}
