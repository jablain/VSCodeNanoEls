#include "vars.hpp"
#include "config.hpp"
#include "display.hpp"

/* Changing anything below shouldn't be needed for basic use. */

// Configuration for axis connected to A1. This is uncommon. Dividing head (C) motor parameters.
// Throughout the configuration below we assume 1mm = 1degree of rotation, so 1du = 0.0001degree.
#define MOVE_STEP_1 10000 // 1mm
#define MOVE_STEP_2 1000 // 0.1mm
#define MOVE_STEP_3 100 // 0.01mm
#define MOVE_STEP_IMP_1 25400 // 1/10"
#define MOVE_STEP_IMP_2 2540 // 1/100"
#define MOVE_STEP_IMP_3 254 // 1/1000" also known as 1 thou
#define ESTOP_NONE 0
#define ESTOP_KEY 1
#define ESTOP_POS 2
#define ESTOP_MARK_ORIGIN 3
#define ESTOP_ON_OFF 4
int emergencyStop = 0;
bool beepFlag = false; // allows time-critical code to ask for a beep on another core
long nextDupr = dupr; // dupr value that should be applied asap
bool nextDuprFlag = false; // whether nextDupr requires attention
SemaphoreHandle_t motionMutex; // controls blocks of code where variables affecting the motion loop() are changed
int nextStarts = starts; // number of starts that should be used asap
bool nextStartsFlag = false; // whether nextStarts requires attention
float nextConeRatio = 0; // coneRatio that should be applied asap
bool nextConeRatioFlag = false; // whether nextConeRatio requires attention
long opSubIndex = 0; // Sub-index of an automation operation
int opDuprSign = 1; // 1 if dupr was positive when operation started, -1 if negative
long opDupr = 0; // dupr that the multi-pass operation started with
long gcodeFeedDuPerSec = GCODE_FEED_DEFAULT_DU_SEC;
bool gcodeInitialized = false;
bool gcodeAbsolutePositioning = true;
bool gcodeInBrace = false;
bool gcodeInSemicolon = false;
bool timerAttached = false;

String gcodeCommand = "";
bool auxForward = true; // True for external, false for external thread
int starts = 1; // number of starts in a multi-start thread
long dupr = 0; // pitch, tenth of a micron per rotation
float coneRatio = 1; // In cone mode, how much X moves for 1 step of Z
int turnPasses = 3; // In turn mode, how many turn passes to make
long opIndex = 0; // Index of an automation operation
long moveStep = 0; // thousandth of a mm
int measure = MEASURE_METRIC; // Whether to show distances in inches
bool showAngle = false; // Whether to show 0-359 spindle angle on screen
bool showTacho = false; // Whether to show spindle RPM on screen
int shownRpm = 0;
unsigned long shownRpmTime = 0; // micros() when shownRpm was set
unsigned long keypadTimeUs = 0;

void setMeasure(int value) {
  if (measure == value) 
    return;
  measure = value;
  moveStep = measure == MEASURE_METRIC ? MOVE_STEP_1 : MOVE_STEP_IMP_1;
}

