#ifndef Hardware_h
#define Hardware_h

#include <Arduino.h>
#include <ArduinoTimers.h>  

// Library DIO2.h is only compatible with AVR, and SAM digitalWrite is a lot faster.
#if defined(ARDUINO_ARCH_AVR)
#include <DIO2.h>
#endif

// Define constants used for managing the current on the track
const int CURRENT_SAMPLE_TIME = 1;
const float CURRENT_SAMPLE_SMOOTHING = 0.01;
const int RETRY_MILLIS = 1000;

enum control_type_t : uint8_t {
    DIRECTION_ENABLE,
    DUAL_DIRECTION_INVERTED,
    DIRECTION_BRAKE_ENABLE
};

struct Hardware {
    Hardware();

    control_type_t control_scheme;

    bool is_prog_track;
    bool enable_railcom;

    uint8_t railcom_rx_pin;
    uint8_t railcom_tx_pin;     // Doesn't do anything, but valid pin must be specified to instantiate railcom_serial on some architectures
    long railcom_baud;
#if defined(ARDUINO_ARCH_SAMD) 
    Uart* railcom_serial;
    SERCOM* railcom_sercom;
    EPioType railcom_rx_mux;
    SercomRXPad railcom_rx_pad;
    SercomUartTXPad railcom_tx_pad;
#else
    HardwareSerial* railcom_serial;
#endif

    uint8_t signal_a_pin;
    uint8_t signal_b_pin;       // Inverted output if DUAL_DIRECTION_ENABLED, brake pin if DIRECTION_BRAKE_ENABLE
    uint8_t enable_pin;
    uint8_t current_sense_pin;

    int trigger_value;
    float amps_per_volt;

    uint8_t preambleBits;

    // Current reading stuff
    float reading;
	float current;
	bool tripped;
	long int lastCheckTime;
    long int lastTripTime;

    void init();
    void setPower(bool on);
    void setSignal(bool high);
    void setBrake(bool on);
    float getMilliamps() { return current; }
    float getMilliamps(float reading);
    void checkCurrent();
    void enableRailcomSerial(bool on);

    // Configuration modification stuff
    void config_setProgTrack(bool isProgTrack) { is_prog_track = isProgTrack; }
    void config_setRailcom(bool isRailcom) { enable_railcom = isRailcom; }
    void config_setPinSignalA(uint8_t pin) { signal_a_pin = pin; }
    void config_setPinSignalB(uint8_t pin) { signal_b_pin = pin; }
    void config_setPinEnable(uint8_t pin) { enable_pin = pin; }
    void config_setCurrentSensePin(uint8_t pin) { current_sense_pin = pin; }
    void config_setTriggerValue(int triggerValue) { trigger_value = triggerValue; }
    void config_setAmpsPerVolt(float ampsPerVolt) { amps_per_volt = ampsPerVolt; }
    void config_setPreambleBits(uint8_t preambleBits) { this->preambleBits = preambleBits; }
#if defined(ARDUINO_ARCH_SAMD) 
    void config_setRailcomSerial(Uart* serial) { railcom_serial = serial; }
    void config_setRailcomSercom(SERCOM* sercom) { railcom_sercom = sercom; }
    void config_setRailcomRxMux(EPioType mux) { railcom_rx_mux = mux; }
    void config_setRailcomRxPad(SercomRXPad pad) { railcom_rx_pad = pad; }
    void config_setRailcomTxPad(SercomUartTXPad pad) { railcom_tx_pad = pad; }
#else
    void config_setRailcomSerial(HardwareSerial* serial) { railcom_serial = serial; }
#endif
};

#endif