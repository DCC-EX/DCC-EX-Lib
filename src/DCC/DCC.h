/*
 *  DCC.h
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

#ifndef COMMANDSTATION_DCC_DCC_H_
#define COMMANDSTATION_DCC_DCC_H_

#include <Arduino.h>

#include "Hardware.h"
#include "Queue.h"

enum {
  ERR_OK = 1,
  ERR_OUT_OF_RANGE = 2,
  ERR_BUSY = 3,
};

const uint8_t kPacketMaxSize = 6; 

// Threshold (mA) that a sample must cross to ACK
const uint8_t kACKThreshold = 30; 

// TODO(davidcutting42@gmail.com): separate between programming and main track
const uint8_t kQueueSize = 50; 

const uint8_t kIdlePacket[] = {0xFF,0x00,0xFF};
const uint8_t kResetPacket[] = {0x00,0x00,0x00};
const uint8_t kBitMask[] = {0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

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
  uint8_t cvValue;
};

// Represents a DCC channel. 
// After constructor is called, call interruptHandler() every 29us on a timer to
// generate signal and call loop() as often as possible to handle other stuff.
class DCC {
public:
  DCC(uint8_t numDevices, Hardware settings);

  // Static factory constructors for Arduino Motor Shield V3.
  static DCC* Create_Arduino_L298Shield_Main(uint8_t numDev);
  static DCC* Create_Arduino_L298Shield_Prog(uint8_t numDev);
  
  // Static factory constructors for Pololu MC33926 motor shield.
  static DCC* Create_Pololu_MC33926Shield_Main(uint8_t numDev);
  static DCC* Create_Pololu_MC33926Shield_Prog(uint8_t numDev);

  // Static factory constructors for Wasatch Scale Models SAM Command Station.
  static DCC* Create_WSM_SAMCommandStation_Main(uint8_t numDev);
  static DCC* Create_WSM_SAMCommandStation_Prog(uint8_t numDev);

  // Call this function every 29us from the main code
  void interruptHandler();

  // Call this function every loop
  void loop() {
    updateSpeed();
    hdw.checkCurrent();
    if(hdw.getRailcomEnable()) {
      readRailcomData();
    }
    if(hdw.getIsProgTrack()) {
      checkAck();
    }
  }

  int setThrottle(uint8_t nDev, uint16_t cab, uint8_t tSpeed, bool tDirection, 
    setThrottleResponse& response);
  int setFunction(uint16_t cab, uint8_t byte1, setFunctionResponse& response);
  int setFunction(uint16_t cab, uint8_t byte1, uint8_t byte2, 
    setFunctionResponse& response);
  int setAccessory(uint16_t address, uint8_t number, bool activate, 
    setAccessoryResponse& response);
  int writeCVByteMain(uint16_t cab, uint16_t cv, uint8_t bValue, 
    writeCVByteMainResponse& response);
  int writeCVBitMain(uint16_t cab, uint16_t cv, uint8_t bNum, uint8_t bValue, 
    writeCVBitMainResponse& response);
  int writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, 
    uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
  int writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, uint16_t callback, 
    uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse));
  int readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, 
    void(*callbackFunc)(serviceModeResponse));

  Hardware hdw;


  // TODO(davidcutting42@gmail.com): determine if 255 devices is enough.
  uint8_t numDevices;

  // Holds info about a device's speed and direction. 
  struct Speed {
    uint16_t cab;
    uint8_t speed;
    uint8_t forward;
  };
  // Speed table holds speed of all devices on the bus that have been set since
  // startup. 
  // TODO(davidcutting42@gmail.com): turn into compile-time array
  Speed* speedTable;

private:
  // Queues a packet for the next device in line reminding it of its speed.
  void updateSpeed();
  // Holds state for updateSpeed function.
  uint8_t nextDev = 0;

  // Holds info about a packet to be sent on the bus.
  struct Packet {
    uint8_t payload[kPacketMaxSize];
    uint8_t length;
    uint8_t repeats;
    uint16_t transmitID;  // Identifier for railcom, CV programming, etc.
  };
  // Queue of packets, FIFO, that controls what gets sent out next
  Queue<Packet> packetQueue = Queue<Packet>(kQueueSize);
  // Data that controls the packet currently being sent out.
  uint8_t bits_sent;  // Bits sent from byte
  uint8_t bytes_sent; // Bytes sent from packet
  uint8_t currentBit = false;
  uint8_t transmitRepeats = 0;  // Repeats (does not include initial transmit)
  uint8_t remainingPreambles = 0; 
  uint8_t generateStartBit = false;  // Send a start bit for the current byte?
  uint8_t transmitPacket[kPacketMaxSize];
  uint8_t transmitLength;
  uint16_t transmitID;
  
  // Interrupt segments, called in interrupt_handler
  bool interrupt1();
  void interrupt2();
  void signal(bool pinA, bool pinB);
  uint8_t state = 0;  // Waveform generator state

  // Loads buffer into queue for transmission
  void schedulePacket(const uint8_t buffer[], uint8_t byteCount, 
    uint8_t repeats, uint16_t identifier);

  uint16_t counterID = 1; // Maintains the last assigned packet ID
  uint16_t lastID = 1; // ID of the last DCC packet to get processed (railcom)
  inline void incrementCounterID() { 
    counterID++;
    if(counterID == 0) counterID = 1;
  }

  // Railcom cutout stuff
  // Reads and processes railcom data from the railcom serial port
  void readRailcomData();
  volatile bool generateRailcomCutout = false; // Should we do a railcom cutout?
  volatile bool inRailcomCutout = false;    // Are we in a cutout?
  volatile bool railcomData = false;    // Is there railcom data available?  

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

#endif  // COMMANDSTATION_DCC_DCC_H_