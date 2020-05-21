#include "DCC.h"

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

    hdw.enable_pin = PIN_PA09;
    hdw.enable_pin_group = 0;

    hdw.current_sense_pin = A5;
    hdw.trigger_value = 10;         //Todo: set correctly

    return new DCC(numDev, hdw);
}

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

    hdw.enable_pin = PIN_PA08;
    hdw.enable_pin_group = 0;

    hdw.current_sense_pin = A1;
    hdw.trigger_value = 10;         //Todo: set correctly

    return new DCC(numDev, hdw);
}