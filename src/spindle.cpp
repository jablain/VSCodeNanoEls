//see https://forum.arduino.cc/t/does-c-std-atomic-work-with-dual-core-esp32/690214
#define USEATOMICS

#include <atomic>
#include "pcb.hpp"
#include "macros.hpp"
#include "vars.hpp"
#include "config.hpp"
#include "modes.hpp"
#include "spindle.hpp"

unsigned long spindleEncTime; // micros() of the previous spindle update

long spindlePos;     // Spindle position
long spindlePosAvg;  // Backlash corrected Spindle position
int  spindlePosSync; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
long spindlePosGlobal = 0; // global spindle position that is unaffected by e.g. zeroing

#ifdef USEATOMICS
std::atomic<long> spindlePosDelta; // Unprocessed encoder ticks.
#else
volatile long spindlePosDelta; // Unprocessed encoder ticks.
#endif

unsigned long pulse1HighMicros = 0;
unsigned long pulse2HighMicros = 0;

volatile int pulse1Delta = 0; // Outstanding pulses generated by pulse generator on terminal A1.
volatile int pulse2Delta = 0; // Outstanding pulses generated by pulse generator on terminal A2.

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
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) 
          spindlePosDiff = ENCODER_STEPS_INT;
      } else {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) 
          spindlePosDiff = -ENCODER_STEPS_INT;
      }
    } else if (z.pos == z.leftStop) {
      long stopSpindlePos = spindleFromPos(&z, z.leftStop);
      if (dupr > 0) {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) 
          spindlePosDiff = -ENCODER_STEPS_INT;
      } else {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) 
          spindlePosDiff = ENCODER_STEPS_INT;
      }
    }
    if (spindlePosDiff != 0) {
      spindlePos += spindlePosDiff;
      spindlePosAvg += spindlePosDiff;
    }
  }
}

//=============================================================================
// This function is called from loop() to process spindle encoder ticks
// recorded by the interrupt routine spinEnc().

unsigned long lastDeltaTime;
std::atomic<float> currentRpm;
std::atomic<float> currentAngle;

void processSpindlePosDelta() {
  const float integrator = 0.9999; // Integrator constant for RPM calculation
  #ifdef USEATOMICS
  long delta = spindlePosDelta; // Atomic read of current spindlePosDelta
  #else
  noInterrupts();
  long delta = spindlePosDelta; // Atomic read of current spindlePosDelta
  spindlePosDelta = 0; // Atomic read of current spindlePosDelta
  interrupts();
  #endif
  unsigned long microsNow = micros();
  // Update tachometer data
  if ((microsNow - lastDeltaTime) > 0) {
    currentRpm = (currentRpm * integrator) + ((abs(delta)/ENCODER_STEPS_FLOAT) * 60000000.0 / (microsNow - lastDeltaTime) * (1-integrator));
    lastDeltaTime = microsNow;
  }
  if (delta == 0) // Nothing more to do
    return;
  spindleEncTime = microsNow;

  // Update spindle position
  spindlePos += delta; 
  currentAngle = ((spindlePos % ENCODER_STEPS_INT) * 360.0 / ENCODER_STEPS_FLOAT);
  spindlePosGlobal = (spindlePosGlobal + delta + ENCODER_STEPS_INT) % ENCODER_STEPS_INT;

  // Update spindle position with backlash compensation
  if (spindlePos > spindlePosAvg) 
    spindlePosAvg = spindlePos;
  else if (spindlePos < spindlePosAvg - ENCODER_BACKLASH)
    spindlePosAvg = spindlePos + ENCODER_BACKLASH;

  if (spindlePosSync != 0) {
    spindlePosSync += delta;
    if (spindlePosSync % ENCODER_STEPS_INT == 0) {
      spindlePosSync = 0;
      Axis* a = getPitchAxis();
      spindlePosAvg = spindlePos = spindleFromPos(a, a->pos);
    }
  }
  #ifdef USEATOMICS
  spindlePosDelta -= delta; // Atomic operation to update spindlePosDelta
  #endif
  discountFullSpindleTurns();
}

//===============================================================================
// These are interrupts routines running on core 0
//
// spinEnc()   : Called on a FALLING edge of the spindle rotary encoder pin.
// pulse1Enc() : Called on a FALLING edge of the first axis rotary encoder pin.
// pulse2Enc() : Called on a FALLING edge of the second axis rotary encoder pin.

void IRAM_ATTR spinEnc() {
  spindlePosDelta += DREAD(ENC_B) ? -1 : 1;
}

const long PULSE_MIN_WIDTH_US = 1000; // Microseconds width of the pulse that is required for it to be registered. Prevents noise.

void IRAM_ATTR pulse1Enc() {
  const bool PULSE_1_INVERT = false; // Set to true to change the direction in which encoder moves the axis
  unsigned long now = micros();
  if (DREAD(A12)) 
    pulse1HighMicros = now;
  else if (now > pulse1HighMicros + PULSE_MIN_WIDTH_US) 
    pulse1Delta += (DREAD(A13) ? -1 : 1) * (PULSE_1_INVERT ? -1 : 1);
}

void IRAM_ATTR pulse2Enc() {
  const bool PULSE_2_INVERT = true;  // Set to false to change the direction in which encoder moves the axis
  unsigned long now = micros();
  if (DREAD(A22)) 
    pulse2HighMicros = now;
  else if (now > pulse2HighMicros + PULSE_MIN_WIDTH_US)
    pulse2Delta += (DREAD(A23) ? -1 : 1) * (PULSE_2_INVERT ? -1 : 1);
}

//===============================================================================
// This task runs on core 0 during the setup phase. It simply connects
// interrrupt routines to monitor the spindle encoder and possibly up to two
// handwheels.

void taskAttachInterrupts(void *param) {
  zeroSpindlePos();
  lastDeltaTime = micros();
  currentRpm = 0;
  spindleEncTime = 0; // micros() of the previous spindle update
  spindlePosDelta = 0; // Unprocessed encoder ticks.
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), spinEnc, FALLING);
  if (PULSE_1_USE)
    attachInterrupt(digitalPinToInterrupt(A12), pulse1Enc, CHANGE);
  if (PULSE_2_USE)
    attachInterrupt(digitalPinToInterrupt(A22), pulse2Enc, CHANGE);
  vTaskDelete(NULL);
}