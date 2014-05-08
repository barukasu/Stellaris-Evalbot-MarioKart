//#include "drivers/motor.h"
#include "DriverControl.h"
#include "gamepad.h"
#include "sounds.h"

//extern volatile tButtonState controller;
//extern volatile unsigned long dPadRight;
//extern volatile unsigned long aButtonDown;
//extern volatile unsigned long startButton;
extern volatile unsigned long currentMeasuredDistance;
//extern tWaveHeader xWaveHeader;
extern volatile unsigned long bBumperPressed;

void ActivateSpinout()
{
	if(motorControlState != Spinout)
	{
		//xWaveHeader = sWaveClips[2];
		bBumperPressed = false;
		motorControlState = Spinout;
		activeSpinoutTimer = 0;
		spinOutControlSubState = leg1;
		SpinOut(leg1);
	}
}
void DecactiveSpinOut()
{
	SpinOutMotorIsActive = 0;
}
void ActivateBoost()
{
	if(motorControlState != Boost && motorControlState != Spinout)
	{
		//xWaveHeader = sWaveClips[1];
		motorControlState = Boost;
		activeBoostTimer = 0;

	}
}
void DeactiveBoost()
{
	BoostMotorIsActive = 0;
}

/*********************************************
 * name: initDriveControl
 * parameters: N/A
 * description: This function initializes the 
 * motors for drive use
 *********************************************/

void initDriveControl()
{
	StartIsActive = 0;

	activeBoostTimer = 0;
	activeSpinoutTimer = 0;
	activeAccelerationTimer = 0;

	motorControlState = ControllerInput;
	spinOutControlSubState = leg1;
	currentLeftWheelSpeed = 0;
	currentRightWheelSpeed = 0;
	targetLeftWheelSpeed = 0;
	targetRightWheelSpeed = 0;

	// Initialize motor to 0 speed and with a forward direction
	MotorsInit();
	MotorSpeed(LEFT_SIDE, (0 << 8));
	MotorRun(LEFT_SIDE);
	MotorDir(LEFT_SIDE, FORWARD);
	MotorSpeed(RIGHT_SIDE, (0 << 8));
	MotorRun(RIGHT_SIDE);
	MotorDir(RIGHT_SIDE, FORWARD);
}

void stateController()
{
	// Forces to only enter once
	if(!StartIsActive)
	{
		if(GamepadButtons.buttonStart)
		{
			// Function to actually simulate a time out for later
			//countDown();
			//xWaveHeader = sWaveClips[0];
			StartIsActive = 1;
		}
	}
	if(StartIsActive)
	{
		if(bBumperPressed == 1)
			ActivateSpinout();
		if(currentMeasuredDistance > 400)
			ActivateBoost();

		stepMotor();
	}
}

void stepMotor()
{
	if(motorControlState == Spinout)
	{
		manageSpinOut();
	}
	else if(motorControlState == Boost)
	{
		manageBoost();
	}
	else if(motorControlState == ControllerInput)
	{
		manageController();
	}

}

void manageSpinOut()
{
	activeSpinoutTimer = activeSpinoutTimer + 1;
	// First leg of spinning out, turn clockwise
	if(spinOutControlSubState == leg1)
	{
		if(activeSpinoutTimer == (unsigned long)SPINOUT_TIME)
		{
			spinOutControlSubState = leg2;
			SpinOut(leg2);
			activeSpinoutTimer = 0;
		}
	}
	else if(spinOutControlSubState == leg2)
	{
		if(activeSpinoutTimer == SPINOUT_TIME)
		{
			//spinOutControlSubState = leg3;
			//SpinOut(leg3);
			//activeSpinoutTimer = 0;
			spinOutControlSubState = NoState;
						SpinOut(NoState);
						motorControlState = ControllerInput;
		}
	}
	else if(spinOutControlSubState == leg3)
	{
		if(activeSpinoutTimer == SPINOUT_TIME)
		{
			spinOutControlSubState = NoState;
			SpinOut(NoState);
			motorControlState = ControllerInput;
		}
	}
}

void SpinOut(spinOutState state)
{
	if(state == leg1 || state == leg3)
	{
		MotorSpeed(LEFT_SIDE, (30 << 8));
		MotorRun(LEFT_SIDE);
		MotorDir(LEFT_SIDE, FORWARD);
		MotorSpeed(RIGHT_SIDE, (30 << 8));
		MotorRun(RIGHT_SIDE);
		MotorDir(RIGHT_SIDE, REVERSE);
	}
	else if(state == leg2)
	{
		MotorSpeed(RIGHT_SIDE, (30 << 8));
		MotorRun(RIGHT_SIDE);
		MotorDir(RIGHT_SIDE, FORWARD);
		MotorSpeed(LEFT_SIDE, (30 << 8));
		MotorRun(LEFT_SIDE);
		MotorDir(LEFT_SIDE, REVERSE);
	}
	else if(state == NoState)
	{
		MotorSpeed(LEFT_SIDE, 0);
		MotorSpeed(RIGHT_SIDE, 0);
		MotorDir(RIGHT_SIDE, FORWARD);
		MotorDir(LEFT_SIDE, FORWARD);
	}
}


void manageBoost()
{
	activeBoostTimer = activeBoostTimer + 1;
	if(GamepadButtons.dpad == 6)
		MotorSpeed(LEFT_SIDE, (BOOST_SPEED * TURN_PERCENT));
	else
		MotorSpeed(LEFT_SIDE, BOOST_SPEED);
	if(GamepadButtons.dpad == 2)
		MotorSpeed(RIGHT_SIDE, (BOOST_SPEED * TURN_PERCENT));
	else
		MotorSpeed(RIGHT_SIDE, BOOST_SPEED);

	MotorDir(RIGHT_SIDE, FORWARD);
	MotorDir(LEFT_SIDE, FORWARD);
	if(activeBoostTimer == BOOST_TIME)
	{
		targetWheelSpeed = 0;
		MotorSpeed(LEFT_SIDE, 0);
		MotorSpeed(RIGHT_SIDE, 0);
		MotorDir(RIGHT_SIDE, FORWARD);
		MotorDir(LEFT_SIDE, FORWARD);
		motorControlState = ControllerInput;
	}
}

void manageController()
{
	activeAccelerationTimer = activeAccelerationTimer + 1;
	if(GamepadButtons.buttonA)
		targetWheelSpeed = (60 << 8);
	else
		targetWheelSpeed = 0;

	if(activeAccelerationTimer == 10)
	{
		activeAccelerationTimer = 0;

		if(currentWheelSpeed != targetWheelSpeed)
		{
			if(targetWheelSpeed > currentWheelSpeed)
				currentWheelSpeed = currentWheelSpeed + (5 << 8);
			else
				currentWheelSpeed = currentWheelSpeed - (5 << 8);
		}

		if(GamepadButtons.dpad == 6)
			MotorSpeed(LEFT_SIDE, (currentWheelSpeed * TURN_PERCENT));
		else
			MotorSpeed(LEFT_SIDE, currentWheelSpeed);
		if(GamepadButtons.dpad == 2)
			MotorSpeed(RIGHT_SIDE, (currentWheelSpeed * TURN_PERCENT));
		else
			MotorSpeed(RIGHT_SIDE, currentWheelSpeed);
	}
}







