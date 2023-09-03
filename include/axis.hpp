#pragma once

struct Axis {
  SemaphoreHandle_t mutex;

  char name;
  bool active;
  bool rotational;
  float motorSteps; // motor steps per revolution of the axis
  float screwPitch; // lead screw pitch in deci-microns (10^-7 of a meter)

  long pos; // relative position of the tool in stepper motor steps
  long savedPos; // value saved in Preferences
  float fractionalPos; // fractional distance in steps that we meant to travel but couldn't
  long originPos; // relative position of the stepper motor to origin, in steps
  long savedOriginPos; // originPos saved in Preferences
  long posGlobal; // global position of the motor in steps
  long savedPosGlobal; // posGlobal saved in Preferences
  int pendingPos; // steps of the stepper motor that we should make as soon as possible
  long motorPos; // position of the motor in stepper motor steps, same as pos unless moving back, then differs by backlashSteps
  long savedMotorPos; // motorPos saved in Preferences
  bool continuous; // whether current movement is expected to continue until an unknown position

  long leftStop; // left stop value of pos
  long savedLeftStop; // value saved in Preferences
  long nextLeftStop; // left stop value that should be applied asap
  bool nextLeftStopFlag; // whether nextLeftStop required attention

  long rightStop; // right stop value of pos
  long savedRightStop; // value saved in Preferences
  long nextRightStop; // right stop value that should be applied asap
  bool nextRightStopFlag; // whether nextRightStop requires attention

  long speed; // motor speed in steps / second
  long speedStart; // Initial speed of a motor, steps / second.
  long speedMax; // To limit max speed e.g. for manual moves
  long speedManualMove; // Maximum speed of a motor during manual move, steps / second.
  long acceleration; // Acceleration of a motor, steps / second ^ 2.
  long decelerateSteps; // Number of steps before the end position the deceleration should start.

  bool direction; // To reset speed when direction changes.
  bool directionInitialized;
  unsigned long stepStartUs;
  int stepperEnableCounter;
  bool disabled;
  bool savedDisabled;

  bool invertStepper; // change (true/false) if the carriage moves e.g. "left" when you press "right".
  bool needsRest; // set to false for closed-loop drivers, true for open-loop.
  bool movingManually; // whether stepper is being moved by left/right buttons
  long estopSteps; // amount of steps to exceed machine limits
  long backlashSteps; // amount of steps in reverse direction to re-engage the carriage
  long gcodeRelativePos; // absolute position in steps that relative GCode refers to

  int ena; // Enable pin of this motor
  int dir; // Direction pin of this motor
  int step; // Step pin of this motor
};

extern Axis z;
extern Axis x;
extern Axis a1;

void initAxis(Axis* a, char name, bool active, bool rotational, float motorSteps, float screwPitch, long speedStart, long speedManualMove,
    long acceleration, bool invertStepper, bool needsRest, long maxTravelMm, long backlashDu, int ena, int dir, int step);

inline long stepsToDu(Axis* a, long steps) { return round(steps * a->screwPitch / a->motorSteps); }
inline long getAxisPosDu(Axis* a) { return stepsToDu(a, a->pos + a->originPos); }
inline void markAxis0(Axis* a) { a->originPos = -a->pos; }

inline long getAxisStopDiffDu(Axis* a) {
  if (a->leftStop == LONG_MAX || a->rightStop == LONG_MIN)
    return 0;
  return stepsToDu(a, a->leftStop - a->rightStop);
}

inline bool stepperIsRunning(Axis* a) {
  unsigned long nowUs = micros();
  return nowUs > a->stepStartUs ? nowUs - a->stepStartUs < 50000 : nowUs < 25000;
}

void updateEnable(Axis* a);
void reset(); // === Should this be here ??
void stepperEnable(Axis* a, bool value);
// Loose the thread and mark current physical positions of
// encoder and stepper as a new 0. To be called when dupr changes
// or ELS is turned on/off. Without this, changing dupr will
// result in stepper rushing across the lathe to the new position.
// Must be called while holding motionMutex.
void markOrigin();

void markAxisOrigin(Axis* a);
Axis* getAsyncAxis();
void setDir(Axis* a, bool dir);
Axis* getPitchAxis();
void waitForPendingPosNear0(Axis* a);
void waitForPendingPos0(Axis* a);

// For rotational axis the moveStep of 0.1" means 0.1°.
long getMoveStepForAxis(Axis* a);
long getStepMaxSpeed(Axis* a);
void waitForStep(Axis* a);
int getAndResetPulses(Axis* a);
// Calculates stepper position from spindle position.
long posFromSpindle(Axis* a, long s, bool respectStops);
long mmOrInchToAbsolutePos(Axis* a, float mmOrInch);
// Calculates spindle position from stepper position.
long spindleFromPos(Axis* a, long p);
void leaveStop(Axis* a, long oldStop);
void applyLeftStop(Axis* a);
void applyRightStop(Axis* a);
void moveAxis(Axis* a);
void updateAxisSpeeds(long diffX, long diffZ, long diffA1);