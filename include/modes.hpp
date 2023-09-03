#pragma once

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
void setIsOnFromLoop(bool on);
void setModeFromLoop(int value);

// Used in keypad

bool needZStops();

// === Should this be here ??

void setDupr(long value);
void setStarts(int value);
void setConeRatio(float value);
void updateAsyncTimerSettings();



