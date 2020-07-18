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

// invalid value (not conforming to the 4bit weighting requirement)
const uint8_t INV = 0xFF;
// Railcom ACK; the decoder received the message ok. NOTE: some early
// software versions may have ACK and NACK exchanged.
const uint8_t ACK = 0xFE;
// The decoder rejected the packet.
const uint8_t NACK = 0xFD;
// The decoder is busy; send the packet again. This is typically returned
// when a POM CV write is still pending; the caller must re-try sending the
// packet later.
const uint8_t BUSY = 0xFC;
// Reserved for future expansion.
const uint8_t RESVD1 = 0xFB;
const uint8_t RESVD2 = 0xFA;
const uint8_t RESVD3 = 0xF8;

enum RailcomInstructionType : uint8_t {
  kMOBInstruction,
  kSTATInstruction,
  kNoInstruction
};

enum RailcomMOBID : uint8_t {
  kMOB_POM = 0,
  kMOB_ADR_HIGH = 1,
  kMOB_ADR_LOW = 2,
  kMOB_EXT = 3,
  kMOB_DYN = 7,
  kMOB_SUBID = 12
};

enum RailcomSTATID : uint8_t {
  kSTAT_POM = 0,
  kSTAT_STAT1 = 4,
  kSTAT_TIME = 5,
  kSTAT_ERROR = 6,
  kSTAT_DYN = 7,
  kSTAT_STAT2 = 8,
  kSTAT_SUBID = 12
};

enum PacketType : uint8_t {
  kResetType,
  kIdleType,
  kThrottleType,
  kFunctionType,
  kAccessoryType,
  kPOMByteWriteType,  // Railcom is same as standard command for write byte
  kPOMBitWriteType,   // Railcom is same as standard command for write bit
  kPOMReadType,
  kPOMLongReadType,
  kSrvcByteWriteType,
  kSrvcBitWriteType,
  kSrvcReadType
};

struct RailcomDatagram {
  uint8_t identifier; // 4-bit ID, LSB justified
  uint8_t channel;  // Railcom channel the data came in on, either 1 or 2
  uint32_t payload; // LSB justified payload of the datagram, excluding the ID
};

struct RailcomPOMResponse {
  uint32_t data;
  uint16_t transactionID;
};

class Railcom
{
public:
  uint8_t enable;

  Railcom() {}
  void setup();

  void enableRecieve(uint8_t on);
  void readData(uint16_t dataID, PacketType _packetType, uint16_t _address);
  void processData();

  // Railcom config modification
  void config_setEnable(uint8_t isRailcom) { enable = isRailcom; }
  void config_setRxPin(uint8_t pin) { rx_pin = pin; }
  void config_setTxPin(uint8_t pin) { tx_pin = pin; }
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAMC)
  Uart* getSerial() { return serial; }
  void config_setSerial(Uart* serial) { this->serial = serial; }
  void config_setSercom(SERCOM* sercom) { this->sercom = sercom; }
  void config_setRxMux(EPioType mux) { rx_mux = mux; }
  void config_setRxPad(SercomRXPad pad) { rx_pad = pad; }
  void config_setTxPad(SercomUartTXPad pad) { tx_pad = pad; }
  void config_setDACValue(uint8_t value) { dac_value = value; }
#else
  HardwareSerial* getSerial() { return serial; }
  void config_setSerial(HardwareSerial* serial) { this->serial = serial; }
#endif
  void config_setPOMResponseCallback(void (*_POMResponse)(RailcomPOMResponse)) {
    POMResponse = _POMResponse;
  }

private:
  uint8_t rawData[8];
  uint16_t uniqueID;
  uint16_t address;
  PacketType type;
  bool dataReady = false;
  void (*POMResponse)(RailcomPOMResponse);

  // Railcom hardware declarations
  uint8_t rx_pin;
  uint8_t tx_pin;     
  static const long baud = 250000;
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAMC)
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
};

#endif  // COMMANDSTATION_DCC_RAILCOM_H_