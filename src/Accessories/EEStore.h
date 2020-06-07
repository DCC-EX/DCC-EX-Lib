#ifndef COMMANDSTATION_ACCESSORIES_EESTORE_H_
#define COMMANDSTATION_ACCESSORIES_EESTORE_H_

#include <Arduino.h>

#if defined(ARDUINO_ARCH_SAMD)
#include <SparkFun_External_EEPROM.h>
#endif

#if defined(ARDUINO_ARCH_SAMD)
extern ExternalEEPROM EEPROM;
#endif

#define EESTORE_ID "DCC++"

struct EEStoreData{
    char id[sizeof(EESTORE_ID)];
    int nTurnouts;
    int nSensors;  
    int nOutputs;
};

struct EEStore{
    static EEStore *eeStore;
    EEStoreData data;
    static int eeAddress;
    static void init();
    static void reset();
    static int pointer();
    static void advance(int);
    static void store();
    static void clear();
};

#endif  // COMMANDSTATION_ACCESSORIES_EESTORE_H_