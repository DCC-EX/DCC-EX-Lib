#include "DCC.h"

DCC* DCC::Create_Arduino_L298Shield_Main(int numDev) {
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

    hdw.config_setRailcom(false);

    return new DCC(numDev, hdw);
}

DCC* DCC::Create_Arduino_L298Shield_Prog(int numDev) {
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

    hdw.config_setRailcom(false);

    return new DCC(numDev, hdw);
}

/////////////////////////////////////////////////////////////////////////////////////

DCC* DCC::Create_Pololu_MC33926Shield_Main(int numDev) {
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

    hdw.config_setRailcom(false);

    return new DCC(numDev, hdw);
}

DCC* DCC::Create_Pololu_MC33926Shield_Prog(int numDev) {
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

    hdw.config_setRailcom(false);

    return new DCC(numDev, hdw);
}

/////////////////////////////////////////////////////////////////////////////////////

#if defined(ARDUINO_ARCH_SAMD)
// TI DRV8874 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Main(int numDev) {
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

    hdw.config_setRailcom(true);
    hdw.config_setRailcomRxPin(5);
    hdw.config_setRailcomTxPin(2);     // Doesn't do anything, but valid pin must be specified to instantiate railcom_serial on some architectures
    hdw.config_setRailcomBaud(250000);
    hdw.config_setRailcomSerial(nullptr);  // Will be initialized in Hardware::init();
    hdw.config_setRailcomSercom(&sercom4);
    hdw.config_setRailcomRxMux(PIO_SERCOM_ALT);
    hdw.config_setRailcomRxPad(SERCOM_RX_PAD_3);
    hdw.config_setRailcomTxPad(UART_TX_PAD_2);

    return new DCC(numDev, hdw);
}

// TI DRV8876 on custom board
DCC* DCC::Create_WSM_SAMCommandStation_Prog(int numDev) {
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

    hdw.config_setRailcom(false);
    hdw.config_setRailcomRxPin(13);

    return new DCC(numDev, hdw);
}

#endif // ARCH_ARDUINO_SAMD