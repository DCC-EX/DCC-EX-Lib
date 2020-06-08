/*
 *  HardwarePresets.h
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

#include "DCC.h"

DCC* DCC::Create_Arduino_L298Shield_Main(uint8_t numDev) {
  Hardware hdw;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);

  hdw.config_setProgTrack(false);

  hdw.config_setPinSignalA(12);
  hdw.config_setPinSignalB(9);
  hdw.config_setDefaultSignalB(LOW);
  hdw.config_setPinEnable(3);
  hdw.config_setPinCurrentSense(A0);
  
  hdw.config_setTriggerValue(1500);
  hdw.config_setMaxValue(2000);
  hdw.config_setAmpsPerVolt(0.606061);

  hdw.config_setPreambleBits(16);

  hdw.railcom.config_setEnable(false);

  return new DCC(numDev, hdw);
}

DCC* DCC::Create_Arduino_L298Shield_Prog(uint8_t numDev) {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);

  hdw.config_setProgTrack(true);

  hdw.config_setPinSignalA(13);  
  hdw.config_setPinSignalB(8);  
  hdw.config_setDefaultSignalB(LOW);
  hdw.config_setPinEnable(11);    
  hdw.config_setPinCurrentSense(A1); 
  
  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(2000);
  hdw.config_setAmpsPerVolt(0.606061);

  hdw.config_setPreambleBits(22);

  hdw.railcom.config_setEnable(false);

  return new DCC(numDev, hdw);
}

////////////////////////////////////////////////////////////////////////////////

DCC* DCC::Create_Pololu_MC33926Shield_Main(uint8_t numDev) {
  Hardware hdw;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);

  hdw.config_setProgTrack(false);
  
  hdw.config_setPinSignalA(7);  
  hdw.config_setPinSignalB(9);  
  hdw.config_setDefaultSignalB(HIGH);
  hdw.config_setPinEnable(4);    
  hdw.config_setPinCurrentSense(A0); 
  
  hdw.config_setTriggerValue(2500); 
  hdw.config_setMaxValue(3000);
  hdw.config_setAmpsPerVolt(1.904762);

  hdw.config_setPreambleBits(16);

  hdw.railcom.config_setEnable(false);

  return new DCC(numDev, hdw);
}

DCC* DCC::Create_Pololu_MC33926Shield_Prog(uint8_t numDev) {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);

  hdw.config_setProgTrack(true);
  
  hdw.config_setPinSignalA(8);  
  hdw.config_setPinSignalB(10);  
  hdw.config_setDefaultSignalB(HIGH);
  hdw.config_setPinEnable(4);    
  hdw.config_setPinCurrentSense(A1); 

  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(3000);
  hdw.config_setAmpsPerVolt(1.904762);

  hdw.config_setPreambleBits(22);

  hdw.railcom.config_setEnable(false);

  return new DCC(numDev, hdw);
}

////////////////////////////////////////////////////////////////////////////////

#if defined(ARDUINO_ARCH_SAMD)
// TI DRV8874 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Main(uint8_t numDev) {
  Hardware hdw;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DUAL_DIRECTION_INVERTED);

  hdw.config_setProgTrack(false);

  hdw.config_setPinSignalA(6);  
  hdw.config_setPinSignalB(7);  
  hdw.config_setPinEnable(3);    
  hdw.config_setPinCurrentSense(A5); 

  hdw.config_setTriggerValue(5500); 
  hdw.config_setMaxValue(6000);
  hdw.config_setAmpsPerVolt(1.998004);

  hdw.config_setPreambleBits(16);

  hdw.railcom.config_setEnable(true);
  hdw.railcom.config_setRxPin(5);
  hdw.railcom.config_setTxPin(2);   
  hdw.railcom.config_setSerial(nullptr); 
  hdw.railcom.config_setSercom(&sercom4);
  hdw.railcom.config_setRxMux(PIO_SERCOM_ALT);
  hdw.railcom.config_setRxPad(SERCOM_RX_PAD_3);
  hdw.railcom.config_setTxPad(UART_TX_PAD_2);
  hdw.railcom.config_setDACValue(0x7);

  return new DCC(numDev, hdw);
}

// TI DRV8876 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Prog(uint8_t numDev) {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DUAL_DIRECTION_INVERTED);

  hdw.config_setProgTrack(true);

  hdw.config_setPinSignalA(8);  
  hdw.config_setPinSignalB(9);  
  hdw.config_setPinEnable(4);    
  hdw.config_setPinCurrentSense(A1);

  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(3500);
  hdw.config_setAmpsPerVolt(0.909089);

  hdw.config_setPreambleBits(22);

  hdw.railcom.config_setEnable(false);

  return new DCC(numDev, hdw);
}

#endif // ARCH_ARDUINO_SAMD