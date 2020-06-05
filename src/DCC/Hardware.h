#ifndef Hardware_h
#define Hardware_h

#include <Arduino.h>
#include <ArduinoTimers.h>  

// Library DIO2.h is only compatible with AVR, and SAM digitalWrite is a lot faster than AVR digitalWrite.
#if defined(ARDUINO_ARCH_AVR)
#include <DIO2.h>
#endif

// Todo: determine if we really need to slow down the arduino Uno's current sampling
#if defined(ARDUINO_AVR_UNO)
const int CURRENT_SAMPLE_TIME = 10;
#else
const int CURRENT_SAMPLE_TIME = 1;
#endif
const float CURRENT_SAMPLE_SMOOTHING = 0.01;

// Number of milliseconds between retries when the "breaker" is tripped.
const int RETRY_MILLIS = 10000;

enum control_type_t : uint8_t {
    DIRECTION_ENABLE,           // One direction pin and one enable pin. Active high on both. Railcom is not supported with this setup.
    DUAL_DIRECTION_INVERTED,    // Two pins plus enable, each pin controlling half of the H-bridge. Set both high to put the shield in brake. Example: DRV8876
    DIRECTION_BRAKE_ENABLE      // Separate brake and enable lines. Brake is active high. Example: Arduino Motor Shield
};

class Hardware {
public:
    Hardware() {}
    void setup();

    bool getIsProgTrack() { return is_prog_track; }
    uint8_t getPreambles() { return preambleBits; }

    void setPower(bool on);
    void setSignal(bool high);
    void setBrake(bool on);
    bool getStatus();

    // Current reading stuff
    uint32_t readCurrent();
    float getLastRead() { return reading; }
    float getLastMilliamps() { return current; }
    float getMilliamps(uint32_t reading);
    void checkCurrent();
    void checkAck();
    void setBaseCurrent();
    
    // Railcom stuff
#if defined(ARDUINO_ARCH_SAMD) 
    void enableRailcomDAC();
    Uart* railcomSerial() { return railcom_serial; }
#else
    HardwareSerial* railcomSerial() { return railcom_serial; }
#endif
    void enableRailcomSerial(bool on);
    void readRailcomData();
    bool getRailcomEnable() { return enable_railcom; }

    // Configuration modification stuff
    void config_setChannelName(const char *name) { channel_name = name; }
    void config_setControlScheme(control_type_t scheme) { control_scheme = scheme; }
    void config_setProgTrack(bool isProgTrack) { is_prog_track = isProgTrack; }
    void config_setPinSignalA(uint8_t pin) { signal_a_pin = pin; }
    void config_setPinSignalB(uint8_t pin) { signal_b_pin = pin; }
    void config_setDefaultSignalB(bool default_state) { signal_b_default = default_state; }
    void config_setPinEnable(uint8_t pin) { enable_pin = pin; }
    void config_setPinCurrentSense(uint8_t pin) { current_sense_pin = pin; }
    void config_setTriggerValue(int triggerValue) { trigger_value = triggerValue; }
    void config_setMaxValue(int maxValue) { maximum_value = maxValue; }
    void config_setAmpsPerVolt(float ampsPerVolt) { amps_per_volt = ampsPerVolt; }
    void config_setPreambleBits(uint8_t preambleBits) { this->preambleBits = preambleBits; }
    void config_setRailcom(bool isRailcom) { enable_railcom = isRailcom; }
    void config_setRailcomRxPin(uint8_t pin) { railcom_rx_pin = pin; }
    void config_setRailcomTxPin(uint8_t pin) { railcom_tx_pin = pin; }
    void config_setRailcomBaud(long baud) { railcom_baud = baud; }
#if defined(ARDUINO_ARCH_SAMD) 
    void config_setRailcomSerial(Uart* serial) { railcom_serial = serial; }
    void config_setRailcomSercom(SERCOM* sercom) { railcom_sercom = sercom; }
    void config_setRailcomRxMux(EPioType mux) { railcom_rx_mux = mux; }
    void config_setRailcomRxPad(SercomRXPad pad) { railcom_rx_pad = pad; }
    void config_setRailcomTxPad(SercomUartTXPad pad) { railcom_tx_pad = pad; }
    void config_setRailcomDACValue(uint8_t value) { railcom_dac_value = value; }
#else
    void config_setRailcomSerial(HardwareSerial* serial) { railcom_serial = serial; }
#endif


    // ACK detection stuff
    float baseMilliamps;

private:
    const char *channel_name;
    control_type_t control_scheme;
    bool is_prog_track;
    uint8_t preambleBits;

    uint8_t signal_a_pin;
    uint8_t signal_b_pin;       // Inverted output if DUAL_DIRECTION_ENABLED, brake pin if DIRECTION_BRAKE_ENABLE, else not enabled
    bool signal_b_default;      // Default state of signal B pin. If true, the signal B pin is HIGH by default. Else low.
    uint8_t enable_pin;
    uint8_t current_sense_pin;

    int trigger_value;          // Trigger value in milliamps
    int maximum_value;          // Maximum current in milliamps
    float amps_per_volt;        

    // Railcom stuff
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
    uint8_t railcom_dac_value;      // Sets the DAC according to the calculation in the datasheet for a 1V reference
#else
    HardwareSerial* railcom_serial;
#endif

    // Current reading stuff
    float reading;
	float current;
	bool tripped;
	long int lastCheckTime;
    long int lastTripTime;

    
};

#endif