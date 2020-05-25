#include "DCC.h"
#include <ArduinoTimers.h>

bool DCC::interrupt1() {
  // NOTE: this must consume transmission buffers even if the power is off 
  // otherwise can cause hangs in main loop waiting for the pendingBuffer. 
    switch (state) {
    case 0:  // start of bit transmission
        digitalWrite(hdw.signal_a_pin, HIGH);
        if(hdw.control_scheme == DUAL_DIRECTION_INVERTED)
            digitalWrite(hdw.signal_b_pin, LOW);
        state = 1;
        return true; // must call interrupt2 to set timing of bits
    case 1:  // 58us after case 0 if the bit is a zero, 100us after case 1 if the bit is a one.
        digitalWrite(hdw.signal_a_pin, LOW);
        if(hdw.control_scheme == DUAL_DIRECTION_INVERTED)
            digitalWrite(hdw.signal_b_pin, HIGH);
        state = 0;
        break;
    }
    return false;
}

void DCC::interrupt2() {
    // set currentBit to be the next bit to be sent.
  
    if (remainingPreambles > 0 ) {
        currentBit=true;
        remainingPreambles--;
        int_timer.setPeriod(58);
        return;
    }
  
    // beware OF 9-BIT MASK  generating a zero to start each byte   
    currentBit=transmitPacket[bytes_sent] & bitMask[bits_sent];
    bits_sent++;

    if(currentBit) {
        int_timer.setPeriod(58);
    } else {
        int_timer.setPeriod(100);
    }

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
