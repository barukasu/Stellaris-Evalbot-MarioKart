//*****************************************************************************
//
// sounds.h - Header file for sound_demo audio clips
//
// Copyright (c) 2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 7243 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

extern const unsigned char g_ucSoundWav1[];
extern const unsigned char g_ucSoundWav2[];
extern const unsigned char g_ucSoundWav3[];
extern const unsigned char g_ucSoundWav4[];
extern const unsigned char g_ucSoundWav5[];


extern const unsigned long g_ulSoundWav1Len;
extern const unsigned long g_ulSoundWav2Len;
extern const unsigned long g_ulSoundWav3Len;
extern const unsigned long g_ulSoundWav4Len;
extern const unsigned long g_ulSoundWav5Len;

extern const unsigned char g_ucSoundWav6[];
extern const unsigned char g_ucSoundWav7[];
extern const unsigned char g_ucSoundWav8[];
extern const unsigned char g_ucSoundWav9[];
extern const unsigned char g_ucSoundWav10[];

extern const unsigned long g_ulSoundWav6Len;
extern const unsigned long g_ulSoundWav7Len;
extern const unsigned long g_ulSoundWav8Len;
extern const unsigned long g_ulSoundWav9Len;
extern const unsigned long g_ulSoundWav10Len;
/**/

typedef struct
{
    const unsigned char *pucWav;
    const char * pcName;
} tWaveClip;

static const tWaveClip sWaveClips[] =
{
    { g_ucSoundWav1, "Mario" },
    { g_ucSoundWav2, "Boing" },
    { g_ucSoundWav3, "Crash" },
    { g_ucSoundWav4, "Horn" },
    { g_ucSoundWav5, "Note" },
    { g_ucSoundWav6, "Note" },
    { g_ucSoundWav7, "Note" },
    { g_ucSoundWav8, "Note" },
    { g_ucSoundWav9, "Note" },
    { g_ucSoundWav10, "Note" },
};
//    { g_ucSoundWav6, "huh?" },
//    { g_ucSoundWav7, "mk64" },
//    { g_ucSoundWav8, "crash" },
//    { g_ucSoundWav9, "countdown" },

#define NUM_WAVES (sizeof(sWaveClips) / sizeof(tWaveClip))
