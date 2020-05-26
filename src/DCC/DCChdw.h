#ifndef DCCHardware_h
#define DCCHardware_h

#include <Arduino.h>
#include <ArduinoTimers.h>

enum control_type_t : uint8_t {
    DIRECTION_ENABLE,
    DUAL_DIRECTION_INVERTED,
    DIRECTION_BRAKE_ENABLE
};

struct DCChdw {
    control_type_t control_scheme;

    bool is_prog_track;
    bool enable_railcom;
    Timer* railcom_timer;       // Optional if !enable_railcom

    uint8_t signal_a_pin;
    uint8_t signal_b_pin;       // Inverted output if DUAL_DIRECTION_ENABLED, brake pin if DIRECTION_BRAKE_ENABLE
    uint8_t enable_pin;
    uint8_t current_sense_pin;

    int trigger_value;
    float amps_per_volt;

    uint8_t preambleBits;
};

#endif