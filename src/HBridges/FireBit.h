/*
 *  FireBit.h
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

#ifndef COMMANDSTATION_HBRIDGES_FIREBIT_H_
#define COMMANDSTATION_HBRIDGES_FIREBIT_H_

#include "HBridge.h"

struct HBridgeConfigFireBit : public HBridgeConfig {
  int fault_pin;
  int limit_pin;
  int cutout_pin;
};

class HBridgeFireBit : public HBridge
{
public:
  HBridgeConfigFireBit config;

  HBridgeFireBit(HBridgeConfigFireBit _config) {
    config = _config;
  }

  static HBridgeConfigFireBit getDefaultConfig( 
    const char* track_name,
    int signal_a_pin, 
    int signal_b_pin, 
    int sleep_pin, 
    int sense_pin, 
    // Board specific configs
    int fault_pin = NOT_A_PIN, 
    int limit_pin = NOT_A_PIN,
    int cutout_pin = NOT_A_PIN
  ) {  
    HBridgeConfigFireBit _config;

    _config.track_name = track_name;
    _config.signal_a_pin = signal_a_pin;
    _config.signal_b_pin = signal_b_pin;
    _config.enable_pin = sleep_pin;
    _config.sense_pin = sense_pin;   
    _config.board_voltage = 3.3;
    _config.amps_per_volt = 0.6;
    _config.current_trip = 5000;
    _config.current_trip_prog = 250;
    _config.prog_trip_time = 100;
    _config.main_preambles = 16;
    _config.prog_preambles = 22;
    _config.track_power_callback = nullptr; // Needs to be set in the main file

    // Board-specific configs
    _config.fault_pin = fault_pin;
    _config.limit_pin = limit_pin;

    return _config;
  }

  void setup();

  const char* getName();
  
  void power(bool, bool announce);
  void signal(bool);
  void cutout(bool);
  void progMode(bool);

  uint16_t getCurrentRaw();
  uint16_t getCurrentMilliamps();
  uint16_t getCurrentMilliamps(uint16_t reading);

  uint16_t setCurrentBase();
  uint16_t getCurrentBase();

  bool getStatus();

  void checkOverload();

  uint8_t getPreambles();
  
private:
  bool isCurrentLimiting();
};


#endif