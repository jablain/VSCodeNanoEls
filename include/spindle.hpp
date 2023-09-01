#pragma once

#include <atomic>
#include "config.hpp"

extern unsigned long spindleEncTime; // micros() of the previous spindle update
extern unsigned long spindleEncTimeDiffBulk; // micros() between RPM_BULK spindle updates
extern unsigned long spindleEncTimeAtIndex0; // micros() when spindleEncTimeIndex was 0
extern int spindleEncTimeIndex; // counter going between 0 and RPM_BULK - 1
extern long spindlePos; // Spindle position
extern long spindlePosAvg; // Spindle position accounting for encoder backlash
extern long savedSpindlePosAvg; // spindlePosAvg saved in Preferences
extern long savedSpindlePos; // spindlePos value saved in Preferences
extern std::atomic<long> spindlePosDelta; // Unprocessed encoder ticks. see https://forum.arduino.cc/t/does-c-std-atomic-work-with-dual-core-esp32/690214
extern int spindlePosSync; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
extern int savedSpindlePosSync; // spindlePosSync saved in Preferences
extern long spindlePosGlobal; // global spindle position that is unaffected by e.g. zeroing
extern long savedSpindlePosGlobal; // spindlePosGlobal saved in Preferences

extern bool showAngle; // Whether to show 0-359 spindle angle on screen
extern bool showTacho; // Whether to show spindle RPM on screen
extern bool savedShowAngle; // showAngle value saved in Preferences
extern bool savedShowTacho; // showTacho value saved in Preferences
extern int shownRpm;
extern unsigned long shownRpmTime; // micros() when shownRpm was set

const float ENCODER_STEPS_FLOAT = ENCODER_STEPS_INT; // Convenience float version of ENCODER_STEPS_INT

extern volatile int pulse1Delta; // Outstanding pulses generated by pulse generator on terminal A1.
extern volatile int pulse2Delta; // Outstanding pulses generated by pulse generator on terminal A2.

int getApproxRpm();
long spindleModulo(long value);
void zeroSpindlePos();

// Called on a FALLING interrupt for the spindle rotary encoder pin.
void IRAM_ATTR spinEnc();
// Called on a FALLING interrupt for the first axis rotary encoder pin.
void IRAM_ATTR pulse1Enc();
// Called on a FALLING interrupt for the second axis rotary encoder pin.
void IRAM_ATTR pulse2Enc();