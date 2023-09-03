#pragma once

#include <Arduino.h>
#include "axis.hpp"

#define ESTOP_OFF_MANUAL_MOVE 5

extern hw_timer_t *async_timer;

void setEmergencyStop(int kind);
void setAsyncTimerEnable(bool value);
void setIsOnFromTask(bool on);
void applySettings();
void taskMotorControl ();
