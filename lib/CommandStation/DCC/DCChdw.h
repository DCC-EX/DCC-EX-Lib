#ifndef DCCHardware_h
#define DCCHardware_h

#include <Arduino.h>

// Using a 1:4 prescaler value at a 48MHz clock rate
// Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with exactly 50% duty cycle
#define DCC_ZERO_BIT_TOTAL_DURATION 2400
#define DCC_ZERO_BIT_PULSE_DURATION 1200
#define DCC_ONE_BIT_TOTAL_DURATION 1392
#define DCC_ONE_BIT_PULSE_DURATION 696

#define RC_PULSE_DURATION 348 // 29us
#define RC_CUTOUT_DURATION 5220 // 435us

struct DCChdw {
    bool is_prog_track;
    bool use_dual_signal;
    Tcc* timer;
    uint8_t timer_num, gclk_num;
    uint8_t signal_a_pin, signal_a_group, signal_a_timer_bit, signal_a_timer_mux;
    uint8_t signal_b_pin, signal_b_group, signal_b_timer_bit, signal_b_timer_mux;
    uint8_t enable_pin;
    uint8_t current_sense_pin;
    int trigger_value;

    /////////////////////////////////////////////////////////////////////////////////////
    // current in milliamps computed from ((Vcc/4096)/volts_per_amp) * 1000
    // VCC is the voltage range of the sensor pin (3.3v) while 4096
    // is the resolution or the range of the sensor (12bits). volts_per_amp
    // is the number of volts out of the current sensor board that corresponds
    // to one amp of current.
    /////////////////////////////////////////////////////////////////////////////////////
    float current_conversion_factor;

    uint8_t preambleBits;
};

#endif