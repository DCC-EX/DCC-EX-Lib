#ifndef Hardware_h
#define Hardware_h

#include <Arduino.h>
#include "../../DCC.h"

// Using a 1:4 prescaler value at a 48MHz clock rate
// Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with exactly 50% duty cycle
const int DCC_ZERO_BIT_TOTAL_DURATION = 2400;
const int DCC_ZERO_BIT_PULSE_DURATION = 1200;
const int DCC_ONE_BIT_TOTAL_DURATION = 1392;
const int DCC_ONE_BIT_PULSE_DURATION = 696;

const int RC_PULSE_DURATION = 348; // 29us
const int RC_CUTOUT_DURATION = 5220; // 435us

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
    void (*volatile timer_callback)();

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