#include "DCC.h"

// DRV8874 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Main(int numDev) {
    DCChdw hdw;

    hdw.track_name = "Main";
    hdw.is_prog_track = false;
    hdw.use_dual_signal = 1;

    hdw.timer_num = 0;
    hdw.gclk_num = 4;

    hdw.signal_a_pin = PIN_PA20;
    hdw.signal_a_group = 0;
    hdw.signal_a_timer_bit = 6;
    hdw.signal_a_timer_mux = MUX_PA20F_TCC0_WO6;

    hdw.signal_b_pin = PIN_PA21;
    hdw.signal_b_group = 0;
    hdw.signal_b_timer_bit = 7;
    hdw.signal_b_timer_mux = MUX_PA21F_TCC0_WO7;

    hdw.enable_pin = 3; // Arduino pin

    hdw.current_sense_pin = A5; // Arduino pin
    hdw.trigger_value = 5500; // Trips at 5500mA
    hdw.current_conversion_factor = 1.60972; // Sanity check: 4096*1.60972 = 6593.40 mA, about right.

    return new DCC(numDev, hdw);
}

// DRV8876 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Prog(int numDev) {
    DCChdw hdw;
    
    hdw.track_name = "Prog";
    hdw.is_prog_track = true;
    hdw.use_dual_signal = 1;

    hdw.timer_num = 1;
    hdw.gclk_num = 5;

    hdw.signal_a_pin = PIN_PA06;
    hdw.signal_a_group = 0;
    hdw.signal_a_timer_bit = 0;
    hdw.signal_a_timer_mux = MUX_PA06E_TCC1_WO0;

    hdw.signal_b_pin = PIN_PA07;
    hdw.signal_b_group = 0;
    hdw.signal_b_timer_bit = 1;
    hdw.signal_b_timer_mux = MUX_PA07E_TCC1_WO1;

    hdw.enable_pin = 4;     // Arduino pin

    hdw.current_sense_pin = A1; // Arduino pin
    hdw.trigger_value = 250;       
    hdw.current_conversion_factor = 0.73242;

    return new DCC(numDev, hdw);
}