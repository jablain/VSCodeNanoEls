#include "modes.hpp"
#include "tasks.hpp"
#include "vars.hpp"

hw_timer_t *async_timer = timerBegin(0, 80, true);

void setAsyncTimerEnable(bool value) {
  if (value) {
    timerAlarmEnable(async_timer);
  } else {
    timerAlarmDisable(async_timer);
  }
}

void setEmergencyStop(int kind) {
  emergencyStop = kind;
  setAsyncTimerEnable(false);
  xSemaphoreTake(z.mutex, 10);
  xSemaphoreTake(x.mutex, 10);
  xSemaphoreTake(a1.mutex, 10);
}

void setIsOnFromTask(bool on) {
  nextIsOn = on;
  nextIsOnFlag = true;
}

