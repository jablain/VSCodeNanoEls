#include "vars.hpp"
#include "modes.hpp"
#include "axis.hpp"
#include "display.hpp"

volatile int mode = -1; // mode of operation (ELS, multi-start ELS, asynchronous)
int nextMode = 0; // mode value that should be applied asap
bool nextModeFlag = false; // whether nextMode needs attention
int savedMode = -1; // mode saved in Preferences

bool isOn = false;
bool nextIsOn; // isOn value that should be applied asap
bool nextIsOnFlag; // whether nextIsOn requires attention

bool isPassMode() {
  return mode == MODE_TURN || mode == MODE_FACE || mode == MODE_CUT || mode == MODE_THREAD || mode == MODE_ELLIPSE;
}

long getPassModeZStart() {
  if (mode == MODE_TURN || mode == MODE_THREAD) return dupr > 0 ? z.rightStop : z.leftStop;
  if (mode == MODE_FACE) return auxForward ? z.rightStop : z.leftStop;
  if (mode == MODE_ELLIPSE) return dupr > 0 ? z.leftStop : z.rightStop;
  return z.pos;
}

long getPassModeXStart() {
  if (mode == MODE_TURN || mode == MODE_THREAD) return auxForward ? x.rightStop : x.leftStop;
  if (mode == MODE_FACE || mode == MODE_CUT) return dupr > 0 ? x.rightStop : x.leftStop;
  if (mode == MODE_ELLIPSE) return x.rightStop;
  return x.pos;
}

int getLastSetupIndex() {
  if (mode == MODE_CONE) return 2;
  if (mode == MODE_TURN || mode == MODE_FACE || mode == MODE_CUT || mode == MODE_THREAD || mode == MODE_ELLIPSE) return 3;
  return 0;
}

void setModeFromTask(int value) {
  nextMode = value;
  nextModeFlag = true;
}

bool needZStops() {
  return mode == MODE_TURN || mode == MODE_FACE || mode == MODE_THREAD || mode == MODE_ELLIPSE;
}

void setDupr(long value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  nextDupr = value;
  nextDuprFlag = true;
}

void setStarts(int value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  nextStarts = value;
  nextStartsFlag = true;
}

void setConeRatio(float value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  nextConeRatio = value;
  nextConeRatioFlag = true;
}


