#******************************************************************************
#
# Makefile - Rules for building the EvalBot Sound example.
#
# Copyright (c) 2011 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 7243 of the Stellaris Firmware Development Package.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=LM3S9B96

#
# The base directory for StellarisWare.
#
ROOT=.

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

VERBOSE=1

#
# Where to find source files that do not live in this directory.
#
VPATH =./drivers
VPATH+=./utils
VPATH+=./driverlib

#
# Where to find header files that do not live in the source directory.
#
IPATH=.

# The default rule, which causes the EvalBot Sound example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/mariokart.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the EvalBot Sound example.
#

${COMPILER}/mariokart.axf: ${COMPILER}/main.o
${COMPILER}/mariokart.axf: ${COMPILER}/dac.o
${COMPILER}/mariokart.axf: ${COMPILER}/uartstdio.o
${COMPILER}/mariokart.axf: ${COMPILER}/ustdlib.o
${COMPILER}/mariokart.axf: ${COMPILER}/display96x16x1.o
${COMPILER}/mariokart.axf: ${COMPILER}/gpio.o
${COMPILER}/mariokart.axf: ${COMPILER}/ethernet.o
${COMPILER}/mariokart.axf: ${COMPILER}/dac.o
${COMPILER}/mariokart.axf: ${COMPILER}/io.o
${COMPILER}/mariokart.axf: ${COMPILER}/sensors.o
${COMPILER}/mariokart.axf: ${COMPILER}/motor.o
${COMPILER}/mariokart.axf: ${COMPILER}/rangefinder.o
${COMPILER}/mariokart.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/mariokart.axf: ${ROOT}/driverlib/gcc-cm3/libdriver-cm3.a
${COMPILER}/mariokart.axf: mariokart.ld
SCATTERgcc_mariokart=mariokart.ld
ENTRY_sound_demo=ResetISR

CFLAGSgcc=-DTARGET_IS_TEMPEST_RB1 -DUART_BUFFERED

CFLAGSgcc+=-DUART_BUFFERED -g
#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
