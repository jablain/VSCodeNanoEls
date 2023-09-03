#pragma once
#include <Arduino.h>

void gcodeWaitEpsilon(int epsilon);
void gcodeWaitNear();
void gcodeWaitStop();
// Rapid positioning / linear interpolation.
void G00_01(const String& command);
bool handleGcode(const String& command);
bool handleMcode(const String& command);
// Process one command, return ok flag.
bool handleGcodeCommand(String command);