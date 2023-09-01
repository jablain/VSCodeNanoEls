#pragma once

#include <Adafruit_TCA8418.h>
#include "axis.hpp"

#define B_LEFT 57
#define B_RIGHT 37
#define B_UP 47
#define B_DOWN 67
#define B_MINUS 5
#define B_PLUS 64
#define B_ON 17
#define B_OFF 27
#define B_STOPL 7
#define B_STOPR 15
#define B_STOPU 6
#define B_STOPD 16
#define B_DISPL 14
#define B_STEP 24
#define B_SETTINGS 34
#define B_MEASURE 54
#define B_REVERSE 44
#define B_0 51
#define B_1 41
#define B_2 61
#define B_3 31
#define B_4 2
#define B_5 21
#define B_6 12
#define B_7 11
#define B_8 22
#define B_9 1
#define B_BACKSPACE 32
#define B_MODE_GEARS 42
#define B_MODE_TURN 52
#define B_MODE_FACE 62
#define B_MODE_CONE 3
#define B_MODE_CUT 13
#define B_MODE_THREAD 23
#define B_MODE_OTHER 33
#define B_X 53
#define B_Z 43
#define B_A 4
#define B_B 63

extern Adafruit_TCA8418 keypad;

extern unsigned long keypadTimeUs;

// Most buttons we only have "down" handling, holding them has no effect.
// Buttons with special "holding" logic have flags below.
extern bool buttonLeftPressed;
extern bool buttonRightPressed;
extern bool buttonUpPressed;
extern bool buttonDownPressed;
extern bool buttonOffPressed;
extern bool buttonGearsPressed;
extern bool buttonTurnPressed;
extern unsigned long resetMillis;
extern bool opIndexAdvanceFlag; // Whether user requested to move to the next pass


extern bool inNumpad;
extern int numpadDigits[20];
extern int numpadIndex;
extern long getNumpadResult();
extern float numpadToConeRatio();
extern long numpadToDeciMicrons();

void buttonOnOffPress(bool on);
void processKeypadEvent();
void beep();
void buttonOffRelease();
bool processNumpad(int keyCode);

void numpadPress(int digit);
void numpadBackspace();
void numpadPlusMinus(bool plus);
void resetNumpad();


void buttonPlusMinusPress(bool plus);
void buttonLeftStopPress(Axis* a);
void buttonRightStopPress(Axis* a);
void buttonDisplayPress();
void buttonMoveStepPress();
void buttonModePress();

void buttonMeasurePress();
void buttonReversePress();
bool processNumpadResult(int keyCode);

void setTurnPasses(int value);
void setLeftStop(Axis* a, long value);
void setRightStop(Axis* a, long value);
long normalizePitch(long pitch);
void setMeasure(int value);
bool stepToFinal(Axis* a, long newPos);
bool stepTo(Axis* a, long newPos, bool continuous);
bool stepToContinuous(Axis* a, long newPos);