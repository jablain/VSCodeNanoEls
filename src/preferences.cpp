#include <Preferences.h>
#include "preferences.hpp"
#include "vars.hpp"
#include "axis.hpp"
#include "spindle.hpp"
#include "display.hpp"
#include "modes.hpp"
#include "macros.hpp"

Settings::Settings() : Preferences() {
  dupr = 0; // pitch, tenth of a micron per rotation
  starts = 1; // number of starts in a multi-start thread
  // init axis z x a1
  spindlePos = 0; // Spindle position
  spindlePosAvg = 0; // Spindle position accounting for encoder backlash
  spindlePosSync = 0; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
  spindlePosGlobal = 0; // global spindle position that is unaffected by e.g. zeroing
  showAngle = false; // Whether to show 0-359 spindle angle on screen
  showTacho = false; // Whether to show spindle RPM on screen
  moveStep = 0; // thousandth of a mm
  measure = MEASURE_METRIC; // Whether to show distances in inches
  mode = -1; // mode of operation (ELS, multi-start ELS, asynchronous)
  coneRatio = 1; // In cone mode, how much X moves for 1 step of Z
  turnPasses = 3; // In turn mode, how many turn passes to make
  auxForward = true; // True for external, false for external thread
};

void Settings::Load() {
  // Read in
  begin(PREF_NAMESPACE);
  if (getInt(PREF_VERSION) != PREFERENCES_VERSION) {
    clear();
    putInt(PREF_VERSION, PREFERENCES_VERSION);
  }
  savedDupr = dupr = getLong(PREF_DUPR);
  savedStarts = starts = min(STARTS_MAX, max(1, getInt(PREF_STARTS)));
  z.savedPos = z.pos = getLong(PREF_POS_Z);
  z.savedPosGlobal = z.posGlobal = getLong(PREF_POS_GLOBAL_Z);
  z.savedOriginPos = z.originPos = getLong(PREF_ORIGIN_POS_Z);
  z.savedMotorPos = z.motorPos = getLong(PREF_MOTOR_POS_Z);
  z.savedLeftStop = z.leftStop = getLong(PREF_LEFT_STOP_Z, LONG_MAX);
  z.savedRightStop = z.rightStop = getLong(PREF_RIGHT_STOP_Z, LONG_MIN);
  z.savedDisabled = z.disabled = getBool(PREF_DISABLED_Z, false);
  x.savedPos = x.pos = getLong(PREF_POS_X);
  x.savedPosGlobal = x.posGlobal = getLong(PREF_POS_GLOBAL_X);
  x.savedOriginPos = x.originPos = getLong(PREF_ORIGIN_POS_X);
  x.savedMotorPos = x.motorPos = getLong(PREF_MOTOR_POS_X);
  x.savedLeftStop = x.leftStop = getLong(PREF_LEFT_STOP_X, LONG_MAX);
  x.savedRightStop = x.rightStop = getLong(PREF_RIGHT_STOP_X, LONG_MIN);
  x.savedDisabled = x.disabled = getBool(PREF_DISABLED_X, false);
  a1.savedPos = a1.pos = getLong(PREF_POS_A1);
  a1.savedPosGlobal = a1.posGlobal = getLong(PREF_POS_GLOBAL_A1);
  a1.savedOriginPos = a1.originPos = getLong(PREF_ORIGIN_POS_A1);
  a1.savedMotorPos = a1.motorPos = getLong(PREF_MOTOR_POS_A1);
  a1.savedLeftStop = a1.leftStop = getLong(PREF_LEFT_STOP_A1, LONG_MAX);
  a1.savedRightStop = a1.rightStop = getLong(PREF_RIGHT_STOP_A1, LONG_MIN);
  a1.savedDisabled = a1.disabled = getBool(PREF_DISABLED_A1, false);
  savedSpindlePos = spindlePos = getLong(PREF_SPINDLE_POS);
  savedSpindlePosAvg = spindlePosAvg = getLong(PREF_SPINDLE_POS_AVG);
  savedSpindlePosSync = spindlePosSync = getInt(PREF_OUT_OF_SYNC);
  savedSpindlePosGlobal = spindlePosGlobal = getLong(PREF_SPINDLE_POS_GLOBAL);
  savedShowAngle = showAngle = getBool(PREF_SHOW_ANGLE);
  savedShowTacho = showTacho = getBool(PREF_SHOW_TACHO);
  savedMoveStep = moveStep = getLong(PREF_MOVE_STEP, MOVE_STEP_1);
  setModeFromLoop(savedMode = getInt(PREF_MODE));
  savedMeasure = measure = getInt(PREF_MEASURE);
  savedConeRatio = coneRatio = getFloat(PREF_CONE_RATIO, coneRatio);
  savedTurnPasses = turnPasses = getInt(PREF_TURN_PASSES, turnPasses);
  savedAuxForward = auxForward = getBool(PREF_AUX_FORWARD, true);
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
  end();
};

void Settings::Save() {
  begin(PREF_NAMESPACE);
  if (dupr != savedDupr) putLong(PREF_DUPR, savedDupr = dupr);
  if (starts != savedStarts) putInt(PREF_STARTS, savedStarts = starts);
  if (z.pos != z.savedPos) putLong(PREF_POS_Z, z.savedPos = z.pos);
  if (z.posGlobal != z.savedPosGlobal) putLong(PREF_POS_GLOBAL_Z, z.savedPosGlobal = z.posGlobal);
  if (z.originPos != z.savedOriginPos) putLong(PREF_ORIGIN_POS_Z, z.savedOriginPos = z.originPos);
  if (z.motorPos != z.savedMotorPos) putLong(PREF_MOTOR_POS_Z, z.savedMotorPos = z.motorPos);
  if (z.leftStop != z.savedLeftStop) putLong(PREF_LEFT_STOP_Z, z.savedLeftStop = z.leftStop);
  if (z.rightStop != z.savedRightStop) putLong(PREF_RIGHT_STOP_Z, z.savedRightStop = z.rightStop);
  if (z.disabled != z.savedDisabled) putBool(PREF_DISABLED_Z, z.savedDisabled = z.disabled);
  if (spindlePos != savedSpindlePos) putLong(PREF_SPINDLE_POS, savedSpindlePos = spindlePos);
  if (spindlePosAvg != savedSpindlePosAvg) putLong(PREF_SPINDLE_POS_AVG, savedSpindlePosAvg = spindlePosAvg);
  if (spindlePosSync != savedSpindlePosSync) putInt(PREF_OUT_OF_SYNC, savedSpindlePosSync = spindlePosSync);
  if (spindlePosGlobal != savedSpindlePosGlobal) putLong(PREF_SPINDLE_POS_GLOBAL, savedSpindlePosGlobal = spindlePosGlobal);
  if (showAngle != savedShowAngle) putBool(PREF_SHOW_ANGLE, savedShowAngle = showAngle);
  if (showTacho != savedShowTacho) putBool(PREF_SHOW_TACHO, savedShowTacho = showTacho);
  if (moveStep != savedMoveStep) putLong(PREF_MOVE_STEP, savedMoveStep = moveStep);
  if (mode != savedMode) putInt(PREF_MODE, savedMode = mode);
  if (measure != savedMeasure) putInt(PREF_MEASURE, savedMeasure = measure);
  if (x.pos != x.savedPos) putLong(PREF_POS_X, x.savedPos = x.pos);
  if (x.posGlobal != x.savedPosGlobal) putLong(PREF_POS_GLOBAL_X, x.savedPosGlobal = x.posGlobal);
  if (x.originPos != x.savedOriginPos) putLong(PREF_ORIGIN_POS_X, x.savedOriginPos = x.originPos);
  if (x.motorPos != x.savedMotorPos) putLong(PREF_MOTOR_POS_X, x.savedMotorPos = x.motorPos);
  if (x.leftStop != x.savedLeftStop) putLong(PREF_LEFT_STOP_X, x.savedLeftStop = x.leftStop);
  if (x.rightStop != x.savedRightStop) putLong(PREF_RIGHT_STOP_X, x.savedRightStop = x.rightStop);
  if (x.disabled != x.savedDisabled) putBool(PREF_DISABLED_X, x.savedDisabled = x.disabled);
  if (a1.pos != a1.savedPos) putLong(PREF_POS_A1, a1.savedPos = a1.pos);
  if (a1.posGlobal != a1.savedPosGlobal) putLong(PREF_POS_GLOBAL_A1, a1.savedPosGlobal = a1.posGlobal);
  if (a1.originPos != a1.savedOriginPos) putLong(PREF_ORIGIN_POS_A1, a1.savedOriginPos = a1.originPos);
  if (a1.motorPos != a1.savedMotorPos) putLong(PREF_MOTOR_POS_A1, a1.savedMotorPos = a1.motorPos);
  if (a1.leftStop != a1.savedLeftStop) putLong(PREF_LEFT_STOP_A1, a1.savedLeftStop = a1.leftStop);
  if (a1.rightStop != a1.savedRightStop) putLong(PREF_RIGHT_STOP_A1, a1.savedRightStop = a1.rightStop);
  if (a1.disabled != a1.savedDisabled) putBool(PREF_DISABLED_A1, a1.savedDisabled = a1.disabled);
  if (coneRatio != savedConeRatio) putFloat(PREF_CONE_RATIO, savedConeRatio = coneRatio);
  if (turnPasses != savedTurnPasses) putInt(PREF_TURN_PASSES, savedTurnPasses = turnPasses);
  if (auxForward != savedAuxForward) putBool(PREF_AUX_FORWARD, savedAuxForward = auxForward);
  end();
};

Settings::~Settings() {
};

void setupPreferences() {
  motionMutex = xSemaphoreCreateMutex(); // controls blocks of code where variables affecting the motion loop() are changed
  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  if (pref.getInt(PREF_VERSION) != PREFERENCES_VERSION) {
    pref.clear();
    pref.putInt(PREF_VERSION, PREFERENCES_VERSION);
  }
  savedDupr = dupr = pref.getLong(PREF_DUPR);
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
  pref.end();
};
