#pragma once

// === Used in keypad

extern bool splashScreen;
const long PASSES_MAX = 999; // No more turn or face passes than this

bool needZStops();

// === Used in main

extern long savedMoveStep; // moveStep saved in Preferences

void updateDisplay();
void displayEstop();
void lcdSetup();