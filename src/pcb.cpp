#include <Arduino.h>
#include "pcb.hpp"

void beep() {
  tone(BUZZ, 1000, 500);
}

void setupPins() {
  pinMode(Z_ENA, OUTPUT);
  pinMode(Z_DIR, OUTPUT);
  pinMode(Z_STEP, OUTPUT);
  pinMode(X_ENA, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(X_STEP, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(SCL, OUTPUT);
  pinMode(SDA, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);
  pinMode(A13, OUTPUT);
  pinMode(A21, OUTPUT);
  pinMode(A22, OUTPUT);
  pinMode(A23, OUTPUT);
}