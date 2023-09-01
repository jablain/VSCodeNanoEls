// https://github.com/kachurovskiy/nanoels

#include <Arduino.h>
#include <atomic>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Preferences.h>
#include "tasks.hpp"
#include "vars.hpp"
#include "config.hpp"
#include "pcbpins.hpp"
#include "keypad.hpp"
#include "preferences.hpp"
#include "modes.hpp"
#include "macros.hpp"
#include "display.hpp"
#include "axis.hpp"
#include "spindle.hpp"

bool stepperIsRunning(Axis* a) {
  unsigned long nowUs = micros();
  return nowUs > a->stepStartUs ? nowUs - a->stepStartUs < 50000 : nowUs < 25000;
}

Axis* getPitchAxis() {
  return mode == MODE_FACE ? &x : &z;
}

bool saveIfChanged() {
  // Should avoid calling Preferences whenever possible to reduce memory wear and avoid ~20ms write delay that blocks interrupts.
  if (dupr == savedDupr && starts == savedStarts && z.pos == z.savedPos && z.originPos == z.savedOriginPos && z.posGlobal == z.savedPosGlobal && z.motorPos == z.savedMotorPos && z.leftStop == z.savedLeftStop && z.rightStop == z.savedRightStop && z.disabled == z.savedDisabled &&
      spindlePos == savedSpindlePos && spindlePosAvg == savedSpindlePosAvg && spindlePosSync == savedSpindlePosSync && savedSpindlePosGlobal == spindlePosGlobal && showAngle == savedShowAngle && showTacho == savedShowTacho && moveStep == savedMoveStep &&
      mode == savedMode && measure == savedMeasure && x.pos == x.savedPos && x.originPos == x.savedOriginPos && x.posGlobal == x.savedPosGlobal && x.motorPos == x.savedMotorPos && x.leftStop == x.savedLeftStop && x.rightStop == x.savedRightStop && x.disabled == x.savedDisabled &&
      a1.pos == a1.savedPos && a1.originPos == a1.savedOriginPos && a1.posGlobal == a1.savedPosGlobal && a1.motorPos == a1.savedMotorPos && a1.leftStop == a1.savedLeftStop && a1.rightStop == a1.savedRightStop && a1.disabled == a1.savedDisabled &&
      coneRatio == savedConeRatio && turnPasses == savedTurnPasses && savedAuxForward == auxForward) return false;

  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  if (dupr != savedDupr) pref.putLong(PREF_DUPR, savedDupr = dupr);
  if (starts != savedStarts) pref.putInt(PREF_STARTS, savedStarts = starts);
  if (z.pos != z.savedPos) pref.putLong(PREF_POS_Z, z.savedPos = z.pos);
  if (z.posGlobal != z.savedPosGlobal) pref.putLong(PREF_POS_GLOBAL_Z, z.savedPosGlobal = z.posGlobal);
  if (z.originPos != z.savedOriginPos) pref.putLong(PREF_ORIGIN_POS_Z, z.savedOriginPos = z.originPos);
  if (z.motorPos != z.savedMotorPos) pref.putLong(PREF_MOTOR_POS_Z, z.savedMotorPos = z.motorPos);
  if (z.leftStop != z.savedLeftStop) pref.putLong(PREF_LEFT_STOP_Z, z.savedLeftStop = z.leftStop);
  if (z.rightStop != z.savedRightStop) pref.putLong(PREF_RIGHT_STOP_Z, z.savedRightStop = z.rightStop);
  if (z.disabled != z.savedDisabled) pref.putBool(PREF_DISABLED_Z, z.savedDisabled = z.disabled);
  if (spindlePos != savedSpindlePos) pref.putLong(PREF_SPINDLE_POS, savedSpindlePos = spindlePos);
  if (spindlePosAvg != savedSpindlePosAvg) pref.putLong(PREF_SPINDLE_POS_AVG, savedSpindlePosAvg = spindlePosAvg);
  if (spindlePosSync != savedSpindlePosSync) pref.putInt(PREF_OUT_OF_SYNC, savedSpindlePosSync = spindlePosSync);
  if (spindlePosGlobal != savedSpindlePosGlobal) pref.putLong(PREF_SPINDLE_POS_GLOBAL, savedSpindlePosGlobal = spindlePosGlobal);
  if (showAngle != savedShowAngle) pref.putBool(PREF_SHOW_ANGLE, savedShowAngle = showAngle);
  if (showTacho != savedShowTacho) pref.putBool(PREF_SHOW_TACHO, savedShowTacho = showTacho);
  if (moveStep != savedMoveStep) pref.putLong(PREF_MOVE_STEP, savedMoveStep = moveStep);
  if (mode != savedMode) pref.putInt(PREF_MODE, savedMode = mode);
  if (measure != savedMeasure) pref.putInt(PREF_MEASURE, savedMeasure = measure);
  if (x.pos != x.savedPos) pref.putLong(PREF_POS_X, x.savedPos = x.pos);
  if (x.posGlobal != x.savedPosGlobal) pref.putLong(PREF_POS_GLOBAL_X, x.savedPosGlobal = x.posGlobal);
  if (x.originPos != x.savedOriginPos) pref.putLong(PREF_ORIGIN_POS_X, x.savedOriginPos = x.originPos);
  if (x.motorPos != x.savedMotorPos) pref.putLong(PREF_MOTOR_POS_X, x.savedMotorPos = x.motorPos);
  if (x.leftStop != x.savedLeftStop) pref.putLong(PREF_LEFT_STOP_X, x.savedLeftStop = x.leftStop);
  if (x.rightStop != x.savedRightStop) pref.putLong(PREF_RIGHT_STOP_X, x.savedRightStop = x.rightStop);
  if (x.disabled != x.savedDisabled) pref.putBool(PREF_DISABLED_X, x.savedDisabled = x.disabled);
  if (a1.pos != a1.savedPos) pref.putLong(PREF_POS_A1, a1.savedPos = a1.pos);
  if (a1.posGlobal != a1.savedPosGlobal) pref.putLong(PREF_POS_GLOBAL_A1, a1.savedPosGlobal = a1.posGlobal);
  if (a1.originPos != a1.savedOriginPos) pref.putLong(PREF_ORIGIN_POS_A1, a1.savedOriginPos = a1.originPos);
  if (a1.motorPos != a1.savedMotorPos) pref.putLong(PREF_MOTOR_POS_A1, a1.savedMotorPos = a1.motorPos);
  if (a1.leftStop != a1.savedLeftStop) pref.putLong(PREF_LEFT_STOP_A1, a1.savedLeftStop = a1.leftStop);
  if (a1.rightStop != a1.savedRightStop) pref.putLong(PREF_RIGHT_STOP_A1, a1.savedRightStop = a1.rightStop);
  if (a1.disabled != a1.savedDisabled) pref.putBool(PREF_DISABLED_A1, a1.savedDisabled = a1.disabled);
  if (coneRatio != savedConeRatio) pref.putFloat(PREF_CONE_RATIO, savedConeRatio = coneRatio);
  if (turnPasses != savedTurnPasses) pref.putInt(PREF_TURN_PASSES, savedTurnPasses = turnPasses);
  if (auxForward != savedAuxForward) pref.putBool(PREF_AUX_FORWARD, savedAuxForward = auxForward);
  pref.end();
  return true;
}

void taskDisplay(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    updateDisplay();
    // Calling Preferences.commit() blocks all interrupts for 30ms, don't call saveIfChanged() if
    // encoder is likely to move soon.
    unsigned long now = micros();
    if (!stepperIsRunning(&z) && !stepperIsRunning(&x) && (now > spindleEncTime + SAVE_DELAY_US) && (now < saveTime || now > saveTime + SAVE_DELAY_US) && (now < keypadTimeUs || now > keypadTimeUs + SAVE_DELAY_US)) {
      if (saveIfChanged()) {
        saveTime = now;
      }
    }
    if (beepFlag) {
      beepFlag = false;
      beep();
    }
    if (abs(z.pendingPos) > z.estopSteps || abs(x.pendingPos) > x.estopSteps) {
      setEmergencyStop(ESTOP_POS);
    }
    taskYIELD();
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EMERGENCY STOP");
  lcd.setCursor(0, 1);
  if (emergencyStop == ESTOP_KEY) {
    lcd.print("Key down at power-up");
    lcd.setCursor(0, 2);
    lcd.print("Hardware failure?");
  } else if (emergencyStop == ESTOP_POS) {
    lcd.print("Requested position");
    lcd.setCursor(0, 2);
    lcd.print("outside machine");
  } else if (emergencyStop == ESTOP_MARK_ORIGIN) {
    lcd.print("Unable to");
    lcd.setCursor(0, 2);
    lcd.print("mark origin");
  } else if (emergencyStop == ESTOP_ON_OFF) {
    lcd.print("Unable to");
    lcd.setCursor(0, 2);
    lcd.print("turn on/off");
  } else if (emergencyStop == ESTOP_OFF_MANUAL_MOVE) {
    lcd.print("Off during");
    lcd.setCursor(0, 2);
    lcd.print("manual move");
  }
  vTaskDelete(NULL);
}

void taskKeypad(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    processKeypadEvent();
    taskYIELD();
  }
  vTaskDelete(NULL);
}

void waitForPendingPosNear0(Axis* a) {
  while (abs(a->pendingPos) > a->motorSteps / 3) {
    taskYIELD();
  }
}

void waitForPendingPos0(Axis* a) {
  while (a->pendingPos != 0) {
    taskYIELD();
  }
}

bool isContinuousStep() {
  return moveStep == (measure == MEASURE_METRIC ? MOVE_STEP_1 : MOVE_STEP_IMP_1);
}

// For rotational axis the moveStep of 0.1" means 0.1°.
long getMoveStepForAxis(Axis* a) {
  return (a->rotational && measure != MEASURE_METRIC) ? (moveStep / 25.4) : moveStep;
}

long getStepMaxSpeed(Axis* a) {
  return isContinuousStep() ? a->speedManualMove : min(long(a->speedManualMove), abs(getMoveStepForAxis(a)) * 1000 / STEP_TIME_MS);
}

void waitForStep(Axis* a) {
  if (isContinuousStep()) {
    // Move continuously for default step.
    waitForPendingPosNear0(a);
  } else {
    // Move with tiny pauses allowing to stop precisely.
    a->continuous = false;
    waitForPendingPos0(a);
    DELAY(DELAY_BETWEEN_STEPS_MS);
  }
}

int getAndResetPulses(Axis* a) {
  int delta = 0;
  if (PULSE_1_AXIS == a->name) {
    if (pulse1Delta < -PULSE_HALF_BACKLASH) {
      noInterrupts();
      delta = pulse1Delta + PULSE_HALF_BACKLASH;
      pulse1Delta = -PULSE_HALF_BACKLASH;
      interrupts();
    } else if (pulse1Delta > PULSE_HALF_BACKLASH) {
      noInterrupts();
      delta = pulse1Delta - PULSE_HALF_BACKLASH;
      pulse1Delta = PULSE_HALF_BACKLASH;
      interrupts();
    }
  } else if (PULSE_2_AXIS == a->name) {
    if (pulse2Delta < -PULSE_HALF_BACKLASH) {
      noInterrupts();
      delta = pulse2Delta + PULSE_HALF_BACKLASH;
      pulse2Delta = -PULSE_HALF_BACKLASH;
      interrupts();
    } else if (pulse2Delta > PULSE_HALF_BACKLASH) {
      noInterrupts();
      delta = pulse2Delta - PULSE_HALF_BACKLASH;
      pulse2Delta = PULSE_HALF_BACKLASH;
      interrupts();
    }
  }
  return delta;
}

void stepperEnable(Axis* a, bool value) {
  if (!a->needsRest || !a->active) {
    return;
  }
  if (value) {
    a->stepperEnableCounter++;
    if (value == 1) {
      updateEnable(a);
    }
  } else if (a->stepperEnableCounter > 0) {
    a->stepperEnableCounter--;
    if (a->stepperEnableCounter == 0) {
      updateEnable(a);
    }
  }
}

// Calculates stepper position from spindle position.
long posFromSpindle(Axis* a, long s, bool respectStops) {
  long newPos = s * a->motorSteps / a->screwPitch / ENCODER_STEPS_FLOAT * dupr * starts;

  // Respect left/right stops.
  if (respectStops) {
    if (newPos < a->rightStop) {
      newPos = a->rightStop;
    } else if (newPos > a->leftStop) {
      newPos = a->leftStop;
    }
  }

  return newPos;
}

void markAxisOrigin(Axis* a) {
  bool hasSemaphore = xSemaphoreTake(a->mutex, 10) == pdTRUE;
  if (!hasSemaphore) {
    beepFlag = true;
  }
  if (a->leftStop != LONG_MAX) {
    a->leftStop -= a->pos;
  }
  if (a->rightStop != LONG_MIN) {
    a->rightStop -= a->pos;
  }
  a->motorPos -= a->pos;
  a->originPos += a->pos;
  a->pos = 0;
  a->fractionalPos = 0;
  a->pendingPos = 0;
  if (hasSemaphore) {
    xSemaphoreGive(a->mutex);
  }
}

// Loose the thread and mark current physical positions of
// encoder and stepper as a new 0. To be called when dupr changes
// or ELS is turned on/off. Without this, changing dupr will
// result in stepper rushing across the lathe to the new position.
// Must be called while holding motionMutex.
void markOrigin() {
  markAxisOrigin(&z);
  markAxisOrigin(&x);
  markAxisOrigin(&a1);
  zeroSpindlePos();
}

void setDir(Axis* a, bool dir) {
  // Start slow if direction changed.
  if (a->direction != dir || !a->directionInitialized) {
    a->speed = a->speedStart;
    a->direction = dir;
    a->directionInitialized = true;
    DWRITE(a->dir, dir ^ a->invertStepper);
    delayMicroseconds(DIRECTION_SETUP_DELAY_US);
  }
}

unsigned int getTimerLimit() {
  if (dupr == 0) {
    return 65535;
  }
  return min(long(65535), long(1000000 / (z.motorSteps * abs(dupr) / z.screwPitch)) - 1); // 1000000/Hz - 1
}

Axis* getAsyncAxis() {
  return mode == MODE_A1 ? &a1 : &z;
}

void updateAsyncTimerSettings() {
  // dupr and therefore direction can change while we're in async mode.
  setDir(getAsyncAxis(), dupr > 0);

  // dupr can change while we're in async mode, keep updating timer frequency.
  timerAlarmWrite(async_timer, getTimerLimit(), true);
  // without this timer stops working if already above new limit
  timerWrite(async_timer, 0);
}

void taskMoveZ(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    int pulseDelta = getAndResetPulses(&z);
    bool left = buttonLeftPressed;
    bool right = buttonRightPressed;
    if (!left && !right && pulseDelta == 0) {
      taskYIELD();
      continue;
    }
    if (spindlePosSync != 0) {
      // Edge case.
      taskYIELD();
      continue;
    }
    if (isOn && isPassMode()) {
      setIsOnFromTask(false);
    }
    int sign = pulseDelta == 0 ? (left ? 1 : -1) : (pulseDelta > 0 ? 1 : -1);
    bool stepperOn = true;
    stepperEnable(&z, true);
    z.movingManually = true;
    if (isOn && dupr != 0 && mode == MODE_NORMAL) {
      // Move by moveStep in the desired direction but stay in the thread by possibly traveling a little more.
      int diff = ceil(moveStep * 1.0 / abs(dupr * starts)) * ENCODER_STEPS_FLOAT * sign * (dupr > 0 ? 1 : -1);
      long prevSpindlePos = spindlePos;
      bool resting = false;
      do {
        z.speedMax = z.speedManualMove;
        if (xSemaphoreTake(motionMutex, 100) == pdTRUE) {
          if (!resting) {
            spindlePos += diff;
            spindlePosAvg += diff;
          }
          // If spindle is moving, it will be changing spindlePos at the same time. Account for it.
          while (diff > 0 ? (spindlePos < prevSpindlePos) : (spindlePos > prevSpindlePos)) {
            spindlePos += diff;
            spindlePosAvg += diff;
          };
          prevSpindlePos = spindlePos;
          xSemaphoreGive(motionMutex);
        }

        long newPos = posFromSpindle(&z, prevSpindlePos, true);
        if (newPos != z.pos) {
          stepToContinuous(&z, newPos);
          waitForPendingPosNear0(&z);
        } else if (z.pos == (left ? z.leftStop : z.rightStop)) {
          // We're standing on a stop with the L/R move button pressed.
          resting = true;
          if (stepperOn) {
            stepperEnable(&z, false);
            stepperOn = false;
          }
          DELAY(200);
        }
      } while (left ? buttonLeftPressed : buttonRightPressed);
    } else {
      z.speedMax = getStepMaxSpeed(&z);
      int delta = 0;
      do {
        float fractionalDelta = (pulseDelta == 0 ? moveStep * sign / z.screwPitch : pulseDelta / PULSE_PER_REVOLUTION) * z.motorSteps + z.fractionalPos;
        delta = round(fractionalDelta);
        // Don't lose fractional steps when moving by 0.01" or 0.001".
        z.fractionalPos = fractionalDelta - delta;
        if (delta == 0) {
          // When moveStep is e.g. 1 micron and MOTOR_STEPS_Z is 200, make delta non-zero.
          delta = sign;
        }

        long posCopy = z.pos + z.pendingPos;
        // Don't left-right move out of stops.
        if (posCopy + delta > z.leftStop) {
          delta = z.leftStop - posCopy;
        } else if (posCopy + delta < z.rightStop) {
          delta = z.rightStop - posCopy;
        }
        z.speedMax = getStepMaxSpeed(&z);
        stepToContinuous(&z, posCopy + delta);
        waitForStep(&z);
      } while (delta != 0 && (left ? buttonLeftPressed : buttonRightPressed));
      z.continuous = false;
      waitForPendingPos0(&z);
      if (isOn && mode == MODE_CONE) {
        if (xSemaphoreTake(motionMutex, 100) != pdTRUE) {
          setEmergencyStop(ESTOP_MARK_ORIGIN);
        } else {
          markOrigin();
          xSemaphoreGive(motionMutex);
        }
      } else if (isOn && mode == MODE_ASYNC) {
        // Restore async direction.
        updateAsyncTimerSettings();
      }
    }
    z.movingManually = false;
    if (stepperOn) {
      stepperEnable(&z, false);
    }
    z.speedMax = LONG_MAX;
    taskYIELD();
  }
  vTaskDelete(NULL);
}

void taskMoveX(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    int pulseDelta = getAndResetPulses(&x);
    bool up = buttonUpPressed || pulseDelta > 0;
    bool down = buttonDownPressed || pulseDelta < 0;
    if (!up && !down) {
      taskYIELD();
      continue;
    }
    if (isOn && isPassMode()) {
      setIsOnFromTask(false);
    }
    x.movingManually = true;
    x.speedMax = getStepMaxSpeed(&x);
    stepperEnable(&x, true);

    int delta = 0;
    int sign = up ? 1 : -1;
    do {
      float fractionalDelta = (pulseDelta == 0 ? moveStep * sign / x.screwPitch : pulseDelta / PULSE_PER_REVOLUTION) * x.motorSteps + x.fractionalPos;
      delta = round(fractionalDelta);
      // Don't lose fractional steps when moving by 0.01" or 0.001".
      x.fractionalPos = fractionalDelta - delta;
      if (delta == 0) {
        // When moveStep is e.g. 1 micron and MOTOR_STEPS_Z is 200, make delta non-zero.
        delta = sign;
      }

      long posCopy = x.pos + x.pendingPos;
      if (posCopy + delta > x.leftStop) {
        delta = x.leftStop - posCopy;
      } else if (posCopy + delta < x.rightStop) {
        delta = x.rightStop - posCopy;
      }
      stepToContinuous(&x, posCopy + delta);
      waitForStep(&x);
      pulseDelta = getAndResetPulses(&x);
    } while (delta != 0 && (pulseDelta != 0 || (up ? buttonUpPressed : buttonDownPressed)));
    x.continuous = false;
    waitForPendingPos0(&x);
    if (isOn && mode == MODE_CONE) {
      if (xSemaphoreTake(motionMutex, 100) != pdTRUE) {
        setEmergencyStop(ESTOP_MARK_ORIGIN);
      } else {
        markOrigin();
        xSemaphoreGive(motionMutex);
      }
    }
    x.movingManually = false;
    x.speedMax = LONG_MAX;
    stepperEnable(&x, false);

    taskYIELD();
  }
  vTaskDelete(NULL);
}

void taskMoveA1(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    bool plus = buttonTurnPressed;
    bool minus = buttonGearsPressed;
    if (mode != MODE_A1 || (!plus && !minus)) {
      taskYIELD();
      continue;
    }
    a1.movingManually = true;
    a1.speedMax = getStepMaxSpeed(&a1);
    stepperEnable(&a1, true);

    int delta = 0;
    int sign = plus ? 1 : -1;
    do {
      float fractionalDelta = getMoveStepForAxis(&a1) * sign / a1.screwPitch * a1.motorSteps + a1.fractionalPos;
      delta = round(fractionalDelta);
      a1.fractionalPos = fractionalDelta - delta;
      if (delta == 0) delta = sign;

      long posCopy = a1.pos + a1.pendingPos;
      if (posCopy + delta > a1.leftStop) {
        delta = a1.leftStop - posCopy;
      } else if (posCopy + delta < a1.rightStop) {
        delta = a1.rightStop - posCopy;
      }
      stepToContinuous(&a1, posCopy + delta);
      waitForStep(&a1);
    } while (plus ? buttonTurnPressed : buttonGearsPressed);
    a1.continuous = false;
    waitForPendingPos0(&a1);
    // Restore async direction.
    if (isOn && mode == MODE_A1) updateAsyncTimerSettings();
    a1.movingManually = false;
    a1.speedMax = LONG_MAX;
    stepperEnable(&a1, false);
    taskYIELD();
  }
  vTaskDelete(NULL);
}

String getValueString(const String& command, char letter) {
  int index = command.indexOf(letter);
  if (index == -1) {
    return "";
  }
  String valueString;
  for (int i = index + 1; i < command.length(); i++) {
    char c = command.charAt(i);
    if (isDigit(c) || c == '.' || c == '-') {
      valueString += c;
    } else {
      break;
    }
  }
  return valueString;
}

float getFloat(const String& command, char letter) {
  return getValueString(command, letter).toFloat();
}

int getInt(const String& command, char letter) {
  return getValueString(command, letter).toInt();
}

void setFeedRate(const String& command) {
  float feed = getFloat(command, 'F');
  if (feed <= 0) return;
  gcodeFeedDuPerSec = round(feed * (measure == MEASURE_METRIC ? 10000 : 254000) / 60.0);
}

long mmOrInchToAbsolutePos(Axis* a, float mmOrInch) {
  long scaleToDu = measure == MEASURE_METRIC ? 10000 : 254000;
  long part1 = a->gcodeRelativePos;
  long part2 = round(mmOrInch * scaleToDu / a->screwPitch * a->motorSteps);
  return part1 + part2;
}

void updateAxisSpeeds(long diffX, long diffZ, long diffA1) {
  if (diffX == 0 && diffZ == 0 && diffA1 == 0) return;
  long absX = abs(diffX);
  long absZ = abs(diffZ);
  long absC = abs(diffA1);
  float stepsPerSecX = gcodeFeedDuPerSec * x.motorSteps / x.screwPitch;
  float minStepsPerSecX = GCODE_FEED_MIN_DU_SEC * x.motorSteps / x.screwPitch;
  if (stepsPerSecX > x.speedManualMove) stepsPerSecX = x.speedManualMove;
  else if (stepsPerSecX < minStepsPerSecX) stepsPerSecX = minStepsPerSecX;
  float stepsPerSecZ = gcodeFeedDuPerSec * z.motorSteps / z.screwPitch;
  float minStepsPerSecZ = GCODE_FEED_MIN_DU_SEC * z.motorSteps / z.screwPitch;
  if (stepsPerSecZ > z.speedManualMove) stepsPerSecZ = z.speedManualMove;
  else if (stepsPerSecZ < minStepsPerSecZ) stepsPerSecZ = minStepsPerSecZ;
  float stepsPerSecA1 = gcodeFeedDuPerSec * a1.motorSteps / a1.screwPitch;
  float minStepsPerSecA1 = GCODE_FEED_MIN_DU_SEC * a1.motorSteps / a1.screwPitch;
  if (stepsPerSecA1 > a1.speedManualMove) stepsPerSecA1 = a1.speedManualMove;
  else if (stepsPerSecA1 < minStepsPerSecA1) stepsPerSecA1 = minStepsPerSecA1;
  float secX = absX / stepsPerSecX;
  float secZ = absZ / stepsPerSecZ;
  float secA1 = absC / stepsPerSecA1;
  float sec = ACTIVE_A1 ? max(max(secX, secZ), secA1) : max(secX, secZ);
  x.speedMax = sec > 0 ? absX / sec : x.speedManualMove;
  z.speedMax = sec > 0 ? absZ / sec : z.speedManualMove;
  a1.speedMax = sec > 0 ? absC / sec : a1.speedManualMove;
}

void gcodeWaitEpsilon(int epsilon) {
  while (abs(x.pendingPos) > epsilon || abs(z.pendingPos) > epsilon || abs(a1.pendingPos) > epsilon) {
    taskYIELD();
  }
}

void gcodeWaitNear() {
  gcodeWaitEpsilon(GCODE_WAIT_EPSILON_STEPS);
}

void gcodeWaitStop() {
  gcodeWaitEpsilon(0);
}

// Rapid positioning / linear interpolation.
void G00_01(const String& command) {
  long xStart = x.pos;
  long zStart = z.pos;
  long a1Start = a1.pos;
  long xEnd = command.indexOf(x.name) >= 0 ? mmOrInchToAbsolutePos(&x, getFloat(command, x.name)) : xStart;
  long zEnd = command.indexOf(z.name) >= 0 ? mmOrInchToAbsolutePos(&z, getFloat(command, z.name)) : zStart;
  long a1End = command.indexOf(a1.name) >= 0 ? mmOrInchToAbsolutePos(&a1, getFloat(command, a1.name)) : a1Start;
  long xDiff = xEnd - xStart;
  long zDiff = zEnd - zStart;
  long a1Diff = a1End - a1Start;
  updateAxisSpeeds(xDiff, zDiff, a1Diff);
  long chunks = round(max(max(abs(xDiff), abs(zDiff)), abs(a1Diff)) * LINEAR_INTERPOLATION_PRECISION);
  for (long i = 0; i < chunks; i++) {
    if (!isOn) return;
    float scale = i / float(chunks);
    stepToContinuous(&x, xStart + xDiff * scale);
    stepToContinuous(&z, zStart + zDiff * scale);
    if (ACTIVE_A1) stepToContinuous(&a1, a1Start + a1Diff * scale);
    gcodeWaitNear();
  }
  // To avoid any rounding error, move to precise position.
  stepToFinal(&x, xEnd);
  stepToFinal(&z, zEnd);
  if (ACTIVE_A1) stepToFinal(&a1, a1End);
  gcodeWaitStop();
}

bool handleGcode(const String& command) {
  int op = getInt(command, 'G');
  if (op == 0 || op == 1) { // 0 also covers X and Z commands without G.
    G00_01(command);
  } else if (op == 20 || op == 21) {
    setMeasure(op == 20 ? MEASURE_INCH : MEASURE_METRIC);
  } else if (op == 90 || op == 91) {
    gcodeAbsolutePositioning = op == 90;
  } else if (op == 94) {
    /* no-op feed per minute */
  } else if (op == 18) {
    /* no-op ZX plane selection */
  } else {
    Serial.print("error: unsupported command ");
    Serial.println(command);
    return false;
  }
  return true;
}

bool handleMcode(const String& command) {
  int op = getInt(command, 'M');
  if (op == 0 || op == 1 || op == 2 || op == 30) {
    setIsOnFromTask(false);
  } else {
    setIsOnFromTask(false);
    Serial.print("error: unsupported command ");
    Serial.println(command);
    return false;
  }
  return true;
}

// Process one command, return ok flag.
bool handleGcodeCommand(String command) {
  command.trim();
  if (command.length() == 0) return false;

  // Trim N.. prefix.
  char code = command.charAt(0);
  int spaceIndex = command.indexOf(' ');
  if (code == 'N' && spaceIndex > 0) {
    command = command.substring(spaceIndex + 1);
    code = command.charAt(0);
  }

  // Update position for relative calculations right before performing them.
  z.gcodeRelativePos = gcodeAbsolutePositioning ? -z.originPos : z.pos;
  x.gcodeRelativePos = gcodeAbsolutePositioning ? -x.originPos : x.pos;
  a1.gcodeRelativePos = gcodeAbsolutePositioning ? -a1.originPos : a1.pos;

  setFeedRate(command);
  switch (code) {
    case 'G':
    case NAME_Z:
    case NAME_X:
    case NAME_A1: return handleGcode(command);
    case 'F': return true; /* feed already handled above */
    case 'M': return handleMcode(command);
    case 'T': return true; /* ignoring tool changes */
    default: Serial.print("error: unsupported command "); Serial.println(code); return false;
  }
  return false;
}

void taskGcode(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    if (mode != MODE_GCODE) {
      gcodeInitialized = false;
      taskYIELD();
      continue;
    }
    if (!gcodeInitialized) {
      gcodeInitialized = true;
      gcodeCommand = "";
      gcodeAbsolutePositioning = true;
      gcodeFeedDuPerSec = GCODE_FEED_DEFAULT_DU_SEC;
      gcodeInBrace = false;
      gcodeInSemicolon = false;
    }
    // Implementing a relevant subset of RS274 (Gcode) and GRBL (state management) covering basic use cases.
    if (Serial.available() > 0) {
      char receivedChar = Serial.read();
      int charCode = int(receivedChar);
      if (gcodeInBrace) {
        if (receivedChar == ')') gcodeInBrace = false;
      } else if (receivedChar == '(') {
        gcodeInBrace = true;
      } else if (receivedChar == ';' /* start of comment till end of line */) {
        gcodeInSemicolon = true;
      } else if (gcodeInSemicolon && charCode >= 32) {
        // Ignoring comment.
      } else if (receivedChar == '!' /* stop */) {
        setIsOnFromTask(false);
      } else if (receivedChar == '~' /* resume */) {
        setIsOnFromTask(true);
      } else if (receivedChar == '%' /* start/end marker */) {
        // Not using % markers in this implementation.
      } else if (receivedChar == '?' /* status */) {
        Serial.print("<");
        Serial.print(isOn ? "Run" : "Idle");
        Serial.print("|WPos:");
        float divisor = measure == MEASURE_METRIC ? 10000.0 : 254000.0;
        Serial.print(getAxisPosDu(&x) / divisor, 3);
        Serial.print(",0.000,");
        Serial.print(getAxisPosDu(&z) / divisor, 3);
        Serial.print("|FS:");
        Serial.print(round(gcodeFeedDuPerSec * 60 / 10000.0));
        Serial.print(",");
        Serial.print(getApproxRpm());
        Serial.print(">"); // no new line to allow client to easily cut out the status response
      } else if (isOn) {
        if (gcodeInBrace && charCode < 32) {
          Serial.println("error: comment not closed");
          setIsOnFromTask(false);
        } else if (charCode < 32 && gcodeCommand.length() > 1) {
          if (handleGcodeCommand(gcodeCommand)) Serial.println("ok");
          gcodeCommand = "";
          gcodeInSemicolon = false;
        } else if (charCode < 32) {
          Serial.println("ok");
          gcodeCommand = "";
        } else if (charCode >= 32 && (charCode == 'G' || charCode == 'M')) {
          // Split consequent G and M commands on one line.
          // No "ok" for commands in the middle of the line.
          handleGcodeCommand(gcodeCommand);
          gcodeCommand = receivedChar;
        } else if (charCode >= 32) {
          gcodeCommand += receivedChar;
        }
      } else {
        // ignoring non-realtime command input when off
        // to flush any commands coming after an error
      }
    }
    taskYIELD();
  }
  vTaskDelete(NULL);
}

void taskAttachInterrupts(void *param) {
  // Attaching interrupt on core 0 to have more time on core 1 where axes are moved.
  spindlePosDelta = 0; // Unprocessed encoder ticks.
  attachInterrupt(digitalPinToInterrupt(ENC_A), spinEnc, FALLING);
  if (PULSE_1_USE) attachInterrupt(digitalPinToInterrupt(A12), pulse1Enc, CHANGE);
  if (PULSE_2_USE) attachInterrupt(digitalPinToInterrupt(A22), pulse2Enc, CHANGE);
  vTaskDelete(NULL);
}

void setIsOnFromLoop(bool on) {
  if (isOn && on) {
    return;
  }
  if (!on) {
    isOn = false;
    setupIndex = 0;
  }
  stepperEnable(&z, on);
  stepperEnable(&x, on);
  stepperEnable(&a1, on);
  markOrigin();
  if (on) {
    isOn = true;
    opDuprSign = dupr >= 0 ? 1 : -1;
    opDupr = dupr;
    opIndex = 0;
    opIndexAdvanceFlag = false;
    opSubIndex = 0;
    setupIndex = 0;
  }
}

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

  DLOW(a->step);
  a->stepStartUs = micros();
  delayMicroseconds(10);
  DHIGH(a->step);
}

void setModeFromLoop(int value) {
  if (mode == value) {
    return;
  }
  if (isOn) {
    setIsOnFromLoop(false);
  }
  if (mode == MODE_THREAD) {
    setStarts(1);
  } else if (mode == MODE_ASYNC || mode == MODE_A1) {
    setAsyncTimerEnable(false);
  }
  mode = value;
  setupIndex = 0;
  if (mode == MODE_ASYNC || mode == MODE_A1) {
    if (!timerAttached) {
      timerAttached = true;
      timerAttachInterrupt(async_timer, &onAsyncTimer, true);
    }
    updateAsyncTimerSettings();
    setAsyncTimerEnable(true);
  }
}

// Must be called while holding motionMutex.
void applyDupr() {
  if (nextDupr == dupr) {
    return;
  }
  dupr = nextDupr;
  markOrigin();
  if (mode == MODE_ASYNC || mode == MODE_A1) {
    updateAsyncTimerSettings();
  }
}

// Must be called while holding motionMutex.
void applyStarts() {
  if (starts == nextStarts) {
    return;
  }
  starts = nextStarts;
  markOrigin();
}

void applyConeRatio() {
  if (nextConeRatio == coneRatio) {
    return;
  }
  coneRatio = nextConeRatio;
  markOrigin();
}

// Calculates spindle position from stepper position.
long spindleFromPos(Axis* a, long p) {
  return p * a->screwPitch * ENCODER_STEPS_FLOAT / a->motorSteps / (dupr * starts);
}

void leaveStop(Axis* a, long oldStop) {
  if (mode == MODE_CONE) {
    // To avoid rushing to a far away position if standing on limit.
    markOrigin();
  } else if (mode == MODE_NORMAL && a == getPitchAxis() && a->pos == oldStop) {
    // Spindle is most likely out of sync with the stepper because
    // it was spinning while the lead screw was on the stop.
    spindlePosSync = spindleModulo(spindlePos - spindleFromPos(a, a->pos));
  }
}

void applyLeftStop(Axis* a) {
  // Accept left stop even if it's lower than pos.
  // Stop button press processing takes time during which motor could have moved.
  long oldStop = a->leftStop;
  a->leftStop = a->nextLeftStop;
  leaveStop(a, oldStop);
}

void applyRightStop(Axis* a) {
  // Accept right stop even if it's higher than pos.
  // Stop button press processing takes time during which motor could have moved.
  long oldStop = a->rightStop;
  a->rightStop = a->nextRightStop;
  leaveStop(a, oldStop);
}

void moveAxis(Axis* a) {
  // Most of the time a step isn't needed.
  if (a->pendingPos == 0) {
    if (a->speed > a->speedStart) {
      a->speed--;
    }
    return;
  }

  unsigned long nowUs = micros();
  float delayUs = 1000000.0 / a->speed;
  if (nowUs < a->stepStartUs) a->stepStartUs = 0; // micros() overflow
  if (nowUs < (a->stepStartUs + delayUs - 5)) {
    // Not enough time has passed to issue this step.
    return;
  }

  if (xSemaphoreTake(a->mutex, 1) == pdTRUE) {
    // Check pendingPos again now that we have the mutex.
    if (a->pendingPos != 0) {
      bool dir = a->pendingPos > 0;
      setDir(a, dir);

      DLOW(a->step);
      int delta = dir ? 1 : -1;
      a->pendingPos -= delta;
      if (dir && a->motorPos >= a->pos) {
        a->pos++;
      } else if (!dir && a->motorPos <= (a->pos - a->backlashSteps)) {
        a->pos--;
      }
      a->motorPos += delta;
      a->posGlobal += delta;

      bool accelerate = a->continuous || a->pendingPos >= a->decelerateSteps || a->pendingPos <= -a->decelerateSteps;
      a->speed += (accelerate ? 1 : -1) * a->acceleration * delayUs / 1000000.0;
      if (a->speed > a->speedMax) {
        a->speed = a->speedMax;
      } else if (a->speed < a->speedStart) {
        a->speed = a->speedStart;
      }
      a->stepStartUs = nowUs;

      DHIGH(a->step);
    }
    xSemaphoreGive(a->mutex);
  }
}

void modeGearbox() {
  if (z.movingManually) {
    return;
  }
  z.speedMax = LONG_MAX;
  stepToContinuous(&z, posFromSpindle(&z, spindlePosAvg, true));
}

long auxSafeDistance, startOffset;
void modeTurn(Axis* main, Axis* aux) {
  if (main->movingManually || aux->movingManually || turnPasses <= 0 ||
      main->leftStop == LONG_MAX || main->rightStop == LONG_MIN ||
      aux->leftStop == LONG_MAX || aux->rightStop == LONG_MIN ||
      dupr == 0 || (dupr * opDuprSign < 0) || starts < 1) {
    setIsOnFromLoop(false);
    return;
  }

  // Variables below have to be re-calculated every time because origin can change
  // while TURN is running e.g. due to dupr change.
  long mainStartStop = opDuprSign > 0 ? main->rightStop : main->leftStop;
  long mainEndStop = opDuprSign > 0 ? main->leftStop : main->rightStop;
  long auxStartStop = auxForward ? aux->rightStop : aux->leftStop;
  long auxEndStop = auxForward ? aux->leftStop : aux->rightStop;

  // opIndex 0 is only executed once, do setup calculations here.
  if (opIndex == 0) {
    auxSafeDistance = (auxForward ? -1 : 1) * SAFE_DISTANCE_DU * aux->motorSteps / aux->screwPitch;
    startOffset = starts == 1 ? 0 : round(ENCODER_STEPS_FLOAT / starts);

    // Move to right-bottom limit.
    main->speedMax = main->speedManualMove;
    aux->speedMax = aux->speedManualMove;
    long auxPos = auxStartStop;
    // Overstep by 1 so that "main" backlash is taken out before "opSubIndex == 1".
    long mainPos = mainStartStop + (opDuprSign > 0 ? -1 : 1);
    stepToFinal(main, mainPos);
    stepToFinal(aux, auxPos);
    if (main->pos == mainPos && aux->pos == auxPos) {
      stepToFinal(main, mainStartStop);
      opIndex = 1;
      opSubIndex = 0;
    }
  } else if (opIndex <= turnPasses * starts) {
    if (opIndexAdvanceFlag && (opIndex + starts) < turnPasses * starts) {
      opIndexAdvanceFlag = false;
      opIndex += starts;
    }
    long auxPos = auxEndStop - (auxEndStop - auxStartStop) / turnPasses * (turnPasses - ceil(opIndex / float(starts)));
    // Bringing X to starting position.
    if (opSubIndex == 0) {
      stepToFinal(aux, auxPos);
      if (aux->pos == auxPos) {
        opSubIndex = 1;
        spindlePosSync = spindleModulo(spindlePosGlobal - spindleFromPos(main, main->posGlobal) + startOffset * (opIndex - 1));
        return; // Instead of jumping to the next step, let spindlePosSync get to 0 first.
      }
    }
    // spindlePosSync counted down to 0, start thread from here.
    if (opSubIndex == 1) {
      markOrigin();
      main->speedMax = LONG_MAX;
      opSubIndex = 2;
      // markOrigin() changed Start/EndStop values, re-calculate them.
      return;
    }
    // Doing the pass cut.
    if (opSubIndex == 2) {
      // In case we were pushed to the next opIndex before finishing the current one.
      stepToFinal(aux, auxPos);
      stepToContinuous(main, posFromSpindle(main, spindlePosAvg, true));
      if (main->pos == mainEndStop) {
        opSubIndex = 3;
      }
    }
    // Retracting the tool
    if (opSubIndex == 3) {
      long auxPos = auxStartStop + auxSafeDistance;
      stepToFinal(aux, auxPos);
      if (aux->pos == auxPos) {
        opSubIndex = 4;
      }
    }
    // Returning to start of main.
    if (opSubIndex == 4) {
      main->speedMax = main->speedManualMove;
      // Overstep by 1 so that "main" backlash is taken out before "opSubIndex == 2".
      long mainPos = mainStartStop + (opDuprSign > 0 ? -1 : 1);
      stepToFinal(main, mainPos);
      if (main->pos == mainPos) {
        stepToFinal(main, mainStartStop);
        opSubIndex = 0;
        opIndex++;
      }
    }
  } else {
    // Move to right-bottom limit.
    main->speedMax = main->speedManualMove;
    long auxPos = auxStartStop;
    long mainPos = mainStartStop;
    stepToFinal(main, mainPos);
    stepToFinal(aux, auxPos);
    if (main->pos == mainPos && aux->pos == auxPos) {
      setIsOnFromLoop(false);
      beep();
    }
  }
}

void modeCone() {
  if (z.movingManually || x.movingManually || coneRatio == 0) {
    return;
  }

  float zToXRatio = -coneRatio / 2 / z.motorSteps * x.motorSteps / x.screwPitch * z.screwPitch * (auxForward ? 1 : -1);
  if (zToXRatio == 0) {
    return;
  }

  // TODO: calculate maximum speeds and accelerations to avoid potential desync.
  x.speedMax = LONG_MAX;
  z.speedMax = LONG_MAX;

  // Respect limits of both axis by translating them into limits on spindlePos value.
  long spindle = spindlePosAvg;
  long spindleMin = LONG_MIN;
  long spindleMax = LONG_MAX;
  if (z.leftStop != LONG_MAX) {
    (dupr > 0 ? spindleMax : spindleMin) = spindleFromPos(&z, z.leftStop);
  }
  if (z.rightStop != LONG_MIN) {
    (dupr > 0 ? spindleMin: spindleMax) = spindleFromPos(&z, z.rightStop);
  }
  if (x.leftStop != LONG_MAX) {
    long lim = spindleFromPos(&z, round(x.leftStop / zToXRatio));
    if (zToXRatio < 0) {
      (dupr > 0 ? spindleMin: spindleMax) = lim;
    } else {
      (dupr > 0 ? spindleMax : spindleMin) = lim;
    }
  }
  if (x.rightStop != LONG_MIN) {
    long lim = spindleFromPos(&z, round(x.rightStop / zToXRatio));
    if (zToXRatio < 0) {
      (dupr > 0 ? spindleMax : spindleMin) = lim;
    } else {
      (dupr > 0 ? spindleMin: spindleMax) = lim;
    }
  }
  if (spindle > spindleMax) {
    spindle = spindleMax;
  } else if (spindle < spindleMin) {
    spindle = spindleMin;
  }

  stepToContinuous(&z, posFromSpindle(&z, spindle, true));
  stepToContinuous(&x, round(z.pos * zToXRatio));
}

void modeCut() {
  if (x.movingManually || turnPasses <= 0 || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN || dupr == 0 || dupr * opDuprSign < 0) {
    setIsOnFromLoop(false);
    return;
  }

  long startStop = opDuprSign > 0 ? x.rightStop : x.leftStop;
  long endStop = opDuprSign > 0 ? x.leftStop : x.rightStop;

  if (opIndex == 0) {
    // Move to back limit.
    x.speedMax = x.speedManualMove;
    long xPos = startStop;
    stepToFinal(&x, xPos);
    if (x.pos == xPos) {
      opIndex = 1;
      opSubIndex = 0;
    }
  } else if (opIndex <= turnPasses) {
    // Set spindlePos and x.pos in sync.
    if (opSubIndex == 0) {
      spindlePosAvg = spindlePos = spindleFromPos(&x, x.pos);
      opSubIndex = 1;
    }
    // Doing the pass cut.
    if (opSubIndex == 1) {
      x.speedMax = LONG_MAX;
      long endPos = endStop - (endStop - startStop) / turnPasses * (turnPasses - opIndex);
      long xPos = posFromSpindle(&x, spindlePosAvg, true);
      if (dupr > 0 && xPos > endPos) xPos = endPos;
      else if (dupr < 0 && xPos < endPos) xPos = endPos;
      stepToContinuous(&x, xPos);
      if (x.pos == endPos) {
        opSubIndex = 2;
      }
    }
    // Returning to start.
    if (opSubIndex == 2) {
      x.speedMax = x.speedManualMove;
      stepToFinal(&x, startStop);
      if (x.pos == startStop) {
        opSubIndex = 0;
        opIndex++;
      }
    }
  } else {
    setIsOnFromLoop(false);
    beep();
  }
}

void modeEllipse(Axis* main, Axis* aux) {
  if (main->movingManually || aux->movingManually || turnPasses <= 0 ||
      main->leftStop == LONG_MAX || main->rightStop == LONG_MIN ||
      aux->leftStop == LONG_MAX || aux->rightStop == LONG_MIN ||
      main->leftStop == main->rightStop ||
      aux->leftStop == aux->rightStop ||
      dupr == 0 || dupr != opDupr) {
    setIsOnFromLoop(false);
    return;
  }

  // Start from left or right depending on the pitch.
  long mainStartStop = opDuprSign > 0 ? main->rightStop : main->leftStop;
  long mainEndStop = opDuprSign > 0 ? main->leftStop : main->rightStop;
  long auxStartStop = aux->rightStop;
  long auxEndStop = aux->leftStop;

  main->speedMax = main->speedManualMove;
  aux->speedMax = aux->speedManualMove;

  if (opIndex == 0) {
    opIndex = 1;
    opSubIndex = 0;
    spindlePos = 0;
    spindlePosAvg = 0;
  } else if (opIndex <= turnPasses) {
    float pass0to1 = opIndex / float(turnPasses);
    long mainDelta = round(pass0to1 * (mainEndStop - mainStartStop));
    long auxDelta = round(pass0to1 * (auxEndStop - auxStartStop));
    long spindleDelta = spindleFromPos(main, mainDelta);

    // Move to starting position.
    if (opSubIndex == 0) {
      long auxPos = auxStartStop;
      stepToFinal(aux, auxPos);
      if (aux->pos == auxPos) {
        opSubIndex = 1;
      }
    } else if (opSubIndex == 1) {
      long mainPos = mainEndStop - mainDelta;
      stepToFinal(main, mainPos);
      if (main->pos == mainPos) {
        opSubIndex = 2;
        spindlePos = 0;
        spindlePosAvg = 0;
      }
    } else if (opSubIndex == 2) {
      float progress0to1 = 0;
      if ((spindleDelta > 0 && spindlePosAvg >= spindleDelta) || (spindleDelta < 0 && spindlePosAvg <= spindleDelta)) {
        progress0to1 = 1;
      } else {
        progress0to1 = spindlePosAvg / float(spindleDelta);
      }
      float mainCoeff = auxForward ? cos(HALF_PI * (3 + progress0to1)) : (1 + sin(HALF_PI * (progress0to1 - 1)));
      long mainPos = mainEndStop - mainDelta + round(mainDelta * mainCoeff);
      float auxCoeff = auxForward ? (1 + sin(HALF_PI * (3 + progress0to1))) : sin(HALF_PI * progress0to1);
      long auxPos = auxStartStop + round(auxDelta * auxCoeff);
      stepToContinuous(main, mainPos);
      stepToContinuous(aux, auxPos);
      if (progress0to1 == 1 && main->pos == mainPos && aux->pos == auxPos) {
        opIndex++;
        opSubIndex = 0;
      }
    }
  } else if (opIndex == turnPasses + 1) {
    stepToFinal(aux, auxStartStop);
    if (aux->pos == auxStartStop) {
      setIsOnFromLoop(false);
      beep();
    }
  }
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
  if (delta == 0) {
    return;
  }
  unsigned long microsNow = micros();
  if (showTacho || mode == MODE_GCODE) {
    if (spindleEncTimeIndex >= RPM_BULK) {
      spindleEncTimeDiffBulk = microsNow - spindleEncTimeAtIndex0;
      spindleEncTimeAtIndex0 = microsNow;
      spindleEncTimeIndex = 0;
    }
    spindleEncTimeIndex += abs(delta);
  } else {
    spindleEncTimeDiffBulk = 0;
  }

  spindlePos += delta;
  spindlePosGlobal += delta;
  if (spindlePosGlobal > ENCODER_STEPS_INT) {
    spindlePosGlobal -= ENCODER_STEPS_INT;
  } else if (spindlePosGlobal < 0) {
    spindlePosGlobal += ENCODER_STEPS_INT;
  }
  if (spindlePos > spindlePosAvg) {
    spindlePosAvg = spindlePos;
  } else if (spindlePos < spindlePosAvg - ENCODER_BACKLASH) {
    spindlePosAvg = spindlePos + ENCODER_BACKLASH;
  }
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

// Apply changes requested by the keyboard thread.
void applySettings() {
  if (nextDuprFlag) {
    applyDupr();
    nextDuprFlag = false;
  }
  if (nextStartsFlag) {
    applyStarts();
    nextStartsFlag = false;
  }
  if (z.nextLeftStopFlag) {
    applyLeftStop(&z);
    z.nextLeftStopFlag = false;
  }
  if (z.nextRightStopFlag) {
    applyRightStop(&z);
    z.nextRightStopFlag = false;
  }
  if (x.nextLeftStopFlag) {
    applyLeftStop(&x);
    x.nextLeftStopFlag = false;
  }
  if (x.nextRightStopFlag) {
    applyRightStop(&x);
    x.nextRightStopFlag = false;
  }
  if (a1.nextLeftStopFlag) {
    applyLeftStop(&a1);
    a1.nextLeftStopFlag = false;
  }
  if (a1.nextRightStopFlag) {
    applyRightStop(&a1);
    a1.nextRightStopFlag = false;
  }
  if (nextConeRatioFlag) {
    applyConeRatio();
    nextConeRatioFlag = false;
  }
  if (nextIsOnFlag) {
    setIsOnFromLoop(nextIsOn);
    nextIsOnFlag = false;
  }
  if (nextModeFlag) {
    setModeFromLoop(nextMode);
    nextModeFlag = false;
  }
}

void loop() {
  if (emergencyStop != ESTOP_NONE) {
    return;
  }
  if (xSemaphoreTake(motionMutex, 1) != pdTRUE) {
    return;
  }
  applySettings();
  processSpindlePosDelta();
  discountFullSpindleTurns();
  if (!isOn || dupr == 0 || spindlePosSync != 0) {
    // None of the modes work.
  } else if (mode == MODE_NORMAL) {
    modeGearbox();
  } else if (mode == MODE_TURN) {
    modeTurn(&z, &x);
  } else if (mode == MODE_FACE) {
    modeTurn(&x, &z);
  } else if (mode == MODE_CUT) {
    modeCut();
  } else if (mode == MODE_CONE) {
    modeCone();
  } else if (mode == MODE_THREAD) {
    modeTurn(&z, &x);
  } else if (mode == MODE_ELLIPSE) {
    modeEllipse(&z, &x);
  }
  moveAxis(&z);
  moveAxis(&x);
  if (ACTIVE_A1) moveAxis(&a1);
  xSemaphoreGive(motionMutex);
}

void setup() {
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  pinMode(Z_DIR, OUTPUT);
  pinMode(Z_STEP, OUTPUT);
  pinMode(Z_ENA, OUTPUT);
  DHIGH(Z_STEP);

  pinMode(X_DIR, OUTPUT);
  pinMode(X_STEP, OUTPUT);
  pinMode(X_ENA, OUTPUT);
  DHIGH(X_STEP);

  if (ACTIVE_A1) {
    pinMode(A12, OUTPUT);
    pinMode(A13, OUTPUT);
    pinMode(A11, OUTPUT);
    DHIGH(A13);
  }

  pinMode(BUZZ, OUTPUT);

  if (PULSE_1_USE) {
    pinMode(A11, OUTPUT);
    pinMode(A12, INPUT);
    pinMode(A13, INPUT);
    DLOW(A11);
  }

  if (PULSE_2_USE) {
    pinMode(A21, OUTPUT);
    pinMode(A22, INPUT);
    pinMode(A23, INPUT);
    DLOW(A21);
  }

  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  if (pref.getInt(PREF_VERSION) != PREFERENCES_VERSION) {
    pref.clear();
    pref.putInt(PREF_VERSION, PREFERENCES_VERSION);
  }

  initAxis(&z, NAME_Z, true, false, MOTOR_STEPS_Z, SCREW_Z_DU, SPEED_START_Z, SPEED_MANUAL_MOVE_Z, ACCELERATION_Z, INVERT_Z, NEEDS_REST_Z, MAX_TRAVEL_MM_Z, BACKLASH_DU_Z, Z_ENA, Z_DIR, Z_STEP);
  initAxis(&x, NAME_X, true, false, MOTOR_STEPS_X, SCREW_X_DU, SPEED_START_X, SPEED_MANUAL_MOVE_X, ACCELERATION_X, INVERT_X, NEEDS_REST_X, MAX_TRAVEL_MM_X, BACKLASH_DU_X, X_ENA, X_DIR, X_STEP);
  initAxis(&a1, NAME_A1, ACTIVE_A1, ROTARY_A1, MOTOR_STEPS_A1, SCREW_A1_DU, SPEED_START_A1, SPEED_MANUAL_MOVE_A1, ACCELERATION_A1, INVERT_A1, NEEDS_REST_A1, MAX_TRAVEL_MM_A1, BACKLASH_DU_A1, A11, A12, A13);

  isOn = false;
  savedDupr = dupr = pref.getLong(PREF_DUPR);
  motionMutex = xSemaphoreCreateMutex();
  savedStarts = starts = min(STARTS_MAX, max(1, pref.getInt(PREF_STARTS)));
  z.savedPos = z.pos = pref.getLong(PREF_POS_Z);
  z.savedPosGlobal = z.posGlobal = pref.getLong(PREF_POS_GLOBAL_Z);
  z.savedOriginPos = z.originPos = pref.getLong(PREF_ORIGIN_POS_Z);
  z.savedMotorPos = z.motorPos = pref.getLong(PREF_MOTOR_POS_Z);
  z.savedLeftStop = z.leftStop = pref.getLong(PREF_LEFT_STOP_Z, LONG_MAX);
  z.savedRightStop = z.rightStop = pref.getLong(PREF_RIGHT_STOP_Z, LONG_MIN);
  z.savedDisabled = z.disabled = pref.getBool(PREF_DISABLED_Z, false);
  x.savedPos = x.pos = pref.getLong(PREF_POS_X);
  x.savedPosGlobal = x.posGlobal = pref.getLong(PREF_POS_GLOBAL_X);
  x.savedOriginPos = x.originPos = pref.getLong(PREF_ORIGIN_POS_X);
  x.savedMotorPos = x.motorPos = pref.getLong(PREF_MOTOR_POS_X);
  x.savedLeftStop = x.leftStop = pref.getLong(PREF_LEFT_STOP_X, LONG_MAX);
  x.savedRightStop = x.rightStop = pref.getLong(PREF_RIGHT_STOP_X, LONG_MIN);
  x.savedDisabled = x.disabled = pref.getBool(PREF_DISABLED_X, false);
  a1.savedPos = a1.pos = pref.getLong(PREF_POS_A1);
  a1.savedPosGlobal = a1.posGlobal = pref.getLong(PREF_POS_GLOBAL_A1);
  a1.savedOriginPos = a1.originPos = pref.getLong(PREF_ORIGIN_POS_A1);
  a1.savedMotorPos = a1.motorPos = pref.getLong(PREF_MOTOR_POS_A1);
  a1.savedLeftStop = a1.leftStop = pref.getLong(PREF_LEFT_STOP_A1, LONG_MAX);
  a1.savedRightStop = a1.rightStop = pref.getLong(PREF_RIGHT_STOP_A1, LONG_MIN);
  a1.savedDisabled = a1.disabled = pref.getBool(PREF_DISABLED_A1, false);
  savedSpindlePos = spindlePos = pref.getLong(PREF_SPINDLE_POS);
  savedSpindlePosAvg = spindlePosAvg = pref.getLong(PREF_SPINDLE_POS_AVG);
  savedSpindlePosSync = spindlePosSync = pref.getInt(PREF_OUT_OF_SYNC);
  savedSpindlePosGlobal = spindlePosGlobal = pref.getLong(PREF_SPINDLE_POS_GLOBAL);
  savedShowAngle = showAngle = pref.getBool(PREF_SHOW_ANGLE);
  savedShowTacho = showTacho = pref.getBool(PREF_SHOW_TACHO);
  savedMoveStep = moveStep = pref.getLong(PREF_MOVE_STEP, MOVE_STEP_1);
  setModeFromLoop(savedMode = pref.getInt(PREF_MODE));
  savedMeasure = measure = pref.getInt(PREF_MEASURE);
  savedConeRatio = coneRatio = pref.getFloat(PREF_CONE_RATIO, coneRatio);
  savedTurnPasses = turnPasses = pref.getInt(PREF_TURN_PASSES, turnPasses);
  savedAuxForward = auxForward = pref.getBool(PREF_AUX_FORWARD, true);
  pref.end();

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

  lcd.begin(20, 4);
  lcd.createChar(customCharMmCode, customCharMm);
  lcd.createChar(customCharLimLeftCode, customCharLimLeft);
  lcd.createChar(customCharLimRightCode, customCharLimRight);
  lcd.createChar(customCharLimUpCode, customCharLimUp);
  lcd.createChar(customCharLimDownCode, customCharLimDown);
  lcd.createChar(customCharLimUpDownCode, customCharLimUpDown);
  lcd.createChar(customCharLimLeftRightCode, customCharLimLeftRight);

  Serial.begin(115200);

  if (!Wire.begin(SDA, SCL)) {
    Serial.println("I2C initialization failed");
  } else if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("TCA8418 key controller not found");
  } else {
    keypad.matrix(7, 7);
    keypad.flush();
  }

  // Non-time-sensitive tasks on core 0.
  xTaskCreatePinnedToCore(taskDisplay, "taskDisplay", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);

  delay(100);
  if (keypad.available()) {
    setEmergencyStop(ESTOP_KEY);
    return;
  } else {
    xTaskCreatePinnedToCore(taskKeypad, "taskKeypad", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  }

  xTaskCreatePinnedToCore(taskMoveZ, "taskMoveZ", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskMoveX, "taskMoveX", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  if (a1.active) xTaskCreatePinnedToCore(taskMoveA1, "taskMoveA1", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskAttachInterrupts, "taskAttachInterrupts", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskGcode, "taskGcode", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
}

int main () {
  setup();
  while (true)
    loop();
};