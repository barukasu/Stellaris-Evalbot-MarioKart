#include "drivers/motor.h"


typedef enum
{
	//! Basic controller Input
	ControllerInput = 0,
	//! +BoostSpeed
	Boost,
	//! Spinout
	Spinout
} ControlState;

typedef enum
{
	NoState = 0,
	 //! Spin Clockwise
	leg1,
	//! Spin Counterclockwise
	leg2,
	 //! Spin Clockwise
	leg3
} spinOutState;



#define BOOST_SPEED (90 << 8)
#define REG_MAX_SPEED (60 << 8)
#define BOOST_TIME 400
#define SPINOUT_TIME 20
#define TURN_PERCENT (2.0/3.0)
//Timers
static volatile unsigned long activeBoostTimer;
static volatile unsigned long activeSpinoutTimer;
static volatile unsigned long activeAccelerationTimer;

// State controllers
	// Variables for maintaining Higher state levels
static volatile unsigned long StartIsActive;
static volatile unsigned long SpinOutMotorIsActive;
static volatile unsigned long BoostMotorIsActive;
static volatile unsigned long RegMotorIsActive;
	// Variables for maintaining substate levels
static volatile unsigned long motorControlState;
static volatile unsigned long spinOutControlSubState;
	// Variables for maintaining wheel status
static volatile unsigned long currentWheelSpeed;
static volatile unsigned long currentLeftWheelSpeed;
static volatile unsigned long currentRightWheelSpeed;

static volatile unsigned long targetWheelSpeed;
static volatile unsigned long targetLeftWheelSpeed;
static volatile unsigned long targetRightWheelSpeed;

void stepMotor();
void initDriveControl();
void stateController();

void ActivateSpinout();
void DecactiveSpinOut();
void ActivateBoost();
void DeactiveBoost();

void setDirectionForward();
void setDriectionReverse();
void SpinOut(spinOutState state);

void manageController();
void manageBoost();
void manageSpinOut();

