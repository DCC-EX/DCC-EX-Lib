/*
 *  DCCMain.h
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

#ifndef COMMANDSTATION_DCC_DCCMAIN_H_
#define COMMANDSTATION_DCC_DCCMAIN_H_

#include <Arduino.h>

#include "Waveform.h"
#include "Railcom.h"
#include "Queue.h"

struct setThrottleResponse {
  uint8_t device;
  uint8_t speed;
  uint8_t direction;
  uint16_t transactionID;
};

struct setFunctionResponse {
  uint16_t transactionID;
};

struct setAccessoryResponse {
  uint16_t transactionID;
};

struct writeCVByteMainResponse {
  uint16_t transactionID;
};

struct writeCVBitMainResponse {
  uint16_t transactionID;
};

class DCCMain : public Waveform {
public:
  DCCMain(uint8_t numDevices, Hardware hardware, Railcom railcom);

  static DCCMain* Create_Arduino_L298Shield_Main(uint8_t numDevices);
  static DCCMain* Create_Pololu_MC33926Shield_Main(uint8_t numDevices);
  static DCCMain* Create_WSM_SAMCommandStation_Main(uint8_t numDevices);

  void loop() {
    Waveform::loop();
    updateSpeed();
    railcom.processData();
  }

  int setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, 
    setThrottleResponse& response);
  int setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response);
  int setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, 
    setFunctionResponse& response);
  int setAccessory(uint16_t address, uint8_t number, bool activate, 
    setAccessoryResponse& response);
  int writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, 
    writeCVByteMainResponse& response, void (*POMCallback)(RailcomPOMResponse));
  int writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, 
    writeCVBitMainResponse& response, void (*POMCallback)(RailcomPOMResponse));

  uint8_t numDevices;

  // Holds info about a device's speed and direction. 
  // TODO(davidcutting42@gmail.com): Make this private
  struct Speed {
    uint16_t cab;
    // TODO(davidcutting42@gmail.com): Merge these 2 variables into 1 uint8_t
    uint8_t speed;
    uint8_t forward;
  };
  // Speed table holds speed of all devices on the bus that have been set since
  // startup. 
  Speed* speedTable;

  // Railcom object, complements hdw object inherited from Waveform
  Railcom railcom;

private:
  

  // Queues a packet for the next device in line reminding it of its speed.
  void updateSpeed();
  // Holds state for updateSpeed function.
  uint8_t nextDev = 0;

  struct Packet {
    uint8_t payload[kPacketMaxSize];
    uint8_t length;
    uint8_t repeats;
    uint16_t transmitID;  // Identifier for railcom, etc.
    PacketType type;
    uint16_t address;
  };

  PacketType transmitType = kIdleType;
  uint16_t transmitAddress = 0;

  // Queue of packets, FIFO, that controls what gets sent out next. Size 5.
  Queue<Packet, 5> packetQueue;

  void schedulePacket(const uint8_t buffer[], uint8_t byteCount, 
    uint8_t repeats, uint16_t identifier, PacketType type, uint16_t address);

  bool interrupt1();
  void interrupt2();

  // Railcom cutout variables
  // TODO(davidcutting42@gmail.com): Move these to the railcom class
  bool generateRailcomCutout = false; // Should we do a railcom cutout?
  bool inRailcomCutout = false;    // Are we in a cutout?
  bool railcomData = false;    // Is there railcom data available? 
};

#endif