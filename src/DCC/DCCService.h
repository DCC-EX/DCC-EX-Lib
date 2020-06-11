/*
 *  DCCService.h
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

#ifndef COMMANDSTATION_DCC_DCCSERVICE_H_
#define COMMANDSTATION_DCC_DCCSERVICE_H_

#include <Arduino.h>

#include "Waveform.h"
#include "Queue.h"

const uint8_t kServiceQueueSize = 35;

// Threshold (mA) that a sample must cross to ACK
const uint8_t kACKThreshold = 30; 

enum cv_edit_type {
  READCV,
  WRITECV,
  WRITECVBIT,
};

struct serviceModeResponse {
  cv_edit_type type;
  uint16_t callback;
  uint16_t callbackSub;
  uint16_t cv;
  uint8_t cvBitNum;
  int cvValue;  // Might be -1, so int works
};

class DCCService : public Waveform {
public:
  DCCService(Hardware settings);

  static DCCService* Create_Arduino_L298Shield_Prog();
  static DCCService* Create_Pololu_MC33926Shield_Prog();
  static DCCService* Create_WSM_SAMCommandStation_Prog();

  void loop() {
    Waveform::loop(); // Checks for overcurrent and manages power
    checkAck();
  }

  int writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, 
    uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
  int writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, 
    uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
  int readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, 
    void(*callbackFunc)(serviceModeResponse));

private:
  struct Packet {
    uint8_t payload[kPacketMaxSize];
    uint8_t length;
    uint8_t repeats;
    uint16_t transmitID;  // Identifier for CV programming
  };

  // Queue of packets, FIFO, that controls what gets sent out next. Size 35.
  Queue<Packet, kServiceQueueSize> packetQueue;

  void schedulePacket(const uint8_t buffer[], uint8_t byteCount, 
    uint8_t repeats, uint16_t identifier);

  bool interrupt1();
  void interrupt2();

  // Checks service mode track for an ACK pulse, and handles state of ACK engine
  void checkAck();
  serviceModeResponse cvState;
  uint8_t ackBuffer; // Bits keeps track of what the ack values are.
  uint8_t ackNeeded = 0; // Bits denote where we still need an ack.
  uint16_t ackPacketID[8]; // Packet IDs that correspond to ACK opportunities
  uint8_t verifyPayload[4]; // Packet sent to confirm CV read
  uint8_t inVerify = false;   // (bool) Set when verifying read/write
  uint8_t backToIdle;  // (bool) Gone back to idle after setting CV instruction?
  // Callback function, returns response to comm API.
  void (*cvResponse)(serviceModeResponse);    
};

#endif