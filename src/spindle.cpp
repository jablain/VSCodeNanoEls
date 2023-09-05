#include <Arduino.h>
#include "macros.hpp"
#include "config.hpp"
#include "pcb.hpp"
#include "spindle.hpp"
#include "axis.hpp"
#include "modes.hpp"
#include "vars.hpp"

#ifdef WITHSPINDLECLASS
// Singleton instance getter
Spindle& Spindle::getInstance() {
    static Spindle instance;
    return instance;
}

// Private constructor
Spindle::Spindle() {
  Spindle::taskAttachInterrupts();
}

void Spindle::zeroSpindlePos() {
  spindlePos = 0;
  spindlePosAvg = 0;
  spindlePosSync = 0;
}

int Spindle::getApproxRpm() {
  unsigned long t = micros();
  if (t > spindleEncTime + 100000) 
    // RPM less than 10.
    return 0;
  if (t < shownRpmTime + RPM_UPDATE_INTERVAL_MICROS) 
    // Don't update RPM too often to avoid flickering.
    return shownRpm;
  int rpm = 0;
  if (spindleEncTimeDiffBulk > 0) {
    rpm = 60000000 / spindleEncTimeDiffBulk;
    if (abs(rpm - shownRpm) < (rpm < 1000 ? 2 : 5)) 
      // Don't update RPM with insignificant differences.
      rpm = shownRpm;
  }
  return rpm;
}

long Spindle::spindleModulo(long value) {
  value = value % ENCODER_STEPS_INT;
  if (value < 0) 
    value += ENCODER_STEPS_INT;
  return value;
}

void Spindle::discountFullSpindleTurns() {
  // When standing at the stop, ignore full spindle turns.
  // This allows to avoid waiting when spindle direction reverses
  // and reduces the chance of the skipped stepper steps since
  // after a reverse the spindle starts slow.
  if (dupr != 0 && !stepperIsRunning(&z) && (mode == MODE_NORMAL || mode == MODE_CONE)) {
    int spindlePosDiff = 0;
    if (z.pos == z.rightStop) {
      long stopSpindlePos = spindleFromPos(&z, z.rightStop);
      if (dupr > 0) {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) {
          spindlePosDiff = ENCODER_STEPS_INT;
        }
      } else {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) {
          spindlePosDiff = -ENCODER_STEPS_INT;
        }
      }
    } else if (z.pos == z.leftStop) {
      long stopSpindlePos = spindleFromPos(&z, z.leftStop);
      if (dupr > 0) {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) {
          spindlePosDiff = -ENCODER_STEPS_INT;
        }
      } else {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) {
          spindlePosDiff = ENCODER_STEPS_INT;
        }
      }
    }
    if (spindlePosDiff != 0) {
      spindlePos += spindlePosDiff;
      spindlePosAvg += spindlePosDiff;
    }
  }
}

void Spindle::processSpindlePosDelta() {
  long delta = spindlePosDelta;
  if (delta == 0) 
    return;
  unsigned long microsNow = micros();
  if (showTacho || mode == MODE_GCODE) {
    if (spindleEncTimeIndex >= RPM_BULK) {
      spindleEncTimeDiffBulk = microsNow - spindleEncTimeAtIndex0;
      spindleEncTimeAtIndex0 = microsNow;
      spindleEncTimeIndex = 0;
    }
    spindleEncTimeIndex += abs(delta);
  } else 
    spindleEncTimeDiffBulk = 0;

  spindlePos += delta;
  spindlePosGlobal += delta;
  if (spindlePosGlobal > ENCODER_STEPS_INT) 
    spindlePosGlobal -= ENCODER_STEPS_INT;
  else if (spindlePosGlobal < 0) 
    spindlePosGlobal += ENCODER_STEPS_INT;
  if (spindlePos > spindlePosAvg) 
    spindlePosAvg = spindlePos;
  else if (spindlePos < spindlePosAvg - ENCODER_BACKLASH)
    spindlePosAvg = spindlePos + ENCODER_BACKLASH;
  spindleEncTime = microsNow;

  if (spindlePosSync != 0) {
    spindlePosSync += delta;
    if (spindlePosSync % ENCODER_STEPS_INT == 0) {
      spindlePosSync = 0;
      Axis* a = getPitchAxis();
      spindlePosAvg = spindlePos = spindleFromPos(a, a->pos);
    }
  }
  spindlePosDelta -= delta;
}

void IRAM_ATTR Spindle::spinEnc() {
  TheSpindle.spindlePosDelta += DREAD(ENC_B) ? -1 : 1;
}

void IRAM_ATTR Spindle::pulse1Enc() {
  unsigned long now = micros();
  if (DREAD(A12)) {
    TheSpindle.pulse1HighMicros = now;
  } else if (now > TheSpindle.pulse1HighMicros + PULSE_MIN_WIDTH_US) {
    TheSpindle.pulse1Delta += (DREAD(A13) ? -1 : 1) * (PULSE_1_INVERT ? -1 : 1);
  }
}

void IRAM_ATTR Spindle::pulse2Enc() {
  unsigned long now = micros();
  if (DREAD(A22)) {
    TheSpindle.pulse2HighMicros = now;
  } else if (now > TheSpindle.pulse2HighMicros + PULSE_MIN_WIDTH_US) {
    TheSpindle.pulse2Delta += (DREAD(A23) ? -1 : 1) * (PULSE_2_INVERT ? -1 : 1);
  }
}

void Spindle::taskAttachInterrupts() {
  // Attaching interrupt on core 0 to have more time on core 1 where axes are moved.
  TheSpindle.spindlePosDelta = 0; // Unprocessed encoder ticks.
  attachInterrupt(digitalPinToInterrupt(ENC_A), TheSpindle.spinEnc, FALLING);
  if (PULSE_1_USE)
    attachInterrupt(digitalPinToInterrupt(A12), TheSpindle.pulse1Enc, CHANGE);
  if (PULSE_2_USE)
    attachInterrupt(digitalPinToInterrupt(A22), TheSpindle.pulse2Enc, CHANGE);
  vTaskDelete(NULL);
}

  Spindle& TheSpindle = Spindle::getInstance();
#else

unsigned long spindleEncTime = 0; // micros() of the previous spindle update
unsigned long spindleEncTimeDiffBulk = 0; // micros() between RPM_BULK spindle updates
unsigned long spindleEncTimeAtIndex0 = 0; // micros() when spindleEncTimeIndex was 0
int spindleEncTimeIndex = 0; // counter going between 0 and RPM_BULK - 1
long spindlePos = 0; // Spindle position
long spindlePosAvg = 0; // Spindle position accounting for encoder backlash
std::atomic<long> spindlePosDelta; // Unprocessed encoder ticks. see https://forum.arduino.cc/t/does-c-std-atomic-work-with-dual-core-esp32/690214
int spindlePosSync = 0; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
long spindlePosGlobal = 0; // global spindle position that is unaffected by e.g. zeroing

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

void discountFullSpindleTurns() {
  // When standing at the stop, ignore full spindle turns.
  // This allows to avoid waiting when spindle direction reverses
  // and reduces the chance of the skipped stepper steps since
  // after a reverse the spindle starts slow.
  if (dupr != 0 && !stepperIsRunning(&z) && (mode == MODE_NORMAL || mode == MODE_CONE)) {
    int spindlePosDiff = 0;
    if (z.pos == z.rightStop) {
      long stopSpindlePos = spindleFromPos(&z, z.rightStop);
      if (dupr > 0) {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) {
          spindlePosDiff = ENCODER_STEPS_INT;
        }
      } else {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) {
          spindlePosDiff = -ENCODER_STEPS_INT;
        }
      }
    } else if (z.pos == z.leftStop) {
      long stopSpindlePos = spindleFromPos(&z, z.leftStop);
      if (dupr > 0) {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) {
          spindlePosDiff = -ENCODER_STEPS_INT;
        }
      } else {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) {
          spindlePosDiff = ENCODER_STEPS_INT;
        }
      }
    }
    if (spindlePosDiff != 0) {
      spindlePos += spindlePosDiff;
      spindlePosAvg += spindlePosDiff;
    }
  }
}

void processSpindlePosDelta() {
  long delta = spindlePosDelta;
  if (delta == 0) 
    return;
  unsigned long microsNow = micros();
  if (showTacho || mode == MODE_GCODE) {
    if (spindleEncTimeIndex >= RPM_BULK) {
      spindleEncTimeDiffBulk = microsNow - spindleEncTimeAtIndex0;
      spindleEncTimeAtIndex0 = microsNow;
      spindleEncTimeIndex = 0;
    }
    spindleEncTimeIndex += abs(delta);
  } else 
    spindleEncTimeDiffBulk = 0;

  spindlePos += delta;
  spindlePosGlobal += delta;
  if (spindlePosGlobal > ENCODER_STEPS_INT) 
    spindlePosGlobal -= ENCODER_STEPS_INT;
  else if (spindlePosGlobal < 0) 
    spindlePosGlobal += ENCODER_STEPS_INT;
  if (spindlePos > spindlePosAvg) 
    spindlePosAvg = spindlePos;
  else if (spindlePos < spindlePosAvg - ENCODER_BACKLASH)
    spindlePosAvg = spindlePos + ENCODER_BACKLASH;
  spindleEncTime = microsNow;

  if (spindlePosSync != 0) {
    spindlePosSync += delta;
    if (spindlePosSync % ENCODER_STEPS_INT == 0) {
      spindlePosSync = 0;
      Axis* a = getPitchAxis();
      spindlePosAvg = spindlePos = spindleFromPos(a, a->pos);
    }
  }
  spindlePosDelta -= delta;
}

// Called on a FALLING interrupt for the spindle rotary encoder pin.
void IRAM_ATTR spinEnc() { spindlePosDelta += DREAD(ENC_B) ? -1 : 1; }

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

void taskAttachInterrupts(void *param) {
  // Attaching interrupt on core 0 to have more time on core 1 where axes are moved.
  spindlePosDelta = 0; // Unprocessed encoder ticks.
  attachInterrupt(digitalPinToInterrupt(ENC_A), spinEnc, FALLING);
  if (PULSE_1_USE) attachInterrupt(digitalPinToInterrupt(A12), pulse1Enc, CHANGE);
  if (PULSE_2_USE) attachInterrupt(digitalPinToInterrupt(A22), pulse2Enc, CHANGE);
  vTaskDelete(NULL);
}

#endif