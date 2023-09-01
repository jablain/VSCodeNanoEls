#pragma once

// === Used in keypad

const long PASSES_MAX = 999; // No more turn or face passes than this
extern bool splashScreen;

// === Used in main

extern long savedMoveStep; // moveStep saved in Preferences

void updateDisplay();
void displayEstop();
void lcdSetup();
void taskDisplay(void *param);