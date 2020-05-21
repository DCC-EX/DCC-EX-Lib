#ifndef EEPROM_h
#define EEPROM_h

#include <SparkFun_External_EEPROM.h>
#include <Arduino.h>

extern ExternalEEPROM EEPROM;

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

#endif