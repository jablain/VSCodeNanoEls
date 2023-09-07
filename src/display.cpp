#include <LiquidCrystal.h>
#include "pcb.hpp"
#include "vars.hpp"
#include "preferences.hpp"
#include "modes.hpp"
#include "spindle.hpp"
#include "keypad.hpp"
#include "tasks.hpp"

#define LCD_HASH_INITIAL -3845709 // Random number that's unlikely to naturally occur as an actual hash
// To be incremented whenever a measurable improvement is made.
#define SOFTWARE_VERSION 7
// To be changed whenever a different PCB / encoder / stepper / ... design is used.
#define HARDWARE_VERSION 4

const int customCharMmCode           = 0;
const int customCharLimUpCode        = 1;
const int customCharLimDownCode      = 2;
const int customCharLimLeftCode      = 3;
const int customCharLimRightCode     = 4;
const int customCharLimUpDownCode    = 5;
const int customCharLimLeftRightCode = 6;

// For MEASURE_TPI, round TPI to the nearest integer if it's within this range of it.
// E.g. 80.02tpi would be shown as 80tpi but 80.04tpi would be shown as-is.
const float TPI_ROUND_EPSILON = 0.03;

LiquidCrystal lcd (21, 48, 47, 38, 39, 40, 41, 42, 2, 1);

long setupIndex    = 0; // Index microsof automation setup step
long lcdHashLine0;
long lcdHashLine1;
long lcdHashLine2;
long lcdHashLine3;

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

// Returns number of letters printed.
int printDeciMicrons(long deciMicrons, int precisionPointsMax) {
  if (deciMicrons == 0) 
    return lcd.print("0");
  bool imperial = (measure != MEASURE_METRIC);
  long v = imperial ? round(deciMicrons / 25.4) : deciMicrons;
  int points = 0;
  if (v == 0 && precisionPointsMax >= 5) 
    points = 5;
  else if ((v % 10) != 0 && precisionPointsMax >= 4) 
    points = 4;
  else if ((v % 100) != 0 && precisionPointsMax >= 3) 
    points = 3;
  else if ((v % 1000) != 0 && precisionPointsMax >= 2) 
    points = 2;
  else if ((v % 10000) != 0 && precisionPointsMax >= 1) 
    points = 1;
  int count = lcd.print(deciMicrons / (imperial ? 254000.0 : 10000.0), points);
  count += imperial ? lcd.print("\"") : lcd.write(customCharMmCode);
  return count;
}

int printDegrees(long degrees10000) {
  int points = 0;
  if ((degrees10000 % 100) != 0) 
    points = 3;
  else if ((degrees10000 % 1000) != 0) 
    points = 2;
  else if ((degrees10000 % 10000) != 0) 
    points = 1;
  int count = lcd.print(degrees10000 / 10000.0, points);
  count += lcd.print(char(223)); // degree symbol
  return count;
}

int printDupr(long value) {
  int count = 0;
  if (measure != MEASURE_TPI)
    count += printDeciMicrons(value, 5);
  else {
    float tpi = 254000.0 / value;
    if (abs(tpi - round(tpi)) < TPI_ROUND_EPSILON) 
      count += lcd.print(int(round(tpi)));
    else {
      int tpi100 = round(tpi * 100);
      int points = 0;
      if ((tpi100 % 10) != 0) 
        points = 2;
      else if ((tpi100 % 100) != 0) 
        points = 1;
      count += lcd.print(tpi, points);
    }
    count += lcd.print("tpi");
  }
  return count;
}

void printLcdSpaces(int charIndex) {
  // Our screen has width 20.
  for (; charIndex < 20; charIndex++) 
    lcd.print(" ");
}

int printAxisStopDiff(Axis* a, bool addTrailingSpace) {
  int count = 0;
  if (a->rotational) 
    count = printDegrees(getAxisStopDiffDu(a));
  else
    count = printDeciMicrons(getAxisStopDiffDu(a), 3);
  if (addTrailingSpace) 
    count += lcd.print(' ');
  return count;
}

int printAxisPos(Axis* a) {
  if (a->rotational)
    return printDegrees(getAxisPosDu(a));
  return printDeciMicrons(getAxisPosDu(a), 3);
}

int printAxisPosWithName(Axis* a, bool addTrailingSpace) {
  if (!a->active || a->disabled)
    return 0;
  int count = lcd.print(a->name);
  count += printAxisPos(a);
  if (addTrailingSpace)
    count += lcd.print(' ');
  return count;
}

int printNoTrailing0(float value) {
  long v = round(value * 100000);
  int points = 0;
  if ((v % 10) != 0) 
    points = 5;
  else if ((v % 100) != 0) 
    points = 4;
  else if ((v % 1000) != 0) 
    points = 3;
  else if ((v % 10000) != 0) 
    points = 2;
  else if ((v % 100000) != 0) 
    points = 1;
  return lcd.print(value, points);
}

int printMode() {
  if (mode == MODE_ASYNC) 
    return lcd.print("ASY ");
  else if (mode == MODE_CONE) 
    return lcd.print("CONE ");
  else if (mode == MODE_TURN) 
    return lcd.print("TURN ");
  else if (mode == MODE_FACE) 
    return lcd.print("FACE ");
  else if (mode == MODE_CUT) 
    return lcd.print("CUT ");
  else if (mode == MODE_THREAD) 
    return lcd.print("THRD ");
  else if (mode == MODE_ELLIPSE) 
    return lcd.print("ELLI ");
  else if (mode == MODE_GCODE) 
    return lcd.print("GCODE ");
  else if (mode == MODE_A1) 
    return lcd.print("A1 ");
  return 0;
}

bool splashScreen  = true;

void showSplashScreen() {
  splashScreen = false;
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("NanoEls");
  lcd.setCursor(6, 2);
  lcd.print("H" + String(HARDWARE_VERSION) + " V" + String(SOFTWARE_VERSION));
  lcdHashLine0 = LCD_HASH_INITIAL;
  lcdHashLine1 = LCD_HASH_INITIAL;
  lcdHashLine2 = LCD_HASH_INITIAL;
  lcdHashLine3 = LCD_HASH_INITIAL;
  delay(2000);
}

void updateLine0 () {
  long newHashLine0 = isOn + (z.leftStop - z.rightStop) + (x.leftStop - x.rightStop) + spindlePosSync + moveStep + mode + measure + setupIndex;
  if (lcdHashLine0 == newHashLine0) 
    return;
  lcdHashLine0 = newHashLine0;
  int charIndex = 0;
  lcd.setCursor(0, 0);
  if (setupIndex == 0 || !isPassMode()) {
    charIndex += printMode();
    charIndex += lcd.print(isOn ? "ON " : "off ");
    int beforeStops = charIndex;
    if (z.leftStop != LONG_MAX)
      charIndex += lcd.write(customCharLimLeftCode);
    if (x.leftStop != LONG_MAX && x.rightStop != LONG_MIN)
      charIndex += lcd.write(customCharLimUpDownCode);
    else if (x.leftStop != LONG_MAX)
      charIndex += lcd.write(customCharLimUpCode);
    else if (x.rightStop != LONG_MIN)
      charIndex += lcd.write(customCharLimDownCode);
    if (z.rightStop != LONG_MIN)
      charIndex += lcd.write(customCharLimRightCode);
    if (beforeStops != charIndex)
      charIndex += lcd.print(" ");
    if (spindlePosSync && !isPassMode())
      charIndex += lcd.print("SYN ");
    if (mode == MODE_NORMAL && !spindlePosSync)
      charIndex += lcd.print("step ");
    charIndex += printDeciMicrons(moveStep, 5);
  } else {
    if (needZStops()) {
      charIndex += lcd.write(customCharLimLeftRightCode);
      charIndex += printAxisStopDiff(&z, true);
      while (charIndex < 10) charIndex += lcd.print(" ");
    } else
      charIndex += printMode();
    charIndex += lcd.write(customCharLimUpDownCode);
    charIndex += printAxisStopDiff(&x, false);
  }
  printLcdSpaces(charIndex);
}

void updateLine1() {
  long newHashLine1 = dupr + starts + mode + measure + setupIndex;
  if (lcdHashLine1 == newHashLine1)
    return;
  lcdHashLine1 = newHashLine1;
  int charIndex = 0;
  lcd.setCursor(0, 1);
  charIndex += lcd.print("Pitch ");
  charIndex += printDupr(dupr);
  if (starts != 1) {
    charIndex += lcd.print(" x");
    charIndex += lcd.print(starts);
  }
  printLcdSpaces(charIndex);
}

void updateLine2() {
  long zDisplayPos = z.pos + z.originPos;
  long xDisplayPos = x.pos + x.originPos;
  long a1DisplayPos = a1.pos + a1.originPos;
  long newHashLine2 = zDisplayPos + xDisplayPos + a1DisplayPos + measure + z.disabled + x.disabled + mode;
  if (lcdHashLine2 == newHashLine2) 
    return;
  lcdHashLine2 = newHashLine2;
  int charIndex = 0;
  lcd.setCursor(0, 2);
  charIndex += printAxisPosWithName(&z, true);
  while (charIndex < 10) charIndex += lcd.print(" ");
  charIndex += printAxisPosWithName(&x, true);
  printLcdSpaces(charIndex);
}

void updateLine3() {
  int rpm = showTacho ? getApproxRpm() : 0;
  long numpadResult = getNumpadResult();
  long gcodeCommandHash = 0;
  for (int i = 0; i < gcodeCommand.length(); i++)
    gcodeCommandHash += gcodeCommand.charAt(i);
  long newHashLine3 = z.pos + (showAngle ? spindlePos : -1) + (showTacho ? rpm : -2) + measure + (numpadResult > 0 ? numpadResult : -1) + mode * 5 + dupr +
      (mode == MODE_CONE ? round(coneRatio * 10000) : 0) + turnPasses + opIndex + setupIndex + (isOn ? 139 : -117) + (in_Numpad() ? 10 : 0) + (auxForward ? 17 : -31) +
      (z.leftStop == LONG_MAX ? 123 : z.leftStop) + (z.rightStop == LONG_MIN ? 1234 : z.rightStop) +
      (x.leftStop == LONG_MAX ? 1235 : x.leftStop) + (x.rightStop == LONG_MIN ? 123456 : x.rightStop) + gcodeCommandHash +
      (mode == MODE_A1 ? a1.pos + a1.originPos + (a1.leftStop == LONG_MAX ? 123 : a1.leftStop) + (a1.rightStop == LONG_MIN ? 1234 : a1.rightStop) + a1.disabled : 0) + x.pos + z.pos;
  if (lcdHashLine3 == newHashLine3)
    return;
  lcdHashLine3 = newHashLine3;
  int charIndex = 0;
  lcd.setCursor(0, 3);
  if (mode == MODE_A1 && !in_Numpad()) {
    if (a1.leftStop != LONG_MAX && a1.rightStop != LONG_MIN) {
      charIndex += lcd.write(customCharLimUpDownCode);
      charIndex += lcd.print(" ");
    } else if (a1.leftStop != LONG_MAX) {
      charIndex += lcd.write(customCharLimDownCode);
      charIndex += lcd.print(" ");
    } else if (a1.rightStop != LONG_MIN) {
      charIndex += lcd.write(customCharLimUpCode);
      charIndex += lcd.print(" ");
    }
    charIndex += printAxisPosWithName(&a1, false);
  } else if (mode == MODE_GCODE) 
    charIndex += lcd.print(gcodeCommand.substring(0, 20));
  else if (isPassMode()) {
    bool missingZStops = needZStops() && (z.leftStop == LONG_MAX || z.rightStop == LONG_MIN);
    bool missingStops = missingZStops || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN;
    if (!in_Numpad() && missingStops) {
      charIndex += lcd.print(needZStops() ? "Set all stops" : "Set X stops");
    } else if (numpadResult != 0 && setupIndex == 1) {
      long passes = min(PASSES_MAX, numpadResult);
      charIndex += lcd.print(passes);
      if (passes == 1) charIndex += lcd.print(" pass?");
      else charIndex += lcd.print(" passes?");
    } else if (!isOn && setupIndex == 1) {
      charIndex += lcd.print(turnPasses);
      if (turnPasses == 1) charIndex += lcd.print(" pass?");
      else charIndex += lcd.print(" passes?");
    } else if (!isOn && setupIndex == 2) {
      if (mode == MODE_FACE) {
        charIndex += lcd.print(auxForward ? "Right to left?" : "Left to right?");
      } else if (mode == MODE_CUT) {
        charIndex += lcd.print(dupr >= 0 ? "Pitch > 0, external" : "Pitch < 0, internal");
      } else {
        charIndex += lcd.print(auxForward ? "External?" : "Internal?");
      }
    } else if (!isOn && setupIndex == 3) {
      long zOffset = getPassModeZStart() - z.pos;
      long xOffset = getPassModeXStart() - x.pos;
      charIndex += lcd.print("Go");
      if (zOffset != 0) {
        charIndex += lcd.print(" ");
        charIndex += lcd.print(z.name);
        charIndex += printDeciMicrons(stepsToDu(&z, zOffset), 2);
      }
      if (xOffset != 0) {
        charIndex += lcd.print(" ");
        charIndex += lcd.print(x.name);
        charIndex += printDeciMicrons(stepsToDu(&x, xOffset), 2);
      }
      charIndex += lcd.print("?");
    } else if (isOn && numpadResult == 0) {
      charIndex += lcd.print("Pass ");
      charIndex += lcd.print(opIndex);
      charIndex += lcd.print(" of ");
      charIndex += lcd.print(max(opIndex, long(turnPasses * starts)));
    }
  } else if (mode == MODE_CONE) {
    if (numpadResult != 0 && setupIndex == 1) {
      charIndex += lcd.print("Use ratio ");
      charIndex += lcd.print(numpadToConeRatio(), 5);
      charIndex += lcd.print("?");
    } else if (!isOn && setupIndex == 1) {
      charIndex += lcd.print("Use ratio ");
      charIndex += printNoTrailing0(coneRatio);
      charIndex += lcd.print("?");
    } else if (!isOn && setupIndex == 2) {
      charIndex += lcd.print(auxForward ? "External?" : "Internal?");
    } else if (!isOn && setupIndex == 3) {
      charIndex += lcd.print("Go?");
    } else if (isOn && numpadResult == 0) {
      charIndex += lcd.print("Cone ratio ");
      charIndex += printNoTrailing0(coneRatio);
    }
  }
  if (charIndex == 0 && in_Numpad()) { // Also show for 0 input to allow setting limits to 0.
    charIndex += lcd.print("Use ");
    charIndex += printDupr(numpadToDeciMicrons());
     charIndex += lcd.print("?");
  }
  if (charIndex > 0) {
    // No space for shared RPM/angle text.
  } else if (showAngle) {
    charIndex += lcd.print("Angle ");
    charIndex += lcd.print(spindleModulo(spindlePos) * 360 / ENCODER_STEPS_FLOAT, 2);
    charIndex += lcd.print(char(223));
  } else if (showTacho) {
    charIndex += lcd.print("Tacho ");
    charIndex += lcd.print(rpm);
    if (shownRpm != rpm) {
      shownRpm = rpm;
      shownRpmTime = micros();
    }
    charIndex += lcd.print("rpm");
  }
  printLcdSpaces(charIndex);
}

//==============================================================================
// Interface

void updateDisplay() {
  if (splashScreen) {
    showSplashScreen();
    lcd.clear();
  }
  updateLine0();
  updateLine1();
  updateLine2();
  updateLine3();
}

void estopDisplay() {
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
};

void taskDisplay(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    updateDisplay();
    if (writePreferences()) {
      powerOnBeep();
      powerOnBeep();
    }
    if (beepFlag) {
      beepFlag = false;
      warningBeep();
    }
    if (abs(z.pendingPos) > z.estopSteps || abs(x.pendingPos) > x.estopSteps) 
      setEmergencyStop(ESTOP_POS);
    taskYIELD();
  }
  estopDisplay();
  vTaskDelete(NULL);
}

void setupDisplay() {
  lcd.begin(20, 4);
  lcd.createChar(customCharMmCode, customCharMm);
  lcd.createChar(customCharLimLeftCode, customCharLimLeft);
  lcd.createChar(customCharLimRightCode, customCharLimRight);
  lcd.createChar(customCharLimUpCode, customCharLimUp);
  lcd.createChar(customCharLimDownCode, customCharLimDown);
  lcd.createChar(customCharLimUpDownCode, customCharLimUpDown);
  lcd.createChar(customCharLimLeftRightCode, customCharLimLeftRight);
  lcdHashLine0 = LCD_HASH_INITIAL;
  lcdHashLine1 = LCD_HASH_INITIAL;
  lcdHashLine2 = LCD_HASH_INITIAL;
  lcdHashLine3 = LCD_HASH_INITIAL;
  xTaskCreatePinnedToCore(taskDisplay, "taskDisplay", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
};