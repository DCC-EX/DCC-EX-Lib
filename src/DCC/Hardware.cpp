#include "Hardware.h"
#include "wiring_private.h"

void Hardware::setup() {
    // Set up the output pins for this track
    pinMode(signal_a_pin, OUTPUT);
    digitalWrite(signal_a_pin, LOW);
    if(control_scheme == DUAL_DIRECTION_INVERTED || control_scheme == DIRECTION_BRAKE_ENABLE) {
        pinMode(signal_b_pin, OUTPUT);
        digitalWrite(signal_b_pin, signal_b_default);
    }
    pinMode(enable_pin, OUTPUT);
    digitalWrite(enable_pin, LOW);

    // Set up the current sense pin
    pinMode(current_sense_pin, INPUT);

    // Set up the railcom comparator DAC and serial (SAMD21 only)
    // TODO: Move this into a separate function or library

    if(enable_railcom) {
    #if defined(ARDUINO_ARCH_SAMD)
        enableRailcomDAC();
        if(railcom_serial == nullptr) {
            railcom_serial = new Uart(railcom_sercom, railcom_rx_pin, railcom_tx_pin, railcom_rx_pad, railcom_tx_pad);
        }
    #endif
        railcom_serial->begin(railcom_baud);
    }

    tripped = false;
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
        digitalWrite2(signal_b_pin, signal_b_default?on:!on);
    }
    #else
    if(control_scheme == DUAL_DIRECTION_INVERTED) {
        digitalWrite(signal_a_pin, on);
        digitalWrite(signal_b_pin, on);
    }
    else if(control_scheme == DIRECTION_BRAKE_ENABLE) {
        digitalWrite(signal_b_pin, signal_b_default?on:!on);
    }
    #endif
}

bool Hardware::getStatus() {
	return digitalRead(enable_pin);
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
		reading = readCurrent() * CURRENT_SAMPLE_SMOOTHING + reading * (1.0 - CURRENT_SAMPLE_SMOOTHING);

        current = getMilliamps(reading);

		if(current > trigger_value && digitalRead(enable_pin)) {
			setPower(false);    // Todo: add announce feature back in so JMRI knows when the power goes out.
			tripped=true;
			lastTripTime=millis();
		} 
        else if(current < trigger_value && tripped) {
			if (millis() - lastTripTime > RETRY_MILLIS) {
			    setPower(true);
			    tripped=false;
			}
		}
	}
}

uint16_t Hardware::readCurrent() {
    return analogRead(current_sense_pin);
}

// Todo: fix for AVR
void Hardware::enableRailcomSerial(bool on) {
    if(on) {
    #if defined(ARDUINO_ARCH_SAMD)
        pinPeripheral(railcom_rx_pin, railcom_rx_mux);
    #endif    
    }
    else {
    #if defined(ARDUINO_ARCH_SAMD)
        pinPeripheral(railcom_rx_pin, PIO_INPUT);
    #endif
    }
}

#if defined(ARDUINO_ARCH_SAMD)
// Sets up the DAC on pin A0
void Hardware::enableRailcomDAC() {
    PORT->Group[0].PINCFG[2].bit.INEN = 0;      // Disable input on DAC pin
    PORT->Group[0].PINCFG[2].bit.PULLEN = 0;    // Disable pullups
    PORT->Group[0].DIRCLR.reg = 1 << 2;         // Disable digital outputs on DAC pin
    PORT->Group[0].PINCFG[2].bit.PMUXEN = 1;    // Enables pinmuxing
    PORT->Group[0].PMUX[2 >> 1].reg |= PORT_PMUX_PMUXE_B;   // Sets pin to analog mux

    // // Select the voltage reference to the internal 1V reference
    DAC->CTRLB.bit.REFSEL = 0x0;
    while(DAC->STATUS.bit.SYNCBUSY==1);     // Wait for sync
    // // Enable the DAC
    DAC->CTRLA.bit.ENABLE = 1;
    while(DAC->STATUS.bit.SYNCBUSY==1);     // Wait for sync
    // // Enable the DAC as an external output
    DAC->CTRLB.bit.EOEN = 1;
    while(DAC->STATUS.bit.SYNCBUSY==1);     // Wait for sync
    // Set the output voltage   
    DAC->CTRLB.bit.LEFTADJ = 0;
    while(DAC->STATUS.bit.SYNCBUSY==1);     // Wait for sync
    DAC->DATA.reg = railcom_dac_value;    // ~10mV reference voltage
    while(DAC->STATUS.bit.SYNCBUSY==1);     // Wait for sync
}
#endif