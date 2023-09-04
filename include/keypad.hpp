#pragma once

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

bool in_Numpad();
long getNumpadResult();
float numpadToConeRatio();
long numpadToDeciMicrons();
bool setupKeypad();
void taskKeypad(void *param);
