#pragma once

#include <Preferences.h>
#include "axis.hpp"

#define PREF_VERSION "v"
#define PREF_DUPR "d"
#define PREF_POS_Z "zp"
#define PREF_LEFT_STOP_Z "zls"
#define PREF_RIGHT_STOP_Z "zrs"
#define PREF_ORIGIN_POS_Z "zpo"
#define PREF_POS_GLOBAL_Z "zpg"
#define PREF_MOTOR_POS_Z "zpm"
#define PREF_DISABLED_Z "zd"
#define PREF_POS_X "xp"
#define PREF_LEFT_STOP_X "xls"
#define PREF_RIGHT_STOP_X "xrs"
#define PREF_ORIGIN_POS_X "xpo"
#define PREF_POS_GLOBAL_X "xpg"
#define PREF_MOTOR_POS_X "xpm"
#define PREF_DISABLED_X "xd"
#define PREF_POS_A1 "a1p"
#define PREF_LEFT_STOP_A1 "a1ls"
#define PREF_RIGHT_STOP_A1 "a1rs"
#define PREF_ORIGIN_POS_A1 "a1po"
#define PREF_POS_GLOBAL_A1 "a1pg"
#define PREF_MOTOR_POS_A1 "a1pm"
#define PREF_DISABLED_A1 "a1d"
#define PREF_SPINDLE_POS "sp"
#define PREF_SPINDLE_POS_AVG "spa"
#define PREF_OUT_OF_SYNC "oos"
#define PREF_SPINDLE_POS_GLOBAL "spg"
#define PREF_SHOW_ANGLE "ang"
#define PREF_SHOW_TACHO "rpm"
#define PREF_STARTS "sta"
#define PREF_MODE "mod"
#define PREF_MEASURE "mea"
#define PREF_CONE_RATIO "cr"
#define PREF_TURN_PASSES "tp"
#define PREF_MOVE_STEP "ms"
#define PREF_AUX_FORWARD "af"

// Version of the pref storage format, should be changed when non-backward-compatible
// changes are made to the storage logic, resulting in Preferences wipe on first start.
#define PREFERENCES_VERSION 1
#define PREF_NAMESPACE "h4"

class Settings : public Preferences {
public:
  Settings();
 ~Settings();
  void Load();
  void Save();
private:
  long dupr0; // pitch, tenth of a micron per rotation
  int starts; // number of starts in a multi-start thread
  Axis z;
  Axis x;
  Axis a1;
  long spindlePos; // Spindle position
  long spindlePosAvg; // Spindle position accounting for encoder backlash
  int spindlePosSync; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
  long spindlePosGlobal; // global spindle position that is unaffected by e.g. zeroing
  bool showAngle; // Whether to show 0-359 spindle angle on screen
  bool showTacho; // Whether to show spindle RPM on screen
  long moveStep; // thousandth of a mm
  int measure; // Whether to show distances in inches
  volatile int mode; // mode of operation (ELS, multi-start ELS, asynchronous)
  float coneRatio; // In cone mode, how much X moves for 1 step of Z
  int turnPasses; // In turn mode, how many turn passes to make
  bool auxForward; // True for external, false for external thread
/*
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
*/
};

void setupPreferences();
