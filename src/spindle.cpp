#include <Arduino.h>
#include "macros.hpp"
#include "config.hpp"
#include "pcb.hpp"
#include "spindle.hpp"

unsigned long spindleEncTime = 0; // micros() of the previous spindle update
unsigned long spindleEncTimeDiffBulk = 0; // micros() between RPM_BULK spindle updates
unsigned long spindleEncTimeAtIndex0 = 0; // micros() when spindleEncTimeIndex was 0
int spindleEncTimeIndex = 0; // counter going between 0 and RPM_BULK - 1
long spindlePos = 0; // Spindle position
long spindlePosAvg = 0; // Spindle position accounting for encoder backlash
long savedSpindlePosAvg = 0; // spindlePosAvg saved in Preferences
long savedSpindlePos = 0; // spindlePos value saved in Preferences
std::atomic<long> spindlePosDelta; // Unprocessed encoder ticks. see https://forum.arduino.cc/t/does-c-std-atomic-work-with-dual-core-esp32/690214
int spindlePosSync = 0; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
int savedSpindlePosSync = 0; // spindlePosSync saved in Preferences
long spindlePosGlobal = 0; // global spindle position that is unaffected by e.g. zeroing
long savedSpindlePosGlobal = 0; // spindlePosGlobal saved in Preferences

bool savedShowAngle = false; // showAngle value saved in Preferences
bool savedShowTacho = false; // showTacho value saved in Preferences
int shownRpm = 0;
unsigned long shownRpmTime = 0; // micros() when shownRpm was set

const long RPM_UPDATE_INTERVAL_MICROS = 1000000; // Don't redraw RPM more often than once per second

unsigned long pulse1HighMicros = 0;
unsigned long pulse2HighMicros = 0;

const long PULSE_MIN_WIDTH_US = 1000; // Microseconds width of the pulse that is required for it to be registered. Prevents noise.

volatile int pulse1Delta = 0; // Outstanding pulses generated by pulse generator on terminal A1.
volatile int pulse2Delta = 0; // Outstanding pulses generated by pulse generator on terminal A2.

const bool PULSE_1_INVERT = false; // Set to true to change the direction in which encoder moves the axis
const bool PULSE_2_INVERT = true; // Set to false to change the direction in which encoder moves the axis

void zeroSpindlePos() {
  spindlePos = 0;
  spindlePosAvg = 0;
  spindlePosSync = 0;
}

int getApproxRpm() {
  unsigned long t = micros();
  if (t > spindleEncTime + 100000) {
    // RPM less than 10.
    return 0;
  }
  if (t < shownRpmTime + RPM_UPDATE_INTERVAL_MICROS) {
    // Don't update RPM too often to avoid flickering.
    return shownRpm;
  }
  int rpm = 0;
  if (spindleEncTimeDiffBulk > 0) {
    rpm = 60000000 / spindleEncTimeDiffBulk;
    if (abs(rpm - shownRpm) < (rpm < 1000 ? 2 : 5)) {
      // Don't update RPM with insignificant differences.
      rpm = shownRpm;
    }
  }
  return rpm;
}

long spindleModulo(long value) {
  value = value % ENCODER_STEPS_INT;
  if (value < 0) {
    value += ENCODER_STEPS_INT;
  }
  return value;
}

// Called on a FALLING interrupt for the spindle rotary encoder pin.
void IRAM_ATTR spinEnc() {
  spindlePosDelta += DREAD(ENC_B) ? -1 : 1;
}

// Called on a FALLING interrupt for the first axis rotary encoder pin.
void IRAM_ATTR pulse1Enc() {
  unsigned long now = micros();
  if (DREAD(A12)) {
    pulse1HighMicros = now;
  } else if (now > pulse1HighMicros + PULSE_MIN_WIDTH_US) {
    pulse1Delta += (DREAD(A13) ? -1 : 1) * (PULSE_1_INVERT ? -1 : 1);
  }
}

// Called on a FALLING interrupt for the second axis rotary encoder pin.
void IRAM_ATTR pulse2Enc() {
  unsigned long now = micros();
  if (DREAD(A22)) {
    pulse2HighMicros = now;
  } else if (now > pulse2HighMicros + PULSE_MIN_WIDTH_US) {
    pulse2Delta += (DREAD(A23) ? -1 : 1) * (PULSE_2_INVERT ? -1 : 1);
  }
}

