#include "DCC.h"

DCC* DCC::Create_Arduino_L298Shield_Main(int numDev) {
    DCChdw hdw;

    hdw.control_scheme = DIRECTION_BRAKE_ENABLE;

    hdw.is_prog_track = false;
    hdw.enable_railcom = true;
    hdw.railcom_timer = &TimerB;    // Timer2 on UNO, Timer3 on MEGA, TCC1 on SAMD

    hdw.signal_a_pin = 12;
    hdw.signal_b_pin = 9;

    hdw.enable_pin = 4;

    hdw.current_sense_pin = A0;
    hdw.trigger_value = 1500; // Trips at 1500mA
    hdw.amps_per_volt = 0.606061;

    hdw.preambleBits = 16;

    return new DCC(numDev, hdw);
}

DCC* DCC::Create_Arduino_L298Shield_Prog(int numDev) {
    DCChdw hdw;
    
    hdw.control_scheme = DIRECTION_BRAKE_ENABLE;

    hdw.is_prog_track = true;
    hdw.enable_railcom = false;
    hdw.railcom_timer = NULL;

    hdw.signal_a_pin = 13;  
    hdw.signal_b_pin = 8;  

    hdw.enable_pin = 11;     // Arduino pin

    hdw.current_sense_pin = A1; // Arduino pin
    hdw.trigger_value = 250;  // Trips at 250mA
    hdw.amps_per_volt = 0.606061;

    hdw.preambleBits = 22;

    return new DCC(numDev, hdw);
}

/////////////////////////////////////////////////////////////////////////////////////

DCC* DCC::Create_Pololu_MC33926Shield_Main(int numDev) {
    DCChdw hdw;

    hdw.control_scheme = DIRECTION_BRAKE_ENABLE;

    hdw.is_prog_track = false;
    hdw.enable_railcom = true;
    hdw.railcom_timer = &TimerB;    // Timer2 on UNO, Timer3 on MEGA, TCC1 on SAMD

    hdw.signal_a_pin = 7;
    hdw.signal_b_pin = 9;

    hdw.enable_pin = 4;

    hdw.current_sense_pin = A0;
    hdw.trigger_value = 3000; // Trips at 3000mA
    hdw.amps_per_volt = 1.904762;

    hdw.preambleBits = 16;

    return new DCC(numDev, hdw);
}

DCC* DCC::Create_Pololu_MC33926Shield_Prog(int numDev) {
    DCChdw hdw;
    
    hdw.control_scheme = DIRECTION_BRAKE_ENABLE;

    hdw.is_prog_track = true;
    hdw.enable_railcom = false;
    hdw.railcom_timer = NULL;

    hdw.signal_a_pin = 8;  
    hdw.signal_b_pin = 10;  

    hdw.enable_pin = 5;     // Enables are tied together by default. Cut the !D2 jumper and connect the PROG channel !D2 to this pin.

    hdw.current_sense_pin = A1; // Arduino pin
    hdw.trigger_value = 250;  // Trips at 250mA
    hdw.amps_per_volt = 1.904762;

    hdw.preambleBits = 22;

    return new DCC(numDev, hdw);
}

/////////////////////////////////////////////////////////////////////////////////////

// TI DRV8874 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Main(int numDev) {
    DCChdw hdw;

    hdw.control_scheme = DUAL_DIRECTION_INVERTED;

    hdw.is_prog_track = false;
    hdw.enable_railcom = true;
    hdw.railcom_timer = &TimerB;

    hdw.signal_a_pin = 6;
    hdw.signal_b_pin = 7;

    hdw.enable_pin = 3;

    hdw.current_sense_pin = A5;
    hdw.trigger_value = 5500; // Trips at 5500mA
    hdw.amps_per_volt = 1.998004;

    hdw.preambleBits = 16;

    return new DCC(numDev, hdw);
}

// TI DRV8876 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Prog(int numDev) {
    DCChdw hdw;
    
    hdw.control_scheme = DUAL_DIRECTION_INVERTED;

    hdw.is_prog_track = true;
    hdw.enable_railcom = false;
    hdw.railcom_timer = NULL;

    hdw.signal_a_pin = 8;  
    hdw.signal_b_pin = 9;  

    hdw.enable_pin = 4;     // Arduino pin

    hdw.current_sense_pin = A1; // Arduino pin
    hdw.trigger_value = 250;  // Trips at 250mA
    hdw.amps_per_volt = 0.909089;

    hdw.preambleBits = 22;

    return new DCC(numDev, hdw);
}