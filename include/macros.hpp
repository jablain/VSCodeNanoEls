#pragma once
#include <Arduino.h>

#define DREAD(x) digitalRead(x)
#define DHIGH(x) digitalWrite(x, HIGH)
#define DLOW(x) digitalWrite(x, LOW)
#define DWRITE(x, y) digitalWrite(x, y)
#define DELAY(x) vTaskDelay(x / portTICK_PERIOD_MS);

#define DEBUG  // Undefine to disable debugging to console
#ifdef DEBUG
  #define DPRINT(x) Serial.print(x)
  #define DPRINT2(x,y) Serial.print(x,y)
  #define DPRINTLN(x) Serial.println(x)
  #define DPRINT2LN(x, y) Serial.println(x, y)
#else
  #define DPRINT(x)
  #define DPRINT2(x,y)
  #define DPRINTLN(x)
  #define DPRINT2LN(x, x)
#endif