/*
 *  BoardWSMFireBoxMK1T.h
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

#ifndef COMMANDSTATION_BOARDS_BOARDARDUINOMOTORSHIELD_H_
#define COMMANDSTATION_BOARDS_BOARDARDUINOMOTORSHIELD_H_

#include "Board.h"
#include "../HBridges/FireBit.h"

struct BoardConfigWSMFireBoxMK1T : public BoardConfig {
  bool wifi_enabled;
  int wifi_boot_pin;
  int wifi_reset_pin;
  Uart wifi_serial_instance;
};

class BoardWSMFireBoxMK1T : public Board
{
public:
  BoardConfigWSMFireBoxMK1T config;

  BoardWSMFireBoxMK1T(BoardConfigWSMFireBoxMK1T _config) {
    config = _config;

    mainTrack = new HBridgeFireBit(HBridgeFireBit::getDefaultConfig(
      "MAIN", PIN_MAIN_CTRL_A, PIN_MAIN_CTRL_B, PIN_MAIN_
    ))
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

  HBridgeFireBit* mainTrack;
  HBridgeFireBit* progTrack;
  
private:
  bool isCurrentLimiting();
};


#endif