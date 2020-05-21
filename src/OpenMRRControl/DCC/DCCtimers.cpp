#include "DCC.h"

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
    switch (hdwSettings.timer_num)
    {
    case 0:
        if (TCC0->INTFLAG.bit.OVF)             
        { 
            TCC0->INTFLAG.bit.OVF = 1;
            if(currentBit == currentDev->activePacket->nBits) {    /* IF no more bits in this DCC Packet */
                currentBit = 0;                                       /*   reset current bit pointer and determine which Register and Packet to process next--- */ 
                if(nRepeat > 0 && currentDev == dev) {               /*   IF current Register is first Register AND should be repeated */ 
                    nRepeat--;                                        /*     decrement repeat count; result is this same Packet will be repeated */ 
                } else if(nextDev != NULL) {                           /*   ELSE IF another Register has been updated */ 
                    currentDev = nextDev;                             /*     update currentReg to nextReg */ 
                    nextDev = NULL;                                     /*     reset nextReg to NULL */ 
                    tempPacket = currentDev->activePacket;            /*     flip active and update Packets */ 
                    currentDev->activePacket = currentDev->updatePacket; 
                    currentDev->updatePacket = tempPacket;               
                } else {                                               /*   ELSE simply move to next Register */ 
                    if(currentDev == maxLoadedDev)                    /*     BUT IF this is last Register loaded */ 
                        currentDev = dev;                               /*       first reset currentReg to base Register, THEN */ 
                    currentDev++;                                     /*     increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ 
                }                                                     /*   END-ELSE */ 
                }                                                       /*   END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */ 
                if(currentDev->activePacket->buf[currentBit/8] & bitMask[currentBit%8]){    /* IF bit is a ONE */ 
                TCC0->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;                                         /*   set register CC0 for timer N to half cycle duration of DCC ONE bit */
                TCC0->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */  
                TCC0->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;                                         /*   set register PER for timer N to full cycle duration of DCC ONE bit */ 
                } else{                                                                             /* ELSE it is a ZERO */ 
                TCC0->CC[0].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
                TCC0->CC[1].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
                TCC0->PER.reg = DCC_ZERO_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
            }                                                                                   /* END-ELSE */    
            currentBit++;                                         /* point to next bit in current Packet */
        }
        break;
    
    case 1:
        if (TCC1->INTFLAG.bit.OVF)             
        { 
            TCC1->INTFLAG.bit.OVF = 1;
            if(currentBit == currentDev->activePacket->nBits) {    /* IF no more bits in this DCC Packet */
                currentBit = 0;                                       /*   reset current bit pointer and determine which Register and Packet to process next--- */ 
                if(nRepeat > 0 && currentDev == dev) {               /*   IF current Register is first Register AND should be repeated */ 
                    nRepeat--;                                        /*     decrement repeat count; result is this same Packet will be repeated */ 
                } else if(nextDev != NULL) {                           /*   ELSE IF another Register has been updated */ 
                    currentDev = nextDev;                             /*     update currentReg to nextReg */ 
                    nextDev = NULL;                                     /*     reset nextReg to NULL */ 
                    tempPacket = currentDev->activePacket;            /*     flip active and update Packets */ 
                    currentDev->activePacket = currentDev->updatePacket; 
                    currentDev->updatePacket = tempPacket;               
                } else {                                               /*   ELSE simply move to next Register */ 
                    if(currentDev == maxLoadedDev)                    /*     BUT IF this is last Register loaded */ 
                        currentDev = dev;                               /*       first reset currentReg to base Register, THEN */ 
                    currentDev++;                                     /*     increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ 
                }                                                     /*   END-ELSE */ 
                }                                                       /*   END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */ 
                if(currentDev->activePacket->buf[currentBit/8] & bitMask[currentBit%8]){    /* IF bit is a ONE */ 
                TCC1->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;                                         /*   set register CC0 for timer N to half cycle duration of DCC ONE bit */
                TCC1->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */  
                TCC1->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;                                         /*   set register PER for timer N to full cycle duration of DCC ONE bit */ 
                } else{                                                                             /* ELSE it is a ZERO */ 
                TCC1->CC[0].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
                TCC1->CC[1].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
                TCC1->PER.reg = DCC_ZERO_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
            }                                                                                   /* END-ELSE */    
            currentBit++;                                         /* point to next bit in current Packet */
        }
        break;
    case 2:
        if (TCC1->INTFLAG.bit.OVF)             
        { 
            TCC1->INTFLAG.bit.OVF = 1;
            if(currentBit == currentDev->activePacket->nBits) {    /* IF no more bits in this DCC Packet */
                currentBit = 0;                                       /*   reset current bit pointer and determine which Register and Packet to process next--- */ 
                if(nRepeat > 0 && currentDev == dev) {               /*   IF current Register is first Register AND should be repeated */ 
                    nRepeat--;                                        /*     decrement repeat count; result is this same Packet will be repeated */ 
                } else if(nextDev != NULL) {                           /*   ELSE IF another Register has been updated */ 
                    currentDev = nextDev;                             /*     update currentReg to nextReg */ 
                    nextDev = NULL;                                     /*     reset nextReg to NULL */ 
                    tempPacket = currentDev->activePacket;            /*     flip active and update Packets */ 
                    currentDev->activePacket = currentDev->updatePacket; 
                    currentDev->updatePacket = tempPacket;               
                } else {                                               /*   ELSE simply move to next Register */ 
                    if(currentDev == maxLoadedDev)                    /*     BUT IF this is last Register loaded */ 
                        currentDev = dev;                               /*       first reset currentReg to base Register, THEN */ 
                    currentDev++;                                     /*     increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ 
                }                                                     /*   END-ELSE */ 
                }                                                       /*   END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */ 
                if(currentDev->activePacket->buf[currentBit/8] & bitMask[currentBit%8]){    /* IF bit is a ONE */ 
                TCC2->CC[0].reg = DCC_ONE_BIT_PULSE_DURATION;                                         /*   set register CC0 for timer N to half cycle duration of DCC ONE bit */
                TCC2->CC[1].reg = DCC_ONE_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */  
                TCC2->PER.reg = DCC_ONE_BIT_TOTAL_DURATION;                                         /*   set register PER for timer N to full cycle duration of DCC ONE bit */ 
                } else{                                                                             /* ELSE it is a ZERO */ 
                TCC2->CC[0].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC0 for timer N to half cycle duration of DCC ZERO bit */ 
                TCC2->CC[1].reg = DCC_ZERO_BIT_PULSE_DURATION;                                        /*   set register CC1 for timer N to half cycle duration of DCC ZERO bit */ 
                TCC2->PER.reg = DCC_ZERO_BIT_TOTAL_DURATION;                                        /*   set register PER for timer N to full cycle duration of DCC ZERO bit */ 
            }                                                                                   /* END-ELSE */    
            currentBit++;                                         /* point to next bit in current Packet */
        }
        break;
    }
}

byte DCC::bitMask[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};         // masks used in interrupt routine to speed the query of a single bit in a Packet