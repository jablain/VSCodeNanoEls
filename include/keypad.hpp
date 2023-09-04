#pragma once
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
void taskKeypad(void *param);
