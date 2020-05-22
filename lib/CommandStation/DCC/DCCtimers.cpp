#include "DCC.h"

byte DCC::bitMask[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};         // masks used in interrupt routine to speed the query of a single bit in a Packet

void DCC::init_timers() {
    // Enable outputs on the DCC output pins
    PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].DIRSET.reg = (uint32_t)(1 << g_APinDescription[hdw.signal_a_pin].ulPin);
    // Enable pinmux
    PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].PINCFG[g_APinDescription[hdw.signal_a_pin].ulPin].bit.PMUXEN = 1;
    // Set pinmux 
    if(g_APinDescription[hdw.signal_a_pin].ulPin % 2 == 0) 
        PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].PMUX[g_APinDescription[hdw.signal_a_pin].ulPin >> 1].reg |= hdw.signal_a_timer_mux << PORT_PMUX_PMUXE_Pos;
    else
        PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].PMUX[g_APinDescription[hdw.signal_a_pin].ulPin >> 1].reg |= hdw.signal_a_timer_mux << PORT_PMUX_PMUXO_Pos;
    
    if(hdw.use_dual_signal) {
        PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].DIRSET.reg = (uint32_t)(1 << g_APinDescription[hdw.signal_b_pin].ulPin);
        PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].PINCFG[g_APinDescription[hdw.signal_b_pin].ulPin].bit.PMUXEN = 1;
        if(g_APinDescription[hdw.signal_b_pin].ulPin % 2 == 0) 
            PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].PMUX[g_APinDescription[hdw.signal_b_pin].ulPin >> 1].reg |= hdw.signal_b_timer_mux << PORT_PMUX_PMUXE_Pos;
        else
            PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].PMUX[g_APinDescription[hdw.signal_b_pin].ulPin >> 1].reg |= hdw.signal_b_timer_mux << PORT_PMUX_PMUXO_Pos;
    }

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

    hdw.timer->DRVCTRL.reg |= (1 << (TCC_DRVCTRL_INVEN_Pos + hdw.signal_b_timer_bit));

    hdw.timer->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV4_Val);
    
    hdw.timer->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;         // Select NPWM as waveform
    while (hdw.timer->SYNCBUSY.bit.WAVE);                // Wait for synchronization
    
    hdw.timer->WEXCTRL.bit.OTMX = 0x02;    // Make all of the timer pins change on CC0 compare
    
    hdw.timer->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;
    while (hdw.timer->SYNCBUSY.bit.CC0);
    hdw.timer->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;
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

        if(preambleLeft > 0) {
            preambleLeft--;
            if(preambleLeft == 0)
                generateStartBit = true;
            if(!hdw.is_prog_track) {
                if(preambleLeft == hdw.preambleBits - 2) {
                    writeRailcomPulsetoTrack();
                return;
                }
                else if(preambleLeft == hdw.preambleBits - 3) {
                    writeCutouttoTrack();
                    preambleLeft -= 2;
                    return;
                }
                else if(preambleLeft == hdw.preambleBits - 6) {
                    recoverFromCutout();
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

void DCC::writeOnetoTrack() volatile {
    hdw.timer->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
    hdw.timer->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
    hdw.timer->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::writeZerotoTrack() volatile {
    hdw.timer->CC[0].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
    hdw.timer->CC[1].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
    hdw.timer->PER.reg = DCC_ZERO_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::writeRailcomPulsetoTrack() volatile {
    hdw.timer->CC[0].reg = RC_PULSE_DURATION + 1;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
    hdw.timer->CC[1].reg = RC_PULSE_DURATION + 1;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
    hdw.timer->PER.reg = RC_PULSE_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::writeCutouttoTrack() volatile {
    PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].PINCFG[g_APinDescription[hdw.signal_a_pin].ulPin].bit.PMUXEN = 0;
    PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].PINCFG[g_APinDescription[hdw.signal_b_pin].ulPin].bit.PMUXEN = 0;
    // TODO: Add support for a brake pin instead of the second signal

    PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].OUTSET.reg |= 1 << g_APinDescription[hdw.signal_a_pin].ulPin;
    PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].OUTSET.reg |= 1 << g_APinDescription[hdw.signal_b_pin].ulPin;
 
    hdw.timer->PER.reg = RC_CUTOUT_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::recoverFromCutout() volatile {
    PORT->Group[g_APinDescription[hdw.signal_a_pin].ulPort].PINCFG[g_APinDescription[hdw.signal_a_pin].ulPin].bit.PMUXEN = 1;
    PORT->Group[g_APinDescription[hdw.signal_b_pin].ulPort].PINCFG[g_APinDescription[hdw.signal_b_pin].ulPin].bit.PMUXEN = 1;
}