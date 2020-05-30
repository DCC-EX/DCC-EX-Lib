#include "Hardware.h"
#include "wiring_private.h"

void Hardware::init() {
    // Set up the output pins for this track
    pinMode(signal_a_pin, OUTPUT);
    digitalWrite(signal_a_pin, LOW);
    if(control_scheme == DUAL_DIRECTION_INVERTED || control_scheme == DIRECTION_BRAKE_ENABLE) {
        pinMode(signal_b_pin, OUTPUT);
        digitalWrite(signal_b_pin, HIGH);
    }
    pinMode(enable_pin, OUTPUT);
    digitalWrite(enable_pin, LOW);

    // Set up the current sense pin
    pinMode(current_sense_pin, INPUT);

    // Set up the railcom comparator DAC and serial (SAMD21 only)
    // TODO: Move this into a separate function or library
#if defined(ARDUINO_ARCH_SAMD)
    if(enable_railcom) {
        PORT->Group[0].PINCFG[2].bit.INEN = 0;
        PORT->Group[0].PINCFG[2].bit.PULLEN = 0;
        PORT->Group[0].DIRCLR.reg = 1 << 2;
        PORT->Group[0].PINCFG[2].bit.PMUXEN = 1;        // A0, PA02
        PORT->Group[0].PMUX[2 >> 1].reg |= PORT_PMUX_PMUXE_B; 

        // Enable and configure the DAC
        // // Select the voltage reference using CTRLB.REFSEL
        DAC->CTRLB.bit.REFSEL = 0x0;
        while(DAC->STATUS.bit.SYNCBUSY==1);
        // // Enable the DAC using CTRLA.ENABLE
        DAC->CTRLA.bit.ENABLE = 1;
        while(DAC->STATUS.bit.SYNCBUSY==1);
        // // Enable the DAC as an external output
        DAC->CTRLB.bit.EOEN = 1;
        while(DAC->STATUS.bit.SYNCBUSY==1);
        // Set the output voltage
        DAC->CTRLB.bit.LEFTADJ = 0;
        while(DAC->STATUS.bit.SYNCBUSY==1);
        DAC->DATA.reg = 0x7;    // ~10mV reference voltage
        while(DAC->STATUS.bit.SYNCBUSY==1);

        enableRailcomSerial(false);
    }
#endif
}

void Hardware::setPower(bool on) {
    digitalWrite(enable_pin, on);
}

void Hardware::setSignal(bool high) {
    #if defined(ARDUINO_ARCH_AVR)
    digitalWrite2(signal_a_pin, high);
    if(control_scheme == DUAL_DIRECTION_INVERTED)
        digitalWrite2(signal_b_pin, !high);
    #else
    digitalWrite(signal_a_pin, high);
    if(control_scheme == DUAL_DIRECTION_INVERTED)
        digitalWrite(signal_b_pin, !high);
    #endif
}

void Hardware::setBrake(bool on) {
    #if defined(ARDUINO_ARCH_AVR)
    if(control_scheme == DUAL_DIRECTION_INVERTED) {
        digitalWrite2(signal_a_pin, on);
        digitalWrite2(signal_b_pin, on);
    }
    else if(control_scheme == DIRECTION_BRAKE_ENABLE) {
        digitalWrite2(signal_b_pin, on);
    }
    #else
    if(control_scheme == DUAL_DIRECTION_INVERTED) {
        digitalWrite(signal_a_pin, on);
        digitalWrite(signal_b_pin, on);
    }
    else if(control_scheme == DIRECTION_BRAKE_ENABLE) {
        digitalWrite(signal_b_pin, on);
    }
    #endif
}

float Hardware::getMilliamps(float reading) {
    #if defined(ARDUINO_ARCH_AVR)   // Todo: Using this as a 3.3V/5V and precision detector, but need more robust way to do this.
        return (reading / 1023 * 5 * 1000 * amps_per_volt);
    #elif defined(ARDUINO_ARCH_SAMD)
        return (reading / 4095 * 3.3 * 1000 * amps_per_volt);
    #else
        #error "Cannot compile - invalid architecture for current sensing"
    #endif
}

void Hardware::checkCurrent() {
    // if we have exceeded the CURRENT_SAMPLE_TIME we need to check if we are over/under current.
	if(millis() - lastCheckTime > CURRENT_SAMPLE_TIME) { // TODO can we integrate this with the readBaseCurrent and ackDetect routines?
		lastCheckTime = millis();
		reading = analogRead(current_sense_pin) * CURRENT_SAMPLE_SMOOTHING + reading * (1.0 - CURRENT_SAMPLE_SMOOTHING);

        current = getMilliamps(reading);

		if(current > trigger_value && digitalRead(enable_pin)) {
			setPower(false);
			tripped=true;
			lastTripTime=millis();
		} 
        else if(current < trigger_value && tripped) { // TODO need to put a delay in here so it only tries after X seconds
			if (millis() - lastTripTime > RETRY_MILLIS) {  // TODO make this a global constant
			    setPower(true);
			    tripped=false;
			}
		}
	}
}

// TODO: fix the sercom enable and disable so it is easier to change pins
void Hardware::enableRailcomSerial(bool on) {
#if defined(ARDUINO_ARCH_SAMD)
    if(on) 
        pinPeripheral(5, PIO_INPUT);
    else    
        pinPeripheral(5, PIO_SERCOM);
#endif
}