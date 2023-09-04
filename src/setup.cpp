// https://github.com/kachurovskiy/nanoels

#include "pcb.hpp"
#include "keypad.hpp"
#include "tasks.hpp"
#include "vars.hpp"
#include "preferences.hpp"
#include "display.hpp"
#include "axis.hpp"
#include "spindle.hpp"

void setup() {
  setupPCB();
  setupAxis();
  setupPreferences ();
  setupDisplay();
  setupKeypad();
  xTaskCreatePinnedToCore(taskDisplay, "taskDisplay", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskMoveZ, "taskMoveZ", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskMoveX, "taskMoveX", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  if (a1.active)
    xTaskCreatePinnedToCore(taskMoveA1, "taskMoveA1", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore  (taskAttachInterrupts, "taskAttachInterrupts", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore  (taskGcode, "taskGcode", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  powerOnBeep();
}
