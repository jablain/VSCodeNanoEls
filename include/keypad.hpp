#pragma once

// Most buttons we only have "down" handling, holding them has no effect.
// Buttons with special "holding" logic have flags below.
bool btnLeftPressed();
bool btnRightPressed();
bool btnUpPressed();
bool btnDownPressed();
bool btnOffPressed();
bool btnGearsPressed();
bool btnTurnPressed();
unsigned long kpadTimeUs();

bool in_Numpad();
long getNumpadResult();
float numpadToConeRatio();
long numpadToDeciMicrons();
void setopIndexAdvanceFlag (bool); // Is the user requesting to move to the next pass
bool getopIndexAdvanceFlag (); // Has the user requested to move to the next pass
void setupKeypad();
