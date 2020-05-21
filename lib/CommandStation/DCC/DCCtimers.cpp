#include "DCC.h"

byte DCC::bitMask[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};         // masks used in interrupt routine to speed the query of a single bit in a Packet

void DCC::init_timers() {
    // Allow readback of the DCC output pins
    PORT->Group[hdwSettings.signal_a_group].PINCFG[hdwSettings.signal_a_pin].reg=(uint8_t)PORT_PINCFG_INEN;
    // Enable outputs on the DCC output pins
    PORT->Group[hdwSettings.signal_a_group].DIRSET.reg = (uint32_t)(1 << hdwSettings.signal_a_pin);
    // Enable pinmux
    PORT->Group[hdwSettings.signal_a_group].PINCFG[hdwSettings.signal_a_pin].bit.PMUXEN = 1;
    // Set pinmux 
    if(hdwSettings.signal_a_pin % 2 == 0) 
        PORT->Group[hdwSettings.signal_a_group].PMUX[hdwSettings.signal_a_pin >> 1].reg |= hdwSettings.signal_a_timer_mux << PORT_PMUX_PMUXE_Pos;
    else
        PORT->Group[hdwSettings.signal_a_group].PMUX[hdwSettings.signal_a_pin >> 1].reg |= hdwSettings.signal_a_timer_mux << PORT_PMUX_PMUXO_Pos;
    
    if(hdwSettings.use_dual_signal) {
        PORT->Group[hdwSettings.signal_b_group].PINCFG[hdwSettings.signal_b_pin].reg = (uint8_t)PORT_PINCFG_INEN;
        PORT->Group[hdwSettings.signal_b_group].DIRSET.reg = (uint32_t)(1 << hdwSettings.signal_b_pin);
        PORT->Group[hdwSettings.signal_b_group].PINCFG[hdwSettings.signal_b_pin].bit.PMUXEN = 1;
        if(hdwSettings.signal_b_pin % 2 == 0) 
            PORT->Group[hdwSettings.signal_b_group].PMUX[hdwSettings.signal_b_pin >> 1].reg |= hdwSettings.signal_b_timer_mux << PORT_PMUX_PMUXE_Pos;
        else
            PORT->Group[hdwSettings.signal_b_group].PMUX[hdwSettings.signal_b_pin >> 1].reg |= hdwSettings.signal_b_timer_mux << PORT_PMUX_PMUXO_Pos;
    }

    REG_GCLK_GENDIV =   GCLK_GENDIV_DIV(1) |          // Divide 48MHz by 1
                        GCLK_GENDIV_ID(hdwSettings.gclk_num);            // Apply to GCLK4
    while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization
        
    REG_GCLK_GENCTRL =  GCLK_GENCTRL_GENEN |          // Enable GCLK
                        GCLK_GENCTRL_SRC_DFLL48M |    // Set the 48MHz clock source
                        GCLK_GENCTRL_ID(hdwSettings.gclk_num);           // Select GCLK4
    while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization
    
    if(hdwSettings.timer_num == 0) {
        REG_GCLK_CLKCTRL =  GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                        hdwSettings.gclk_num << GCLK_CLKCTRL_GEN_Pos |      // Apply to GCLK4
                        GCLK_CLKCTRL_ID_TCC0_TCC1;    // Feed GCLK to TCC0/1
        while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

        // TCC0->DRVCTRL.reg |= (1 << (TCC_DRVCTRL_INVEN_Pos + hdwSettings.signal_b_timer_bit));
        TCC0->DRVCTRL.reg |= TCC_DRVCTRL_INVEN7;

        TCC0->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV4_Val);
        
        TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;         // Select NPWM as waveform
        while (TCC0->SYNCBUSY.bit.WAVE);                // Wait for synchronization
        
        TCC0->WEXCTRL.bit.OTMX = 0x02;    // Make all of the timer pins change on CC0 compare
        
        TCC0->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;
        while (TCC0->SYNCBUSY.bit.CC0);
        TCC0->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;
        while (TCC0->SYNCBUSY.bit.PER);

        TCC0->INTENSET.reg = TCC_INTENSET_OVF;

        NVIC_EnableIRQ((IRQn_Type) TCC0_IRQn);

        TCC0->CTRLA.reg |= (TCC_CTRLA_ENABLE);          // Turn on the output
        while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
    }
    else if(hdwSettings.timer_num == 1) {
        REG_GCLK_CLKCTRL =  GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                            hdwSettings.gclk_num << GCLK_CLKCTRL_GEN_Pos |      // Apply to GCLK4
                            GCLK_CLKCTRL_ID_TCC0_TCC1;    // Feed GCLK to TCC0/1
        while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

        TCC1->DRVCTRL.reg |= (1 << (TCC_DRVCTRL_INVEN_Pos + hdwSettings.signal_b_timer_bit));
        
        TCC1->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV4_Val);
        
        TCC1->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;         // Select NPWM as waveform
        while (TCC1->SYNCBUSY.bit.WAVE);                // Wait for synchronization
        
        TCC1->WEXCTRL.bit.OTMX = 0x02;    // Make all of the timer pins change on CC0 compare
        
        // Set the duty cycle to 50%. For some reason there is a bug here that requires you to set both CC[0] and CC[1] on TCC1/2
        TCC1->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;
        while (TCC1->SYNCBUSY.bit.CC0);
        TCC1->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;
        while (TCC1->SYNCBUSY.bit.CC1);

        TCC1->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;
        while (TCC1->SYNCBUSY.bit.PER);

        TCC1->INTENSET.reg = TCC_INTENSET_OVF;

        NVIC_EnableIRQ((IRQn_Type) TCC1_IRQn);

        TCC1->CTRLA.reg |= (TCC_CTRLA_ENABLE);          // Turn on the output
        while (TCC1->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
    }
    else if(hdwSettings.timer_num == 2) {
        REG_GCLK_CLKCTRL =  GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                            hdwSettings.gclk_num << GCLK_CLKCTRL_GEN_Pos |      // Apply to GCLK4
                            GCLK_CLKCTRL_ID_TCC2_TC3;    // Feed GCLK to TCC2
        while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

        TCC2->DRVCTRL.reg |= (1 << (TCC_DRVCTRL_INVEN_Pos + hdwSettings.signal_b_timer_bit));
        TCC2->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV4_Val);
        TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;         // Select NPWM as waveform
        while (TCC2->SYNCBUSY.bit.WAVE);                // Wait for synchronization
        
        TCC2->WEXCTRL.bit.OTMX = 0x02;    // Make all of the timer pins change on CC0 compare
        
        // Set the duty cycle to 50%. For some reason there is a bug here that requires you to set both CC[0] and CC[1] on TCC1/2
        TCC2->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;
        while (TCC2->SYNCBUSY.bit.CC0);
        TCC2->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;
        while (TCC2->SYNCBUSY.bit.CC1);
        
        TCC2->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;
        while (TCC2->SYNCBUSY.bit.PER);

        TCC2->INTENSET.reg = TCC_INTENSET_OVF;

        NVIC_EnableIRQ((IRQn_Type) TCC2_IRQn);

        TCC2->CTRLA.reg |= (TCC_CTRLA_ENABLE);          // Turn on the output
        while (TCC2->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
    }
}

void DCC::interrupt_handler() volatile {
    if (hdwSettings.timer->INTFLAG.bit.OVF)             
    { 
        hdwSettings.timer->INTFLAG.bit.OVF = 1;

        if(currentBit == 8) {
            currentBit = 0;
            currentByte++;
            generateStartBit = true;
        }

        if(currentByte == currentDev->activePacket->nBytes) {   // If we've reached the end of the payload
            currentByte = 0;
            currentBit = 0;
            generateStartBit = false;
            preambleLeft = hdwSettings.preambleBits + 1; // Terminate the last packet with a 1, and begin the next with required preamble bits
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
            if(!hdwSettings.is_prog_track) {
                if(preambleLeft == hdwSettings.preambleBits - 2) {
                    writeRailcomPulsetoTrack();
                return;
                }
                else if(preambleLeft == hdwSettings.preambleBits - 3) {
                    writeCutouttoTrack();
                    preambleLeft -= 2;
                    return;
                }
                else if(preambleLeft == hdwSettings.preambleBits - 6) {
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
    hdwSettings.timer->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
    hdwSettings.timer->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
    hdwSettings.timer->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::writeZerotoTrack() volatile {
    hdwSettings.timer->CC[0].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
    hdwSettings.timer->CC[1].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
    hdwSettings.timer->PER.reg = DCC_ZERO_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::writeRailcomPulsetoTrack() volatile {
    hdwSettings.timer->CC[0].reg = RC_PULSE_DURATION + 1;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
    hdwSettings.timer->CC[1].reg = RC_PULSE_DURATION + 1;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
    hdwSettings.timer->PER.reg = RC_PULSE_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::writeCutouttoTrack() volatile {
    PORT->Group[hdwSettings.signal_a_group].PINCFG[hdwSettings.signal_a_pin].bit.PMUXEN = 0;
    PORT->Group[hdwSettings.signal_b_group].PINCFG[hdwSettings.signal_b_pin].bit.PMUXEN = 0;
    // TODO: Add support for a brake pin instead of the second signal

    PORT->Group[hdwSettings.signal_a_group].OUTSET.reg |= 1 << hdwSettings.signal_a_pin;
    PORT->Group[hdwSettings.signal_a_group].OUTSET.reg |= 1 << hdwSettings.signal_b_pin;
 
    hdwSettings.timer->PER.reg = RC_CUTOUT_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
}

void DCC::recoverFromCutout() volatile {
    PORT->Group[hdwSettings.signal_a_group].PINCFG[hdwSettings.signal_a_pin].bit.PMUXEN = 1;
    PORT->Group[hdwSettings.signal_b_group].PINCFG[hdwSettings.signal_b_pin].bit.PMUXEN = 1;
}