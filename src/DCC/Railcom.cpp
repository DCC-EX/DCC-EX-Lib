/*
 *  Railcom.cpp
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

#include "Railcom.h"

#include <avr/pgmspace.h>

#include "../CommInterface/CommManager.h"
#include "DCC.h"

#if defined(ARDUINO_ARCH_SAMD)
  #include "wiring_private.h"
#endif

const uint8_t railcom_decode[256] PROGMEM =
{    INV,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
     INV,    INV,    INV,    INV,    INV,    INV,    INV,   NACK,
     INV,    INV,    INV,    INV,    INV,    INV,    INV,   0x33,
     INV,    INV,    INV,   0x34,    INV,   0x35,   0x36,    INV,
     INV,    INV,    INV,    INV,    INV,    INV,    INV,   0x3A,
     INV,    INV,    INV,   0x3B,    INV,   0x3C,   0x37,    INV,
     INV,    INV,    INV,   0x3F,    INV,   0x3D,   0x38,    INV,
     INV,   0x3E,   0x39,    INV, RESVD3,    INV,    INV,    INV,
     INV,    INV,    INV,    INV,    INV,    INV,    INV,   0x24,
     INV,    INV,    INV,   0x23,    INV,   0x22,   0x21,    INV,
     INV,    INV,    INV,   0x1F,    INV,   0x1E,   0x20,    INV,
     INV,   0x1D,   0x1C,    INV,   0x1B,    INV,    INV,    INV,
     INV,    INV,    INV,   0x19,    INV,   0x18,   0x1A,    INV,
     INV,   0x17,   0x16,    INV,   0x15,    INV,    INV,    INV,
     INV,   0x25,   0x14,    INV,   0x13,    INV,    INV,    INV,
    0x32,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
     INV,    INV,    INV,    INV,    INV,    INV,    INV, RESVD2,
     INV,    INV,    INV,   0x0E,    INV,   0x0D,   0x0C,    INV,
     INV,    INV,    INV,   0x0A,    INV,   0x09,   0x0B,    INV,
     INV,   0x08,   0x07,    INV,   0x06,    INV,    INV,    INV,
     INV,    INV,    INV,   0x04,    INV,   0x03,   0x05,    INV,
     INV,   0x02,   0x01,    INV,   0x00,    INV,    INV,    INV,
     INV,   0x0F,   0x10,    INV,   0x11,    INV,    INV,    INV,
    0x12,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
     INV,    INV,    INV, RESVD1,    INV,   0x2B,   0x30,    INV,
     INV,   0x2A,   0x2F,    INV,   0x31,    INV,    INV,    INV,
     INV,   0x29,   0x2E,    INV,   0x2D,    INV,    INV,    INV,
    0x2C,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
     INV,   BUSY,   0x28,    INV,   0x27,    INV,    INV,    INV,
    0x26,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
     ACK,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
     INV,    INV,    INV,    INV,    INV,    INV,    INV,    INV,
};

void Railcom::setup() {
  if(enable) {

  #if defined(ARDUINO_ARCH_SAMD)
    setupDAC();
    if(serial == nullptr) {
      serial = new Uart(sercom, rx_pin, tx_pin, rx_pad, tx_pad);
    }
  #endif

    serial->begin(baud);

  }
}

#if defined(ARDUINO_ARCH_SAMD)
// Sets up the DAC on pin A0
void Railcom::setupDAC() {
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
  DAC->DATA.reg = dac_value;    // ~10mV reference voltage
  while(DAC->STATUS.bit.SYNCBUSY==1);     // Wait for sync
}
#endif

// TODO(davidcutting42@gmail.com): test on AVR
void Railcom::enableRecieve(uint8_t on) {
  if(on) {
  #if defined(ARDUINO_ARCH_SAMD)
    while(serial->available()) {
      serial->read();   // Flush the buffer so we don't get a bunch of garbage
    }
    pinPeripheral(rx_pin, rx_mux);
  #else
    serial->begin(baud);
  #endif    
  }
  else {
  #if defined(ARDUINO_ARCH_SAMD)
    pinPeripheral(rx_pin, PIO_INPUT);
  #else
    serial->end();
  #endif    
  }
}

// This is called from an interrupt routine, so it's gotta be quick. DON'T try
// to write to the serial port here. You'll destroy the waveform.
void Railcom::readData(uint16_t _uniqueID) {
  if(dataReady) return;
  
  uint8_t bytes = serial->available();
  if(bytes > 8) bytes = 8;
  serial->readBytes(rawData, bytes);

  uniqueID = _uniqueID;
  
  if(bytes > 0)
    dataReady = true;
}

void Railcom::processData() {
  if(dataReady) {
    CommManager::printf(F("Railcom RAW %d = %x %x %x %x %x %x %x %x\n\r"), uniqueID,
      rawData[0], rawData[1], rawData[2], rawData[3], 
      rawData[4], rawData[5], rawData[6], rawData[7]
    );

    for (size_t i = 0; i < 8; i++)
    {
      rawData[i] = pgm_read_byte_near(&railcom_decode[rawData[i]]);
      if(rawData[i] == INV) {
        dataReady = false;
        return;
      }
    }
    
    CommManager::printf(F("Railcom DCD %d = %x %x %x %x %x %x %x %x\n\r"), uniqueID,
      rawData[0], rawData[1], rawData[2], rawData[3], 
      rawData[4], rawData[5], rawData[6], rawData[7]
    );
    
    dataReady = false;
    return;
  }
}