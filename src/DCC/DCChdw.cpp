#include "DCC.h"

// DRV8874 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Main(int numDev) {
    DCChdw hdw;

    hdw.control_scheme = DUAL_DIRECTION_INVERTED;

    hdw.is_prog_track = false;
    hdw.enable_railcom = true;

    hdw.signal_a_pin = 6u;  // Arduino pin, used always as the direction pin
    hdw.signal_b_pin = 7u;  // Arduino pin, used as the direction pin, brake pin, or disabled

    hdw.enable_pin = 3; // Arduino pin

    hdw.current_sense_pin = A5; // Arduino pin
    hdw.trigger_value = 5500; // Trips at 5500mA
    hdw.current_conversion_factor = 1.60972; // Sanity check: 4096*1.60972 = 6593.40 mA, about right.

    hdw.preambleBits = 16;

#if defined (ATSAMD21G)
    return new DCC(numDev, hdw, TimerTCC0);
#elif defined (ATMEGA2560)
    return new DCC(numDev, hdw, Timer1);
#endif
}

// DRV8876 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Prog(int numDev) {
    DCChdw hdw;
    
    hdw.control_scheme = DUAL_DIRECTION_INVERTED;

    hdw.is_prog_track = true;
    hdw.enable_railcom = false;

    hdw.signal_a_pin = 8u;  // Arduino pin, used always as the direction pin
    hdw.signal_b_pin = 9u;  // Arduino pin, used as the direction pin, brake pin, or disabled

    hdw.enable_pin = 4;     // Arduino pin

    hdw.current_sense_pin = A1; // Arduino pin
    hdw.trigger_value = 250;  // Trips at 250mA
    hdw.current_conversion_factor = 0.73242; // Sanity check: 4096*0.73242 = 2999.99 mA, about right.

    hdw.preambleBits = 22;
#if defined (ATSAMD21G)
    return new DCC(numDev, hdw, TimerTCC1);
#elif defined (ATMEGA2560)
    return new DCC(numDev, hdw, Timer3);
#endif
}