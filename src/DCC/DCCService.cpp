/*
 *  DCCService.cpp
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

#include "DCCService.h"

DCCService::DCCService(Hardware settings) {
  this->hdw = settings; 
}

void DCCService::schedulePacket(const uint8_t buffer[], uint8_t byteCount, 
  uint8_t repeats, uint16_t identifier) {
  if(byteCount >= kPacketMaxSize) return; // allow for checksum
  
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

  const Packet pushPacket = newPacket;
  noInterrupts();
  packetQueue.push(pushPacket);
  interrupts();   
}

int DCCService::writeCVByte(uint16_t cv, uint8_t bValue, uint16_t callback, 
  uint16_t callbackSub, void(*callbackFunc)(serviceModeResponse)) {
  
  // If we're in the middle of a read/write or if there's not room in the queue.
  if(ackNeeded != 0 || inVerify 
    || (packetQueue.count() > (kServiceQueueSize - 8))) 
    return ERR_BUSY;
  
  uint8_t bWrite[4];

  hdw.setBaseCurrent();

  cv--;       // actual CV addresses are cv-1 (0-1023)

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  bWrite[0]=0x7C+(highByte(cv)&0x03);                     
  bWrite[1]=lowByte(cv);
  bWrite[2]=bValue;

  incrementCounterID();
  // NMRA recommends starting with 3 reset packets (one plus two repeats)
  schedulePacket(kResetPacket, 2, 2, counterID);           
  // NMRA recommends 5 verify packets (one plue 4 repeats)
  schedulePacket(bWrite, 3, 4, counterID);          
  // NMRA recommends 6 write or reset packets for decoder recovery time 
  // (one plus 5 repeats)      
  schedulePacket(bWrite, 3, 5, counterID);                
  
  incrementCounterID();
   // set-up to re-verify entire byte
  bWrite[0]=0x74+(highByte(cv)&0x03);                
  // NMRA recommends starting with 3 reset packets (one plus two repeats)    
  schedulePacket(kResetPacket, 2, 2, counterID);        
  // We send 2 packets before looking for an ack (one plus one repeat)   
  schedulePacket(bWrite, 3, 1, counterID);                

  incrementCounterID();
  // NMRA recommends 5 verify packets - we sent 2, here are three more 
  // (one plus two repeats)
  schedulePacket(bWrite, 3, 2, counterID);           
  // NMRA recommends 6 write or reset packets for decoder recovery time 
  // (one plus five repeats)     
  schedulePacket(bWrite, 3, 5, counterID);                
  
  ackPacketID[0] = counterID;
  
  incrementCounterID();
  // Final reset packet (and decoder begins to respond) (one plus no repeats)
  schedulePacket(kResetPacket, 2, 0, counterID);           

  inVerify = true;

  cvState.type = WRITECV;
  cvState.callback = callback;
  cvState.callbackSub = callbackSub;
  cvState.cv = cv+1;
  cvState.cvValue = bValue;

  backToIdle = false;

  cvResponse = callbackFunc;

  return ERR_OK;
}


int DCCService::writeCVBit(uint16_t cv, uint8_t bNum, uint8_t bValue, 
  uint16_t callback, uint16_t callbackSub, 
  void(*callbackFunc)(serviceModeResponse)) {
  
  // If we're in the middle of a read/write or if there's not room in the queue.
  if(ackNeeded != 0 || inVerify 
    || (packetQueue.count() > (kServiceQueueSize - 8))) 
    return ERR_BUSY;
  
  byte bWrite[4];

  hdw.setBaseCurrent();

  cv--;         // actual CV addresses are cv-1 (0-1023)
  bValue=bValue%2;
  bNum=bNum%8;

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  bWrite[0]=0x78+(highByte(cv)&0x03);   
  bWrite[1]=lowByte(cv);
  bWrite[2]=0xF0+bValue*8+bNum;

  incrementCounterID();
  // NMRA recommends starting with 3 reset packets (one plus two repeats)
  schedulePacket(kResetPacket, 2, 2, counterID);          
  // NMRA recommends 5 verify packets (one plue 4 repeats) 
  schedulePacket(bWrite, 3, 4, counterID);                
  // NMRA recommends 6 write or reset packets for decoder recovery time 
  // (one plus 5 repeats)
  schedulePacket(bWrite, 3, 5, counterID);                
  
  incrementCounterID();
  bitClear(bWrite[2],4);  // change instruction code from Write to Verify
  // NMRA recommends starting with 3 reset packets (one plus two repeats)
  schedulePacket(kResetPacket, 2, 2, counterID); 
  // We send 2 packets before looking for an ack (one plus one repeat)
  schedulePacket(bWrite, 3, 1, counterID);                

  incrementCounterID();
  // NMRA recommends 5 verify packets - we already sent 2, here are three more 
  // (one plus two repeats)
  schedulePacket(bWrite, 3, 2, counterID);              
  // NMRA recommends 6 write or reset packets for decoder recovery time 
  // (one plus five repeats)  
  schedulePacket(bWrite, 3, 5, counterID);                
  
  ackPacketID[0] = counterID;
  
  incrementCounterID();
  // Final reset packet (and decoder begins to respond) (one plus no repeats)
  schedulePacket(kResetPacket, 2, 0, counterID);           

  inVerify = true;

  cvState.type = WRITECVBIT;
  cvState.callback = callback;
  cvState.callbackSub = callbackSub;
  cvState.cv = cv+1;
  cvState.cvBitNum = bNum;
  cvState.cvValue = bValue;

  backToIdle = false;

  cvResponse = callbackFunc;

  return ERR_OK;
}


int DCCService::readCV(uint16_t cv, uint16_t callback, uint16_t callbackSub, 
  void(*callbackFunc)(serviceModeResponse)) {
  
  // If we're in the middle of a read/write or if there's not room in the queue.
  if(ackNeeded != 0 || inVerify 
    || (packetQueue.count() > (kServiceQueueSize - 25))) 
    return ERR_BUSY;
  
  uint8_t bRead[4];

  hdw.setBaseCurrent();

  cv--;    // actual CV addresses are cv-1 (0-1023)

  // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
  bRead[0]=0x78+(highByte(cv)&0x03);            
  bRead[1]=lowByte(cv);
  
  // Queue up all unique packets required for the CV read. 
  for(int i=0;i<8;i++) {                                  
    bRead[2]=0xE8+i;

    incrementCounterID();
    // NMRA recommends starting with 3 reset packets (one plue two repeats)
    schedulePacket(kResetPacket, 2, 2, counterID);       
    // We send 2 packets before looking for an ack (one plus one repeat)
    schedulePacket(bRead, 3, 1, counterID);             

    incrementCounterID();
    // NMRA recommends 5 verify packets - we already sent 2, here are three more 
    // (one plus two repeats)
    schedulePacket(bRead, 3, 2, counterID);             

    ackPacketID[i] = counterID;

    incrementCounterID();
    // NMRA recommends 1 reset packet after checking for an ACK 
    // (one and no repeats)
    schedulePacket(kResetPacket, 2, 0, counterID);       
  }

  ackNeeded = 0b11111111;
  
  cvState.type = READCV;
  cvState.callback = callback;
  cvState.callbackSub = callbackSub;
  cvState.cv = cv+1;

  verifyPayload[0]=0x74+(highByte(cv)&0x03);  // set-up to re-verify entire byte
  verifyPayload[1]=lowByte(cv);
  // verifyPayload[2] and verifyPayload[3] get set in checkAck when verifying 

  backToIdle = false;

  cvResponse = callbackFunc;

  return ERR_OK;
}

void DCCService::checkAck() {
  float currentMilliamps = hdw.getMilliamps();
  if(!inVerify && (ackNeeded == 0)) return;

  if(!inVerify) {
    uint16_t currentAckID;
    for (uint8_t i = 0; i < 8; i++)
    {
      if(!bitRead(ackNeeded, i)) continue;  // We don't need an ack on this bit

      currentAckID = ackPacketID[i];
      uint16_t compareID = transmitID;
      if(currentAckID == compareID) {
        if((currentMilliamps - hdw.getBaseCurrent()) > kACKThreshold) {
          bitSet(ackBuffer, i);       // We got an ack on this bit
          bitClear(ackNeeded, i);     // We no longer need an ack on this bit

          // Fast-forward to the next packet set
          noInterrupts();
          transmitRepeats = 0;    // Stop transmitting current packet
          while(packetQueue.peek().transmitID == currentAckID) {
            packetQueue.pop();  // Pop off all packets with the the same ID
          }
          interrupts();
        }
      }
      // TODO(davidcutting42@gmail.com): check for wraparound
      else if(compareID > currentAckID || backToIdle) {    
        bitClear(ackBuffer, i);  // We didn't get an ack on this bit (timeout)
        bitClear(ackNeeded, i);  // We no longer need an ack on this bit
      }

      if(ackNeeded == 0) {        // If we've now gotten all the ACKs we need 
        if(cvState.type == READCV) {
          verifyPayload[2] = ackBuffer;   // Set up the verifyPayload for verify
        
          incrementCounterID();               
          // Load 3 reset packets
          schedulePacket(kResetPacket, 2, 2, counterID); 
          // Load 5 verify packets
          schedulePacket(verifyPayload, 3, 4, counterID);     
          // Load 1 Reset packet
          schedulePacket(kResetPacket, 2, 0, counterID);       

          ackPacketID[0] = counterID;

          incrementCounterID();
          // We need one additional packet with incremented counter so ACK 
          // completes and doesn't hang in checkAck()
          schedulePacket(kResetPacket, 2, 0, counterID);   

          inVerify = true;
          backToIdle = false;
        }
        break;
      }
    }    
  }
  else {
    uint16_t compareID = transmitID;
    if(ackPacketID[0] == compareID) {
      if((currentMilliamps - hdw.getBaseCurrent()) > kACKThreshold) {
        inVerify = false;
        if(cvState.type == READCV)
          cvState.cvValue = ackBuffer;
        cvResponse(cvState);

        // Fast-forward to the next packet set
        noInterrupts();
        transmitRepeats = 0;  // Stop transmitting current packet
        while(packetQueue.peek().transmitID == ackPacketID[0]) {
          packetQueue.pop();  // Pop off all packets with the the same ID
        }
        interrupts();
      }
    }
    // TODO(davidcutting42@gmail.com): check for wraparound
    else if(compareID > ackPacketID[0] || backToIdle) {
      inVerify = false;
      
      cvState.cvValue = -1;
      cvResponse(cvState);
    }
  }
}