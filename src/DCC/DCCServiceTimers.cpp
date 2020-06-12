/*
 *  DCCServiceTimers.cpp
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

bool DCCService::interrupt1() {
  switch (interruptState) {
  case 0:   // start of bit transmission
    hdw.setSignal(HIGH);    
    interruptState = 1; 
    return true; // must call interrupt2 to set currentBit
  // Case 1 falls to default case
  case 2:   // 58us after case 0
    if(currentBit) {
      hdw.setSignal(LOW);  
    }
    interruptState = 3;
    break; 
  case 3:   // 87us after case 0
    if(currentBit) {
      // Return to case zero for start of another bit at 116us
      interruptState = 0;
    }
    else interruptState = 4;
    break;
  case 4:   // 116us after case 0
    hdw.setSignal(LOW);
    interruptState = 5;
    break;
  // Case 5 and 6 fall to default case
  case 7:   // 203us after case 0
    interruptState = 0; 
    break;
  default:
    interruptState++;
    break;
  }

  return false;   // Don't call interrupt2
}

void DCCService::interrupt2() {
  if (remainingPreambles > 0 ) {    // If there's more preambles to be sent
    currentBit=true;                // Send a one bit (preambles are one)
    remainingPreambles--;   // decrement the number of preambles to send
    return;
  }
  
  // beware OF 9-BIT MASK  generating a zero to start each byte   
  currentBit=transmitPacket[bytes_sent] & kBitMask[bits_sent];
  bits_sent++;

  // If this is the last bit of a byte, prepare for the next byte 
  if (bits_sent==9) { // zero followed by 8 bits of a byte
    //end of Byte
    bits_sent=0;
    bytes_sent++;
    // if this is the last byte, prepare for next packet
    if (bytes_sent >= transmitLength) { 
      // end of transmission buffer... repeat or switch to next message
      bytes_sent = 0;
      remainingPreambles = hdw.getPreambles() + 1;  // Add one for the stop bit

      int pendingCount = packetQueue.count();

      // Note that the number of repeats does not include the final repeat, so
      // the number of times transmitted is nRepeats+1
      if (transmitRepeats > 0) {
        transmitRepeats--;
      }
      else if (pendingCount > 0) {
        // Copy pending packet to transmit packet
        Packet pendingPacket = packetQueue.pop();

        // Load info about the packet into the transmit variables.
        // TODO(davidcutting42@gmail.com): check if this can be done with a 
        // peek() into packetQueue intead.
        for (int b=0;b<pendingPacket.length;b++) 
          transmitPacket[b] = pendingPacket.payload[b];
        transmitLength=pendingPacket.length;
        transmitRepeats=pendingPacket.repeats;
        transmitID=pendingPacket.transmitID;
      }
      else {
        // Load an idle packet or a reset packet
        // Fortunately reset and idle packets are the same length
        memcpy( transmitPacket, hdw.getIsProgTrack()?kResetPacket:kIdlePacket, 
          sizeof(kIdlePacket));
        transmitLength=sizeof(kIdlePacket);
        transmitRepeats=0;
        backToIdle = true;
      }
    }
  }
}
