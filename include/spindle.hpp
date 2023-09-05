#pragma once

#include <atomic>
#include "config.hpp"

//#define WITHSPINDLECLASS
#ifdef WITHSPINDLECLASS
class Spindle {
public:
    const float ENCODER_STEPS_FLOAT = ENCODER_STEPS_INT; // Convenience float version of ENCODER_STEPS_INT
    volatile int pulse1Delta;
    volatile int pulse2Delta;

    static Spindle& getInstance();

    // Deleted to prevent copying and assignment
    Spindle(Spindle const&) = delete;
    void operator=(Spindle const&) = delete;

    void zeroSpindlePos();
    int getApproxRpm();
    long spindleModulo(long value);
    void discountFullSpindleTurns();
    void processSpindlePosDelta();

    static void spinEnc();
    static void pulse1Enc();
    static void pulse2Enc();
    void taskAttachInterrupts();
private:
    Spindle();  // Private to restrict direct instantiation

    std::atomic<long> spindlePosDelta;
    unsigned long spindleEncTime;
    unsigned long spindleEncTimeDiffBulk;
    unsigned long spindleEncTimeAtIndex0;
    int spindleEncTimeIndex;
    long spindlePos;
    long spindlePosAvg;
    int spindlePosSync;
    long spindlePosGlobal;
    unsigned long pulse1HighMicros;
    unsigned long pulse2HighMicros;
};

  extern Spindle& TheSpindle;

#else

extern unsigned long spindleEncTime; // micros() of the previous spindle update
extern unsigned long spindleEncTimeDiffBulk; // micros() between RPM_BULK spindle updates
extern unsigned long spindleEncTimeAtIndex0; // micros() when spindleEncTimeIndex was 0
extern int spindleEncTimeIndex; // counter going between 0 and RPM_BULK - 1
extern long spindlePos; // Spindle position
extern long spindlePosAvg; // Spindle position accounting for encoder backlash
extern std::atomic<long> spindlePosDelta; // Unprocessed encoder ticks. see https://forum.arduino.cc/t/does-c-std-atomic-work-with-dual-core-esp32/690214
extern int spindlePosSync; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
extern long spindlePosGlobal; // global spindle position that is unaffected by e.g. zeroing


const float ENCODER_STEPS_FLOAT = ENCODER_STEPS_INT; // Convenience float version of ENCODER_STEPS_INT

extern volatile int pulse1Delta; // Outstanding pulses generated by pulse generator on terminal A1.
extern volatile int pulse2Delta; // Outstanding pulses generated by pulse generator on terminal A2.

int getApproxRpm();
long spindleModulo(long value);
void zeroSpindlePos();
void processSpindlePosDelta();
void discountFullSpindleTurns();
// Called on a FALLING interrupt for the spindle rotary encoder pin.
void IRAM_ATTR spinEnc();
// Called on a FALLING interrupt for the first axis rotary encoder pin.
void IRAM_ATTR pulse1Enc();
// Called on a FALLING interrupt for the second axis rotary encoder pin.
void IRAM_ATTR pulse2Enc();
void taskAttachInterrupts(void *param);

#endif