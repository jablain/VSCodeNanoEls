#include <Arduino.h>
#include "macros.hpp"
#include "axis.hpp"
#include "vars.hpp"
#include "modes.hpp"

// Only used for async movement in ASYNC and A1 modes.
// Keep code in this method to absolute minimum to achieve high stepper speeds.
void IRAM_ATTR onAsyncTimer() {
  Axis* a = getAsyncAxis();
  if (!isOn || a->movingManually) {
    return;
  } else if (dupr > 0 && (a->leftStop == LONG_MAX || a->pos < a->leftStop)) {
    if (a->pos >= a->motorPos) {
      a->pos++;
    }
    a->motorPos++;
    a->posGlobal++;
  } else if (dupr < 0 && (a->rightStop == LONG_MIN || a->pos > a->rightStop)) {
    if (a->pos >= a->motorPos + a->backlashSteps) {
      a->pos--;
    }
    a->motorPos--;
    a->posGlobal--;
  } else {
    return;
  }

  DLOW(a->stepPin);
  a->stepStartUs = micros();
  delayMicroseconds(10);
  DHIGH(a->stepPin);
}

