/*
 *  Hardware.cpp
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

#include "Hardware.h"

#if defined(ARDUINO_ARCH_SAMD)
#include "wiring_private.h"
#define writePin digitalWrite
#elif defined(ARDUINO_ARCH_AVR)
// Library DIO2.h is only compatible with AVR, and SAM digitalWrite is a lot 
// faster than AVR digitalWrite.
#include <DIO2.h>
#define writePin digitalWrite2
#endif

void Hardware::setup() {
  // Set up the output pins for this track
  pinMode(signal_a_pin, OUTPUT);
  writePin(signal_a_pin, LOW);
  if(control_scheme == DUAL_DIRECTION_INVERTED 
    || control_scheme == DIRECTION_BRAKE_ENABLE) {
    pinMode(signal_b_pin, OUTPUT);
    writePin(signal_b_pin, signal_b_default);
  }
  pinMode(enable_pin, OUTPUT);
  writePin(enable_pin, LOW);

  // Set up the current sense pin
  pinMode(current_sense_pin, INPUT);

  pinMode(railcom_rx_pin, INPUT);

  if(enable_railcom) {
  #if defined(ARDUINO_ARCH_SAMD)
    enableRailcomDAC();
    if(railcom_serial == nullptr) {
      railcom_serial = new Uart(railcom_sercom, railcom_rx_pin, railcom_tx_pin, 
        railcom_rx_pad, railcom_tx_pad);
    }
  #endif
    railcom_serial->begin(railcom_baud);
  }

  tripped = false;
}

void Hardware::setPower(bool on) {
  writePin(enable_pin, on);
}

void Hardware::setSignal(bool high) {
  writePin(signal_a_pin, high);
  if(control_scheme == DUAL_DIRECTION_INVERTED)
    writePin(signal_b_pin, !high);
}

void Hardware::setBrake(bool on) {
  if(control_scheme == DUAL_DIRECTION_INVERTED) {
    writePin(signal_a_pin, on);
    writePin(signal_b_pin, on);
  }
  else if(control_scheme == DIRECTION_BRAKE_ENABLE) {
    writePin(signal_b_pin, signal_b_default?on:!on);
  }
}

float Hardware::getMilliamps(uint32_t reading) {
  // TODO(davidcutting42@gmail.com): Using this as a 3.3V/5V and precision 
  // detector, but need more robust way to do this.
  #if defined(ARDUINO_ARCH_AVR)   
    return ((float)reading / 1023.0 * 5 * 1000 * amps_per_volt);
  #elif defined(ARDUINO_ARCH_SAMD)
    return ((float)reading / 4095.0 * 3.3 * 1000 * amps_per_volt);
  #else
    #error "Cannot compile - invalid architecture for current sensing"
  #endif
}

void Hardware::checkCurrent() {
  // if we have exceeded the CURRENT_SAMPLE_TIME we need to check if we are 
  // over/under current.
  if(millis() - lastCheckTime > kCurrentSampleTime) {
    lastCheckTime = millis();
    reading = readCurrent() * kCurrentSampleSmoothing + reading * 
      (1.0 - kCurrentSampleSmoothing);

    current = getMilliamps(reading);

    if(current > trigger_value && digitalRead(enable_pin)) {
      // TODO(davidcutting42@gmail.com) add announce feature back in so JMRI 
      // knows when the power goes out.
      setPower(false);    
      tripped=true;
      lastTripTime=millis();
    } 
    else if(current < trigger_value && tripped) {
      if (millis() - lastTripTime > kRetryTime) {
        setPower(true);
        tripped=false;
      }
    }
  }
}

// TODO(davidcutting42@gmail.com): test on AVR
void Hardware::enableRailcomSerial(bool on) {
  if(on) {
  #if defined(ARDUINO_ARCH_SAMD)
    pinPeripheral(railcom_rx_pin, railcom_rx_mux);
  #else
    railcom_serial->begin(railcom_baud);
  #endif    
  }
  else {
  #if defined(ARDUINO_ARCH_SAMD)
    pinPeripheral(railcom_rx_pin, PIO_INPUT);
  #else
    railcom_serial->end();
  #endif    
  }
}

#if defined(ARDUINO_ARCH_SAMD)
// Sets up the DAC on pin A0
void Hardware::enableRailcomDAC() {
  PORT->Group[0].PINCFG[2].bit.INEN = 0;      // Disable input on DAC pin
  PORT->Group[0].PINCFG[2].bit.PULLEN = 0;    // Disable pullups
  PORT->Group[0].DIRCLR.reg = 1 << 2;      // Disable digital outputs on DAC pin
  PORT->Group[0].PINCFG[2].bit.PMUXEN = 1;    // Enables pinmuxing
  PORT->Group[0].PMUX[2 >> 1].reg |= PORT_PMUX_PMUXE_B; // Sets pin to analog

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