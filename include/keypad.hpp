#pragma once

#define B_LEFT 57
#define B_RIGHT 37
#define B_UP 47
#define B_DOWN 67
#define B_MINUS 5
#define B_PLUS 64
#define B_ON 17
#define B_OFF 27
#define B_STOPL 7
#define B_STOPR 15
#define B_STOPU 6
#define B_STOPD 16
#define B_DISPL 14
#define B_STEP 24
#define B_SETTINGS 34
#define B_MEASURE 54
#define B_REVERSE 44
#define B_0 51
#define B_1 41
#define B_2 61
#define B_3 31
#define B_4 2
#define B_5 21
#define B_6 12
#define B_7 11
#define B_8 22
#define B_9 1
#define B_BACKSPACE 32
#define B_MODE_GEARS 42
#define B_MODE_TURN 52
#define B_MODE_FACE 62
#define B_MODE_CONE 3
#define B_MODE_CUT 13
#define B_MODE_THREAD 23
#define B_MODE_OTHER 33
#define B_X 53
#define B_Z 43
#define B_A 4
#define B_B 63

Adafruit_TCA8418 keypad;
unsigned long keypadTimeUs = 0;

// Most buttons we only have "down" handling, holding them has no effect.
// Buttons with special "holding" logic have flags below.
bool buttonLeftPressed = false;
bool buttonRightPressed = false;
bool buttonUpPressed = false;
bool buttonDownPressed = false;
bool buttonOffPressed = false;
bool buttonGearsPressed = false;
bool buttonTurnPressed = false;

bool inNumpad = false;
int numpadDigits[20];
int numpadIndex = 0;

const int customCharMmCode = 0;
byte customCharMm[] = {
  B11010,
  B10101,
  B10101,
  B00000,
  B11010,
  B10101,
  B10101,
  B00000
};
const int customCharLimUpCode = 1;
byte customCharLimUp[] = {
  B11111,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00000,
  B00000
};
const int customCharLimDownCode = 2;
byte customCharLimDown[] = {
  B00000,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B11111,
  B00000
};
const int customCharLimLeftCode = 3;
byte customCharLimLeft[] = {
  B10000,
  B10010,
  B10100,
  B11111,
  B10100,
  B10010,
  B10000,
  B00000
};
const int customCharLimRightCode = 4;
byte customCharLimRight[] = {
  B00001,
  B01001,
  B00101,
  B11111,
  B00101,
  B01001,
  B00001,
  B00000
};
const int customCharLimUpDownCode = 5;
byte customCharLimUpDown[] = {
  B11111,
  B00100,
  B01110,
  B00000,
  B01110,
  B00100,
  B11111,
  B00000
};
const int customCharLimLeftRightCode = 6;
byte customCharLimLeftRight[] = {
  B00000,
  B10001,
  B10001,
  B11111,
  B10001,
  B10001,
  B00000,
  B00000
};

