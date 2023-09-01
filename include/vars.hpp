#pragma once

#include <Arduino.h>
#include "config.hpp"

/* Changing anything below shouldn't be needed for basic use. */

// Configuration for axis connected to A1. This is uncommon. Dividing head (C) motor parameters.
// Throughout the configuration below we assume 1mm = 1degree of rotation, so 1du = 0.0001degree.
const bool ACTIVE_A1 = false; // Whether the axis is connected
const bool ROTARY_A1 = true; // Whether the axis is rotary or linear
const long MOTOR_STEPS_A1 = 300; // Number of motor steps for 1 rotation of the the worm gear screw (full step with 20:30 reduction)
const long SCREW_A1_DU = 20000; // Degrees multiplied by 10000 that the spindle travels per 1 turn of the worm gear. 2 degrees.
const long SPEED_START_A1 = 1600; // Initial speed of a motor, steps / second.
const long ACCELERATION_A1 = 16000; // Acceleration of a motor, steps / second ^ 2.
const long SPEED_MANUAL_MOVE_A1 = 3200; // Maximum speed of a motor during manual move, steps / second.
const bool INVERT_A1 = false; // change (true/false) if the carriage moves e.g. "left" when you press "right".
const bool INVERT_A1_ENA = true;
const bool NEEDS_REST_A1 = false; // Set to false for closed-loop drivers. Open-loop: true if you need holding torque, false otherwise.
const long MAX_TRAVEL_MM_A1 = 360; // Probably doesn't make sense to ask the dividin head to travel multiple turns.
const long BACKLASH_DU_A1 = 0; // Assuming no backlash on the worm gear
const char NAME_A1 = 'C'; // Text shown on screen before axis position value, GCode axis name
// Manual handwheels on A1 and A2. Ignore if you don't have them installed.
const bool PULSE_1_USE = false; // Whether there's a pulse generator connected on A11-A13 to be used for movement.
const char PULSE_1_AXIS = NAME_Z; // Set to NAME_X to make A11-A13 pulse generator control X instead.
const bool PULSE_2_USE = false; // Whether there's a pulse generator connected on A21-A23 to be used for movement.
const char PULSE_2_AXIS = NAME_X; // Set to NAME_Z to make A21-A23 pulse generator control Z instead.
const float PULSE_PER_REVOLUTION = 100; // PPR of handwheels used on A1 and/or A2.
const long PULSE_HALF_BACKLASH = 2; // Prevents spurious reverses when moving using a handwheel. Raise to 3 or 4 if they still happen.
const long DUPR_MAX = 254000; // No more than 1 inch pitch
const int STARTS_MAX = 124; // No more than 124-start thread
const long SAFE_DISTANCE_DU = 5000; // Step back 0.5mm from the material when moving between cuts in automated modes
const long SAVE_DELAY_US = 5000000; // Wait 5s after last save and last change of saveable data before saving again
const long DIRECTION_SETUP_DELAY_US = 5; // Stepper driver needs some time to adjust to direction change
const long STEPPED_ENABLE_DELAY_MS = 100; // Delay after stepper is enabled and before issuing steps
// GCode-related constants.
const float LINEAR_INTERPOLATION_PRECISION = 0.1; // 0 < x <= 1, smaller values make for quicker G0 and G1 moves
const long GCODE_WAIT_EPSILON_STEPS = 10;
const long RPM_BULK = ENCODER_STEPS_INT; // Measure RPM averaged over this number of encoder pulses
const long GCODE_FEED_DEFAULT_DU_SEC = 20000; // Default feed in du/sec in GCode mode
const float GCODE_FEED_MIN_DU_SEC = 167; // Minimum feed in du/sec in GCode mode - F1

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
#define MEASURE_METRIC 0
#define MEASURE_INCH 1
#define MEASURE_TPI 2

extern int emergencyStop;
extern bool beepFlag; // allows time-critical code to ask for a beep on another core
extern long savedDupr; // dupr saved in Preferences
extern long nextDupr; // dupr value that should be applied asap
extern bool nextDuprFlag; // whether nextDupr requires attention
extern SemaphoreHandle_t motionMutex; // controls blocks of code where variables affecting the motion loop() are changed
extern int savedStarts; // starts saved in Preferences
extern int nextStarts; // number of starts that should be used asap
extern bool nextStartsFlag; // whether nextStarts requires attention
extern unsigned long saveTime; // micros() of the previous Prefs write
extern int savedMeasure; // measure value saved in Preferences
extern float savedConeRatio; // value of coneRatio saved in Preferences
extern float nextConeRatio; // coneRatio that should be applied asap
extern bool nextConeRatioFlag; // whether nextConeRatio requires attention
extern int savedTurnPasses; // value of turnPasses saved in Preferences
extern bool savedAuxForward; // value of auxForward saved in Preferences
extern long opSubIndex; // Sub-index of an automation operation
extern int opDuprSign; // 1 if dupr was positive when operation started, -1 if negative
extern long opDupr; // dupr that the multi-pass operation started with
extern long gcodeFeedDuPerSec;
extern bool gcodeInitialized;
extern bool gcodeAbsolutePositioning;
extern bool gcodeInBrace;
extern bool gcodeInSemicolon;
extern bool timerAttached;

extern String gcodeCommand;
extern bool auxForward; // True for external, false for external thread
extern int starts; // number of starts in a multi-start thread
extern long dupr; // pitch, tenth of a micron per rotation
extern float coneRatio; // In cone mode, how much X moves for 1 step of Z
extern int turnPasses; // In turn mode, how many turn passes to make
extern long opIndex; // Index of an automation operation

extern long setupIndex; // Index microsof automation setup step

extern long moveStep; // thousandth of a mm
extern int measure; // Whether to show distances in inches
extern bool showAngle; // Whether to show 0-359 spindle angle on screen
extern bool showTacho; // Whether to show spindle RPM on screen
