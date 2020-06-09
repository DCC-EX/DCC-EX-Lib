/*
 *  Railcom.h
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

#ifndef COMMANDSTATION_DCC_RAILCOM_H_
#define COMMANDSTATION_DCC_RAILCOM_H_

#include <Arduino.h>

#include "Queue.h"

#if defined(ARDUINO_ARCH_AVR)
  #include <HardwareSerial.h>
#endif

extern const uint8_t railcom_decode[256];

/// invalid value (not conforming to the 4bit weighting requirement)
const uint8_t INV = 0xFF;
/// Railcom ACK; the decoder received the message ok. NOTE: some early
/// software versions may have ACK and NACK exchanged.
const uint8_t ACK = 0xFE;
/// The decoder rejected the packet.
const uint8_t NACK = 0xFD;
/// The decoder is busy; send the packet again. This is typically returned
/// when a POM CV write is still pending; the caller must re-try sending the
/// packet later.
const uint8_t BUSY = 0xFC;
/// Reserved for future expansion.
const uint8_t RESVD1 = 0xFB;
/// Reserved for future expansion.
const uint8_t RESVD2 = 0xFA;
/// Reserved for future expansion.
const uint8_t RESVD3 = 0xF8;


enum railcom_type : uint8_t {
  MOB,
  STAT,
  NONE
};

enum railcom_mob_id : uint8_t {
  MOB_POM = 0,
  MOB_ADR_HIGH = 1,
  MOB_ADR_LOW = 2,
  MOB_EXT = 3,
  MOB_DYN = 7,
  MOB_SUBID = 12
};

enum railcom_stat_id : uint8_t {
  STAT_POM = 0,
  STAT_STAT1 = 4,
  STAT_TIME = 5,
  STAT_ERROR = 6,
  STAT_DYN = 7,
  STAT_STAT2 = 8,
  STAT_SUBID = 12
};

struct Datagram {
  uint8_t type;
  uint8_t data[6];  // We store six bits per byte for a max of 36 bits

};

class Railcom
{
public:
  uint8_t enable;

  Railcom() {}
  void setup();

  void enableRecieve(uint8_t on);
  void readData(uint16_t dataID);
  void processData();

  // Railcom config modification
  void config_setEnable(uint8_t isRailcom) { enable = isRailcom; }
  void config_setRxPin(uint8_t pin) { rx_pin = pin; }
  void config_setTxPin(uint8_t pin) { tx_pin = pin; }
#if defined(ARDUINO_ARCH_SAMD) 
  Uart* getSerial() { return serial; }
  void config_setSerial(Uart* serial) { this->serial = serial; }
  void config_setSercom(SERCOM* sercom) { this->sercom = sercom; }
  void config_setRxMux(EPioType mux) { rx_mux = mux; }
  void config_setRxPad(SercomRXPad pad) { rx_pad = pad; }
  void config_setTxPad(SercomUartTXPad pad) { tx_pad = pad; }
  void config_setDACValue(uint8_t value) { dac_value = value; }
#else
  HardwareSerial* getSerial() { return serial; }
  void config_setSerial(HardwareSerial* serial) { railcom_serial = serial; }
#endif

private:
  struct RailcomFeedback {
    uint8_t data[8]; 
    uint16_t dataID; 
    railcom_type type;   // MOB or STAT or NONE
  };

  Queue<RailcomFeedback, 3> processBuffer;

  uint8_t rx_pin;
  uint8_t tx_pin;     
  static const long baud = 250000;
#if defined(ARDUINO_ARCH_SAMD) 
  Uart* serial = nullptr;
  SERCOM* sercom;
  EPioType rx_mux;
  SercomRXPad rx_pad;
  SercomUartTXPad tx_pad;
  uint8_t dac_value;      // Sets the DAC according to the calculation 
                          // in the datasheet for a 1V reference
  void setupDAC();       // Enable DAC for LM393 reference
#else
  HardwareSerial* serial;
#endif

  enum : uint8_t
  {
    GARBAGE,
    ACK,
    NACK,
    BUSY,
    MOB_POM,
    MOB_ADRHIGH,
    MOB_ADRLOW,
    MOB_EXT,
    MOB_DYN,
    MOB_SUBID
  };
};

#endif  // COMMANDSTATION_DCC_RAILCOM_H_