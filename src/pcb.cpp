#include <Arduino.h>
#include "macros.hpp"
#include "vars.hpp"
#include "pcb.hpp"

void powerOnBeep(){
  tone(BUZZ, 500, 100);
  DELAY(100);
  tone(BUZZ, 750, 100);
  DELAY(100);
  tone(BUZZ, 1000, 100);
};

void warningBeep(){
  tone(BUZZ, 1500, 500);
  DELAY(500);
  tone(BUZZ, 500, 500);
};

void doneBeep(){
  tone(BUZZ, 1000, 500);
};

void setupPCB(){
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
  Serial.begin(115200); // Used to send GCode and (maybe for debugging).
};
