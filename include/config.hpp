#pragma once

/* Change values in this section to suit your hardware. */

// Define your hardware parameters here.
const int ENCODER_STEPS_INT = 1024; // 600 step spindle optical rotary encoder. Fractional values not supported.
const int ENCODER_BACKLASH = 0; // was 3 Number of impulses encoder can issue without movement of the spindle

// Spindle rotary encoder pins. Swap values if the rotation direction is wrong.
#define ENC_A 7
#define ENC_B 15

// Main lead screw (Z) parameters.
const long SCREW_Z_DU = 5386; // was 5386 8 TPI lead 2:1 gearbox 3:1 pulleys in deci-microns (10^-7 of a meter)
const long MOTOR_STEPS_Z = 400; //1600
const long SPEED_START_Z = 5 * MOTOR_STEPS_Z; // Initial speed of a motor, steps / second.
const long ACCELERATION_Z = 100 * MOTOR_STEPS_Z; // Acceleration of a motor, steps / second ^ 2.
const long SPEED_MANUAL_MOVE_Z = 20 * MOTOR_STEPS_Z; // was 6 Maximum speed of a motor during manual move, steps / second.
const bool INVERT_Z = true; // change (true/false) if the carriage moves e.g. "left" when you press "right".
const bool INVERT_Z_ENA = true;
const bool NEEDS_REST_Z = false; // Set to false for closed-loop drivers, true for open-loop.
const long MAX_TRAVEL_MM_Z = 300; // Lathe bed doesn't allow to travel more than this in one go, 30cm / ~1 foot
const long BACKLASH_DU_Z = 8382; // was 8382 33 mil backlash in deci-microns (10^-7 of a meter)
const char NAME_Z = 'Z'; // Text shown on screen before axis position value, GCode axis name

// Cross-slide lead screw (X) parameters.
const long SCREW_X_DU = 8445; // 8466; // 10 tpi lead 3:1 pulleys in deci-microns (10^-7) of a meter
const long MOTOR_STEPS_X = 400; // 1600 pulses/revolution
const long SPEED_START_X = 3 * MOTOR_STEPS_X; // Initial speed of a motor, steps / second.
const long ACCELERATION_X = 100 * MOTOR_STEPS_X; // was 10 Acceleration of a motor, steps / second ^ 2.
const long SPEED_MANUAL_MOVE_X = 10 * MOTOR_STEPS_X; // was 3 Maximum speed of a motor during manual move, steps / second.
const bool INVERT_X = true; // change (true/false) if the carriage moves e.g. "left" when you press "right".
const bool INVERT_X_ENA = true;
const bool NEEDS_REST_X = false; // Set to false for all kinds of drivers or X will be unlocked when not moving.
const long MAX_TRAVEL_MM_X = 100; // Cross slide doesn't allow to travel more than this in one go, 10cm
const long BACKLASH_DU_X = 4064; // 16 mil backlash in deci-microns (10^-7 of a meter)
const char NAME_X = 'X'; // Text shown on screen before axis position value, GCode axis name

// Manual stepping with left/right/up/down buttons. Only used when step isn't default continuous (1mm or 0.1").
const long STEP_TIME_MS = 500; // Time in milliseconds it should take to make 1 manual step.
const long DELAY_BETWEEN_STEPS_MS = 80; // Time in milliseconds to wait between steps.

