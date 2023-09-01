#pragma once

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "axis.hpp"

extern LiquidCrystal lcd;

#define MEASURE_METRIC 0
#define MEASURE_INCH 1
#define MEASURE_TPI 2

extern String gcodeCommand;
extern bool auxForward; // True for external, false for external thread
extern int starts; // number of starts in a multi-start thread
extern long dupr; // pitch, tenth of a micron per rotation
extern float coneRatio; // In cone mode, how much X moves for 1 step of Z
extern int turnPasses; // In turn mode, how many turn passes to make
extern long opIndex; // Index of an automation operation

extern long setupIndex; // Index microsof automation setup step

extern long moveStep; // thousandth of a mm
extern long savedMoveStep; // moveStep saved in Preferences

extern long lcdHashLine0;
extern long lcdHashLine1;
extern long lcdHashLine2;
extern long lcdHashLine3;
extern bool splashScreen;

const long PASSES_MAX = 999; // No more turn or face passes than this
const int customCharMmCode = 0;

extern byte customCharMm[];
const int customCharLimUpCode = 1;
extern byte customCharLimUp[];
const int customCharLimDownCode = 2;
extern byte customCharLimDown[];
const int customCharLimLeftCode = 3;
extern byte customCharLimLeft[];
const int customCharLimRightCode = 4;
extern byte customCharLimRight[];
const int customCharLimUpDownCode = 5;
extern byte customCharLimUpDown[];
const int customCharLimLeftRightCode = 6;
extern byte customCharLimLeftRight[];

extern int measure; // Whether to show distances in inches
// For MEASURE_TPI, round TPI to the nearest integer if it's within this range of it.
// E.g. 80.02tpi would be shown as 80tpi but 80.04tpi would be shown as-is.
const float TPI_ROUND_EPSILON = 0.03;
extern bool showAngle; // Whether to show 0-359 spindle angle on screen
extern bool showTacho; // Whether to show spindle RPM on screen

// To be incremented whenever a measurable improvement is made.
#define SOFTWARE_VERSION 7
// To be changed whenever a different PCB / encoder / stepper / ... design is used.
#define HARDWARE_VERSION 4



// Returns number of letters printed.
int printDeciMicrons(long deciMicrons, int precisionPointsMax);
int printDegrees(long degrees10000);
int printDupr(long value);
void printLcdSpaces(int charIndex);

int printAxisStopDiff(Axis* a, bool addTrailingSpace);
int printAxisPosWithName(Axis* a, bool addTrailingSpace);
int printNoTrailing0(float value);
int printAxisPos(Axis* a);
int printMode();
void updateDisplay();
bool needZStops();

