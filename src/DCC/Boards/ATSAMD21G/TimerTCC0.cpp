#ifdef ATSAMD21G

#include "TimerTCC0.h"

TimerTCC0 TimerTCC0Inst;

void (*TimerTCC0::isrCallback)() = TimerTCC0::isrDefaultUnused;

void TCC0_Handler() {
    TimerTCC0Inst.isrCallback();
}

#endif