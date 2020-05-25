#include "../DCC.h"

#if defined(ATSAMD21G)
#include "ATSAMD21G/Hardware.h"



void DCC::init_timers() {
    pinMode(hdw.signal_a_pin, OUTPUT);
    if(hdw.use_dual_signal) {
        pinMode(hdw.signal_b_pin, OUTPUT);
    }

    hdw.timer_callback = interrupt_handler;

    REG_GCLK_GENDIV =   GCLK_GENDIV_DIV(1) |          // Divide 48MHz by 1
                        GCLK_GENDIV_ID(hdw.gclk_num);            // Apply to GCLK4
    while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization
        
    REG_GCLK_GENCTRL =  GCLK_GENCTRL_GENEN |          // Enable GCLK
                        GCLK_GENCTRL_SRC_DFLL48M |    // Set the 48MHz clock source
                        GCLK_GENCTRL_ID(hdw.gclk_num);           // Select GCLK4
    while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization
    
    if(hdw.timer == TCC0) {
        REG_GCLK_CLKCTRL =  GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                            hdw.gclk_num << GCLK_CLKCTRL_GEN_Pos |      // Apply to GCLK4
                            GCLK_CLKCTRL_ID_TCC0_TCC1;    // Feed GCLK to TCC0/1
        while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

        NVIC_EnableIRQ((IRQn_Type) TCC0_IRQn);
    }
    else if(hdw.timer == TCC1) {
        REG_GCLK_CLKCTRL =  GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                            hdw.gclk_num << GCLK_CLKCTRL_GEN_Pos |      // Apply to GCLK4
                            GCLK_CLKCTRL_ID_TCC0_TCC1;    // Feed GCLK to TCC0/1
        while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

        NVIC_EnableIRQ((IRQn_Type) TCC1_IRQn);
    }
    else if(hdw.timer == TCC2) {
        REG_GCLK_CLKCTRL =  GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                            hdw.gclk_num << GCLK_CLKCTRL_GEN_Pos |      // Apply to GCLK4
                            GCLK_CLKCTRL_ID_TCC2_TC3;    // Feed GCLK to TCC2
        while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

        NVIC_EnableIRQ((IRQn_Type) TCC2_IRQn);
    }

    hdw.timer->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV4_Val);
    
    hdw.timer->PER.reg = DCC_ONE_BIT_PULSE_DURATION;
    while (hdw.timer->SYNCBUSY.bit.PER);

    hdw.timer->INTENSET.reg = TCC_INTENSET_OVF;

    hdw.timer->CTRLA.reg |= (TCC_CTRLA_ENABLE);          // Turn on the output
    while (hdw.timer->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
}

void DCC::interrupt_handler() volatile {
    if (hdw.timer->INTFLAG.bit.OVF)             
    { 
        hdw.timer->INTFLAG.bit.OVF = 1;

        if(currentBit == 8) {
            currentBit = 0;
            currentByte++;
            generateStartBit = true;
        }

        if(currentByte == currentDev->activePacket->nBytes) {   // If we've reached the end of the payload
            currentByte = 0;
            currentBit = 0;
            generateStartBit = false;
            preambleLeft = hdw.preambleBits + 1; // Terminate the last packet with a 1, and begin the next with required preamble bits
            if(nRepeat > 0 && currentDev == dev) {
                nRepeat--;
            }
            else if(nextDev != NULL) {
                currentDev = nextDev;
                nextDev = NULL;
                tempPacket = currentDev->activePacket;
                currentDev->activePacket = currentDev->updatePacket; 
                currentDev->updatePacket = tempPacket; 
            } 
            else {                                               
                if(currentDev == maxLoadedDev)                    
                    currentDev = dev;                               
                currentDev++;                                    
            }    
        }

        // Preamble generation code
        if(preambleLeft > 0) {
            preambleLeft--;
            if(preambleLeft == 0)
                generateStartBit = true;
            // Railcom cutout generation code - only done for main tracks
            if(!hdw.is_prog_track) {
                if(preambleLeft == hdw.preambleBits - 2) {  //  If we've already written one- 1 bit to the track
                    writeRailcomPulsetoTrack();             //  Pulse the track to begin the railcom cutout
                return;
                }
                else if(preambleLeft == hdw.preambleBits - 3) { // We're off timing now, using preambleLeft as a status keeper
                    writeCutouttoTrack();                   // Start the cutout on the track
                    preambleLeft -= 2;                      // Jump ahead an additional two bits (total three plus extra time from pulse at beginning)
                    return;
                }
                else if(preambleLeft == hdw.preambleBits - 6) {
                    recoverFromCutout();                    // Turn the track back on, now we will generate the rest of the preamble bits.
                }
            }
            writeOnetoTrack(); 
            return;
        }

        if(generateStartBit) {
            generateStartBit = false;
            writeZerotoTrack(); 
            return;
        }
            
        if(currentDev->activePacket->payload[currentByte] & bitMask[currentBit]){    /* IF bit is a ONE */ 
            writeOnetoTrack();
        } 
        else{                                                                             /* ELSE it is a ZERO */ 
            writeZerotoTrack();
        }                                                                                   /* END-ELSE */    
        currentBit++;                                         /* point to next bit in current Packet */
    }
}

#elif defined(ATMEGA2560)

#include <TimerThree.h>


#elif defined(ATSAM3X8E)

#elif defined(ATMEGA328)

#endif