/*
 *  Hardware.h
 * 
 *  This file is part of CommandStation.
 *
 *  CommandStation is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CommandStation is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMMANDSTATION_DCC_HARDWARE_H_
#define COMMANDSTATION_DCC_HARDWARE_H_

#include "AnalogReadFast.h"

#include <Arduino.h>
#include <ArduinoTimers.h>

// Time between current samples (millis)
const int kCurrentSampleTime = 1;

// Smoothing constant for exponential current smoothing algorithm
const float kCurrentSampleSmoothing = 0.01;

// Number of milliseconds between retries when the "breaker" is tripped.
const int kRetryTime = 10000;

enum control_type_t : uint8_t {
  // One direction pin and one enable pin. Active high on both. Railcom is not 
  // supported with this setup.
  DIRECTION_ENABLE,           
  // Two pins plus enable, each pin controlling half of the H-bridge. Set both 
  // high to put the shield in brake. Example: DRV8876
  DUAL_DIRECTION_INVERTED,    
  // Separate brake and enable lines. Brake is active high. Example: Arduino 
  // Motor Shield
  DIRECTION_BRAKE_ENABLE      
};

class Hardware {
public:
  Hardware() {}
  void setup();

  // General configuration and status getter functions
  bool getStatus() { return digitalRead(enable_pin); }
  uint8_t getPreambles() { return preambleBits; }
  
  // Waveform control functions
  void setPower(bool on);
  void setSignal(bool high);
  void setBrake(bool on);

  // Checks for overcurrent and manages power. Call often.
  void checkCurrent();

  // Current functions
  float getLastRead() { return reading; }
  float getLastMilliamps() { return current; }
  float getMilliamps() { return getMilliamps(readCurrent()); }
  
  void setBaseCurrent() { baseMilliamps = getMilliamps(readCurrent()); }
  float getBaseCurrent() { return baseMilliamps; }
  

  // General config modification
  void config_setChannelName(const char *name) { channel_name = name; }
  void config_setControlScheme(control_type_t scheme) 
    { control_scheme = scheme; }
  void config_setPreambleBits(uint8_t preambleBits) 
    { this->preambleBits = preambleBits; }

  // Pin config modification
  void config_setPinSignalA(uint8_t pin) { signal_a_pin = pin; }
  void config_setPinSignalB(uint8_t pin) { signal_b_pin = pin; }
  void config_setDefaultSignalB(bool default_state) 
    { signal_b_default = default_state; }
  void config_setPinEnable(uint8_t pin) { enable_pin = pin; }
  void config_setPinCurrentSense(uint8_t pin) { current_sense_pin = pin; }

  // Current config modification
  void config_setTriggerValue(int triggerValue) 
    { trigger_value = triggerValue; }
  void config_setMaxValue(int maxValue) { maximum_value = maxValue; }
  void config_setAmpsPerVolt(float ampsPerVolt) { amps_per_volt = ampsPerVolt; }

private:
  float getMilliamps(uint32_t reading);
  uint32_t readCurrent() {
    return analogReadFast(current_sense_pin);
  }

  const char *channel_name;
  control_type_t control_scheme;
  uint8_t preambleBits;

  uint8_t signal_a_pin;
  uint8_t signal_b_pin;       // Inverted output if DUAL_DIRECTION_ENABLED, 
                              // brake pin if DIRECTION_BRAKE_ENABLE, else not 
                              // enabled
  uint8_t signal_b_default;   // Default state of signal B pin. If true, the 
                              // signal B pin is HIGH by default. Else low.
  uint8_t enable_pin;
  uint8_t current_sense_pin;

  int trigger_value;          // Trigger value in milliamps
  int maximum_value;          // Maximum current in milliamps
  float amps_per_volt;        

  // Current reading variables
  float reading;
  float current;
  bool tripped;
  long int lastCheckTime;
  long int lastTripTime;

  // ACK detection base current
  float baseMilliamps;
};

#endif  // COMMANDSTATION_DCC_HARDWARE_H_