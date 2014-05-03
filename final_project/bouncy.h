/*
 * bouncy.h
 *
 *  Created on: May 3, 2014
 *      Author: ece
 */

#ifndef BOUNCY_H_
#define BOUNCY_H_

/* Hardware initialization for Bouncy.  Call this with other init
 * functions before invoking the scheduler */
void vBouncyInit();

/* Bounces a pixel on left side of OLED display.  Must be repeatedly
 * called by a scheduler to continue animation.  Good way to check
 * that the scheduler is not stuck */
void vBouncyUpdate();


#endif /* BOUNCY_H_ */
