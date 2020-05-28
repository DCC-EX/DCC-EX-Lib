#include "DCC.h"

// Library DIO2.h is only compatible with AVR, and SAM digitalWrite is a lot faster.
#if defined(ARDUINO_ARCH_AVR)
#include <DIO2.h>
#endif

void DCC::signal(bool pinA, bool pinB) volatile {
    #if defined(ARDUINO_ARCH_AVR)
    digitalWrite2(hdw.signal_a_pin, pinA);
    if(hdw.control_scheme == DUAL_DIRECTION_INVERTED)
        digitalWrite2(hdw.signal_b_pin, pinB);
    #else
    digitalWrite(hdw.signal_a_pin, pinA);
    if(hdw.control_scheme == DUAL_DIRECTION_INVERTED)
        digitalWrite(hdw.signal_b_pin, pinB);
    #endif
}

void DCC::interruptHandler() {
    if(interrupt1()) {
        interrupt2();
    }
}

bool DCC::interrupt1() volatile {
    // NOTE: this must consume transmission buffers even if the power is off 
    // otherwise can cause hangs in main loop waiting for the pendingBuffer. 
    switch (state) {
    case 0:  // start of bit transmission
        signal(HIGH, LOW);    
        state = 1;
        return true; // must call interrupt2 to set currentBit
    // Case 1 falls to default case, we simply increment the state
    case 2: // 58us after case 0
        if(currentBit) {
            signal(LOW, HIGH);
        }
        state = 3;
        break; 
    case 3: // 87us after case 0
        if(currentBit && !inRailcomCutout) {
            state = 0;
        }
        else state = 4;
        if(inRailcomCutout) {
            signal(HIGH, HIGH);     // Start the cutout
        }
        break;
    case 4:  // 116us after case 0
        if(!inRailcomCutout) {
            signal(LOW,HIGH);
        }
        state = 5;
        break;
    // Case 5 and 6 fall to default case, we simply increment the state
    case 7:
        if(!inRailcomCutout) {
            state = 0;
        }
        else state = 8;
        break;
    // Cases 8-16 are for railcom timing, we increment the state
    case 17:
        state = 0;
        inRailcomCutout = false;
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
            remainingPreambles=hdw.preambleBits;
            if(hdw.enable_railcom == true) {
                inRailcomCutout = true;
                remainingPreambles -= 4;
            }

            if (transmitRepeats > 0) {
                transmitRepeats--;
            }
            else if (packetPending) {
                // Copy pending packet to transmit packet
                for (int b=0;b<pendingLength;b++) transmitPacket[b] = pendingPacket[b];
                transmitLength=pendingLength;
                transmitRepeats=pendingRepeats;
                packetPending=false;
            }
            else {
                // Fortunately reset and idle packets are the same length
                memcpy( transmitPacket, hdw.is_prog_track?resetPacket:idlePacket, sizeof(idlePacket));
                transmitLength=sizeof(idlePacket);
                transmitRepeats=0;
            }
        }
    }
}
