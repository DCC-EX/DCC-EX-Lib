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

#include "DCCMain.h"
#include "DCCService.h"

#if defined(ARDUINO_ARCH_SAMC)

// TI DRV8873 on custom board
DCCMain* DCCMain::Create_WSM_FireBox_MK1S_Main(uint8_t numDevices) {
  Hardware hdw;
  Railcom rcom;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DUAL_DIRECTION_INVERTED);

  hdw.config_setPinSignalA(21);  
  hdw.config_setPinSignalB(22);  
  hdw.config_setPinEnable(24);    
  hdw.config_setDefaultEnable(DEFAULT_HIGH);
  hdw.config_setPinSleep(25);
  hdw.config_setDefaultSleep(DEFAULT_LOW);
  hdw.config_setPinCurrentSense(23); 

  hdw.config_setTriggerValue(5000); 
  hdw.config_setMaxValue(6500);
  hdw.config_setAmpsPerVolt(6.111);

  hdw.config_setPreambleBits(16);

  rcom.config_setEnable(true);
  rcom.config_setRxPin(34);
  rcom.config_setTxPin(34);   
  rcom.config_setSerial(nullptr); 
  rcom.config_setSercom(&sercom0);
  rcom.config_setRxMux(PIO_SERCOM);
  rcom.config_setRxPad(SERCOM_RX_PAD_0);
  rcom.config_setTxPad(UART_TX_PAD_2);
  rcom.config_setDACValue(0x7);

  return new DCCMain(numDevices, hdw, rcom);
}

// TI DRV8873 on custom board
DCCService* DCCService::Create_WSM_FireBox_MK1S_Prog() {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DUAL_DIRECTION_INVERTED);

  hdw.config_setPinSignalA(27);  
  hdw.config_setPinSignalB(28);  
  hdw.config_setPinEnable(30);    
  hdw.config_setDefaultEnable(DEFAULT_HIGH);
  hdw.config_setPinSleep(31);
  hdw.config_setDefaultSleep(DEFAULT_LOW);
  hdw.config_setPinCurrentSense(29);

  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(6500);
  hdw.config_setAmpsPerVolt(6.111);

  hdw.config_setPreambleBits(22);

  return new DCCService(hdw);
}

#elif defined(ARDUINO_ARCH_SAMD)
// TI DRV8874 on custom board
DCCMain* DCCMain::Create_WSM_FireBox_MK1_Main(uint8_t numDevices) {
  Hardware hdw;
  Railcom rcom;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DUAL_DIRECTION_INVERTED);

  hdw.config_setPinSignalA(6);  
  hdw.config_setPinSignalB(7);  
  hdw.config_setPinEnable(3);    
  hdw.config_setDefaultEnable(DEFAULT_LOW);
  hdw.config_setDefaultSleep(DEFAULT_NOT_USED);
  hdw.config_setPinCurrentSense(A5); 

  hdw.config_setTriggerValue(5500); 
  hdw.config_setMaxValue(6000);
  hdw.config_setAmpsPerVolt(1.998004);

  hdw.config_setPreambleBits(16);

  rcom.config_setEnable(true);
  rcom.config_setRxPin(5);
  rcom.config_setTxPin(2);   
  rcom.config_setSerial(nullptr); 
  rcom.config_setSercom(&sercom4);
  rcom.config_setRxMux(PIO_SERCOM_ALT);
  rcom.config_setRxPad(SERCOM_RX_PAD_3);
  rcom.config_setTxPad(UART_TX_PAD_2);
  rcom.config_setDACValue(0x7);

  return new DCCMain(numDevices, hdw, rcom);
}

// TI DRV8876 on custom board
DCCService* DCCService::Create_WSM_FireBox_MK1_Prog() {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DUAL_DIRECTION_INVERTED);

  hdw.config_setPinSignalA(8);  
  hdw.config_setPinSignalB(9);  
  hdw.config_setPinEnable(4);   
  hdw.config_setDefaultEnable(DEFAULT_LOW);
  hdw.config_setDefaultSleep(DEFAULT_NOT_USED); 
  hdw.config_setPinCurrentSense(A1);

  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(3500);
  hdw.config_setAmpsPerVolt(0.909089);

  hdw.config_setPreambleBits(22);

  return new DCCService(hdw);
}

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_AVR)

DCCMain* DCCMain::Create_Arduino_L298Shield_Main(uint8_t numDevices) {
  Hardware hdw;
  Railcom rcom;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);

  hdw.config_setPinSignalA(12);
  hdw.config_setPinSignalB(9);
  hdw.config_setDefaultSignalB(DEFAULT_LOW);
  hdw.config_setPinEnable(3);
  hdw.config_setDefaultEnable(DEFAULT_LOW);
  hdw.config_setDefaultSleep(DEFAULT_NOT_USED);
  hdw.config_setPinCurrentSense(A0);
  
  hdw.config_setTriggerValue(1500);
  hdw.config_setMaxValue(2000);
  hdw.config_setAmpsPerVolt(0.606061);

  hdw.config_setPreambleBits(16);

  rcom.config_setEnable(false);

  return new DCCMain(numDevices, hdw, rcom);
}

DCCService* DCCService::Create_Arduino_L298Shield_Prog() {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);

  hdw.config_setPinSignalA(13);  
  hdw.config_setPinSignalB(8);  
  hdw.config_setDefaultSignalB(DEFAULT_LOW);
  hdw.config_setPinEnable(11);    
  hdw.config_setDefaultEnable(DEFAULT_LOW);
  hdw.config_setDefaultSleep(DEFAULT_NOT_USED);
  hdw.config_setPinCurrentSense(A1); 
  
  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(2000);
  hdw.config_setAmpsPerVolt(0.606061);

  hdw.config_setPreambleBits(22);

  return new DCCService(hdw);
}

////////////////////////////////////////////////////////////////////////////////

DCCMain* DCCMain::Create_Pololu_MC33926Shield_Main(uint8_t numDevices) {
  Hardware hdw;
  Railcom rcom;

  hdw.config_setChannelName("MAIN");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);
  
  hdw.config_setPinSignalA(7);  
  hdw.config_setPinSignalB(9);  
  hdw.config_setDefaultSignalB(DEFAULT_HIGH);
  hdw.config_setPinEnable(4);    
  hdw.config_setDefaultEnable(DEFAULT_LOW);
  hdw.config_setDefaultSleep(DEFAULT_NOT_USED);
  hdw.config_setPinCurrentSense(A0); 
  
  hdw.config_setTriggerValue(2500); 
  hdw.config_setMaxValue(3000);
  hdw.config_setAmpsPerVolt(1.904762);

  hdw.config_setPreambleBits(16);

  rcom.config_setEnable(false);

  return new DCCMain(numDevices, hdw, rcom);
}

DCCService* DCCService::Create_Pololu_MC33926Shield_Prog() {
  Hardware hdw;
  
  hdw.config_setChannelName("PROG");

  hdw.config_setControlScheme(DIRECTION_BRAKE_ENABLE);
  
  hdw.config_setPinSignalA(8);  
  hdw.config_setPinSignalB(10);  
  hdw.config_setDefaultSignalB(DEFAULT_HIGH);
  hdw.config_setPinEnable(4);    
  hdw.config_setDefaultEnable(DEFAULT_LOW);
  hdw.config_setDefaultSleep(DEFAULT_NOT_USED);
  hdw.config_setPinCurrentSense(A1); 

  hdw.config_setTriggerValue(250); 
  hdw.config_setMaxValue(3000);
  hdw.config_setAmpsPerVolt(1.904762);

  hdw.config_setPreambleBits(22);

  return new DCCService(hdw);
}

#endif

