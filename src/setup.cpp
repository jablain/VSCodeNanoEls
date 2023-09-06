// https://github.com/kachurovskiy/nanoels

#include "pcb.hpp"
#include "macros.hpp"
#include "keypad.hpp"
#include "tasks.hpp"
#include "vars.hpp"
#include "preferences.hpp"
#include "display.hpp"
#include "axis.hpp"
#include "spindle.hpp"

void setup() {
  setupPCB();
  // VVV Warning : The following code must run in this call order VVV
  setupAxis();
  readPreferences ();
  // This code must run after setupAxis()
  if (!z.needsRest && !z.disabled) {
    if (INVERT_Z_ENA)
      DLOW(z.ena);
    else
      DHIGH(z.ena);
  }
  if (!x.needsRest && !x.disabled) {
    if (INVERT_X_ENA)
      DLOW(x.ena);
    else
      DHIGH(x.ena);
  }
  if (a1.active && !a1.needsRest && !a1.disabled) {
    if (INVERT_A1_ENA)
      DLOW(a1.ena);
    else
      DHIGH(a1.ena);
  }
  // ^^^ Warning : The following code must run in this call order ^^^
  setupDisplay();
  setupKeypad();
  xTaskCreatePinnedToCore(taskMoveZ, "taskMoveZ", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskMoveX, "taskMoveX", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  if (a1.active)
    xTaskCreatePinnedToCore(taskMoveA1, "taskMoveA1", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore  (taskAttachInterrupts, "taskAttachInterrupts", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore  (taskGcode, "taskGcode", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  powerOnBeep();
}
