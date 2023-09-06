#pragma once

extern unsigned long saveTime; // micros() of the previous Prefs write

void readPreferences();
bool writePreferences();