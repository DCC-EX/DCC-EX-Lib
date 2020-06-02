#include "DCC.h"
#if defined(ARDUINO_ARCH_SAMD)
#include "wiring_private.h"
#endif

void DCC::interruptHandler() {
    if(interrupt1()) {
        interrupt2();
    }
}

bool DCC::interrupt1() {
    // NOTE: this must consume transmission buffers even if the power is off 
    // otherwise can cause hangs in main loop waiting for the pendingBuffer. 
    switch (state) {
    case 0:  // start of bit transmission
        hdw.setSignal(HIGH);    
        state = 1;
        return true; // must call interrupt2 to set currentBit
    case 1:
        if(generateRailcomCutout) {
            hdw.setBrake(true);    // Start the cutout
            inRailcomCutout = true;
            hdw.enableRailcomSerial(true);  // Start this a little early so it has time to start up
        }
        state = 2;
        break;
    case 2: // 58us after case 0
        if(currentBit && !generateRailcomCutout) {
            hdw.setSignal(LOW);  
        }
        state = 3;
        break; 
    case 3: // 87us after case 0
        if(currentBit && !generateRailcomCutout) {
            state = 0;
        }
        else state = 4;
        break;
    case 4:  // 116us after case 0
        if(!generateRailcomCutout) {
            hdw.setSignal(LOW);
        }
        state = 5;
        break;
    // Case 5 and 6 fall to default case, we simply increment the state
    case 7:
        if(!generateRailcomCutout) {
            state = 0;
        }
        else state = 8;
        break;
    // Cases 8-15 are for railcom timing, we increment the state
    case 16:
        hdw.setBrake(false);
        hdw.setSignal(LOW);
        hdw.enableRailcomSerial(false);
        railcomData = true;
        generateRailcomCutout = false;
        inRailcomCutout = false;
        lastID = transmitID;
        state = 0;
        break;
    default:
        state++;
        break;
    }

    return false;
}

void DCC::interrupt2() {
    // set currentBit to be the next bit to be sent.
  
    if (remainingPreambles > 0 ) {
        currentBit=true;
        if((hdw.getPreambles() - remainingPreambles == 1) && hdw.getRailcomEnable()) {
            generateRailcomCutout = true;
            remainingPreambles -= 4;
            return;
        }
        remainingPreambles--;
        return;
    }
  
    // beware OF 9-BIT MASK  generating a zero to start each byte   
    currentBit=transmitPacket[bytes_sent] & bitMask[bits_sent];
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
            remainingPreambles=hdw.getPreambles();

            int pendingCount = packetQueue.count();

            if (transmitRepeats > 0) {
                transmitRepeats--;
            }
            else if (pendingCount > 0) {
                // Copy pending packet to transmit packet
                Packet pendingPacket = packetQueue.pop();   // PlatformIO marks this as an error, but we can just ignore it. It compiles.

                for (int b=0;b<pendingPacket.length;b++) transmitPacket[b] = pendingPacket.payload[b];
                transmitLength=pendingPacket.length;
                transmitRepeats=pendingPacket.repeats;
                transmitID=pendingPacket.transmitID;
            }
            else {
                // Fortunately reset and idle packets are the same length
                memcpy( transmitPacket, hdw.getIsProgTrack()?resetPacket:idlePacket, sizeof(idlePacket));
                transmitLength=sizeof(idlePacket);
                transmitRepeats=0;
            }
        }
    }
}
