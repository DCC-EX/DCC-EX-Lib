/*
 *  DCCMain.cpp
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

#include "DCCMain.h"

DCCMain::DCCMain(uint8_t numDevices, Hardware hardware, Railcom railcom) {
  this->hdw = hardware;
  this->railcom = railcom;
  this->numDevices = numDevices;
  
  // Purge the queue memory
  packetQueue.clear();

  // Allocate memory for the speed table and clear it
  speedTable = (Speed *)calloc(numDevices+1, sizeof(Speed));
  for (int i = 0; i < numDevices+1; i++)
  {
    speedTable[i].cab = 0;
    speedTable[i].forward = true;
    speedTable[i].speed = 0;
  }
}

void DCCMain::schedulePacket(const uint8_t buffer[], uint8_t byteCount, 
  uint8_t repeats, uint16_t identifier, PacketType type, uint16_t address) {
  
  Packet newPacket;

  uint8_t checksum=0;
  for (int b=0; b<byteCount; b++) {
    checksum ^= buffer[b];
    newPacket.payload[b] = buffer[b];
  }
  newPacket.payload[byteCount] = checksum;
  newPacket.length = byteCount+1;
  newPacket.repeats = repeats;
  newPacket.transmitID = identifier;
  newPacket.type = type;
  newPacket.address = address;

  const Packet pushPacket = newPacket;
  noInterrupts();
  packetQueue.push(pushPacket); // Push the packet into the queue for processing
  interrupts();  
}

void DCCMain::updateSpeed() {
  if (packetQueue.count() > 0) return;  // Only update speed if queue is empty

  for (; nextDev < numDevices; nextDev++) {
    if (speedTable[nextDev].cab > 0) {
      setThrottleResponse response;
      setThrottle(nextDev, speedTable[nextDev].cab, speedTable[nextDev].speed, 
        speedTable[nextDev].forward, response);
      nextDev++;
      return;
    }
  }
  for (nextDev = 0; nextDev < numDevices; nextDev++) {
    if (speedTable[nextDev].cab > 0) {
      setThrottleResponse response;
      setThrottle(nextDev, speedTable[nextDev].cab, speedTable[nextDev].speed, 
        speedTable[nextDev].forward, response);
      nextDev++;
      return;
    }
  }
}

uint8_t DCCMain::setThrottle(uint8_t slot, uint16_t addr, uint8_t speed, 
  uint8_t direction, setThrottleResponse& response) {
  
  uint8_t b[5];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  if((slot < 1) || (slot > numDevices))
    return ERR_OUT_OF_RANGE;

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  } 

  b[nB++]=lowByte(addr);
  railcomAddr |= lowByte(addr);
  b[nB++]=0x3F;   // 128-step speed control byte
  if(speed>=0)
    // max speed is 126, but speed codes range from 2-127 
    // (0=stop, 1=emergency stop)
    b[nB++]=speed+(speed>0)+direction*128;   
  else{
    b[nB++]=1;
    speed=0;
  }

  incrementCounterID();
  schedulePacket(b, nB, 0, counterID, kThrottleType, railcomAddr);

  speedTable[slot].speed = speed;
  speedTable[slot].cab = addr;
  speedTable[slot].forward = direction; 

  response.device = addr;
  response.direction = direction;
  response.speed = speed;
  response.transactionID = counterID;

  return ERR_OK;
}

uint8_t DCCMain::setFunction(uint16_t addr, uint8_t byte1, 
  genericResponse& response) {
  
  uint8_t b[4];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  }

  b[nB++] = lowByte(addr);
  railcomAddr |= lowByte(addr);

  b[nB++] = (byte1 | 0x80) & 0xBF;

  incrementCounterID();
  // Repeat the packet four times (one plus 3 repeats)
  schedulePacket(b, nB, 3, counterID, kFunctionType, railcomAddr);  

  response.transactionID = counterID;

  return ERR_OK;
}

uint8_t DCCMain::setFunction(uint16_t addr, uint8_t byte1, uint8_t byte2, 
  genericResponse& response) {
  
  uint8_t b[4];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  }

  b[nB++] = lowByte(addr);
  railcomAddr |= lowByte(addr);

  // for safety this guarantees that first byte will either be 0xDE 
  // (for F13-F20) or 0xDF (for F21-F28)
  b[nB++]=(byte1 | 0xDE) & 0xDF;     
  b[nB++]=byte2;
  
  incrementCounterID();
  // Repeat the packet four times (one plus 3 repeats)
  schedulePacket(b, nB, 3, counterID, kFunctionType, railcomAddr);  

  response.transactionID = counterID;

  return ERR_OK;
}

uint8_t DCCMain::setAccessory(uint16_t addr, uint8_t number, bool activate, 
  genericResponse& response) {
  
  uint8_t b[3];     // Packet payload. Save space for checksum byte
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least 
  // signifcant bits of accessory address
  b[0] = (addr % 64) + 128;           
  // second byte is of the form 1AAACDDD, where C should be 1, and the least 
  // significant D represent activate/deactivate                                
  b[1] = ((((addr / 64) % 8) << 4) + (number % 4 << 1) + activate % 2) ^ 0xF8;      
  railcomAddr = (b[0] << 8) | b[1];

  incrementCounterID();
  // Repeat the packet four times (one plus 3 repeats)
  schedulePacket(b, 2, 3, counterID, kAccessoryType, railcomAddr); 

  response.transactionID = counterID;

  return ERR_OK;
}

uint8_t DCCMain::writeCVByteMain(uint16_t addr, uint16_t cv, uint8_t bValue, 
  genericResponse& response, Print* stream, void (*POMCallback)(Print*, RailcomPOMResponse)) {
  
  uint8_t b[6];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  cv--;   // actual CV addresses are cv-1 (0-1023)

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  } 

  b[nB++] = lowByte(addr);
  railcomAddr |= lowByte(addr);

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  b[nB++] = 0xEC + (highByte(cv) & 0x03);   
  b[nB++] = lowByte(cv);
  b[nB++] = bValue;

  railcom.config_setPOMResponseCallback(stream, POMCallback);

  incrementCounterID();
  // Repeat the packet four times (one plus 3 repeats)
  schedulePacket(b, nB, 3, counterID, kPOMByteWriteType, railcomAddr); 

  response.transactionID = counterID;

  return ERR_OK;
}

uint8_t DCCMain::writeCVBitMain(uint16_t addr, uint16_t cv, uint8_t bNum, 
  uint8_t bValue, genericResponse& response, Print *stream, 
  void (*POMCallback)(Print*, RailcomPOMResponse)) {
  
  uint8_t b[6];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  cv--;   // actual CV addresses are cv-1 (0-1023)

  bValue = bValue % 2;
  bNum = bNum % 8;

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  } 

  b[nB++] = lowByte(addr);
  railcomAddr |= lowByte(addr);

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  b[nB++] = 0xE8 + (highByte(cv) & 0x03);   
  b[nB++] = lowByte(cv);
  b[nB++] = 0xF0 + (bValue * 8) + bNum;

  railcom.config_setPOMResponseCallback(stream, POMCallback);

  incrementCounterID();
  schedulePacket(b, nB, 4, counterID, kPOMBitWriteType, railcomAddr);

  response.transactionID = counterID;

  return ERR_OK;
}

uint8_t DCCMain::readCVByteMain(uint16_t addr, uint16_t cv, 
  genericResponse& response, Print *stream, void (*POMCallback)(Print*, RailcomPOMResponse)) {

  uint8_t b[6];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  cv--;   // actual CV addresses are cv-1 (0-1023)

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  } 

  b[nB++] = lowByte(addr);
  railcomAddr |= lowByte(addr);

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  b[nB++] = 0xE4 + (highByte(cv) & 0x03);   
  b[nB++] = lowByte(cv);
  b[nB++] = 0;  // For some reason the railcom spec leaves an empty byte  

  railcom.config_setPOMResponseCallback(stream, POMCallback);

  incrementCounterID();
  // Repeat the packet four times (one plus 3 repeats)
  schedulePacket(b, nB, 3, counterID, kPOMReadType, railcomAddr); 

  response.transactionID = counterID;

  return ERR_OK;

}

uint8_t DCCMain::readCVBytesMain(uint16_t addr, uint16_t cv, 
  genericResponse& response, Print *stream, void (*POMCallback)(Print*, RailcomPOMResponse)) {

  uint8_t b[5];     // Packet payload. Save space for checksum byte
  uint8_t nB = 0;   // Counter for number of bytes in the packet
  uint16_t railcomAddr = 0;  // For detecting the railcom instruction type

  cv--;   // actual CV addresses are cv-1 (0-1023)

  if(addr > 127) {
    b[nB++] = highByte(addr) | 0xC0;    // convert address to packet format
    railcomAddr = (highByte(addr) | 0xC0) << 8;
  } 

  b[nB++] = lowByte(addr);
  railcomAddr |= lowByte(addr);

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  b[nB++] = 0xE0 + (highByte(cv) & 0x03);   
  b[nB++] = lowByte(cv);  

  railcom.config_setPOMResponseCallback(stream, POMCallback);

  incrementCounterID();
  // Repeat the packet four times (one plus 3 repeats)
  schedulePacket(b, nB, 3, counterID, kPOMLongReadType, railcomAddr); 

  response.transactionID = counterID;

  return ERR_OK;

}