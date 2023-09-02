#pragma once

#include "axis.hpp"

#define MODE_NORMAL 0
#define MODE_ASYNC 2
#define MODE_CONE 3
#define MODE_TURN 4
#define MODE_FACE 5
#define MODE_CUT 6
#define MODE_THREAD 7
#define MODE_ELLIPSE 8
#define MODE_GCODE 9
#define MODE_A1 10

extern volatile int mode; // mode of operation (ELS, multi-start ELS, asynchronous)
extern int nextMode; // mode value that should be applied asap
extern bool nextModeFlag; // whether nextMode needs attention
extern int savedMode; // mode saved in Preferences

extern bool isOn;
extern bool nextIsOn; // isOn value that should be applied asap
extern bool nextIsOnFlag; // whether nextIsOn requires attention

bool isPassMode();
long getPassModeZStart();
long getPassModeXStart();
int getLastSetupIndex();
void setModeFromTask(int value);

// Used in keypad

bool needZStops();

// === Should this be here ??

void setDupr(long value);
void setStarts(int value);
void setConeRatio(float value);
void setModeFromLoop(int value);
void setIsOnFromLoop(bool on);
// Loose the thread and mark current physical positions of
// encoder and stepper as a new 0. To be called when dupr changes
// or ELS is turned on/off. Without this, changing dupr will
// result in stepper rushing across the lathe to the new position.
// Must be called while holding motionMutex.
void markOrigin();
void updateAsyncTimerSettings();