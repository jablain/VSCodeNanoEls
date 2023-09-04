#pragma once
#include "axis.hpp"

// Used by display

extern bool inNumpad;
extern long getNumpadResult();
extern float numpadToConeRatio();
extern long numpadToDeciMicrons();

// Used by main

// Most buttons we only have "down" handling, holding them has no effect.
// Buttons with special "holding" logic have flags below.
extern bool buttonLeftPressed;
extern bool buttonRightPressed;
extern bool buttonUpPressed;
extern bool buttonDownPressed;
extern bool buttonOffPressed;
extern bool buttonGearsPressed;
extern bool buttonTurnPressed;
extern bool opIndexAdvanceFlag; // Whether user requested to move to the next pass
extern unsigned long keypadTimeUs;

bool setupKeypad();
bool keypadAvailable();
void taskKeypad(void *param);
void setMeasure(int value);
bool stepToFinal(Axis* a, long newPos);
bool stepToContinuous(Axis* a, long newPos);
String getValueString(const String& command, char letter);
float getFloat(const String& command, char letter);
int getInt(const String& command, char letter);