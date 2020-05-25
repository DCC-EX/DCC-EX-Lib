#ifndef DCCHardware_h
#define DCCHardware_h

#include <Arduino.h>

struct DCChdw {
    bool is_prog_track;
    bool use_dual_signal;
    bool enable_railcom;
    Sercom* railcom_sercom;
    Tcc* timer;
    uint8_t gclk_num;
    uint8_t signal_a_pin;
    uint8_t signal_b_pin;
    uint8_t enable_pin;
    uint8_t current_sense_pin;
    int trigger_value;

    /////////////////////////////////////////////////////////////////////////////////////
    // current in milliamps computed from ((Vcc/4096)/volts_per_amp) * 1000
    // VCC is the voltage range of the sensor pin (3.3v) while 4096
    // is the resolution or the range of the sensor (12bits). volts_per_amp
    // is the number of volts out of the current sensor board that corresponds
    // to one amp of current.
    
    float current_conversion_factor;

    /////////////////////////////////////////////////////////////////////////////////////

    uint8_t preambleBits;
};

#endif